/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SysT7Knew_H
#define __SysT7Knew_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Define ---------------------------------------------------------*/
//#define RELEASETIME 1613347201 // начальное время установки релиза 15.02.2021 00:00:01
//#define RELEASETIME 1614643201 // начальное время установки релиза 02.03.2021 00:00:01    1614643201
//#define RELEASETIME 1649152800 // начальное время установки релиза 05.04.2022 10:00:00     версия 3
//#define RELEASETIME 1658484000 // начальное время установки релиза 22.07.2022 10:00:00     версия 6
//#define RELEASETIME 1660125600 // начальное время установки релиза 10.08.2022 10:00:00     версия 6
//#define RELEASETIME 1660816800 // начальное время установки релиза 18.08.2022 10:00:00     версия 12
//#define RELEASETIME 1661162400 // начальное время установки релиза 22.08.2022 10:00:00     версия 12
//#define RELEASETIME 1661248800 // начальное время установки релиза 23.08.2022 10:00:00     версия 18
//#define RELEASETIME 1664791200 // начальное время установки релиза 03.10.2022 10:00:00    версия 19
//#define RELEASETIME 1664877600 // начальное время установки релиза 04.10.2022 10:00:00    версия 20
//#define RELEASETIME 1664964000 // начальное время установки релиза 05.10.2022 10:00:00    версия 21
//#define RELEASETIME 1667296800 // начальное время установки релиза 01.11.2022 10:00:00    версия 23
//#define RELEASETIME 1667988000 // начальное время установки релиза 09.11.2022 10:00:00    версия 27
//#define RELEASETIME 1673258400 // начальное время установки релиза 09.01.2023 10:00:00    версия 35 НОВЫЙ ГОД!
//#define RELEASETIME 1676282400 // начальное время установки релиза 13.02.2023 10:00:00    версия 37 НОВЫЙ ГОД!
//#define RELEASETIME 1676628000 // начальное время установки релиза 17.02.2023 10:00:00    версия - 38// общие в MAIN и прочее 1673258400
//#define RELEASETIME  // начальное время установки релиза 15.03.2023 10:00:00    версия - 41// общие в MAIN и прочее 1673258400
//#define RELEASETIME 1682503200 // начальное время установки релиза 26.04.2023 10:00:00    версия - 44// 
//#define RELEASETIME 1683280800 // начальное время установки релиза 05.05.2023 10:00:00    версия - 45// 
//#define RELEASETIME 1692093600 // начальное время установки релиза 15.08.2023 10:00:00    версия - 46// 
//#define RELEASETIME 1692698400 // начальное время установки релиза 22.08.2023 10:00:00    версия - 50// скачек версий из-за многих итераций изменений
//#define RELEASETIME 1707732000 // начальное время установки релиза 12.02.2024 10:00:00    версия - 52// 
#define RELEASETIME 1709287200 // начальное время установки релиза 01.03.2024 10:00:00    версия - 53// 
#define RELEASEYEAR 24
#define RELEASEMONTH 3
#define RELEASEDATE 1
// время информации до автовыключения...   
#define LASTTIMEOFF 30   
   
#define KEYSREAD 50 // период чтения клавиатуры
#define ADCREAD 12 // период проверки состояния АЦП P1
   #if defined(STM32F302xE) || defined(STM32F303xE) || defined(STM32F398xx) || \
    defined(STM32F302xC) || defined(STM32F303xC) || defined(STM32F358xx) || \
    defined(STM32F303x8) || defined(STM32F334x8) || defined(STM32F328xx) || \
    defined(STM32F301x8) || defined(STM32F302x8) || defined(STM32F318xx) 

#define UADC 3.0/4095 // Вес АЦП
#endif
#if defined (STM32F103x6) || defined (STM32F103xB) || defined (STM32F105xC) || defined (STM32F107xC) || defined (STM32F103xE) || defined (STM32F103xG) || \
    defined (STM32F205xx)
      #define UADC 2.5/4095 // Вес АЦП

#endif      
/* Absolute value */
#define ABS(x)   ((x) > 0 ? (x) : -(x))
#define MAXBAT 4.1//4.2
#define MINBAT 3.1
#define MaxAT  250 // максимальные тики в режиме At 15сек, тик 50мС
   
   // установочные определения
