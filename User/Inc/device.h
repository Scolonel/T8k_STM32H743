// конфигурация пинов под плату

#ifndef __DEVICE_H
#define __DEVICE_H

#define NUM_SSP 0 //номер порта SSP на котором FLASH
#define MEM_FL0 0 // инициализация SSP для флэш на 0 порт
#define MEM_FL1 1 // инициализация SSP для флэш на 1 порт
#define SPI_ALT 2 // SSP для альтеры
#define SPI_PM 3    // SSP для измерителя
  
#define CLK_72MHz_EN       PINSEL5_bit.P2_18  // CLKOUT0 - 72 MHz
#define CS_BANK_3          PINSEL4_bit.P2_15  // CS3 for Ext/Bank3 - обращение к альтера
// Сигналы управления питанием
#define POW_DET_ON         (0x80)  // P7 (PWR_TSTR_ON)
#define POW_REF_ON         (0x100)  // P8 (PWR_OTDR_ON)
//#define V_ALT_ON           (1UL<<14)  // P4.14 (159) 
#define POWDET(a)    ((a>0)?(CtrlExpand(POW_DET_ON, POW_DET_ON)):(CtrlExpand(0, POW_DET_ON)))  /* set POW_DET_ON to high/low */
#define POWREF(a)    ((a>0)?(CtrlExpand(POW_REF_ON, POW_REF_ON)):(CtrlExpand(0, POW_REF_ON)))  /* set POW_REF_ON to high/low */
//#define POWALT(a)    ((a>0)?(FIO4SET |= V_ALT_ON):(FIO4CLR |= V_ALT_ON))  /* set V_ALT_ON to high/low */
#define EXT_POW           HAL_GPIO_ReadPin(PWR_EXT_GPIO_Port, PWR_EXT_Pin)  // PD10 (79) // сигнал внешнего питания 0-внешнее питание
// пины конфигураторы железа
#define TEST_VER           (1UL<<26)  // P3.26 (55) // сигнал контроля версий
#define TEST_PIN2         (1UL<<21)  // P1.21 (72) пин контроля новых плат июнь2012 (перепутана клава)
#define ENA_PIL           (1UL<<23)  // P1.23 (76) // сигнал переназначения пинов управления лазером в постоянном излучении и генератора пилы
                                      //PHLD - переключается на пин P0.29 (61) P0.26 (12) - AOUT - для генерации пилы,

// сигналы управления высоким напряжением -  в новых платах 
#define HV_CTRL           (0x02)  // P1 (CTRL) // сигнал пониженного напряжения
#define HV_SHDN           (0x40)  // P6 (SHDN) // сигнал управления высоким напряжением
#define HV_LOW(a)    ((a>0)?(CtrlExpand (HV_CTRL, HV_CTRL)):(CtrlExpand (0, HV_CTRL)))  /* set HIGH VOLT low (1) */
#define HV_SW(a)    ((a>0)?(CtrlExpand (HV_SHDN, HV_SHDN)):(CtrlExpand (0, HV_SHDN)))  /* set ON/OFF HigHVolt  */

// Сигналы управления другие
#define LED_G(a) ((a>0)?(HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET)):(HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET)))

#define LED_KT(a)    ((a>0)?(HAL_GPIO_WritePin(KTS_GPIO_Port, KTS_Pin, GPIO_PIN_SET)):(HAL_GPIO_WritePin(KTS_GPIO_Port, KTS_Pin, GPIO_PIN_RESET))) /* set LED_START to high/low */

//#define LEDSTART         (1UL<<5)  // P1.5 (156) 

