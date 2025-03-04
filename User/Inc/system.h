#ifndef __SYSTEM_H
#define __SYSTEM_H
// INCLUDES FILES
#define ON 1
#define OFF 0
#define YES 1
#define NO 0
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(*a))
// языки
#define Rus 0
#define Eng 1
#define Cze 2
#define MAX_LANG 4 // число разрешенных языков без учета 0-русского
//
#define MANUAL 0
#define AUTO 1
//#define MaxMemPM 3200
//#define MaxMemOTDR 200
//#define MaxMemOTDRExt 250
//#define MAXMEMALL ((CheckIDMEM)?(MaxMemOTDR+MaxMemOTDRExt):(MaxMemOTDR))
//число сообщений и языков
#define LANG_NUM 4  // число столбцов в таблице языков

#define MSG_NUM 59
#define CMD_NUM 35 //команды Nextion

#define TIMERE 500 //время цикла красного глаза по 500 мС

#define LIGHTSPEED 299792458
// для часов 
  #define YearSecV  31622400
  #define YearSec   31536000
  #define DaySec    86400
  #define Y2000Sec  946684800 // число секунд 2000 года

// размеры массивов 
//#define POINTSIZE    5200 //4096 // число активных точек, размер числа точек графика

//#define NEXSIZE    5200 //4096 //0x1000  //размер для НЕКСТИОН
//#define OUTSIZE    5200 //4096 //0x1000  // выходной массив
//#define RAWSIZE    5500 //4384 //0x1200  //(4096+288)
//#define FLTSIZE    5 // размерность фильтра
// массивы из первичной проверки...перекликаются с размерами объявленными ранее
#define SizeBuf_ADC_int 4
//#define SizeBuf_ADC 6000//8192
//#define SizeLogBuf 8000
//#define SizeBuf_DAC 75
//#define ResolutionADC  10


// для управления всякими UART
#define BUFSIZE   0x80
#define BUFSIZEUART1   0x80
#define BUFSIZEUART2   0x80

#define STOP_UART  0
#define START_UART  1
#define END_UART  2

#define BEG_ADD  64 // добавка для определения начала зонд импульса для разных диапазонов

//
#define LENGTH_LINE_NUM 7 // число длин линий (диапазонов)

#define NUM_VER                 56  // стартуем от 50 (22.01.2025) по этому номеру
// 55 03.03.2025 -  добавлены команды дистанционного управленя, режим и установка - запрос ЦАП
// 56 04.03.2025 - 3.02e запись коэфф по два байта при настройке
#define MAX_DB 1600 // максимальное значение дБ умноженное на 20 для обеспечения шага 0.05
#define SHAG 20.0  // делитель для индикации дБ
// получение идентификатора платы
#define GETIDPLT           (HAL_GPIO_ReadPin(IDC1_GPIO_Port, IDC1_Pin)<<1)+(HAL_GPIO_ReadPin(IDC0_GPIO_Port, IDC0_Pin))  // PD10 (79) // сигнал внешнего питания 0-внешнее питание

#define GETEXTPWR           (HAL_GPIO_ReadPin(EXT_POWER_GPIO_Port, EXT_POWER_Pin)) // PE3 (2) // сигнал внешнего питания 0-внешнее питание

//#define BELCORESIZE 224
#define ENAOLDLCD 0 //разрешение прорисовки на старый LCD 1-разрешено, 0-запрещено


#include "integer.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "stdint.h"
#include "math.h"
//#include "pm.h" // функции измерителя
#include "rtc.h" // 
//#include "fatfs.h"
#include "pca955x.h"
#include "buttons.h" 
#include "Att_mode.h"// //from T7kAR

//#include "sdmmc.h"
//#include "ff.h"
// из FATFS библиотеки
//#include "fatfs.h"
//#include "ff_gen_drv.h"
//#include "ff.h"
//#include "ffconf.h"

//#include "usbd_cdc_acm_if.h"
//#include "OTDR_Meas.h" //from T7kAR
//#include "HardSetDevice.h" // функции управления "железом" 
#include "usart.h" // for UART 

//#include "events.h" //from T7kAR
#include "devicedata.h"  //from T7kAR
#include "device.h"  //from T7kAR
//#include "modes.h"  //from T7kAR
#include "UartNEX.h"  //from T7kAR
#include "UartEXT.h"  //from T7kAR
//#include "drawfunctions.h" //from T7kAR
//#include "memflash.h"// //from T7kAR

extern const char* DeviceIDN[2];

extern  char DigitSet; //шаг изменеия устанавливаемого затухания
extern  char LvlBatInd; //индикатор уровня батарейки

      // тики опроса клавиатура взято из Т7К_АР
uint32_t GetSysTick( int Mode); // получение тиков 1 мС. 0 - получение счетчика от предыдущего сброса 1- сброс

void NEX_Transmit(uint8_t *Str);
// управление таймером в измерителе АВТОМАТЕ
WORD TimerPA (BYTE Set);

