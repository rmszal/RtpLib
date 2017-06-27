#ifndef UART_PORT_H
#define UART_PORT_H

#include "ring_buffer.h"
#include "stm32f7xx_hal.h"

extern volatile uint8_t flag;
extern UART_HandleTypeDef UartPort_UartHandle;

#define UARTPORT_ON

typedef enum {
	DEBUG_LVL_ERROR = 0,
	DEBUG_LVL_WARNING,
	DEBUG_LVL_INFO,
	DEBUG_LVL_DEBUG,
	DEBUG_LVL_ALL,
} debug_level_e;

#define UARTPORT_DEBUG_LEVEL DEBUG_LVL_ALL

void UartPort_Init();
size_t UartPort_WriteData(uint8_t *data, size_t dataSize);
size_t UartPort_ReadData(uint8_t *data, size_t maxDataSize);
size_t UartPort_ReadString(uint8_t *data, size_t maxDataSize);
bool UartPort_WriteString(char *string);
bool UartPort_PutChar(char ch);
bool UartPort_GetChar(char *ch);
bool UartPort_SetCallbackFunction(void (*foo)(void));
void UartPort_RxCpltCallbck(UART_HandleTypeDef *huart);
void UartPort_TxCpltCallbck(UART_HandleTypeDef *huart);
size_t UartPort_Printf(debug_level_e lvl, const char * format, ... );

#endif
