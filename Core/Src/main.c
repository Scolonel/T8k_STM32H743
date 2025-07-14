/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "fatfs.h"
#include "i2c.h"
#include "rtc.h"
#include "sdmmc.h"
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//#include "usbd_cdc_if.h"
#include "usb_device.h"
#include "usbd_cdc_acm_if.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
    // !!!¬ Ќ » ћ ј Ќ » ≈!!!
    // идентификатор платы должен совпасть с программным
#define ID_PLATE  0x1 //0x3 - “7к  0x2 - “5к
//#define ID_PLATE  0x2 //0x3 - “7к  0x2 - “5к
#define DEL_PWR (100./62. + 1) //  делитель дл€ контрол€ напр€жени€ батарейки

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

const uint16_t TstDacCode[9]={128,512,64,1024,32,2048,256,384,3000};
uint16_t KeyP; // клавиши нажатые 
  char DigitSet = 1; //шаг изменеи€ устанавливаемого затухани€
  char LvlBatInd=8; //индикатор уровн€ батарейки

  // контроль идентификатора платы
 uint8_t CheckErrID_Plate=0; 

     float CWDMData[18]; // данные 
     float CWDMDataMem[18]; // данные из пам€ти
     BYTE g_IndexMeas = 0; // счетчик индекс циклов
     BYTE g_IndexLW = 0; // индекс указатель на длину волны индикации
uint8_t g_EnaQuickReDraw=0;; // признак быстрой перерисовки экрана когда анализатор
 // окна 
     uint8_t CntChanel=0; // счетчик каналов
// режим работы прибора дл€ настройки (1 - настройка, 0- работа)
 char ModeWork = 0;
 // признак перерисовки экрана в динамических режимах
 char ModeReDrawLCD = 0; // јнализатор, главное меню (там часы)
 // Ќомер текущей страницы индикатора дл€ востановлени€
  char NumCurrPage = 0; // 
 // нужно обработать функцию. по времени или по событию
  char NeedRunFunc = 0; // 

//variable USB
//uint32_t RecievUSB=0 ; // признак прин€ти€ данных по USB, число данных в буфере
uint32_t BusyUSB=0 ; // признак передачи данных по USB, с SD картой
// при приеме передаче взводим на 10 м— , и перезаводим при следующей передаче/приеме
uint32_t PresentUSB = 0; // признак подключенного USB
uint32_t ModeUSB = 0; // признак работы USB дл€ индикации
 
unsigned int CheckErrMEM; 
BYTE CurrLang; // текущий €зык

DWORD TimeBegin; // врем€ начала, дл€ контрол€ начальной заставки

uint8_t TxDMA = 0; // признак зан€тости DMA 
char TxBufAns[512]; // буффер передачи в USB
// переменные UART I2C
uint16_t Dummy; // пустое чтение буфферов UART
// строка дл€ индикатора
char Str[64];
static char Stri[32]; // строка дл€ отображени€ сообщени€ о подключении USB

char VerFW_LCD[25] = {"No version LCD          \0"}; //верси€ ѕќ индикатора NEXION

// ¬—ѕќћќ√ј“≈Ћ№Ќџ≈ ѕ≈–≈ћ≈ЌЌџ≈
DWORD CountTimerPA = 0;
char ScreenReDraw=0; // признак необходимости перерисовать экран
char NeedSaveParam=0; // признак необходимости сохранить параметры

uint16_t CurrLevelDAC=0; //текущий уровень дл€ ÷јѕ (востанавливаем из тех что храним в UserSet)

uint32_t CcMinute=0; // счетчик минуты (посекундно)
uint32_t BadLevelBat=0; //режим плохого уровн€ батарейки
uint32_t BadBatCnt = 0; // счетчик времени плохой батарейки
  
