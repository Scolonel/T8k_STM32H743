/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    rtc.c
  * @brief   This file provides code for the configuration
  *          of the RTC instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "rtc.h"

/* USER CODE BEGIN 0 */

  #define YearSecV  31622400
  #define YearSec   31536000
  #define DaySec    86400
  #define Y2000Sec  946684800 // секунды до 2000 года

RTCTime TimeSaveOTDR;
RTCTime TimeSaveOLT;
RTCTime current_time;

/* USER CODE END 0 */

RTC_HandleTypeDef hrtc;

/* RTC init function */
void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */
    // HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    //            HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
       // время в структуре
    unsigned int CurTime = TotalSec (RTCGetTime()); 
   //  1731664800 - 15 nov 2024 10.00.00
  if(CurTime < 1731664800)
  {
  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 10;
  sTime.Minutes = 0;
  sTime.Seconds = 0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_NOVEMBER;
  sDate.Date = 21;
  sDate.Year = 24;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */
  }
  /* USER CODE END RTC_Init 2 */

}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{

  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

  /* USER CODE END RTC_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* RTC clock enable */
    __HAL_RCC_RTC_ENABLE();
  /* USER CODE BEGIN RTC_MspInit 1 */

  /* USER CODE END RTC_MspInit 1 */
  }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
// здесь может быть код перевода времени в разные форматы
/**
  * @brief  Backup save parameter 
  * @param  address: RTC Backup data Register number.
  *                  This parameter can be: RTC_BKP_DRx where x can be from 0 to 19 to 
  *                                         specify the register.
  * @param  Data:    Data to be written in the specified RTC Backup data register.
  * @retval None
  */
void k_BkupSaveParameter(uint32_t address, uint32_t data)
{
  HAL_RTCEx_BKUPWrite(&hrtc,address,data);  
}

/**
  * @brief  Backup restore parameter. 
  * @param  address: RTC Backup data Register number.
  *                  This parameter can be: RTC_BKP_DRx where x can be from 0 to 19 to 
  *                                         specify the register. 
  * @retval None
  */
uint32_t k_BkupRestoreParameter(uint32_t address)
{
   return HAL_RTCEx_BKUPRead(&hrtc,address);  
}

/**
  * @brief  RTC Get time. 
  * @param  Time: Pointer to Time structure
  * @retval None
  */
void k_GetTime(  RTC_TimeTypeDef *Time)
{
   HAL_RTC_GetTime(&hrtc, Time, FORMAT_BIN);
}

/**
  * @brief  RTC Set time. 
  * @param  Time: Pointer to Time structure
  * @retval None
  */
void k_SetTime(  RTC_TimeTypeDef *Time)
{
   Time->StoreOperation = 0;
   Time->SubSeconds = 0;
   Time->DayLightSaving = 0;
   HAL_RTC_SetTime(&hrtc, Time, FORMAT_BIN);
}

/**
  * @brief  RTC Get date
  * @param  Date: Pointer to Date structure
  * @retval None
  */
void k_GetDate(  RTC_DateTypeDef *Date)
{
   HAL_RTC_GetDate(&hrtc, Date, FORMAT_BIN);
   
   if((Date->Date == 0) || (Date->Month == 0))
   {
     Date->Date = Date->Month = 1;
   }    
}

/**
  * @brief  RTC Set date
  * @param  Date: Pointer to Date structure
  * @retval None
  */
void k_SetDate(  RTC_DateTypeDef *Date)
{
   HAL_RTC_SetDate(&hrtc, Date, FORMAT_BIN);
}
//функция из t7kAR
// получение текущего времени, читаем регистры, переписываем в структкру
RTCTime RTCGetTime( void )
{
  RTC_TimeTypeDef CsTime = {0};
  RTC_DateTypeDef CsDate = {0};
  
  RTCTime LocalTime; // выходная структура
  // получим текущее время
  k_GetDate(&CsDate);
  HAL_Delay(1);
  k_GetTime(&CsTime);
  
  LocalTime.RTC_Sec = CsTime.Seconds;
  LocalTime.RTC_Min = CsTime.Minutes;
  LocalTime.RTC_Hour = CsTime.Hours;
  LocalTime.RTC_Mday = CsDate.Date;
  LocalTime.RTC_Wday = CsDate.WeekDay;
  LocalTime.RTC_Yday = 200;
  LocalTime.RTC_Mon = CsDate.Month;
  LocalTime.RTC_Year = CsDate.Year+2000;
  return ( LocalTime );
}
 //
void RTCSetTime( RTCTime Time )
{
  RTC_TimeTypeDef CsTime = {0};
  RTC_DateTypeDef CsDate = {0};
  
  CsTime.Seconds = Time.RTC_Sec;
  CsTime.Minutes = Time.RTC_Min;
  CsTime.Hours = Time.RTC_Hour;
  CsDate.Date = Time.RTC_Mday;
  CsDate.WeekDay = Time.RTC_Wday;
  //DOY = Time.RTC_Yday;
  CsDate.Month = Time.RTC_Mon;
  CsDate.Year = Time.RTC_Year%100;
  
  k_SetTime(&CsTime);
  k_SetDate(&CsDate);
  
  return;
}


