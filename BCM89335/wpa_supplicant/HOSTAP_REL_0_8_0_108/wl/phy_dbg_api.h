/*
 * Debug module public interface (to MAC driver).
 *
 * Copyright (C) 2014, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id$
 */

#ifndef _phy_dbg_api_h_
#define _phy_dbg_api_h_

/* Debug message levels */
#define PHYHAL_ERROR		0x0001
#define PHYHAL_TRACE		0x0002
#define PHYHAL_INFORM		0x0004
#define PHYHAL_TMP		0x0008
#define PHYHAL_TXPWR		0x0010
#define PHYHAL_CAL		0x0020
#define PHYHAL_ACI		0x0040
#define PHYHAL_RADAR		0x0080
#define PHYHAL_THERMAL		0x0100
#define PHYHAL_PAPD		0x0200
#define PHYHAL_FCBS		0x0400
#define PHYHAL_WD		0x0800
#define PHYHAL_RXIQ		0x1000

#define PHYHAL_TIMESTAMP	0x8000

#endif /* _phy_dbg_api_h_ */
