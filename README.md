NNTPSwitch 0.12+
----------------

This is NNTPSwitch 0.12+

This is the last available production version when www.news-service.com was shut down.

See documentation at: http://redmine.chiparus.net/projects/nntpswitch/wiki

----------------

# wheezy
deb http://archive.debian.org/debian wheezy main contrib non-free
deb http://archive.debian.org/debian-security wheezy/updates main contrib non-free
deb http://archive.debian.org/debian wheezy-backports main contrib non-free

apt-get install build-essential unzip bison libmysqlclient-dev libpq-dev libssl-dev libcurl3-openssl-dev libjson-c-dev libmcrypt-dev libldap2-dev

# stretch
apt install unzip bison default-libmysqlclient-dev libpq-dev libssl1.0-dev libcurl4-openssl-dev libjson-c-dev libmcrypt-dev libldap2-dev

# on a 64bit system add -fPIC to the CFLAGS variable in the makefile of both packages.
```
cd /usr/src
# tacacs/libmd.tar.gz 10880 bytes
# MD5 3b114cb6d5b491bbf78dbf89d4a60b0e  libmd.tar.gz
# SHA256 b01190f41c988e77e58f81e0e531817f527db51ae2747ae618771ede991dbe13  libmd.tar.gz
# wget http://portal-to-web.de/tacacs/libmd.tar.gz
# wget https://up.ovpn.to/file/3k452BGrLjSinZlhpH4yxnIXA9HmKj5OpFhhe7oz/libmd.tar.gz
tar xfvz libmd.tar.gz
cd libmd
mkdir /usr/local/man/man3
make  && make install

cd /usr/src
# tacacs/libradius-linux-20040827.tar.gz 15791 bytes
# MD5 c7cc2f49acdd9955a052029326833fe1  libradius-linux-20040827.tar.gz
# SHA256 e8beead35a82836171f1e5a199b185555d35f12acaaa86c6533af47b4a4f109b  libradius-linux-20040827.tar.gz
# wget http://portal-to-web.de/tacacs/libradius-linux-20040827.tar.gz
# wget https://up.ovpn.to/file/Spd7lU1oTwcpmcEwtgEt6Q780fePIZ6WM4TzSg1O/libradius-linux-20040827.tar.gz
tar xfvz libradius-linux-20040827.tar.gz
cd libradius-linux
make && make install

useradd -m -s /bin/bash nntpswitch
cd /home/nntpswitch
su nntpswitch

cd #
wget https://github.com/tomarus/nntpswitch/archive/master.zip
unzip master.zip
cd nntpswitch-master
```

libmd
```
6480eedccaab7e014ccc4bd2229b3e7d  Makefile
3fc668845d2980b8bb36446786feabe7  md2c.c
8382580ecd575810b50148d28a6e9bc0  md2.copyright
3dea7466f4a71fbc6832666a8de798da  md2.h
5dc2525dbd65e2d7a582595eac5c0f79  md4c.c
08810d568d0e710e411fa6183d15f66f  md4.copyright
a50233a9c763895aa1eb3fc500507e09  md4.h
ad3f7fd831f3227a7963d03d9377ed6e  md5c.c
4fd5a5f031473d08f902f754b0d9d74f  md5.copyright
b9fa9bc32c149c7d3ffe96a7c2dcde6c  md5.h
faaf018f540fc59e668fb35d0f324e2c  mddriver.c
dc1246470757f8b927dd98a32ca69d7b  mdX.3
1c0e63499802048cfe5a8c0ffedb2eea  mdXhl.c
8f8a3e36457856c6c5906c76dc7c02b5  README
```

libradius-linux-20040827
```
3529d884538b1007345ed8b80a220f77  libradius.3
ff94c2c30331cc004698d904f7c143b1  Makefile
b9fa9bc32c149c7d3ffe96a7c2dcde6c  md5.h
cd2f13f4ceddd295b090194f562cf311  radius.conf.5
44816c51b83d4ae516065ff4e2f7c7c6  radlib.c
8d590ceac33a370d5d9809cb674bec29  radlib.h
f98c5f450ad6dfb8cd9d86d8bbd6aef2  radlib_private.h
f8df6c1e97cc4471ddc72ec88fb85298  README
```

