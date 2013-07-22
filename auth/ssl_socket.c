/*
 * sll_socket.c  Wrapping Open SSL calls to work with SLL sockets 
 * 
 */

#include "aconfig.h"
#include "nntpd.h"
#include "aprotos.h"
#include "openssl/err.h"

#define  MAX_ERROR_MESSAGE 121

static char ssl_password[128] = "1234";        

/*
    Writes an error to system log.
    Error consists of function name and error message from OpenSSL librar
        ssl  - SSL handler
        func - function call/name of OpenSSL
        ret  - value returned by call
*/
void ssl_error (SSL *ssl, const char *func, int ret)
{
    int err = SSL_get_error (ssl, ret);
    char msg [MAX_ERROR_MESSAGE];
    ERR_error_string (err,msg);
    syslog (LOG_DEBUG, "SSL call %s cause error: %s", func, msg);
}

/*
    Password callback function
*/
static int password_callback(char *buf, int num, int rwflag, void *userdata)
{
    if(num < strlen(ssl_password)+1)
        return(0);

    strcpy(buf, ssl_password);
    return(strlen(ssl_password));
}


/*
    Performs initialization of SSL context 
*/
PROTO SSL_CTX *ssl_init_ctx (char *keyfile, char *password, char *ca_list) 
{
    SSL_CTX *ctx = SSL_CTX_new (SSLv23_server_method());
 
    if(!SSL_CTX_use_certificate_chain_file(ctx, keyfile))
    {
        syslog (LOG_ERR, "Could not read SSL certificate from file %s", keyfile);
        return NULL;
    }
    info("Loaded SSL certificate from file %s", keyfile); 

    strncpy (ssl_password, password, sizeof(ssl_password)-1);
    SSL_CTX_set_default_passwd_cb(ctx, password_callback);
    
    
    if(!SSL_CTX_use_PrivateKey_file(ctx, keyfile, SSL_FILETYPE_PEM)) 
    {
        syslog (LOG_ERR,  "Cant read key file %s", keyfile );
        return NULL;
    }
    info("Loaded SSL private key"); 

    /* Load the CAs we trust*/
    if(!(SSL_CTX_load_verify_locations(ctx, ca_list, 0))) 
    {
        syslog (LOG_ERR, "Can't read CA list");
        return NULL;
    }
    info("Loaded CA list");
    return ctx;
} 

/*
    Perfroms initialization of OpenSSL library
*/
PROTO void ssl_init(void)
{
    SSL_load_error_strings(); 
    SSL_library_init();
}

/*
    Closes OpenSSL  library
*/
PROTO void ssl_done(SSL_CTX * ssl_ctx)
{
        SSL_CTX_free(ssl_ctx);
}

/* 
    Initializes SSL server part of socket from original non-sucure socket.
    Performs negotiation.
*/
PROTO SSL* ssl_init_accept (int socket, SSL_CTX * ssl_ctx)
{
    SSL *ssl = SSL_new(ssl_ctx);
    if (ssl)
    {

        fcntl(socket, F_SETFL, fcntl(socket, F_GETFL) | O_NONBLOCK );
        if (!SSL_set_fd (ssl, socket)) 
        {
            syslog (LOG_DEBUG, "Error applying SSL socket" );
            SSL_free (ssl);
            ssl = NULL;
        }
        else 
        {
            int cont = 1;
            while ( cont )
            {
                int ret = SSL_accept (ssl);
                switch ( ret ) 
                {
                    case 1:     // successfull handshake
                        cont = 0;
                        break;

                    case -1:    // maybe we need to repeat after some pause
                    {
                        int err = SSL_get_error(ssl, ret);
                        if ( (err != SSL_ERROR_WANT_READ) && (err != SSL_ERROR_WANT_WRITE) )
                        {
                            ssl_error (ssl, "SSL_accept", ret);
                            SSL_free (ssl);
                            ssl = NULL;
                        }
                        else
                        {       // we need to make pause then repeat
                            fd_set rdfs;
                            struct timeval tv;
                            FD_ZERO(&rdfs);
                            FD_SET(socket, &rdfs);
                            tv.tv_sec  = client->profile->ClientReadTimeout;
                            tv.tv_usec = 0;

                            int sel_ret = select(socket+1, &rdfs, NULL, NULL, &tv);

                            if ( sel_ret == 0 )   
                            {   // timeout 
                                syslog (LOG_DEBUG, "SSL_accept timeout");
                                SSL_free (ssl);
                                ssl = NULL;
                            } 
                            else if ( sel_ret < 0 ) 
                            {   // socket error
                                syslog (LOG_DEBUG, "SSL underlaying socket error (select() failed)");
                                SSL_free (ssl);
                                ssl = NULL;
                            }
                            // else repeat last op

                        }
                        break;
                    }

                    default:
                    {
                        ssl_error (ssl, "SSL_accept", ret);
                        SSL_free (ssl);
                        ssl = NULL;
                    }
                }   // switch
                if ( !ssl ) 
                    cont = 0;
            }   // while
            fcntl(socket, F_SETFL, fcntl(socket, F_GETFL) & ~O_NONBLOCK );
        }
    }
    else
    {
            syslog (LOG_ERR, "Error creating SSL socket");
    }
    return ssl;
}

