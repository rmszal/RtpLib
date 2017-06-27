#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__


#include "lwip/err.h"
#include "lwip/netif.h"
#include "FreeRTOS.h"
#include "semphr.h"

/* Exported types ------------------------------------------------------------*/
/* Structure that include link thread parameters */
   struct link_str {
  struct netif *netif;
  SemaphoreHandle_t semaphore;
};

/* Exported types ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
err_t ethernetif_init(struct netif *netif);
void ETHERNET_IRQHandler(void);
u32_t sys_now(void);
void ethernetif_update_config(struct netif *netif);
void ethernetif_set_link(void const *argument);
#endif
