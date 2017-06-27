/*******
 * 
 * FILE INFO:
 * project:	udpserv
 * file:	udpserv.c
 * started on:	05/01/03 15:14:26
 * started by:	Julien Dupasquier <jdupasquier@wanadoo.fr>
 * 
 * 
 * TODO:
 * 
 * BUGS:
 * 
 * UPDATE INFO:
 * updated on:	05/15/03 16:18:24
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
#include		<errno.h>

#include 		"Config.h"
#include 		"RTP.h"
#include 		"Macros.h"
#include 		"Proto.h"

#include		"Rtp_Exemple_Receive.h"

// FONCTIONS Local@udpserv.c

#define HELLO_GROUP "239.0.0.1"

static void		us_serve(struct us *);
static void		us_conf(struct us *);
static int		us_start(struct us *);
static int		us_setup(struct us *us);
static struct us	*us_init();
static void		us_event(struct us *us, int cid, int *len);

int			us_start(struct us *us)
{
  t_listener		*srv;
  int			len;

  srv = us->listeners;
  if ((srv->fd = socket(srv->type, srv->family, 0)) == -1)
    {
      perror("socket");
      exit(EXIT_FAILURE);
    }
#ifdef HAVE_INET6
  if (srv->family == AF_INET6)
    {
      struct sockaddr_in6 *sin;
      
      MEM_ALLOC(sin);
      sin->sin6_len = sizeof (*sin);
      sin->sin6_addr = IN6_ADDR_ANY;
      sin->sin6_port = htons(srv->port);
      len = sin->sin6_len;
      srv->add = (struct sockaddr *) sin;
    }
  else
#endif
    {
      struct sockaddr_in *sin = NULL;
            
      MEM_ALLOC(sin);
      sin->sin_len = sizeof (*sin);
      sin->sin_family = AF_INET;
      sin->sin_addr.s_addr = htonl(INADDR_ANY);;
      sin->sin_port = htons(srv->port);
      srv->add = (struct sockaddr *) sin;
      srv->len = sizeof(*sin); 
    }
  if ((bind(srv->fd, srv->add, srv->len)) == -1)
    {
      perror("bind");
	  exit(EXIT_FAILURE);	  
    }

  struct ip_mreq mreq;

  mreq.imr_multiaddr.s_addr=inet_addr(HELLO_GROUP);
  mreq.imr_interface.s_addr=htonl(INADDR_ANY);

  int ret;
  if ((ret = setsockopt(srv->fd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq))) < 0) {
	  perror("setsockopt");
	  exit(1);
  }
}


void			us_conf(struct us *us)
{
  t_listener		*srv;
  t_client		*client;


  /* Listen Config */
  MEM_ALLOC(srv);
  srv->type = SOCK_DGRAM;
  srv->family = AF_INET;
  srv->port = UDP_PORT;
  us->listeners =  srv;

#ifdef HAVE_INET6
  MEM_ALLOC(srv);
  srv->type = SOCK_DGRAM;
  srv->family = AF_INET6;
  srv->port = UDP_PORT + 2;
  us->listeners = srv;
#endif
  
  /* Client Config */
  MEM_ALLOC(client);
  srv->clients = client;
#ifdef HAVE_INET6
  if (srv->type == AF_INET6)
    client->len = sizeof(struct sockaddr_in6);
#endif
  if (srv->type == AF_INET)
    client->len = sizeof(struct sockaddr_in);
  MEM_SALLOC(client->add, client->len);
}

struct us		*us_init()
{
  struct us		*us;

  MEM_ALLOC(us);
  /*
   * eventuellement il faudrait faire une liste de listeners a partir
   * d'un fichier de conf.
   */
  us_conf(us); /* XXX ne sert a rien */
  (void)us_start(us);
  return (us);
}

void			us_event(struct us *us, int cid, int *len)
{
  static char			msg[MAX_PACKET_LEN];
  t_listener		*srv;
  t_client		*client;
  
  srv = us->listeners;
  client = srv->clients;
  if (FD_ISSET(srv->fd, &(us->fdset)))
    {
      /* un nouveau message est arrive */
      RTP_Receive(cid, srv->fd, msg, len, client->add);
      msg[*len] = '\0';
 //     Print_context(msg, *len, cid);
    }
}

int			us_setup(struct us *us)
{
  int			max = 0;
  t_listener		*srv;
  
  FD_ZERO(&(us->fdset));
  srv = us->listeners;
  FD_SET(srv->fd, &(us->fdset));
  if (srv->fd > max)
    max = srv->fd;
  return ++max;
}

void			us_serve(struct us *us)
{
  int			max;
  int			cid;
  int			len;

  RTP_Create(&cid);
  while (1)
    {
      max = us_setup(us);
      switch (select(max, &us->fdset, NULL, NULL, NULL))
	{
	case -1:
	  if (errno != EINTR) /* le interupted par system call c'est pas grave */
	    {
	      perror("select");
	      exit (EXIT_FAILURE);
	    }
	  break;
	case 0:
	  perror("j'y comprends rien");
	  /* si il y a un timeout c'est possible, sinon non */
	  exit (EXIT_FAILURE);
	default:
	  us_event(us, cid, &len);
	}
    }
  RTP_Destroy(cid);
}

int			rtp_socket(void *arg)
{
  struct us		*server;

  server = us_init();
  us_serve(server);
  return 0;
}
