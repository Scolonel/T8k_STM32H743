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
    // !!!� � � � � � � �!!!
    // ������������� ����� ������ �������� � �����������
#define ID_PLATE  0x1 //0x3 - �7�  0x2 - �5�
//#define ID_PLATE  0x2 //0x3 - �7�  0x2 - �5�
#define DEL_PWR (100./62. + 1) //  �������� ��� �������� ���������� ���������

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint16_t KeyP; // ������� ������� 
  char DigitSet = 1; //��� �������� ���������������� ���������
  char LvlBatInd=8; //��������� ������ ���������

  // �������� �������������� �����
 uint8_t CheckErrID_Plate=0; 

// ����� ������ ������� ��� ��������� (1 - ���������, 0- ������)
 char ModeWork = 0;
 
unsigned int CheckErrMEM; 
BYTE CurrLang; // ������� ����

DWORD TimeBegin; // ����� ������, ��� �������� ��������� ��������

uint8_t TxDMA = 0; // ������� ��������� DMA 
char TxBufAns[512]; // ������ �������� � USB
// ���������� UART I2C
uint16_t Dummy; // ������ ������ �������� UART
uint16_t BufADC[SizeBuf_ADC_int]; // ����� ����������� ��� (8), � ���� ����� ��� ����� DMA, ������ �� 8
// ������ ��� ����������
uint8_t Str[64];
char VerFW_LCD[25] = {"No version LCD          \0"}; //������ �� ���������� NEXION

// ��������������� ����������
DWORD CountTimerPA = 0;
char ScreenReDraw=0; // ������� ������������� ������������ �����
char NeedSaveParam=0; // ������� ������������� ��������� ���������

uint16_t CurrLevelDAC=0; //������� ������� ��� ��� (�������������� �� ��� ��� ������ � UserSet)

uint32_t CcMinute=0; // ������� ������ (����������)
uint32_t BadLevelBat=0; //����� ������� ������ ���������
uint32_t BadBatCnt = 0; // ������� ������� ������ ���������
  
