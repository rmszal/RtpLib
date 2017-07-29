/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32f7xx.h"
#include "stm32746g_discovery.h"
#include "uart_port.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "lwip/netif.h"
#include "ethernetif.h"
#include "tcpip.h"
#include "lwip/sockets.h"
#include "app_sockets.h"

static volatile uint8_t stackInitialized = 0;
SemaphoreHandle_t Netif_LinkSemaphore = NULL;
/* Ethernet link thread Argument */
struct link_str link_arg;
extern struct netif gnetif;

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 200000000
  *            HCLK(Hz)                       = 200000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 25
  *            PLL_N                          = 400
  *            PLL_P                          = 2
  *            PLL_Q                          = 8
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 7
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  HAL_StatusTypeDef ret = HAL_OK;

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 400;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 8;

  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }

  /* Activate the OverDrive to reach the 200 MHz Frequency */
  ret = HAL_PWREx_EnableOverDrive();
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6);
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }
}

/**
  * @brief  CPU L1-Cache enable.
  * @param  None
  * @retval None
  */
static void CPU_CACHE_Enable(void)
{
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  //SCB_EnableDCache();
}

void LedTask(void *arg);
void StartThread(void *arg);
void TcpInitResult(void *arg);

int main(void)
{
	SystemClock_Config();
	HAL_Init();
	CPU_CACHE_Enable();
	UartPort_Init();

	xTaskCreate(StartThread, "INIT_TASK", 250, NULL, 1, NULL);

	vTaskStartScheduler();

	while(1)
	{

	}
}


void TcpSocketTask(void *arg);

void StartThread(void *arg)
{
	// Run net task
	tcpip_init(TcpInitResult, NULL);
	while(!stackInitialized);

	// Creating tasks
	Netif_Config();

	xTaskCreate(LedTask, "SCENE_TASK", 250, NULL, 3, NULL);
	xTaskCreate(ethernetif_set_link, "ETH_TASK", 1000, &gnetif, 3, NULL);
	//xTaskCreate(tcp_echo_socket, "TCP_ECHO_TASK", 1000, NULL, 6, NULL);
	//xTaskCreate(udp_echo_socket, "UDP_ECHO_TASK", 1000, NULL, 6, NULL);
	//xTaskCreate(rtp_socket, "UDP_ECHO_TASK", 3000, NULL, 6, NULL);



	while(1)
	{
	    /* Delete the Init Thread */
		vTaskDelete(NULL);
	}
}

void TcpInitResult(void *arg)
{
	stackInitialized = 1;
	UartPort_Printf(DEBUG_LVL_INFO, "TCP/IP stack initialized!\n");
}

void TcpSocketTask(void *arg)
{
	  int sock, newconn, size;
	  struct sockaddr_in address, remotehost;

	 /* create a TCP socket */
	  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	  {
	    return;
	  }

	  /* bind to port 80 at any interface */
	  address.sin_family = AF_INET;
	  address.sin_port = htons(5555);
	  address.sin_addr.s_addr = INADDR_ANY;

	  if (bind(sock, (struct sockaddr *)&address, sizeof (address)) < 0)
	  {
	    return;
	  }

	  /* listen for incoming connections (TCP listen backlog = 5) */
	  listen(sock, 5);

	  size = sizeof(remotehost);

	  while (1)
	  {
	    newconn = accept(sock, (struct sockaddr *)&remotehost, (socklen_t *)&size);
	  }
}


void LedTask(void *arg)
{
	BSP_LED_Init(LED_GREEN);

	// turn off time stamp IRQ
	ETH->MACIMR |= ETH_MACIMR_TSTIM;

	// enable time stamping
	ETH->PTPTSCR |= ETH_PTPTSCR_TSE;

	ETH->PTPTSCR |= ETH_PTPTSSR_TSSSR;

	ETH->PTPTSCR |= ETH_PTPTSCR_TSFCU;

	// program subsecond register
	ETH->PTPSSIR = 10;

	// program addend register
	ETH->PTPTSAR = 1 << 31;
	ETH->PTPTSCR |= ETH_PTPTSCR_TSARU;

	while(ETH->PTPTSCR & ETH_PTPTSCR_TSARU);

	ETH->PTPTSCR |= ETH_PTPTSCR_TSSTU;

	// program time stamp high and low update registers
	ETH->PTPTSHUR = 666;
	ETH->PTPTSLUR = 0;

	// set timestamp init bit
	ETH->PTPTSCR |= ETH_PTPTSCR_TSSTI;

	//

	while(1)
	{

		UartPort_Printf(DEBUG_LVL_INFO, "Seconds register: %u!\n", ETH->PTPTSHR);

		vTaskDelay(500);
		BSP_LED_Toggle(LED_GREEN);
	}
}


