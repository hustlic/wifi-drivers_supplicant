/** @file wapi.h
 *  @brief This header file contains data structures and function declarations of wapi
 *
 *  Copyright (C) 2001-2008, Iwncomm Ltd.
 */

#ifndef WAPI_H
#define WAPI_H

#include "wapi_asue.h"

//#define BIT(n) (1 << (n))


#define WAI_VERSION 1
#define WAI_TYPE 1
/*WAI packets type*/
enum { 
	WAI_PREAUTH_START	= 0x01,	/*pre-authentication start*/
	WAI_STAKEY_REQUEST = 0x02,	/*STAKey request */
	WAI_AUTHACTIVE	= 0x03,			/*authentication activation*/
	WAI_ACCESS_AUTH_REQUEST = 0x04,	/*access authentication request */
	WAI_ACCESS_AUTH_RESPONSE = 0x05,	/*access authentication response */
	WAI_CERT_AUTH_REQUEST = 0x06,	/*certificate authentication request */
	WAI_CERT_AUTH_RESPONSE = 0x07,	/*certificate authentication response */
	WAI_USK_NEGOTIATION_REQUEST = 0x08,	/*unicast key negotiation request */
	WAI_USK_NEGOTIATION_RESPONSE = 0x09,	/* unicast key negotiation response */
	WAI_USK_NEGOTIATION_CONFIRMATION = 0x0A,/*unicast key negotiation confirmation */
	WAI_MSK_ANNOUNCEMENT = 0x0B, /*multicast key/STAKey announcement */
	WAI_MSK_ANNOUNCEMENT_RESPONSE = 0x0C, /*multicast key/STAKey announcement response */
	//WAI_SENDBK = 0x10, /*BK  for TMC ??*/
};


enum {
	REASON_RETRY_TIMEOUT = 25
};

//void * MEMCPY(void *dbuf, const void *srcbuf, int len) ;

#endif /* WPA_H */