float Ubat=4.6; // ��������� ���������� �������
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
    uint16_t  Dummy = huart7.Instance->RDR ; // ������ ������ ������ �� NEXTION
    HAL_UART_Receive_IT(&huart7, RX_BufNEX,1); // ���� �������� ������� ����� �� �������� ����
    /* disable the UART Parity Error Interrupt */
    __HAL_UART_DISABLE_IT(&huart7, UART_IT_PE);
    /* disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
    __HAL_UART_DISABLE_IT(&huart7, UART_IT_ERR);
    
    // ������������ UART7  ��� NEXTION
    huart7.Init.BaudRate = 9600;
    if (HAL_UART_Init(&huart7) != HAL_OK)
    {
      Error_Handler();
    }
    HAL_Delay(10);
    sprintf((void*)Str,"bauds=115200���");
    HAL_UART_Transmit(&huart7, (void*)Str,strlen((void*)Str),20); // ������ 
    
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
    sprintf((void*)Str, "t0.txt=\"! ������ !\"���"); // auto
    NEX_Transmit((void*)Str);    // 
    sprintf((void*)Str, "t1.txt=\"!������ �� ���!\"���"); // auto
    NEX_Transmit((void*)Str);    // 
    
    while(1);
    
  }
  HAL_Delay(10);
  sprintf((void*)Str, "page 0���"); // < START>
  NEX_Transmit((void*)Str);    //
//       StartRecievNEX (500);
//    sprintf((void*)Str,"get t10.txt���");
//    NEX_Transmit((void*)Str);//
    //NEX_Transmit((void*)CmdBuf);//
  HAL_Delay(10);
  sprintf((void*)Str, "page 0���"); // < START>
  NEX_Transmit((void*)Str);    //
  HAL_Delay(10);
//    sprintf((void*)Str, "t0.txt=\"������\"���"); // auto
//    NEX_Transmit((void*)Str);    // 
//      HAL_Delay(10);

       StartRecievNEX (600);
    sprintf((void*)Str,"get t10.txt���");
    NEX_Transmit((void*)Str);//
  //HAL_Delay(200);
    while(!((g_WtRdyNEX)||(ReadyNEX==4)));
       StartRecievNEX (600);
    sprintf((void*)Str,"get t10.txt���");
    NEX_Transmit((void*)Str);//
  //HAL_Delay(200);
    while(!((g_WtRdyNEX)||(ReadyNEX==4)));
    // ����� ������ ����� ��������� �� ���������� ������� �� ����������
    // ��� ����� ��� ������ �����������
    // ���� ����� ��������� ������
    if(RX_BufNEX[0] == 0x70) // ���� �����! ��������� ������
    {
      for(int i=0;i<25;++i)VerFW_LCD[i]=RX_BufNEX[i+1];
      VerFW_LCD[23]=0;
      // ����� ������� ������������� ���������� (���� ��� �������)
      // �� ����� ��� ��������� ����������� ��� ��������� ������������� � � ������
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
  // ������ ��������� � ��������� ...
    sprintf((void*)Str, "t1.txt=\"���������...\"���"); // auto
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
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */
  // ����� ������� ��������� ���
  
    HAL_DAC_Start(&hdac1,DAC_CHANNEL_2);

      HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_2,DAC_ALIGN_12B_R,CurrLevelDAC);
  
   // ��������� ������������ �����, ����� �� ��������� ��������� �� ����������
  // ���� ����� ������ ����� ���������, ���������� �������� � ��������� � ����������
  

  // Start Uart7 - Nextion
  Dummy = huart7.Instance->RDR ; // ������ ������ ������ �� NEXTION
  HAL_UART_Receive_IT(&huart7, RX_BufNEX,1); // ���� �������� ������� ����� �� �������� ����
  /* disable the UART Parity Error Interrupt */
  __HAL_UART_DISABLE_IT(&huart7, UART_IT_PE);
  /* disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
  __HAL_UART_DISABLE_IT(&huart7, UART_IT_ERR);

//  // ������������ UART7  ��� NEXTION
//  huart7.Init.BaudRate = 9600;
//  if (HAL_UART_Init(&huart7) != HAL_OK)
//  {
//    Error_Handler();
//  }
//  HAL_Delay(10);
//  sprintf((void*)Str,"bauds=115200���");
//  HAL_UART_Transmit(&huart7, (void*)Str,strlen((void*)Str),20); // ������ 
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
//  sprintf((void*)Str, "page 0���"); // < START>
//  NEX_Transmit((void*)Str);    //
//  // ������ ��������� � ��������� ...
//    sprintf((void*)Str, "t1.txt=\"���������...\"���"); // auto
//    NEX_Transmit((void*)Str);    // 
  
 
  
  // ��� ��� ��������� ������������ �� 7kAR, �� ������������ �� DataDevice MemFlash(� ��� PCA955x)
  CheckErrMEM =   BeginConfig();
// ���������� ����������� ���
    if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED) != HAL_OK)
  {
      myBeep(100);

    Error_Handler();
   }
        // ����� ��������� ����� � ���, �������� ����� ���
     // if(HAL_ADC_Start_DMA(&hadc1,(uint32_t*)&BufADC,3) != HAL_OK) Error_Handler();//3

  if (HAL_ADC_Start_DMA(&hadc1,
                        (uint32_t *)&BufADC,
                        3
                          ) != HAL_OK)
  {
      myBeep(100);
    Error_Handler();
  }

    // Start Uart3 - ������� ���
  Dummy = huart3.Instance->RDR ; // ������ ������ ������ �� SIM
  HAL_UART_Receive_IT(&huart3, RxBufExt,1); // ���� �������� ������� ����� �� �������� ����
  /* disable the UART Parity Error Interrupt */
  __HAL_UART_DISABLE_IT(&huart3, UART_IT_PE);
  /* disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
  __HAL_UART_DISABLE_IT(&huart3, UART_IT_ERR);
  // �������� ������� �� UART
  sprintf((void*)TxBufAns,"TEst\n"); //  
  HAL_UART_Transmit(&huart3,(void*)TxBufAns, strlen((void*)TxBufAns),100);
  // Start Uart5 - Optics
  Dummy = huart5.Instance->RDR ; // ������ ������ ������ �� OPTIC
  HAL_UART_Receive_IT(&huart5, RxBufExt,1); // ���� �������� ������� ����� �� �������� ����
  /* disable the UART Parity Error Interrupt */
  __HAL_UART_DISABLE_IT(&huart5, UART_IT_PE);
  /* disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
  __HAL_UART_DISABLE_IT(&huart5, UART_IT_ERR);
  //UARTSendExt ((BYTE*)TxBufAns, strlen(TxBufAns));
// ������������� ���������� 
  InitBtns(); 
  
    // ������ ������..
  TimeBegin = HAL_GetTick();
// ������� � ��������� ���������
        Ubat = 2.5*DEL_PWR*BufADC[0]/4096; 
        // ������� ������ ����h����, ��� ���������
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
  // �������� � ������
  EEPROM_write(&LvlBatSav.BatControl[0], ADR_BatSave  , 4);
  EEPROM_write(&LvlBatSav.BatControl[CountBat], ADR_BatSave +  4*CountBat , 4);


  //CmdInitPage(0);// ����� ���� ��������
  //HAL_Delay(10);
  SetMode (ModeWelcome);
  CmdInitPage(0);// ������� ������� ������������ ���� �� Welcome � ��������� �������� ������� �����
  //  MX_USB_DEVICE_Init();


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // �������� ������ 
    if((GetSysTick(0)>30)&&(!ProgFW_LCD))// ������ 30 �� ��� ������...� �� � ����������������
    {
      KeyP = SetBtnStates( GetExpand (), 1 ); // ����� ����������
      GetSysTick(1);// ����� ���������� ��������
      // ���������� ������� �������
      // ������������ ���������
      // ���������� ������� PA
      // ������� ������� ������� ���
      HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_2,DAC_ALIGN_12B_R,CurrLevelDAC);
      
      CountTimerPA++;
      if(CountTimerPA>33)
      {
        // ������ �������, ��������� ���������
        // (BufADC[0]*(2.5/4096))
        // ������ ���������� 5.3-5.4 -
        // ��� ������������ �� ���� ���� 4.33 
        // ���� ������� 4.1 ��� - 5.0 ����
        Ubat = 2.5*DEL_PWR*BufADC[0]/4096; 
        // ������� ������ ����h����, ��� ���������
        if(Ubat > 5.1) LvlBatInd = 8;
        else if (Ubat > 5.01) LvlBatInd = 7;
        else if (Ubat > 4.92) LvlBatInd = 6;
        else if (Ubat > 4.83) LvlBatInd = 5;
        else if (Ubat > 4.74) LvlBatInd = 4;
        else if (Ubat > 4.65) LvlBatInd = 3;
        else if (Ubat > 4.56) LvlBatInd = 2;
        else if (Ubat >= 4.5) LvlBatInd = 1;
        // ������� �������
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
            BadBatCnt++; //���� ���� ���
            //LED_START(1);//On  LED
            //myBeep(10);
            //HAL_Delay(50);
            //LED_START(0);//Off  LED
            
          }
          if(!BadLevelBat && (BadBatCnt==3))
          { // ������������� � ����� ��������� ������ ��������
            BadLevelBat = 1;
            SetMode (BadBattery);
            CmdInitPage(5);
          }
          
        }
        else // ������� � �����
        {
          BadBatCnt = 0;
          
        }
        //LvlBatInd = (char)(Ubat*10. - 40.)+1;
        //if(Ubat<4.0) LvlBatInd = 0;
        //if((Ubat>4.9)||(LvlBatInd>8)) LvlBatInd = 8;
        sprintf((void*)Str,"p0.pic=%d���",LvlBatInd);
        NEX_Transmit((void*)Str);//
        // ������� ������� ����� � ��������� ���
        if(CcMinute++>60)
        {
          if(CountBat++>2048)
            CountBat=1;
          
          LvlBatSav.BatControl[CountBat] = Ubat;
          LvlBatSav.BatControl[0] = CountBat;
          // �������� � ������
          EEPROM_write(&LvlBatSav.BatControl[0], ADR_BatSave  , 4);
          EEPROM_write(&LvlBatSav.BatControl[CountBat], ADR_BatSave +  4*CountBat , 4);
          
          CcMinute=0;
        }
        
        //LvlBatInd++;
        CountTimerPA = 0;
        // ����� ����� ��������� ��������� ���
        if (HAL_ADC_Start_DMA(&hadc1,(uint32_t *)&BufADC,3) != HAL_OK)
        {
          myBeep(100);
          Error_Handler();
        }
      }
      
      
    }
    // �������� ������ �� UART EXT
    if (RSDecYes) // ����� ��������� ��������� �������� �������� �� UART
    {
      DecodeCommandRS();
    }
    
    // ����� ���������������� ���������� � ������ �� ����������
    if(Uart2DecYes)
    {
      if(ProgFW_LCD)
      {
        CDC_Transmit_FS((uint8_t*)RX_BufNEX, CntRXNEX ); // echo back on same channel
      }
      else
      {
        // ���-�� ������� � ����� �� ����������, ����� ����������
        CheckStrNEX (); // �������� �������� ������  
        
      }
      Uart2DecYes=0;
      RecievNEX=STOP_UART;
    }
    
    if(NeedSaveParam)
    {
      
      WriteNeedStruct(NeedSaveParam);
      NeedSaveParam = 0;
    }
    // �������� ����������� ������
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
// ���� ������ ���������� ����� �� �7�_��
uint32_t GetSysTick( int Mode) // ��������� ����� 1 ��. 0 - ��������� �������� �� ����������� ������ 1- �����
{
  static uint32_t MemTick;
  if(Mode) MemTick=HAL_GetTick();
  return HAL_GetTick()-MemTick;
}

// ���������� �������� � ���������� ��������
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