#define CALIBRNUM 8 // число волн калибровки
#define LSPLACENUM 3 // число мест установки лазеров
#define WAVE_LENGTHS_NUM 7 // число основных длин волн калибровки (по последним данным 850,1300,1310,1490,1550,1625,1650)
#define WAVE_LENGTHS_SPEC 10 // число точек корректировки спектралки (0-коэфф для 650 нм)
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(*a))
#define ADCSIZE 16 // буффер усреденения АЦП P2
#define DELTAP2 4.0 // разница между порогами P2
#define DELTAP2mW 0.000002 // разница между порогами P2 in mW (40nW)
   
   // ORL LEVEL POROG
#define ORL_LVL_P  60.0
 // границы разрешения корректировки калибровки ORL на "верху" 14.7дБ     
#define LIM_ORL_UP  13.0
#define LIM_ORL_DN  17.0
#define LIM_ORL_CORR_NOISE -47.0
      
#define CntrlA(a) (a)?(HAL_GPIO_WritePin(ktA_GPIO_Port, ktA_Pin, GPIO_PIN_SET)):(HAL_GPIO_WritePin(ktA_GPIO_Port, ktA_Pin, GPIO_PIN_RESET)) // ON / off ktA

#define TsT(a) (a)?(HAL_GPIO_WritePin(test_GPIO_Port, test_Pin, GPIO_PIN_SET)):(HAL_GPIO_WritePin(test_GPIO_Port, test_Pin, GPIO_PIN_RESET)) // ON / off test
#define SW5V(a) (a)?(HAL_GPIO_WritePin(ENA_5V_GPIO_Port, ENA_5V_Pin, GPIO_PIN_SET)):(HAL_GPIO_WritePin(ENA_5V_GPIO_Port, ENA_5V_Pin, GPIO_PIN_RESET)) // ON / off 5V
 
// использование таймера отладки для подсчета временных интервалов
#define    DWT_CYCCNT    *(volatile uint32_t*)0xE0001004
#define    DWT_CONTROL   *(volatile uint32_t*)0xE0001000
#define    SCB_DEMCR     *(volatile uint32_t*)0xE000EDFC
// длительность импульза для измерителя длины
#define xImp 25
#define NUMMEASLN 4 // число измерний длины
#define SEQMEASLN 8 // последовательность принятых данных     
      //
   // переключатель дефайнов для регистров UART в зависимости от процессора 
// регистры ДАННЫХ и Флагов прерывания 
  // for F103 
#if defined (STM32F103x6) || defined (STM32F103xB) || defined (STM32F105xC) || defined (STM32F107xC) || defined (STM32F103xE) || defined (STM32F103xG) || \
    defined (STM32F205xx)
#define Clr_UART huart2.Instance->DR
//#define REG_ISR huart->Instance->SR

#endif

#if defined(STM32F302xE) || defined(STM32F303xE) || defined(STM32F398xx) || \
    defined(STM32F302xC) || defined(STM32F303xC) || defined(STM32F358xx) || \
    defined(STM32F303x8) || defined(STM32F334x8) || defined(STM32F328xx) || \
    defined(STM32F301x8) || defined(STM32F302x8) || defined(STM32F318xx) || \
    defined (STM32L471xx) || defined (STM32L475xx) || defined (STM32L476xx) || defined (STM32L485xx) || defined (STM32L486xx) || \
    defined (STM32L496xx) || defined (STM32L4A6xx) || \
    defined (STM32L4P5xx) || defined (STM32L4Q5xx) || \
    defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
      /* L476 */

#define Clr_UART huart2.Instance->RDR
//#define REG_ISR huart->Instance->ISR

#endif
// from t7k_AR
      //число сообщений и языков
#define LANG_NUM 6  // число столбцов в таблице языков

#define MSG_NUM 126
#define CMD_NUM 33 //команды Nextion


 /* Includes ------------------------------------------------------------------*/
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "stdint.h"
#include "math.h"
#include "pm.h" // функции измерителя P1
//#include "DrawFunctions.h"
//#include "fonts.h"
#include "pca955x.h"
#include "buttons.h"      
//#include "ssd1305.h"
//#include "KB_processing.h"
#include "HardSetDevice.h" // функции управления "железом" 
//#include "usartExt.h" // for UART 
//#include "MyClock.h"
//#include "Automat.h"

      // функции общие
      // тики опроса клавиатура взято из Т7К_АР
