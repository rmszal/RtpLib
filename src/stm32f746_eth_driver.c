/*
 * stm32f746_eth_driver.c
 *
 *  Created on: 29 lip 2017
 *      Author: Rafal
 */

#include "stm32f746_eth_driver.h"

/*---------------------------------  PTP  ------------------------------------*/

u32_t ETH_PTPSubSecond2NanoSecond(u32_t SubSecondValue)
{
  uint64_t val = SubSecondValue * 1000000000ll;
  val >>=31;
  return val;
}


u32_t ETH_PTPNanoSecond2SubSecond(u32_t SubSecondValue)
{
  uint64_t val = SubSecondValue * 0x80000000ll;
  val /= 1000000000;
  return val;
}


void ETH_PTPTime_GetTime(struct ptptime_t * timestamp)
{
  timestamp->tv_nsec = ETH_PTPSubSecond2NanoSecond(ETH_GetPTPRegister(ETH_PTPTSLR));
  timestamp->tv_sec = ETH_GetPTPRegister(ETH_PTPTSHR);
}

/**
  * @brief  Updated the PTP block for fine correction with the Time Stamp Addend register value.
  * @param  None
  * @retval None
  */
void ETH_EnablePTPTimeStampAddend(void)
{
  /* Enable the PTP block update with the Time Stamp Addend register value */
  ETH->PTPTSCR |= ETH_PTPTSCR_TSARU;
}

/**
  * @brief  Enable the PTP Time Stamp interrupt trigger
  * @param  None
  * @retval None
  */
void ETH_EnablePTPTimeStampInterruptTrigger(void)
{
  /* Enable the PTP target time interrupt */
  ETH->PTPTSCR |= ETH_PTPTSCR_TSITE;
}

/**
  * @brief  Updated the PTP system time with the Time Stamp Update register value.
  * @param  None
  * @retval None
  */
void ETH_EnablePTPTimeStampUpdate(void)
{
  /* Enable the PTP system time update with the Time Stamp Update register value */
  ETH->PTPTSCR |= ETH_PTPTSCR_TSSTU;
}

/**
  * @brief  Initialize the PTP Time Stamp
  * @param  None
  * @retval None
  */
void ETH_InitializePTPTimeStamp(void)
{
  /* Initialize the PTP Time Stamp */
  ETH->PTPTSCR |= ETH_PTPTSCR_TSSTI;
}

/**
  * @brief  Selects the PTP Update method
  * @param  UpdateMethod: the PTP Update method
  *   This parameter can be one of the following values:
  *     @arg ETH_PTP_FineUpdate   : Fine Update method
  *     @arg ETH_PTP_CoarseUpdate : Coarse Update method
  * @retval None
  */
void ETH_PTPUpdateMethodConfig(uint32_t UpdateMethod)
{
  /* Check the parameters */
  assert_param(IS_ETH_PTP_UPDATE(UpdateMethod));

  if (UpdateMethod != ETH_PTP_CoarseUpdate)
  {
    /* Enable the PTP Fine Update method */
    ETH->PTPTSCR |= ETH_PTPTSCR_TSFCU;
  }
  else
  {
    /* Disable the PTP Coarse Update method */
    ETH->PTPTSCR &= (~(uint32_t)ETH_PTPTSCR_TSFCU);
  }
}

/**
  * @brief  Enables or disables the PTP time stamp for transmit and receive frames.
  * @param  NewState: new state of the PTP time stamp for transmit and receive frames
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ETH_PTPTimeStampCmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    /* Enable the PTP time stamp for transmit and receive frames */
    ETH->PTPTSCR |= ETH_PTPTSCR_TSE | ETH_PTPTSSR_TSSIPV4FE | ETH_PTPTSSR_TSSIPV6FE | ETH_PTPTSSR_TSSARFE;
  }
  else
  {
    /* Disable the PTP time stamp for transmit and receive frames */
    ETH->PTPTSCR &= (~(uint32_t)ETH_PTPTSCR_TSE);
  }
}

/**
  * @brief  Checks whether the specified ETHERNET PTP flag is set or not.
  * @param  ETH_PTP_FLAG: specifies the flag to check.
  *   This parameter can be one of the following values:
  *     @arg ETH_PTP_FLAG_TSARU : Addend Register Update
  *     @arg ETH_PTP_FLAG_TSITE : Time Stamp Interrupt Trigger Enable
  *     @arg ETH_PTP_FLAG_TSSTU : Time Stamp Update
  *     @arg ETH_PTP_FLAG_TSSTI  : Time Stamp Initialize
  * @retval The new state of ETHERNET PTP Flag (SET or RESET).
  */
