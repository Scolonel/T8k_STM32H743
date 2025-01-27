/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    rtc.h
  * @brief   This file contains all the function prototypes for
  *          the rtc.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RTC_H__
#define __RTC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "stm32h7xx_hal_rtc.h"
/* USER CODE END Includes */

extern RTC_HandleTypeDef hrtc;

/* USER CODE BEGIN Private defines */
typedef struct {
    DWORD RTC_Sec;     /* Second value - [0,59] */
    DWORD RTC_Min;     /* Minute value - [0,59] */
    DWORD RTC_Hour;    /* Hour value - [0,23] */
    DWORD RTC_Mday;    /* Day of the month value - [1,31] */
    DWORD RTC_Mon;     /* Month value - [1,12] */
    DWORD RTC_Year;    /* Year value - [0,4095] */
    DWORD RTC_Wday;    /* Day of week value - [0,6] */
    DWORD RTC_Yday;    /* Day of year value - [1,365] */
} RTCTime;

extern RTCTime current_time;
extern RTCTime TimeSaveOTDR; // время сохраненной рефлектограммы
extern RTCTime TimeSaveOLT; // время сохраненной рефлектограммы

/* USER CODE END Private defines */

void MX_RTC_Init(void);

/* USER CODE BEGIN Prototypes */
//void     k_CalendarBkupInit(void);
void     k_BkupSaveParameter(uint32_t address, uint32_t data);
uint32_t k_BkupRestoreParameter(uint32_t address);

void k_SetTime  (RTC_TimeTypeDef *Time);
void k_GetTime  (RTC_TimeTypeDef *Time);
void k_SetDate  (RTC_DateTypeDef *Date);
void k_GetDate  (RTC_DateTypeDef *Date);

void RTCSetTime( RTCTime Time );

void GetFolder (char *Str); // получение названия папки по текущему времени каждые 7 дней меняется папка

RTCTime RTCGetTime(void); // получение времени в формате T7kAR из рабочих регистров
uint32_t get_fattime_RTC (void);
unsigned int TotalSec( RTCTime CurrentTime); // подсчет общего времени в сек
void Sec2Date( unsigned long TimeSec, RTCTime* CurrentTime); // перевод секунд в дату
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __RTC_H__ */