bash RESTORE libmd
```
#!/bin/bash
DIRNAME=libmd
mkdir "libmd" && cd "libmd" || exit 1

MD5=3529d884538b1007345ed8b80a220f77 # libradius.3
cat <<'EOF' > "libradius.3"
.\" Copyright 1998 Juniper Networks, Inc.
.\" All rights reserved.
.\"
.\" Redistribution and use in source and binary forms, with or without
.\" modification, are permitted provided that the following conditions
.\" are met:
.\" 1. Redistributions of source code must retain the above copyright
.\"    notice, this list of conditions and the following disclaimer.
.\" 2. Redistributions in binary form must reproduce the above copyright
.\"    notice, this list of conditions and the following disclaimer in the
.\"    documentation and/or other materials provided with the distribution.
.\"
.\" THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
.\" ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
.\" IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
.\" ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
.\" FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
.\" DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
.\" OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
.\" HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
.\" LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
.\" OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
.\" SUCH DAMAGE.
.\"
.\" $FreeBSD: src/lib/libradius/libradius.3,v 1.6.2.4 2001/12/17 10:08:31 ru Exp $
.\"
.Dd October 30, 1999
.Dt LIBRADIUS 3
.Os
.Sh NAME
.Nm libradius
.Nd RADIUS client library
.Sh SYNOPSIS
.In radlib.h
.Ft struct rad_handle *
.Fn rad_acct_open "void"
.Ft int
.Fn rad_add_server "struct rad_handle *h" "const char *host" "int port" "const char *secret" "int timeout" "int max_tries"
.Ft struct rad_handle *
.Fn rad_auth_open "void"
.Ft void
.Fn rad_close "struct rad_handle *h"
.Ft int
.Fn rad_config "struct rad_handle *h" "const char *file"
.Ft int
.Fn rad_continue_send_request "struct rad_handle *h" "int selected" "int *fd" "struct timeval *tv"
.Ft int
.Fn rad_create_request "struct rad_handle *h" "int code"
.Ft struct in_addr
.Fn rad_cvt_addr "const void *data"
.Ft u_int32_t
.Fn rad_cvt_int "const void *data"
.Ft char *
.Fn rad_cvt_string "const void *data" "size_t len"
.Ft int
.Fn rad_get_attr "struct rad_handle *h" "const void **data" "size_t *len"
.Ft int
.Fn rad_init_send_request "struct rad_handle *h" "int *fd" "struct timeval *tv"
.Ft int
.Fn rad_put_addr "struct rad_handle *h" "int type" "struct in_addr addr"
.Ft int
.Fn rad_put_attr "struct rad_handle *h" "int type" "const void *data" "size_t len"
.Ft int
.Fn rad_put_int "struct rad_handle *h" "int type" "u_int32_t value"
.Ft int
.Fn rad_put_string "struct rad_handle *h" "int type" "const char *str"
.Ft int
.Fn rad_send_request "struct rad_handle *h"
.Ft const char *
.Fn rad_strerror "struct rad_handle *h"
.Sh DESCRIPTION
The
.Nm
library implements the client side of the Remote Authentication Dial
In User Service (RADIUS).  RADIUS, defined in RFCs 2138 and 2139,
allows clients to perform authentication and accounting by means of
network requests to remote servers.
.Sh INITIALIZATION
To use the library, an application must first call
.Fn rad_auth_open
or
.Fn rad_acct_open
to obtain a
.Va struct rad_handle * ,
which provides the context for subsequent operations.
The former function is used for RADIUS authentication and the
latter is used for RADIUS accounting.
Calls to
.Fn rad_auth_open
and
.Fn rad_acct_open
always succeed unless insufficient virtual memory is available.  If
the necessary memory cannot be allocated, the functions return
.Dv NULL .
For compatibility with earlier versions of this library,
.Fn rad_open
is provided as a synonym for
.Fn rad_auth_open .
.Pp
Before issuing any RADIUS requests, the library must be made aware
of the servers it can contact.  The easiest way to configure the
library is to call
.Fn rad_config .
.Fn rad_config
causes the library to read a configuration file whose format is
described in
.Xr radius.conf 5 .
The pathname of the configuration file is passed as the
.Va file
argument to
.Fn rad_config .
This argument may also be given as
.Dv NULL ,
in which case the standard configuration file
.Pa /etc/radius.conf
is used.
.Fn rad_config
returns 0 on success, or -1 if an error occurs.
.Pp
The library can also be configured programmatically by calls to
.Fn rad_add_server .
The
.Va host
parameter specifies the server host, either as a fully qualified
domain name or as a dotted-quad IP address in text form.
The
.Va port
parameter specifies the UDP port to contact on the server.  If
.Va port
is given as 0, the library looks up the
.Ql radius/udp
or
.Ql radacct/udp
service in the network services database, and uses the port found
there.  If no entry is found, the library uses the standard RADIUS
ports, 1812 for authentication and 1813 for accounting.
The shared secret for the server host is passed to the
.Va secret
parameter.
It may be any NUL-terminated string of bytes.  The RADIUS protocol
ignores all but the leading 128 bytes of the shared secret.
The timeout for receiving replies from the server is passed to the
.Va timeout
parameter, in units of seconds.  The maximum number of repeated
requests to make before giving up is passed into the
.Va max_tries
parameter.
.Fn rad_add_server
returns 0 on success, or -1 if an error occurs.
.Pp
.Fn rad_add_server
may be called multiple times, and it may be used together with
.Fn rad_config .
At most 10 servers may be specified.
When multiple servers are given, they are tried in round-robin
fashion until a valid response is received, or until each server's
.Va max_tries
limit has been reached.
.Sh CREATING A RADIUS REQUEST
A RADIUS request consists of a code specifying the kind of request,
and zero or more attributes which provide additional information.  To
begin constructing a new request, call
.Fn rad_create_request .
In addition to the usual
.Va struct rad_handle * ,
this function takes a
.Va code
parameter which specifies the type of the request.  Most often this
will be
.Dv RAD_ACCESS_REQUEST .
.Fn rad_create_request
returns 0 on success, or -1 on if an error occurs.
.Pp
After the request has been created with
.Fn rad_create_request ,
attributes can be attached to it.  This is done through calls to
.Fn rad_put_addr ,
.Fn rad_put_int ,
and
.Fn rad_put_string .
Each accepts a
.Va type
parameter identifying the attribute, and a value which may be
an Internet address, an integer, or a NUL-terminated string,
respectively.
.Pp
The library also provides a function
.Fn rad_put_attr
which can be used to supply a raw, uninterpreted attribute.  The
.Va data
argument points to an array of bytes, and the
.Va len
argument specifies its length.
.Pp
The
.Fn rad_put_X
functions return 0 on success, or -1 if an error occurs.
.Sh SENDING THE REQUEST AND RECEIVING THE RESPONSE
After the RADIUS request has been constructed, it is sent either by means of
.Fn rad_send_request
or by a combination of calls to
.Fn rad_init_send_request
and
.Fn rad_continue_send_request .
.Pp
The
.Fn rad_send_request
function sends the request and waits for a valid reply,
retrying the defined servers in round-robin fashion as necessary.
If a valid response is received,
.Fn rad_send_request
returns the RADIUS code which specifies the type of the response.
This will typically be
.Dv RAD_ACCESS_ACCEPT ,
.Dv RAD_ACCESS_REJECT ,
or
.Dv RAD_ACCESS_CHALLENGE .
If no valid response is received,
.Fn rad_send_request
returns -1.
.Pp
As an alternative, if you do not wish to block waiting for a response,
.Fn rad_init_send_request
and
.Fn rad_continue_send_request
may be used instead.  If a reply is received from the RADIUS server or a
timeout occurs, these functions return a value as described for
.Fn rad_send_request .
Otherwise, a value of zero is returned and the values pointed to by
.Ar fd
and
.Ar tv
are set to the descriptor and timeout that should be passed to
.Xr select 2 .
.Pp
.Fn rad_init_send_request
must be called first, followed by repeated calls to
.Fn rad_continue_send_request
as long as a return value of zero is given.
Between each call, the application should call
.Xr select 2 ,
passing
.Ar *fd
as a read descriptor and timing out after the interval specified by
.Ar tv .
When select returns,
.Fn rad_continue_send_request
should be called with
.Ar selected
set to a non-zero value if
.Xr select 2
indicated that the descriptor is readable.
.Pp
Like RADIUS requests, each response may contain zero or more
attributes.  After a response has been received successfully by
.Fn rad_send_request
or
.Fn rad_continue_send_request ,
its attributes can be extracted one by one using
.Fn rad_get_attr .
Each time
.Fn rad_get_attr
is called, it gets the next attribute from the current response, and
stores a pointer to the data and the length of the data via the
reference parameters
.Va data
and
.Va len ,
respectively.  Note that the data resides in the response itself,
and must not be modified.
A successful call to
.Fn rad_get_attr
returns the RADIUS attribute type.
If no more attributes remain in the current response,
.Fn rad_get_attr
returns 0.
If an error such as a malformed attribute is detected, -1 is
returned.
.Pp
The common types of attributes can be decoded using
.Fn rad_cvt_addr ,
.Fn rad_cvt_int ,
and
.Fn rad_cvt_string .
These functions accept a pointer to the attribute data, which should
have been obtained using
.Fn rad_get_attr .
In the case of
.Fn rad_cvt_string ,
the length
.Va len
must also be given.  These functions interpret the attribute as an
Internet address, an integer, or a string, respectively, and return
its value.
.Fn rad_cvt_string
returns its value as a NUL-terminated string in dynamically
allocated memory.  The application should free the string using
.Xr free 3
when it is no longer needed.
.Pp
If insufficient virtual memory is available,
.Fn rad_cvt_string
returns
.Dv NULL .
.Fn rad_cvt_addr
and
.Fn rad_cvt_int
cannot fail.
.Sh OBTAINING ERROR MESSAGES
Those functions which accept a
.Va struct rad_handle *
argument record an error message if they fail.  The error message
can be retrieved by calling
.Fn rad_strerror .
The message text is overwritten on each new error for the given
.Va struct rad_handle * .
Thus the message must be copied if it is to be preserved through
subsequent library calls using the same handle.
.Sh CLEANUP
To free the resources used by the RADIUS library, call
.Fn rad_close .
.Sh RETURN VALUES
The following functions return a non-negative value on success.  If
they detect an error, they return -1 and record an error message
which can be retrieved using
.Fn rad_strerror .
.Pp
.Bl -item -offset indent -compact
.It
.Fn rad_add_server
.It
.Fn rad_config
.It
.Fn rad_create_request
.It
.Fn rad_get_attr
.It
.Fn rad_put_addr
.It
.Fn rad_put_attr
.It
.Fn rad_put_int
.It
.Fn rad_put_string
.It
.Fn rad_init_send_request
.It
.Fn rad_continue_send_request
.It
.Fn rad_send_request
.El
.Pp
The following functions return a
.No non- Ns Dv NULL
pointer on success.  If they are unable to allocate sufficient
virtual memory, they return
.Dv NULL ,
without recording an error message.
.Pp
.Bl -item -offset indent -compact
.It
.Fn rad_acct_open
.It
.Fn rad_auth_open
.It
.Fn rad_cvt_string
.El
.Sh FILES
.Pa /etc/radius.conf
.Sh SEE ALSO
.Xr radius.conf 5
.Rs
.%A C. Rigney, et al
.%T "Remote Authentication Dial In User Service (RADIUS)"
.%O RFC 2138
.Re
.Rs
.%A C. Rigney
.%T RADIUS Accounting
.%O RFC 2139
.Re
.Sh AUTHORS
This software was originally written by
.An John Polstra ,
and donated to the
.Fx
project by Juniper Networks, Inc.
Oleg Semyonov subsequently added the ability to perform RADIUS
accounting.
EOF
FMD5=$(md5sum "libradius.3"|cut -d' ' -f1)
test "$MD5" = "$FMD5" && echo "restored: libradius.3 (md5 ok)" || echo "ERROR: libradius.3 (!md5)"
MD5=ff94c2c30331cc004698d904f7c143b1 # Makefile
cat <<'EOF' > "Makefile"
# Copyright 1998 Juniper Networks, Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
#	$FreeBSD: src/lib/libradius/Makefile,v 1.1.1.1 1998/11/13 00:53:01 jdp Exp $

PREFIX=	/usr/local

RM=		rm -f
LN=		ln
CC=		cc
CFLAGS= -O2

# linux -L$(PREFIX)/lib -lmd
LIBMD= -L$(PREFIX)/lib -lmd

# netbsd - it's in libc
#LIBMD= -L/usr/lib -lc


LIB=    radius
OBJS=   radlib.o

all:    lib$(LIB).a


lib$(LIB).a:    $(OBJS)
	ar cru $@ $(OBJS)
	ranlib $@

radlib.o:       radlib.c
	$(CC) $(CFLAGS) -c radlib.c -o radlib_r.o
	ld -r -o radlib.o radlib_r.o -L$(PREFIX)/lib $(LIBMD)
	$(CC) -shared radlib_r.o -o libradius.so.1.0  -L$(PREFIX)/lib $(LIBMD)
	$(RM) radlib_r.o

clean:
	$(RM) *.o

clobber:	clean
	$(RM) lib$(LIB).a

install:	all
	install -c -m 644 -o bin -g bin lib$(LIB).a $(PREFIX)/lib
	install -c -m 644 -o bin -g bin lib$(LIB).so.1.0 $(PREFIX)/lib
	$(LN) -sf $(PREFIX)/lib/lib$(LIB).so.1.0 $(PREFIX)/lib/lib$(LIB).so
	install -c -m 444 -o bin -g bin radlib.h $(PREFIX)/include
	install -c -m 444 -o bin -g bin libradius.3 $(PREFIX)/man/man3
	install -c -m 444 -o bin -g bin radius.conf.5 $(PREFIX)/man/man5
EOF
FMD5=$(md5sum "Makefile"|cut -d' ' -f1)
test "$MD5" = "$FMD5" && echo "restored: Makefile (md5 ok)" || echo "ERROR: Makefile (!md5)"
MD5=b9fa9bc32c149c7d3ffe96a7c2dcde6c # md5.h
cat <<'EOF' > "md5.h"
/* MD5.H - header file for MD5C.C
 * $Id: md5.h,v 1.6.2.1 1998/02/18 02:28:14 jkh Exp $
 */

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
 */

#ifndef _MD5_H_
#define _MD5_H_
/* MD5 context. */
typedef struct MD5Context {
  u_int32_t state[4];	/* state (ABCD) */
  u_int32_t count[2];	/* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];	/* input buffer */
} MD5_CTX;

#include <sys/cdefs.h>

__BEGIN_DECLS
void   MD5Init (MD5_CTX *);
void   MD5Update (MD5_CTX *, const unsigned char *, unsigned int);
void   MD5Final (unsigned char [16], MD5_CTX *);
char * MD5End(MD5_CTX *, char *);
char * MD5File(const char *, char *);
char * MD5Data(const unsigned char *, unsigned int, char *);
__END_DECLS

#endif /* _MD5_H_ */
EOF
FMD5=$(md5sum "md5.h"|cut -d' ' -f1)
test "$MD5" = "$FMD5" && echo "restored: md5.h (md5 ok)" || echo "ERROR: md5.h (!md5)"
MD5=cd2f13f4ceddd295b090194f562cf311 # radius.conf.5
cat <<'EOF' > "radius.conf.5"
.\" Copyright 1998 Juniper Networks, Inc.
.\" All rights reserved.
.\"
.\" Redistribution and use in source and binary forms, with or without
.\" modification, are permitted provided that the following conditions
.\" are met:
.\" 1. Redistributions of source code must retain the above copyright
.\"    notice, this list of conditions and the following disclaimer.
.\" 2. Redistributions in binary form must reproduce the above copyright
.\"    notice, this list of conditions and the following disclaimer in the
.\"    documentation and/or other materials provided with the distribution.
.\"
.\" THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
.\" ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
.\" IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
.\" ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
.\" FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
.\" DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
.\" OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
.\" HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
.\" LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
.\" OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
.\" SUCH DAMAGE.
.\"
.\" $FreeBSD: src/lib/libradius/radius.conf.5,v 1.4.2.3 2001/08/17 15:43:06 ru Exp $
.\"
.Dd October 30, 1999
.Dt RADIUS.CONF 5
.Os
.Sh NAME
.Nm radius.conf
.Nd RADIUS client configuration file
.Sh SYNOPSIS
.Pa /etc/radius.conf
.Sh DESCRIPTION
.Nm
contains the information necessary to configure the RADIUS client
library.  It is parsed by
.Xr rad_config 3 .
The file contains one or more lines of text, each describing a
single RADIUS server which will be used by the library.  Leading
white space is ignored, as are empty lines and lines containing
only comments.
.Pp
A RADIUS server is described by three to five fields on a line:
.Pp
.Bl -item -offset indent -compact
.It
Service type
.It
Server host
.It
Shared secret
.It
Timeout
.It
Retries
.El
.Pp
The fields are separated by white space.  The
.Ql #
character at the beginning of a field begins a comment, which extends
to the end of the line.  A field may be enclosed in double quotes,
in which case it may contain white space and/or begin with the
.Ql #
character.  Within a quoted string, the double quote character can
be represented by
.Ql \e\&" ,
and the backslash can be represented by
.Ql \e\e .
No other escape sequences are supported.
.Pp
.Pp
The first field gives the service type, either
.Ql auth
for RADIUS authentication or
.Ql acct
for RADIUS accounting.  If a single server provides both services, two
lines are required in the file.  Earlier versions of this file did
not include a service type.  For backward compatibility, if the first
field is not
.Ql auth
or
.Ql acct
the library behaves as if
.Ql auth
were specified, and interprets the fields in the line as if they
were fields two through five.
.Pp
The second field specifies
the server host, either as a fully qualified domain name or as a
dotted-quad IP address.  The host may optionally be followed by a
.Ql \&:
and a numeric port number, without intervening white space.  If the
port specification is omitted, it defaults to the
.Ql radius
or
.Ql radacct
service in the
.Pa /etc/services
file for service types
.Ql auth
and
.Ql acct ,
respectively.
If no such entry is present, the standard ports 1812 and 1813 are
used.
.Pp
The third field contains the shared secret, which should be known
only to the client and server hosts.  It is an arbitrary string of
characters, though it must be enclosed in double quotes if it
contains white space.  The shared secret may be
any length, but the RADIUS protocol uses only the first 128
characters.  N.B., some popular RADIUS servers have bugs which
prevent them from working properly with secrets longer than 16
characters.
.Pp
The fourth field contains a decimal integer specifying the timeout in
seconds for receiving a valid reply from the server.  If this field
is omitted, it defaults to 3 seconds.
.Pp
The fifth field contains a decimal integer specifying the maximum
number of attempts that will be made to authenticate with the server
before giving up.  If omitted, it defaults to 3 attempts.  Note,
this is the total number of attempts and not the number of retries.
.Pp
Up to 10 RADIUS servers may be specified for each service type.
The servers are tried in
round-robin fashion, until a valid response is received or the
maximum number of tries has been reached for all servers.
.Pp
The standard location for this file is
.Pa /etc/radius.conf .
But an alternate pathname may be specified in the call to
.Xr rad_config 3 .
Since the file contains sensitive information in the form of the
shared secrets, it should not be readable except by root.
.Sh FILES
.Pa /etc/radius.conf
.Sh EXAMPLES
.Bd -literal
# A simple entry using all the defaults:
acct  radius1.domain.com  OurLittleSecret

# A server still using the obsolete RADIUS port, with increased
# timeout and maximum tries:
auth  auth.domain.com:1645  "I can't see you"  5  4

# A server specified by its IP address:
auth  192.168.27.81  $X*#..38947ax-+=
.Ed
.Sh SEE ALSO
.Xr libradius 3
.Rs
.%A C. Rigney, et al
.%T "Remote Authentication Dial In User Service (RADIUS)"
.%O RFC 2138
.Re
.Rs
.%A C. Rigney
.%T RADIUS Accounting
.%O RFC 2139
.Re
.Sh AUTHORS
This documentation was written by
.An John Polstra ,
and donated to the
.Fx
project by Juniper Networks, Inc.
EOF
FMD5=$(md5sum "radius.conf.5"|cut -d' ' -f1)
test "$MD5" = "$FMD5" && echo "restored: radius.conf.5 (md5 ok)" || echo "ERROR: radius.conf.5 (!md5)"
MD5=44816c51b83d4ae516065ff4e2f7c7c6 # radlib.c
cat <<'EOF' > "radlib.c"
/*-
 * Copyright 1998 Juniper Networks, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	$FreeBSD: src/lib/libradius/radlib.c,v 1.4.2.2 2000/11/09 00:24:26 eivind Exp $
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>
#include "md5.h"
#include <netdb.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "radlib_private.h"
#if defined(__linux__)
#define MSG_WAITALL     0
#endif
#if !defined(__printflike)
#define __printflike(fmtarg, firstvararg)                               \
        __attribute__((__format__ (__printf__, fmtarg, firstvararg)))
#endif
#if !defined(timersub)
#define timersub(tvp, uvp, vvp)                                         \
        do {                                                            \
                (vvp)->tv_sec = (tvp)->tv_sec - (uvp)->tv_sec;          \
                (vvp)->tv_usec = (tvp)->tv_usec - (uvp)->tv_usec;       \
                if ((vvp)->tv_usec < 0) {                               \
                        (vvp)->tv_sec--;                                \
                        (vvp)->tv_usec += 1000000;                      \
                }                                                       \
        } while (0)
#endif



static void	 clear_password(struct rad_handle *);
static void	 generr(struct rad_handle *, const char *, ...)
		    __printflike(2, 3);
static void	 insert_scrambled_password(struct rad_handle *, int);
static void	 insert_request_authenticator(struct rad_handle *, int);
static int	 is_valid_response(struct rad_handle *, int,
		    const struct sockaddr_in *);
static int	 put_password_attr(struct rad_handle *, int,
		    const void *, size_t);
static int	 put_raw_attr(struct rad_handle *, int,
		    const void *, size_t);
static int	 split(char *, char *[], int, char *, size_t);

static void
clear_password(struct rad_handle *h)
{
	if (h->pass_len != 0) {
		memset(h->pass, 0, h->pass_len);
		h->pass_len = 0;
	}
	h->pass_pos = 0;
}

static void
generr(struct rad_handle *h, const char *format, ...)
{
	va_list		 ap;

	va_start(ap, format);
	vsnprintf(h->errmsg, ERRSIZE, format, ap);
	va_end(ap);
}

static void
insert_scrambled_password(struct rad_handle *h, int srv)
{
	MD5_CTX ctx;
	unsigned char md5[16];
	const struct rad_server *srvp;
	int padded_len;
	int pos;

	srvp = &h->servers[srv];
	padded_len = h->pass_len == 0 ? 16 : (h->pass_len+15) & ~0xf;

	memcpy(md5, &h->request[POS_AUTH], LEN_AUTH);
	for (pos = 0;  pos < padded_len;  pos += 16) {
		int i;

		/* Calculate the new scrambler */
		MD5Init(&ctx);
		MD5Update(&ctx, srvp->secret, strlen(srvp->secret));
		MD5Update(&ctx, md5, 16);
		MD5Final(md5, &ctx);

		/*
		 * Mix in the current chunk of the password, and copy
		 * the result into the right place in the request.  Also
		 * modify the scrambler in place, since we will use this
		 * in calculating the scrambler for next time.
		 */
		for (i = 0;  i < 16;  i++)
			h->request[h->pass_pos + pos + i] =
			    md5[i] ^= h->pass[pos + i];
	}
}

static void
insert_request_authenticator(struct rad_handle *h, int srv)
{
	MD5_CTX ctx;
	const struct rad_server *srvp;

	srvp = &h->servers[srv];

	/* Create the request authenticator */
	MD5Init(&ctx);
	MD5Update(&ctx, &h->request[POS_CODE], POS_AUTH - POS_CODE);
	MD5Update(&ctx, memset(&h->request[POS_AUTH], 0, LEN_AUTH), LEN_AUTH);
	MD5Update(&ctx, &h->request[POS_ATTRS], h->req_len - POS_ATTRS);
	MD5Update(&ctx, srvp->secret, strlen(srvp->secret));
	MD5Final(&h->request[POS_AUTH], &ctx);
}

/*
 * Return true if the current response is valid for a request to the
 * specified server.
 */
static int
is_valid_response(struct rad_handle *h, int srv,
    const struct sockaddr_in *from)
{
	MD5_CTX ctx;
	unsigned char md5[16];
	const struct rad_server *srvp;
	int len;

	srvp = &h->servers[srv];

	/* Check the source address */
	if (from->sin_family != srvp->addr.sin_family ||
	    from->sin_addr.s_addr != srvp->addr.sin_addr.s_addr ||
	    from->sin_port != srvp->addr.sin_port)
		return 0;

	/* Check the message length */
	if (h->resp_len < POS_ATTRS)
		return 0;
	len = h->response[POS_LENGTH] << 8 | h->response[POS_LENGTH+1];
	if (len > h->resp_len)
		return 0;

	/* Check the response authenticator */
	MD5Init(&ctx);
	MD5Update(&ctx, &h->response[POS_CODE], POS_AUTH - POS_CODE);
	MD5Update(&ctx, &h->request[POS_AUTH], LEN_AUTH);
	MD5Update(&ctx, &h->response[POS_ATTRS], len - POS_ATTRS);
	MD5Update(&ctx, srvp->secret, strlen(srvp->secret));
	MD5Final(md5, &ctx);
	if (memcmp(&h->response[POS_AUTH], md5, sizeof md5) != 0)
		return 0;

	return 1;
}

static int
put_password_attr(struct rad_handle *h, int type, const void *value, size_t len)
{
	int padded_len;
	int pad_len;

	if (h->pass_pos != 0) {
		generr(h, "Multiple User-Password attributes specified");
		return -1;
	}
	if (len > PASSSIZE)
		len = PASSSIZE;
	padded_len = len == 0 ? 16 : (len+15) & ~0xf;
	pad_len = padded_len - len;

	/*
	 * Put in a place-holder attribute containing all zeros, and
	 * remember where it is so we can fill it in later.
	 */
	clear_password(h);
	put_raw_attr(h, type, h->pass, padded_len);
	h->pass_pos = h->req_len - padded_len;

	/* Save the cleartext password, padded as necessary */
	memcpy(h->pass, value, len);
	h->pass_len = len;
	memset(h->pass + len, 0, pad_len);
	return 0;
}

static int
put_raw_attr(struct rad_handle *h, int type, const void *value, size_t len)
{
	if (len > 253) {
		generr(h, "Attribute too long");
		return -1;
	}
	if (h->req_len + 2 + len > MSGSIZE) {
		generr(h, "Maximum message length exceeded");
		return -1;
	}
	h->request[h->req_len++] = type;
	h->request[h->req_len++] = len + 2;
	memcpy(&h->request[h->req_len], value, len);
	h->req_len += len;
	return 0;
}

int
rad_add_server(struct rad_handle *h, const char *host, int port,
    const char *secret, int timeout, int tries)
{
	struct rad_server *srvp;

	if (h->num_servers >= MAXSERVERS) {
		generr(h, "Too many RADIUS servers specified");
		return -1;
	}
	srvp = &h->servers[h->num_servers];

	memset(&srvp->addr, 0, sizeof srvp->addr);
#if !defined(__linux__)
        srvp->addr.sin_len = sizeof srvp->addr;
#endif
	srvp->addr.sin_family = AF_INET;
	if (!inet_aton(host, &srvp->addr.sin_addr)) {
		struct hostent *hent;

		if ((hent = gethostbyname(host)) == NULL) {
			generr(h, "%s: host not found", host);
			return -1;
		}
		memcpy(&srvp->addr.sin_addr, hent->h_addr,
		    sizeof srvp->addr.sin_addr);
	}
	if (port != 0)
		srvp->addr.sin_port = htons(port);
	else {
		struct servent *sent;

		if (h->type == RADIUS_AUTH)
			srvp->addr.sin_port =
			    (sent = getservbyname("radius", "udp")) != NULL ?
				sent->s_port : htons(RADIUS_PORT);
		else
			srvp->addr.sin_port =
			    (sent = getservbyname("radacct", "udp")) != NULL ?
				sent->s_port : htons(RADACCT_PORT);
	}
	if ((srvp->secret = strdup(secret)) == NULL) {
		generr(h, "Out of memory");
		return -1;
	}
	srvp->timeout = timeout;
	srvp->max_tries = tries;
	srvp->num_tries = 0;
	h->num_servers++;
	return 0;
}

void
rad_close(struct rad_handle *h)
{
	int srv;

	if (h->fd != -1)
		close(h->fd);
	for (srv = 0;  srv < h->num_servers;  srv++) {
		memset(h->servers[srv].secret, 0,
		    strlen(h->servers[srv].secret));
		free(h->servers[srv].secret);
	}
	clear_password(h);
	free(h);
}

int
rad_config(struct rad_handle *h, const char *path)
{
	FILE *fp;
	char buf[MAXCONFLINE];
	int linenum;
	int retval;

	if (path == NULL)
		path = PATH_RADIUS_CONF;
	if ((fp = fopen(path, "r")) == NULL) {
		generr(h, "Cannot open \"%s\": %s", path, strerror(errno));
		return -1;
	}
	retval = 0;
	linenum = 0;
	while (fgets(buf, sizeof buf, fp) != NULL) {
		int len;
		char *fields[5];
		int nfields;
		char msg[ERRSIZE];
		char *type;
		char *host, *res;
		char *port_str;
		char *secret;
		char *timeout_str;
		char *maxtries_str;
		char *end;
		char *wanttype;
		unsigned long timeout;
		unsigned long maxtries;
		int port;
		int i;

		linenum++;
		len = strlen(buf);
		/* We know len > 0, else fgets would have returned NULL. */
		if (buf[len - 1] != '\n') {
			if (len == sizeof buf - 1)
				generr(h, "%s:%d: line too long", path,
				    linenum);
			else
				generr(h, "%s:%d: missing newline", path,
				    linenum);
			retval = -1;
			break;
		}
		buf[len - 1] = '\0';

		/* Extract the fields from the line. */
		nfields = split(buf, fields, 5, msg, sizeof msg);
		if (nfields == -1) {
			generr(h, "%s:%d: %s", path, linenum, msg);
			retval = -1;
			break;
		}
		if (nfields == 0)
			continue;
		/*
		 * The first field should contain "auth" or "acct" for
		 * authentication or accounting, respectively.  But older
		 * versions of the file didn't have that field.  Default
		 * it to "auth" for backward compatibility.
		 */
		if (strcmp(fields[0], "auth") != 0 &&
		    strcmp(fields[0], "acct") != 0) {
			if (nfields >= 5) {
				generr(h, "%s:%d: invalid service type", path,
				    linenum);
				retval = -1;
				break;
			}
			nfields++;
			for (i = nfields;  --i > 0;  )
				fields[i] = fields[i - 1];
			fields[0] = "auth";
		}
		if (nfields < 3) {
			generr(h, "%s:%d: missing shared secret", path,
			    linenum);
			retval = -1;
			break;
		}
		type = fields[0];
		host = fields[1];
		secret = fields[2];
		timeout_str = fields[3];
		maxtries_str = fields[4];

		/* Ignore the line if it is for the wrong service type. */
		wanttype = h->type == RADIUS_AUTH ? "auth" : "acct";
		if (strcmp(type, wanttype) != 0)
			continue;

		/* Parse and validate the fields. */
		res = host;
		host = strsep(&res, ":");
		port_str = strsep(&res, ":");
		if (port_str != NULL) {
			port = strtoul(port_str, &end, 10);
			if (*end != '\0') {
				generr(h, "%s:%d: invalid port", path,
				    linenum);
				retval = -1;
				break;
			}
		} else
			port = 0;
		if (timeout_str != NULL) {
			timeout = strtoul(timeout_str, &end, 10);
			if (*end != '\0') {
				generr(h, "%s:%d: invalid timeout", path,
				    linenum);
				retval = -1;
				break;
			}
		} else
			timeout = TIMEOUT;
		if (maxtries_str != NULL) {
			maxtries = strtoul(maxtries_str, &end, 10);
			if (*end != '\0') {
				generr(h, "%s:%d: invalid maxtries", path,
				    linenum);
				retval = -1;
				break;
			}
		} else
			maxtries = MAXTRIES;

		if (rad_add_server(h, host, port, secret, timeout, maxtries) ==
		    -1) {
			strcpy(msg, h->errmsg);
			generr(h, "%s:%d: %s", path, linenum, msg);
			retval = -1;
			break;
		}
	}
	/* Clear out the buffer to wipe a possible copy of a shared secret */
	memset(buf, 0, sizeof buf);
	fclose(fp);
	return retval;
}

/*
 * rad_init_send_request() must have previously been called.
 * Returns:
 *   0     The application should select on *fd with a timeout of tv before
 *         calling rad_continue_send_request again.
 *   < 0   Failure
 *   > 0   Success
 */
int
rad_continue_send_request(struct rad_handle *h, int selected, int *fd,
                          struct timeval *tv)
{
	int n;

	if (selected) {
		struct sockaddr_in from;
		int fromlen;

		fromlen = sizeof from;
		h->resp_len = recvfrom(h->fd, h->response,
		    MSGSIZE, MSG_WAITALL, (struct sockaddr *)&from, &fromlen);
		if (h->resp_len == -1) {
			generr(h, "recvfrom: %s", strerror(errno));
			return -1;
		}
		if (is_valid_response(h, h->srv, &from)) {
			h->resp_len = h->response[POS_LENGTH] << 8 |
			    h->response[POS_LENGTH+1];
			h->resp_pos = POS_ATTRS;
			return h->response[POS_CODE];
		}
	}

	if (h->try == h->total_tries) {
		generr(h, "No valid RADIUS responses received");
		return -1;
	}

	/*
         * Scan round-robin to the next server that has some
         * tries left.  There is guaranteed to be one, or we
         * would have exited this loop by now.
	 */
	while (h->servers[h->srv].num_tries >= h->servers[h->srv].max_tries)
		if (++h->srv >= h->num_servers)
			h->srv = 0;

	if (h->request[POS_CODE] == RAD_ACCOUNTING_REQUEST)
		/* Insert the request authenticator into the request */
		insert_request_authenticator(h, h->srv);
	else
		/* Insert the scrambled password into the request */
		if (h->pass_pos != 0)
			insert_scrambled_password(h, h->srv);

	/* Send the request */
	n = sendto(h->fd, h->request, h->req_len, 0,
	    (const struct sockaddr *)&h->servers[h->srv].addr,
	    sizeof h->servers[h->srv].addr);
	if (n != h->req_len) {
		if (n == -1)
			generr(h, "sendto: %s", strerror(errno));
		else
			generr(h, "sendto: short write");
		return -1;
	}

	h->try++;
	h->servers[h->srv].num_tries++;
	tv->tv_sec = h->servers[h->srv].timeout;
	tv->tv_usec = 0;
	*fd = h->fd;

	return 0;
}

int
rad_create_request(struct rad_handle *h, int code)
{
	int i;

	h->request[POS_CODE] = code;
	h->request[POS_IDENT] = ++h->ident;
	/* Create a random authenticator */
	for (i = 0;  i < LEN_AUTH;  i += 2) {
		long r;
		r = random();
		h->request[POS_AUTH+i] = r;
		h->request[POS_AUTH+i+1] = r >> 8;
	}
	h->req_len = POS_ATTRS;
	clear_password(h);
	return 0;
}

struct in_addr
rad_cvt_addr(const void *data)
{
	struct in_addr value;

	memcpy(&value.s_addr, data, sizeof value.s_addr);
	return value;
}

u_int32_t
rad_cvt_int(const void *data)
{
	u_int32_t value;

	memcpy(&value, data, sizeof value);
	return ntohl(value);
}

char *
rad_cvt_string(const void *data, size_t len)
{
	char *s;

	s = malloc(len + 1);
	if (s != NULL) {
		memcpy(s, data, len);
		s[len] = '\0';
	}
	return s;
}

/*
 * Returns the attribute type.  If none are left, returns 0.  On failure,
 * returns -1.
 */
int
rad_get_attr(struct rad_handle *h, const void **value, size_t *len)
{
	int type;

	if (h->resp_pos >= h->resp_len)
		return 0;
	if (h->resp_pos + 2 > h->resp_len) {
		generr(h, "Malformed attribute in response");
		return -1;
	}
	type = h->response[h->resp_pos++];
	*len = h->response[h->resp_pos++] - 2;
	if (h->resp_pos + *len > h->resp_len) {
		generr(h, "Malformed attribute in response");
		return -1;
	}
	*value = &h->response[h->resp_pos];
	h->resp_pos += *len;
	return type;
}

/*
 * Returns -1 on error, 0 to indicate no event and >0 for success
 */
int
rad_init_send_request(struct rad_handle *h, int *fd, struct timeval *tv)
{
	int srv;

	/* Make sure we have a socket to use */
	if (h->fd == -1) {
		struct sockaddr_in sin;

		if ((h->fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
			generr(h, "Cannot create socket: %s", strerror(errno));
			return -1;
		}
		memset(&sin, 0, sizeof sin);
#if !defined(__linux__)
		sin.sin_len = sizeof sin;
#endif
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = INADDR_ANY;
		sin.sin_port = htons(0);
		if (bind(h->fd, (const struct sockaddr *)&sin,
		    sizeof sin) == -1) {
			generr(h, "bind: %s", strerror(errno));
			close(h->fd);
			h->fd = -1;
			return -1;
		}
	}

	if (h->request[POS_CODE] == RAD_ACCOUNTING_REQUEST) {
		/* Make sure no password given */
		if (h->pass_pos || h->chap_pass) {
			generr(h, "User or Chap Password in accounting request");
			return -1;
		}
	} else {
		/* Make sure the user gave us a password */
		if (h->pass_pos == 0 && !h->chap_pass) {
			generr(h, "No User or Chap Password attributes given");
			return -1;
		}
		if (h->pass_pos != 0 && h->chap_pass) {
			generr(h, "Both User and Chap Password attributes given");
			return -1;
		}
	}

	/* Fill in the length field in the message */
	h->request[POS_LENGTH] = h->req_len >> 8;
	h->request[POS_LENGTH+1] = h->req_len;

	/*
	 * Count the total number of tries we will make, and zero the
	 * counter for each server.
	 */
	h->total_tries = 0;
	for (srv = 0;  srv < h->num_servers;  srv++) {
		h->total_tries += h->servers[srv].max_tries;
		h->servers[srv].num_tries = 0;
	}
	if (h->total_tries == 0) {
		generr(h, "No RADIUS servers specified");
		return -1;
	}

	h->try = h->srv = 0;

	return rad_continue_send_request(h, 0, fd, tv);
}

/*
 * Create and initialize a rad_handle structure, and return it to the
 * caller.  Can fail only if the necessary memory cannot be allocated.
 * In that case, it returns NULL.
 */
struct rad_handle *
rad_auth_open(void)
{
	struct rad_handle *h;

	h = (struct rad_handle *)malloc(sizeof(struct rad_handle));
	if (h != NULL) {
#if defined(__linux__) || defined(__NetBSD__)
                        srandom((unsigned long)time(NULL));
#else
                        srandomdev();
#endif
		h->fd = -1;
		h->num_servers = 0;
		h->ident = random();
		h->errmsg[0] = '\0';
		memset(h->pass, 0, sizeof h->pass);
		h->pass_len = 0;
		h->pass_pos = 0;
		h->chap_pass = 0;
		h->type = RADIUS_AUTH;
	}
	return h;
}

struct rad_handle *
rad_acct_open(void)
{
	struct rad_handle *h;

	h = rad_open();
	if (h != NULL)
	        h->type = RADIUS_ACCT;
	return h;
}

struct rad_handle *
rad_open(void)
{
    return rad_auth_open();
}

int
rad_put_addr(struct rad_handle *h, int type, struct in_addr addr)
{
	return rad_put_attr(h, type, &addr.s_addr, sizeof addr.s_addr);
}

int
rad_put_attr(struct rad_handle *h, int type, const void *value, size_t len)
{
	int result;

	if (type == RAD_USER_PASSWORD)
		result = put_password_attr(h, type, value, len);
	else {
		result = put_raw_attr(h, type, value, len);
		if (result == 0 && type == RAD_CHAP_PASSWORD)
			h->chap_pass = 1;
	}

	return result;
}

int
rad_put_int(struct rad_handle *h, int type, u_int32_t value)
{
	u_int32_t nvalue;

	nvalue = htonl(value);
	return rad_put_attr(h, type, &nvalue, sizeof nvalue);
}

int
rad_put_string(struct rad_handle *h, int type, const char *str)
{
	return rad_put_attr(h, type, str, strlen(str));
}

/*
 * Returns the response type code on success, or -1 on failure.
 */
int
rad_send_request(struct rad_handle *h)
{
	struct timeval timelimit;
	struct timeval tv;
	int fd;
	int n;

	n = rad_init_send_request(h, &fd, &tv);

	if (n != 0)
		return n;

	gettimeofday(&timelimit, NULL);
	timeradd(&tv, &timelimit, &timelimit);

	for ( ; ; ) {
		fd_set readfds;

		FD_ZERO(&readfds);
		FD_SET(fd, &readfds);

		n = select(fd + 1, &readfds, NULL, NULL, &tv);

		if (n == -1) {
			generr(h, "select: %s", strerror(errno));
			return -1;
		}

		if (!FD_ISSET(fd, &readfds)) {
			/* Compute a new timeout */
			gettimeofday(&tv, NULL);
			timersub(&timelimit, &tv, &tv);
			if (tv.tv_sec > 0 || (tv.tv_sec == 0 && tv.tv_usec > 0))
				/* Continue the select */
				continue;
		}

		n = rad_continue_send_request(h, n, &fd, &tv);

		if (n != 0)
			return n;

		gettimeofday(&timelimit, NULL);
		timeradd(&tv, &timelimit, &timelimit);
	}
}

const char *
rad_strerror(struct rad_handle *h)
{
	return h->errmsg;
}

/*
 * Destructively split a string into fields separated by white space.
 * `#' at the beginning of a field begins a comment that extends to the
 * end of the string.  Fields may be quoted with `"'.  Inside quoted
 * strings, the backslash escapes `\"' and `\\' are honored.
 *
 * Pointers to up to the first maxfields fields are stored in the fields
 * array.  Missing fields get NULL pointers.
 *
 * The return value is the actual number of fields parsed, and is always
 * <= maxfields.
 *
 * On a syntax error, places a message in the msg string, and returns -1.
 */
