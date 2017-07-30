/*
 * stm32f746_eth_driver.h
 *
 *  Created on: 29 lip 2017
 *      Author: Rafal
 */

#ifndef STM32F746_ETH_DRIVER_H_
#define STM32F746_ETH_DRIVER_H_

#include "stm32f7xx.h"
#include "lwip/err.h"
#include "lwip/netif.h"

/**--------------------------------------------------------------------------**/
/**
  * @brief                           Ethernet PTP defines
  */
/**--------------------------------------------------------------------------**/
/**
  * @}
  */

/** @defgroup ETH_PTP_time_update_method
  * @{
  */
#define ETH_PTP_FineUpdate        ((uint32_t)0x00000001)  /*!< Fine Update method */
#define ETH_PTP_CoarseUpdate      ((uint32_t)0x00000000)  /*!< Coarse Update method */
#define IS_ETH_PTP_UPDATE(UPDATE) (((UPDATE) == ETH_PTP_FineUpdate) || \
                                   ((UPDATE) == ETH_PTP_CoarseUpdate))

/**
  * @}
  */


/** @defgroup ETH_PTP_Flags
  * @{
  */
#define ETH_PTP_FLAG_TSARU        ((uint32_t)0x00000020)  /*!< Addend Register Update */
#define ETH_PTP_FLAG_TSITE        ((uint32_t)0x00000010)  /*!< Time Stamp Interrupt Trigger */
#define ETH_PTP_FLAG_TSSTU        ((uint32_t)0x00000008)  /*!< Time Stamp Update */
#define ETH_PTP_FLAG_TSSTI        ((uint32_t)0x00000004)  /*!< Time Stamp Initialize */

#define ETH_PTP_FLAG_TSTTR        ((uint32_t)0x10000002)  /* Time stamp target time reached */
#define ETH_PTP_FLAG_TSSO         ((uint32_t)0x10000001)  /* Time stamp seconds overflow */

#define IS_ETH_PTP_GET_FLAG(FLAG) (((FLAG) == ETH_PTP_FLAG_TSARU) || \
                                    ((FLAG) == ETH_PTP_FLAG_TSITE) || \
                                    ((FLAG) == ETH_PTP_FLAG_TSSTU) || \
                                    ((FLAG) == ETH_PTP_FLAG_TSSTI) || \
                                    ((FLAG) == ETH_PTP_FLAG_TSTTR) || \
                                    ((FLAG) == ETH_PTP_FLAG_TSSO))

/**
  * @brief  ETH PTP subsecond increment
  */
#define IS_ETH_PTP_SUBSECOND_INCREMENT(SUBSECOND) ((SUBSECOND) <= 0xFF)

/**
  * @}
  */


/** @defgroup ETH_PTP_time_sign
  * @{
  */
#define ETH_PTP_PositiveTime      ((uint32_t)0x00000000)  /*!< Positive time value */
#define ETH_PTP_NegativeTime      ((uint32_t)0x80000000)  /*!< Negative time value */
#define IS_ETH_PTP_TIME_SIGN(SIGN) (((SIGN) == ETH_PTP_PositiveTime) || \
                                    ((SIGN) == ETH_PTP_NegativeTime))

/**
  * @brief  ETH PTP time stamp low update
  */
#define IS_ETH_PTP_TIME_STAMP_UPDATE_SUBSECOND(SUBSECOND) ((SUBSECOND) <= 0x7FFFFFFF)

/**
  * @brief  ETH PTP registers
  */
#define ETH_PTPTSCR     ((uint32_t)0x00000700)  /*!< PTP TSCR register */
#define ETH_PTPSSIR     ((uint32_t)0x00000704)  /*!< PTP SSIR register */
#define ETH_PTPTSHR     ((uint32_t)0x00000708)  /*!< PTP TSHR register */
#define ETH_PTPTSLR     ((uint32_t)0x0000070C)  /*!< PTP TSLR register */
#define ETH_PTPTSHUR    ((uint32_t)0x00000710)  /*!< PTP TSHUR register */
#define ETH_PTPTSLUR    ((uint32_t)0x00000714)  /*!< PTP TSLUR register */
#define ETH_PTPTSAR     ((uint32_t)0x00000718)  /*!< PTP TSAR register */
#define ETH_PTPTTHR     ((uint32_t)0x0000071C)  /*!< PTP TTHR register */
#define ETH_PTPTTLR     ((uint32_t)0x00000720)  /* PTP TTLR register */
#define ETH_PTPTSSR     ((uint32_t)0x00000728)  /* PTP TSSR register */

/**
  * @brief  ETHERNET PTP clock
  */
