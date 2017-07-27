#ifndef JBUF_H
#define JBUF_H

#include "stm32f7xx.h"

enum {
    /** 50 ms data at 44.1ksps */
    JBUF_FRAME_SIZE = 441
};

/** \return non-zero on overflow */
int jbuf_put(int16_t sample);
/** \brief Mark End-Of-Packet */
void jbuf_eop(void);
int16_t* jbuf_get(void);

#endif