FlagStatus ETH_GetPTPFlagStatus(uint32_t ETH_PTP_FLAG)
{
  FlagStatus bitstatus = RESET;
  /* Check the parameters */
  assert_param(IS_ETH_PTP_GET_FLAG(ETH_PTP_FLAG));

  if ((ETH->PTPTSCR & ETH_PTP_FLAG) != (uint32_t)RESET)
  {
    bitstatus = SET;
  }
  else
  {
    bitstatus = RESET;
  }
  return bitstatus;
}

/**
  * @brief  Sets the system time Sub-Second Increment value.
  * @param  SubSecondValue: specifies the PTP Sub-Second Increment Register value.
  * @retval None
  */
void ETH_SetPTPSubSecondIncrement(uint32_t SubSecondValue)
{
  /* Check the parameters */
  assert_param(IS_ETH_PTP_SUBSECOND_INCREMENT(SubSecondValue));
  /* Set the PTP Sub-Second Increment Register */
  ETH->PTPSSIR = SubSecondValue;
}

/**
  * @brief  Sets the Time Stamp update sign and values.
  * @param  Sign: specifies the PTP Time update value sign.
  *   This parameter can be one of the following values:
  *     @arg ETH_PTP_PositiveTime : positive time value.
  *     @arg ETH_PTP_NegativeTime : negative time value.
  * @param  SecondValue: specifies the PTP Time update second value.
  * @param  SubSecondValue: specifies the PTP Time update sub-second value.
  *   This parameter is a 31 bit value, bit32 correspond to the sign.
  * @retval None
  */
void ETH_SetPTPTimeStampUpdate(uint32_t Sign, uint32_t SecondValue, uint32_t SubSecondValue)
{
  /* Check the parameters */
  assert_param(IS_ETH_PTP_TIME_SIGN(Sign));
  assert_param(IS_ETH_PTP_TIME_STAMP_UPDATE_SUBSECOND(SubSecondValue));
  /* Set the PTP Time Update High Register */
  ETH->PTPTSHUR = SecondValue;

  /* Set the PTP Time Update Low Register with sign */
  ETH->PTPTSLUR = Sign | SubSecondValue;
}

/**
  * @brief  Sets the Time Stamp Addend value.
  * @param  Value: specifies the PTP Time Stamp Addend Register value.
  * @retval None
  */
void ETH_SetPTPTimeStampAddend(uint32_t Value)
{
  /* Set the PTP Time Stamp Addend Register */
  ETH->PTPTSAR = Value;
}

/**
  * @brief  Sets the Target Time registers values.
  * @param  HighValue: specifies the PTP Target Time High Register value.
  * @param  LowValue: specifies the PTP Target Time Low Register value.
  * @retval None
  */
void ETH_SetPTPTargetTime(uint32_t HighValue, uint32_t LowValue)
{
  /* Set the PTP Target Time High Register */
  ETH->PTPTTHR = HighValue;
  /* Set the PTP Target Time Low Register */
  ETH->PTPTTLR = LowValue;
}

/**
  * @brief  Get the specified ETHERNET PTP register value.
  * @param  ETH_PTPReg: specifies the ETHERNET PTP register.
  *   This parameter can be one of the following values:
  *     @arg ETH_PTPTSCR  : Sub-Second Increment Register
  *     @arg ETH_PTPSSIR  : Sub-Second Increment Register
  *     @arg ETH_PTPTSHR  : Time Stamp High Register
  *     @arg ETH_PTPTSLR  : Time Stamp Low Register
  *     @arg ETH_PTPTSHUR : Time Stamp High Update Register
  *     @arg ETH_PTPTSLUR : Time Stamp Low Update Register
  *     @arg ETH_PTPTSAR  : Time Stamp Addend Register
  *     @arg ETH_PTPTTHR  : Target Time High Register
  *     @arg ETH_PTPTTLR  : Target Time Low Register
  * @retval The value of ETHERNET PTP Register value.
  */
uint32_t ETH_GetPTPRegister(uint32_t ETH_PTPReg)
{
  /* Check the parameters */
  assert_param(IS_ETH_PTP_REGISTER(ETH_PTPReg));

  /* Return the selected register value */
  return (*(__IO uint32_t *)(ETH_MAC_BASE + ETH_PTPReg));
}