static int
split(char *str, char *fields[], int maxfields, char *msg, size_t msglen)
{
	char *p;
	int i;
	static const char ws[] = " \t";

	for (i = 0;  i < maxfields;  i++)
		fields[i] = NULL;
	p = str;
	i = 0;
	while (*p != '\0') {
		p += strspn(p, ws);
		if (*p == '#' || *p == '\0')
			break;
		if (i >= maxfields) {
			snprintf(msg, msglen, "line has too many fields");
			return -1;
		}
		if (*p == '"') {
			char *dst;

			dst = ++p;
			fields[i] = dst;
			while (*p != '"') {
				if (*p == '\\') {
					p++;
					if (*p != '"' && *p != '\\' &&
					    *p != '\0') {
						snprintf(msg, msglen,
						    "invalid `\\' escape");
						return -1;
					}
				}
				if (*p == '\0') {
					snprintf(msg, msglen,
					    "unterminated quoted string");
					return -1;
				}
				*dst++ = *p++;
			}
			*dst = '\0';
			p++;
			if (*fields[i] == '\0') {
				snprintf(msg, msglen,
				    "empty quoted string not permitted");
				return -1;
			}
			if (*p != '\0' && strspn(p, ws) == 0) {
				snprintf(msg, msglen, "quoted string not"
				    " followed by white space");
				return -1;
			}
		} else {
			fields[i] = p;
			p += strcspn(p, ws);
			if (*p != '\0')
				*p++ = '\0';
		}
		i++;
	}
	return i;
}
EOF
FMD5=$(md5sum "radlib.c"|cut -d' ' -f1)
test "$MD5" = "$FMD5" && echo "restored: radlib.c (md5 ok)" || echo "ERROR: radlib.c (!md5)"
MD5=8d590ceac33a370d5d9809cb674bec29 # radlib.h
cat <<'EOF' > "radlib.h"
/*-
 * Copyright 1998 Juniper Networks, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	$FreeBSD: src/lib/libradius/radlib.h,v 1.3 1999/10/31 04:47:59 jdp Exp $
 */

