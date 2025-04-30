
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

const char* DeviceIDN[2]= {"ТОПАЗ-5000","MOT-500M-60"};

volatile DWORD UART0Status;
volatile BYTE UART0TxEmpty = 1;
volatile BYTE UART0Buffer[BUFSIZE];
volatile DWORD UART0Count = 0;
volatile DWORD Reciev = STOP_UART;
volatile BYTE RX_Buf[BUFSIZEUART1];
volatile DWORD CntRX = 0;
volatile uint32_t RSDecYes = 0;
char NameReadFile[32]; // глобальная имя файла при чтении в сохранении
char BufString[225];
        RTCTime SetNewTime;


void SendBelcoreSet (void); // посылает установки белкора

// Прием байта по UART 
void RS_com (BYTE temp)
{
  switch (Reciev)
  {
  case STOP_UART:                    //Состояние готовности к приёму новой команды
    if ((temp=='*')||(temp==';'))
    {
      CntRX=0;
      RX_Buf[CntRX]=temp;
      Reciev=START_UART;
      CntRX++;
    }
    break;
  case START_UART:                   //Состояние приёма текущей команды (исправлено V <-> W) 02/09/03
    {
      switch (temp)
      {
      case '*':                 //Ежели начало посылки во время приёма предыдущей - то ошибка, начинаем сначала.
        CntRX=0;
        RX_Buf[CntRX]=temp;
        CntRX++;
        break;
      case ';':                 //Ежели начало посылки во время приёма предыдущей - то ошибка, начинаем сначала.
        CntRX=0;
        RX_Buf[CntRX]=temp;
        CntRX++;
        break;
      case 0xD:                 //Конец команды
        RX_Buf[CntRX]=temp;
        Reciev=END_UART;
        RX_Buf[CntRX+1]=0x0;
        RSDecYes = 1;            //Команда принята полностью, поэтому надо обработать установим признакRSDecode();
        break;
      case 0xA:                 //Конец команды
        RX_Buf[CntRX]=temp;
        Reciev=END_UART;
        RX_Buf[CntRX+1]=0x0;
        RSDecYes = 1;            //Команда принята полностью, поэтому надо обработать установим признакRSDecode();
        break;
      default:                  //Продолжение приёма команды
        RX_Buf[CntRX]=temp;
        CntRX++;
        if (CntRX > BUFSIZEUART1) // если сторка больше приемного буфера
        {
          CntRX=0;
          Reciev=STOP_UART;
        }
        break;
      }
    }
  case END_UART:                     //Состояние принятой полностью команды
    break;                      //Предыдущая команда не отработана - поэтому ничего не делать
  }
 // SCI1TXBUF = temp; // эхо приема для отладки
}