#define LED_START(a)    ((a>0)?(HAL_GPIO_WritePin(LEDSTART_GPIO_Port, LEDSTART_Pin, GPIO_PIN_SET)):(HAL_GPIO_WritePin(LEDSTART_GPIO_Port, LEDSTART_Pin, GPIO_PIN_RESET))) /* set LED_START to high/low */
#define PIN_PHLD         (0x400)  // P10 (PHLD) 
#define PHLD(a)    ((a>0)?(CtrlExpand (PIN_PHLD, PIN_PHLD)):(CtrlExpand (0,PIN_PHLD)))  /* set PHLD to high/low */
#define PIN_LMOD         (1UL<<16)  // P3.16 (137)  // переделка
#define LMOD(a)    ((a>0)?(FIO3SET |= PIN_LMOD):(FIO3CLR |= PIN_LMOD))  /* set LMOD to high/low */
#define PIN_REDEYE         (0x200)  // P9 (VFL_ON) 
#define REDEYE(a)    ((a>0)?(CtrlExpand (PIN_REDEYE, PIN_REDEYE)):(CtrlExpand (0, PIN_REDEYE)))  /* set REDEYE to high/low */
#define PIN_SWFLTR         (0x01)  // P0 (OTDR_FLTR) 
#define SW_FLTR(a)    ((a>0)?(CtrlExpand (PIN_SWFLTR, PIN_SWFLTR)):(CtrlExpand (0, PIN_SWFLTR)))  /* set SW_FLTR to high/low */
#define PIN_ADCEN         (1UL<<12)  // P0.12 (41) 
#define ADC_DATA_EN(a)    ((a>0)?(FIO0SET |= PIN_ADCEN):(FIO0CLR |= PIN_ADCEN))  /* set ADC_DATA_EN to high/low */
#define PIN_LSEL0         (0x1000)  // P12 (LSEL0) 
#define LSEL0(a)    ((a>0)?(CtrlExpand (PIN_LSEL0, PIN_LSEL0)):(CtrlExpand (0, PIN_LSEL0)))  /* set LSEL0 to high/low */
#define PIN_LSEL1         (0x800)  // P11 (LSEL1) 
#define LSEL1(a)    ((a>0)?(CtrlExpand (PIN_LSEL1, PIN_LSEL1)):(CtrlExpand (0, PIN_LSEL1)))  /* set LSEL1 to high/low */
#define PIN_K1         (0x20)  // P5 (1) - ключи переключения диапазонов
#define PIN_K2         (0x10)  // P4 (2) - ключи переключения диапазонов
#define PIN_K3         (0x08)  // P3 (3) - ключи переключения диапазонов
#define KEY1(a)    ((a>0)?(CtrlExpand (PIN_K1, PIN_K1)):(CtrlExpand (0, PIN_K1)))  /* set K1 to high/low */
#define KEY2(a)    ((a>0)?(CtrlExpand (PIN_K2, PIN_K2)):(CtrlExpand (0, PIN_K2)))  /* set K2 to high/low */
#define KEY3(a)    ((a>0)?(CtrlExpand (PIN_K3, PIN_K3)):(CtrlExpand (0, PIN_K3)))  /* set K3 to high/low */
// Test signal
#define TEST_PIN1         (0x04)  // P2 (FREE6) 
#define TST_P1(a)    ((a>0)?(CtrlExpand (TEST_PIN1, TEST_PIN1)):(CtrlExpand (0, TEST_PIN1)))  /* set SSEL P1.20 to high/low */
// пины разрешения зонд импульса
#define PIN_EN_ZI1         (0x4000)  // P14 (EN_ZI1) 
#define PIN_EN_ZI2         (0x2000)  // P13 (EN_ZI2) 
#define PIN_EN_ZI3         (0x8000)  // P15 (EN_ZI3) 
#define ENZ1(a)    ((a>0)?(CtrlExpand (PIN_EN_ZI1, PIN_EN_ZI1)):(CtrlExpand (0, PIN_EN_ZI1)))  /* set EN_ZI1 to high/low */
#define ENZ2(a)    ((a>0)?(CtrlExpand (PIN_EN_ZI2, PIN_EN_ZI2)):(CtrlExpand (0, PIN_EN_ZI2)))  /* set EN_ZI1 to high/low */
#define ENZ3(a)    ((a>0)?(CtrlExpand (PIN_EN_ZI3, PIN_EN_ZI3)):(CtrlExpand (0, PIN_EN_ZI3)))  /* set EN_ZI1 to high/low */

#define PM_CS(a)    ((a>0)?(PM_CS_GPIO_Port->BSRR = PM_CS_Pin):(PM_CS_GPIO_Port->BSRR = (uint32_t)PM_CS_Pin << 16))  /* set CS_PM PB.04 to high/low */
#define PM_CLK(a)    ((a>0)?(PM_SCLK_GPIO_Port->BSRR = PM_SCLK_Pin):(PM_SCLK_GPIO_Port->BSRR = (uint32_t)PM_SCLK_Pin << 16))  /* set CLK_PM PB.08 to high/low */
#define GET_PM_DATA    HAL_GPIO_ReadPin(PM_DOUT_GPIO_Port, PM_DOUT_Pin)  /* set DATA_PM PB.05 to high/low */


