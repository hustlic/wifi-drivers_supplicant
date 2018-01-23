/*
 * Driver interaction with extended Linux CFG8021
 * Copyright (c) 2012-2013, Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 */

#include "driver_nl80211.h"
#include "wpa_supplicant_i.h"
#include "config.h"
#include "wpabuf.h"
#include "wpa_ctrl.h"
#include "p2p_supplicant.h"
#include "brcm_vendor.h"
#ifdef ANDROID
#include "android_drv.h"
#endif

#ifdef BCM_GENL
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <linux/rtnetlink.h>
#endif

#define WPA_PS_ENABLED		0
#define WPA_PS_DISABLED		1

#define MAX_WPSP2PIE_CMD_SIZE		512
#ifdef BCM_MAP_SDCMDS_2_SDOCMDS
#define SD_REQ_ASCII_HEADER_LEN 11			//space after P2P_SD_REQ  is included
#define MAC_ADDR_ASCII_LEN 18				//space after Mac address is included
#endif

#ifdef BRCM_DRV_ROAM
#define MAX_NUM_SUITES	10
#endif /* BRCM_DRV_ROAM */

typedef struct android_wifi_priv_cmd {
	char *buf;
	int used_len;
	int total_len;
} android_wifi_priv_cmd;

#define SDO_FRAGMENT_NUM_MASK	 0x7f
#define SDO_MORE_FRAGMNT_MASK	 0x80
typedef struct sdo_hdr {
    u8	addr[ETH_ALEN];
    u16	freq;        /* channel */
    u8	count;        /* GAS fragment id */
	u16	update_ind;
} sdo_hdr_t;

/* service discovery TLV */
typedef struct sd_tlv {
     u16  length;         /* length of response_data */
     u8   protocol;       /* service protocol type */
     u8   transaction_id;     /* service transaction id */
     u8   status_code;        /* status code */
     u8   data[1];        /* response data */
} sd_tlv_t;

/* Service Protocol Type */
typedef enum svc_protype {
    SVC_RPOTYPE_ALL = 0,
    SVC_RPOTYPE_BONJOUR = 1,
    SVC_RPOTYPE_UPNP = 2,
    SVC_RPOTYPE_WSD = 3,
    SVC_RPOTYPE_VENDOR = 255
} svc_protype_t;


#ifndef HOSTAPD
extern int wpas_enable_dd_offload(struct wpa_supplicant *wpa_s, u8 enable);
extern int wpas_get_listen_channel(void *priv);
extern void wpas_p2p_scan_res_handler(void *priv, struct wpa_scan_results *scan_res);
#endif
int send_and_recv_msgs(struct wpa_driver_nl80211_data *drv, struct nl_msg *msg,
		       int (*valid_handler)(struct nl_msg *, void *),
		       void *valid_data);
extern int nl80211_send_vendor_command(struct wpa_driver_nl80211_data *drv ,
					struct i802_bss *bss, int,  char *,
					size_t len, int vendor_id);

#if defined(BCM_GENL) && defined(BCM_GENL_UNICAST)
/* Only required, if you need to support GENL UNICAST*/
static int wpa_driver_set_supplicant_pid(void *priv);
#endif /* BCM_GENL && BCM_GENL_UNICAST */

#ifdef BCM_GENL
struct family_data {
        const char *group;
        int id;
};

int driver_get_multicast_id(void *priv,
                               const char *family, const char *group);
/* attributes (variables): the index in this enum is used as a reference for the type,
 *             userspace application has to indicate the corresponding type
 *             the policy is used for security considerations
 */
enum {
        BCM_EVENT_UNSPEC,
        BCM_EVENT_SVC_FOUND,
        BCM_EVENT_DEV_FOUND,
        BCM_EVENT_DEV_LOST,
        BCM_EVENT_MAX
};

enum {
        BCM_GENL_ATTR_UNSPEC,
        BCM_GENL_ATTR_STRING,
        BCM_GENL_ATTR_MSG,
        __BCM_GENL_ATTR_MAX,
};
#define BCM_GENL_ATTR_MAX (__BCM_GENL_ATTR_MAX - 1)

/* commands: enumeration of all commands (functions),
 * used by userspace application to identify command to be ececuted
 */
enum {
        BCM_GENL_CMD_UNSPEC,
        BCM_GENL_CMD_MSG,
        __BCM_GENL_CMD_MAX,
};
#define BCM_GENL_CMD_MAX (__BCM_GENL_CMD_MAX - 1)

typedef struct bcm_event_hdr {
	u16 type;
	u16 len;
} bcm_hdr_t;

typedef struct bcm_dev_info {
        u16 band;
        u16 freq;
        s16 rssi;
        u16 ie_len;
        u8 bssid[ETH_ALEN];
} bcm_dev_info_t;

static int wpa_driver_handle_attr_data(struct wpa_driver_nl80211_data *drv,
					char *data, unsigned int len);
int wpa_driver_register_genl(void *priv);
int wpa_driver_unregister_genl(void *priv);
static int family_handler(struct nl_msg *msg, void *arg);
static int no_seq_check(struct nl_msg *msg, void *arg);
#define GENLMSG_DATA(glh) ((char *)(NLMSG_DATA(glh) + GENL_HDRLEN))
#define GENLMSG_PAYLOAD(glh) ((char *)(NLMSG_PAYLOAD(glh, 0) - GENL_HDRLEN))
#define NLA_DATA(na) (void *) ((char *)(na) + NLA_HDRLEN)
#endif /* BCM_GENL */

static int drv_errors = 0;
static int g_privlib_initialized = 0;

static void wpa_driver_send_hang_msg(struct wpa_driver_nl80211_data *drv)
{
	drv_errors++;
	if (drv_errors > DRV_NUMBER_SEQUENTIAL_ERRORS) {
		drv_errors = 0;
		wpa_msg(drv->ctx, MSG_INFO, WPA_EVENT_DRIVER_STATE "HANGED");
	}
}

static int wpa_driver_set_power_save(void *priv, int state)
{
	struct i802_bss *bss = priv;
	struct wpa_driver_nl80211_data *drv = bss->drv;
	struct nl_msg *msg;
	int ret = -1;
	enum nl80211_ps_state ps_state;

	msg = nlmsg_alloc();
	if (!msg)
		return -1;

	genlmsg_put(msg, 0, 0, drv->global->nl80211_id, 0, 0,
		    NL80211_CMD_SET_POWER_SAVE, 0);

	if (state == WPA_PS_ENABLED)
		ps_state = NL80211_PS_ENABLED;
	else
		ps_state = NL80211_PS_DISABLED;

	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, drv->ifindex);
	NLA_PUT_U32(msg, NL80211_ATTR_PS_STATE, ps_state);

	ret = send_and_recv_msgs(drv, msg, NULL, NULL);
	msg = NULL;
	if (ret < 0)
		wpa_printf(MSG_ERROR, "nl80211: Set power mode fail: %d", ret);
