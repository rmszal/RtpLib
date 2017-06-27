#include "stm32f7xx_hal.h"
#include "uart_port.h"
#include "stdlib.h"
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#define UARTPORT_UART USART1

#define TX_BUFFER_SIZE	512
#define RX_BUFFER_SIZE	128
#define UART_PRIORITY         15
#define UART_RX_SUBPRIORITY   0

void (*UartPort_Callback)(void);

volatile uint8_t flag;

static RingBuffer_t uartTxBuffer;
static RingBuffer_t uartRxBuffer;

static char txData[TX_BUFFER_SIZE] __attribute__ (( section(".tcram")));
static char rxData[RX_BUFFER_SIZE] __attribute__ (( section(".tcram")));
static char printfBuff[TX_BUFFER_SIZE] __attribute__ (( section(".tcram")));

UART_HandleTypeDef UartPort_UartHandle;

static char rxChar, txChar;

void UartPort_PortInit(void);

#ifdef UARTPORT_ON

void UartPort_Init()
{
	flag = 0;
	UartPort_Callback = NULL;
	RingBuffer_Init(&uartRxBuffer, (uint8_t *)rxData, (size_t)RX_BUFFER_SIZE);
	RingBuffer_Init(&uartTxBuffer, (uint8_t *)txData, (size_t)TX_BUFFER_SIZE);

	UartPort_PortInit();

	HAL_UART_Receive_IT(&UartPort_UartHandle, (uint8_t *)&rxChar, 1);
}
#else
void UartPort_Init()
{

}
#endif

#ifdef UARTPORT_ON
size_t UartPort_WriteData(uint8_t *data, size_t dataSize)
{
	assert(data);
	assert(dataSize);

	size_t cnt = 0;
	while(cnt < dataSize && RingBuffer_PutChar(&uartTxBuffer, (char)data[cnt]))
	{
		cnt++;
	}
	if(!RingBuffer_IsEmpty(&uartTxBuffer))
	{
		RingBuffer_GetChar(&uartTxBuffer, (char *)&txChar);
		HAL_UART_Transmit_IT(&UartPort_UartHandle, (uint8_t *)&txChar, 1);
	}
	return cnt;
}
#else
size_t UartPort_WriteData(uint8_t *data, size_t dataSize)
{

}
#endif

size_t UartPort_ReadData(uint8_t *data, size_t maxDataSize)
{
	size_t cnt = 0;
	char ch;
	while(RingBuffer_GetChar(&uartRxBuffer, &ch) && cnt < maxDataSize)
	{
		data[cnt] = ch;
		cnt++;
	}
	return cnt;
}

bool UartPort_WriteString(char *string)
{
	return UartPort_WriteData((uint8_t *)string, strlen(string));
}

bool UartPort_PutChar(char ch)
{
	bool result = RingBuffer_PutChar(&uartTxBuffer, ch);
	if(!RingBuffer_IsEmpty(&uartTxBuffer) && HAL_UART_STATE_READY == HAL_UART_GetState(&UartPort_UartHandle))
	{
		RingBuffer_GetChar(&uartTxBuffer, (char *)&txChar);
		HAL_UART_Transmit_IT(&UartPort_UartHandle, (uint8_t *)&txChar, 1);
	}
	return result;
}

bool UartPort_GetChar(char *ch)
{
	return RingBuffer_GetChar(&uartRxBuffer, ch);
}

size_t UartPort_ReadString(uint8_t *data, size_t maxDataSize)
{
	if(data)
	{
		size_t cnt = 0;
		char ch;
		while(cnt+1 < maxDataSize && RingBuffer_GetChar(&uartRxBuffer, &ch))
		{
			data[cnt] = ch;
			cnt++;
			if(ch == 0)
				return cnt;
		}
		RingBuffer_GetChar(&uartRxBuffer, &ch);
		cnt++;
		data[cnt] = 0;
		return cnt;
	}
	return 0;
}


size_t UartPort_Printf(debug_level_e lvl, const char * format, ... )
{
#ifdef UARTPORT_ON

    va_list args;
    va_start(args, format);

    if(lvl <= UARTPORT_DEBUG_LEVEL)
    {
        vsprintf(printfBuff,format, args);// This still uses standaes formating
        UartPort_WriteString(printfBuff);
    }

    va_end(args);
#endif
    return 0;// Before return you can redefine it back if you want...
}

void UartPort_PortInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_USART1_CLK_ENABLE();
	__HAL_RCC_USART1_FORCE_RESET();
	__HAL_RCC_USART1_RELEASE_RESET();

	GPIO_InitStruct.Pin = GPIO_PIN_9;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_10;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	UartPort_UartHandle.Instance = UARTPORT_UART;
	UartPort_UartHandle.Init.BaudRate = 115200;
	UartPort_UartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	UartPort_UartHandle.Init.Mode = UART_MODE_TX_RX;
	UartPort_UartHandle.Init.OverSampling = UART_OVERSAMPLING_8;
	UartPort_UartHandle.Init.Parity = UART_PARITY_NONE;
	UartPort_UartHandle.Init.StopBits = UART_STOPBITS_1;
	UartPort_UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
	HAL_UART_Init(&UartPort_UartHandle);

	HAL_NVIC_SetPriority(USART1_IRQn, UART_PRIORITY, UART_RX_SUBPRIORITY);
	HAL_NVIC_EnableIRQ(USART1_IRQn);
	HAL_UART_Receive_IT(&UartPort_UartHandle, (uint8_t *)&rxChar, 1);
}

bool UartPort_SetCallbackFunction(void (*foo)(void))
{
	assert(foo);
	UartPort_Callback = foo;
	return true;
}

void UartPort_RxCpltCallbck(UART_HandleTypeDef *huart)
{
	(void)huart;

	if(rxChar == '\n')
	{
		rxChar = 0;
		UartPort_Callback();
		flag = 1;
	}
	RingBuffer_PutChar(&uartRxBuffer, (char)rxChar);
	HAL_UART_Receive_IT(&UartPort_UartHandle, (uint8_t *)&rxChar, 1);
}

void UartPort_TxCpltCallbck(UART_HandleTypeDef *huart)
{
	(void)huart;

	if(!RingBuffer_IsEmpty(&uartTxBuffer))
	{
		RingBuffer_GetChar(&uartTxBuffer, (char *)&txChar);
		HAL_UART_Transmit_IT(&UartPort_UartHandle, (uint8_t *)&txChar, 1);
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	UartPort_TxCpltCallbck(huart);
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	UartPort_RxCpltCallbck(huart);
}

#ifdef UARTPORT_ON
void USART1_IRQHandler(void)
{
	HAL_UART_IRQHandler(&UartPort_UartHandle);
}
#endif
