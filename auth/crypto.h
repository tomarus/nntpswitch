/* $Id: crypto.h,v 1.1 2006-10-09 11:44:57 news Exp $
 * (c) 2006 News-Service.com
 * NNTPSwitch Scramble Functions
 * scramble_*, md5 stuff, mcrypt
 */

#include <openssl/md5.h>

char *md5_bytes2digest (char digest[MD5_DIGEST_LENGTH * 2 + 1]
			, unsigned char md5_bytes[MD5_DIGEST_LENGTH]);