uint32_t GetSysTick( int Mode); // получение тиков 1 мС. 0 - получение счетчика от предыдущего сброса 1- сброс

void NEX_Transmit(uint8_t *Str);
  
   
   
//const uint16_t CalibrLW[WAVE_LENGTHS_NUM]={850,1300,1310,1490,1550,1625,1650};
   
////переменные режимов работы
enum WorkM {SetFn, P1, P1a, Sc, P2, Rl, Ln, Ls, At, RE, Clock, SetPrm}; // объявление перечислений
//enum WorkM CondWork = P1, OldCWork = SetFn, ConSet; //текущий режим работы, старый режим работы, выбираемый режим (режим выбора)
//переменные для управления режимами работы и индикации
enum Condition {dBm, dB, W, RCell, WCell, Loss_Lim_Set, Hold, WrHold}; // режим индикации текущий, старый, для измерителя P2
enum CondSource {OFFs, SCW, S27W, S2KW, SCW_P, OFF_S}; // для источника + OFF_S (установка мощности источника)
enum AtM {Wt, Orl, Px, Sl_Px, Sl_Orl, AskOrl, AnsOrl, Lnx, LnR, CmdSw, AnsOK, SavPw, CalPw, Err_Ans, View, Clb_Err, LnR_Clb, Cfg_At}; // подрежимы автоматического режима
enum CondView {LOSS, ORL, LNM, BasV}; // подрежимы для индикации результатов PON измерителя
enum SubSetting {sTime, sAutoOff, sNumF, sLOSSm, sLOSS, sORL, sSCp, sMem, sSWm, sVer};   // установка времени, Номер волокна и признак инкремента, порогов индикации затуханий P1a, мощности источиков, Меню очистки памяти, Переключатель признаков сохранения ORL-Loss-Ln in At.
enum CodeCmd {NOTcmd, SMPcmd, SLAcmd, YPcmd, SWMcmd, SOKcmd, SRLcmd, SWLcmd, SLOcmd}; // тип(номер) принятой команды по оптике
// структура блока памяти прибора
typedef struct
{
  uint32_t mTime; // время записи
  uint16_t mNumFiber;// Номер волокна
  uint16_t mLW[3]; // длина волны записей (3 значения)
  float mRes[3][4]; // блок записи данных (3 блока по 4 записи Float)
  uint16_t mLen; // длина участка
  uint8_t mCode; // код записи для расшифровки
  uint8_t mFree; // резерв 
  // блок данных 64 байта (512 записей 
} MEM_BLK;
// структура часов прибора
typedef struct
{
uint8_t cYear; //десятки годов
uint8_t cMonth;// месяц
uint8_t cDay;// день
uint8_t cHour;//час
uint8_t cMinutes;//минуты
uint8_t cSeconds; //секунды
//Alarm
uint8_t aYear; //десятки годов
uint8_t aMonth;// месяц
uint8_t aDay;// день
uint8_t aHour;//час
uint8_t aMinutes;//минуты
uint8_t aSeconds; //секунды
} CLOCK_DEV;

// структура конфигурации прибора
typedef struct
{
uint16_t NumDevice; //2 number device
uint8_t NameDevice[16];//16 строка наименование прибора
uint8_t CfgPM;//1  конфигурация измерителя 0 - нет, 1- фотодиод, 2 -сфера
uint8_t CfgRE;//1 конфигурация красного глаза 0 - нет, 1 - есть + newPlate 0x2 2- нет сновой платой Аналоговой 3- есть
uint8_t CfgL;//1 конфигурация тестера с автоматом
uint16_t PlaceLS[LSPLACENUM]; //6  Место 1, (1310) Конфигурация установленных лазеров по местам 2byte   
uint8_t CfgOUT[LSPLACENUM];//3 конфигурация выхода в режиме автоматического переключения длин волн 0- не в списке, 1- в списке (буковка "d"), но теперь для каждого лазера свое
uint32_t SmPx[WAVE_LENGTHS_NUM];// 28смещения (начальное) для измерителя P2 длин волн калибровки //28
// калибровочные коэффициенты измерителя длины (
float KGen; //4
float MZone; //4
float Kprop; // коэфф пропорциональности для расчета мощности P2 4
uint8_t YearManuf;// год производства
//27 byte+12
} DEV_CFG;