nla_put_failure:
	nlmsg_free(msg);
	return ret;
}

static int get_power_mode_handler(struct nl_msg *msg, void *arg)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	int *state = (int *)arg;

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	if (!tb[NL80211_ATTR_PS_STATE])
		return NL_SKIP;

	if (state) {
		*state = (int)nla_get_u32(tb[NL80211_ATTR_PS_STATE]);
		wpa_printf(MSG_DEBUG, "nl80211: Get power mode = %d", *state);
		*state = (*state == NL80211_PS_ENABLED) ?
				WPA_PS_ENABLED : WPA_PS_DISABLED;
	}

	return NL_SKIP;
}

static int wpa_driver_get_power_save(void *priv, int *state)
{
	struct i802_bss *bss = priv;
	struct wpa_driver_nl80211_data *drv = bss->drv;
	struct nl_msg *msg;
	int ret = -1;
	enum nl80211_ps_state ps_state;

	msg = nlmsg_alloc();
	if (!msg)
		return -1;

	genlmsg_put(msg, 0, 0, drv->global->nl80211_id, 0, 0,
		    NL80211_CMD_GET_POWER_SAVE, 0);

	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, drv->ifindex);

	ret = send_and_recv_msgs(drv, msg, get_power_mode_handler, state);
	msg = NULL;
	if (ret < 0)
		wpa_printf(MSG_ERROR, "nl80211: Get power mode fail: %d", ret);
nla_put_failure:
	nlmsg_free(msg);
	return ret;
}

#if defined(BCM_GENL) && defined(BCM_GENL_UNICAST)
static int wpa_driver_set_supplicant_pid(void *priv)
{
	struct i802_bss *bss = priv;
	struct wpa_driver_nl80211_data *drv = bss->drv;
	struct wpa_supplicant *wpa_s;
	struct ifreq ifr;
	android_wifi_priv_cmd priv_cmd;
	int ret = 0, i = 0, bp;
	char buf[30];
	int len = 0;

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "SUPP_PID %d", getpid());
	len = strlen(buf);

	memset(&ifr, 0, sizeof(ifr));
	memset(&priv_cmd, 0, sizeof(priv_cmd));
	os_strncpy(ifr.ifr_name, bss->ifname, IFNAMSIZ);

	priv_cmd.buf = buf;
	priv_cmd.used_len = len;
	priv_cmd.total_len = len;
	ifr.ifr_data = &priv_cmd;

	ret = ioctl(drv->global->ioctl_sock, SIOCDEVPRIVATE + 1, &ifr);

	if (ret < 0) {
        wpa_printf(MSG_ERROR, "ioctl[SIOCSIWPRIV] SUPP_PID: %d", ret);
        wpa_driver_send_hang_msg(drv);
    } else {
        drv_errors = 0;
    }

    return ret;
}
#endif /* BCM_GENL && BCM_GENL_UNICAST */
static int wpa_driver_set_backgroundscan_params(void *priv)
{
	struct i802_bss *bss = priv;
	struct wpa_driver_nl80211_data *drv = bss->drv;
	struct wpa_supplicant *wpa_s;
	struct ifreq ifr;
	android_wifi_priv_cmd priv_cmd;
	int ret = 0, i = 0, bp;
	char buf[WEXT_PNO_MAX_COMMAND_SIZE];
	struct wpa_ssid *ssid_conf;

	if (drv == NULL) {
		wpa_printf(MSG_ERROR, "%s: drv is NULL. Exiting", __func__);
		return -1;
	}
	if (drv->ctx == NULL) {
		wpa_printf(MSG_ERROR, "%s: drv->ctx is NULL. Exiting", __func__);
		return -1;
	}
	wpa_s = (struct wpa_supplicant *)(drv->ctx);
	if (wpa_s->conf == NULL) {
		wpa_printf(MSG_ERROR, "%s: wpa_s->conf is NULL. Exiting", __func__);
		return -1;
	}
	ssid_conf = wpa_s->conf->ssid;

	bp = WEXT_PNOSETUP_HEADER_SIZE;
	os_memcpy(buf, WEXT_PNOSETUP_HEADER, bp);
	buf[bp++] = WEXT_PNO_TLV_PREFIX;
	buf[bp++] = WEXT_PNO_TLV_VERSION;
	buf[bp++] = WEXT_PNO_TLV_SUBVERSION;
	buf[bp++] = WEXT_PNO_TLV_RESERVED;

	while ((i < WEXT_PNO_AMOUNT) && (ssid_conf != NULL)) {
		/* Check that there is enough space needed for 1 more SSID, the other sections and null termination */
		if ((bp + WEXT_PNO_SSID_HEADER_SIZE + MAX_SSID_LEN + WEXT_PNO_NONSSID_SECTIONS_SIZE + 1) >= (int)sizeof(buf))
			break;
		if ((!ssid_conf->disabled) && (ssid_conf->ssid_len <= MAX_SSID_LEN)){
			wpa_printf(MSG_DEBUG, "For PNO Scan: %s", ssid_conf->ssid);
			buf[bp++] = WEXT_PNO_SSID_SECTION;
			buf[bp++] = ssid_conf->ssid_len;
			os_memcpy(&buf[bp], ssid_conf->ssid, ssid_conf->ssid_len);
			bp += ssid_conf->ssid_len;
			i++;
		}
		ssid_conf = ssid_conf->next;
	}

	buf[bp++] = WEXT_PNO_SCAN_INTERVAL_SECTION;
	os_snprintf(&buf[bp], WEXT_PNO_SCAN_INTERVAL_LENGTH + 1, "%x", WEXT_PNO_SCAN_INTERVAL);
	bp += WEXT_PNO_SCAN_INTERVAL_LENGTH;

	buf[bp++] = WEXT_PNO_REPEAT_SECTION;
	os_snprintf(&buf[bp], WEXT_PNO_REPEAT_LENGTH + 1, "%x", WEXT_PNO_REPEAT);
	bp += WEXT_PNO_REPEAT_LENGTH;

	buf[bp++] = WEXT_PNO_MAX_REPEAT_SECTION;
	os_snprintf(&buf[bp], WEXT_PNO_MAX_REPEAT_LENGTH + 1, "%x", WEXT_PNO_MAX_REPEAT);
	bp += WEXT_PNO_MAX_REPEAT_LENGTH + 1;

	memset(&ifr, 0, sizeof(ifr));
	memset(&priv_cmd, 0, sizeof(priv_cmd));
	os_strncpy(ifr.ifr_name, bss->ifname, IFNAMSIZ);

	priv_cmd.buf = buf;
	priv_cmd.used_len = bp;
	priv_cmd.total_len = bp;
	ifr.ifr_data = (void *)&priv_cmd;

	ret = ioctl(drv->global->ioctl_sock, SIOCDEVPRIVATE + 1, &ifr);

	if (ret < 0) {
		wpa_printf(MSG_ERROR, "ioctl[SIOCSIWPRIV] (pnosetup): %d", ret);
		wpa_driver_send_hang_msg(drv);
	} else {
		drv_errors = 0;
	}
	return ret;
}

