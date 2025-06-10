/**
  ******************************************************************************
  * @file    ili9341.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    18-February-2014
  * @brief   This file contains all the functions prototypes for the ili9341.c
  *          driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PCA955x_H
#define __PCA955x_H

/* Includes ------------------------------------------------------------------*/
//#include "..\Common\lcd.h"
//#include "stm32F3xx_hal.h"
#include "main.h"


extern I2C_HandleTypeDef hi2c1;


#ifdef __cplusplus
 extern "C" {
#endif 

/** @addtogroup BSP
  * @{
  */ 

/** @addtogroup Component
  * @{
  */ 
  
  
/**
  * @}
  */ 

#define NOT_PCA9554_KB 0x01
#define CFG_ERR_PCA9554_KB 0x02
#define NOT_PCA9555 0x04
#define CFG_ERR_PCA9555 0x08
#define NOT_EEPROM_24LC128 0x10
#define ERR_SJDSU 0x20         // ошибка установок для измерения JDSU
#define ERR_DB_NAME 0x40 // ошибка альтернативного имени от Д.Б. (0x40)
#define ERR_DEVCFG 0x80 // плохая конфигурация прибора (0x80)
#define ERR_SPECTR 0x100 // плохая спектралка правим! (0x100)
#define ERR_USERSET 0x200 // плохие пользоваельские настройки (0x200)
#define ERR_EVENTS 0x400 // плохие настройки контроля рефлектограммы(0x400)
#define ERR_OTDRSET 0x800 // плохие настройки установок  рефлектометра(0x800)
#define ERR_SDCard 0x1000 // не монтируется карта
#define CLR_SDCard 0x2000 // чистили карта
#define NOT_RVOTDR 0x10
#define CFG_ERR_RVOTDR 0x20
#define EXPANDERS_ERR 0x40
#define LASER_ERR 0x80
#define NOT_PCA9555_ESP 0x200
#define CFG_ERR_PCA9555_ESP 0x400
#define NOT_EEPROM_24SR16 0x800
    
#define REMOUT 0x01   // дисстанционное
#define MONITOR 0x02   // мониторинг
#define ENBL 0x01   // ON
#define DSBL 0x00   // OFF
#define On 0x01   // ON
#define Off 0x00   // OFF
#define WRTz 0x01   // запись
#define RDz 0x00   // чтение
#define UPn 0x01   // увеличение
#define DWn 0x00   // уменьшение

// i2C
#define EEPROM24LC128 0x50   // базовый адрес EEPROM24LC128
#define EEPROM24SR16 0x56   // базовый адрес EEPROM
#define EXPWRCTRL 0x20   // адрес расширителя управления питанием и контроля 9555
#define COMMCTRL 0x21   // адрес расширителя управления на аналоговой плате 16 chanel
#define KEYBOARD 0x38   // адрес расширителя клавиатуры  8 chanel 9554
#define RVOTDR 0x2E   // адрес резистора регулировки напряжения 70В и смещения
#define RVBASEA 0x2C   // базовый адрес резистор регулировки мощности лазера
#define EN_5V_VCC 0x08   // бит включения 5В 
#define EN_5V_EXT 0x20   // бит включения 5В USB

    
    

// Адреса блоков хранения в EEPROM переменных (изначально для микросхемы 128кбит (16КБайт) по 64 байта страница)
   // переделаем на микросхемы 512кбит (64КБайт) страница 128 байт
#define ADR_ConfigDevice 0x0   // адрес хранения конфигурации прибора размер 0х80
#define ADR_UserMeasConfig 0x200   // адрес хранения настроек(пользовательских установок) для востановления при включении
#define ADR_CoeffPM 0x400   // адрес хранения настроек установки уровня с 768 байта??? 0х3200 1600*2*4
    // в старом было 2048*2*4
#define ADR_BatSave 0x8000   // адрес хранения уровня батарейки (пока 1024*4)
//#define ADR_EventSet 0x300   // адрес хранения настроек измерителя с 512 байта???
//#define ADR_ComplJDSU 0x500   // адрес хранения ячеек памяти
//#define ADR_ReflSetting 0x600   // адрес хранения установок измерения релектометра
   // раньше хранилось в "0" файле теперь надо хранить отдельно
    
// пины уроавления расширителем
#define ONG 0x01   // бит включения Генератора
#define CW 0x02   // бит включения CW
#define SW_L 0x04   // бит включения LOW bit Switch Place
#define PM_K3 0x08   // бит включения 3 управление ключами диапазонов
#define SWRS1K2 0x10   // бит переключения оптического UART
#define PM_K2 0x20   // // бит включения 2 управление ключами диапазонов
#define SW_H 0x40   // бит включения HIGH bit Switch Place
#define PM_K1 0x80   // // бит включения 2 управление ключами диапазонов
   

/**
  * @}
  */
   
   
uint32_t GetExpand (void); // получение состояния пинов Expanders (9554 b 9555), одним словом 23-16:9554,15-0:9555
//конфигурация PCA9555 ( клавитатура ) адресс 0x20
// устанавливаем как входы P0-P7, P8-P15 output  несмотря на то что после сброса уже должно быть как входд
HAL_StatusTypeDef ConfigKBRD (void);

HAL_StatusTypeDef CtrlExpand (uint16_t PinOut, uint16_t Mask ); // управление пинами выходов Expanders ( 9555), 

uint32_t BeginConfig (void); // начальная конфигурация

// запись/ чтение структур в/из EEPROM
//функция чтения EEPROM
HAL_StatusTypeDef EEPROM_read(void *buff, uint16_t address, uint32_t byte);
//функция записи EEPROM
HAL_StatusTypeDef EEPROM_write(void *buff, uint16_t address, uint32_t byte);
// разбор и запись соответствующей структуры
void WriteNeedStruct (int NumStruct);

/** @defgroup tft _Exported_Functions
  * @{
  */ 

/** @defgroup ILI9341_Exported_Functions
  * @{
  */ 

/* LCD driver structure */
      
#ifdef __cplusplus
}
#endif

#endif /* __TFT5INCH_H */

/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */
  
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
