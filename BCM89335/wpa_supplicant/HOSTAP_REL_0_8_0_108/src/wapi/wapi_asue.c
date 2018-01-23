/* @file wapi_asue.c
 * @brief This file contains functions for wapi_asue
 *
 *  Copyright (C) 2001-2008, Iwncomm Ltd.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
/*for declaration of function `inet_addr'*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/time.h>

#include "wireless_copy.h"
#include "common.h"
#include "version.h"
#include "wapi_asue.h"
#include "wapi.h"
#include "wapi_config.h"
#include "wapi_asue_i.h"
#include "utils/eloop.h"
#include "drivers/driver_wext.h"
#include "wapi/wapi_interface.h"
//#include "wpa_supplicant.h"
#include "wpa.h"

#include "wpa_supplicant_i.h"
#include "wpa_ctrl.h"

extern void eloop_handle_alarm(int sig);

void *free_rxfrag(struct wapi_rxfrag *frag)
{
	if(frag != NULL){
		free_buffer((void *)(frag->data), frag->maxlen);
		free_buffer((void *)frag, sizeof(struct wapi_rxfrag ));
	}
	return NULL;
}

void wapi_asue_rx_wai(void *ctx, const u8 *src_addr, const u8 *buf, size_t len)
{
    wpa_printf(MSG_DEBUG, "%s, received %d bytes packet: %s\n", __FUNCTION__, len, buf);
    WAI_RX_packets_indication(buf, len);
}

unsigned char WIFI_get_rand_byte()
{
	return (unsigned char)(rand() & 0xFF);
}

void WIFI_Action_Deauth()
{
    struct wpa_global *global;
    struct wpa_supplicant *wpa_s;

    global = (struct wpa_global *)eloop_get_user_data(); 
    wpa_s = global->ifaces;

    wpa_msg(wpa_s, MSG_INFO, "WAPI: authentication failed");
    wpa_supplicant_deauthenticate(wpa_s, 15);
}

unsigned long WIFI_TX_packet(const char* pbuf, int length)
{
	struct wpa_global *global;
	struct wpa_supplicant *wpa_s;
	int ret;

	global = (struct wpa_global *)eloop_get_user_data(); 
	wpa_s = global->ifaces;

	wpa_printf(MSG_DEBUG, "%s: sending l2 %d bytes data: %s\n",
		   __FUNCTION__, length, pbuf);
	ret = l2_packet_send(wpa_s->l2_wapi, wpa_s->bssid, 0x88b4, (const u8*)pbuf, (unsigned  int)length);

	if (ret < 0)
	{
		return 0;
	}
	else
	{
		return (unsigned long)ret;
	}
}

int WIFI_group_key_set(const unsigned char* pKeyValue, int keylength, int key_idx, const unsigned char* keyIV)
{
	struct wpa_global *global;
	struct wpa_supplicant *wpa_s;
	int alg;
	u8 msk_len;
	
	wpa_printf(MSG_DEBUG, "WIFI_group_key_set: Installing group key to the driver.");
	wpa_hexdump(MSG_DEBUG, "WIFI_group_key_set: KEYSC", keyIV, 16);

	global = (struct wpa_global *)eloop_get_user_data(); 
	wpa_s = global->ifaces;

	alg = WAPI_ALG_SMS4;
	msk_len = 32;
	if (wapi_drv_set_key(wpa_s, alg, (u8 *) "\xff\xff\xff\xff\xff\xff",  key_idx, 1, keyIV, 16,   pKeyValue, keylength) < 0) 

	{
		wpa_printf(MSG_WARNING, "WPA: Failed to set MSK to "
			   "the driver.");
		return -1;
	}
	else
	    wpa_printf(MSG_DEBUG, "WAPI: succeeded to set MSK to the driver.");

	wpa_printf(MSG_DEBUG,"WIFI_group_key_set(): after setting the group key, set the wapi again.\n");

	wpa_printf(MSG_DEBUG,"wpa_s->conf->wapi_policy = 0x%x\n", wpa_s->wapi_conf->wapi_policy);

	if (wapi_drv_set_wapi(wpa_s, wpa_s->wapi_conf->wapi_policy) < 0) {
		wpa_printf(MSG_ERROR, "Failed to enable WPA in the driver.\n");
		return -1;
	}
	else {
	    wpa_printf(MSG_DEBUG,"WIFI_group_key_set(): calling wpa_supplicant_set_state(WPA_COMPLETE).\n");
	    wpa_s->wapi_state = WAPI_COMPLETED; 
	    wpa_supplicant_set_state(wpa_s, WPA_COMPLETED);
	}
	
	return 0;
}


int WIFI_unicast_key_set(const char* pKeyValue, int keylength, int key_idx)
{
	struct wpa_global *global;
	struct wpa_supplicant *wpa_s;

	int alg, keylen, rsclen;
	u8 *key_rsc;
	u8 null_rsc[16] = { 0, 0, 0, 0, 0, 0, 0, 0 };

	wpa_printf(MSG_DEBUG, "WIFI_unicast_key_set: Installing USK to the driver.");

	global = (struct wpa_global *)eloop_get_user_data(); 
	wpa_s = global->ifaces;

	alg = WAPI_ALG_SMS4;
	keylen = 32;
	rsclen = 16;

	key_rsc = null_rsc;
	if (wapi_drv_set_key(wpa_s, alg, wpa_s->bssid, key_idx, 1, key_rsc, rsclen,
			    (u8 *) pKeyValue, keylen) < 0) {
	    wpa_printf(MSG_WARNING, "WIFI_unicast_key_set: Failed to set PTK to the driver");
	    return -1;
	}

	return 0;
}

static void init_sigaction(int signum, OS_timer_expired handle)
{
	signal(signum, handle);
}

void* OS_timer_setup(int deltaTimer, int repeated, OS_timer_expired pfunction, const void* pdata)
{
	struct itimerval value;
	int sec = 0, usec = 0;
	int repsec = 0, repusec = 0;
	
	if (deltaTimer < 0 || repeated < 0 || pfunction == NULL)
		return NULL;
	
	init_sigaction(SIGALRM, pfunction);

	sec  = deltaTimer/(1000);
	usec = (deltaTimer%(1000)) * 1000;

	repsec  = repeated/(1000);
	repusec = (repeated%(1000)) * 1000;

	memset(&value, 0, sizeof(value));

	value.it_interval.tv_sec   = repsec;
	value.it_interval.tv_usec = repusec;

	value.it_value.tv_sec   = sec;
	value.it_value.tv_usec = usec;

	setitimer(ITIMER_REAL, &value, NULL);

	return (void *)pfunction;
}

void OS_timer_clean(void* pTimer)
{
	struct itimerval value;
	
	memset(&value, 0, sizeof(value));
	setitimer(ITIMER_REAL, &value, NULL);

	signal(SIGALRM, eloop_handle_alarm);
}

int WIFI_WAI_IE_set(const u8* pbuf, int length)
{
	struct wpa_global *global;
	struct wpa_supplicant *wpa_s;
	
	global = (struct wpa_global *)eloop_get_user_data(); 


	wpa_printf(MSG_ERROR, "#### Enter WIFI_WAI_IE_set\n");
	if (global == NULL) {
	    wpa_printf(MSG_ERROR, " global = NULL\n");
	    return -1;
	}
	else
	    wpa_s = global->ifaces;

	if (wpa_s == NULL) {
	    wpa_printf(MSG_ERROR, " wpa_s = NULL\n");
	    return -1;
	}

	wpa_printf(MSG_ERROR, " Set wpa_s to global->ifaces");
	wpa_s = global->ifaces;
	wapi_drv_set_wpa_ie(wpa_s, pbuf, length);
	wpa_s->assoc_wapi_ie_len = length;
                    wpa_printf(MSG_DEBUG, "wpa_s->assoc_wapi_ie_len : %d",wpa_s->assoc_wapi_ie_len);
	memcpy(wpa_s->assoc_wapi_ie, pbuf, length);
                    wpa_printf(MSG_DEBUG, "wpa_s->assoc_wapi_ie: [%02x][%02x][%02x][%02x]\n",
                            wpa_s->assoc_wapi_ie[0], wpa_s->assoc_wapi_ie[1], wpa_s->assoc_wapi_ie[2], wpa_s->assoc_wapi_ie[3]);
		    wpa_printf(MSG_DEBUG, "@@@@@@@@@@@@@ wpa_s ===> %p\n",wpa_s);

	return 0;
}