#ifdef BRCM_DRV_ROAM
static int wpa_driver_set_roampref(void *priv)
{
	struct i802_bss *bss = priv;
	struct wpa_driver_nl80211_data *drv = bss->drv;
	struct wpa_supplicant *wpa_s;
	struct ifreq ifr;
	android_wifi_priv_cmd priv_cmd;
	struct wpa_ssid *ssid;
	int ret = 0;
	char buf[MAX_DRV_CMD_SIZE];
	int num_akm_suites = 0;
	int num_ucipher_suites = 0;
	u32 akm_suites[MAX_NUM_SUITES];
	u32 ucipher_suites[MAX_NUM_SUITES];
	int i, bp = 0;

	if (drv == NULL) {
		wpa_printf(MSG_ERROR, "%s: drv is NULL. Exiting", __func__);
		return -1;
	}
	if (drv->ctx == NULL) {
		wpa_printf(MSG_ERROR, "%s: drv->ctx is NULL. Exiting", __func__);
		return -1;
	}
	wpa_s = (struct wpa_supplicant *)(drv->ctx);
	if (wpa_s->conf == NULL) {
		wpa_printf(MSG_ERROR, "%s: wpa_s->conf is NULL. Exiting", __func__);
		return -1;
	}
	ssid = wpa_s->conf->ssid;

	/* If the key_mgmt is set for open network only, then num_akm_suites=0 is passed
	 * to the driver which in turn resets the previously configured join preferences.
	 * If the key_mgmt is not set in *.conf file, then it gets set to default
	 * (WPA_KEY_MGMT_PSK and WPA_KEY_MGMT_IEEE8021X).
	 */
	if (ssid->key_mgmt & WPA_KEY_MGMT_IEEE8021X)
		akm_suites[num_akm_suites++] = WLAN_AKM_SUITE_8021X;
	if (ssid->key_mgmt & WPA_KEY_MGMT_PSK)
		akm_suites[num_akm_suites++] = WLAN_AKM_SUITE_PSK;

	/* If the pairwise cipher is not set in *.conf file, then it gets set to
	 * default (WPA_CIPHER_CCMP and WPA_CIPHER_TKIP).
	 */
	if (ssid->pairwise_cipher & WPA_CIPHER_CCMP)
		ucipher_suites[num_ucipher_suites++] = WLAN_CIPHER_SUITE_CCMP;
	if (ssid->pairwise_cipher & WPA_CIPHER_GCMP)
		ucipher_suites[num_ucipher_suites++] = WLAN_CIPHER_SUITE_GCMP;
	if (ssid->pairwise_cipher & WPA_CIPHER_TKIP)
		ucipher_suites[num_ucipher_suites++] = WLAN_CIPHER_SUITE_TKIP;
	if (ssid->pairwise_cipher & WPA_CIPHER_WEP104)
		ucipher_suites[num_ucipher_suites++] = WLAN_CIPHER_SUITE_WEP104;
	if (ssid->pairwise_cipher & WPA_CIPHER_WEP40)
		ucipher_suites[num_ucipher_suites++] = WLAN_CIPHER_SUITE_WEP40;

	if (num_ucipher_suites * num_akm_suites > 16) {
		wpa_printf(MSG_ERROR, "%s: Exceeds max supported wpa configs ", __func__);
		return -1;
	}

	os_memset(buf, 0, sizeof(buf));
	bp += os_snprintf(&buf[bp], sizeof(buf), "%s ", "SET_ROAMPREF");
	bp += os_snprintf(&buf[bp], sizeof(buf), "%02x ", num_akm_suites);
	for (i = 0; i < num_akm_suites; i++)
		bp += os_snprintf(&buf[bp], sizeof(buf), "%08x", akm_suites[i]);

	bp += os_snprintf(&buf[bp], sizeof(buf), "%02x ", num_ucipher_suites);
	for (i = 0; i < num_ucipher_suites; i++)
		bp += os_snprintf(&buf[bp], sizeof(buf), "%08x", ucipher_suites[i]);

	/* Increment the pointer to copy the null character to ioctl buffer */
	bp++;

	memset(&ifr, 0, sizeof(ifr));
	memset(&priv_cmd, 0, sizeof(priv_cmd));
	os_strncpy(ifr.ifr_name, bss->ifname, IFNAMSIZ);
	priv_cmd.buf = buf;
	priv_cmd.used_len = bp;
	priv_cmd.total_len = bp;
	ifr.ifr_data = &priv_cmd;

	ret = ioctl(drv->global->ioctl_sock, SIOCDEVPRIVATE + 1, &ifr);
	if (ret < 0) {
		wpa_printf(MSG_ERROR, "ioctl[SIOCSIWPRIV] SET_ROAMPREF: %d", ret);
		wpa_driver_send_hang_msg(drv);
	} else {
		drv_errors = 0;
	}
	return ret;
}
#endif