#ifndef _RADLIB_H_
#define _RADLIB_H_

#include <sys/types.h>
#include <netinet/in.h>

/* Message types */
#define RAD_ACCESS_REQUEST		1
#define RAD_ACCESS_ACCEPT		2
#define RAD_ACCESS_REJECT		3
#define RAD_ACCOUNTING_REQUEST		4
#define RAD_ACCOUNTING_RESPONSE		5
#define RAD_ACCESS_CHALLENGE		11

/* Attribute types and values */
#define RAD_USER_NAME			1	/* String */
#define RAD_USER_PASSWORD		2	/* String */
#define RAD_CHAP_PASSWORD		3	/* String */
#define RAD_NAS_IP_ADDRESS		4	/* IP address */
#define RAD_NAS_PORT			5	/* Integer */
#define RAD_SERVICE_TYPE		6	/* Integer */
	#define RAD_LOGIN			1
	#define RAD_FRAMED			2
	#define RAD_CALLBACK_LOGIN		3
	#define RAD_CALLBACK_FRAMED		4
	#define RAD_OUTBOUND			5
	#define RAD_ADMINISTRATIVE		6
	#define RAD_NAS_PROMPT			7
	#define RAD_AUTHENTICATE_ONLY		8
	#define RAD_CALLBACK_NAS_PROMPT		9
#define RAD_FRAMED_PROTOCOL		7	/* Integer */
	#define RAD_PPP				1
	#define RAD_SLIP			2
	#define RAD_ARAP			3	/* Appletalk */
	#define RAD_GANDALF			4
	#define RAD_XYLOGICS			5
#define RAD_FRAMED_IP_ADDRESS		8	/* IP address */
#define RAD_FRAMED_IP_NETMASK		9	/* IP address */
#define RAD_FRAMED_ROUTING		10	/* Integer */
#define RAD_FILTER_ID			11	/* String */
#define RAD_FRAMED_MTU			12	/* Integer */
#define RAD_FRAMED_COMPRESSION		13	/* Integer */
	#define RAD_COMP_NONE			0
	#define RAD_COMP_VJ			1
	#define RAD_COMP_IPXHDR			2
#define RAD_LOGIN_IP_HOST		14	/* IP address */
#define RAD_LOGIN_SERVICE		15	/* Integer */
#define RAD_LOGIN_TCP_PORT		16	/* Integer */
     /* unassiged			17 */
#define RAD_REPLY_MESSAGE		18	/* String */
#define RAD_CALLBACK_NUMBER		19	/* String */
#define RAD_CALLBACK_ID			20	/* String */
     /* unassiged			21 */
#define RAD_FRAMED_ROUTE		22	/* String */
#define RAD_FRAMED_IPX_NETWORK		23	/* IP address */
#define RAD_STATE			24	/* String */
#define RAD_CLASS			25	/* Integer */
#define RAD_VENDOR_SPECIFIC		26	/* Integer */
#define RAD_SESSION_TIMEOUT		27	/* Integer */
#define RAD_IDLE_TIMEOUT		28	/* Integer */
#define RAD_TERMINATION_ACTION		29	/* Integer */
#define RAD_CALLED_STATION_ID		30	/* String */
#define RAD_CALLING_STATION_ID		31	/* String */
#define RAD_NAS_IDENTIFIER		32	/* Integer */
#define RAD_PROXY_STATE			33	/* Integer */
#define RAD_LOGIN_LAT_SERVICE		34	/* Integer */
#define RAD_LOGIN_LAT_NODE		35	/* Integer */
#define RAD_LOGIN_LAT_GROUP		36	/* Integer */
#define RAD_FRAMED_APPLETALK_LINK	37	/* Integer */
#define RAD_FRAMED_APPLETALK_NETWORK	38	/* Integer */
#define RAD_FRAMED_APPLETALK_ZONE	39	/* Integer */
     /* reserved for accounting		40-59 */
#define RAD_CHAP_CHALLENGE		60	/* String */
#define RAD_NAS_PORT_TYPE		61	/* Integer */
	#define RAD_ASYNC			0
	#define RAD_SYNC			1
	#define RAD_ISDN_SYNC			2
	#define RAD_ISDN_ASYNC_V120		3
	#define RAD_ISDN_ASYNC_V110		4
	#define RAD_VIRTUAL			5
#define RAD_PORT_LIMIT			62	/* Integer */
#define RAD_LOGIN_LAT_PORT		63	/* Integer */
#define RAD_CONNECT_INFO		77	/* String */

/* Accounting attribute types and values */
#define RAD_ACCT_STATUS_TYPE		40	/* Integer */
	#define RAD_START			1
	#define RAD_STOP			2
	#define RAD_ACCOUNTING_ON		7
	#define RAD_ACCOUNTING_OFF		8
#define RAD_ACCT_DELAY_TIME		41	/* Integer */
#define RAD_ACCT_INPUT_OCTETS		42	/* Integer */
#define RAD_ACCT_OUTPUT_OCTETS		43	/* Integer */
#define RAD_ACCT_SESSION_ID		44	/* String */
#define RAD_ACCT_AUTHENTIC		45	/* Integer */
	#define RAD_AUTH_RADIUS			1
	#define RAD_AUTH_LOCAL			2
	#define RAD_AUTH_REMOTE			3
#define RAD_ACCT_SESSION_TIME		46	/* Integer */
#define RAD_ACCT_INPUT_PACKETS		47	/* Integer */
#define RAD_ACCT_OUTPUT_PACKETS		48	/* Integer */
#define RAD_ACCT_TERMINATE_CAUSE	49	/* Integer */
        #define RAD_TERM_USER_REQUEST		1
        #define RAD_TERM_LOST_CARRIER		2
        #define RAD_TERM_LOST_SERVICE		3
        #define RAD_TERM_IDLE_TIMEOUT		4
        #define RAD_TERM_SESSION_TIMEOUT	5
        #define RAD_TERM_ADMIN_RESET		6
        #define RAD_TERM_ADMIN_REBOOT		7
        #define RAD_TERM_PORT_ERROR		8
        #define RAD_TERM_NAS_ERROR		9
        #define RAD_TERM_NAS_REQUEST		10
        #define RAD_TERM_NAS_REBOOT		11
        #define RAD_TERM_PORT_UNNEEDED		12
        #define RAD_TERM_PORT_PREEMPTED		13
        #define RAD_TERM_PORT_SUSPENDED		14
        #define RAD_TERM_SERVICE_UNAVAILABLE    15
        #define RAD_TERM_CALLBACK		16
        #define RAD_TERM_USER_ERROR		17
        #define RAD_TERM_HOST_REQUEST		18
#define	RAD_ACCT_MULTI_SESSION_ID	50	/* String */
#define	RAD_ACCT_LINK_COUNT		51	/* Integer */

struct rad_handle;
struct timeval;

__BEGIN_DECLS
struct rad_handle	*rad_acct_open(void);
int			 rad_add_server(struct rad_handle *,
			    const char *, int, const char *, int, int);
struct rad_handle	*rad_auth_open(void);
void			 rad_close(struct rad_handle *);
int			 rad_config(struct rad_handle *, const char *);
int			 rad_continue_send_request(struct rad_handle *, int,
			    int *, struct timeval *);
int			 rad_create_request(struct rad_handle *, int);
struct in_addr		 rad_cvt_addr(const void *);
u_int32_t		 rad_cvt_int(const void *);
char			*rad_cvt_string(const void *, size_t);
int			 rad_get_attr(struct rad_handle *, const void **,
			    size_t *);
int			 rad_init_send_request(struct rad_handle *, int *,
			    struct timeval *);
struct rad_handle	*rad_open(void);  /* Deprecated, == rad_auth_open */
int			 rad_put_addr(struct rad_handle *, int, struct in_addr);
int			 rad_put_attr(struct rad_handle *, int,
			    const void *, size_t);
int			 rad_put_int(struct rad_handle *, int, u_int32_t);
int			 rad_put_string(struct rad_handle *, int,
			    const char *);
int			 rad_send_request(struct rad_handle *);
const char		*rad_strerror(struct rad_handle *);
__END_DECLS

#endif /* _RADLIB_H_ */
EOF
FMD5=$(md5sum "radlib.h"|cut -d' ' -f1)
test "$MD5" = "$FMD5" && echo "restored: radlib.h (md5 ok)" || echo "ERROR: radlib.h (!md5)"
MD5=f98c5f450ad6dfb8cd9d86d8bbd6aef2 # radlib_private.h
cat <<'EOF' > "radlib_private.h"
/*-
 * Copyright 1998 Juniper Networks, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	$FreeBSD: src/lib/libradius/radlib_private.h,v 1.4 1999/10/31 04:47:59 jdp Exp $
 */

#ifndef RADLIB_PRIVATE_H
#define RADLIB_PRIVATE_H

#include <sys/types.h>
#include <netinet/in.h>

#include "radlib.h"

/* Handle types */
#define RADIUS_AUTH		0   /* RADIUS authentication, default */
#define RADIUS_ACCT		1   /* RADIUS accounting */

/* Defaults */
#define MAXTRIES		3
#define PATH_RADIUS_CONF	"/etc/radius.conf"
#define RADIUS_PORT		1812
#define RADACCT_PORT		1813
#define TIMEOUT			3	/* In seconds */

/* Limits */
#define ERRSIZE		128		/* Maximum error message length */
#define MAXCONFLINE	1024		/* Maximum config file line length */
#define MAXSERVERS	10		/* Maximum number of servers to try */
#define MSGSIZE		4096		/* Maximum RADIUS message */
#define PASSSIZE	128		/* Maximum significant password chars */

/* Positions of fields in RADIUS messages */
#define POS_CODE	0		/* Message code */
#define POS_IDENT	1		/* Identifier */
#define POS_LENGTH	2		/* Message length */
#define POS_AUTH	4		/* Authenticator */
#define LEN_AUTH	16		/* Length of authenticator */
#define POS_ATTRS	20		/* Start of attributes */

struct rad_server {
	struct sockaddr_in addr;	/* Address of server */
	char		*secret;	/* Shared secret */
	int		 timeout;	/* Timeout in seconds */
	int		 max_tries;	/* Number of tries before giving up */
	int		 num_tries;	/* Number of tries so far */
};

struct rad_handle {
	int		 fd;		/* Socket file descriptor */
	struct rad_server servers[MAXSERVERS];	/* Servers to contact */
	int		 num_servers;	/* Number of valid server entries */
	int		 ident;		/* Current identifier value */
	char		 errmsg[ERRSIZE];	/* Most recent error message */
	unsigned char	 request[MSGSIZE];	/* Request to send */
	int		 req_len;	/* Length of request */
	char		 pass[PASSSIZE];	/* Cleartext password */
	int		 pass_len;	/* Length of cleartext password */
	int		 pass_pos;	/* Position of scrambled password */
	char	 	 chap_pass;	/* Have we got a CHAP_PASSWORD ? */
	unsigned char	 response[MSGSIZE];	/* Response received */
	int		 resp_len;	/* Length of response */
	int		 resp_pos;	/* Current position scanning attrs */
	int		 total_tries;	/* How many requests we'll send */
	int		 try;		/* How many requests we've sent */
	int		 srv;		/* Server number we did last */
	int		 type;		/* Handle type */
};

