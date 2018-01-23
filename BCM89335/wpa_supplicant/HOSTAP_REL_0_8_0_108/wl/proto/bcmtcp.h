/*
 * Fundamental constants relating to TCP Protocol
 *
 * $Copyright Open Broadcom Corporation$
 *
 * $Id: bcmtcp.h 434656 2013-11-07 01:11:33Z $
 */

#ifndef _bcmtcp_h_
#define _bcmtcp_h_

#ifndef _TYPEDEFS_H_
#include <typedefs.h>
#endif


#include <packed_section_start.h>


#define TCP_SRC_PORT_OFFSET	0	
#define TCP_DEST_PORT_OFFSET	2	
#define TCP_SEQ_NUM_OFFSET	4	
#define TCP_ACK_NUM_OFFSET	8	
#define TCP_HLEN_OFFSET		12	
#define TCP_FLAGS_OFFSET	13	
#define TCP_CHKSUM_OFFSET	16	

#define TCP_PORT_LEN		2	


#define TCP_FLAG_URG            0x20
#define TCP_FLAG_ACK            0x10
#define TCP_FLAG_PSH            0x08
#define TCP_FLAG_RST            0x04
#define TCP_FLAG_SYN            0x02
#define TCP_FLAG_FIN            0x01

#define TCP_HLEN_MASK           0xf000
#define TCP_HLEN_SHIFT          12


BWL_PRE_PACKED_STRUCT struct bcmtcp_hdr
{
	uint16	src_port;	
	uint16	dst_port;	
	uint32	seq_num;	
	uint32	ack_num;	
	uint16	hdrlen_rsvd_flags;	
	uint16	tcpwin;		
	uint16	chksum;		
	uint16	urg_ptr;	
} BWL_POST_PACKED_STRUCT;

#define TCP_MIN_HEADER_LEN 20

#define TCP_HDRLEN_MASK 0xf0
#define TCP_HDRLEN_SHIFT 4
#define TCP_HDRLEN(hdrlen) (((hdrlen) & TCP_HDRLEN_MASK) >> TCP_HDRLEN_SHIFT)

#define TCP_FLAGS_MASK  0x1f
#define TCP_FLAGS(hdrlen) ((hdrlen) & TCP_FLAGS_MASK)


#include <packed_section_end.h>

#endif	