#ifdef BCM_MAP_SDCMDS_2_SDOCMDS
int wpa_driver_sdo_req(char *buf)
{
	unsigned char pos[50] = {0};
	char temp[1000] = {0};
	int len = 0;
	if (os_strstr(buf, "upnp")) {
		/* So the request after change in ctrl_iface.c must be in below string format for QR pair
		 * P2P_SD_REQ xx:xx:xx:xx:xx:xx upnp 10 urn:schemas-upnp-org:service:ContentDirectory:1
		 * And our format to driver should be P2P SD_REQ upnp 0x10urn:schemas-upnporg:service:Con
		 * tentDirectory:1
		 */
		os_snprintf(temp, sizeof(temp), "%s%s", "upnp 0x10",
				(buf + SD_REQ_ASCII_HEADER_LEN + MAC_ADDR_ASCII_LEN + os_strlen("upnp 10 ")));
		/*"P2P_SD_REQ " is already in buf, so start copy to 11th position*/
		os_snprintf(buf + 11, sizeof(temp), "%s", temp);
	} else if (os_strstr(buf, "bonjour")) {
		/* Code to be added Later*/
	} else {
		/* So the request after change in ctrl_iface.c must be in below ASCII format for QR pair
		  * P2P_SD_REQ 00:00:00:00:00:00 320002011075726e3a736368656d61732d75706e702d6f72673a736
		  * 572766963653a436f6e74656e744469726563746f72793a31
		  * Lets calculate the buf len starting from the actual qr pair, so 29 bytes includes
		  * P2P_SD_REQ and len, transactionid and proto */
		len = os_strlen(buf + 29);
		/*length should contains proto and transaction ID*/
		if ((len & 1) || (len < 8))
			return -1;
		len /= 2;
		/* we may get request with or without version so verifying for 4 byte only*/
		if (hexstr2bin(buf + 29, pos, 4) < 0) {
			wpa_printf(MSG_DEBUG, "error in len proto parsing");
			return -1;
		}
		/* pos[3] is Transaction ID */
		wpa_printf(MSG_DEBUG, "len %d proto %d transaction_id %d\n", pos[0], pos[2], pos[3]);
		/* check is required to verify data exists or not */
		if(pos[0] > 3) {
			/* substracting 3 to exclude length and proto, Passing transaction ID and
			 * version with request
			 */
			len -= 3;
			/* we need to move 39 bytes to get start pointer of data buffer of P2P_SD_REQ
			 * we reached upto this pointer after excluding initial 10 bytes which are length,
			 * proto and transaction ID
			 */
			//transaction_id to be included
			if (hexstr2bin((buf + (SD_REQ_ASCII_HEADER_LEN + MAC_ADDR_ASCII_LEN + 6)), pos, len) < 0) {
				wpa_printf(MSG_DEBUG, "error in string parsing\n");
				return -1;
			}
			wpa_printf(MSG_DEBUG, "pos:%s\n", pos);
			os_snprintf(temp, sizeof(temp), "%s %x %s", "upnp", pos[3],  "0x10");
		} else {
			/*upnp*/
			if(pos[2] == 2) {
				wpa_printf(MSG_DEBUG, "upnp\n");
				os_snprintf(temp, sizeof(temp), "%s %x", "upnp", pos[3]);
			}
		}
		/*"P2P_SD_REQ " is already in buf, so start copy to 11th position*/
		os_snprintf(buf + 11, sizeof(temp), "%s", temp);
		wpa_printf(MSG_DEBUG, "Control interface command '%s'", buf);
	}
	return 0;
}

#endif /* BCM_MAP_SDCMDS_2_SDOCMDS */

static int wpa_get_best_channels(void *drv_ctx, char *buf)
{
	union wpa_event_data event;

	if (NULL == drv_ctx) {
		wpa_printf(MSG_ERROR, "%s: drv_ctx is NULL. Exiting", __func__);
		return -1;
	}

	memset(&event, 0, sizeof(event));

	sscanf(buf, "%04d %04d %04d", &(event.best_chan.freq_24),
		&(event.best_chan.freq_5), &(event.best_chan.freq_overall));

	wpa_supplicant_event(drv_ctx, EVENT_BEST_CHANNEL, &event);

	return 0;
}