#endif
EOF
FMD5=$(md5sum "radlib_private.h"|cut -d' ' -f1)
test "$MD5" = "$FMD5" && echo "restored: radlib_private.h (md5 ok)" || echo "ERROR: radlib_private.h (!md5)"
MD5=f8df6c1e97cc4471ddc72ec88fb85298 # README
cat <<'EOF' > "README"
This is a library to generate RADIUS authentication request.

Please read the copyrights in radlib.c :)

Some parts of the radlib.c I took from http://www.nlc.net.au/~john
who did the same for an older version of junipers libradius.

many tnx!
	Martin
(gremlin AT portal-to-web DOT de)



Installation:

FreeBSD
-------

This library is included in all FreeBSD 4.x and higher versions. So, if
you run one of this operating systems, you don't need this one!


NetBSD
------

It should compile now on NetBSD as well - tnx to Emmanuel Dreyfus ( manu AT netbsd DOT org )
for submitting the patch. 
libmd is not needed ( at least up from netbsd 1.6 - need to check older versions ), because
the MD5 functions are all in libc. comment out the section on 'netbsd' in the Makefile
to use the libc's MD5 routines 


Linux
-----

You will first need the MD5 library, libmd, installed. 
You can find it at http://www.portal-to-web.de/tacacs/download.php or
http://www.nlc.net.au/~john/software/libmd.tar.gz

The default location prefix for installation is /usr/local/
It also guesses, that libmd has the same install prefix. If you
want to install this lib somewhere else (like homedir...) modifiy
the PREFIX statement in the Makefile

Type the following to build...

	make 

Type the following to install..

	make install


EOF
FMD5=$(md5sum "README"|cut -d' ' -f1)
test "$MD5" = "$FMD5" && echo "restored: README (md5 ok)" || echo "ERROR: README (!md5)"
```

bash RESTORE libradius-linux-20040827
```
#!/bin/bash
DIRNAME=libradius-linux-20040827
mkdir "libradius-linux-20040827" && cd "libradius-linux-20040827" || exit 1

MD5=3529d884538b1007345ed8b80a220f77 # libradius.3
cat <<'EOF' > "libradius.3"
.\" Copyright 1998 Juniper Networks, Inc.
.\" All rights reserved.
.\"
.\" Redistribution and use in source and binary forms, with or without
.\" modification, are permitted provided that the following conditions
.\" are met:
.\" 1. Redistributions of source code must retain the above copyright
.\"    notice, this list of conditions and the following disclaimer.
.\" 2. Redistributions in binary form must reproduce the above copyright
.\"    notice, this list of conditions and the following disclaimer in the
.\"    documentation and/or other materials provided with the distribution.
.\"
.\" THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
.\" ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
.\" IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
.\" ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
.\" FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
.\" DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
.\" OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
.\" HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
.\" LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
.\" OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
.\" SUCH DAMAGE.
.\"
.\" $FreeBSD: src/lib/libradius/libradius.3,v 1.6.2.4 2001/12/17 10:08:31 ru Exp $
.\"
.Dd October 30, 1999
.Dt LIBRADIUS 3
.Os
.Sh NAME
.Nm libradius
.Nd RADIUS client library
.Sh SYNOPSIS
.In radlib.h
.Ft struct rad_handle *
.Fn rad_acct_open "void"
.Ft int
.Fn rad_add_server "struct rad_handle *h" "const char *host" "int port" "const char *secret" "int timeout" "int max_tries"
.Ft struct rad_handle *
.Fn rad_auth_open "void"
.Ft void
.Fn rad_close "struct rad_handle *h"
.Ft int
.Fn rad_config "struct rad_handle *h" "const char *file"
.Ft int
.Fn rad_continue_send_request "struct rad_handle *h" "int selected" "int *fd" "struct timeval *tv"
.Ft int
.Fn rad_create_request "struct rad_handle *h" "int code"
.Ft struct in_addr
.Fn rad_cvt_addr "const void *data"
.Ft u_int32_t
.Fn rad_cvt_int "const void *data"
.Ft char *
.Fn rad_cvt_string "const void *data" "size_t len"
.Ft int
.Fn rad_get_attr "struct rad_handle *h" "const void **data" "size_t *len"
.Ft int
.Fn rad_init_send_request "struct rad_handle *h" "int *fd" "struct timeval *tv"
.Ft int
.Fn rad_put_addr "struct rad_handle *h" "int type" "struct in_addr addr"
.Ft int
.Fn rad_put_attr "struct rad_handle *h" "int type" "const void *data" "size_t len"
.Ft int
.Fn rad_put_int "struct rad_handle *h" "int type" "u_int32_t value"
.Ft int
.Fn rad_put_string "struct rad_handle *h" "int type" "const char *str"
.Ft int
.Fn rad_send_request "struct rad_handle *h"
.Ft const char *
.Fn rad_strerror "struct rad_handle *h"
.Sh DESCRIPTION
The
.Nm
library implements the client side of the Remote Authentication Dial
In User Service (RADIUS).  RADIUS, defined in RFCs 2138 and 2139,
allows clients to perform authentication and accounting by means of
network requests to remote servers.
.Sh INITIALIZATION
To use the library, an application must first call
.Fn rad_auth_open
or
.Fn rad_acct_open
to obtain a
.Va struct rad_handle * ,
which provides the context for subsequent operations.
The former function is used for RADIUS authentication and the
latter is used for RADIUS accounting.
Calls to
.Fn rad_auth_open
and
.Fn rad_acct_open
always succeed unless insufficient virtual memory is available.  If
the necessary memory cannot be allocated, the functions return
.Dv NULL .
For compatibility with earlier versions of this library,
.Fn rad_open
is provided as a synonym for
.Fn rad_auth_open .
.Pp
Before issuing any RADIUS requests, the library must be made aware
of the servers it can contact.  The easiest way to configure the
library is to call
.Fn rad_config .
.Fn rad_config
causes the library to read a configuration file whose format is
described in
.Xr radius.conf 5 .
The pathname of the configuration file is passed as the
.Va file
argument to
.Fn rad_config .
This argument may also be given as
.Dv NULL ,
in which case the standard configuration file
.Pa /etc/radius.conf
is used.
.Fn rad_config
returns 0 on success, or -1 if an error occurs.
.Pp
The library can also be configured programmatically by calls to
.Fn rad_add_server .
The
.Va host
parameter specifies the server host, either as a fully qualified
domain name or as a dotted-quad IP address in text form.
The
.Va port
parameter specifies the UDP port to contact on the server.  If
.Va port
is given as 0, the library looks up the
.Ql radius/udp
or
.Ql radacct/udp
service in the network services database, and uses the port found
there.  If no entry is found, the library uses the standard RADIUS
ports, 1812 for authentication and 1813 for accounting.
The shared secret for the server host is passed to the
.Va secret
parameter.
It may be any NUL-terminated string of bytes.  The RADIUS protocol
ignores all but the leading 128 bytes of the shared secret.
The timeout for receiving replies from the server is passed to the
.Va timeout
parameter, in units of seconds.  The maximum number of repeated
requests to make before giving up is passed into the
.Va max_tries
parameter.
.Fn rad_add_server
returns 0 on success, or -1 if an error occurs.
.Pp
.Fn rad_add_server
may be called multiple times, and it may be used together with
.Fn rad_config .
At most 10 servers may be specified.
When multiple servers are given, they are tried in round-robin
fashion until a valid response is received, or until each server's
.Va max_tries
limit has been reached.
.Sh CREATING A RADIUS REQUEST
A RADIUS request consists of a code specifying the kind of request,
and zero or more attributes which provide additional information.  To
begin constructing a new request, call
.Fn rad_create_request .
In addition to the usual
.Va struct rad_handle * ,
this function takes a
.Va code
parameter which specifies the type of the request.  Most often this
will be
.Dv RAD_ACCESS_REQUEST .
.Fn rad_create_request
returns 0 on success, or -1 on if an error occurs.
.Pp
After the request has been created with
.Fn rad_create_request ,
attributes can be attached to it.  This is done through calls to
.Fn rad_put_addr ,
.Fn rad_put_int ,
and
.Fn rad_put_string .
Each accepts a
.Va type
parameter identifying the attribute, and a value which may be
an Internet address, an integer, or a NUL-terminated string,
respectively.
.Pp
The library also provides a function
.Fn rad_put_attr
which can be used to supply a raw, uninterpreted attribute.  The
.Va data
argument points to an array of bytes, and the
.Va len
argument specifies its length.
.Pp
The
.Fn rad_put_X
functions return 0 on success, or -1 if an error occurs.
.Sh SENDING THE REQUEST AND RECEIVING THE RESPONSE
After the RADIUS request has been constructed, it is sent either by means of
.Fn rad_send_request
or by a combination of calls to
.Fn rad_init_send_request
and
.Fn rad_continue_send_request .
.Pp
The
.Fn rad_send_request
function sends the request and waits for a valid reply,
retrying the defined servers in round-robin fashion as necessary.
If a valid response is received,
.Fn rad_send_request
returns the RADIUS code which specifies the type of the response.
This will typically be
.Dv RAD_ACCESS_ACCEPT ,
.Dv RAD_ACCESS_REJECT ,
or
.Dv RAD_ACCESS_CHALLENGE .
If no valid response is received,
.Fn rad_send_request
returns -1.
.Pp
As an alternative, if you do not wish to block waiting for a response,
.Fn rad_init_send_request
and
.Fn rad_continue_send_request
may be used instead.  If a reply is received from the RADIUS server or a
timeout occurs, these functions return a value as described for
.Fn rad_send_request .
Otherwise, a value of zero is returned and the values pointed to by
.Ar fd
and
.Ar tv
are set to the descriptor and timeout that should be passed to
.Xr select 2 .
.Pp
.Fn rad_init_send_request
must be called first, followed by repeated calls to
.Fn rad_continue_send_request
as long as a return value of zero is given.
Between each call, the application should call
.Xr select 2 ,
passing
.Ar *fd
as a read descriptor and timing out after the interval specified by
.Ar tv .
When select returns,
.Fn rad_continue_send_request
should be called with
.Ar selected
set to a non-zero value if
.Xr select 2
indicated that the descriptor is readable.
.Pp
Like RADIUS requests, each response may contain zero or more
attributes.  After a response has been received successfully by
.Fn rad_send_request
or
.Fn rad_continue_send_request ,
its attributes can be extracted one by one using
.Fn rad_get_attr .
Each time
.Fn rad_get_attr
is called, it gets the next attribute from the current response, and
stores a pointer to the data and the length of the data via the
reference parameters
.Va data
and
.Va len ,
respectively.  Note that the data resides in the response itself,
and must not be modified.
A successful call to
.Fn rad_get_attr
returns the RADIUS attribute type.
If no more attributes remain in the current response,
.Fn rad_get_attr
returns 0.
If an error such as a malformed attribute is detected, -1 is
returned.
.Pp
The common types of attributes can be decoded using
.Fn rad_cvt_addr ,
.Fn rad_cvt_int ,
and
.Fn rad_cvt_string .
These functions accept a pointer to the attribute data, which should
have been obtained using
.Fn rad_get_attr .
In the case of
.Fn rad_cvt_string ,
the length
.Va len
must also be given.  These functions interpret the attribute as an
Internet address, an integer, or a string, respectively, and return
its value.
.Fn rad_cvt_string
returns its value as a NUL-terminated string in dynamically
allocated memory.  The application should free the string using
.Xr free 3
when it is no longer needed.
.Pp
If insufficient virtual memory is available,
.Fn rad_cvt_string
returns
.Dv NULL .
.Fn rad_cvt_addr
and
.Fn rad_cvt_int
cannot fail.
.Sh OBTAINING ERROR MESSAGES
Those functions which accept a
.Va struct rad_handle *
argument record an error message if they fail.  The error message
can be retrieved by calling
.Fn rad_strerror .
The message text is overwritten on each new error for the given
.Va struct rad_handle * .
Thus the message must be copied if it is to be preserved through
subsequent library calls using the same handle.
.Sh CLEANUP
To free the resources used by the RADIUS library, call
.Fn rad_close .
.Sh RETURN VALUES
The following functions return a non-negative value on success.  If
they detect an error, they return -1 and record an error message
which can be retrieved using
.Fn rad_strerror .
.Pp
.Bl -item -offset indent -compact
.It
.Fn rad_add_server
.It
.Fn rad_config
.It
.Fn rad_create_request
.It
.Fn rad_get_attr
.It
.Fn rad_put_addr
.It
.Fn rad_put_attr
.It
.Fn rad_put_int
.It
.Fn rad_put_string
.It
.Fn rad_init_send_request
.It
.Fn rad_continue_send_request
.It
.Fn rad_send_request
.El
.Pp
The following functions return a
.No non- Ns Dv NULL
pointer on success.  If they are unable to allocate sufficient
virtual memory, they return
.Dv NULL ,
without recording an error message.
.Pp
.Bl -item -offset indent -compact
.It
.Fn rad_acct_open
.It
.Fn rad_auth_open
.It
.Fn rad_cvt_string
.El
.Sh FILES
.Pa /etc/radius.conf
.Sh SEE ALSO
.Xr radius.conf 5
.Rs
.%A C. Rigney, et al
.%T "Remote Authentication Dial In User Service (RADIUS)"
.%O RFC 2138
.Re
.Rs
.%A C. Rigney
.%T RADIUS Accounting
.%O RFC 2139
.Re
.Sh AUTHORS
This software was originally written by
.An John Polstra ,
and donated to the
.Fx
project by Juniper Networks, Inc.
Oleg Semyonov subsequently added the ability to perform RADIUS
accounting.
EOF
FMD5=$(md5sum "libradius.3"|cut -d' ' -f1)
test "$MD5" = "$FMD5" && echo "restored: libradius.3 (md5 ok)" || echo "ERROR: libradius.3 (!md5)"
MD5=ff94c2c30331cc004698d904f7c143b1 # Makefile
cat <<'EOF' > "Makefile"
# Copyright 1998 Juniper Networks, Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
#	$FreeBSD: src/lib/libradius/Makefile,v 1.1.1.1 1998/11/13 00:53:01 jdp Exp $

PREFIX=	/usr/local

RM=		rm -f
LN=		ln
CC=		cc
CFLAGS= -O2

# linux -L$(PREFIX)/lib -lmd
LIBMD= -L$(PREFIX)/lib -lmd

# netbsd - it's in libc
#LIBMD= -L/usr/lib -lc


LIB=    radius
OBJS=   radlib.o

all:    lib$(LIB).a


lib$(LIB).a:    $(OBJS)
	ar cru $@ $(OBJS)
	ranlib $@