/*******************************************************************************
* Function Name  : ETH_PTPStart
* Description    : Initialize timestamping ability of ETH
* Input          : UpdateMethod:
*                       ETH_PTP_FineUpdate   : Fine Update method
*                       ETH_PTP_CoarseUpdate : Coarse Update method
* Output         : None
* Return         : None
*******************************************************************************/
void ETH_PTPStart(uint32_t UpdateMethod)
{
  /* Check the parameters */
  assert_param(IS_ETH_PTP_UPDATE(UpdateMethod));

  /* Mask the Time stamp trigger interrupt by setting bit 9 in the MACIMR register. */
	ETH->MACIMR |= ETH_MACIMR_TSTIM;

  /* Program Time stamp register bit 0 to enable time stamping. */
  ETH_PTPTimeStampCmd(ENABLE);

  /* Program the Subsecond increment register based on the PTP clock frequency. */
  ETH_SetPTPSubSecondIncrement(ADJ_FREQ_BASE_INCREMENT); /* to achieve 20 ns accuracy, the value is ~ 43 */

  if (UpdateMethod == ETH_PTP_FineUpdate)
	{
    /* If you are using the Fine correction method, program the Time stamp addend register
     * and set Time stamp control register bit 5 (addend register update). */
    ETH_SetPTPTimeStampAddend(ADJ_FREQ_BASE_ADDEND);
    ETH_EnablePTPTimeStampAddend();

    /* Poll the Time stamp control register until bit 5 is cleared. */
    while(ETH_GetPTPFlagStatus(ETH_PTP_FLAG_TSARU) == SET);
  }

  /* To select the Fine correction method (if required),
   * program Time stamp control register  bit 1. */
  ETH_PTPUpdateMethodConfig(UpdateMethod);

  /* Program the Time stamp high update and Time stamp low update registers
   * with the appropriate time value. */
  ETH_SetPTPTimeStampUpdate(ETH_PTP_PositiveTime, 0, 0);

  /* Set Time stamp control register bit 2 (Time stamp init). */
  ETH_InitializePTPTimeStamp();

	/* The enhanced descriptor format is enabled and the descriptor size is
	 * increased to 32 bytes (8 DWORDS). This is required when time stamping
	 * is activated above. */
	ETH_EnhancedDescriptorCmd(ENABLE);

  /* The Time stamp counter starts operation as soon as it is initialized
   * with the value written in the Time stamp update register. */
}

void ETH_EnhancedDescriptorCmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    /* Enable enhanced descriptor structure */
    ETH->DMABMR |= ETH_DMABMR_EDE;
  }
  else
  {
    /* Disable enhanced descriptor structure */
    ETH->DMABMR &= ~ETH_DMABMR_EDE;
  }
}

/*******************************************************************************
* Function Name  : ETH_PTPTimeStampAdjFreq
* Description    : Updates time stamp addend register
* Input          : Correction value in thousandth of ppm (Adj*10^9)
* Output         : None
* Return         : None
*******************************************************************************/
void ETH_PTPTime_AdjFreq(int32_t Adj)
{
	uint32_t addend;

	/* calculate the rate by which you want to speed up or slow down the system time
		 increments */

	/* precise */
	/*
	int64_t addend;
	addend = Adj;
	addend *= ADJ_FREQ_BASE_ADDEND;
	addend /= 1000000000-Adj;
	addend += ADJ_FREQ_BASE_ADDEND;
	*/

	/* 32bit estimation
	ADJ_LIMIT = ((1l<<63)/275/ADJ_FREQ_BASE_ADDEND) = 11258181 = 11 258 ppm*/
	if( Adj > 5120000) Adj = 5120000;
	if( Adj < -5120000) Adj = -5120000;

	addend = ((((275LL * Adj)>>8) * (ADJ_FREQ_BASE_ADDEND>>24))>>6) + ADJ_FREQ_BASE_ADDEND;

	/* Reprogram the Time stamp addend register with new Rate value and set ETH_TPTSCR */
	ETH_SetPTPTimeStampAddend((uint32_t)addend);
	ETH_EnablePTPTimeStampAddend();
}

