/** @file wapi_asue.h
 *  @brief This header file contains data structures and function declarations of wapi_asue
 *
 *  Copyright (C) 2001-2008, Iwncomm Ltd.
 */


#ifndef __WAPI_ASUE_H__
#define __WAPI_ASUE_H__
#include <sys/socket.h>
#include <netinet/in.h>

#include <sys/time.h>
#include "config.h"
#include "drivers/driver.h"
#include "wapi_config.h"
#include "l2_packet/l2_packet.h"
//#include "wpa_supplicant.h"
#include "wapi.h"

#define CTRL_IFACE_SOCK struct sockaddr_in
#define WPA_CTRL_IFACE_PORT 9003
//#define WPA_CGI_IFACE_PORT 9002

typedef enum {
	WAPI_DISCONNECTED=0,		
#ifdef WAPI
	WAPI_UNDEFINED_1,
#endif
	WAPI_INACTIVE,
	WAPI_SCANNING,
#ifdef WAPI
        WAPI_UNDEFINED_2,
#endif
	WAPI_ASSOCIATING,
	WAPI_ASSOCIATED,
	WAPI_AUTHENICATING,
	WAPI_CERT_HANDSHAKE,
	WAPI_3WAY_HANDSHAKE,
	WAPI_3WAYING,
	WAPI_GROUP_HANDSHAKE,
	WAPI_GROUPING,
	WAPI_COMPLETED
} wapi_states;

typedef enum {
	WAI_ERR_INIT=0,
	WAI_ERR_BKID,
	WAI_ERR_CHALLENGE,
	WAI_ERR_IE,
	WAI_ERR_MIC,
} wapi_wai_err;

struct wai_hdr {
	u8 version[2];
	u8 type;
	u8 stype;
	u8 reserve[2]; 
	u16 length;
	u8 rxseq[2];				/* Packet	Sequence	Number	*/
	u8 frag_sc;				/* Fragment Sequence Number*/
	u8 more_frag;				/* Flag */
	/* followed octets of data */
};// __attribute__ ((packed));

struct wapi_state_machine {
	int bksa_count; /* number of entries in PMKSA cache */

	u8 own_addr[ETH_ALEN];
	const char *ifname;
	u8 bssid[ETH_ALEN];

	/* Selected configuration (based on Beacon/ProbeResp WAPI IE) */
	unsigned int pairwise_cipher;
	unsigned int group_cipher;
	unsigned int key_mgmt;

	u8 assoc_wapi_ie[256]; /* Own WAPI/RSN IE from (Re)AssocReq */
	u8 assoc_wapi_ie_len;
	u8 ap_wapi_ie[256];
	u8 ap_wapi_ie_len;
};

struct wapi_rxfrag{
	const u8 *data;
	int data_len;
	int maxlen;
	int rxfragstamp;
};
typedef struct byte_data_{
	u8 length;
	u8 data[256];
}byte_data;


/*Structure retry buf*/
struct _resendbuf_st{
	u16 		cur_count;
	u16		len;
	void 	*data;
};
typedef struct _resendbuf_st resendbuf_st;


/**
 * struct wapi_ssid - Network configuration data
 *
 * This structure includes all the configuration variables for a network. This
 * data is included in the per-interface configuration data as an element of
 * the network list, struct wpa_config::ssid. Each network block in the
 * configuration is mapped to a struct wpa_ssid instance.
 */

struct wapi_asue_st {
	struct l2_packet_data *l2;
	u8 own_addr[ETH_ALEN];
	u8 bssid[ETH_ALEN];
	char ifname[100];
	char *confname;
	struct wapi_config *conf;
	int reassociate; /* reassociation requested */
	int disconnected; /* all connections disabled; i.e., do no reassociate
			   * before this has been cleared */
	int wai_received;		   	
	/* Selected configuration (based on Beacon/ProbeResp WPA IE) */
	int pairwise_cipher;
	int group_cipher;
	int key_mgmt;

	void *drv_priv; /* private data used by driver_ops */
	struct wpa_scan_result *scan_results;
	int num_scan_results;
	struct wpa_driver_ops *driver;
	struct wapi_state_machine *wapi_sm;
	int ctrl_sock; /* UNIX domain socket for control interface or -1 if
			* not used */
	int cgi_sock; /* UNIX domain socket for control interface or -1 if
			* not used */
	int rxfragstamp;
	struct wapi_rxfrag *rxfrag;
	u16 rxseq;
	u16 txseq;

	wapi_states wapi_state;
	wapi_wai_err wai_err;
	unsigned char last_wai_src[ETH_ALEN];
	u8 pad;
	u8 flag;
	struct _resendbuf_st buf;
	u8 assoc_wapi_ie[256]; /* Own WAPI/RSN IE from (Re)AssocReq */
	u8 ap_wapi_ie[256];
	u8 ap_wapi_ie_len;
	u8 assoc_wapi_ie_len;
};
/**
 * wpa_supplicant_event - report a driver event for wpa_supplicant
 * @wpa_s: pointer to wpa_supplicant data; this is the @ctx variable registered
 *	with wpa_driver_events_init()
 * @event: event type (defined above)
 * @data: possible extra data for the event
 *
 * Driver wrapper code should call this function whenever an event is received
 * from the driver.
 */
//void wapi_asue_event(struct wpa_supplicant *wpa_s, wpa_event_type event,
//			  void *data);
//int wapi_asue_get_scan_results(struct wpa_supplicant *wpa_s);
//void wapi_asue_cancel_auth_timeout(struct wpa_supplicant *wpa_s);
#ifdef CONFIG_NO_WPA_MSG
#define wpa_msg(args...) do { } while (0)
#else
extern void wpa_msg(void *ctx, int level, const char *fmt, ...);
#endif
//void wapi_asue_req_scan(struct wpa_supplicant *wpa_s, int sec, int usec);
//void wapi_asue_associate(struct wpa_supplicant *wpa_s,const u8 *ssid, int ssid_len);
//void wapi_asue_disassociate(struct wpa_supplicant *wpa_s, int reason_code);
void *free_rxfrag(struct wapi_rxfrag *frag);
void wapi_asue_rx_wai(void *ctx, const u8 *src_addr, const u8 *buf, size_t len);
#endif