/*
    Reads from SSL sockets and returns value >0(number of read bytes) if success
*/
PROTO int ssl_read (SSL* ssl, void *buffer, int buf_len)
{
    int ret;
    do
    {
        ret = SSL_read (ssl, buffer, buf_len);
        /* 
            In case of SSL_ERROR_WANT_READ status the call will be repeated because of
            rehandshaking takes place
        */ 
    } while (SSL_get_error(ssl, ret) == SSL_ERROR_WANT_READ);

    if (ret <= 0)
        ssl_error (ssl, "SSL_read", ret);
    return ret;
}

/*
    Reads string from SSL socket until EOL or maxlen reached, returns whole string
*/
PROTO char *ssl_getsrn(SSL* ssl, char* ptr, int maxlen)
{
        int     n, rc;
        char    c;

        for (n = 1; n < maxlen; n++) 
        {
                if ( (rc = ssl_read(ssl, &c, 1)) == 1) 
                {
                        *ptr++ = c;
                        if (c == '\n')
                                break;
                } else if (rc == 0) {
                        if (n == 1)
                                return(NULL);   /* EOF, no data read */
                        else
                                break;          /* EOF, some data was read */
                } else {
                        if ( errno ) set_errormsg("ssl_getsrn: %m");
                        return(NULL);   /* error */
                }
        }

        *ptr = 0;
        return(ptr);
}

/*
    Reads a text line from client with timeout if need
*/

PROTO char *ssl_readclient(CLIENT* client, char *buf, int size, int nowait) 
{
        fd_set rdfs;
        struct timeval tv;

        int wait_condition = (!nowait) && (!SSL_pending(client->ssl));
        if ( wait_condition ) 
        {
                FD_ZERO(&rdfs);
                FD_SET(client->socket, &rdfs);

                tv.tv_sec  = client->profile->ClientReadTimeout;
                tv.tv_usec = 0;

                if ( select(client->socket+1, &rdfs, NULL, NULL, &tv) == 0 ) 
                {
                        client->timeout++;
                        return NULL;
                }
        }
        return ssl_getsrn(client->ssl, buf, size);
}

/*
    Reads from  client SSL socket with specified  timeout
*/
PROTO ssize_t ssl_read_client_timeout (CLIENT* client, char *buf ,int size, int timeout)
{
            fd_set rdfs;
            struct timeval tv;

            FD_ZERO(&rdfs);
            FD_SET(client->socket, &rdfs);

            tv.tv_sec  = timeout;
            tv.tv_usec = 0;

            if ( SSL_pending(client->ssl) == 0 &&
                 select(client->socket+1, &rdfs, NULL, NULL, &tv) == 0 ) 
            {
                syslog(LOG_DEBUG, "client_read_socket: select timeout");
                return -1;
            }

            ssize_t nread = ssl_read(client->ssl, buf, size);

            return nread;
}



/*
    Writes to SSL sockets and returns value >0(number of written bytes) if success
*/
PROTO int ssl_write(SSL* ssl, void *buffer, int buf_len)
{
    int ret;
    do
    {
        ret = SSL_write(ssl, buffer, buf_len);
        /*
            In case of SSL_ERROR_WANT_WRITE status the call will be repeated because of
            rehandshaking takes place
        */
    } while (SSL_get_error(ssl, ret) == SSL_ERROR_WANT_WRITE);

    if (ret <= 0)
        ssl_error (ssl, "SSL_write", ret);
    return ret;
}

/*
    Writes string plus \r\n characters to SSL socket
*/
PROTO int ssl_writeln(SSL* ssl, char *str, int len)
{
    char buf[8192];

    if ( len > 8189 ) {
        syslog(LOG_ERR, "ssl_writeln: buf > 8189");
        return -1;
    }

    strcpy(buf, str);
    buf[len] = '\r';
    buf[len+1] = '\n';
    buf[len+2] = '\0';

    return ssl_write(ssl, buf, len+2);
}

/*
    Closes socket on SSL
*/
PROTO void ssl_close (SSL* ssl) 
{
    SSL_shutdown (ssl);
}