/*******************************************************************************
* Function Name  : ETH_PTPTimeStampUpdateOffset
* Description    : Updates time base offset
* Input          : Time offset with sign
* Output         : None
* Return         : None
*******************************************************************************/
void ETH_PTPTime_UpdateOffset(struct ptptime_t * timeoffset)
{
	uint32_t Sign;
	uint32_t SecondValue;
	uint32_t NanoSecondValue;
	uint32_t SubSecondValue;
	uint32_t addend;

	/* determine sign and correct Second and Nanosecond values */
	if(timeoffset->tv_sec < 0 || (timeoffset->tv_sec == 0 && timeoffset->tv_nsec < 0))
	{
		Sign = ETH_PTP_NegativeTime;
		SecondValue = -timeoffset->tv_sec;
		NanoSecondValue = -timeoffset->tv_nsec;
	}
	else
	{
		Sign = ETH_PTP_PositiveTime;
		SecondValue = timeoffset->tv_sec;
		NanoSecondValue = timeoffset->tv_nsec;
	}

	/* convert nanosecond to subseconds */
	SubSecondValue = ETH_PTPNanoSecond2SubSecond(NanoSecondValue);

	/* read old addend register value*/
	addend = ETH_GetPTPRegister(ETH_PTPTSAR);

	while(ETH_GetPTPFlagStatus(ETH_PTP_FLAG_TSSTU) == SET);
	while(ETH_GetPTPFlagStatus(ETH_PTP_FLAG_TSSTI) == SET);

	/* Write the offset (positive or negative) in the Time stamp update high and low registers. */
	ETH_SetPTPTimeStampUpdate(Sign, SecondValue, SubSecondValue);

	/* Set bit 3 (TSSTU) in the Time stamp control register. */
	ETH_EnablePTPTimeStampUpdate();

	/* The value in the Time stamp update registers is added to or subtracted from the system */
	/* time when the TSSTU bit is cleared. */
	while(ETH_GetPTPFlagStatus(ETH_PTP_FLAG_TSSTU) == SET);

	/* Write back old addend register value. */
	ETH_SetPTPTimeStampAddend(addend);
	ETH_EnablePTPTimeStampAddend();
}

/*******************************************************************************
* Function Name  : ETH_PTPTimeStampSetTime
* Description    : Initialize time base
* Input          : Time with sign
* Output         : None
* Return         : None
*******************************************************************************/
void ETH_PTPTime_SetTime(struct ptptime_t * timestamp)
{
	uint32_t Sign;
	uint32_t SecondValue;
	uint32_t NanoSecondValue;
	uint32_t SubSecondValue;

	/* determine sign and correct Second and Nanosecond values */
	if(timestamp->tv_sec < 0 || (timestamp->tv_sec == 0 && timestamp->tv_nsec < 0))
	{
		Sign = ETH_PTP_NegativeTime;
		SecondValue = -timestamp->tv_sec;
		NanoSecondValue = -timestamp->tv_nsec;
	}
	else
	{
		Sign = ETH_PTP_PositiveTime;
		SecondValue = timestamp->tv_sec;
		NanoSecondValue = timestamp->tv_nsec;
	}

	/* convert nanosecond to subseconds */
	SubSecondValue = ETH_PTPNanoSecond2SubSecond(NanoSecondValue);

	/* Write the offset (positive or negative) in the Time stamp update high and low registers. */
	ETH_SetPTPTimeStampUpdate(Sign, SecondValue, SubSecondValue);
	/* Set Time stamp control register bit 2 (Time stamp init). */
	ETH_InitializePTPTimeStamp();
	/* The Time stamp counter starts operation as soon as it is initialized
	 * with the value written in the Time stamp update register. */
	while(ETH_GetPTPFlagStatus(ETH_PTP_FLAG_TSSTI) == SET);
}