int wpa_driver_nl80211_driver_cmd(void *priv, char *cmd, char *buf,
				  size_t buf_len )
{
	struct i802_bss *bss = priv;
	struct wpa_driver_nl80211_data *drv = bss->drv;
	struct ifreq ifr;
	android_wifi_priv_cmd priv_cmd;
	int ret = 0;
	struct wpa_supplicant *wpa_s;
	char *token = NULL, *cmd_ptr;
	enum brcm_nl80211_vendor_subcmds type;

	if (bss->ifindex <= 0 && bss->wdev_id > 0) {
		/*
		 * DRIVER CMD received on the DEDICATED P2P Interface which doesn't
		 * have an NETDEVICE associated with it. So we have to re-route the
		 * command to the parent NETDEVICE
		 */
		wpa_printf(MSG_INFO, "%s: DRIVER cmd received on P2P Dedicated"
				"IFACE. Routing the command to parent iface", __func__);
		wpa_s = (struct wpa_supplicant *)(drv->ctx);
		if (wpa_s && wpa_s->parent) {
			/* Update the nl80211 pointers corresponding to parent iface */
			bss = priv = wpa_s->parent->drv_priv;
			drv = bss->drv;
			wpa_printf(MSG_DEBUG, "Re-routing command to iface: %s "
				"cmd (%s)", bss->ifname, cmd);
		}
	}

	if (os_strcasecmp(cmd, "STOP") == 0) {
		linux_set_iface_flags(drv->global->ioctl_sock, bss->ifname, 0);
		wpa_msg(drv->ctx, MSG_INFO, WPA_EVENT_DRIVER_STATE "STOPPED");
	} else if (os_strncasecmp(cmd, "VENDOR ", 7) == 0) {
		wpa_printf(MSG_INFO, "%s: Vendor cmd(0x%x) received on wlan %s %s",
				__func__, NL80211_CMD_VENDOR, buf, cmd);
		cmd_ptr = cmd + strlen("VENDOR ");
		return nl80211_send_vendor_command(drv,bss, BRCM_VENDOR_SUBCMD_PRIV_STR,
				cmd_ptr, strlen(cmd_ptr), OUI_BRCM);
	} else if (os_strcasecmp(cmd, "START") == 0) {
		linux_set_iface_flags(drv->global->ioctl_sock, bss->ifname, 1);
		wpa_msg(drv->ctx, MSG_INFO, WPA_EVENT_DRIVER_STATE "STARTED");
	} else if (os_strcasecmp(cmd, "MACADDR") == 0) {
		u8 macaddr[ETH_ALEN] = {};

		ret = linux_get_ifhwaddr(drv->global->ioctl_sock, bss->ifname, macaddr);
		if (!ret) {
			ret = os_snprintf(buf, buf_len,
					"Macaddr = " MACSTR "\n", MAC2STR(macaddr));
			drv_errors = 0;
		} else {
			wpa_driver_send_hang_msg(drv);
		}
		return ret;
	} else if (os_strcasecmp(cmd, "RELOAD") == 0) {
		wpa_msg(drv->ctx, MSG_INFO, WPA_EVENT_DRIVER_STATE "HANGED");
		return 0;
	} else if (os_strncasecmp(cmd, "POWERMODE ", 10) == 0) {
		int state;

		state = atoi(cmd + 10);
		ret = wpa_driver_set_power_save(priv, state);
		if (ret < 0) {
			wpa_driver_send_hang_msg(drv);
		} else {
			drv_errors = 0;
			ret = 0;
		}
		return ret;
	} else if (os_strncasecmp(cmd, "GETPOWER", 8) == 0) {
		int state = -1;

		ret = wpa_driver_get_power_save(priv, &state);
		if (!ret && (state != -1)) {
			ret = os_snprintf(buf, buf_len, "POWERMODE = %d\n", state);
			drv_errors = 0;
		} else {
			wpa_driver_send_hang_msg(drv);
		}
		return ret;
	}
#ifdef BCM_SDO
		else if (os_strncasecmp(cmd, "P2P_SD_REQ", 10) == 0) {
#ifdef BCM_MAP_SDCMDS_2_SDOCMDS
			wpa_driver_sdo_req(cmd);
#endif
		} else if ((os_strncasecmp(cmd, "P2P_SD_FIND", 11) == 0) ||
			(os_strncasecmp(cmd, "P2P_SD_LISTEN", 13) == 0)) {
			u8 channel = 0;
			char tmp[250] = {0};
			channel = wpas_get_listen_channel(drv->ctx);
			os_snprintf(tmp, sizeof(tmp), "%s%s%d", cmd, " chan=", channel);
			os_snprintf(cmd, sizeof(tmp), "%s", tmp);
			wpa_printf(MSG_DEBUG, "cmd:%s\n", cmd);
            wpas_set_probe_resp_ies(drv->ctx);
		}
#endif /* BCM_SDO */

		if (os_strcasecmp(cmd, "BGSCAN-START") == 0) {
			ret = wpa_driver_set_backgroundscan_params(priv);
			if (ret < 0) {
				return ret;
			}
			os_memcpy(buf, "PNOFORCE 1", 11);
		} else if (os_strcasecmp(cmd, "BGSCAN-STOP") == 0) {
			os_memcpy(buf, "PNOFORCE 0", 11);
#ifdef BRCM_DRV_ROAM
		} else if (os_strcasecmp(cmd, "SET_ROAMPREF") == 0) {
			ret = wpa_driver_set_roampref(priv);
			return ret;
#endif
		} else {
			os_memcpy(buf, cmd, strlen(cmd) + 1);
		}
		memset(&ifr, 0, sizeof(ifr));
		memset(&priv_cmd, 0, sizeof(priv_cmd));
		os_strncpy(ifr.ifr_name, bss->ifname, IFNAMSIZ);

		priv_cmd.buf = buf;
		priv_cmd.used_len = buf_len;
		priv_cmd.total_len = buf_len;
		ifr.ifr_data = (void *)&priv_cmd;

		if ((ret = ioctl(drv->global->ioctl_sock, SIOCDEVPRIVATE + 1, &ifr)) < 0) {
			wpa_printf(MSG_ERROR, "%s: failed to issue private commands ret = %d\n", __func__,ret);
			wpa_driver_send_hang_msg(drv);
		} else {
			drv_errors = 0;
			ret = 0;
			if ((os_strncasecmp(cmd, "LINKSPEED", 9) == 0) ||
				(os_strncasecmp(cmd, "RSSI", 4) == 0) ||
				(os_strcasecmp(cmd, "GETBAND") == 0) ||
				(os_strcasecmp(cmd, "SETCSA") == 0) ||
				(os_strcasecmp(cmd, "CHANSPEC") == 0) ||
				(os_strncasecmp(cmd, "MODE", 4) == 0) ||
				(os_strncasecmp(cmd, "DATARATE", 8) == 0) ||
				(os_strcasecmp(cmd, "ASSOCLIST") == 0) ||
				(os_strncasecmp(cmd, "NAN_STATUS", 10) == 0))
				ret = strlen(buf);
			else if (os_strncasecmp(cmd, "COUNTRY", 7) == 0) {
				wpa_supplicant_event(drv->ctx, EVENT_CHANNEL_LIST_CHANGED,
				     NULL);
#if !defined(HOSTAPD) && defined(BCM_SDO)
			} else if ((os_strncasecmp(cmd, "P2P_SD_FIND", 11) == 0) ||
				(os_strncasecmp(cmd, "P2P_SD_LISTEN", 13) == 0)) {
				 wpas_enable_dd_offload(drv->ctx, 1);
			} else if ((os_strncasecmp(cmd, "P2P_SD_STOP", 11) == 0)){
				 wpas_enable_dd_offload(drv->ctx, 0);
#endif /* !HOSTAPD && BCM_SDO */
			} else if (os_strcasecmp(cmd, "GET_BEST_CHANNELS") == 0) {
				wpa_get_best_channels(drv->ctx, buf);
			} else if (os_strncasecmp(cmd, "SETBAND", 7) == 0) {
				/* private command band needs to update Channel list */
				wpa_supplicant_event(drv->ctx, EVENT_CHANNEL_LIST_CHANGED,
				     NULL);
			}
			if (os_strncasecmp(cmd, "WLS_BATCHING GET", 16) == 0) {
				token = strtok(buf, "\n");
				while (token != NULL) {
					wpa_printf(MSG_DEBUG, "%s", token);
					token = strtok(NULL, "\n");
				}
			} else
				wpa_printf(MSG_DEBUG, "%s %s len = %d, %d", __func__, buf, ret, strlen(buf));
		}
	return ret;
}

int wpa_driver_set_p2p_noa(void *priv, u8 count, int start, int duration)
{
	char buf[MAX_DRV_CMD_SIZE];

	memset(buf, 0, sizeof(buf));
	wpa_printf(MSG_DEBUG, "%s: Entry", __func__);
	snprintf(buf, sizeof(buf), "P2P_SET_NOA %d %d %d", count, start, duration);
	return wpa_driver_nl80211_driver_cmd(priv, buf, buf, strlen(buf)+1);
}

int wpa_driver_get_p2p_noa(void *priv, u8 *buf, size_t len)
{
	/* Return 0 till we handle p2p_presence request completely in the driver */
	return 0;
}

int wpa_driver_set_p2p_ps(void *priv, int legacy_ps, int opp_ps, int ctwindow)
{
	char buf[MAX_DRV_CMD_SIZE];

	memset(buf, 0, sizeof(buf));
	wpa_printf(MSG_DEBUG, "%s: Entry", __func__);
	snprintf(buf, sizeof(buf), "P2P_SET_PS %d %d %d", legacy_ps, opp_ps, ctwindow);
	return wpa_driver_nl80211_driver_cmd(priv, buf, buf, strlen(buf) + 1);
}

