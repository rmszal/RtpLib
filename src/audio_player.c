/*
 * audio_player.c
 *
 *  Created on: 6 lip 2017
 *      Author: Rafal
 */

#include "stm32f7xx.h"
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_audio.h"
#include "stm32746g_discovery_sd.h"

#include "sd_diskio.h"
#include "ff.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "jbuf.h"
#include "uart_port.h"

int16_t samplesaudio[JBUF_FRAME_SIZE*2];
__IO uint32_t uwVolume = 50;

void audio_player_init(void)
{
	if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_HEADPHONE, uwVolume, 44100) == 0)
	{
		UartPort_WriteString("Audio OUT Init OK\n");
	}

	BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);

	if(BSP_AUDIO_OUT_Play(samplesaudio, sizeof(samplesaudio)) == AUDIO_OK)
	{
		UartPort_WriteString("Audio Play OK\n");
	}
}

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
	int16_t *ptr;
	uint16_t i;

	ptr = jbuf_get();

	for(i = 0; i < JBUF_FRAME_SIZE; i++)
	{
		samplesaudio[i+JBUF_FRAME_SIZE] = ptr[i];
	}
}

/**
  * @brief  Manages the DMA Half Transfer complete event.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_OUT_HalfTransfer_CallBack(void)
{
	int16_t *ptr;
	uint16_t i;

	ptr = jbuf_get();

	for(i = 0; i < JBUF_FRAME_SIZE; i++)
	{
		samplesaudio[i] = ptr[i];
	}
}

int16_t sdAudio[JBUF_FRAME_SIZE];

/**
  * @brief  Manages the DMA FIFO error event.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_OUT_Error_CallBack(void)
{
	UartPort_WriteString("Audio Error!\n");
}

FRESULT res;
uint16_t bw;
FIL MyFile;     /* File object */

volatile uint32_t lasttick;
volatile uint8_t buffReady = 0;
volatile uint8_t buffReady1 = 0;
volatile uint8_t buffReady2 = 0;

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
		  samplesaudio[2*j] = sdAudio[j];
		  samplesaudio[2*j+1] = sdAudio[j];
	  }

	while(1)
	{

		if(buffReady1)
		{
			buffReady1 = 0;
			  for(j = JBUF_FRAME_SIZE/2; j < JBUF_FRAME_SIZE; j++)
			  {
				  samplesaudio[2*j] = sdAudio[j];
				  samplesaudio[2*j+1] = sdAudio[j];
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
				  samplesaudio[2*j] = sdAudio[j];
				  samplesaudio[2*j+1] = sdAudio[j];
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
