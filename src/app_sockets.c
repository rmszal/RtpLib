/*
 * app_sockets.c
 *
 *  Created on: 18 cze 2017
 *      Author: Rafal
 */


#include "app_sockets.h"
#include "lwip/sockets.h"
#include "lwip/api.h"
#include "lwip/inet.h"

#define UDP_SOCKET_PORT	5004
#define TCP_SOCKET_PORT 7777

uint8_t recv_buffer[2048];

#define MAXBUF 1024*2
char bufin[MAXBUF];

#define HELLO_PORT 12345
#define HELLO_GROUP "239.0.0.1"

void udp_echo_socket(void *arg)
{
	  int ld;
	  struct sockaddr_in skaddr;
	  int length;
    int len,n;
    struct sockaddr_in remote;
    struct ip_mreq mreq;

    if ((ld = socket( PF_INET, SOCK_DGRAM, 0 )) < 0) {
      return;
    }

    skaddr.sin_family = AF_INET;
    skaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    skaddr.sin_port = htons(UDP_SOCKET_PORT);


    if (bind(ld, (struct sockaddr *) &skaddr, sizeof(skaddr))<0) {
      return;
    }

    setsockopt (ld, IPPROTO_IP, IP_MULTICAST_IF, &skaddr, sizeof(skaddr));

    /* use setsockopt() to request that the kernel join a multicast group */
    mreq.imr_multiaddr.s_addr=inet_addr(HELLO_GROUP);
    mreq.imr_interface.s_addr=htonl(INADDR_ANY);
    int ret;
    if ((ret = setsockopt(ld,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq))) < 0) {
	  perror("setsockopt");
	  exit(1);
    }

    length = sizeof( skaddr );

    while (1) {
      /* read a datagram from the socket (put result in bufin) */
      n=recvfrom(ld,bufin,MAXBUF,0,(struct sockaddr *)&remote,&len);

      rtp_receive(bufin, n);
//      if (n<0) {
//
//      } else {
//        /* Got something, just send it back */
//        sendto(ld,bufin,n,0,(struct sockaddr *)&remote,len);
//      }
    }
}

void tcp_echo_socket(void *arg)
{
	int sock, newconn, size;
	struct sockaddr_in address, remotehost;
	size_t buflen = 1500;
	int len;
	size_t retLen = 100;

	 /* create a TCP socket */
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		return;
	}

	/* bind to port 80 at any interface */
	address.sin_family = AF_INET;
	address.sin_port = htons(TCP_SOCKET_PORT);
	address.sin_addr.s_addr = INADDR_ANY;

	if (bind(sock, (struct sockaddr *)&address, sizeof (address)) < 0)
	{
		return;
	}

	/* listen for incoming connections (TCP listen backlog = 5) */
	lwip_listen(sock, 5);

	size = sizeof(remotehost);

	while (1)
	{
		newconn = lwip_accept(sock, (struct sockaddr *)&remotehost, (socklen_t *)&size);
	    lwip_write(newconn, recv_buffer, retLen);

		while((len = lwip_read(newconn, recv_buffer, buflen)) > 0)
		{
			retLen = len;
		    lwip_write(newconn, recv_buffer, retLen);
		}
	}
}