radlib.o:       radlib.c
	$(CC) $(CFLAGS) -c radlib.c -o radlib_r.o
	ld -r -o radlib.o radlib_r.o -L$(PREFIX)/lib $(LIBMD)
	$(CC) -shared radlib_r.o -o libradius.so.1.0  -L$(PREFIX)/lib $(LIBMD)
	$(RM) radlib_r.o

clean:
	$(RM) *.o

clobber:	clean
	$(RM) lib$(LIB).a

install:	all
	install -c -m 644 -o bin -g bin lib$(LIB).a $(PREFIX)/lib
	install -c -m 644 -o bin -g bin lib$(LIB).so.1.0 $(PREFIX)/lib
	$(LN) -sf $(PREFIX)/lib/lib$(LIB).so.1.0 $(PREFIX)/lib/lib$(LIB).so
	install -c -m 444 -o bin -g bin radlib.h $(PREFIX)/include
	install -c -m 444 -o bin -g bin libradius.3 $(PREFIX)/man/man3
	install -c -m 444 -o bin -g bin radius.conf.5 $(PREFIX)/man/man5
EOF
FMD5=$(md5sum "Makefile"|cut -d' ' -f1)
test "$MD5" = "$FMD5" && echo "restored: Makefile (md5 ok)" || echo "ERROR: Makefile (!md5)"
MD5=b9fa9bc32c149c7d3ffe96a7c2dcde6c # md5.h
cat <<'EOF' > "md5.h"
/* MD5.H - header file for MD5C.C
 * $Id: md5.h,v 1.6.2.1 1998/02/18 02:28:14 jkh Exp $
 */

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
 */

#ifndef _MD5_H_
#define _MD5_H_
/* MD5 context. */
typedef struct MD5Context {
  u_int32_t state[4];	/* state (ABCD) */
  u_int32_t count[2];	/* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];	/* input buffer */
} MD5_CTX;

#include <sys/cdefs.h>

__BEGIN_DECLS
void   MD5Init (MD5_CTX *);
void   MD5Update (MD5_CTX *, const unsigned char *, unsigned int);
void   MD5Final (unsigned char [16], MD5_CTX *);
char * MD5End(MD5_CTX *, char *);
char * MD5File(const char *, char *);
char * MD5Data(const unsigned char *, unsigned int, char *);
__END_DECLS

#endif /* _MD5_H_ */
EOF
FMD5=$(md5sum "md5.h"|cut -d' ' -f1)
test "$MD5" = "$FMD5" && echo "restored: md5.h (md5 ok)" || echo "ERROR: md5.h (!md5)"
MD5=cd2f13f4ceddd295b090194f562cf311 # radius.conf.5
cat <<'EOF' > "radius.conf.5"
.\" Copyright 1998 Juniper Networks, Inc.
.\" All rights reserved.
.\"
.\" Redistribution and use in source and binary forms, with or without
.\" modification, are permitted provided that the following conditions
.\" are met:
.\" 1. Redistributions of source code must retain the above copyright
.\"    notice, this list of conditions and the following disclaimer.
.\" 2. Redistributions in binary form must reproduce the above copyright
.\"    notice, this list of conditions and the following disclaimer in the
.\"    documentation and/or other materials provided with the distribution.
.\"
.\" THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
.\" ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
.\" IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
.\" ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
.\" FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
.\" DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
.\" OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
.\" HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
.\" LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
.\" OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
.\" SUCH DAMAGE.
.\"
.\" $FreeBSD: src/lib/libradius/radius.conf.5,v 1.4.2.3 2001/08/17 15:43:06 ru Exp $
.\"
.Dd October 30, 1999
.Dt RADIUS.CONF 5
.Os
.Sh NAME
.Nm radius.conf
.Nd RADIUS client configuration file
.Sh SYNOPSIS
.Pa /etc/radius.conf
.Sh DESCRIPTION
.Nm
contains the information necessary to configure the RADIUS client
library.  It is parsed by
.Xr rad_config 3 .
The file contains one or more lines of text, each describing a
single RADIUS server which will be used by the library.  Leading
white space is ignored, as are empty lines and lines containing
only comments.
.Pp
A RADIUS server is described by three to five fields on a line:
.Pp
.Bl -item -offset indent -compact
.It
Service type
.It
Server host
.It
Shared secret
.It
Timeout
.It
Retries
.El
.Pp
The fields are separated by white space.  The
.Ql #
character at the beginning of a field begins a comment, which extends
to the end of the line.  A field may be enclosed in double quotes,
in which case it may contain white space and/or begin with the
.Ql #
character.  Within a quoted string, the double quote character can
be represented by
.Ql \e\&" ,
and the backslash can be represented by
.Ql \e\e .
No other escape sequences are supported.
.Pp
.Pp
The first field gives the service type, either
.Ql auth
for RADIUS authentication or
.Ql acct
for RADIUS accounting.  If a single server provides both services, two
lines are required in the file.  Earlier versions of this file did
not include a service type.  For backward compatibility, if the first
field is not
.Ql auth
or
.Ql acct
the library behaves as if
.Ql auth
were specified, and interprets the fields in the line as if they
were fields two through five.
.Pp
The second field specifies
the server host, either as a fully qualified domain name or as a
dotted-quad IP address.  The host may optionally be followed by a
.Ql \&:
and a numeric port number, without intervening white space.  If the
port specification is omitted, it defaults to the
.Ql radius
or
.Ql radacct
service in the
.Pa /etc/services
file for service types
.Ql auth
and
.Ql acct ,
respectively.
If no such entry is present, the standard ports 1812 and 1813 are
used.
.Pp
The third field contains the shared secret, which should be known
only to the client and server hosts.  It is an arbitrary string of
characters, though it must be enclosed in double quotes if it
contains white space.  The shared secret may be
any length, but the RADIUS protocol uses only the first 128
characters.  N.B., some popular RADIUS servers have bugs which
prevent them from working properly with secrets longer than 16
characters.
.Pp
The fourth field contains a decimal integer specifying the timeout in
seconds for receiving a valid reply from the server.  If this field
is omitted, it defaults to 3 seconds.
.Pp
The fifth field contains a decimal integer specifying the maximum
number of attempts that will be made to authenticate with the server
before giving up.  If omitted, it defaults to 3 attempts.  Note,
this is the total number of attempts and not the number of retries.
.Pp
Up to 10 RADIUS servers may be specified for each service type.
The servers are tried in
round-robin fashion, until a valid response is received or the
maximum number of tries has been reached for all servers.
.Pp
The standard location for this file is
.Pa /etc/radius.conf .
But an alternate pathname may be specified in the call to
.Xr rad_config 3 .
Since the file contains sensitive information in the form of the
shared secrets, it should not be readable except by root.
.Sh FILES
.Pa /etc/radius.conf
.Sh EXAMPLES
.Bd -literal
# A simple entry using all the defaults:
acct  radius1.domain.com  OurLittleSecret

# A server still using the obsolete RADIUS port, with increased
# timeout and maximum tries:
auth  auth.domain.com:1645  "I can't see you"  5  4

# A server specified by its IP address:
auth  192.168.27.81  $X*#..38947ax-+=
.Ed
.Sh SEE ALSO
.Xr libradius 3
.Rs
.%A C. Rigney, et al
.%T "Remote Authentication Dial In User Service (RADIUS)"
.%O RFC 2138
.Re
.Rs
.%A C. Rigney
.%T RADIUS Accounting
.%O RFC 2139
.Re
.Sh AUTHORS
This documentation was written by
.An John Polstra ,
and donated to the
.Fx
project by Juniper Networks, Inc.
EOF
FMD5=$(md5sum "radius.conf.5"|cut -d' ' -f1)
test "$MD5" = "$FMD5" && echo "restored: radius.conf.5 (md5 ok)" || echo "ERROR: radius.conf.5 (!md5)"
MD5=44816c51b83d4ae516065ff4e2f7c7c6 # radlib.c
cat <<'EOF' > "radlib.c"
/*-
 * Copyright 1998 Juniper Networks, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	$FreeBSD: src/lib/libradius/radlib.c,v 1.4.2.2 2000/11/09 00:24:26 eivind Exp $
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>
#include "md5.h"
#include <netdb.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "radlib_private.h"
#if defined(__linux__)
#define MSG_WAITALL     0
#endif
#if !defined(__printflike)
#define __printflike(fmtarg, firstvararg)                               \
        __attribute__((__format__ (__printf__, fmtarg, firstvararg)))
#endif
#if !defined(timersub)
#define timersub(tvp, uvp, vvp)                                         \
        do {                                                            \
                (vvp)->tv_sec = (tvp)->tv_sec - (uvp)->tv_sec;          \
                (vvp)->tv_usec = (tvp)->tv_usec - (uvp)->tv_usec;       \
                if ((vvp)->tv_usec < 0) {                               \
                        (vvp)->tv_sec--;                                \
                        (vvp)->tv_usec += 1000000;                      \
                }                                                       \
        } while (0)
#endif



static void	 clear_password(struct rad_handle *);
static void	 generr(struct rad_handle *, const char *, ...)
		    __printflike(2, 3);
static void	 insert_scrambled_password(struct rad_handle *, int);
static void	 insert_request_authenticator(struct rad_handle *, int);
static int	 is_valid_response(struct rad_handle *, int,
		    const struct sockaddr_in *);
static int	 put_password_attr(struct rad_handle *, int,
		    const void *, size_t);
static int	 put_raw_attr(struct rad_handle *, int,
		    const void *, size_t);
static int	 split(char *, char *[], int, char *, size_t);

static void
clear_password(struct rad_handle *h)
{
	if (h->pass_len != 0) {
		memset(h->pass, 0, h->pass_len);
		h->pass_len = 0;
	}
	h->pass_pos = 0;
}

static void
generr(struct rad_handle *h, const char *format, ...)
{
	va_list		 ap;

	va_start(ap, format);
	vsnprintf(h->errmsg, ERRSIZE, format, ap);
	va_end(ap);
}

static void
insert_scrambled_password(struct rad_handle *h, int srv)
{
	MD5_CTX ctx;
	unsigned char md5[16];
	const struct rad_server *srvp;
	int padded_len;
	int pos;

	srvp = &h->servers[srv];
	padded_len = h->pass_len == 0 ? 16 : (h->pass_len+15) & ~0xf;

	memcpy(md5, &h->request[POS_AUTH], LEN_AUTH);
	for (pos = 0;  pos < padded_len;  pos += 16) {
		int i;

		/* Calculate the new scrambler */
		MD5Init(&ctx);
		MD5Update(&ctx, srvp->secret, strlen(srvp->secret));
		MD5Update(&ctx, md5, 16);
		MD5Final(md5, &ctx);

		/*
		 * Mix in the current chunk of the password, and copy
		 * the result into the right place in the request.  Also
		 * modify the scrambler in place, since we will use this
		 * in calculating the scrambler for next time.
		 */
		for (i = 0;  i < 16;  i++)
			h->request[h->pass_pos + pos + i] =
			    md5[i] ^= h->pass[pos + i];
	}
}

static void
insert_request_authenticator(struct rad_handle *h, int srv)
{
	MD5_CTX ctx;
	const struct rad_server *srvp;

	srvp = &h->servers[srv];

	/* Create the request authenticator */
	MD5Init(&ctx);
	MD5Update(&ctx, &h->request[POS_CODE], POS_AUTH - POS_CODE);
	MD5Update(&ctx, memset(&h->request[POS_AUTH], 0, LEN_AUTH), LEN_AUTH);
	MD5Update(&ctx, &h->request[POS_ATTRS], h->req_len - POS_ATTRS);
	MD5Update(&ctx, srvp->secret, strlen(srvp->secret));
	MD5Final(&h->request[POS_AUTH], &ctx);
}

/*
 * Return true if the current response is valid for a request to the
 * specified server.
 */
static int
is_valid_response(struct rad_handle *h, int srv,
    const struct sockaddr_in *from)
{
	MD5_CTX ctx;
	unsigned char md5[16];
	const struct rad_server *srvp;
	int len;

	srvp = &h->servers[srv];

	/* Check the source address */
	if (from->sin_family != srvp->addr.sin_family ||
	    from->sin_addr.s_addr != srvp->addr.sin_addr.s_addr ||
	    from->sin_port != srvp->addr.sin_port)
		return 0;

	/* Check the message length */
	if (h->resp_len < POS_ATTRS)
		return 0;
	len = h->response[POS_LENGTH] << 8 | h->response[POS_LENGTH+1];
	if (len > h->resp_len)
		return 0;

	/* Check the response authenticator */
	MD5Init(&ctx);
	MD5Update(&ctx, &h->response[POS_CODE], POS_AUTH - POS_CODE);
	MD5Update(&ctx, &h->request[POS_AUTH], LEN_AUTH);
	MD5Update(&ctx, &h->response[POS_ATTRS], len - POS_ATTRS);
	MD5Update(&ctx, srvp->secret, strlen(srvp->secret));
	MD5Final(md5, &ctx);
	if (memcmp(&h->response[POS_AUTH], md5, sizeof md5) != 0)
		return 0;

	return 1;
}

static int
put_password_attr(struct rad_handle *h, int type, const void *value, size_t len)
{
	int padded_len;
	int pad_len;

	if (h->pass_pos != 0) {
		generr(h, "Multiple User-Password attributes specified");
		return -1;
	}
	if (len > PASSSIZE)
		len = PASSSIZE;
	padded_len = len == 0 ? 16 : (len+15) & ~0xf;
	pad_len = padded_len - len;

	/*
	 * Put in a place-holder attribute containing all zeros, and
	 * remember where it is so we can fill it in later.
	 */
	clear_password(h);
	put_raw_attr(h, type, h->pass, padded_len);
	h->pass_pos = h->req_len - padded_len;

	/* Save the cleartext password, padded as necessary */
	memcpy(h->pass, value, len);
	h->pass_len = len;
	memset(h->pass + len, 0, pad_len);
	return 0;
}

static int
put_raw_attr(struct rad_handle *h, int type, const void *value, size_t len)
{
	if (len > 253) {
		generr(h, "Attribute too long");
		return -1;
	}
	if (h->req_len + 2 + len > MSGSIZE) {
		generr(h, "Maximum message length exceeded");
		return -1;
	}
	h->request[h->req_len++] = type;
	h->request[h->req_len++] = len + 2;
	memcpy(&h->request[h->req_len], value, len);
	h->req_len += len;
	return 0;
}

