/*
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * Author : junlim
 */

#ifndef WPA_CCKM_H
#define WPA_CCKM_H

#include "defs.h"
#include "wpa_i.h"

int wpa_cckm_reassoc_process(struct wpa_sm *sm, int driver_4way_handshake);
int wpa_cckm_reassoc_is_completed(struct wpa_sm *sm);
int wpa_generate_krk_btk_cckm(struct wpa_sm *sm, const u8 *anonce);
void wpa_derive_ptk_cckm(struct wpa_sm *sm, const u8* bssid, u8 *ptk, size_t ptk_len);

#endif /* WPA_CCKM_H */
