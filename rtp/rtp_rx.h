#ifndef RTP_RX_H
#define RTP_RX_H

#include <stdint.h>

void rtp_rx_init(void);
void rtp_receive(uint8_t *data, unsigned int len);

#endif
