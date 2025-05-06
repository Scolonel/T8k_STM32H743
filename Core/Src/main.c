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
#include "i2c.h"
#include "rtc.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
    // !!!В Н И М А Н И Е!!!
    // идентификатор платы должен совпасть с программным
#define ID_PLATE  0x2 //0x3 - Т7к  0x2 - Т5к
//#define ID_PLATE  0x2 //0x3 - Т7к  0x2 - Т5к
#define DEL_PWR (100./62. + 1) //  делитель для контроля напряжения батарейки

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint16_t KeyP; // клавиши нажатые 
  char DigitSet = 1; //шаг изменеия устанавливаемого затухания
  char LvlBatInd=8; //индикатор уровня батарейки

  // контроль идентификатора платы
 uint8_t CheckErrID_Plate=0; 

// режим работы прибора для настройки (1 - настройка, 0- работа)
 char ModeWork = 0;
 
unsigned int CheckErrMEM; 
BYTE CurrLang; // текущий язык

DWORD TimeBegin; // время начала, для контроля начальной заставки

uint8_t TxDMA = 0; // признак занятости DMA 
char TxBufAns[512]; // буффер передачи в USB
// переменные UART I2C
uint16_t Dummy; // пустое чтение буфферов UART
uint16_t BufADC[SizeBuf_ADC_int]; // буфер внутреннего АЦП (8), в него пишем при съеме DMA, размер до 8
// строка для индикатора
uint8_t Str[64];
// ВСПОМОГАТЕЛЬНЫЕ ПЕРЕМЕННЫЕ
DWORD CountTimerPA = 0;
char ScreenReDraw=0; // признак необходимости перерисовать экран
char NeedSaveParam=0; // признак необходимости сохранить параметры

uint16_t CurrLevelDAC=0; //текущий уровень для ЦАП (востанавливаем из тех что храним в UserSet)

  uint32_t CcMinute=0; // счетчик минуты (посекундно)

  
