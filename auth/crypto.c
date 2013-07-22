/* $Id: crypto.c,v 1.1 2006-10-09 11:44:57 news Exp $
 * (c) 2006 News-Service.com
 * NNTPSwitch Crypto Functions
 */

#include "crypto.h"

char *md5_bytes2digest (char digest[MD5_DIGEST_LENGTH * 2 + 1]
			, unsigned char md5_bytes[MD5_DIGEST_LENGTH])
{
	static const char hex[]="0123456789abcdef";

	for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
		digest[2 * i] = hex[md5_bytes[i] >> 4];
		digest[2 * i + 1] = hex[md5_bytes[i] & 0x0f];
	}

	digest[MD5_DIGEST_LENGTH * 2] = '\0';
	return digest;
}