// check Speed Uart2
int CheckSpeedUart2 (void);
// for T5K 
extern unsigned int CheckErrMEM; // контроль 

extern WORD ProcBat , ProcBatInd; // процент баттареи


extern UART_HandleTypeDef huart5;

extern char ScreenReDraw; // признак необходимости перерисовать экран
extern char NeedSaveParam; // признак необходимости сохранить параметры
extern uint16_t CurrLevelDAC; //текущий уровень для ЦАП (востанавливаем из тех что храним в UserSet)

extern volatile unsigned char rawPressure;
extern volatile unsigned char rawPressKeyS; // признак необработанной нажатой клавиши S по прерыванию
extern volatile unsigned char KeyS; 
//extern char screen[1024] ;
extern uint8_t RX_BufNEX[BUFSIZEUART2];
extern char VerFW_LCD[25]; //версия ПО индикатора NEXION
extern char NameReadFile[32]; // глобальная имя файла при чтении в сохранении

extern volatile DWORD CntRXNEX;
extern volatile uint32_t RSDecYes; // здесь и признак приема по UART и чиcло байт при приеме от USB
extern volatile BYTE Uart2DecYes;
extern volatile DWORD RecievNEX;
extern volatile BYTE ReadyNEX; // готовность управления NEXTION
extern volatile BYTE g_WtRdyNEX;
extern volatile BYTE ProgFW_LCD; // признак программирования индикатора Nexion
extern volatile DWORD UART2Count; // счетчик приема байт UART2 от индикатора NEX - интересны в режиме программирования
extern volatile DWORD UART0Count; // счетчик приема байт UART0 для индикатора NEX - интересны в режиме программирования
extern volatile BYTE UART2Buffer[BUFSIZEUART2];
extern volatile WORD CountTimerUart2;
//extern unsigned short DataADC ;
extern DWORD TimeBegin; // время начала, для контроля начальной заставки
extern uint8_t WIDE_VER;
extern BYTE CurrLang; // текущий язык
extern unsigned short KeyP; // клавиши нажатые 
//extern int CntAccumulat; // счетчик накоплений
// кнопки клавиатуры
// режим работы прибора, переключает индикацию
extern  char ModeWork; 

// внешняя переменная для конфигурации пина управления (CW) лазером
//extern unsigned long PIN_PHLD;

extern const char *CmdNextion[CMD_NUM]; 
extern const char *MsgMass[MSG_NUM][LANG_NUM]; 
//extern const char MsgMass[MSG_NUM][LANG_NUM][32]; 
//extern const DWORD DelayPeriod[LENGTH_LINE_NUM];// задержка периода в тиках CreatDelay()~ 83.33 нс
//extern const DWORD NumPointsPeriod[LENGTH_LINE_NUM];
//extern const DWORD MultIndex[LENGTH_LINE_NUM];//множитель уменьшения частоты съема

extern const unsigned char fonts[2048];
extern char BufString[225];
//extern char BufString[60];
//extern const char *NumVer; // номер версии
extern char NumVer[10]; // номер версии

extern BYTE CheckIDMEM; // какая флэшь стоит? (0- 16 , 1- 32)

//extern unsigned int TimerValueJDSU; // текущее значение частоты приемника RS
//extern unsigned long TimerCR; // значение таймера 1 (его разница) при подсчете периода между 2-мя импульсами на входе 
// для индикатора
extern volatile BYTE g_GetStr; // признак ожидания получения строки из редактора. 
extern volatile BYTE g_NeedScr; // перерисовки экрана ! Аккуратно! надо проверить 
extern volatile BYTE g_FirstScr; // первый вход в экран, нужна полная перерисовка (заполнение), далее изменяем только нужные поля, для NEXTION
extern volatile BYTE IndexSmall; // переключатель для 2 км ,0 - 0.5 км, 1 - 2км 
extern volatile BYTE TypeLCD ; // тип индикатора в идентификаторе v-3.2(=0) s-3.5(=1) 
extern volatile BYTE g_NeedChkAnsvNEX; // признак получения строки из редактора.и ее проверка
extern volatile BYTE g_AutoSave; // признак режима авто сохранения измерений на выбраных длинах волн
extern volatile int NeedReturn; // необходимость вернуться в окно сохранения
//extern BYTE g_SetModeLW; // 16.11.2022 запоминаем установочную конфигурацию по длине волны Index 

//extern BYTE MemTable[MaxMemOTDR+1]; // таблица рефлектограмм ячейки памяти меняется в памяти до MaxMemOTDR
//extern BYTE MemTableExt[MaxMemOTDRExt+1]; // таблица рефлектограмм расширенной ячейки памяти меняется в памяти до MaxMemOTDRExt
extern uint8_t BusyUSB ; // признак передачи данных по USB, с SD картой
extern uint16_t PresentUSB ; // признак подключенного USB
extern uint8_t ModeUSB ; // признак подключенного USB

//extern   FIL Fil;
//extern   FRESULT FR_Status;
//extern   FRESULT res;
//extern  DIR dir;

#endif