float Ubat=4.1; // начальное напряжение батареи
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
  if(ID_PLATE != GETIDPLT)
  {
    CheckErrID_Plate=1;
  }
  if(CheckErrID_Plate)
  {
    MX_DMA_Init();
    MX_UART7_Init();
    // Start Uart7 - Nextion
    uint16_t  Dummy = huart7.Instance->RDR ; // чистим буффер приема от NEXTION
    HAL_UART_Receive_IT(&huart7, RX_BufNEX,1); // ждем принятия первого байта из внешнего мира
    /* disable the UART Parity Error Interrupt */
    __HAL_UART_DISABLE_IT(&huart7, UART_IT_PE);
    /* disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
    __HAL_UART_DISABLE_IT(&huart7, UART_IT_ERR);
    
    // перенастроим UART7  для NEXTION
    huart7.Init.BaudRate = 9600;
    if (HAL_UART_Init(&huart7) != HAL_OK)
    {
      Error_Handler();
    }
    HAL_Delay(10);
    sprintf((void*)Str,"bauds=115200яяя");
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
    sprintf((void*)Str, "t0.txt=\"! ОШИБКА !\"яяя"); // auto
    NEX_Transmit((void*)Str);    // 
    sprintf((void*)Str, "t1.txt=\"!прибор не тот!\"яяя"); // auto
    NEX_Transmit((void*)Str);    // 
    
    while(1);
    
  }

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
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */
  // сразу пробуем поставить ЦАП
  
    HAL_DAC_Start(&hdac1,DAC_CHANNEL_2);

      HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_2,DAC_ALIGN_12B_R,CurrLevelDAC);
  
   // проверяем конфигурацию платы, чтобы не запустить программу по исправленю
  // если вдруг зашили чужую программу, попытаемся написать в индикатор и зациклится
  

  // Start Uart7 - Nextion
  uint16_t  Dummy = huart7.Instance->RDR ; // чистим буффер приема от NEXTION
  HAL_UART_Receive_IT(&huart7, RX_BufNEX,1); // ждем принятия первого байта из внешнего мира
  /* disable the UART Parity Error Interrupt */
  __HAL_UART_DISABLE_IT(&huart7, UART_IT_PE);
  /* disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
  __HAL_UART_DISABLE_IT(&huart7, UART_IT_ERR);

  // перенастроим UART7  для NEXTION
  huart7.Init.BaudRate = 9600;
  if (HAL_UART_Init(&huart7) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_Delay(10);
  sprintf((void*)Str,"bauds=115200яяя");
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
  sprintf((void*)Str, "page 0яяя"); // < START>
  NEX_Transmit((void*)Str);    //
  // пошлем сообщение о включении ...
    sprintf((void*)Str, "t1.txt=\"Включение...\"яяя"); // auto
    NEX_Transmit((void*)Str);    // 
  
 
  
  // так как повторяем конфигурацию из 7kAR, то скомбинируем из DataDevice MemFlash(у нас PCA955x)
  CheckErrMEM =   BeginConfig();
// подготовка внутреннего АЦП
    if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED) != HAL_OK)
  {
      myBeep(100);

    Error_Handler();
   }
        // конец обработки клавы и АЦП, запустим снова АЦП
     // if(HAL_ADC_Start_DMA(&hadc1,(uint32_t*)&BufADC,3) != HAL_OK) Error_Handler();//3

  if (HAL_ADC_Start_DMA(&hadc1,
                        (uint32_t *)&BufADC,
                        3
                          ) != HAL_OK)
  {
      myBeep(100);
    Error_Handler();
  }

    // Start Uart3 - внешний мир
  Dummy = huart3.Instance->RDR ; // чистим буффер приема от SIM
  HAL_UART_Receive_IT(&huart3, RxBufExt,1); // ждем принятия первого байта из внешнего мира
  /* disable the UART Parity Error Interrupt */
  __HAL_UART_DISABLE_IT(&huart3, UART_IT_PE);
  /* disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
  __HAL_UART_DISABLE_IT(&huart3, UART_IT_ERR);
  // Тестовая посылка по UART
  sprintf((void*)TxBufAns,"TEst\n"); //  
  HAL_UART_Transmit(&huart3,(void*)TxBufAns, strlen((void*)TxBufAns),100);
  // Start Uart5 - Optics
  Dummy = huart5.Instance->RDR ; // чистим буффер приема от OPTIC
  HAL_UART_Receive_IT(&huart5, RxBufExt,1); // ждем принятия первого байта из внешнего мира
  /* disable the UART Parity Error Interrupt */
  __HAL_UART_DISABLE_IT(&huart5, UART_IT_PE);
  /* disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
  __HAL_UART_DISABLE_IT(&huart5, UART_IT_ERR);
  //UARTSendExt ((BYTE*)TxBufAns, strlen(TxBufAns));
// инициализация клавиатуры 
  InitBtns(); 
  
    // начало работы..
  TimeBegin = HAL_GetTick();
// получим и посчитаем батарейку
        Ubat = 2.5*DEL_PWR*BufADC[0]/4096; 
        LvlBatInd = (char)(Ubat*10. - 40.)+1;
        if(Ubat<4.0) LvlBatInd = 0;
        if((Ubat>4.9)||(LvlBatInd>8)) LvlBatInd = 8;
        if(GETEXTPWR == 0)  LvlBatInd = 9;
   // CountBat = 0;    
  LvlBatSav.BatControl[0] = CountBat++;
  LvlBatSav.BatControl[CountBat] = Ubat;
  // пропишем в память
  EEPROM_write(&LvlBatSav.BatControl[0], ADR_BatSave  , 4);
  EEPROM_write(&LvlBatSav.BatControl[CountBat], ADR_BatSave +  4*CountBat , 4);


  //CmdInitPage(0);// вызов окна заставки
  //HAL_Delay(10);
  SetMode (ModeWelcome);
  CmdInitPage(0);// посылка команды переключения окна на Welcome и установка признака первого входа
  //  MX_USB_DEVICE_Init();


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // проверка кнопок 
    if((GetSysTick(0)>30)&&(!ProgFW_LCD))// каждые 30 мС или больше...и не в программировании
    {
      KeyP = SetBtnStates( GetExpand (), 1 ); // опрос клавиатуры
      GetSysTick(1);// сброс системного ожидания
      // управление красным лазером
      // поконтролить батарейку
      // инекремент таймаре PA
      // выведем текущий уровень ЦАП
      HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_2,DAC_ALIGN_12B_R,CurrLevelDAC);
      
      CountTimerPA++;
      if(CountTimerPA>33)
      {
        // каждую секунду, посчитаем батарейку
        // (BufADC[0]*(2.5/4096))
        // хорошо заряженные 5.3-5.4 -
        // без аккумулятора от сети вижу 4.33 
        // пока возьмем 4.1 мин - 5.0 макс
        Ubat = 2.5*DEL_PWR*BufADC[0]/4096; 
        LvlBatInd = (char)(Ubat*10. - 40.)+1;
        if(Ubat<4.0) LvlBatInd = 0;
        if((Ubat>4.9)||(LvlBatInd>8)) LvlBatInd = 8;
        if(GETEXTPWR == 0)  LvlBatInd = 9;
        sprintf((void*)Str,"p0.pic=%dяяя",LvlBatInd);
        NEX_Transmit((void*)Str);//
        // получим текущее время и оработаем его
        if(CcMinute++>60)
        {
          if(CountBat++>2048)
            CountBat=1;
          
          LvlBatSav.BatControl[CountBat] = Ubat;
          LvlBatSav.BatControl[0] = CountBat;
          // пропишем в память
          EEPROM_write(&LvlBatSav.BatControl[0], ADR_BatSave  , 4);
          EEPROM_write(&LvlBatSav.BatControl[CountBat], ADR_BatSave +  4*CountBat , 4);
          
          CcMinute=0;
        }
        
        //LvlBatInd++;
        CountTimerPA = 0;
        // здесь можно запустить Измерение АЦП
        if (HAL_ADC_Start_DMA(&hadc1,(uint32_t *)&BufADC,3) != HAL_OK)
        {
          myBeep(100);
          Error_Handler();
        }
      }
      
      
    }
    // проверка приема по UART EXT
    if (RSDecYes) // вызов программы обработки комманды принятой по UART
    {
      DecodeCommandRS();
    }
    
    // режим программирования индикатора и ответы от индикатора
    if(Uart2DecYes)
    {
      if(ProgFW_LCD)
      {
        CDC_Transmit_FS((uint8_t*)RX_BufNEX, CntRXNEX ); // echo back on same channel
      }
      else
      {
        // что-то приняли в ответ от индикатора, можно посмотреть
        CheckStrNEX (); // проверка принятой строки  
        
      }
      Uart2DecYes=0;
      RecievNEX=STOP_UART;
    }
    
    if(NeedSaveParam)
    {
      
      WriteNeedStruct(NeedSaveParam);
      NeedSaveParam = 0;
    }
    // основное отображение режима
    if(!ProgFW_LCD)
      ModeFuncTmp();
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
  RCC_OscInitStruct.PLL.PLLR = 2;
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
                              |RCC_PERIPHCLK_UART5|RCC_PERIPHCLK_UART7
                              |RCC_PERIPHCLK_USART3;
  PeriphClkInitStruct.PLL3.PLL3M = 16;
  PeriphClkInitStruct.PLL3.PLL3N = 96;
  PeriphClkInitStruct.PLL3.PLL3P = 2;
  PeriphClkInitStruct.PLL3.PLL3Q = 4;
  PeriphClkInitStruct.PLL3.PLL3R = 4;
  PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_1;
  PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
  PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
  PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_PLL3;
  PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_PLL3;
  PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL3;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
// тики опроса клавиатура взято из Т7К_АР
uint32_t GetSysTick( int Mode) // получение тиков 1 мС. 0 - получение счетчика от предыдущего сброса 1- сброс
{
  static uint32_t MemTick;
  if(Mode) MemTick=HAL_GetTick();
  return HAL_GetTick()-MemTick;
}

// управление таймером в измерителе АВТОМАТЕ
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