// структура коэффициентов измерителя мощности
typedef struct
{
int ShZeroRng[4];//16 начальные смещения диапазонов
float CoefStykRange[4]; //16 коэффициенты стыковки диапазонов={1.0,1.0,1.0,1.0};
float CoefSpctrL[6]; //24 коэффициенты спектральной харр. 800-900={NumWave = 0}
float CoefSpctrM[10]; //40 коэффициенты спектральной харр. 1210-1390 (NumWave = 1,2}
float CoefSpctrH[13]; //52 коэффициенты спектральной харр. 1410-1650={NumWave = 3,4,5,6}
float CoefPointKlb[WAVE_LENGTHS_NUM]; //28 коэффициенты привязки длин волн калибровки=(по последним данным 850,1300,1310,1490,1550,1625,1650)
float CoefTune[3]; //  подстроечного коэффициента для диапазонов 0-(800-900);1-(1210-1405);2-(1410-1650)
float BaseRet[WAVE_LENGTHS_NUM]; // базовые уровни для расчета обратного отражения (все семь т.к. надо измерять и  в режиме АВТОМАТ)
float Pow_mW_Clbr; // уровень смещения для точных измерений ORL 
//168 byte
} TAB_SET;
// структура коэффициентов измерителя мощности
typedef struct
{
uint16_t PointSpecKalib[WAVE_LENGTHS_SPEC];//специфических длины волн калибровки если 0 то ее нет=число точек корректировки спектралки (0-коэфф для 650 нм)их 10 шт.
float CoefSpecKlb[WAVE_LENGTHS_SPEC]; // коэффициенты привязки специфических длин волн калибровки= их 10 шт.
    // NumWave = 7... 
} TAB_SPECIFIC;

// структура настроечных данных источника(мощности), номер волокна,  авто инкремент, признаки разрешения сохранений и индикации, счетчик памяти
typedef struct
{
float PowSc[LSPLACENUM]; // значения мощности излучения установленных лазеров //12
uint16_t NumberFbrCount;// номер волокна 2
uint8_t EnaAutoInkrNF; // признак разрешения автоинкремента 1
uint8_t EnaCmpLOSS; // признак разрешения применять пороги при индикации затухания для 3 длин волн источников  
uint8_t EnaCmpORL; // признак разрешения применять пороги при индикации ORL для 3 длин волн источников  
uint8_t EnaWrtMem; // признак разрешения записи измерений в память 1бит-LOSS, 2-ORL, 3-Ln(длина)
uint16_t CountMemory; // счетчик ячеек памяти в которых хранятся данные
} TAB_DEVS;

// структура пользовательских данных для прибора
typedef struct
{
  //базовые уровни измерителя P1
float BaseLevel; // базовый уровень текущий 4
float BaseLvl[WAVE_LENGTHS_NUM]; // базовый уровень для калибровочных длин волн (7 шт) 28
float BaseSpecKlb[WAVE_LENGTHS_SPEC]; // значения базовых уровней для специфиеских длин волн = их 10 шт. 40
  //пределы изменения индикации для затухания и ORL в автоматических режимах простого измерителя
float Lim_LS[5]; // предел для затухания 12 (+8)
float Lim_ORL[3]; // предел для ORL 12
float BaseLevelP2; // базовый уровень текущий для Р2 4
float BaseLvlP2[WAVE_LENGTHS_NUM]; // базовый уровень для калибровочных длин волн (7 шт) для Р2  28
//132+8
uint16_t CalibLnAutoMem; //Калибровка измерителя длины в автомате +2 = 142 
// 30.09.2022 - добавляем поля сохранения режимов для востановления при включении
uint16_t PMWaveLenghtMem; // длина волны измериеля P1
uint8_t CondWorkMem; // текущий режим работы для востановления P1,P1a,Sc,P2 
uint8_t ScModeMem; // режим источника, возможно надо выключать
uint8_t Ind_LWMem; // индекс длины волны источника из разрешенных
uint8_t ModeIMem; // режим индикации P1
uint8_t ModeIP1aMem; // режим индикации P1а
uint8_t P2ModeMem; // режим индикации P2
uint8_t IndxP2LWMem; //индекс длины волны измерителя P2
uint8_t AutoOff; // признак авто выключения ( возможно время не более ...) пока почиститим если FFFF до 0 нет, если есть 1...только для индикации
uint8_t AutoOffMod; // Режим  авто выключения ( различные времена),А.К. предложил устанавливать в настройках, после времени 
// включение/выключение двумя кнопками Mode+S долгое нажатие, не прижилось как и кнопки Вверх+Вниз
} TAB_USER;

