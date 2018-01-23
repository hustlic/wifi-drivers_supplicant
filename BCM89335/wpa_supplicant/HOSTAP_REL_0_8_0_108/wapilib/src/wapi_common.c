/** @file common.c
  * @brief This file contains functions for common, include random, print, ecc
 *
 *  Copyright (C) 2001-2008, Iwncomm Ltd.
 */



#include <stdarg.h>
#include <ctype.h>

//#include "hmac_sha256.h"
#include "alg_comm.h"
#include "wapi_common.h"
#include "wapi_interface.h"

#ifdef ANDROID
#include <cutils/log.h>
#endif

static int iwn_debug_level = MSG_INFO;





void iwn_wpa_printf(int level, char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	if (level >= iwn_debug_level) {
	    vprintf(fmt, ap);
	    printf("\n");
#ifdef ANDROID
	    if (level == MSG_DEBUG)
		level = ANDROID_LOG_DEBUG;
	    else if (level == MSG_INFO)
		level = ANDROID_LOG_INFO;
	    else if (level == MSG_WARNING)
		level = ANDROID_LOG_WARN;
	    else if (level == MSG_ERROR)
		level = ANDROID_LOG_ERROR;
	    else
		level = ANDROID_LOG_DEBUG;

	    LOG_PRI_VA(level, "wapilib", fmt, ap);
#endif
	}
	va_end(ap);
}

void iwn_wpa_hexdump(int level, const char *title, const u8 *buf, size_t len)
{
	if (level >= iwn_debug_level)
	{
		print_buf(title, buf, len);
	}
}

void print_buf(const char* title, const void* buf, int len)
{
//#define tmpfun_print printf

	int cl = (len-1)/16 + 1;
	int i;
	int j;
	char txt[260] = {0};
	sprintf(txt, "\n%s(len=%d,0x%X)\n", title, len, len);
	//tmpfun_print(txt);
	iwn_wpa_printf(MSG_DEBUG, txt);
	for (i=0; i<cl; i++)
	{
		int n = 16;
		const unsigned char* p = NULL;
		if (i == cl-1)
		{/*lastest line*/
			n = len%16;
			if (0 == n)
			{
				n = 16;
			}
		}
		sprintf(txt, "%.4X  ", i*16);
		p = (const unsigned char*)buf + i*16;
		for (j=0; j<n; j++)
		{
			sprintf(txt+strlen(txt), "%.2X", p[j]);
			if (j == n-1)
			{/*lastest char*/
				if (16 == n)
				{/*whole line*/
					strcat(txt, "  ");
				}
				else
				{/*fill none whole line*/
					int k;
					for (k=strlen(txt); k<43; k++)
					{
						txt[k] = ' ';
					}
					txt[k] = 0;
				}
			}
			else if (3 == j%4)
			{
				strcat(txt, " ");
			}
		}
		for (j=0; j<n; j++)
		{
			int c = p[j];
			if (c < 32)
			{
				c = '.';
			}
			if (c>127 && c<160)
			{
				c = '.';
			}
			if (c>127)
			{
/*				c = '.';*/
			}
/*			c = '.';*/

			{
				char tmp[2] = {c};
				strcat(txt, tmp);
			}
		}
		//tmpfun_print(txt);
		iwn_wpa_printf(MSG_DEBUG, txt);
		//tmpfun_print("\n");
	}
	iwn_wpa_printf(MSG_DEBUG, "\n");
	//tmpfun_print("\n\n");

//#undef tmpfun_print
}

/* ---------------------------------------------------------------------------------------
 * [Name]      x509_ecc_verify
 * [Function]   verify the sign information with the public key
 * [Input]       const unsigned char *pub_s
 *                  int pub_sl,
 *                  ......
 * [Output]     NULL
 * [Return]     int
 *                              1   success
 *                              0   fail
 * [Limitation] NULL
 * ---------------------------------------------------------------------------------------
 */
int   x509_ecc_verify(const unsigned char *pub_s, int pub_sl, unsigned char *in ,  int in_len, unsigned char *sign,int sign_len)
{
	int ret = 0;

	if (pub_s == NULL || pub_sl <= 0 || in == NULL || in_len <= 0 || sign == NULL || sign_len <= 0)
	{
		return ret;
	}
	else
	{
		ret = ecc192_verify(pub_s, in, in_len, sign, sign_len);
	}

	if (ret <= 0)
		ret = 0;
	else
		ret = 1;

	return ret;
}


/* ---------------------------------------------------------------------------------------
 * [Name]      x509_ecc_sign
 * [Function]   sign with the private key
 * [Input]       const unsigned char *priv_s
 *                  int priv_sl,
 *                  const unsigned char *int,
 *                  int in_len
 * [Output]     unsigned char *out
 * [Return]     int
 *                              > 0   success
 *                              0      fail
 * [Limitation] NULL
 * ---------------------------------------------------------------------------------------
 */
int x509_ecc_sign(const unsigned char *priv_s, int priv_sl, const unsigned char * in, int in_len, unsigned char *out)
{
	priv_sl = priv_sl;/*disable warnning*/
	if (priv_s == NULL || in == NULL || in_len <= 0 || out == NULL)
	{
		return 0;
	}
	else
	{
		return ecc192_sign(priv_s, in, in_len, out);
	}
}



/* ---------------------------------------------------------------------------------------
 * [Name]      x509_ecc_verify_key
 * [Function]   verify the public key and the private key
 * [Input]       const unsigned char *pub_s
 *                  int pub_sl,
 *                  const unsigned char *priv_s
 *                  int priv_sl
 * [Output]     NULL
 * [Return]     int
 *                              1   success
 *                              0   fail
 * [Limitation] NULL
 * ---------------------------------------------------------------------------------------
 */
int x509_ecc_verify_key(const unsigned char *pub_s, int pub_sl, const unsigned char *priv_s, int priv_sl)
{
#define EC962_SIGN_LEN		48
	unsigned char data[] = "123456abcd";
	unsigned char sign[EC962_SIGN_LEN+1];
	int ret = 0;

	if (priv_s == NULL || pub_sl <= 0  || priv_s == NULL || priv_sl <= 0)
	{
		return 0;
	}

	memset(sign, 0, sizeof(sign));
	ret = ecc192_sign(priv_s, data, strlen((char*)data), sign);
	if (ret != EC962_SIGN_LEN)
	{
		printf("ecc192_sign call fail \n");
		ret = 0;
		return ret;
	}

	ret = ecc192_verify(pub_s, data, strlen((char*)data), sign, EC962_SIGN_LEN);
	if (ret <= 0)
	{
		printf("ecc192_verify call fail \n");
		ret = 0;
	}

	return ret;

}