int
rad_add_server(struct rad_handle *h, const char *host, int port,
    const char *secret, int timeout, int tries)
{
	struct rad_server *srvp;

	if (h->num_servers >= MAXSERVERS) {
		generr(h, "Too many RADIUS servers specified");
		return -1;
	}
	srvp = &h->servers[h->num_servers];

	memset(&srvp->addr, 0, sizeof srvp->addr);
#if !defined(__linux__)
        srvp->addr.sin_len = sizeof srvp->addr;
#endif
	srvp->addr.sin_family = AF_INET;
	if (!inet_aton(host, &srvp->addr.sin_addr)) {
		struct hostent *hent;

		if ((hent = gethostbyname(host)) == NULL) {
			generr(h, "%s: host not found", host);
			return -1;
		}
		memcpy(&srvp->addr.sin_addr, hent->h_addr,
		    sizeof srvp->addr.sin_addr);
	}
	if (port != 0)
		srvp->addr.sin_port = htons(port);
	else {
		struct servent *sent;

		if (h->type == RADIUS_AUTH)
			srvp->addr.sin_port =
			    (sent = getservbyname("radius", "udp")) != NULL ?
				sent->s_port : htons(RADIUS_PORT);
		else
			srvp->addr.sin_port =
			    (sent = getservbyname("radacct", "udp")) != NULL ?
				sent->s_port : htons(RADACCT_PORT);
	}
	if ((srvp->secret = strdup(secret)) == NULL) {
		generr(h, "Out of memory");
		return -1;
	}
	srvp->timeout = timeout;
	srvp->max_tries = tries;
	srvp->num_tries = 0;
	h->num_servers++;
	return 0;
}

void
rad_close(struct rad_handle *h)
{
	int srv;

	if (h->fd != -1)
		close(h->fd);
	for (srv = 0;  srv < h->num_servers;  srv++) {
		memset(h->servers[srv].secret, 0,
		    strlen(h->servers[srv].secret));
		free(h->servers[srv].secret);
	}
	clear_password(h);
	free(h);
}

int
rad_config(struct rad_handle *h, const char *path)
{
	FILE *fp;
	char buf[MAXCONFLINE];
	int linenum;
	int retval;

	if (path == NULL)
		path = PATH_RADIUS_CONF;
	if ((fp = fopen(path, "r")) == NULL) {
		generr(h, "Cannot open \"%s\": %s", path, strerror(errno));
		return -1;
	}
	retval = 0;
	linenum = 0;
	while (fgets(buf, sizeof buf, fp) != NULL) {
		int len;
		char *fields[5];
		int nfields;
		char msg[ERRSIZE];
		char *type;
		char *host, *res;
		char *port_str;
		char *secret;
		char *timeout_str;
		char *maxtries_str;
		char *end;
		char *wanttype;
		unsigned long timeout;
		unsigned long maxtries;
		int port;
		int i;

		linenum++;
		len = strlen(buf);
		/* We know len > 0, else fgets would have returned NULL. */
		if (buf[len - 1] != '\n') {
			if (len == sizeof buf - 1)
				generr(h, "%s:%d: line too long", path,
				    linenum);
			else
				generr(h, "%s:%d: missing newline", path,
				    linenum);
			retval = -1;
			break;
		}
		buf[len - 1] = '\0';

		/* Extract the fields from the line. */
		nfields = split(buf, fields, 5, msg, sizeof msg);
		if (nfields == -1) {
			generr(h, "%s:%d: %s", path, linenum, msg);
			retval = -1;
			break;
		}
		if (nfields == 0)
			continue;
		/*
		 * The first field should contain "auth" or "acct" for
		 * authentication or accounting, respectively.  But older
		 * versions of the file didn't have that field.  Default
		 * it to "auth" for backward compatibility.
		 */
		if (strcmp(fields[0], "auth") != 0 &&
		    strcmp(fields[0], "acct") != 0) {
			if (nfields >= 5) {
				generr(h, "%s:%d: invalid service type", path,
				    linenum);
				retval = -1;
				break;
			}
			nfields++;
			for (i = nfields;  --i > 0;  )
				fields[i] = fields[i - 1];
			fields[0] = "auth";
		}
		if (nfields < 3) {
			generr(h, "%s:%d: missing shared secret", path,
			    linenum);
			retval = -1;
			break;
		}
		type = fields[0];
		host = fields[1];
		secret = fields[2];
		timeout_str = fields[3];
		maxtries_str = fields[4];

		/* Ignore the line if it is for the wrong service type. */
		wanttype = h->type == RADIUS_AUTH ? "auth" : "acct";
		if (strcmp(type, wanttype) != 0)
			continue;

		/* Parse and validate the fields. */
		res = host;
		host = strsep(&res, ":");
		port_str = strsep(&res, ":");
		if (port_str != NULL) {
			port = strtoul(port_str, &end, 10);
			if (*end != '\0') {
				generr(h, "%s:%d: invalid port", path,
				    linenum);
				retval = -1;
				break;
			}
		} else
			port = 0;
		if (timeout_str != NULL) {
			timeout = strtoul(timeout_str, &end, 10);
			if (*end != '\0') {
				generr(h, "%s:%d: invalid timeout", path,
				    linenum);
				retval = -1;
				break;
			}
		} else
			timeout = TIMEOUT;
		if (maxtries_str != NULL) {
			maxtries = strtoul(maxtries_str, &end, 10);
			if (*end != '\0') {
				generr(h, "%s:%d: invalid maxtries", path,
				    linenum);
				retval = -1;
				break;
			}
		} else
			maxtries = MAXTRIES;

		if (rad_add_server(h, host, port, secret, timeout, maxtries) ==
		    -1) {
			strcpy(msg, h->errmsg);
			generr(h, "%s:%d: %s", path, linenum, msg);
			retval = -1;
			break;
		}
	}
	/* Clear out the buffer to wipe a possible copy of a shared secret */
	memset(buf, 0, sizeof buf);
	fclose(fp);
	return retval;
}

/*
 * rad_init_send_request() must have previously been called.
 * Returns:
 *   0     The application should select on *fd with a timeout of tv before
 *         calling rad_continue_send_request again.
 *   < 0   Failure
 *   > 0   Success
 */
int
rad_continue_send_request(struct rad_handle *h, int selected, int *fd,
                          struct timeval *tv)
{
	int n;

	if (selected) {
		struct sockaddr_in from;
		int fromlen;

		fromlen = sizeof from;
		h->resp_len = recvfrom(h->fd, h->response,
		    MSGSIZE, MSG_WAITALL, (struct sockaddr *)&from, &fromlen);
		if (h->resp_len == -1) {
			generr(h, "recvfrom: %s", strerror(errno));
			return -1;
		}
		if (is_valid_response(h, h->srv, &from)) {
			h->resp_len = h->response[POS_LENGTH] << 8 |
			    h->response[POS_LENGTH+1];
			h->resp_pos = POS_ATTRS;
			return h->response[POS_CODE];
		}
	}

	if (h->try == h->total_tries) {
		generr(h, "No valid RADIUS responses received");
		return -1;
	}

	/*
         * Scan round-robin to the next server that has some
         * tries left.  There is guaranteed to be one, or we
         * would have exited this loop by now.
	 */
	while (h->servers[h->srv].num_tries >= h->servers[h->srv].max_tries)
		if (++h->srv >= h->num_servers)
			h->srv = 0;

	if (h->request[POS_CODE] == RAD_ACCOUNTING_REQUEST)
		/* Insert the request authenticator into the request */
		insert_request_authenticator(h, h->srv);
	else
		/* Insert the scrambled password into the request */
		if (h->pass_pos != 0)
			insert_scrambled_password(h, h->srv);

	/* Send the request */
	n = sendto(h->fd, h->request, h->req_len, 0,
	    (const struct sockaddr *)&h->servers[h->srv].addr,
	    sizeof h->servers[h->srv].addr);
	if (n != h->req_len) {
		if (n == -1)
			generr(h, "sendto: %s", strerror(errno));
		else
			generr(h, "sendto: short write");
		return -1;
	}

	h->try++;
	h->servers[h->srv].num_tries++;
	tv->tv_sec = h->servers[h->srv].timeout;
	tv->tv_usec = 0;
	*fd = h->fd;

	return 0;
}

int
rad_create_request(struct rad_handle *h, int code)
{
	int i;

	h->request[POS_CODE] = code;
	h->request[POS_IDENT] = ++h->ident;
	/* Create a random authenticator */
	for (i = 0;  i < LEN_AUTH;  i += 2) {
		long r;
		r = random();
		h->request[POS_AUTH+i] = r;
		h->request[POS_AUTH+i+1] = r >> 8;
	}
	h->req_len = POS_ATTRS;
	clear_password(h);
	return 0;
}

struct in_addr
rad_cvt_addr(const void *data)
{
	struct in_addr value;

	memcpy(&value.s_addr, data, sizeof value.s_addr);
	return value;
}

u_int32_t
rad_cvt_int(const void *data)
{
	u_int32_t value;

	memcpy(&value, data, sizeof value);
	return ntohl(value);
}

char *
rad_cvt_string(const void *data, size_t len)
{
	char *s;

	s = malloc(len + 1);
	if (s != NULL) {
		memcpy(s, data, len);
		s[len] = '\0';
	}
	return s;
}

/*
 * Returns the attribute type.  If none are left, returns 0.  On failure,
 * returns -1.
 */
int
rad_get_attr(struct rad_handle *h, const void **value, size_t *len)
{
	int type;

	if (h->resp_pos >= h->resp_len)
		return 0;
	if (h->resp_pos + 2 > h->resp_len) {
		generr(h, "Malformed attribute in response");
		return -1;
	}
	type = h->response[h->resp_pos++];
	*len = h->response[h->resp_pos++] - 2;
	if (h->resp_pos + *len > h->resp_len) {
		generr(h, "Malformed attribute in response");
		return -1;
	}
	*value = &h->response[h->resp_pos];
	h->resp_pos += *len;
	return type;
}

/*
 * Returns -1 on error, 0 to indicate no event and >0 for success
 */