// структура  данных PON AUTO
typedef struct
{

float PW_AB[WAVE_LENGTHS_NUM+1]; // мощность померенная прибором B переданная обратно в  A 
float PW_BA[WAVE_LENGTHS_NUM+1]; // мощность померенная прибором А (М) относительно мощности PS_BA
float PS_BA[WAVE_LENGTHS_NUM+1]; // Мощность источника прибора В(передает в A)  
float LS_AB[WAVE_LENGTHS_NUM+1]; // Потери (затухание) измеренное из А в В
float LS_BA[WAVE_LENGTHS_NUM+1]; // Потери (затухание) измеренное из В в А
float ORL_AB[WAVE_LENGTHS_NUM+1]; // обратное отражение померянное от себя A->B
float ORL_BA[WAVE_LENGTHS_NUM+1]; // Обратное отражение ORL померенное со стороны В (когда излучаем источник)
// +1 чтобы не "пилить" по памяти если примем волну не из списка калибровочных (в нее запишется значение
uint16_t LW_BA[3]; // Длина волны источника прибора В (передает в A) 
uint16_t LW_AB[3]; // Длина волны источника прибора A (дублируется из конфигурации мастера) 
uint16_t LnA; // Длина линии 

} TAB_PONAUTO;

// переменные из HardSetDevice  октябрь 2024
extern uint8_t NeedReDraw; // необходимость перерисовать индикатор
extern uint8_t Range;
// source
extern uint8_t Ind_LW;
extern uint8_t DataDevice[20];
extern uint8_t NeedFreq; // необходимо перезапустить генерацию через ДМА
extern const uint8_t* TxGenOpt;
extern uint16_t IN_BuffADC[4]; // буффер АЦП
extern const char *CmdNextion[CMD_NUM]; 
extern const char *MsgMass[MSG_NUM][LANG_NUM]; 
extern uint16_t KeyP; // клавиши нажатые 
extern volatile unsigned char rawPressKeyS; // признак необработанной нажатой клавиши S по прерыванию

// for DrawFunction
extern uint8_t Str[64];
extern uint8_t Stra[32];
extern uint8_t Strb[32];
extern uint8_t Strc[32]; // 
extern uint8_t Strd[32];
extern uint8_t Strm[32]; // 
extern uint8_t Str2[32]; // разрядность индикации в режиме  
extern uint8_t StrDrM[4]; // строка отображения режима работы на индикаторе
extern uint8_t StrBat[4]; // строка отображения режима работы на индикаторе

// for Automat
extern uint8_t KeyCode; //код клавиш для вызова обработчика (msp430 Inp)


extern uint8_t StPon1[3][5], StPon2[3][8], StPon3[3][8]; //строки для многострочных выводов из составляющих

extern const uint16_t NumVer; 

