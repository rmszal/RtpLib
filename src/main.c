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
#include "stm32746g_discovery_audio.h"
#include "stm32746g_discovery_sd.h"

#include "uart_port.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "lwip/netif.h"
#include "ethernetif.h"
#include "tcpip.h"
#include "lwip/sockets.h"
#include "app_sockets.h"


#include "jbuf.h"

#include "sd_diskio.h"
#include "ff.h"

static volatile uint8_t stackInitialized = 0;
SemaphoreHandle_t Netif_LinkSemaphore = NULL;
/* Ethernet link thread Argument */
struct link_str link_arg;
extern struct netif gnetif;

void FatFsThread(void *arg);
void PlayTask(void *arg);

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 216000000
  *            HCLK(Hz)                       = 216000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 8
  *            PLL_N                          = 432
  *            PLL_P                          = 2
  *            PLL_Q                          = 9
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 7
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  HAL_StatusTypeDef ret = HAL_OK;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

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

  /* activate the OverDrive to reach the 180 Mhz Frequency */
  ret = HAL_PWREx_ActivateOverDrive();

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}


/**
  * @brief  Clock Config.
  * @param  hsai: might be required to set audio peripheral predivider if any.
  * @param  AudioFreq: Audio frequency used to play the audio stream.
  * @note   This API is called by BSP_AUDIO_OUT_Init() and BSP_AUDIO_OUT_SetFrequency()
  *         Being __weak it can be overwritten by the application
  * @retval None
  */
void BSP_AUDIO_OUT_ClockConfig(SAI_HandleTypeDef *hsai, uint32_t AudioFreq, void *Params)
{
  RCC_PeriphCLKInitTypeDef RCC_ExCLKInitStruct;

  HAL_RCCEx_GetPeriphCLKConfig(&RCC_ExCLKInitStruct);

  /* Set the PLL configuration according to the audio frequency */
  if((AudioFreq == AUDIO_FREQUENCY_11K) || (AudioFreq == AUDIO_FREQUENCY_22K) || (AudioFreq == AUDIO_FREQUENCY_44K))
  {
    /* Configure PLLSAI prescalers */
    /* PLLI2S_VCO: VCO_429M
    SAI_CLK(first level) = PLLI2S_VCO/PLLSAIQ = 429/2 = 214.5 Mhz
    SAI_CLK_x = SAI_CLK(first level)/PLLI2SDivQ = 214.5/19 = 11.289 Mhz */
    RCC_ExCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SAI2;
    RCC_ExCLKInitStruct.Sai2ClockSelection = RCC_SAI2CLKSOURCE_PLLI2S;
    RCC_ExCLKInitStruct.PLLI2S.PLLI2SP = 8;
    RCC_ExCLKInitStruct.PLLI2S.PLLI2SN = 429;
    RCC_ExCLKInitStruct.PLLI2S.PLLI2SQ = 2;
    RCC_ExCLKInitStruct.PLLI2SDivQ = 19;
    HAL_RCCEx_PeriphCLKConfig(&RCC_ExCLKInitStruct);
  }
  else /* AUDIO_FREQUENCY_8K, AUDIO_FREQUENCY_16K, AUDIO_FREQUENCY_48K), AUDIO_FREQUENCY_96K */
  {
    /* SAI clock config
    PLLI2S_VCO: VCO_344M
    SAI_CLK(first level) = PLLI2S_VCO/PLLSAIQ = 344/7 = 49.142 Mhz
    SAI_CLK_x = SAI_CLK(first level)/PLLI2SDivQ = 49.142/1 = 49.142 Mhz */
    RCC_ExCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SAI2;
    RCC_ExCLKInitStruct.Sai2ClockSelection = RCC_SAI2CLKSOURCE_PLLI2S;
    RCC_ExCLKInitStruct.PLLI2S.PLLI2SP = 8;
    RCC_ExCLKInitStruct.PLLI2S.PLLI2SN = 344;
    RCC_ExCLKInitStruct.PLLI2S.PLLI2SQ = 7;
    RCC_ExCLKInitStruct.PLLI2SDivQ = 1;
    HAL_RCCEx_PeriphCLKConfig(&RCC_ExCLKInitStruct);
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

__IO uint32_t uwVolume = 50;

uint16_t samplesaudio[JBUF_FRAME_SIZE*2];

void audio_test()
{

	  if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_HEADPHONE, uwVolume, 44100) == 0)
	  {
		  UartPort_WriteString("Audio OUT Init OK\n");
	  }

	  BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);

	  uint16_t i;
	  for( i = 0; i < 11025; i++)
	  {
	//	  samplesaudio[2*i] = 32332 + 32000*sin((i*3141.5)/11024.0);
	//	  samplesaudio[2*i+1] = 32332 + 32000*sin((i*3141.5)/11024.0);
	  }

	  if(BSP_AUDIO_OUT_Play(samplesaudio, sizeof(samplesaudio)) == AUDIO_OK)
		  UartPort_WriteString("Audio Play OK\n");
}