//  очистка приема по UART0 что бы ждать начала
void ClearRS (void)
{
            CntRX=0;
          Reciev=STOP_UART;

}
// обработка принятой команды по RS
void DecodeCommandRS (void)
{
  unsigned char *c;
  unsigned char value;
  uint32_t IndxLW; 
  uint16_t IndxPtn;  // индекс точки сохранения кода ЦАП
  float TmpLvlDB;
  //char Str[128];
  //char BufString[225];
  char Str[32];
  char StartStr[7]={"#48419\0"}; // 4-х значные номера!!!
  //VICINTENCLEAR  = 1 << UART0_INT; /* Disable Interrupt */
  volatile char NeedTransmit; //
  char NeedPrint= 0; //
  int  Mdl=0; // определяем модель по конфигурации длинн волн
  // 0 - команда не обработана надо выслать Err
  // 1 - передаем буффер BufString
  // 2 - уже передали  (бинарный блок, ничего не передаем
  NeedTransmit = 0;
  if (Reciev==END_UART)                //Ежели приём команды закончен - обработка
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
        // приняли признак *IDN -  идентификатор
        //-----------------
        if (!memcmp ((void*)RX_Buf, "*IDN?",5))
          //if ((RX_Buf[1]=='I')&&(RX_Buf[2]=='D')&&(RX_Buf[3]=='N')&&(RX_Buf[4]=='?'))
        {
                Mdl=0; // определяем модель по конфигурации длинн волн
                for (int y=0;y<4;++y)
          {
            if((ConfigDevice.PlaceLW[y]>0)&&(ConfigDevice.PlaceLW[y]<1310))
              Mdl|=1;
            if(ConfigDevice.PlaceLW[y]>1300)
              Mdl|=2;
          }
    
            // версия ПО
            sprintf (Str, "v3.%02d%c", NUM_VER/26, (0x61+(NUM_VER%26))); // версия должна быть не ниже 2.01 (28.06.2022)

          // GetDeviceName( BufString ); // запрос сторки идентификатора
          sprintf(BufString,"%s-%d,%s,SN%04d\r",DeviceIDN[ConfigDevice.ID_Device],Mdl,Str,ConfigDevice.NumDevice);

          NeedTransmit = 1;
          UARTSendExt ((BYTE*)BufString, strlen (BufString));
        }
        if (!memcmp ((void*)RX_Buf, "*IDHW",5)) // идентификатор "железной" реализации
          //if ((RX_Buf[1]=='I')&&(RX_Buf[2]=='D')&&(RX_Buf[3]=='N')&&(RX_Buf[4]=='?'))
        {
          // GetDeviceHW( BufString ); // запрос сторки идентификатора
          NeedTransmit = 1;
          UARTSendExt ((BYTE*)BufString, strlen (BufString));
        }
        if (!memcmp ((void*)RX_Buf, "*IDLCD",6)) // идентификатор ПО "LCD"
          //if ((RX_Buf[1]=='I')&&(RX_Buf[2]=='D')&&(RX_Buf[3]=='N')&&(RX_Buf[4]=='?'))
        {
          sprintf(BufString,"%s\r", "none");
          NeedTransmit = 1;
          UARTSendExt ((BYTE*)BufString, strlen (BufString));
        }
      }
      break;
    case ';':
      //      // выдать экран 
      //      if (!memcmp ((void*)RX_Buf, ";SCR?",5)) // возможно уже не нужна!
      //      {
      //        // приняли признак ;SCR? - 
      //        // ;scr?  - получить файл картинки
      //        // начинаем передачу картинки (Заголовок)
      //        sprintf (StartStr, "#41024");
      //        UARTSendExt ((BYTE*)StartStr, 6);
      //        UARTSendExt ((BYTE*)GetScreen(), 1024);
      //      }
      
      //  ;syst:uart:hi установка скорости UART 460800 ответ уже на большой скорости
      if (!memcmp ((void*)RX_Buf, ";SYST:UART:HI",13)) //
      {
        UARTSendExt ((BYTE*)"OK\r", 3);
        NeedTransmit = 1;
        // отсылаем на старой скорости 
        //while ( !(UART0TxEmpty & 0x01) ); // ждем конца передачи только после этого перестраиваемся
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
        // отсылаем на старой скорости
        //while ( !(UART0TxEmpty & 0x01) ); // ждем конца передачи только после этого перестраиваемся
        
        huart3.Init.BaudRate = 115200;
        if (HAL_UART_Init(&huart3) != HAL_OK)
        {
          Error_Handler();
        }
        
        
      }
      //  ;syst:uart:lo установка скорости UART 57600 ответ уже на меньшей скорости
      if (!memcmp ((void*)RX_Buf, ";SYST:UART:LO",13)) //
      {
        UARTSendExt ((BYTE*)"OK\r", 3);
        // отсылаем на старой скорости
        NeedTransmit = 1;
        //while ( !(UART0TxEmpty & 0x01) ); // ждем конца передачи только после этого перестраиваемся
        
        huart3.Init.BaudRate = 57600;
        if (HAL_UART_Init(&huart3) != HAL_OK)
        {
          Error_Handler();
        }
      }
      // 
      
      // ;syst:date dd,mm,yyyy - установка даты
      if (!memcmp ((void*)RX_Buf, ";SYST:DATE",10)) //
      {
        int i=11;
        //NeedTransmit = 0;

        DWORD Num;
        //RTCTime SetNewTime;
        SetNewTime = RTCGetTime(); // получаем текущее время
        if(RX_Buf[10] == ' ') // надо установить время
        {
        Num = atoi((char*)&RX_Buf[i]); // год
        if ((Num>99)||(Num<25)) Num = 25;
        SetNewTime.RTC_Year = Num;
        i++;
        if (RX_Buf[i] != ',') i++;
        i++;
        Num = atoi((char*)&RX_Buf[i]); // месяц
        if ((Num>12)||(Num<1)) Num = 1;
        SetNewTime.RTC_Mon = Num;
        i++;
        if (RX_Buf[i] != ',') i++;
        i++;
        Num = atoi((char*)&RX_Buf[i]); // день
        if ((Num>31)||(Num<1)) Num = 1;
        SetNewTime.RTC_Mday = Num;
        
        RTCSetTime( SetNewTime ); // запишем новую дату
        HAL_Delay(100);
        NeedPrint = 1;
        }
        if(RX_Buf[10] == '?') // надо send время
        {
        NeedPrint = 1;
        }
        //New
        if(NeedPrint)
        {
        NeedTransmit = 1;
        NeedPrint = 0;
        sprintf(BufString,"%02d,%02d,%02d\r" // выдаем дату
                ,SetNewTime.RTC_Year
                  ,SetNewTime.RTC_Mon
                    ,SetNewTime.RTC_Mday);
        UARTSendExt ((BYTE*)BufString, strlen (BufString));
        }
      }
      // ;syst:time hh,mm,ss - установка времени
      if (!memcmp ((void*)RX_Buf, ";SYST:TIME",10)) //
      {
        int i=11;
        DWORD Num;
        //RTCTime SetNewTime;
        SetNewTime = RTCGetTime(); // получаем текущее время
        if(RX_Buf[10] == ' ') // надо установить время
        {
        Num = atoi((char*)&RX_Buf[i]); // часы
        if (Num>23) Num = 0;
        SetNewTime.RTC_Hour = Num;
        i++;
        if (RX_Buf[i] != ',') i++;
        i++;
        Num = atoi((char*)&RX_Buf[i]); // минуты
        if (Num>59) Num = 0;
        SetNewTime.RTC_Min = Num;
        i++;
        if (RX_Buf[i] != ',') i++;
        i++;
        Num = atoi((char*)&RX_Buf[i]); //секунды 
        if (Num>59) Num = 0;
        SetNewTime.RTC_Sec = Num;
        
        RTCSetTime( SetNewTime ); // запишем новое время
        NeedPrint = 1;
        }
        if(RX_Buf[10] == '?') // надо send время
        {
        NeedPrint = 1;
        }
        //New
        if(NeedPrint)
        {
        NeedTransmit = 1;
        NeedPrint = 0;
        sprintf(BufString,"%02d,%02d,%02d\r" // выдаем дату
                ,SetNewTime.RTC_Hour
                  ,SetNewTime.RTC_Min
                    ,SetNewTime.RTC_Sec);
        UARTSendExt ((BYTE*)BufString, strlen (BufString));
        }
        
      }
      // ;syst:key ss -  имитация нажатия кнопки
      if (!memcmp ((void*)RX_Buf, ";SYST:KEY ",10)) //
      {
        int i=10;
        DWORD Num;
        unsigned char nBtn, nSts; 
        Num = atoi((char*)&RX_Buf[i]); // 
        nBtn = Num%10;
        nSts = (Num/10)%10;
        SetKeyMd (nBtn, nSts);
        sprintf(BufString,"%d\r",Num); // выдаем // 
        UARTSendExt ((BYTE*)BufString, strlen (BufString));
        NeedTransmit = 1;
      }
      
      
      //===== КОМАНДЫ РЕЖИМА УСТАНОВОК ======================    
      if (1)
      {
        
        WORD Data;
        //NeedTransmit = 0;
        
        // ;set:DAC  - работа с кодами ЦАП на прямую
        if (!memcmp ((void*)RX_Buf, ";SET:DAC",8)) //
        {
          
          if(RX_Buf[8] == ' ') // надо установить значение
          {
            Data = (WORD)atoi((char*)&RX_Buf[9]);
            if(Data>4095) Data = 1000;
            CurrLevelDAC = Data;  
            NeedTransmit = 1;
          }
          if(RX_Buf[8] == 'D') // надо установить значение увеличение
          {
            if(CurrLevelDAC>1) CurrLevelDAC--;
            else CurrLevelDAC = 4095;  
            NeedTransmit = 1;
          }
          if(RX_Buf[8] == 'U') // надо установить значение увеличение
          {
            if(CurrLevelDAC<4095) CurrLevelDAC++;
            else CurrLevelDAC = 1000;  
            NeedTransmit = 1;
          }
          if(RX_Buf[8] == '?') // надо send время
          {
            NeedTransmit = 1;
          }
          //New
          if(NeedTransmit)
          {
            if(ModeWork)// если в настройке надо обновить экран
              g_NeedScr = 1;
            sprintf(BufString,"%04d\r",CurrLevelDAC);//c
            UARTSendExt ((BYTE*)BufString, strlen (BufString));
            
          }
        }        
        // ;set:MOD  - режим прибора
        if (!memcmp ((void*)RX_Buf, ";SET:MOD",8)) //
        {
          
          if(RX_Buf[8] == ' ') // надо установить режим
          {
            Data = (WORD)atoi((char*)&RX_Buf[9]);
            if(Data) ModeWork = 1;
            else
            ModeWork = 0;  
            g_NeedScr = 1;
            NeedTransmit = 1;
          }
          if(RX_Buf[8] == '?') // надо send время
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
        // ;set:num  - НОМЕР ПРИБОРА
        if (!memcmp ((void*)RX_Buf, ";SET:SN",7)) //
        {
          
          if(RX_Buf[7] == ' ') // надо установить номер
          {
            Data = (WORD)atoi((char*)&RX_Buf[8]);
            SetNumDevice(Data); // установка номера  прибора      
            NeedTransmit = 1;
            NeedSaveParam |=0x01;
          }
          if(RX_Buf[7] == '?') // надо send время
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
        // ;set:LWi  - установка длин волн по местам
        if (!memcmp ((void*)RX_Buf, ";SET:LW",7)) //
        {
          
          if((RX_Buf[7] == '0')||(RX_Buf[7] == '1')||(RX_Buf[7] == '2')||(RX_Buf[7] == '3')) // надо установить номер длины волны
          {
            IndxLW = (uint32_t)atoi((char*)&RX_Buf[7]);
            Data = (WORD)atoi((char*)&RX_Buf[9]);
            ConfigDevice.PlaceLW[IndxLW] = Data;
            sprintf(BufString,"%d\r",Data);//c
            UARTSendExt ((BYTE*)BufString, strlen (BufString));
            NeedTransmit = 1;
            NeedSaveParam |=0x01;
          }
          if(RX_Buf[7] == '?') // надо send время
          {
            NeedTransmit = 1;
          }
          //New
          if(NeedTransmit)
          {
            sprintf(BufString,"%d\n%d\n%d\n%d\r" // выдаем длины волн конфигурации
                    ,ConfigDevice.PlaceLW[0]
                      ,ConfigDevice.PlaceLW[1]
                        ,ConfigDevice.PlaceLW[2]
                          ,ConfigDevice.PlaceLW[3]);
            UARTSendExt ((BYTE*)BufString, strlen (BufString));
          }
        } 
        // установка уровня в дБ, привязанных к длине волны
        if (!memcmp ((void*)RX_Buf, ";SET:DB",7)) //
        {
          
          if(RX_Buf[7] == ' ') // надо принять значение
          {
            TmpLvlDB = (float)atof((char*)&RX_Buf[7]);
            if ((TmpLvlDB*SHAG>=0)&&(TmpLvlDB*SHAG<=((ConfigDevice.PlaceLW[UserSet.iCurrLW]>1300)?(MAX_DB):(MAX_DB/2))))
            {     
              UserSet.iLvlCurrLW[UserSet.iCurrLW]=(uint16_t)(TmpLvlDB*SHAG);
              CurrLevelDAC = CoeffLW.SetCoefLW[UserSet.iCurrLW][UserSet.iLvlCurrLW[UserSet.iCurrLW]];
              NeedTransmit = 1;
              NeedSaveParam |=0x02;
            }
          }
          if(RX_Buf[7] == '?') // надо send время
          {
            NeedTransmit = 1;
          }
          //New
          if(NeedTransmit)
          {
            sprintf(BufString,"%04d=%.2f\r",UserSet.iLvlCurrLW[UserSet.iCurrLW],UserSet.iLvlCurrLW[UserSet.iCurrLW]/SHAG);
            UARTSendExt ((BYTE*)BufString, strlen (BufString));
          }
        }        
        // установка табличного коэфф. по месту в памяти и ее значение ;SET:SHF1 234,1599
        if (!memcmp ((void*)RX_Buf, ";SET:SHF",8)) //
        {
            IndxLW = (WORD)atoi((char*)&RX_Buf[8]); // получаем индекс для длины волны
          if(IndxLW<4) // можно попробовать записать
          {
            IndxPtn=(uint16_t)atoi((char*)&RX_Buf[10]);  // индекс точки сохранения кода ЦАП
            if(IndxPtn<=1600) // !пишем 1601 ячейку
            {
              // ищем значение оно после ','
              int i=11;
              while((RX_Buf[i]!=',')&&(i<17)) 
              {
                i++;
              }
              if (i++<17)
              { // можно прочитать значение и записать
                Data=(uint16_t)atoi((char*)&RX_Buf[i]);  //значение точки сохранения кода ЦАП
                if(Data < 4096)
                {
                  UserSet.iCurrLW = IndxLW;
                  UserSet.iLvlCurrLW[UserSet.iCurrLW] = IndxPtn;
                  CoeffLW.SetCoefLW[UserSet.iCurrLW][UserSet.iLvlCurrLW[UserSet.iCurrLW]] = (uint16_t)Data;
                  CurrLevelDAC = CoeffLW.SetCoefLW[UserSet.iCurrLW][UserSet.iLvlCurrLW[UserSet.iCurrLW]];
                  // надо быстро сохранить 2 байта, пробуем
                  //EEPROM_write((void*)&CurrLevelDAC, ADR_CoeffPM + 4096*IndxLW + 2*IndxPtn , 2);
                   EEPROM_write(&CurrLevelDAC, ADR_CoeffPM + 4096*IndxLW + 2*IndxPtn , 2);
                 //NeedSaveParam |=0x06;
                  g_NeedScr = 1;
                  NeedTransmit = 1;
                  
                  //New
                  if(NeedTransmit)
                  {
                    sprintf(BufString,"%dnm %d %d\r",ConfigDevice.PlaceLW[UserSet.iCurrLW],UserSet.iLvlCurrLW[UserSet.iCurrLW],CoeffLW.SetCoefLW[UserSet.iCurrLW][UserSet.iLvlCurrLW[UserSet.iCurrLW]]);
                    UARTSendExt ((BYTE*)BufString, strlen (BufString));
                  }
                }
              }
            }
          }
        }        
        // ;set:LAMDA XXXX // установка требуемой длины волны
        if (!memcmp ((void*)RX_Buf, ";SET:LAMDA",10)) //
        {
          if(RX_Buf[10] == ' ') // надо установить длины волны 
          {
            Data = (WORD)atoi((char*)&RX_Buf[11]);
            if((Data==ConfigDevice.PlaceLW[0])||(Data==ConfigDevice.PlaceLW[1])||(Data==ConfigDevice.PlaceLW[2])||(Data==ConfigDevice.PlaceLW[3]))
            {
              while(Data!=ConfigDevice.PlaceLW[UserSet.iCurrLW]) //проверяем иустанавливаем требуемую длину волны 
              {
                if (UserSet.iCurrLW<3) UserSet.iCurrLW++;
                else UserSet.iCurrLW=0;
              }
              // востанавливаем уровень ЦАП для данной длины волны
              CurrLevelDAC = CoeffLW.SetCoefLW[UserSet.iCurrLW][UserSet.iLvlCurrLW[UserSet.iCurrLW]];
              g_NeedScr = 1;
              NeedTransmit = 1;
              NeedSaveParam |=0x02;
            }
          }
          if(RX_Buf[10] == '?') // надо send время
          {
            NeedTransmit = 1;
          }
          //New
          if(NeedTransmit)
          {
            sprintf(BufString,"%04d\r",ConfigDevice.PlaceLW[UserSet.iCurrLW]);//c
            UARTSendExt ((BYTE*)BufString, strlen (BufString));
          }
          
        }
      }
        // запрос всей конфигурации
        if (!memcmp ((void*)RX_Buf, ";SET:ALL?",9)) //
        {
          Mdl=0; // определяем модель по конфигурации длинн волн
          for (int y=0;y<4;++y)
          {
            if((ConfigDevice.PlaceLW[y]>0)&&(ConfigDevice.PlaceLW[y]<1310))
              Mdl|=1;
            if(ConfigDevice.PlaceLW[y]>1300)
              Mdl|=2;
          }
          // прибор
          sprintf(BufString,"%s-%d;\n",MsgMass[6][UserSet.CurrLang],Mdl);
          UARTSendExt ((BYTE*)BufString, strlen (BufString));
          // серийный номер
          sprintf(BufString,"SN#%04d;\n",ConfigDevice.NumDevice);
          UARTSendExt ((BYTE*)BufString, strlen (BufString));
          // версия ПО
          sprintf (Str, "v3.%02d%c", NUM_VER/26, (0x61+(NUM_VER%26))); // версия должна быть не ниже 2.01 (28.06.2022)
          sprintf(BufString,"%s;\n",Str);
          UARTSendExt ((BYTE*)BufString, strlen (BufString));
          // список длин волн
          for (int k=0;k<4;++k)
          {
            sprintf(BufString,"%dnm - %2.2f, %2.2f, %2.2f\n",ConfigDevice.PlaceLW[k],UserSet.iLvlCurrLW[k]/SHAG,UserSet.iLvlFixLW[k][0]/SHAG,UserSet.iLvlFixLW[k][1]/SHAG );
            UARTSendExt ((BYTE*)BufString, strlen (BufString));
          }
          // таблица коэффициентов
          for (int y=0;y<=1600;++y)
          {
            sprintf(BufString,"%.2f %04d %04d %04d %04d\n",y*0.05,CoeffLW.SetCoefLW[0][y],CoeffLW.SetCoefLW[1][y],CoeffLW.SetCoefLW[2][y],CoeffLW.SetCoefLW[3][y]);
        UARTSendExt ((BYTE*)BufString, strlen (BufString));
          }
          sprintf(BufString,"\r");
        UARTSendExt ((BYTE*)BufString, strlen (BufString));
          NeedTransmit = 1;
          
        }        
      
      // ;syst:set:manufactured  - установка производителя
      if (!memcmp ((void*)RX_Buf, ";SYST:SET:MANUFACTURED ",23)) //
      {
        SetID_Dev((BYTE)atoi((char*)&RX_Buf[23])); // установка идентификатора для кого прибор
        //CurrLang = GetLang(CURRENT);
        //GetDeviceName( BufString ); // запрос сторки идентификатора
        UARTSendExt ((BYTE*)BufString, strlen (BufString));
        WriteNeedStruct (0x01); // сохраняем изменения
        NeedTransmit = 1;
      }

      if (!memcmp ((void*)RX_Buf, ";SET:FWLCDON",12)) //
      {
        SetMode(UploadFW_Nextion);
        CmdInitPage(4); // переключаемся на режим индикации сообщения что в режиме программирования
        //ProgFW_LCD = 1; // перенесем в First обработку что бы "правильно" работало
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
      
      // управление источником и красным глазом 
      
      break; // от ;
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
