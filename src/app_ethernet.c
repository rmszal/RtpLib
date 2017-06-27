/**
  ******************************************************************************
  * @file    LwIP/LwIP_HTTP_Server_Socket_RTOS/Src/app_ethernet.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    22-April-2016
  * @brief   Ethernet specefic module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics International N.V. 
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "lwip/opt.h"
#include "lcd_log.h"
#include "lwip/dhcp.h"
#include "app_ethernet.h"
#include "ethernetif.h"
#include "tcpip.h"
#include "uart_port.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#ifdef USE_DHCP
#define MAX_DHCP_TRIES  4
uint32_t DHCPfineTimer = 0;
__IO uint8_t DHCP_state;
#endif

struct netif gnetif; /* network interface structure */
err_t err;

/**
  * @brief  This function notify user about link status changement.
  * @param  netif: the network interface
  * @retval None
  */
void ethernetif_notify_conn_changed(struct netif *netif)
{
  if(netif_is_link_up(netif))
  {
	  netif_set_up(&gnetif);

	  // Start DHCP
	  ip_addr_t ipaddr;
	  ip_addr_t netmask;
	  ip_addr_t gw;

	  /* IP address setting */
	  IP4_ADDR(&ipaddr, 192, 168, 5, 150);
	  IP4_ADDR(&netmask, 255, 255 , 255, 0);
	  IP4_ADDR(&gw, 192, 168, 5, 1);

	  netif_set_addr(netif, &ipaddr, &netmask, &gw);

      err = dhcp_start(netif);
  }
  else
  {
	  // Stop DHCP
      dhcp_stop(netif);
      netif_set_down(&gnetif);
  }
}

void ip_addr_changed(struct netif *netif)
{
	static uint32_t address;
	ip_addr_t * ip_ptr = netif_ip4_addr(netif);
	if(ip_ptr->addr != address)
	{
		address = ip_ptr->addr;
		UartPort_Printf(DEBUG_LVL_INFO, "IP address changed: %u.%u.%u.%u\n", ip4_addr1_16(ip_ptr), ip4_addr2_16(ip_ptr), ip4_addr3_16(ip_ptr), ip4_addr4_16(ip_ptr));
	}
}

/**
  * @brief  Initializes the lwIP stack
  * @param  None
  * @retval None
  */
void Netif_Config(void)
{
  ip_addr_t ipaddr;
  ip_addr_t netmask;
  ip_addr_t gw;

  /* IP address default setting */
  IP4_ADDR(&ipaddr, 192, 168, 5, 150);
  IP4_ADDR(&netmask, 255, 255 , 255, 0);
  IP4_ADDR(&gw, 192, 168, 5, 1);

  /* - netif_add(struct netif *netif, ip_addr_t *ipaddr,
  ip_addr_t *netmask, ip_addr_t *gw,
  void *state, err_t (* init)(struct netif *netif),
  err_t (* input)(struct pbuf *p, struct netif *netif))

  Adds your network interface to the netif_list. Allocate a struct
  netif and pass a pointer to this structure as the first argument.
  Give pointers to cleared ip_addr structures when using DHCP,
  or fill them with sane numbers otherwise. The state pointer may be NULL.

  The init function pointer must point to a initialization function for
  your ethernet netif interface. The following code illustrates it's use.*/

  netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);

  /*  Registers the default network interface. */
  netif_set_default(&gnetif);

//  if (netif_is_link_up(&gnetif))
//  {
//    /* When the netif is fully configured this function must be called.*/
//    netif_set_up(&gnetif);
//  }
//  else
//  {
//    /* When the netif link is down this function must be called */
//    netif_set_down(&gnetif);
//  }


  /* Set the link callback function, this function is called on change of link status*/
  netif_set_link_callback(&gnetif, ethernetif_update_config);

  netif_set_status_callback(&gnetif, ip_addr_changed);
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