FATFS SDFatFs;  /* File system object for SD card logical drive */
FIL MyFile;     /* File object */
char SDPath[4]; /* SD card logical drive path */
FRESULT res;
uint16_t bw;
int main(void)
{

	CPU_CACHE_Enable();
	HAL_Init();


	SystemClock_Config();
//	UartPort_Init();

//	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

	  /*##-1- Link the micro SD disk I/O driver ##################################*/
	  if(FATFS_LinkDriver(&SD_Driver, SDPath) == 0)
	  {
	    /*##-2- Register the file system object to the FatFs module ##############*/
	    if(f_mount(&SDFatFs, (TCHAR const*)SDPath, 1) != FR_OK)
	    {
	    	while(1);
	    }
	  }
	  else
	  {
		  while(1);
	  }

//	  if((res =f_open(&MyFile, filename, FA_CREATE_ALWAYS)) != FR_OK)
//		  while(1);
//	  res = f_close(&MyFile);



	audio_test();

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
	xTaskCreate(udp_echo_socket, "UDP_ECHO_TASK", 2000, NULL, 6, NULL);
	//xTaskCreate(rtp_socket, "UDP_ECHO_TASK", 3000, NULL, 6, NULL);
	xTaskCreate(FatFsThread, "FATFS_TASK", 2000, NULL, 6, NULL);
	xTaskCreate(PlayTask, "Play_TASK", 2000, NULL, 6, NULL);



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



	while(1)
	{


		vTaskDelay(500);
		BSP_LED_Toggle(LED_GREEN);
	}
}

uint32_t samplesCntIrq = 0;

uint32_t lasttick;

volatile uint8_t buffReady = 0;
volatile uint8_t buffReady1 = 0;
volatile uint8_t buffReady2 = 0;

/*------------------------------------------------------------------------------
       Callbacks implementation:
           the callbacks API are defined __weak in the stm32746g_discovery_audio.c file
           and their implementation should be done the user code if they are needed.
           Below some examples of callback implementations.
  ----------------------------------------------------------------------------*/
