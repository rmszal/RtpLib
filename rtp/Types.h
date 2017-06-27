/*******
 *
 * FILE INFO:
 * project:	RTP_lib
 * file:	Types.h
 * started on:	03/26/03
 * started by:	Cedric Lacroix <lacroix_cedric@yahoo.com>
 *
 *
 * TODO:
 *
 * BUGS:
 *
 * UPDATE INFO:
 * updated on:	05/13/03
 * updated by:	Cedric Lacroix <lacroix_cedric@yahoo.com>
 *
 *******/

#ifndef TYPES_H
#define TYPES_H

#include "lwip/sockets.h"

#ifndef	FALSE
#define		FALSE	0
#endif

#ifndef TRUE
#define		TRUE	1
#endif

typedef unsigned char	u_int8;
typedef unsigned short	u_int16;
typedef unsigned long	u_int32;

typedef unsigned int	context;


/**
 ** Declaration for unix
 **/
#ifdef UNIX
typedef int                     SOCKET;
typedef struct sockaddr         SOCKADDR;
typedef struct sockaddr_in      SOCKADDR_IN;
#endif /* UNIX */

#ifdef BARE_ARM
typedef int                     SOCKET;
typedef struct sockaddr         SOCKADDR;
typedef struct sockaddr_in      SOCKADDR_IN;
#endif

#endif /* TYPES_H */