// Strob_сбора данных START (P3.21) (175)
#define START_MASK        (1UL<<21)
#define START_FDIR        FIO3DIR
#define START_FSET        FIO3SET
#define START_FCLR        FIO3CLR
#define START_FIO         FIO3PIN
#define STARTPP(a)    ((a>0)?(START_FSET |= START_MASK):(START_FCLR |= START_MASK))  /* set START to high/low */

// сигналы SSEL для Альтеры и Измерителя соотв.
#define CSALT           (1 << 6) // port   P0.6
#define CSPM            (1 << 14) // port  P0.14,
#define CSFL0            (1 << 16) // port P0.16,
#define CSFL1            (1 << 21) // port P4.21

//#define FLO_CS(a,b) #if (a > 0) \
                    
//#define FL1_CS(a)    ((a>0)?(FIO4SET |= CSFL1):(FIO4CLR |= CSFL1))  /* set SSEL P0.16 to high/low */

//// Сигналы управления LCD
//#define BUS_LCD_DIR    FIO4DIR0 //регистр ШИНЫ
//#define BUS_LCD_WRT    FIO4PIN0
//#define BUS_LCD_SET    FIO4SET0
//#define BUS_LCD_CLR    FIO4CLR0
//#define REG_LCD_CSS    FIO4DIR // регистр установки сигналов CS для LCD
//#define REG_LCD_ENRS    FIO2DIR // регистр установки сигналов RS EN для LCD
//#define REG_LCD_LIGHT    FIO0DIR // регистр установки сигналов LIGHT для LCD
////#define LCD_CONTR         PINSEL3_bit.P1_26  // P1.26 us PWM 
//
//#define CS1LCD            (1 << 17) // CS1, P4.17 (104)
//#define CS2LCD            (1 << 18) // CS2, P4.18 (105)
//#define RSLCD            (1 << 24) // RS, P2.24 (53) 
//#define ENLCD            (1 << 25) // EN, P2.25 (54)
//#define LIGHTLCD         (1UL<<17) // LIGHT, P2.17 (95)
//#define LCD_CS1(a)    ((a>0)?(FIO4SET |= CS1LCD):(FIO4CLR |= CS1LCD))  /* CS1 P4.17 to high/low */
//#define LCD_CS2(a)    ((a>0)?(FIO4SET |= CS2LCD):(FIO4CLR |= CS2LCD))  /* CS2 P4.18 to high/low */
//#define LCD_RS(a)    ((a>0)?(FIO2SET |= RSLCD):(FIO2CLR |= RSLCD))  /* RS P2.24 to high/low */
//#define LCD_EN(a)    ((a>0)?(FIO2SET |= ENLCD):(FIO2CLR |= ENLCD))  /* EN P2.25 to high/low */
//#define LCD_LIGHT(a)    ((a>0)?(FIO2SET |= LIGHTLCD):(FIO2CLR |= LIGHTLCD))  /* LIGHT P0.0 to high/low */

//Клавиатура P2.8... P2.14
//#define BTN_START   (1UL<<10)
//// #define BTN_UP      (1UL<<11)
//// #define BTN_DOWN    (1UL<<9)
//// #define BTN_LEFT    (1UL<<13)
//// #define BTN_RIGHT   (1UL<<14)
//#define BTN_OK      (1UL<<8)
//#define BTN_MENU    (1UL<<12)
////#define BNS_MASK   (BTN_UP + BTN_DOWN + BTN_LEFT + BTN_RIGHT + BTN_OK + BTN_MENU) // не используется 23.06.2022

//#define POINTSIZE    4096 // число активных точек, размер числа точек графика

//#define NEXSIZE    4096 //0x1000
//#define OUTSIZE    4096 //0x1000
//#define RAWSIZE    0x1200  //(4096+288)
#define FLTSIZE    5 // размерность фильтра






#endif /* end __DEVICE_H */

