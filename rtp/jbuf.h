#ifndef JBUF_H
#define JBUF_H

enum {
    /** 10 ms data at 16ksps */
    JBUF_FRAME_SIZE = 410
};

/** \return non-zero on overflow */
int jbuf_put(short sample);
/** \brief Mark End-Of-Packet */
void jbuf_eop(void);
short* jbuf_get(void);

#endif