float Ubat=4.6; // начальное напр€жение батареи
uint16_t BufADC[SizeBuf_ADC_int]; // буфер внутреннего ј÷ѕ (8), в него пишем при съеме DMA, размер до 8
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  
  /* USER CODE END 1 */
  
  /* MCU Configuration--------------------------------------------------------*/
  
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  
  /* USER CODE BEGIN Init */
  
  /* USER CODE END Init */
  
  /* Configure the system clock */
  SystemClock_Config();
  
  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();
  
  /* USER CODE BEGIN SysInit */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_UART7_Init();
  // Start Uart7 - Nextion
  uint16_t  Dummy = huart7.Instance->RDR ; // чистим буффер приема от NEXTION
  HAL_UART_Receive_IT(&huart7, RX_BufNEX,1); // ждем прин€ти€ первого байта из внешнего мира
  /* disable the UART Parity Error Interrupt */
  __HAL_UART_DISABLE_IT(&huart7, UART_IT_PE);
  /* disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
  __HAL_UART_DISABLE_IT(&huart7, UART_IT_ERR);
  
  // перенастроим UART7  дл€ NEXTION
  huart7.Init.BaudRate = 9600;
  if (HAL_UART_Init(&huart7) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_Delay(10);
  sprintf((void*)Str,"bauds=115200€€€");
  HAL_UART_Transmit(&huart7, (void*)Str,strlen((void*)Str),20); // выдаем 
  HAL_Delay(20);
  sprintf((void*)Str,"bauds=115200€€€");
  HAL_UART_Transmit(&huart7, (void*)Str,strlen((void*)Str),20); // выдаем 
  
  //NEX_Transmit(Str);// 
  HAL_Delay(10);
  huart7.Init.BaudRate = 115200;
  if (HAL_UART_Init(&huart7) != HAL_OK)
  {
    Error_Handler();
  }
  //  myBeep(100);
  HAL_Delay(10);
  if(ID_PLATE != GETIDPLT)
  {
    CheckErrID_Plate=1;
  }
  if(CheckErrID_Plate)
  {
    sprintf((void*)Str, "t0.txt=\"! ќЎ»Ѕ ј !\"€€€"); // auto
    NEX_Transmit((void*)Str);    // 
    sprintf((void*)Str, "t1.txt=\"!прибор не тот!\"€€€"); // auto
    NEX_Transmit((void*)Str);    // 
    
    while(1);
    
  }
    CmdInitPage(0);// вызов окна заставки

//  HAL_Delay(10);
//  sprintf((void*)Str, "page 0€€€"); // < START>
//  NEX_Transmit((void*)Str);    //
  //       StartRecievNEX (500);
  //    sprintf((void*)Str,"get t10.txt€€€");
  //    NEX_Transmit((void*)Str);//
  //NEX_Transmit((void*)CmdBuf);//
  CmdInitPage(0);// вызов окна заставки
//  HAL_Delay(10);
//  sprintf((void*)Str, "page 0€€€"); // < START>
//  NEX_Transmit((void*)Str);    //
//  HAL_Delay(10);
  //    sprintf((void*)Str, "t0.txt=\"начало\"€€€"); // auto
  //    NEX_Transmit((void*)Str);    // 
  //      HAL_Delay(10);
  
  StartRecievNEX (400);
  sprintf((void*)Str,"get tlcd.txt€€€");
  NEX_Transmit((void*)Str);//
  //HAL_Delay(200);
  while(!((g_WtRdyNEX)||(ReadyNEX==4)));
    HAL_Delay(50);
  StartRecievNEX (400);
  sprintf((void*)Str,"get tlcd.txt€€€");
  NEX_Transmit((void*)Str);//
  //HAL_Delay(200);
  while(!((g_WtRdyNEX)||(ReadyNEX==4)));
  // здесь просто можем повиснуть не дождавшись ответов от индикатора
  // это плохо при плохих индикаторах
  // надо ждать получени€ ответа
  if(RX_BufNEX[0] == 0x70) // есть ответ! перепишем буффер
  {
    for(int i=0;i<25;++i)VerFW_LCD[i]=RX_BufNEX[i+1];
    VerFW_LCD[23]=0;
    // здесь получим идентификатор индикатора (если его прочтем)
    // он нужен дл€ вариантов отображени€ при просмотре рефлектограмм и в пам€ти
    //      switch(VerFW_LCD[3])
    //      {
    //      case '2':
    //        TypeLCD=0;
    //        KnowLCD = 1;
    //        break;
    //      case '5':
    //        TypeLCD=1;
    //        KnowLCD = 1;
    //        break;
    //      default:
    //        TypeLCD=0;
    //        KnowLCD = 0;
    //        break;
    //      }
  }
  // пошлем сообщение о включении ...
  sprintf((void*)Str, "t1.txt=\"¬ключение...\"€€€"); // auto
  NEX_Transmit((void*)Str);    // 
  
  /* USER CODE END SysInit */
  
  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART3_UART_Init();
  MX_ADC1_Init();
  MX_I2C2_Init();
  MX_RTC_Init();
  MX_UART7_Init();
  MX_UART5_Init();
  MX_DAC1_Init();
  MX_SDMMC2_SD_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */
  // сразу пробуем поставить ÷јѕ
  
  HAL_DAC_Start(&hdac1,DAC_CHANNEL_2);
  
  HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_2,DAC_ALIGN_12B_R,CurrLevelDAC);
  
  // провер€ем конфигурацию платы, чтобы не запустить программу по исправленю
  // если вдруг зашили чужую программу, попытаемс€ написать в индикатор и зациклитс€
  
  
  // Start Uart7 - Nextion
  Dummy = huart7.Instance->RDR ; // чистим буффер приема от NEXTION
  HAL_UART_Receive_IT(&huart7, RX_BufNEX,1); // ждем прин€ти€ первого байта из внешнего мира
  /* disable the UART Parity Error Interrupt */
  __HAL_UART_DISABLE_IT(&huart7, UART_IT_PE);
  /* disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
  __HAL_UART_DISABLE_IT(&huart7, UART_IT_ERR);
  
  //  // перенастроим UART7  дл€ NEXTION
  //  huart7.Init.BaudRate = 9600;
  //  if (HAL_UART_Init(&huart7) != HAL_OK)
  //  {
  //    Error_Handler();
  //  }
  //  HAL_Delay(10);
  //  sprintf((void*)Str,"bauds=115200€€€");
  //  HAL_UART_Transmit(&huart7, (void*)Str,strlen((void*)Str),20); // выдаем 
  //
  //  //NEX_Transmit(Str);// 
  //   HAL_Delay(10);
  //  huart7.Init.BaudRate = 115200;
  //  if (HAL_UART_Init(&huart7) != HAL_OK)
  //  {
  //    Error_Handler();
  //  }
  // //  myBeep(100);
  //  HAL_Delay(10);
  //  sprintf((void*)Str, "page 0€€€"); // < START>
  //  NEX_Transmit((void*)Str);    //
  //  // пошлем сообщение о включении ...
  //    sprintf((void*)Str, "t1.txt=\"¬ключение...\"€€€"); // auto
  //    NEX_Transmit((void*)Str);    // 
  
  
  
  // так как повтор€ем конфигурацию из 7kAR, то скомбинируем из DataDevice MemFlash(у нас PCA955x)
  CheckErrMEM =   BeginConfig();
  
  CheckErrMEM |= StartInitSDcard();
  
  // подготовка внутреннего ј÷ѕ
  if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED) != HAL_OK)
  {
    myBeep(100);
    
    Error_Handler();
  }
  // конец обработки клавы и ј÷ѕ, запустим снова ј÷ѕ
  // if(HAL_ADC_Start_DMA(&hadc1,(uint32_t*)&BufADC,3) != HAL_OK) Error_Handler();//3
  
  if (HAL_ADC_Start_DMA(&hadc1,
                        (uint32_t *)&BufADC,
                        8
                          ) != HAL_OK)
  {
    myBeep(100);
    Error_Handler();
  }
  //LED_START(1);
  // Start Uart3 - внешний мир
  Dummy = huart3.Instance->RDR ; // чистим буффер приема от SIM
  HAL_UART_Receive_IT(&huart3, RxBufExt,1); // ждем прин€ти€ первого байта из внешнего мира
  /* disable the UART Parity Error Interrupt */
  __HAL_UART_DISABLE_IT(&huart3, UART_IT_PE);
  /* disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
  __HAL_UART_DISABLE_IT(&huart3, UART_IT_ERR);
  // “естова€ посылка по UART
  sprintf((void*)TxBufAns,"TEst\n"); //  
  HAL_UART_Transmit(&huart3,(void*)TxBufAns, strlen((void*)TxBufAns),100);
  // Start Uart5 - Optics
  Dummy = huart5.Instance->RDR ; // чистим буффер приема от OPTIC
  HAL_UART_Receive_IT(&huart5, RxBufExt,1); // ждем прин€ти€ первого байта из внешнего мира
  /* disable the UART Parity Error Interrupt */
  __HAL_UART_DISABLE_IT(&huart5, UART_IT_PE);
  /* disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
  __HAL_UART_DISABLE_IT(&huart5, UART_IT_ERR);
  //UARTSendExt ((BYTE*)TxBufAns, strlen(TxBufAns));
  // инициализаци€ клавиатуры 
  InitBtns(); 
  
  // начало работы..
  TimeBegin = HAL_GetTick();
  // получим и посчитаем батарейку
  Ubat = 2.5*DEL_PWR*BufADC[0]/4096; 
  // перебор уровн€ батаhейки, дл€ индикации
  LvlBatInd = 0;
  if(Ubat > 5.1) LvlBatInd = 8;
  else if (Ubat > 5.02) LvlBatInd = 7;
  else if (Ubat > 4.95) LvlBatInd = 6;
  else if (Ubat > 4.88) LvlBatInd = 5;
  else if (Ubat > 4.81) LvlBatInd = 4;
  else if (Ubat > 4.74) LvlBatInd = 3;
  else if (Ubat > 4.67) LvlBatInd = 2;
  else if (Ubat > 4.6) LvlBatInd = 1;
  if(GETEXTPWR == 0)  LvlBatInd = 9;
  // CountBat = 0;    
  LvlBatSav.BatControl[0] = CountBat++;
  LvlBatSav.BatControl[CountBat] = Ubat;
  // пропишем в пам€ть
  EEPROM_write(&LvlBatSav.BatControl[0], ADR_BatSave  , 4);
  EEPROM_write(&LvlBatSav.BatControl[CountBat], ADR_BatSave +  4*CountBat , 4);
  
  CmdInitPage(0);// вызов окна заставки
  
  HAL_Delay(10);
  SetMode (ModeWelcome);
  CmdInitPage(0);// посылка команды переключени€ окна на Welcome и установка признака первого входа
  MX_USB_DEVICE_Init();
  
  
  /* USER CODE END 2 */
  
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // проверка кнопок каждые 30 м—, и тут переключаем и измер€ем каналы
    // взводим признак необходимости выполнени€ функции
    if((GetSysTick(0)>30)&&(!ProgFW_LCD))// каждые 30 м— или больше...и не в программировании
    {
      // расчет данных по полученным данным из ј÷ѕ, каждый момент имеем два значени€ ј÷ѕ
      uint32_t SumAdcOne=0;
      uint32_t SumAdcTwo=0;
      for(int i=NUMAVRG-1;i>0;i--)
      {
        SumAdcOne +=AdcCodes[CntChanel].dADC[i] = AdcCodes[CntChanel].dADC[i-1];
        SumAdcTwo +=AdcCodes[CntChanel+9].dADC[i] = AdcCodes[CntChanel+9].dADC[i-1];
      }
      SumAdcOne +=AdcCodes[CntChanel].dADC[0] = BufADC[2];
      SumAdcTwo +=AdcCodes[CntChanel+9].dADC[0] = BufADC[3];
      AdcCodes[CntChanel].AvrgADC = SumAdcOne/NUMAVRG;
      AdcCodes[CntChanel+9].AvrgADC = SumAdcTwo/NUMAVRG;
      // которое прописываем в соотв €чейку
      //CWDMData[CntChanel] = BufADC[2]*CoeffLW.SlopeChADC[0]+CoeffLW.OffsetLW[CntChanel];    
      //CWDMData[CntChanel+9] = BufADC[3]*CoeffLW.SlopeChADC[1]+CoeffLW.OffsetLW[CntChanel+9];    
      CWDMData[CntChanel] = AdcCodes[CntChanel].AvrgADC*CoeffLW.SlopeChADC[0]+CoeffLW.OffsetLW[CntChanel];    
      CWDMData[CntChanel+9] = AdcCodes[CntChanel+9].AvrgADC*CoeffLW.SlopeChADC[1]+CoeffLW.OffsetLW[CntChanel+9];    
      // измен€ем счетчик перебора
      if(CntChanel<8)CntChanel++;
      else CntChanel=0;
      // управление ключами по счетчику
      CtrlExpand(CntChanel<<3,0x78);
      // выведем текущий уровень ÷јѕ по счетчику
      //HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_2,DAC_ALIGN_12B_R,TstDacCode[CurrLevelDAC]);
      HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_2,DAC_ALIGN_12B_R,TstDacCode[CntChanel]);
      //
      KeyP = SetBtnStates( GetExpand (), 1 ); // опрос клавиатуры
      GetSysTick(1);// сброс системного ожидани€
      // управление красным лазером
      // поконтролить батарейку
      // инекремент таймаре PA
      HAL_Delay(1);
      if(CurrLevelDAC<7)CurrLevelDAC++;
      else CurrLevelDAC=0;
      
      CountTimerPA++;
      if(CountTimerPA>33)
      {
        // каждую секунду, посчитаем батарейку
        // (BufADC[0]*(2.5/4096))
        // хорошо зар€женные 5.3-5.4 -
        // без аккумул€тора от сети вижу 4.33 
        // пока возьмем 4.1 мин - 5.0 макс
        Ubat = 2.5*DEL_PWR*BufADC[0]/4096; 
        // перебор уровн€ батаhейки, дл€ индикации
        if(Ubat > 5.1) LvlBatInd = 8;
        else if (Ubat > 5.01) LvlBatInd = 7;
        else if (Ubat > 4.92) LvlBatInd = 6;
        else if (Ubat > 4.83) LvlBatInd = 5;
        else if (Ubat > 4.74) LvlBatInd = 4;
        else if (Ubat > 4.65) LvlBatInd = 3;
        else if (Ubat > 4.56) LvlBatInd = 2;
        else if (Ubat >= 4.5) LvlBatInd = 1;
        // внешнее питание
        if(GETEXTPWR == 0)
        {
          LvlBatInd = 9;
          BadBatCnt = 0;
        }
        else if (Ubat < 4.5) 
        {
          LvlBatInd = 0;
          if(BadBatCnt<3)
          {
            BadBatCnt++; //ждем пару сек
            //LED_START(1);//On  LED
            //myBeep(10);
            //HAL_Delay(50);
            //LED_START(0);//Off  LED
            
          }
          if(!BadLevelBat && (BadBatCnt==3))
          { // переключаемс€ в режим индикации плохой батаейки
            BadLevelBat = 1;
            SetMode (BadBattery);
            CmdInitPage(5);
          }
          
        }
        else // уровень в норме
        {
          BadBatCnt = 0;
          
        }
        //LvlBatInd = (char)(Ubat*10. - 40.)+1;
        //if(Ubat<4.0) LvlBatInd = 0;
        //if((Ubat>4.9)||(LvlBatInd>8)) LvlBatInd = 8;
        //        sprintf((void*)Str,"p0.pic=%d€€€",LvlBatInd);
        //        NEX_Transmit((void*)Str);//
        // получим текущее врем€ и оработаем его
        if(CcMinute++>60)
        {
          if(CountBat++>2048)
            CountBat=1;
          
          LvlBatSav.BatControl[CountBat] = Ubat;
          LvlBatSav.BatControl[0] = CountBat;
          // пропишем в пам€ть
          EEPROM_write(&LvlBatSav.BatControl[0], ADR_BatSave  , 4);
          EEPROM_write(&LvlBatSav.BatControl[CountBat], ADR_BatSave +  4*CountBat , 4);
          
          CcMinute=0;
        }
        g_IndexMeas++; // просто, проверочный счетчик, приблизительно каждую секунду, дл€ переключени€ индикации
        // о работе USB
        CountTimerPA = 0;
        if(ModeReDrawLCD)
          g_NeedScr=1;
        
      }
      //if(g_EnaQuickReDraw)g_NeedScr=1;
      //LvlBatInd++;
      // здесь можно запустить »змерение ј÷ѕ
      if (HAL_ADC_Start_DMA(&hadc1,(uint32_t *)&BufADC,8) != HAL_OK)
      {
        myBeep(100);
        Error_Handler();
      }
      //LED_START(1);
      NeedRunFunc = 1;
    }// конец 30 м— обработки
    // проверка приема по UART EXT
    if (RSDecYes) // вызов программы обработки комманды прин€той по UART
    {
      DecodeCommandRS();
      NeedRunFunc = 1;
    }
    
    // режим программировани€ индикатора и ответы от индикатора
    if(Uart2DecYes)
    {
      if(ProgFW_LCD)
      {
        CDC_Transmit(0,(uint8_t*)RX_BufNEX, CntRXNEX ); // echo back on same channel
      }
      else
      {
        // что-то прин€ли в ответ от индикатора, можно посмотреть
        CheckStrNEX (); // проверка прин€той строки  
        NeedRunFunc = 1;
        
      }
      Uart2DecYes=0;
      RecievNEX=STOP_UART;
    }
    
    if(NeedSaveParam)
    {
      
      WriteNeedStruct(NeedSaveParam);
      NeedSaveParam = 0;
      NeedRunFunc = 1;
    }
    // основное отображение режима
    if((!ProgFW_LCD)&&(NeedRunFunc)) 
    {
      // сюда попадаем по времени каждые 30м— или по событи€м приема, 
      // дл€ перерисовки экрана
//      if(ModeUSB) // устанавливаем когда пишем или читаем по USB SDCard
//      {
//        // сбрасываем нажатые кнопки
//        ClrKeyAll();
//      }
      // прорисовка основной функции
//        ModeFuncTmp();
      // если ‘лэшка зан€та, 

      NeedRunFunc = 0; 
      if(ModeUSB) // устанавливаем когда пишем или читаем по USB SDCard
      {
        // сбрасываем нажатые кнопки
        ClrKeyAll();
        switch (ModeUSB)
        {
        case 3:
          //sprintf(Stri,"xstr 80,85,350,60,2,BLACK,2016,1,1,1,\"%s\"€€€",MsgMass[61][CurrLang]); // зеленый ?
          //NEX_Transmit((void*)Stri);//
         //HAL_Delay(2);
         //sprintf(Stri,"xstr 80,145,350,60,2,BLACK,2016,1,1,1,\"%s\"€€€",MsgMass[27][CurrLang]); // зеленый ?
         //NEX_Transmit((void*)Stri);//
         // HAL_Delay(50);
                    sprintf((void*)Str,"pic 144,94,%d€€€",(g_IndexMeas%3)+53);
                    //sprintf((void*)Str,"pic 144,94,53€€€");
                    NEX_Transmit((void*)Str);//
 
//                    sprintf((void*)Str,"fill 0,0,480,4,RED€€€");
//                    NEX_Transmit((void*)Str);//
//                    sprintf((void*)Str,"fill 0,0,4,320,RED€€€");
//                    NEX_Transmit((void*)Str);//
//                    sprintf((void*)Str,"fill 476,4,4,320,RED€€€");
//                    NEX_Transmit((void*)Str);//
//                    sprintf((void*)Str,"fill 0,316,4480,4,RED€€€");
//                    NEX_Transmit((void*)Str);//
          ModeUSB=2;
          break;
        case 2:
          //sprintf(Stri,"xstr 80,85,350,60,2,BLACK,2016,1,1,1,\"%s\"€€€",MsgMass[61][CurrLang]); // зеленый ?
          //NEX_Transmit((void*)Stri);//
          HAL_Delay(20);
          //sprintf(Stri,"xstr 80,145,350,60,2,BLACK,2016,1,1,1,\"%s\"€€€",MsgMass[27][CurrLang]); // зеленый ?
          //NEX_Transmit((void*)Stri);//
          //HAL_Delay(50);
          
          //          sprintf((void*)Str,"fill 0,0,480,4,RED€€€");
          //          NEX_Transmit((void*)Str);//
          //          sprintf((void*)Str,"fill 0,0,4,320,RED€€€");
          //          NEX_Transmit((void*)Str);//
          //          sprintf((void*)Str,"fill 476,4,4,320,RED€€€");
          //          NEX_Transmit((void*)Str);//
          //          sprintf((void*)Str,"fill 0,316,4480,4,RED€€€");
          //          NEX_Transmit((void*)Str);//
          break;
        default:
          
//                    sprintf((void*)Str,"fill 0,0,480,4,WHITE€€€");
//                    NEX_Transmit((void*)Str);//
//                    sprintf((void*)Str,"fill 0,0,4,320,WHITE€€€");
//                    NEX_Transmit((void*)Str);//
//                    sprintf((void*)Str,"fill 476,4,4,320,WHITE€€€");
//                    NEX_Transmit((void*)Str);//
//                    sprintf((void*)Str,"fill 0,316,4480,4,WHITE€€€");
//                    NEX_Transmit((void*)Str);//
          ModeUSB = 0;
          CmdInitPage(NumCurrPage);
          
          break;
        }
      }
      else
      {
      // прорисовка основной функции
        ModeFuncTmp();
      }

      LED_START(0);
      
    }
    // проверка окончани€ записи индикатора
    if(ProgFW_LCD==2) ProgFW_LCD=0;
    
    /* USER CODE END WHILE */
    
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 120;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 8;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USB|RCC_PERIPHCLK_ADC
                              |RCC_PERIPHCLK_SDMMC|RCC_PERIPHCLK_UART5
                              |RCC_PERIPHCLK_UART7|RCC_PERIPHCLK_USART3;
  PeriphClkInitStruct.PLL2.PLL2M = 16;
  PeriphClkInitStruct.PLL2.PLL2N = 144;
  PeriphClkInitStruct.PLL2.PLL2P = 3;
  PeriphClkInitStruct.PLL2.PLL2Q = 8;
  PeriphClkInitStruct.PLL2.PLL2R = 6;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_1;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.PLL3.PLL3M = 16;
  PeriphClkInitStruct.PLL3.PLL3N = 96;
  PeriphClkInitStruct.PLL3.PLL3P = 2;
  PeriphClkInitStruct.PLL3.PLL3Q = 4;
  PeriphClkInitStruct.PLL3.PLL3R = 12;
  PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_1;
  PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
  PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
  PeriphClkInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL2;
  PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_PLL2;
  PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_PLL3;
  PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL3;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
// тики опроса клавиатура вз€то из “7 _ј–
uint32_t GetSysTick( int Mode) // получение тиков 1 м—. 0 - получение счетчика от предыдущего сброса 1- сброс
{
  static uint32_t MemTick;
  if(Mode) MemTick=HAL_GetTick();
  return HAL_GetTick()-MemTick;
}

// управление таймером в измерителе ј¬“ќћј“≈
WORD TimerPA (BYTE Set)
{
  if (Set) CountTimerPA = 0;
  return CountTimerPA;
}

/* 
 * set  beepTick
 */
void myBeep (unsigned sound)
{
  extern unsigned beepTick; // 

	beepTick = sound + 1;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