extern TAB_PONAUTO PonA; // структура  данных PON AUTO
extern DEV_CFG DeviceConfig; // структура конфигурации прибора
extern TAB_SET CoeffPM; // таблица коэффициентов измерителя (смещение, стыковка, спектралка...)
extern TAB_SPECIFIC SpecCoeffPM; // таблица коэффициентов измерителя (специфических длин волн)
extern TAB_USER UserMeasConfig; // таблица пользовательских данных для прибора
// структура настроечных данных источника(мощности), номер волокна,  авто инкремент, признаки разрешения сохранений и индикации, счетчик памяти
extern TAB_DEVS SettingPrm; // 
extern CLOCK_DEV Clocks; // Часы
extern MEM_BLK MemoryData; // блок данных сохранения (текущий) в нем содержится счетчик волокна
//переменные режимов работы
extern enum WorkM CondWork , OldCWork, ConSet, CondRet; //текущий режим работы, старый режим работы, выбираемый режим (режим выбора)
//переменные для управления режимами работы и индикации
extern enum Condition ModeI, OldMode, P2Mode; // режим индикации текущий, старый, для измерителя P2
extern enum CondSource ScMode, OldScMode, ReMode; // для источника + OFF_S (установка мощности источника)
extern enum AtM Mod_At;// подрежимы автоматического режима
extern enum CondView Mod_View; // подрежимы для индикации результатов PON измерителя
extern enum SubSetting SetupMod;
extern enum CodeCmd OpticCMD; //команды принятые по оптике (в режиме автомат)
extern int iParamOpt; // целочисленный параметр при приеме по оптике
extern float fParamOpt; // float параметр при приеме по оптике
// for UART
extern DMA_HandleTypeDef hdma_uart7_tx;
extern UART_HandleTypeDef huart7;
// for Timer Generator Zimp
extern TIM_HandleTypeDef htim2;
//extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim3;
extern uint8_t CntLCD_LN; // проверочный счетчик 12 мС для проверки прорисовки инликатора для отображения результатов
extern uint8_t CntLCD_P1; // проверочный счетчик 50 мС для проверки инликатора

// глобальная переменная разрешения установок 
extern uint8_t g_EnaSetup;
extern uint8_t N_LS;
// Source
extern uint8_t lSofn; // признак индикации включенного лазера
extern uint8_t TxOptBusy; // признак занятости UatrOptic на передачу по оптике
extern uint8_t StrTXOpt[64]; // буффер передачи данных по оптике
// Автомат
extern volatile uint8_t NeedTransAnsver; // необходимость ответа Slave на принятый импульс от мастера
extern uint8_t DisRecImp; // запрет приема пока передали импульс (думаю на 3-5 мС) (через системный таймер)
extern uint16_t CalibLnAuto ; // смещение длины после калибровки в режиме автомат!
extern uint16_t IndxShowLN; // индекс показаний измеренных расстояний до отражений

extern volatile uint32_t WaitCntLnX; // счетчик паузы в мС для ожидания передачи ответа мастеру от Slave в режиме LnX
extern volatile uint32_t Beg_Evnt; // 
extern volatile uint32_t End_Evnt; // 
extern volatile uint32_t Time_Evnt; // 
// for ADC
extern float Ubat, OldUbat; // напряжение аккумулятра (или системного напряжения)


extern uint8_t NeedSaveChange; // признак необходимости перезаписи блоков памяти
extern uint8_t IndxEditClock; // индекс указателя на редактор элемента in Clock
extern uint8_t IndxEditNumFbr; // индекс указателя на редактор элемента in Number Fiber
extern uint8_t IndxEditLOSS; // индекс указателя на редактор элемента LOSS
extern uint8_t IndxEditLOSSm; // индекс указателя на редактор элемента LOSS MM 850,1300
extern uint8_t IndxEditORL; // индекс указателя на редактор элемента ORL
extern uint8_t IndxEnaSave; // индекс указателя на редактор элемента разрешения сохранений
extern uint8_t IndxPowSC; // индекс указателя на редактор элемента мощности источника
extern uint8_t IndxClrMEM; // индекс указателя разрешения очистки памяти
extern uint8_t IndxInvView; // индекс указателя на инверсию при выводе результатов в автомате (1-первое место, 2-второе, 4-третье)
extern uint8_t MemBlock[256]; // блок для перезаписи ячеек памяти
extern uint8_t CountDrawMsg; // счетчик индикации надписи ( P1 - запись в память)

extern float BaseExtLvl[7]; // базовые уровни  источника принятого извне.
extern uint8_t MsgAuto[3][20]; // строки вывода в режиме автомат
extern uint32_t TestCntWait;// тестовый счетчик измерений (в данном случае P2) 
extern uint16_t Dummy;
extern HAL_StatusTypeDef StatusI2C2;
// счетчик авто выключения
extern int CntPwrOffCount; // счетчик времени автовыключения


   
#ifdef __cplusplus
}
#endif
#endif /*__ sysT7Knew_H */