int wpa_driver_set_ap_wps_p2p_ie(void *priv, const struct wpabuf *beacon,
				 const struct wpabuf *proberesp,
				 const struct wpabuf *assocresp)
{
	char buf[MAX_WPSP2PIE_CMD_SIZE];
	struct wpabuf *ap_wps_p2p_ie = NULL;
	char *_cmd = "SET_AP_WPS_P2P_IE";
	char *pbuf;
	int ret = 0;
	int i;
	struct cmd_desc {
		int cmd;
		const struct wpabuf *src;
	} cmd_arr[] = {
		{0x1, beacon},
		{0x2, proberesp},
		{0x4, assocresp},
		{-1, NULL}
	};

	wpa_printf(MSG_DEBUG, "%s: Entry", __func__);

	for (i = 0; cmd_arr[i].cmd != -1; i++) {
		os_memset(buf, 0, sizeof(buf));
		pbuf = buf;
		pbuf += sprintf(pbuf, "%s %d", _cmd, cmd_arr[i].cmd);
		*pbuf++ = '\0';
		ap_wps_p2p_ie = cmd_arr[i].src ?
			wpabuf_dup(cmd_arr[i].src) : NULL;
		if (ap_wps_p2p_ie) {
			os_memcpy(pbuf, wpabuf_head(ap_wps_p2p_ie), wpabuf_len(ap_wps_p2p_ie));
			ret = wpa_driver_nl80211_driver_cmd(priv, buf, buf,
				strlen(_cmd) + 3 + wpabuf_len(ap_wps_p2p_ie));
			wpabuf_free(ap_wps_p2p_ie);
			if (ret < 0)
				break;
		}
	}

	return ret;
}

/*
 * This will get call per interface. So any interface
 * specific functionality can be handled here. Any generic
 * or single time intialization has to be appropriately handled
 * so that it doesn't happen mulitple times.
 */
int wpa_driver_priv_lib_init(struct i802_bss *bss)
{
	struct wpa_driver_nl80211_data *drv = bss->drv;
	int ret;

	if (g_privlib_initialized)
		return 0;

	wpa_printf(MSG_INFO, "libbcmdhd: priv_lib_init");

	/* Non-interface specific/One time initilization Funcs should go below */

#ifdef BCM_GENL
	if((ret = wpa_driver_register_genl(bss)) < 0) {
		/* GENL init failed. Driver probably doesn't have the GENL support
		 * But don't fail the supplicant/hostapd initialization.
		 */
		wpa_printf(MSG_ERROR, "libbcmdhd: GENL will not be enabled");
	}
#endif /* BCM_GENL */

#if defined(BCM_GENL) && defined(BCM_GENL_UNICAST)
	if((ret = wpa_driver_set_supplicant_pid(bss)) < 0) {
		wpa_printf(MSG_ERROR, "libbcmdhd: GENL will not be enabled");
	}
#endif /* BCM_GENL && BCM_GENL_UNICAST */

	/* Mark as Initialized */
	g_privlib_initialized = 1;

    return 0;
}

int wpa_driver_priv_lib_deinit(struct i802_bss *bss)
{
	struct wpa_driver_nl80211_data *drv = bss->drv;
	int ret;
	static int deinitialized = 0;

	if (deinitialized) {
		wpa_printf(MSG_INFO, "libbcmdhd: already deinitalized. Returning");
		return 0;
	}

	/* Non-Interface specific/One time deinitialization routines
	 * should happen below. The below part would be only invoked once.
	 */

	wpa_printf(MSG_INFO, "libbcmdhd: priv_lib_deinit");

#ifdef BCM_GENL
	wpa_driver_unregister_genl(bss);
#endif /* BCM_GENL */

	deinitialized = 1;

    return 0;
}

#ifdef BCM_GENL /* GENERIC NETLINK RECEIVE PATH */
static int no_seq_check(struct nl_msg *msg, void *arg)
{
	return NL_OK;
}

static void wpa_driver_recv_genl(int sock, void *eloop_ctx, void *handle)
{
	 struct wpa_driver_nl80211_data *drv = eloop_ctx;

        wpa_printf(MSG_ERROR, "BCM-GENL event available");

	if(drv->event_sock)
			nl_recvmsgs(drv->event_sock, nl_socket_get_cb(drv->event_sock));
	else
			wpa_printf(MSG_ERROR, "BCM-GENL event on wrong driver interface");
}

static int wpa_driver_handle_svc_found(struct wpa_driver_nl80211_data *drv, char *data, u32 len)
{
	u16	frag_no = 0;
	u16 more_frag = 0;
	sdo_hdr_t *hdr;
	char *proto = NULL;
	char *data_ptr = data + sizeof(sdo_hdr_t);
	u8 first_frag = 0;
	sd_tlv_t *tlv_hdr;
	u8 *tlv_end = NULL;
	union wpa_event_data event;
	s32 tot_len = len - sizeof(sdo_hdr_t);
	int tlv_num = 0;

	if (!data || tot_len <= 0) {
		wpa_printf(MSG_ERROR, "BCM-GENL [SERVICE-FOUND] Invalid length");
		return -1;
	}

	tot_len = len - sizeof(sdo_hdr_t);
	bzero(&event, sizeof(union wpa_event_data));
	hdr = (sdo_hdr_t *)data;
	tlv_hdr = (sd_tlv_t *) (data + sizeof(sdo_hdr_t));

	if (!hdr || !tlv_hdr) {
		wpa_printf(MSG_ERROR, "BCM-GENL [SERVICE-FOUND] Invalid header");
		return -1;
	}
#if BCM_SDIO
	event.p2p_sd_resp.sa =  hdr->addr;
	event.p2p_sd_resp.update_indic =  hdr->update_ind;
	event.p2p_sd_resp.tlvs = (const u8 *)tlv_hdr;
#ifdef BCM_MAP_SDCMDS_2_SDOCMDS
	/* Response should contains 5 bytes and we need not to handle the case of NULL
	 * response so check if lenth is 5
	 */
	if (tot_len != 5) {
		/*We are adding two in tlv_hdr to add transaction id and proto*/
		wpa_printf(MSG_DEBUG, "hdr->update_ind = %d tlv_num = %d", hdr->update_ind, tlv_num);
		event.p2p_sd_resp.tlvs_len = tlv_hdr->length + 2;
	}
#else
	/* This is temperory code for Testing purpose. Will be removed. */
	tlv_end = (u8 *)tlv_hdr + tot_len;
	while (((u8 *)tlv_hdr < tlv_end) && (tlv_num++ < hdr->count)) {
		wpa_printf(MSG_DEBUG, "BCM-GENL [SERVICE-FOUND] "MACSTR" tlv:%d %s",
				MAC2STR(hdr->addr), tlv_num, tlv_hdr->data);
		if (tlv_hdr->protocol == SVC_RPOTYPE_UPNP) {
			/* Upnp version is in binary format */
			u16 version = tlv_hdr->data[0];
			wpa_msg(drv->ctx, MSG_INFO, P2P_EVENT_SERV_DISC_RESP MACSTR" %u 0x%x%s",
				MAC2STR(hdr->addr), hdr->freq, version,(char *)(tlv_hdr->data + 1));
		} else {
			wpa_msg(drv->ctx, MSG_INFO, P2P_EVENT_SERV_DISC_RESP MACSTR" %u %s",
				MAC2STR(hdr->addr), hdr->freq, (char *)tlv_hdr->data);
		}
		tlv_hdr = (sd_tlv_t *)(tlv_hdr->data + tlv_hdr->length);
	}
#endif
	wpa_supplicant_event(drv->ctx, EVENT_P2P_SD_RESPONSE, &event);
#endif /* BCM_SDO */

	return 0;
}

