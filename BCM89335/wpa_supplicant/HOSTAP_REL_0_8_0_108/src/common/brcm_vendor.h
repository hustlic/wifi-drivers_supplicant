
#ifndef BRCM_VENDOR_H
#define BRCM_VENDOR_H

/*
 * This file is a registry of identifier assignments from the Broadcom
 * OUI 00:10:18 for purposes other than MAC address assignment. New identifiers
 * can be assigned through normal review process for changes to the upstream
 * hostap.git repository.
 */

#define OUI_BRCM  0x001018

/**
 * enum brcm_nl80211_vendor_subcmds - BRCM nl80211 vendor command identifiers
 *
 * @BRCM_VENDOR_SUBCMD_UNSPEC: Reserved value 0
 *
 * @BRCM_VENDOR_SUBCMD_PRIV_STR: String command/event
 */
enum brcm_nl80211_vendor_subcmds {
        BRCM_VENDOR_SUBCMD_UNSPEC,
        BRCM_VENDOR_SUBCMD_PRIV_STR,
};

/**
 * enum brcm_nl80211_vendor_events - BRCM nl80211 asynchoronous event identifiers
 *
 * @BRCM_VENDOR_EVENT_UNSPEC: Reserved value 0
 *
 * @BRCM_VENDOR_EVENT_PRIV_STR: String command/event
 */
enum brcm_nl80211_vendor_events {
        BRCM_VENDOR_EVENT_UNSPEC,
        BRCM_VENDOR_EVENT_PRIV_STR,
};
#endif /* BRCM_VENDOR_H */