/**
  * @brief  Manages the full Transfer complete event.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_OUT_TransferComplete_CallBack(void)
{
//	UartPort_WriteString("Audio IRQ Full!\n");



	uint16_t *ptr;
	uint16_t i;

	ptr = jbuf_get();

	for(i = 0; i < JBUF_FRAME_SIZE; i++)
	{
		samplesCntIrq++;
		samplesaudio[i+JBUF_FRAME_SIZE] = ptr[i];
	}
	buffReady = 1;
	buffReady1 = 1;

}

/**
  * @brief  Manages the DMA Half Transfer complete event.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_OUT_HalfTransfer_CallBack(void)
{
//	UartPort_WriteString("Audio IRQ Half!\n");

	uint16_t *ptr;
	uint16_t i;

	ptr = jbuf_get();

	for(i = 0; i < JBUF_FRAME_SIZE; i++)
	{
		samplesCntIrq++;
		samplesaudio[i] = ptr[i];
	}
	buffReady2 = 1;

	//UartPort_Printf(DEBUG_LVL_INFO, "Stats: %u\n", (samplesCntIrq)/(HAL_GetTick()));
	//UartPort_Printf(DEBUG_LVL_INFO, "Time: %u\n", HAL_GetTick() - lasttick);
	//lasttick = HAL_GetTick();
}

uint16_t sdAudio[JBUF_FRAME_SIZE];

/**
  * @brief  Manages the DMA FIFO error event.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_OUT_Error_CallBack(void)
{
	UartPort_WriteString("Audio Error!\n");
}

char filename[] = "SONG1.WAV";

void PlayTask(void *arg)
{
	uint16_t bw;
	static uint32_t filesize = 0;
	  vTaskDelete(NULL);

	if((res =f_open(&MyFile, filename, FA_OPEN_EXISTING | FA_READ)) != FR_OK)
		  while(1);

	f_read(&MyFile, sdAudio, sizeof(sdAudio), &bw);

	  uint16_t j;

	  for(j = 0; j < JBUF_FRAME_SIZE; j++)
	  {
		  samplesaudio[2*j] = ((int32_t )sdAudio[j]) + 32332;
		  samplesaudio[2*j+1] = ((int32_t )sdAudio[j]) + 32332;
	//	  samplesaudio[2*j] = sdAudio[j];
	//	  samplesaudio[2*j+1] = sdAudio[j];
	  }

	while(1)
	{

		if(buffReady1)
		{
			buffReady1 = 0;
			  for(j = JBUF_FRAME_SIZE/2; j < JBUF_FRAME_SIZE; j++)
			  {
				  samplesaudio[2*j] = ((int32_t )sdAudio[j]) + 32332;
				  samplesaudio[2*j+1] = ((int32_t )sdAudio[j]) + 32332;
			//	  samplesaudio[2*j] = sdAudio[j];
			//	  samplesaudio[2*j+1] = sdAudio[j];
				  }
		}

		if(buffReady2)
		{
			buffReady2 = 0;
			res = f_read(&MyFile, sdAudio, sizeof(sdAudio), &bw);
			if(res != FR_OK)
				while(1);

			  for(j = 0; j < JBUF_FRAME_SIZE/2; j++)
			  {
				  samplesaudio[2*j] = ((int32_t )sdAudio[j]) + 32332;
				  samplesaudio[2*j+1] = ((int32_t )sdAudio[j]) + 32332;
				//  samplesaudio[2*j] = sdAudio[j];
				//  samplesaudio[2*j+1] = sdAudio[j];
			  }
		}

		vTaskDelay(3);
	}


}

void FatFsThread(void *arg)
{
	uint16_t bw;
	static uint32_t filesize = 0;

	if((res =f_open(&MyFile, filename, FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK)
		  while(1);
	  if((res =f_write(&MyFile, samplesaudio, sizeof(samplesaudio), &bw)) != FR_OK)
		  while(1);

	  while(1)
	  {
		  if(buffReady)
		  {
			  uint16_t j;

			  for(j = 0; j < JBUF_FRAME_SIZE; j++)
			  {
				  sdAudio[j] = ((int32_t )samplesaudio[2*j]);
			  }

			  buffReady = 0;
			  if((res =f_write(&MyFile, sdAudio, sizeof(sdAudio), &bw)) != FR_OK)
				  while(1);

			  filesize += bw;

			  if(filesize > 1024*1024*3)
			  {
					vTaskSuspendAll();
				  f_close(&MyFile);
				  xTaskResumeAll();
				  vTaskDelete(NULL);
			  }
		  }

		  vTaskDelay(5);
	  }
}

