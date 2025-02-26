/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
//#include "SysT7Knew.h"
#include "system.h"
  
  extern I2C_HandleTypeDef hi2c2;

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
      extern uint16_t Dummy; // пустое чтение буфферов UART

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void myBeep (unsigned sound);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Beep_Pin GPIO_PIN_2
#define Beep_GPIO_Port GPIOE
#define EXT_POWER_Pin GPIO_PIN_3
#define EXT_POWER_GPIO_Port GPIOE
#define NEX_RX_Pin GPIO_PIN_6
#define NEX_RX_GPIO_Port GPIOF
#define NEX_TX_Pin GPIO_PIN_7
#define NEX_TX_GPIO_Port GPIOF
#define KTS_Pin GPIO_PIN_2
#define KTS_GPIO_Port GPIOA
#define FREE1_Pin GPIO_PIN_7
#define FREE1_GPIO_Port GPIOA
#define PM_SCLK_Pin GPIO_PIN_4
#define PM_SCLK_GPIO_Port GPIOC
#define FREE2_Pin GPIO_PIN_5
#define FREE2_GPIO_Port GPIOC
#define LD1_Pin GPIO_PIN_0
#define LD1_GPIO_Port GPIOB
#define V_BAT_Pin GPIO_PIN_11
#define V_BAT_GPIO_Port GPIOF
#define IDC1_Pin GPIO_PIN_13
#define IDC1_GPIO_Port GPIOF
#define IDC0_Pin GPIO_PIN_14
#define IDC0_GPIO_Port GPIOF
#define LEDSTART_Pin GPIO_PIN_15
#define LEDSTART_GPIO_Port GPIOF
#define BUT_START_Pin GPIO_PIN_14
#define BUT_START_GPIO_Port GPIOE
#define BUT_START_EXTI_IRQn EXTI15_10_IRQn
#define OPT_RX_Pin GPIO_PIN_12
#define OPT_RX_GPIO_Port GPIOB
#define LMOD_TX_Pin GPIO_PIN_13
#define LMOD_TX_GPIO_Port GPIOB
#define STLINK_RX_Pin GPIO_PIN_8
#define STLINK_RX_GPIO_Port GPIOD
#define STLINK_TX_Pin GPIO_PIN_9
#define STLINK_TX_GPIO_Port GPIOD
#define PWR_EXT_Pin GPIO_PIN_10
#define PWR_EXT_GPIO_Port GPIOD
#define IS1_Pin GPIO_PIN_12
#define IS1_GPIO_Port GPIOD
#define IS2_Pin GPIO_PIN_13
#define IS2_GPIO_Port GPIOD
#define KTB_Pin GPIO_PIN_14
#define KTB_GPIO_Port GPIOD
#define SW_SD_Pin GPIO_PIN_4
#define SW_SD_GPIO_Port GPIOG
#define KTA_Pin GPIO_PIN_5
#define KTA_GPIO_Port GPIOG
#define PM_DOUT_Pin GPIO_PIN_6
#define PM_DOUT_GPIO_Port GPIOG
#define PM_CS_Pin GPIO_PIN_7
#define PM_CS_GPIO_Port GPIOG
#define JTMS_Pin GPIO_PIN_13
#define JTMS_GPIO_Port GPIOA
#define JTCK_Pin GPIO_PIN_14
#define JTCK_GPIO_Port GPIOA
#define LD2_Pin GPIO_PIN_1
#define LD2_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
