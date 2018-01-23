/** @file wapi_asue_i.h
 *  @brief This header file contains data structures and function declarations of wapi_asue_i
 *
 *  Copyright (c) 2003-2005, Jouni Malinen <jkmaline@cc.hut.fi>
 *  Copyright (C) 2001-2008, Iwncomm Ltd.
 */

#include "drivers/driver.h"
#include "common.h"
#include "wpa_supplicant_i.h"

/* driver_ops */

static inline int wapi_drv_set_wapi(struct wpa_supplicant *wpa_s, int enabled)
{
	if (wpa_s->driver->set_wapi)
	{
		int ret = -1;
		ret = wpa_s->driver->set_wapi(wpa_s->drv_priv, enabled);
		return ret;
	}
	return -1;
}

static inline int wapi_drv_set_key(struct wpa_supplicant *wpa_s, enum wpa_alg alg,
				   const u8 *addr, int key_idx, int set_tx,
				   const u8 *seq, size_t seq_len,
				   const u8 *key, size_t key_len)
{
	if (wpa_s->driver->set_key) {
		return wpa_s->driver->set_key(wpa_s->ifname, wpa_s->drv_priv, alg, addr,
					      key_idx, set_tx, seq, seq_len,
					      key, key_len);
	}
	return -1;
}

static inline int wapi_drv_set_wpa_ie(struct wpa_supplicant *wpa_s, const u8 *wpa_ie, size_t wpa_ie_len)
{
	if (wpa_s->driver->set_wpa_ie) {
		return wpa_s->driver->set_wpa_ie(wpa_s->drv_priv, wpa_ie, wpa_ie_len);
	}
	return -1;
}
