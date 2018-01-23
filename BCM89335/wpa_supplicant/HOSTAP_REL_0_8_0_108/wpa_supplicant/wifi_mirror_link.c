/*
 * wpa_supplicant - MIRROR LINK 
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "includes.h"

#include "common.h"
#include "p2p/p2p.h"
#include "common/ieee802_11_defs.h"
#include "wpa_supplicant_i.h"
#include "wifi_display.h"
#include "wifi_mirror_link.h"


int wifi_mirror_link_init(struct wpa_global *global)
{
	global->mirror_link = 1;
	return 0;
}

void  wifi_mirror_link_deinit(struct wpa_global *global)
{
	int i;
	for (i = 0; i < MAX_MIRRORLINK_SUBELEMS; i++) {
		wpabuf_free(global->mirrlnk_subelems[i]);
		global->mirrlnk_subelems[i] = NULL;
	}
	global->mirror_link = 0;
}
struct wpabuf* wifi_mrlnk_encaps(struct wpabuf *subelems)
{
	struct wpabuf *ie;
	const u8 *pos, *end;

	if (subelems == NULL)
		return NULL;

	ie = wpabuf_alloc(wpabuf_len(subelems) + 100);
	if (ie == NULL)
		return NULL;

	pos = wpabuf_head(subelems);
	end = pos + wpabuf_len(subelems);
	wpa_printf(MSG_DEBUG, "swa:pos:%s\n", pos);

	while (end > pos) {
		size_t frag_len = end - pos;
	wpa_printf(MSG_DEBUG, "swa:frag_len:%d\n", frag_len);
		if (frag_len > 251)
			frag_len = 251;
		wpabuf_put_u8(ie, WLAN_EID_VENDOR_SPECIFIC);
		wpabuf_put_u8(ie, 4 + frag_len);
		wpabuf_put_be32(ie, MIRRORLNK_IE_VENDOR_TYPE);
		wpabuf_put_data(ie, pos, frag_len);
		pos += frag_len;
	}

	return ie;
}


int wifi_mirror_link_update_subelem(struct wpa_global *global)
{
	struct wpabuf *ie, *buf;
	size_t len;

	if (global->p2p == NULL)
		return 0;

	wpa_printf(MSG_DEBUG, "MIRLNK: Update MIRR IE");

	if (!global->mirror_link) {
		wpa_printf(MSG_DEBUG, "MIRLNK: Mirror link is disabled -not include ie\n");
		p2p_set_mirror_link_ie(global->p2p, NULL);
		return 0;
	}
	p2p_set_mirror_link_upnp_subelem(global->p2p, global->mirrlnk_subelems[MRLNK_SUBELEM_UPNP]);
	p2p_set_mirror_link_internet_access_subelem(global->p2p, global->mirrlnk_subelems[MRLNK_SUBELEM_INTERNET_ACCESS]);

	len = 0;
	if (global->mirrlnk_subelems[MRLNK_SUBELEM_UPNP]) {
		len += wpabuf_len(global->mirrlnk_subelems[MRLNK_SUBELEM_UPNP]);
		wpa_printf(MSG_DEBUG, "upnp ie length%d\n", len);
	}
	if (global->mirrlnk_subelems[MRLNK_SUBELEM_INTERNET_ACCESS]) {
		len += wpabuf_len(global->mirrlnk_subelems[MRLNK_SUBELEM_INTERNET_ACCESS]);
		wpa_printf(MSG_DEBUG, "internet ie length%d\n", len);
	}

	buf = wpabuf_alloc(len);
	if (buf == NULL)
		return -1;
	if (global->mirrlnk_subelems[MRLNK_SUBELEM_UPNP]) {
		wpa_hexdump_buf(MSG_DEBUG, "upnp subelem IE", global->mirrlnk_subelems[MRLNK_SUBELEM_UPNP]);
		wpabuf_put_buf(buf, global->mirrlnk_subelems[MRLNK_SUBELEM_UPNP]);
	}
	if (global->mirrlnk_subelems[MRLNK_SUBELEM_INTERNET_ACCESS]) {
		wpa_hexdump_buf(MSG_DEBUG, "internet subelem IE", global->mirrlnk_subelems[MRLNK_SUBELEM_INTERNET_ACCESS]);
		wpabuf_put_buf(buf, global->mirrlnk_subelems[MRLNK_SUBELEM_INTERNET_ACCESS]);
	}


	ie = wifi_mrlnk_encaps(buf);
	wpa_hexdump_buf(MSG_DEBUG, "MRLNK:MRLNK IE", ie);
	p2p_set_mirror_link_ie(global->p2p, ie);

	wpabuf_free(buf);
	return 0;


}
static void wifi_mirror_link_del(struct wpa_global *global, int subelem)
{
	struct wpabuf *ie, *buf;
	size_t len = 0;

	if (subelem == MRLNK_SUBELEM_UPNP) {
		p2p_set_mirror_link_upnp_subelem(global->p2p, NULL);
		wpabuf_free(global->mirrlnk_subelems[MRLNK_SUBELEM_UPNP]);
		global->mirrlnk_subelems[MRLNK_SUBELEM_UPNP] = NULL;
	}
	else if (subelem == MRLNK_SUBELEM_INTERNET_ACCESS) { 
		p2p_set_mirror_link_internet_access_subelem(global->p2p, NULL);
		wpabuf_free(global->mirrlnk_subelems[MRLNK_SUBELEM_INTERNET_ACCESS]);
		global->mirrlnk_subelems[MRLNK_SUBELEM_INTERNET_ACCESS] = NULL;
	}
		
	if (global->mirrlnk_subelems[MRLNK_SUBELEM_UPNP]) {
		len += wpabuf_len(global->mirrlnk_subelems[MRLNK_SUBELEM_UPNP]);
		wpa_printf(MSG_DEBUG, "upnp ie length%d\n", len);
	}
	if (global->mirrlnk_subelems[MRLNK_SUBELEM_INTERNET_ACCESS]) {
		len += wpabuf_len(global->mirrlnk_subelems[MRLNK_SUBELEM_INTERNET_ACCESS]);
		wpa_printf(MSG_DEBUG, "internet ie length%d\n", len);
	}

	buf = wpabuf_alloc(len);
	if (buf == NULL)
		return;
	if (global->mirrlnk_subelems[MRLNK_SUBELEM_UPNP]) {
		wpa_hexdump_buf(MSG_DEBUG, "upnp subelem IE", global->mirrlnk_subelems[MRLNK_SUBELEM_UPNP]);
		wpabuf_put_buf(buf, global->mirrlnk_subelems[MRLNK_SUBELEM_UPNP]);
	}
	if (global->mirrlnk_subelems[MRLNK_SUBELEM_INTERNET_ACCESS]) {
		wpa_hexdump_buf(MSG_DEBUG, "internet subelem IE", global->mirrlnk_subelems[MRLNK_SUBELEM_INTERNET_ACCESS]);
		wpabuf_put_buf(buf, global->mirrlnk_subelems[MRLNK_SUBELEM_INTERNET_ACCESS]);
	}


	ie = wifi_mrlnk_encaps(buf);
	wpa_hexdump_buf(MSG_DEBUG, "MRLNK:MRLNK IE", ie);
	p2p_set_mirror_link_ie(global->p2p, ie);

	wpabuf_free(buf);
}
int wifi_mirror_link_ie_set(struct wpa_global *global, char *cmd)
{
	char *pos = NULL;
	int subelem = 0;
	size_t len = 0;
	struct wpabuf *e = NULL;
	
	wpa_printf(MSG_DEBUG, "%s entry cmd:%s\n", __FUNCTION__, cmd);

	if (!global->wifi_display) {
		wpa_printf(MSG_ERROR, "%s:wifi_display is not enabled\n", __FUNCTION__);
		return -1;
	}
	pos = cmd;
	pos = os_strchr(cmd, ' ');
	pos++;
	if (pos == NULL) {
		wpa_printf(MSG_ERROR, "%s pos is null\n", __FUNCTION__);
		return -1;
	}


	/* extract subelem id */
	subelem = atoi(pos);

	if (os_strncmp(cmd, "del", 3) == 0) {
		wifi_mirror_link_del(global, subelem);
		wpa_printf(MSG_ERROR, "%s delete mirrorlink ie\n", __FUNCTION__);
		return 0;
	}
	pos = os_strchr(cmd, ' ');
	*pos++ = '\0';
	if (pos++ == NULL) {
		wpa_printf(MSG_ERROR, "[swa]:%s pos is null\n", __FUNCTION__);
		return -1;
	}

	if (subelem < 0 || subelem >= MAX_MIRRORLINK_SUBELEMS) {
		wpa_printf(MSG_ERROR, "%s: invalid subelem :%d\n", __FUNCTION__, subelem);
		return -1;
	}
	/* length of ie */
	pos++;
	wpa_printf(MSG_DEBUG, "swa:%s\n", pos);
	len = os_strlen(pos);
	if (len & 1) {
		wpa_printf(MSG_ERROR, "[swa]:%s len is incorrect:%d\n", __FUNCTION__, len);
		return -1;
	}
	
	len /= 2;

	if (len == 0) {
		/* Clear subelement */
		e = NULL;
		wpa_printf(MSG_DEBUG, "WFD: Clear subelement %d", subelem);
	}

	e = wpabuf_alloc(1 + len);
	if (e == NULL) {
		wpa_printf(MSG_DEBUG, "[swa]:%s mem alloc failed\n", __FUNCTION__);
		return -1;
	}
	if (hexstr2bin(pos, wpabuf_put(e, len), len) < 0) {
		wpa_printf(MSG_DEBUG, "[swa]:%s hex2string failed\n", __FUNCTION__);
		wpabuf_free(e);
		return -1;
	}
	wpa_printf(MSG_DEBUG, "[swa]: Mirror link IE Set %s subelem:%d", e->buf, subelem);

	wpabuf_free(global->mirrlnk_subelems[subelem]);
	global->mirrlnk_subelems[subelem] = e;
	wifi_mirror_link_update_subelem(global);
	return 0;

}
/** Extarct mirror link subelems
**/
char * wifi_mirror_link_subelem_hex(const struct wpabuf *mrlnk_subelems)
{
	char *subelem = NULL;
	const u8 *buf;
	size_t buflen;

	if (!mrlnk_subelems)
		return NULL;

	buf = wpabuf_head_u8(mrlnk_subelems);
	if (!buf)
		return NULL;
	wpa_printf(MSG_DEBUG, "%s:mirr_ie:%s\n",__FUNCTION__, buf);

	buflen = wpabuf_len(mrlnk_subelems);
	wpa_printf(MSG_DEBUG, "%s:buflen:%d\n", __FUNCTION__, buflen);

	subelem = os_zalloc(buflen*2 + 1);
	if (!subelem)
		return NULL;
	wpa_snprintf_hex(subelem, (buflen*2+1), buf, (buflen*2+1));
	return subelem;
}