#define ETH_PTP_OrdinaryClock               ((uint32_t)0x00000000)  /* Ordinary Clock */
#define ETH_PTP_BoundaryClock               ((uint32_t)0x00010000)  /* Boundary Clock */
#define ETH_PTP_EndToEndTransparentClock    ((uint32_t)0x00020000)  /* End To End Transparent Clock */
#define ETH_PTP_PeerToPeerTransparentClock  ((uint32_t)0x00030000)  /* Peer To Peer Transparent Clock */

/**
  * @brief  ETHERNET snapshot
  */
#define ETH_PTP_SnapshotMasterMessage          ((uint32_t)0x00008000)  /* Time stamp snapshot for message relevant to master enable */
#define ETH_PTP_SnapshotEventMessage           ((uint32_t)0x00004000)  /* Time stamp snapshot for event message enable */
#define ETH_PTP_SnapshotIPV4Frames             ((uint32_t)0x00002000)  /* Time stamp snapshot for IPv4 frames enable */
#define ETH_PTP_SnapshotIPV6Frames             ((uint32_t)0x00001000)  /* Time stamp snapshot for IPv6 frames enable */
#define ETH_PTP_SnapshotPTPOverEthernetFrames  ((uint32_t)0x00000800)  /* Time stamp snapshot for PTP over ethernet frames enable */
#define ETH_PTP_SnapshotAllReceivedFrames      ((uint32_t)0x00000100)  /* Time stamp snapshot for all received frames enable */


typedef struct {
  uint32_t TimeStampLow;
  uint32_t TimeStampHigh;
} ETH_TimeStamp;

/**
  * @brief  PTP
  */
void ETH_EnablePTPTimeStampAddend(void);
void ETH_EnablePTPTimeStampInterruptTrigger(void);
void ETH_EnablePTPTimeStampUpdate(void);
void ETH_InitializePTPTimeStamp(void);
void ETH_PTPUpdateMethodConfig(uint32_t UpdateMethod);
void ETH_PTPTimeStampCmd(FunctionalState NewState);
FlagStatus ETH_GetPTPFlagStatus(uint32_t ETH_PTP_FLAG);
void ETH_SetPTPSubSecondIncrement(uint32_t SubSecondValue);
void ETH_SetPTPTimeStampUpdate(uint32_t Sign, uint32_t SecondValue, uint32_t SubSecondValue);
void ETH_SetPTPTimeStampAddend(uint32_t Value);
void ETH_SetPTPTargetTime(uint32_t HighValue, uint32_t LowValue);
uint32_t ETH_GetPTPRegister(uint32_t ETH_PTPReg);

struct ptptime_t {
  s32_t tv_sec;
  s32_t tv_nsec;
};

void ETH_PTPStart(uint32_t UpdateMethod);
void ETH_PTPTime_SetTime(struct ptptime_t * timestamp);
void ETH_PTPTime_GetTime(struct ptptime_t * timestamp);
void ETH_PTPTime_UpdateOffset(struct ptptime_t * timeoffset);
void ETH_PTPTime_AdjFreq(int32_t Adj);

void ETH_EnhancedDescriptorCmd(FunctionalState NewState);

/* Examples of subsecond increment and addend values using SysClk = 144 MHz

 Addend * Increment = 2^63 / SysClk

 ptp_tick = Increment * 10^9 / 2^31

 +-----------+-----------+------------+
 | ptp tick  | Increment | Addend     |
 +-----------+-----------+------------+
 |  119 ns   |   255     | 0x0EF8B863 |
 |  100 ns   |   215     | 0x11C1C8D5 |
 |   50 ns   |   107     | 0x23AE0D90 |
 |   20 ns   |    43     | 0x58C8EC2B |
 |   14 ns   |    30     | 0x7F421F4F |
 +-----------+-----------+------------+
*/

/* Examples of subsecond increment and addend values using SysClk = 168 MHz

 Addend * Increment = 2^63 / SysClk

 ptp_tick = Increment * 10^9 / 2^31

 +-----------+-----------+------------+
 | ptp tick  | Increment | Addend     |
 +-----------+-----------+------------+
 |  119 ns   |   255     | 0x0CD53055 |
 |  100 ns   |   215     | 0x0F386300 |
 |   50 ns   |   107     | 0x1E953032 |
 |   20 ns   |    43     | 0x4C19EF00 |
 |   14 ns   |    30     | 0x6D141AD6 |
 +-----------+-----------+------------+
*/

#define ADJ_FREQ_BASE_ADDEND      994205392
#define ADJ_FREQ_BASE_INCREMENT   43


HAL_StatusTypeDef HAL_ETH_TransmitFrame_TimeStamp(ETH_HandleTypeDef *heth, uint32_t FrameLength, ETH_TimeStamp *TimeStamp);


#endif /* STM32F746_ETH_DRIVER_H_ */
