/**
    Uses RTP decoding code from re project, distributed under following license:

    Copyright (c) 2010 - 2011, Alfred E. Heggestad
    Copyright (c) 2010 - 2011, Richard Aas
    Copyright (c) 2010 - 2011, Creytiv.com
    All rights reserved.


    Redistribution and use in source and binary forms, with or without modification, are permitted provided that
    the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this list of conditions and the
    following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
    following disclaimer in the documentation and/or other materials provided with the distribution.

    3. Neither the name of the Creytiv.com nor the names of its contributors may be used to endorse or
    promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
    WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
    IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
    BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
    IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
    Copyright (c) 2010 - 2011, Alfred E. Heggestad
    Copyright (c) 2010 - 2011, Creytiv.com
    All rights reserved.
*/

#include "rtp_rx.h"
#include "jbuf.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "uart_port.h"


//#define LOCAL_DEBUG
#ifdef LOCAL_DEBUG
#	define TRACE(args) (printf("RTP: "), printf args)
#else
#	define TRACE(args)
#endif
#	define MSG(args) (printf("RTP: "), printf args)

#define ntohs(x) ((((x) & 0xff) << 8) | (((x) & 0xff00) >> 8))
#define htonl(x) ( ((x)<<24 & 0xFF000000UL) | \
                   ((x)<< 8 & 0x00FF0000UL) | \
                   ((x)>> 8 & 0x0000FF00UL) | \
                   ((x)>>24 & 0x000000FFUL) )
#define ntohl(x) htonl(x)

enum {
	RTP_VERSION     =  2,  /**< Defines the RTP version we support */
	RTP_HEADER_SIZE = 12   /**< Number of bytes in RTP Header      */
};

#define bool uint8_t

/** Defines the RTP header */
struct rtp_header {
	uint8_t  ver;       /**< RTP version number     */
	bool     pad;       /**< Padding bit            */
	bool     ext;       /**< Extension bit          */
	uint8_t  cc;        /**< CSRC count             */
	bool     m;         /**< Marker bit             */
	uint8_t  pt;        /**< Payload type           */
	uint16_t seq;       /**< Sequence number        */
	uint32_t ts;        /**< Timestamp              */
	uint32_t ssrc;      /**< Synchronization source */
//	uint32_t csrc[16];  /**< Contributing sources   */
	struct {
		uint16_t type;  /**< Defined by profile     */
		uint16_t len;   /**< Number of 32-bit words */
	} x;
};

/**
    \return number of bytes occupied by header or value < 0 on error
*/
static int rtp_hdr_decode(struct rtp_header *hdr, const uint8_t *buf, int size)
{
	int i;
	int header_len;
	int left = size;

	if (left < RTP_HEADER_SIZE)
		return -1;

	hdr->ver  = (buf[0] >> 6) & 0x03;
	if (RTP_VERSION != hdr->ver)
		return -1;
	hdr->pad  = (buf[0] >> 5) & 0x01;
	hdr->ext  = (buf[0] >> 4) & 0x01;
	hdr->cc   = (buf[0] >> 0) & 0x0f;
	hdr->m    = (buf[1] >> 7) & 0x01;
	hdr->pt   = (buf[1] >> 0) & 0x7f;

	buf += 2;
	left -= 2;

	memcpy(&hdr->seq, buf, sizeof(hdr->seq));
	hdr->seq = ntohs(hdr->seq);
	buf += sizeof(hdr->seq);
	left -= sizeof(hdr->seq);
	memcpy(&hdr->ts, buf, sizeof(hdr->ts));
	hdr->ts = ntohl(hdr->ts);
	buf += sizeof(hdr->ts);
	left -= sizeof(hdr->ts);
	memcpy(&hdr->ssrc, buf, sizeof(hdr->ssrc));
	//ntohl
	buf += sizeof(hdr->ssrc);
	left -= sizeof(hdr->ssrc);

	header_len = hdr->cc*sizeof(uint32_t);
	if (left < header_len)
		return -2;

	for (i=0; i<hdr->cc; i++) {
		//hdr->csrc[i] = ntohl(mbuf_read_u32(mb));
		left -= sizeof(uint32_t);
		buf += sizeof(uint32_t);
	}

	if (hdr->ext) {
		if (left < 4)
			return -3;
		memcpy(&hdr->x.type, buf, sizeof(hdr->x.type));
		buf += sizeof(hdr->x.type);
		hdr->x.type = ntohs(hdr->x.type);
		memcpy(&hdr->x.len, buf, sizeof(hdr->x.len));
		buf += sizeof(hdr->x.len);
		hdr->x.len = ntohs(hdr->x.len);
		left -= 4;

		if (left < hdr->x.len*sizeof(uint32_t))
			return -4;

		buf += hdr->x.len*sizeof(uint32_t);
		left -= hdr->x.len*sizeof(uint32_t);
	}

	return size - left;
}


enum { MAX_PACKET_LEN = 20 };   ///< [ms]
//int16_t samples[44100*MAX_PACKET_LEN/1000];   ///< 20 ms worth of data for G.722

uint32_t tick = 0;
uint32_t sampleCnt = 0;


void rtp_receive(uint8_t *data, unsigned int len) {
    /** \todo check source IP:port to block simultaneous transmissions */
//	unsigned long rcid = *(unsigned long*)pusrdata;
//	if( rcid == taskid ) got = 1;
//	struct espconn *pespconn = (struct espconn *)arg;
#if 0
    ets_uart_printf("RECV %p, %p, %dB\n", arg, pusrdata, (int)len);
    char buf[16];
    strncpy(buf, data, sizeof(buf));
    buf[sizeof(buf)-1] = '\0';
    ets_uart_printf("DATA: %s\n", buf);
#endif
    struct rtp_header hdr;
    int rc = rtp_hdr_decode(&hdr, data, len);
    if (len >= 0) {
        int16_t *ptr;
        int i;
        TRACE(("RX pkt len %d, hdr size %d, pt %d, seq %d, ts %u\n", len, rc, hdr.pt, hdr.seq, hdr.ts));
        ptr = data + rc;
        int payload_len = len - rc;
        if (hdr.pt == 10)
        {
        	if(!tick)
        		tick = HAL_GetTick();

        	sampleCnt += (payload_len/2);

        	//__disable_irq();
        	for(i = 0; i < payload_len/2; i++)
        	{
        		int16_t tempSample = (ntohs(ptr[i]));
        		jbuf_put(tempSample);
        	}
        	//__enable_irq();
        }
        jbuf_eop();
    } else {
        TRACE(("Error %d decoding, len %d\n", rc, len));
    }
    return;
}

void rtp_rx_init(void) {

}