uint32_t get_fattime_RTC (void)
{
  uint32_t res;
  RTC_DateTypeDef          Date;  
  RTC_TimeTypeDef          Time; 
  k_GetDate(&Date);  
  k_GetTime(&Time);   
  
  
  //res =  (((uint32_t)rtcdate.RTC_Year - 1980) << 25) // oae auei e ienaei a aia 2061
  res =  (((uint32_t)(Date.Year + 20 ) ) << 25)// так как храним год более 2000
    | ((uint32_t)Date.Month << 21)
      | ((uint32_t)Date.Date << 16)
        | (uint16_t)(Time.Hours << 11)
          | (uint16_t)(Time.Minutes << 5)
            | (uint16_t)(Time.Seconds >> 1);
  
  return res;
}

unsigned int TotalSec( RTCTime CurrentTime) // подсчет общего времени в сек
{
  int Year;
  unsigned int Month;
  unsigned int Date, days;
  unsigned int Hour;
  unsigned int Minut;
  unsigned int Sec, secs;
  unsigned int _Days[]={31,28,31,30,31,30,31,31,30,31,30,31};
  days = 0;
  secs = 0;
  Sec = CurrentTime.RTC_Sec; 
  Minut = CurrentTime.RTC_Min; 
  Hour = CurrentTime.RTC_Hour; 
  Date = CurrentTime.RTC_Mday; 
  Month = CurrentTime.RTC_Mon; 
  if ((Month > 12) || (Month == 0)) Month = 5; 
  Year = CurrentTime.RTC_Year%100; 
  if((!((Year) % 4))&&(Month>2)) days++;
  
  if (Year > 42) Year = 24; // при плохом годе поставим 2019 (попробуем)
  
  Year--;
  
  while(Year>=0)
  {
    if(!((Year)%4))days+=366;
    else days+=365;
    Year--;
  }
  while(Month-1>0)
    days+=_Days[(--Month)-1];
  
  days+=Date-1;
  
  secs=days*24*3600+Hour*3600+Minut*60+Sec;
  secs = secs + 946684800; // 2000год секунд
  //(946684800)
  
  return(secs);
  
  
}

void GetFolder (char *Str) // получение названия папки по текущему времени каждые 7 дней меняется папка
{
    unsigned int MDays[]={0,31,59,90,120,151,181,212,243,273,304,334};
  RTC_DateTypeDef          Date;  
  uint32_t AllDay;
  k_GetDate(&Date);  
  AllDay = MDays[Date.Month-1] + Date.Date;
  if((!((Date.Year) % 4))&&(Date.Month>2)) AllDay++;
  AllDay = AllDay/7+1;
  sprintf(Str, "%2d_%02d",Date.Year%100, AllDay);
}

void Sec2Date( unsigned long TimeSec, RTCTime* CurrentTime) // перевод секунд в дату
{
  
  unsigned int _Days[]={31,28,31,30,31,30,31,31,30,31,30,31};
  
  int Year = 2000;
  int Month;
  int Day;
  
  int Hour;
  int Minute;
  int Second;
  
  int tmp;
  
  TimeSec = TimeSec - Y2000Sec; // вычитаем секунды до 2000 года
  TimeSec = TimeSec - DaySec; // вычитаем секунды высокосного дня 2000 года
  
  if (TimeSec>0x80000000) TimeSec = 10*YearSecV-1; // это если текущие секунды больше 19 января 2038
  tmp = YearSec;
  while(TimeSec > tmp)
  {
    TimeSec = TimeSec - tmp;
    Year++;
    if(Year%4) tmp = YearSec;
    else tmp = YearSecV;
  }
  // получили год
  CurrentTime->RTC_Year = Year;
  Month = 1;
  
  while(TimeSec>_Days[Month-1]*DaySec)
  {
    TimeSec -= _Days[Month-1]*DaySec;
    Month++;
  }
  // Mounth
  CurrentTime->RTC_Mon = Month;
  
  Day = 1;
  
  while(TimeSec>DaySec)
  {
    TimeSec -= DaySec;
    Day++;    
  }
  // Day 
  CurrentTime->RTC_Mday = Day;
  
  Hour = TimeSec/3600;
  CurrentTime->RTC_Hour = Hour;
  
  TimeSec -= Hour*3600;
  
  Minute = TimeSec/60;
  CurrentTime->RTC_Min = Minute;
  
  TimeSec -= Minute*60;
  
  Second = TimeSec;
  CurrentTime->RTC_Sec = Second;
  
  
  return;
}

/* USER CODE END 1 */