HAL_StatusTypeDef HAL_ETH_TransmitFrame_TimeStamp(ETH_HandleTypeDef *heth, uint32_t FrameLength, ETH_TimeStamp *TimeStamp)
{
	  uint32_t timeout = 0, bufcount = 0, size = 0, i = 0;

		/* Initialize the timestamp. */
		TimeStamp->TimeStampHigh = 0;
		TimeStamp->TimeStampLow = 0;

		/* Check timestamp.  */
	  if (TimeStamp == NULL)
	  {
	    /* Return ERROR: Bad timestamp structure */
			return HAL_ERROR;
	  }

	  /* Process Locked */
	  __HAL_LOCK(heth);

	  /* Set the ETH peripheral state to BUSY */
	  heth->State = HAL_ETH_STATE_BUSY;

	  if (FrameLength == 0)
	  {
	    /* Set ETH HAL state to READY */
	    heth->State = HAL_ETH_STATE_READY;

	    /* Process Unlocked */
	    __HAL_UNLOCK(heth);

	    return  HAL_ERROR;
	  }

	  /* Check if the descriptor is owned by the ETHERNET DMA (when set) or CPU (when reset) */
	  if(((heth->TxDesc)->Status & ETH_DMATXDESC_OWN) != (uint32_t)RESET)
	  {
	    /* OWN bit set */
	    heth->State = HAL_ETH_STATE_BUSY_TX;

	    /* Process Unlocked */
	    __HAL_UNLOCK(heth);

	    return HAL_ERROR;
	  }

	  /* Get the number of needed Tx buffers for the current frame */
	  if (FrameLength > ETH_TX_BUF_SIZE)
	  {
	    bufcount = FrameLength/ETH_TX_BUF_SIZE;
	    if (FrameLength % ETH_TX_BUF_SIZE)
	    {
	      bufcount++;
	    }
	  }
	  else
	  {
	    bufcount = 1;
	  }
	  if (bufcount == 1)
	  {

		/* Set transmit timestamp enable */
		heth->TxDesc->Status |= ETH_DMATXDESC_TTSE;
		/* Set LAST and FIRST segment */
	    heth->TxDesc->Status |=ETH_DMATXDESC_FS|ETH_DMATXDESC_LS;
	    /* Set frame size */
	    heth->TxDesc->ControlBufferSize = (FrameLength & ETH_DMATXDESC_TBS1);
	    /* Set Own bit of the Tx descriptor Status: gives the buffer back to ETHERNET DMA */
	    heth->TxDesc->Status |= ETH_DMATXDESC_OWN;
	  }
	  else
	  {
	    for (i=0; i< bufcount; i++)
	    {
	      /* Clear FIRST and LAST segment bits */
	      heth->TxDesc->Status &= ~(ETH_DMATXDESC_FS | ETH_DMATXDESC_LS);

	      if (i == 0)
	      {
	        /* Setting the first segment bit */
	        heth->TxDesc->Status |= ETH_DMATXDESC_FS;
	      }

	      /* Program size */
	      heth->TxDesc->ControlBufferSize = (ETH_TX_BUF_SIZE & ETH_DMATXDESC_TBS1);

	      if (i == (bufcount-1))
	      {
	    	  /* Set transmit timestamp enable on last descriptor */
	    	  heth->TxDesc->Status |= ETH_DMATXDESC_TTSE;

	        /* Setting the last segment bit */
	        heth->TxDesc->Status |= ETH_DMATXDESC_LS;
	        size = FrameLength - (bufcount-1)*ETH_TX_BUF_SIZE;
	        heth->TxDesc->ControlBufferSize = (size & ETH_DMATXDESC_TBS1);
	      }

	      /* Set Own bit of the Tx descriptor Status: gives the buffer back to ETHERNET DMA */
	      heth->TxDesc->Status |= ETH_DMATXDESC_OWN;

	      if (i < (bufcount - 1))
	      {
	    	  /* point to next descriptor */
	    	  heth->TxDesc = (ETH_DMADescTypeDef *)(heth->TxDesc->Buffer2NextDescAddr);
	      }
	    }
	  }

	  /* When Tx Buffer unavailable flag is set: clear it and resume transmission */
	  if (((heth->Instance)->DMASR & ETH_DMASR_TBUS) != (uint32_t)RESET)
	  {
	    /* Clear TBUS ETHERNET DMA flag */
	    (heth->Instance)->DMASR = ETH_DMASR_TBUS;
	    /* Resume DMA transmission*/
	    (heth->Instance)->DMATPDR = 0;
	  }

	  /* Wait for ETH_DMATxDesc_TTSS flag to be set */
	  do
	  {
	    timeout++;
	  } while (!(heth->TxDesc->Status & ETH_DMATXDESC_TTSS) && (timeout < PHY_READ_TO));

	  /* Fill in time stamp of no timeout */
	  if (timeout < PHY_READ_TO)
	  {
			/* Fill in the time stamp */
			TimeStamp->TimeStampHigh = heth->TxDesc->TimeStampHigh;
			TimeStamp->TimeStampLow = heth->TxDesc->TimeStampLow;
	  }

	  /* Clear the DMATxDescToSet status register TTSS flag */
	  heth->TxDesc->Status &= ~ETH_DMATXDESC_TTSS;

	  /* Selects the next DMA Tx descriptor list for next buffer to send */
	  heth->TxDesc = (ETH_DMADescTypeDef*) (heth->TxDesc->Buffer2NextDescAddr);


	  /* Set ETH HAL State to Ready */
	  heth->State = HAL_ETH_STATE_READY;

	  /* Process Unlocked */
	  __HAL_UNLOCK(heth);

	  /* Return function status */
	  return HAL_OK;
}

