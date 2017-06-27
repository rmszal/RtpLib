/*******
 * 
 * FILE INFO:
 * project:	RTP_Lib
 * file:	Rtp_Exemple_Receive_Print.c
 * started on:	05/14/03 16:54:39
 * started by:	Julien Dupasquier <jdupasquier@wanadoo.fr>
 * 
 * 
 * TODO:
 * 
 * BUGS:
 * 
 * UPDATE INFO:
 * updated on:	05/15/03 16:26:37
 * updated by:	Julien Dupasquier <jdupasquier@wanadoo.fr>
 * 
 *******/

#include		<sys/types.h>
#include		<sys/socket.h>

//#include		<netinet/in.h>
#include		<lwip/inet.h>

#include		<stdio.h>
#include		<stdlib.h>
#include		<string.h>
#include		<unistd.h>
#include		<err.h>
#include 		<math.h>

#include 		"Config.h"
#include 		"RTP.h"
#include 		"Macros.h"
#include 		"Proto.h"

#include "uart_port.h"

void			Print_context(char *msg, int len, int cid)
{
  int			i;

  UartPort_Printf(DEBUG_LVL_INFO, "SSRC number                      [%i]\n",context_list[cid]->my_ssrc);
  UartPort_Printf(DEBUG_LVL_INFO, "Number of packets sent           [%i]\n",context_list[cid]->sending_pkt_count);
  UartPort_Printf(DEBUG_LVL_INFO, "Number of bytes sent             [%i]\n",context_list[cid]->sending_octet_count);
  UartPort_Printf(DEBUG_LVL_INFO, "Version                          [%i]\n",context_list[cid]->version);
  UartPort_Printf(DEBUG_LVL_INFO, "Marker flag                      [%i]\n",context_list[cid]->marker);
  UartPort_Printf(DEBUG_LVL_INFO, "Padding length                   [%i]\n",context_list[cid]->padding);
  UartPort_Printf(DEBUG_LVL_INFO, "CSRC length                      [%i]\n",context_list[cid]->CSRClen);
  UartPort_Printf(DEBUG_LVL_INFO, "Payload type                     [%i]\n",context_list[cid]->pt);
  for(i = 0; i < context_list[cid]->CSRClen; i++)
	  UartPort_Printf(DEBUG_LVL_INFO, "CSRC list[%i]                     [%li]\n", i, context_list[cid]->CSRCList[i]);
  UartPort_Printf(DEBUG_LVL_INFO, "First value of timestamp         [%i]\n",context_list[cid]->init_RTP_timestamp);
  UartPort_Printf(DEBUG_LVL_INFO, "Current value of timestamp       [%i]\n",context_list[cid]->RTP_timestamp);
  UartPort_Printf(DEBUG_LVL_INFO, "Time elapsed since the beginning [%i]\n",context_list[cid]->time_elapsed);
  UartPort_Printf(DEBUG_LVL_INFO, "First sequence number            [%i]\n",context_list[cid]->init_seq_no);
  UartPort_Printf(DEBUG_LVL_INFO, "Current sequence number          [%i]\n",context_list[cid]->seq_no);
  UartPort_Printf(DEBUG_LVL_INFO, "Extension header Type            [%i]\n",context_list[cid]->hdr_extension->ext_type);
  UartPort_Printf(DEBUG_LVL_INFO, "Extension header Len             [%i]\n",context_list[cid]->hdr_extension->ext_len);
  for(i = 0; i < context_list[cid]->hdr_extension->ext_len; i++)
	  UartPort_Printf(DEBUG_LVL_INFO, "Extension header[%i]              [%i]\n", i,context_list[cid]->hdr_extension->hd_ext[i]);
  UartPort_Printf(DEBUG_LVL_INFO, "Message[%i] : [%s]\n\n", len, msg);
}


void			print_hdr(rtp_pkt *pkt)
{
	UartPort_Printf(DEBUG_LVL_INFO, "Header du message :\n");

	UartPort_Printf(DEBUG_LVL_INFO, "Version       [%d]\n", (pkt->RTP_header->flags & 0xd0) >> 6);
	UartPort_Printf(DEBUG_LVL_INFO, "Padding       [%d]\n", (pkt->RTP_header->flags & 0x20) >> 5);
	UartPort_Printf(DEBUG_LVL_INFO, "Ext           [%d]\n", (pkt->RTP_header->flags & 0x10) >> 4);
	UartPort_Printf(DEBUG_LVL_INFO, "Cc            [%d]\n", (pkt->RTP_header->flags & 0x0f));
	UartPort_Printf(DEBUG_LVL_INFO, "marker        [%d]\n", (pkt->RTP_header->mk_pt & 0x10) >> 7);
	UartPort_Printf(DEBUG_LVL_INFO, "PayLoad type  [%d]\n", (pkt->RTP_header->mk_pt & 0x7f));
	UartPort_Printf(DEBUG_LVL_INFO, "sq_nb         [%i]\n", ntohs(pkt->RTP_header->sq_nb));
	UartPort_Printf(DEBUG_LVL_INFO, "ts            [%x]\n", ntohl(pkt->RTP_header->ts));
	UartPort_Printf(DEBUG_LVL_INFO, "ssrc          [%x]\n", ntohl(pkt->RTP_header->ssrc));
  UartPort_Printf(DEBUG_LVL_INFO, "csrc          [%i]\n", ntohl(pkt->RTP_header->csrc[0]));
  UartPort_Printf(DEBUG_LVL_INFO, "ext->type     [%i]\n", ntohs(pkt->RTP_extension->ext_type));
  UartPort_Printf(DEBUG_LVL_INFO, "ext->len      [%i]\n", ntohs(pkt->RTP_extension->ext_len));
  UartPort_Printf(DEBUG_LVL_INFO, "ext[0]        [%i]\n", ntohl(pkt->RTP_extension->hd_ext[0]));
  UartPort_Printf(DEBUG_LVL_INFO, "ext[1]        [%i]\n", ntohl(pkt->RTP_extension->hd_ext[1]));
  UartPort_Printf(DEBUG_LVL_INFO, "len PayLoad   [%i]\n", pkt->payload_len);
  UartPort_Printf(DEBUG_LVL_INFO, "PayLoad       [%s]\n", pkt->payload);
}