static int wpa_driver_handle_attr_data(struct wpa_driver_nl80211_data *drv,
					char *data, unsigned int len)
{
	bcm_hdr_t *hdr;
	u8 *addr = NULL;

	if(len <= sizeof(bcm_hdr_t))
		return -EINVAL;


	hdr = (bcm_hdr_t *)data;

	wpa_printf(MSG_DEBUG, "BCM-GENL event_type:%x event_len: %x", hdr->type, hdr->len);

	wpa_hexdump(MSG_DEBUG, "Event_data dump:", (const u8 *)data, len);

	switch (hdr->type) {
#ifndef HOSTAPD
#ifdef BCM_SDO
		case BCM_EVENT_SVC_FOUND:
			{
				wpa_printf(MSG_DEBUG, "BCM-GENL [SERVICE-FOUND]");
				if (wpa_driver_handle_svc_found(drv,
							((char *)data + sizeof(bcm_hdr_t)), hdr->len) < 0)
					return -1;
				break;
			}
		case BCM_EVENT_DEV_FOUND:
			{
				struct wpa_scan_results scan_res;
				struct wpa_scan_res *bss;
				u16 tot_len = 0;
				u8 *ptr = NULL;
				bcm_dev_info_t *info = (bcm_dev_info_t *)((u8 *)data +
							sizeof(bcm_hdr_t));

				wpa_printf(MSG_DEBUG, "BCM-GENL [DEV-FOUND] band:%x Freq %d"
					" rssi:%d ie_len:%x Mac:"MACSTR"\n",
					info->band, info->freq, info->rssi,
					 info->ie_len, MAC2STR(info->bssid));

				if (info->ie_len > 2000) {
					wpa_printf(MSG_ERROR, "Invalid IE_LEN");
					return -EINVAL;
				}

				tot_len = sizeof(struct wpa_scan_res) + info->ie_len;
				ptr = malloc(tot_len);
				if(!ptr) {
					wpa_printf(MSG_ERROR, "Malloc failed");
					return -ENOMEM;
				}
				memset(ptr, 0, tot_len);

				bss = (struct wpa_scan_res *)ptr;
				memcpy(bss->bssid, info->bssid, ETH_ALEN);
				bss->freq = info->freq;
				bss->level = info->rssi;
				bss->ie_len = info->ie_len;

				/* Copy the IE's immeditately after the wpas_scan_res memory */
				memcpy(ptr+sizeof(struct wpa_scan_res),
					data+sizeof(bcm_hdr_t)+sizeof(bcm_dev_info_t),
					info->ie_len);

				scan_res.res = &bss;
				scan_res.num = 1;

				wpas_p2p_scan_res_handler(drv->ctx, &scan_res);

				if(ptr)
					free(ptr);
				break;
			}

		case BCM_EVENT_DEV_LOST:
			{
				if(hdr->len != 6) {
					wpa_printf(MSG_ERROR, "DEV_LOST: invalid data"
						" (hdr_len != ETH_ALEN)");
					return -EINVAL;

				}

				wpa_printf(MSG_DEBUG, "BCM-GENL [DEV-LOST] "MACSTR,
					MAC2STR((u8 *)data + sizeof(bcm_hdr_t)));
				wpas_expire_p2p_peer(drv->ctx, ((u8 *)data + sizeof(bcm_hdr_t)));
				break;
			}
#endif /* BCM_SDO */
#endif /* HOSTAPD */

		default:
			wpa_printf(MSG_ERROR, "UNKNOWN Event Msg Type:%d len:%d", hdr->type, hdr->len);
			break;
	}

	return 0;
}

static int wpa_driver_handle_genl_event(struct nl_msg *msg, void *arg)
{
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *attrs[BCM_GENL_ATTR_MAX + 1];
	struct wpa_driver_nl80211_data *drv = (struct wpa_driver_nl80211_data *)arg;

	wpa_printf(MSG_DEBUG, "%s: Enter", __func__);

	if (nla_parse(attrs, BCM_GENL_ATTR_MAX,
		genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL) < 0) {
		wpa_printf(MSG_ERROR, "GENL msg parse failed");
		return -1;
	}

	if(attrs[BCM_GENL_ATTR_STRING]) {
		wpa_printf(MSG_ERROR, "BCM-GENL ATTR_STRING FOUND. Dumping the string");
		wpa_msg(drv->ctx, MSG_INFO, "%s", (char *)nla_data(attrs[BCM_GENL_ATTR_STRING]));
	} else if(attrs[BCM_GENL_ATTR_MSG]) {
		wpa_printf(MSG_ERROR, "BCM-GENL ATTR_MSG FOUND. Calling the handler");
		wpa_driver_handle_attr_data(drv, (char *)nla_data(attrs[BCM_GENL_ATTR_MSG]),
		nla_len(attrs[BCM_GENL_ATTR_MSG]));
	} else
		wpa_printf(MSG_ERROR, "BCM-GENL NOT Present");

	return NL_SKIP;

}

static int driver_genl_ctrl_resolve(struct nl_sock *handle,
                                     const char *name)
{
        struct nl_cache *cache = NULL;
        struct genl_family *nl80211 = NULL;
        int id = -1;

        if (genl_ctrl_alloc_cache(handle, &cache) < 0) {
                wpa_printf(MSG_ERROR, "nl80211: Failed to allocate generic "
                           "netlink cache");
                goto fail;
        }

        nl80211 = genl_ctrl_search_by_name(cache, name);
        if (nl80211 == NULL)
                goto fail;

        id = genl_family_get_id(nl80211);

	wpa_printf(MSG_ERROR, "Family id:%d", id);

fail:
        if (nl80211)
                genl_family_put(nl80211);
        if (cache)
                nl_cache_free(cache);

        return id;
}

