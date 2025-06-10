
/*****************************************************************************
 *   uartEXT.c:  
 *
 *   
 *   
 *
 *   History
 *   2024.11.06  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/

#include "system.h"

const char* DeviceIDN[2]= {"�����-5000","MOT-500M-60"};

volatile DWORD UART0Status;
volatile BYTE UART0TxEmpty = 1;
volatile BYTE UART0Buffer[BUFSIZE];
volatile DWORD UART0Count = 0;
volatile DWORD Reciev = STOP_UART;
volatile BYTE RX_Buf[BUFSIZEUART1];
volatile DWORD CntRX = 0;
volatile uint32_t RSDecYes = 0;
char NameReadFile[32]; // ���������� ��� ����� ��� ������ � ����������
char BufString[225];
        RTCTime SetNewTime;


void SendBelcoreSet (void); // �������� ��������� �������

// ����� ����� �� UART 
void RS_com (BYTE temp)
{
  switch (Reciev)
  {
  case STOP_UART:                    //��������� ���������� � ����� ����� �������
    if ((temp=='*')||(temp==';'))
    {
      CntRX=0;
      RX_Buf[CntRX]=temp;
      Reciev=START_UART;
      CntRX++;
    }
    break;
  case START_UART:                   //��������� ����� ������� ������� (���������� V <-> W) 02/09/03
    {
      switch (temp)
      {
      case '*':                 //����� ������ ������� �� ����� ����� ���������� - �� ������, �������� �������.
        CntRX=0;
        RX_Buf[CntRX]=temp;
        CntRX++;
        break;
      case ';':                 //����� ������ ������� �� ����� ����� ���������� - �� ������, �������� �������.
        CntRX=0;
        RX_Buf[CntRX]=temp;
        CntRX++;
        break;
      case 0xD:                 //����� �������
        RX_Buf[CntRX]=temp;
        Reciev=END_UART;
        RX_Buf[CntRX+1]=0x0;
        RSDecYes = 1;            //������� ������� ���������, ������� ���� ���������� ��������� �������RSDecode();
        break;
      case 0xA:                 //����� �������
        RX_Buf[CntRX]=temp;
        Reciev=END_UART;
        RX_Buf[CntRX+1]=0x0;
        RSDecYes = 1;            //������� ������� ���������, ������� ���� ���������� ��������� �������RSDecode();
        break;
      default:                  //����������� ����� �������
        RX_Buf[CntRX]=temp;
        CntRX++;
        if (CntRX > BUFSIZEUART1) // ���� ������ ������ ��������� ������
        {
          CntRX=0;
          Reciev=STOP_UART;
        }
        break;
      }
    }
  case END_UART:                     //��������� �������� ��������� �������
    break;                      //���������� ������� �� ���������� - ������� ������ �� ������
  }
 // SCI1TXBUF = temp; // ��� ������ ��� �������
}


//  ������� ������ �� UART0 ��� �� ����� ������
void ClearRS (void)
{
            CntRX=0;
          Reciev=STOP_UART;

}
// ��������� �������� ������� �� RS
void DecodeCommandRS (void)
{
  unsigned char *c;
  unsigned char value;
  uint32_t IndxLW; 
  uint16_t IndxPtn;  // ������ ����� ���������� ���� ���
  float TmpLvlDB;
  //char Str[128];
  //char BufString[225];
  char Str[32];
  char StartStr[7]={"#48419\0"}; // 4-� ������� ������!!!
  //VICINTENCLEAR  = 1 << UART0_INT; /* Disable Interrupt */
  volatile char NeedTransmit; //
  char NeedPrint= 0; //
  int  Mdl=0; // ���������� ������ �� ������������ ����� ����
  // 0 - ������� �� ���������� ���� ������� Err
  // 1 - �������� ������ BufString
  // 2 - ��� ��������  (�������� ����, ������ �� ��������
  NeedTransmit = 0;
  if (Reciev==END_UART)                //����� ���� ������� �������� - ���������
  {
    for (int i=0; ((i<CntRX)&&(RX_Buf[i]!=0x20)); i++ )
    {
      if ((RX_Buf[i] >=0x61) && (RX_Buf[i]<=0x7A))RX_Buf[i] = RX_Buf[i] - 0x20;
    }
    switch (RX_Buf[0])
    {
    case '*':
      //123      if (GetModeDevice() != MODEMEASURE)
      if(1)
      {
        // ������� ������� *IDN -  �������������
        //-----------------
        if (!memcmp ((void*)RX_Buf, "*IDN?",5))
          //if ((RX_Buf[1]=='I')&&(RX_Buf[2]=='D')&&(RX_Buf[3]=='N')&&(RX_Buf[4]=='?'))
        {
                Mdl=0; // ���������� ������ �� ������������ ����� ����
    
            // ������ ��
            sprintf (Str, "v3.%02d%c", NUM_VER/26, (0x61+(NUM_VER%26))); // ������ ������ ���� �� ���� 2.01 (28.06.2022)

          // GetDeviceName( BufString ); // ������ ������ ��������������
          sprintf(BufString,"%s-%d,%s,SN%04d\r",DeviceIDN[ConfigDevice.ID_Device],Mdl,Str,ConfigDevice.NumDevice);

          NeedTransmit = 1;
          UARTSendExt ((BYTE*)BufString, strlen (BufString));
        }
        if (!memcmp ((void*)RX_Buf, "*IDHW",5)) // ������������� "��������" ����������
          //if ((RX_Buf[1]=='I')&&(RX_Buf[2]=='D')&&(RX_Buf[3]=='N')&&(RX_Buf[4]=='?'))
        {
          BufString[0]= 0;
          GetDeviceHW( BufString ); // ������ ������ ��������������
          NeedTransmit = 1;
          UARTSendExt ((BYTE*)BufString, strlen (BufString));
        }
        if (!memcmp ((void*)RX_Buf, "*IDLCD",6)) // ������������� �� "LCD"
          //if ((RX_Buf[1]=='I')&&(RX_Buf[2]=='D')&&(RX_Buf[3]=='N')&&(RX_Buf[4]=='?'))
        {
          sprintf(BufString,"%s\r", VerFW_LCD);
          NeedTransmit = 1;
          UARTSendExt ((BYTE*)BufString, strlen (BufString));
        }
      }
      break;
    case ';':
      //      // ������ ����� 
      //      if (!memcmp ((void*)RX_Buf, ";SCR?",5)) // �������� ��� �� �����!
      //      {
      //        // ������� ������� ;SCR? - 
      //        // ;scr?  - �������� ���� ��������
      //        // �������� �������� �������� (���������)
      //        sprintf (StartStr, "#41024");
      //        UARTSendExt ((BYTE*)StartStr, 6);
      //        UARTSendExt ((BYTE*)GetScreen(), 1024);
      //      }
      
      //  ;syst:uart:hi ��������� �������� UART 460800 ����� ��� �� ������� ��������
      if (!memcmp ((void*)RX_Buf, ";SYST:UART:HI",13)) //
      {
        UARTSendExt ((BYTE*)"OK\r", 3);
        NeedTransmit = 1;
        // �������� �� ������ �������� 
        //while ( !(UART0TxEmpty & 0x01) ); // ���� ����� �������� ������ ����� ����� ���������������
        huart3.Init.BaudRate = 460800;
        if (HAL_UART_Init(&huart3) != HAL_OK)
        {
          Error_Handler();
        }
        
        
      }
      if (!memcmp ((void*)RX_Buf, ";SYST:UART:ME",13)) //115200
      {
        UARTSendExt ((BYTE*)"OK\r", 3);
        NeedTransmit = 1;
        // �������� �� ������ ��������
        //while ( !(UART0TxEmpty & 0x01) ); // ���� ����� �������� ������ ����� ����� ���������������
        
        huart3.Init.BaudRate = 115200;
        if (HAL_UART_Init(&huart3) != HAL_OK)
        {
          Error_Handler();
        }
        
        
      }
      //  ;syst:uart:lo ��������� �������� UART 57600 ����� ��� �� ������� ��������
      if (!memcmp ((void*)RX_Buf, ";SYST:UART:LO",13)) //
      {
        UARTSendExt ((BYTE*)"OK\r", 3);
        // �������� �� ������ ��������
        NeedTransmit = 1;
        //while ( !(UART0TxEmpty & 0x01) ); // ���� ����� �������� ������ ����� ����� ���������������
        
        huart3.Init.BaudRate = 57600;
        if (HAL_UART_Init(&huart3) != HAL_OK)
        {
          Error_Handler();
        }
      }
      // 
      
      // ;syst:date dd,mm,yyyy - ��������� ����
      if (!memcmp ((void*)RX_Buf, ";SYST:DATE",10)) //
      {
        int i=11;
        //NeedTransmit = 0;

        DWORD Num;
        //RTCTime SetNewTime;
        SetNewTime = RTCGetTime(); // �������� ������� �����
        if(RX_Buf[10] == ' ') // ���� ���������� �����
        {
        Num = atoi((char*)&RX_Buf[i]); // ���
        if ((Num>99)||(Num<25)) Num = 25;
        SetNewTime.RTC_Year = Num;
        i++;
        if (RX_Buf[i] != ',') i++;
        i++;
        Num = atoi((char*)&RX_Buf[i]); // �����
        if ((Num>12)||(Num<1)) Num = 1;
        SetNewTime.RTC_Mon = Num;
        i++;
        if (RX_Buf[i] != ',') i++;
        i++;
        Num = atoi((char*)&RX_Buf[i]); // ����
        if ((Num>31)||(Num<1)) Num = 1;
        SetNewTime.RTC_Mday = Num;
        
        RTCSetTime( SetNewTime ); // ������� ����� ����
        HAL_Delay(100);
        NeedPrint = 1;
        }
        if(RX_Buf[10] == '?') // ���� send �����
        {
        NeedPrint = 1;
        }
        //New
        if(NeedPrint)
        {
        NeedTransmit = 1;
        NeedPrint = 0;
        sprintf(BufString,"%02d,%02d,%02d\r" // ������ ����
                ,SetNewTime.RTC_Year
                  ,SetNewTime.RTC_Mon
                    ,SetNewTime.RTC_Mday);
        UARTSendExt ((BYTE*)BufString, strlen (BufString));
        }
      }
      // ;syst:time hh,mm,ss - ��������� �������
      if (!memcmp ((void*)RX_Buf, ";SYST:TIME",10)) //
      {
        int i=11;
        DWORD Num;
        //RTCTime SetNewTime;
        SetNewTime = RTCGetTime(); // �������� ������� �����
        if(RX_Buf[10] == ' ') // ���� ���������� �����
        {
        Num = atoi((char*)&RX_Buf[i]); // ����
        if (Num>23) Num = 0;
        SetNewTime.RTC_Hour = Num;
        i++;
        if (RX_Buf[i] != ',') i++;
        i++;
        Num = atoi((char*)&RX_Buf[i]); // ������
        if (Num>59) Num = 0;
        SetNewTime.RTC_Min = Num;
        i++;
        if (RX_Buf[i] != ',') i++;
        i++;
        Num = atoi((char*)&RX_Buf[i]); //������� 
        if (Num>59) Num = 0;
        SetNewTime.RTC_Sec = Num;
        
        RTCSetTime( SetNewTime ); // ������� ����� �����
        NeedPrint = 1;
        }
        if(RX_Buf[10] == '?') // ���� send �����
        {
        NeedPrint = 1;
        }
        //New
        if(NeedPrint)
        {
        NeedTransmit = 1;
        NeedPrint = 0;
        sprintf(BufString,"%02d,%02d,%02d\r" // ������ ����
                ,SetNewTime.RTC_Hour
                  ,SetNewTime.RTC_Min
                    ,SetNewTime.RTC_Sec);
        UARTSendExt ((BYTE*)BufString, strlen (BufString));
        }
        
      }
      // ;syst:bat? -  ������ ������ �� ��������� (����)
      if (!memcmp ((void*)RX_Buf, ";SYST:BAT?",10)) //
      {
                  EEPROM_read(&LvlBatSav, ADR_BatSave, sizeof(LvlBatSav)); //�������� ������� � ����������� ������� 
                  sprintf(BufString,"%d\n",CountBat); // ������ ������
                  UARTSendExt ((BYTE*)BufString, strlen (BufString));
                  HAL_Delay(1);
        for(int i= 1;i<2048;i++)
        {
                  sprintf(BufString,"%.3f\n",LvlBatSav.BatControl[i]); // ������ ������
                  UARTSendExt ((BYTE*)BufString, strlen (BufString));
                  HAL_Delay(1);
        }
        NeedTransmit = 1;
      }
      // ;syst:key ss -  �������� ������� ������
      if (!memcmp ((void*)RX_Buf, ";SYST:KEY ",10)) //
      {
        int i=10;
        DWORD Num;
        unsigned char nBtn, nSts; 
        Num = atoi((char*)&RX_Buf[i]); // 
        nBtn = Num%10;
        nSts = (Num/10)%10;
        SetKeyMd (nBtn, nSts);
        sprintf(BufString,"%d\r",Num); // ������ // 
        UARTSendExt ((BYTE*)BufString, strlen (BufString));
        NeedTransmit = 1;
      }
      
      
      //===== ������� ������ ��������� ======================    
      if (1)
      {
        
        WORD Data;
        //NeedTransmit = 0;
        
        // ;set:DAC  - ������ � ������ ��� �� ������
        if (!memcmp ((void*)RX_Buf, ";SET:DAC",8)) //
        {
          
          if(RX_Buf[8] == ' ') // ���� ���������� ��������
          {
            Data = (WORD)atoi((char*)&RX_Buf[9]);
            if(Data>4095) Data = 1000;
            CurrLevelDAC = Data;  
            NeedTransmit = 1;
          }
          if(RX_Buf[8] == 'D') // ���� ���������� �������� ����������
          {
            if(CurrLevelDAC>1) CurrLevelDAC--;
            else CurrLevelDAC = 4095;  
            NeedTransmit = 1;
          }
          if(RX_Buf[8] == 'U') // ���� ���������� �������� ����������
          {
            if(CurrLevelDAC<4095) CurrLevelDAC++;
            else CurrLevelDAC = 1000;  
            NeedTransmit = 1;
          }
          if(RX_Buf[8] == '?') // ���� send �����
          {
            NeedTransmit = 1;
          }
          //New
          if(NeedTransmit)
          {
            if(ModeWork)// ���� � ��������� ���� �������� �����
              g_NeedScr = 1;
            sprintf(BufString,"%04d\r",CurrLevelDAC);//c
            UARTSendExt ((BYTE*)BufString, strlen (BufString));
            
          }
        }        
        // ;set:MOD  - ����� �������
        if (!memcmp ((void*)RX_Buf, ";SET:MOD",8)) //
        {
          
          if(RX_Buf[8] == ' ') // ���� ���������� �����
          {
            Data = (WORD)atoi((char*)&RX_Buf[9]);
            if(Data) ModeWork = 1;
            else
            ModeWork = 0;  
            g_NeedScr = 1;
            NeedTransmit = 1;
          }
          if(RX_Buf[8] == '?') // ���� send �����
          {
            NeedTransmit = 1;
          }
          //New
          if(NeedTransmit)
          {
            sprintf(BufString,"%d\r",ModeWork);//c
            UARTSendExt ((BYTE*)BufString, strlen (BufString));
          }
        }        
        // ;set:num  - ����� �������
        if (!memcmp ((void*)RX_Buf, ";SET:SN",7)) //
        {
          
          if(RX_Buf[7] == ' ') // ���� ���������� �����
          {
            Data = (WORD)atoi((char*)&RX_Buf[8]);
            SetNumDevice(Data); // ��������� ������  �������      
            NeedTransmit = 1;
            NeedSaveParam |=0x01;
          }
          if(RX_Buf[7] == '?') // ���� send �����
          {
            NeedTransmit = 1;
          }
          //New
          if(NeedTransmit)
          {
            sprintf(BufString,"SN#%04d\r",GetNumDevice());//c
            UARTSendExt ((BYTE*)BufString, strlen (BufString));
          }
        }        
        // ;set:LWi  - ��������� ���� ���� �� ������
         
        // ��������� ������ � ��, ����������� � ����� �����
                
        // ��������� ���������� �����. �� ����� � ������ � �� �������� ;SET:SHF1 234,1599
                
        // ;set:LAMDA XXXX // ��������� ��������� ����� �����
        
      }
        // ������ ���� ������������
                
      
      // ;syst:set:manufactured  - ��������� �������������
      if (!memcmp ((void*)RX_Buf, ";SYST:SET:MANUFACTURED ",23)) //
      {
        SetID_Dev((BYTE)atoi((char*)&RX_Buf[23])); // ��������� �������������� ��� ���� ������
        //CurrLang = GetLang(CURRENT);
        //GetDeviceName( BufString ); // ������ ������ ��������������
        UARTSendExt ((BYTE*)BufString, strlen (BufString));
        WriteNeedStruct (0x01); // ��������� ���������
        NeedTransmit = 1;
      }

      if (!memcmp ((void*)RX_Buf, ";SET:FWLCDON",12)) //
      {
        SetMode(UploadFW_Nextion);
        CmdInitPage(4); // ������������� �� ����� ��������� ��������� ��� � ������ ����������������
        //ProgFW_LCD = 1; // ��������� � First ��������� ��� �� "���������" ��������
        //123          //UART2Count = 0;
        //123          //UART0Count = 0;
        sprintf(BufString,"OK\r"); // 
        UARTSendExt ((BYTE*)BufString, strlen (BufString));//
        NeedTransmit = 1;
      }
      if (!memcmp ((void*)RX_Buf, ";SET:FWLCDOFF",13)) //
      {
        sprintf(BufString,"OK\r"); // 
        UARTSendExt ((BYTE*)BufString, strlen (BufString));//
        ProgFW_LCD = 2;
        NeedTransmit = 1;
      }
      
      // ���������� ���������� � ������� ������ 
      
      break; // �� ;
    }
    if(NeedTransmit == 0)
    {
      sprintf(BufString,"Err\r");// 
      UARTSendExt ((BYTE*)BufString, strlen (BufString));
      
    }
    
  }
  RSDecYes = 0;
  Reciev = STOP_UART;
  //VICINTENABLE = 1 << UART0_INT;  /* Enable Interrupt */
  
}







/******************************************************************************
**                            End Of File
******************************************************************************/