int
rad_init_send_request(struct rad_handle *h, int *fd, struct timeval *tv)
{
	int srv;

	/* Make sure we have a socket to use */
	if (h->fd == -1) {
		struct sockaddr_in sin;

		if ((h->fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
			generr(h, "Cannot create socket: %s", strerror(errno));
			return -1;
		}
		memset(&sin, 0, sizeof sin);
#if !defined(__linux__)
		sin.sin_len = sizeof sin;
#endif
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = INADDR_ANY;
		sin.sin_port = htons(0);
		if (bind(h->fd, (const struct sockaddr *)&sin,
		    sizeof sin) == -1) {
			generr(h, "bind: %s", strerror(errno));
			close(h->fd);
			h->fd = -1;
			return -1;
		}
	}

	if (h->request[POS_CODE] == RAD_ACCOUNTING_REQUEST) {
		/* Make sure no password given */
		if (h->pass_pos || h->chap_pass) {
			generr(h, "User or Chap Password in accounting request");
			return -1;
		}
	} else {
		/* Make sure the user gave us a password */
		if (h->pass_pos == 0 && !h->chap_pass) {
			generr(h, "No User or Chap Password attributes given");
			return -1;
		}
		if (h->pass_pos != 0 && h->chap_pass) {
			generr(h, "Both User and Chap Password attributes given");
			return -1;
		}
	}

	/* Fill in the length field in the message */
	h->request[POS_LENGTH] = h->req_len >> 8;
	h->request[POS_LENGTH+1] = h->req_len;

	/*
	 * Count the total number of tries we will make, and zero the
	 * counter for each server.
	 */
	h->total_tries = 0;
	for (srv = 0;  srv < h->num_servers;  srv++) {
		h->total_tries += h->servers[srv].max_tries;
		h->servers[srv].num_tries = 0;
	}
	if (h->total_tries == 0) {
		generr(h, "No RADIUS servers specified");
		return -1;
	}

	h->try = h->srv = 0;

	return rad_continue_send_request(h, 0, fd, tv);
}

/*
 * Create and initialize a rad_handle structure, and return it to the
 * caller.  Can fail only if the necessary memory cannot be allocated.
 * In that case, it returns NULL.
 */
struct rad_handle *
rad_auth_open(void)
{
	struct rad_handle *h;

	h = (struct rad_handle *)malloc(sizeof(struct rad_handle));
	if (h != NULL) {
#if defined(__linux__) || defined(__NetBSD__)
                        srandom((unsigned long)time(NULL));
#else
                        srandomdev();
#endif
		h->fd = -1;
		h->num_servers = 0;
		h->ident = random();
		h->errmsg[0] = '\0';
		memset(h->pass, 0, sizeof h->pass);
		h->pass_len = 0;
		h->pass_pos = 0;
		h->chap_pass = 0;
		h->type = RADIUS_AUTH;
	}
	return h;
}

struct rad_handle *
rad_acct_open(void)
{
	struct rad_handle *h;

	h = rad_open();
	if (h != NULL)
	        h->type = RADIUS_ACCT;
	return h;
}

struct rad_handle *
rad_open(void)
{
    return rad_auth_open();
}

int
rad_put_addr(struct rad_handle *h, int type, struct in_addr addr)
{
	return rad_put_attr(h, type, &addr.s_addr, sizeof addr.s_addr);
}

int
rad_put_attr(struct rad_handle *h, int type, const void *value, size_t len)
{
	int result;

	if (type == RAD_USER_PASSWORD)
		result = put_password_attr(h, type, value, len);
	else {
		result = put_raw_attr(h, type, value, len);
		if (result == 0 && type == RAD_CHAP_PASSWORD)
			h->chap_pass = 1;
	}

	return result;
}

int
rad_put_int(struct rad_handle *h, int type, u_int32_t value)
{
	u_int32_t nvalue;

	nvalue = htonl(value);
	return rad_put_attr(h, type, &nvalue, sizeof nvalue);
}

int
rad_put_string(struct rad_handle *h, int type, const char *str)
{
	return rad_put_attr(h, type, str, strlen(str));
}

/*
 * Returns the response type code on success, or -1 on failure.
 */
int
rad_send_request(struct rad_handle *h)
{
	struct timeval timelimit;
	struct timeval tv;
	int fd;
	int n;

	n = rad_init_send_request(h, &fd, &tv);

	if (n != 0)
		return n;

	gettimeofday(&timelimit, NULL);
	timeradd(&tv, &timelimit, &timelimit);

	for ( ; ; ) {
		fd_set readfds;

		FD_ZERO(&readfds);
		FD_SET(fd, &readfds);

		n = select(fd + 1, &readfds, NULL, NULL, &tv);

		if (n == -1) {
			generr(h, "select: %s", strerror(errno));
			return -1;
		}

		if (!FD_ISSET(fd, &readfds)) {
			/* Compute a new timeout */
			gettimeofday(&tv, NULL);
			timersub(&timelimit, &tv, &tv);
			if (tv.tv_sec > 0 || (tv.tv_sec == 0 && tv.tv_usec > 0))
				/* Continue the select */
				continue;
		}

		n = rad_continue_send_request(h, n, &fd, &tv);

		if (n != 0)
			return n;

		gettimeofday(&timelimit, NULL);
		timeradd(&tv, &timelimit, &timelimit);
	}
}

const char *
rad_strerror(struct rad_handle *h)
{
	return h->errmsg;
}

/*
 * Destructively split a string into fields separated by white space.
 * `#' at the beginning of a field begins a comment that extends to the
 * end of the string.  Fields may be quoted with `"'.  Inside quoted
 * strings, the backslash escapes `\"' and `\\' are honored.
 *
 * Pointers to up to the first maxfields fields are stored in the fields
 * array.  Missing fields get NULL pointers.
 *
 * The return value is the actual number of fields parsed, and is always
 * <= maxfields.
 *
 * On a syntax error, places a message in the msg string, and returns -1.
 */
static int
split(char *str, char *fields[], int maxfields, char *msg, size_t msglen)
{
	char *p;
	int i;
	static const char ws[] = " \t";

	for (i = 0;  i < maxfields;  i++)
		fields[i] = NULL;
	p = str;
	i = 0;
	while (*p != '\0') {
		p += strspn(p, ws);
		if (*p == '#' || *p == '\0')
			break;
		if (i >= maxfields) {
			snprintf(msg, msglen, "line has too many fields");
			return -1;
		}
		if (*p == '"') {
			char *dst;

			dst = ++p;
			fields[i] = dst;
			while (*p != '"') {
				if (*p == '\\') {
					p++;
					if (*p != '"' && *p != '\\' &&
					    *p != '\0') {
						snprintf(msg, msglen,
						    "invalid `\\' escape");
						return -1;
					}
				}
				if (*p == '\0') {
					snprintf(msg, msglen,
					    "unterminated quoted string");
					return -1;
				}
				*dst++ = *p++;
			}
			*dst = '\0';
			p++;
			if (*fields[i] == '\0') {
				snprintf(msg, msglen,
				    "empty quoted string not permitted");
				return -1;
			}
			if (*p != '\0' && strspn(p, ws) == 0) {
				snprintf(msg, msglen, "quoted string not"
				    " followed by white space");
				return -1;
			}
		} else {
			fields[i] = p;
			p += strcspn(p, ws);
			if (*p != '\0')
				*p++ = '\0';
		}
		i++;
	}
	return i;
}
EOF
FMD5=$(md5sum "radlib.c"|cut -d' ' -f1)
test "$MD5" = "$FMD5" && echo "restored: radlib.c (md5 ok)" || echo "ERROR: radlib.c (!md5)"
MD5=8d590ceac33a370d5d9809cb674bec29 # radlib.h
cat <<'EOF' > "radlib.h"
/*-
 * Copyright 1998 Juniper Networks, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	$FreeBSD: src/lib/libradius/radlib.h,v 1.3 1999/10/31 04:47:59 jdp Exp $
 */

#ifndef _RADLIB_H_
#define _RADLIB_H_

#include <sys/types.h>
#include <netinet/in.h>

/* Message types */
#define RAD_ACCESS_REQUEST		1
#define RAD_ACCESS_ACCEPT		2
#define RAD_ACCESS_REJECT		3
#define RAD_ACCOUNTING_REQUEST		4
#define RAD_ACCOUNTING_RESPONSE		5
#define RAD_ACCESS_CHALLENGE		11

/* Attribute types and values */
#define RAD_USER_NAME			1	/* String */
#define RAD_USER_PASSWORD		2	/* String */
#define RAD_CHAP_PASSWORD		3	/* String */
#define RAD_NAS_IP_ADDRESS		4	/* IP address */
#define RAD_NAS_PORT			5	/* Integer */
#define RAD_SERVICE_TYPE		6	/* Integer */
	#define RAD_LOGIN			1
	#define RAD_FRAMED			2
	#define RAD_CALLBACK_LOGIN		3
	#define RAD_CALLBACK_FRAMED		4
	#define RAD_OUTBOUND			5
	#define RAD_ADMINISTRATIVE		6
	#define RAD_NAS_PROMPT			7
	#define RAD_AUTHENTICATE_ONLY		8
	#define RAD_CALLBACK_NAS_PROMPT		9
#define RAD_FRAMED_PROTOCOL		7	/* Integer */
	#define RAD_PPP				1
	#define RAD_SLIP			2
	#define RAD_ARAP			3	/* Appletalk */
	#define RAD_GANDALF			4
	#define RAD_XYLOGICS			5
#define RAD_FRAMED_IP_ADDRESS		8	/* IP address */
#define RAD_FRAMED_IP_NETMASK		9	/* IP address */
#define RAD_FRAMED_ROUTING		10	/* Integer */
#define RAD_FILTER_ID			11	/* String */
#define RAD_FRAMED_MTU			12	/* Integer */
#define RAD_FRAMED_COMPRESSION		13	/* Integer */
	#define RAD_COMP_NONE			0
	#define RAD_COMP_VJ			1
	#define RAD_COMP_IPXHDR			2
#define RAD_LOGIN_IP_HOST		14	/* IP address */
#define RAD_LOGIN_SERVICE		15	/* Integer */
#define RAD_LOGIN_TCP_PORT		16	/* Integer */
     /* unassiged			17 */
#define RAD_REPLY_MESSAGE		18	/* String */
#define RAD_CALLBACK_NUMBER		19	/* String */
#define RAD_CALLBACK_ID			20	/* String */
     /* unassiged			21 */
#define RAD_FRAMED_ROUTE		22	/* String */
#define RAD_FRAMED_IPX_NETWORK		23	/* IP address */
#define RAD_STATE			24	/* String */
#define RAD_CLASS			25	/* Integer */
#define RAD_VENDOR_SPECIFIC		26	/* Integer */
#define RAD_SESSION_TIMEOUT		27	/* Integer */
#define RAD_IDLE_TIMEOUT		28	/* Integer */
#define RAD_TERMINATION_ACTION		29	/* Integer */
#define RAD_CALLED_STATION_ID		30	/* String */
#define RAD_CALLING_STATION_ID		31	/* String */
#define RAD_NAS_IDENTIFIER		32	/* Integer */
#define RAD_PROXY_STATE			33	/* Integer */
#define RAD_LOGIN_LAT_SERVICE		34	/* Integer */
#define RAD_LOGIN_LAT_NODE		35	/* Integer */
#define RAD_LOGIN_LAT_GROUP		36	/* Integer */
#define RAD_FRAMED_APPLETALK_LINK	37	/* Integer */
#define RAD_FRAMED_APPLETALK_NETWORK	38	/* Integer */
#define RAD_FRAMED_APPLETALK_ZONE	39	/* Integer */
     /* reserved for accounting		40-59 */
#define RAD_CHAP_CHALLENGE		60	/* String */
#define RAD_NAS_PORT_TYPE		61	/* Integer */
	#define RAD_ASYNC			0
	#define RAD_SYNC			1
	#define RAD_ISDN_SYNC			2
	#define RAD_ISDN_ASYNC_V120		3
	#define RAD_ISDN_ASYNC_V110		4
	#define RAD_VIRTUAL			5
#define RAD_PORT_LIMIT			62	/* Integer */
#define RAD_LOGIN_LAT_PORT		63	/* Integer */
#define RAD_CONNECT_INFO		77	/* String */

/* Accounting attribute types and values */
#define RAD_ACCT_STATUS_TYPE		40	/* Integer */
	#define RAD_START			1
	#define RAD_STOP			2
	#define RAD_ACCOUNTING_ON		7
	#define RAD_ACCOUNTING_OFF		8
#define RAD_ACCT_DELAY_TIME		41	/* Integer */
#define RAD_ACCT_INPUT_OCTETS		42	/* Integer */
#define RAD_ACCT_OUTPUT_OCTETS		43	/* Integer */
#define RAD_ACCT_SESSION_ID		44	/* String */
#define RAD_ACCT_AUTHENTIC		45	/* Integer */
	#define RAD_AUTH_RADIUS			1
	#define RAD_AUTH_LOCAL			2
	#define RAD_AUTH_REMOTE			3
#define RAD_ACCT_SESSION_TIME		46	/* Integer */
#define RAD_ACCT_INPUT_PACKETS		47	/* Integer */
#define RAD_ACCT_OUTPUT_PACKETS		48	/* Integer */
#define RAD_ACCT_TERMINATE_CAUSE	49	/* Integer */
        #define RAD_TERM_USER_REQUEST		1
        #define RAD_TERM_LOST_CARRIER		2
        #define RAD_TERM_LOST_SERVICE		3
        #define RAD_TERM_IDLE_TIMEOUT		4
        #define RAD_TERM_SESSION_TIMEOUT	5
        #define RAD_TERM_ADMIN_RESET		6
        #define RAD_TERM_ADMIN_REBOOT		7
        #define RAD_TERM_PORT_ERROR		8
        #define RAD_TERM_NAS_ERROR		9
        #define RAD_TERM_NAS_REQUEST		10
        #define RAD_TERM_NAS_REBOOT		11
        #define RAD_TERM_PORT_UNNEEDED		12
        #define RAD_TERM_PORT_PREEMPTED		13
        #define RAD_TERM_PORT_SUSPENDED		14
        #define RAD_TERM_SERVICE_UNAVAILABLE    15
        #define RAD_TERM_CALLBACK		16
        #define RAD_TERM_USER_ERROR		17
        #define RAD_TERM_HOST_REQUEST		18
#define	RAD_ACCT_MULTI_SESSION_ID	50	/* String */
#define	RAD_ACCT_LINK_COUNT		51	/* Integer */

struct rad_handle;
struct timeval;

__BEGIN_DECLS
struct rad_handle	*rad_acct_open(void);
int			 rad_add_server(struct rad_handle *,
			    const char *, int, const char *, int, int);
struct rad_handle	*rad_auth_open(void);
void			 rad_close(struct rad_handle *);
int			 rad_config(struct rad_handle *, const char *);
int			 rad_continue_send_request(struct rad_handle *, int,
			    int *, struct timeval *);
int			 rad_create_request(struct rad_handle *, int);
struct in_addr		 rad_cvt_addr(const void *);
u_int32_t		 rad_cvt_int(const void *);
char			*rad_cvt_string(const void *, size_t);
int			 rad_get_attr(struct rad_handle *, const void **,
			    size_t *);
int			 rad_init_send_request(struct rad_handle *, int *,
			    struct timeval *);
struct rad_handle	*rad_open(void);  /* Deprecated, == rad_auth_open */
int			 rad_put_addr(struct rad_handle *, int, struct in_addr);
int			 rad_put_attr(struct rad_handle *, int,
			    const void *, size_t);
int			 rad_put_int(struct rad_handle *, int, u_int32_t);
int			 rad_put_string(struct rad_handle *, int,
			    const char *);
int			 rad_send_request(struct rad_handle *);
const char		*rad_strerror(struct rad_handle *);
__END_DECLS

#endif /* _RADLIB_H_ */
EOF
FMD5=$(md5sum "radlib.h"|cut -d' ' -f1)
test "$MD5" = "$FMD5" && echo "restored: radlib.h (md5 ok)" || echo "ERROR: radlib.h (!md5)"
MD5=f98c5f450ad6dfb8cd9d86d8bbd6aef2 # radlib_private.h
cat <<'EOF' > "radlib_private.h"
/*-
 * Copyright 1998 Juniper Networks, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	$FreeBSD: src/lib/libradius/radlib_private.h,v 1.4 1999/10/31 04:47:59 jdp Exp $
 */

#ifndef RADLIB_PRIVATE_H
#define RADLIB_PRIVATE_H

#include <sys/types.h>
#include <netinet/in.h>

#include "radlib.h"

/* Handle types */
#define RADIUS_AUTH		0   /* RADIUS authentication, default */
#define RADIUS_ACCT		1   /* RADIUS accounting */

/* Defaults */
#define MAXTRIES		3
#define PATH_RADIUS_CONF	"/etc/radius.conf"
#define RADIUS_PORT		1812
#define RADACCT_PORT		1813
#define TIMEOUT			3	/* In seconds */

/* Limits */
#define ERRSIZE		128		/* Maximum error message length */
#define MAXCONFLINE	1024		/* Maximum config file line length */
#define MAXSERVERS	10		/* Maximum number of servers to try */
#define MSGSIZE		4096		/* Maximum RADIUS message */
#define PASSSIZE	128		/* Maximum significant password chars */

/* Positions of fields in RADIUS messages */
#define POS_CODE	0		/* Message code */
#define POS_IDENT	1		/* Identifier */
#define POS_LENGTH	2		/* Message length */
#define POS_AUTH	4		/* Authenticator */
#define LEN_AUTH	16		/* Length of authenticator */
#define POS_ATTRS	20		/* Start of attributes */

struct rad_server {
	struct sockaddr_in addr;	/* Address of server */
	char		*secret;	/* Shared secret */
	int		 timeout;	/* Timeout in seconds */
	int		 max_tries;	/* Number of tries before giving up */
	int		 num_tries;	/* Number of tries so far */
};

struct rad_handle {
	int		 fd;		/* Socket file descriptor */
	struct rad_server servers[MAXSERVERS];	/* Servers to contact */
	int		 num_servers;	/* Number of valid server entries */
	int		 ident;		/* Current identifier value */
	char		 errmsg[ERRSIZE];	/* Most recent error message */
	unsigned char	 request[MSGSIZE];	/* Request to send */
	int		 req_len;	/* Length of request */
	char		 pass[PASSSIZE];	/* Cleartext password */
	int		 pass_len;	/* Length of cleartext password */
	int		 pass_pos;	/* Position of scrambled password */
	char	 	 chap_pass;	/* Have we got a CHAP_PASSWORD ? */
	unsigned char	 response[MSGSIZE];	/* Response received */
	int		 resp_len;	/* Length of response */
	int		 resp_pos;	/* Current position scanning attrs */
	int		 total_tries;	/* How many requests we'll send */
	int		 try;		/* How many requests we've sent */
	int		 srv;		/* Server number we did last */
	int		 type;		/* Handle type */
};

#endif
EOF
FMD5=$(md5sum "radlib_private.h"|cut -d' ' -f1)
test "$MD5" = "$FMD5" && echo "restored: radlib_private.h (md5 ok)" || echo "ERROR: radlib_private.h (!md5)"
MD5=f8df6c1e97cc4471ddc72ec88fb85298 # README
cat <<'EOF' > "README"
This is a library to generate RADIUS authentication request.

Please read the copyrights in radlib.c :)

Some parts of the radlib.c I took from http://www.nlc.net.au/~john
who did the same for an older version of junipers libradius.

many tnx!
	Martin
(gremlin AT portal-to-web DOT de)



Installation:

FreeBSD
-------

This library is included in all FreeBSD 4.x and higher versions. So, if
you run one of this operating systems, you don't need this one!


NetBSD
------

It should compile now on NetBSD as well - tnx to Emmanuel Dreyfus ( manu AT netbsd DOT org )
for submitting the patch. 
libmd is not needed ( at least up from netbsd 1.6 - need to check older versions ), because
the MD5 functions are all in libc. comment out the section on 'netbsd' in the Makefile
to use the libc's MD5 routines 


Linux
-----

You will first need the MD5 library, libmd, installed. 
You can find it at http://www.portal-to-web.de/tacacs/download.php or
http://www.nlc.net.au/~john/software/libmd.tar.gz

The default location prefix for installation is /usr/local/
It also guesses, that libmd has the same install prefix. If you
want to install this lib somewhere else (like homedir...) modifiy
the PREFIX statement in the Makefile

Type the following to build...

	make 

Type the following to install..

	make install


EOF
FMD5=$(md5sum "README"|cut -d' ' -f1)
test "$MD5" = "$FMD5" && echo "restored: README (md5 ok)" || echo "ERROR: README (!md5)"
```
