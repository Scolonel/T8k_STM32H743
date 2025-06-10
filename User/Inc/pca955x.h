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
#define ERR_SJDSU 0x20         // ������ ��������� ��� ��������� JDSU
#define ERR_DB_NAME 0x40 // ������ ��������������� ����� �� �.�. (0x40)
#define ERR_DEVCFG 0x80 // ������ ������������ ������� (0x80)
#define ERR_SPECTR 0x100 // ������ ���������� ������! (0x100)
#define ERR_USERSET 0x200 // ������ ��������������� ��������� (0x200)
#define ERR_EVENTS 0x400 // ������ ��������� �������� ��������������(0x400)
#define ERR_OTDRSET 0x800 // ������ ��������� ���������  �������������(0x800)
#define ERR_SDCard 0x1000 // �� ����������� �����
#define CLR_SDCard 0x2000 // ������� �����
#define NOT_RVOTDR 0x10
#define CFG_ERR_RVOTDR 0x20
#define EXPANDERS_ERR 0x40
#define LASER_ERR 0x80
#define NOT_PCA9555_ESP 0x200
#define CFG_ERR_PCA9555_ESP 0x400
#define NOT_EEPROM_24SR16 0x800
    
#define REMOUT 0x01   // ��������������
#define MONITOR 0x02   // ����������
#define ENBL 0x01   // ON
#define DSBL 0x00   // OFF
#define On 0x01   // ON
#define Off 0x00   // OFF
#define WRTz 0x01   // ������
#define RDz 0x00   // ������
#define UPn 0x01   // ����������
#define DWn 0x00   // ����������

// i2C
#define EEPROM24LC128 0x50   // ������� ����� EEPROM24LC128
#define EEPROM24SR16 0x56   // ������� ����� EEPROM
#define EXPWRCTRL 0x20   // ����� ����������� ���������� �������� � �������� 9555
#define COMMCTRL 0x21   // ����� ����������� ���������� �� ���������� ����� 16 chanel
#define KEYBOARD 0x38   // ����� ����������� ����������  8 chanel 9554
#define RVOTDR 0x2E   // ����� ��������� ����������� ���������� 70� � ��������
#define RVBASEA 0x2C   // ������� ����� �������� ����������� �������� ������
#define EN_5V_VCC 0x08   // ��� ��������� 5� 
#define EN_5V_EXT 0x20   // ��� ��������� 5� USB

    
    

// ������ ������ �������� � EEPROM ���������� (���������� ��� ���������� 128���� (16�����) �� 64 ����� ��������)
   // ���������� �� ���������� 512���� (64�����) �������� 128 ����
#define ADR_ConfigDevice 0x0   // ����� �������� ������������ ������� ������ 0�80
#define ADR_UserMeasConfig 0x200   // ����� �������� ��������(���������������� ���������) ��� ������������� ��� ���������
#define ADR_CoeffPM 0x400   // ����� �������� �������� ��������� ������ � 768 �����??? 0�3200 1600*2*4
    // � ������ ���� 2048*2*4
#define ADR_BatSave 0x8000   // ����� �������� ������ ��������� (���� 1024*4)
//#define ADR_EventSet 0x300   // ����� �������� �������� ���������� � 512 �����???
//#define ADR_ComplJDSU 0x500   // ����� �������� ����� ������
//#define ADR_ReflSetting 0x600   // ����� �������� ��������� ��������� ������������
   // ������ ��������� � "0" ����� ������ ���� ������� ��������
    
// ���� ���������� ������������
#define ONG 0x01   // ��� ��������� ����������
#define CW 0x02   // ��� ��������� CW
#define SW_L 0x04   // ��� ��������� LOW bit Switch Place
#define PM_K3 0x08   // ��� ��������� 3 ���������� ������� ����������
#define SWRS1K2 0x10   // ��� ������������ ����������� UART
#define PM_K2 0x20   // // ��� ��������� 2 ���������� ������� ����������
#define SW_H 0x40   // ��� ��������� HIGH bit Switch Place
#define PM_K1 0x80   // // ��� ��������� 2 ���������� ������� ����������
   

/**
  * @}
  */
   
   
uint32_t GetExpand (void); // ��������� ��������� ����� Expanders (9554 b 9555), ����� ������ 23-16:9554,15-0:9555
//������������ PCA9555 ( ����������� ) ������ 0x20
// ������������� ��� ����� P0-P7, P8-P15 output  �������� �� �� ��� ����� ������ ��� ������ ���� ��� �����
HAL_StatusTypeDef ConfigKBRD (void);

HAL_StatusTypeDef CtrlExpand (uint16_t PinOut, uint16_t Mask ); // ���������� ������ ������� Expanders ( 9555), 

uint32_t BeginConfig (void); // ��������� ������������

// ������/ ������ �������� �/�� EEPROM
//������� ������ EEPROM
HAL_StatusTypeDef EEPROM_read(void *buff, uint16_t address, uint32_t byte);
//������� ������ EEPROM
HAL_StatusTypeDef EEPROM_write(void *buff, uint16_t address, uint32_t byte);
// ������ � ������ ��������������� ���������
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