int driver_send_msg(void *priv, int len, const void *data)
{
        struct i802_bss *bss = priv;
        struct wpa_driver_nl80211_data *drv = bss->drv;
	struct nl_msg *msg;

 	msg = nlmsg_alloc();
	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, drv->event_family, 0, NLM_F_REQUEST,
             	BCM_GENL_CMD_MSG, 1);

 	nla_put(msg, BCM_GENL_ATTR_MSG, (size_t)len, data);

 	// Send message over netlink socket
 	nl_send_auto_complete(drv->event_sock, msg);

 	nlmsg_free(msg);

	return 0;
}
static int wpa_driver_register_genl_multicast(void *priv)
{
	struct i802_bss *bss = priv;
	struct wpa_driver_nl80211_data *drv = bss->drv;
	int mcast_id = 0;

	mcast_id = driver_get_multicast_id(priv,
                               "bcm-genl", "bcm-genl-mcast");

	if (mcast_id >= 0) {
                if(nl_socket_add_membership(drv->event_sock, mcast_id) < 0) {
			wpa_printf(MSG_ERROR, "%s: MULTICAST ID"
				" add membership failed\n", __func__);
			return -1;
		}

	}

	return mcast_id;

}

int wpa_driver_register_genl(void *priv)
{
	struct i802_bss *bss = priv;
	struct wpa_driver_nl80211_data *drv = bss->drv;
	int ret = 0;
	static int family;
	struct sockaddr_nl src_addr;

	wpa_printf(MSG_DEBUG, "%s: Enter  (PID: %d)", __func__, getpid());

	if(drv->event_sock && drv->event_family) {
		wpa_printf(MSG_ERROR, "GENL Already registered/Initialized");
		return 0;
	}

	drv->event_sock = nl_socket_alloc();
	if(!drv->event_sock) {
		wpa_printf(MSG_ERROR, "nl_socket_alloc failed");
		return -1;
	}

	if(genl_connect(drv->event_sock) < 0) {
		wpa_printf(MSG_ERROR, "genl_connect failed");
		ret = -1;
		goto fail;
	}

	drv->event_family = driver_genl_ctrl_resolve(drv->event_sock, "bcm-genl");
	if(drv->event_family < 0) {
		wpa_printf(MSG_ERROR, "genl_ctrl_resolve failed ret:%d", drv->event_family);
		ret = -1;
		goto fail;
	}

	nl_cb_set(nl_socket_get_cb(drv->event_sock),
		 NL_CB_SEQ_CHECK, NL_CB_CUSTOM, no_seq_check, drv);
	nl_cb_set(nl_socket_get_cb(drv->event_sock),
		 NL_CB_VALID, NL_CB_CUSTOM, wpa_driver_handle_genl_event, drv);

	if (wpa_driver_register_genl_multicast(priv) < 0) {
		wpa_printf(MSG_ERROR, "genl_multicast register failed");
		ret = -1;
		goto fail;
	}

	eloop_register_read_sock(nl_socket_get_fd(drv->event_sock),
                                 wpa_driver_recv_genl, drv, NULL);

	wpa_printf(MSG_INFO, "libbcmdhd: GENL Interface enabled");

	return 0;

fail:
	wpa_printf(MSG_ERROR, "%s: Failed. GENL register failed", __func__);

	if(drv->event_sock)
		nl_socket_free(drv->event_sock);
	drv->event_sock = 0;
	drv->event_family = 0;
	return ret;

}

int wpa_driver_unregister_genl(void *priv)
{
	struct i802_bss *bss = priv;
	struct wpa_driver_nl80211_data *drv = NULL;

	if (bss)
		drv = bss->drv;

	if(!drv || !drv->event_sock || !drv->event_family) {
		wpa_printf(MSG_ERROR, " No socket initialized on this interface");
		return -1;
	}

	eloop_unregister_read_sock(nl_socket_get_fd(drv->event_sock));

	nl_socket_free(drv->event_sock);
	drv->event_sock = NULL;

	wpa_printf(MSG_INFO, "libbcmdhd: GENL Interface disabled");

	return 0;
}

static int family_handler(struct nl_msg *msg, void *arg)
{
     struct family_data *res = arg;
     struct nlattr *tb[CTRL_ATTR_MAX + 1];
     struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
     struct nlattr *mcgrp;
     int i;

     nla_parse(tb, CTRL_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
           genlmsg_attrlen(gnlh, 0), NULL);
     if (!tb[CTRL_ATTR_MCAST_GROUPS])
         return NL_SKIP;

     nla_for_each_nested(mcgrp, tb[CTRL_ATTR_MCAST_GROUPS], i) {
         struct nlattr *tb2[CTRL_ATTR_MCAST_GRP_MAX + 1];
         nla_parse(tb2, CTRL_ATTR_MCAST_GRP_MAX, nla_data(mcgrp),
               nla_len(mcgrp), NULL);
         if (!tb2[CTRL_ATTR_MCAST_GRP_NAME] ||
             !tb2[CTRL_ATTR_MCAST_GRP_ID] ||
             os_strncmp(nla_data(tb2[CTRL_ATTR_MCAST_GRP_NAME]),
                    res->group,
                    nla_len(tb2[CTRL_ATTR_MCAST_GRP_NAME])) != 0)
             continue;
         res->id = nla_get_u32(tb2[CTRL_ATTR_MCAST_GRP_ID]);
         break;
     };

     return NL_SKIP;
}


int driver_get_multicast_id(void *priv,
                               const char *family, const char *group)
{
        struct i802_bss *bss = priv;
        struct wpa_driver_nl80211_data *drv = bss->drv;
        struct nl_msg *msg;
        int ret = -1;
        struct family_data res = { group, -ENOENT };

        msg = nlmsg_alloc();
        if (!msg)
                return -ENOMEM;
        genlmsg_put(msg, 0, 0, genl_ctrl_resolve((struct nl_sock *)drv->global->nl, "nlctrl"),
                    0, 0, CTRL_CMD_GETFAMILY, 0);
        NLA_PUT_STRING(msg, CTRL_ATTR_FAMILY_NAME, family);

        ret = send_and_recv_msgs(drv, msg, family_handler, &res);
        msg = NULL;
        if (ret == 0) {
		wpa_printf(MSG_ERROR, "multicastid: %d", res.id);
                ret = res.id;
	} else
		wpa_printf(MSG_ERROR, "sendmsg returned %d", ret);

nla_put_failure:
        nlmsg_free(msg);
        return ret;
}

#endif  /* BCM_GENL */
