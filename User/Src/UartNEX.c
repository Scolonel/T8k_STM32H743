/*****************************************************************************
 *   
 *  для STM32H743 - перенесен блок из uart.c T7kAR только для 
 *  NEXTION 
 *  было UART2 теперь uart7  
 *
 *  
 *   2024.11.05  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/

#include "system.h"

//  
volatile BYTE UART2TxEmpty = 1;
volatile BYTE UART2Buffer[BUFSIZEUART2];
volatile DWORD UART2Count = 0;
volatile DWORD RecievNEX = STOP_UART;
uint8_t RX_BufNEX[BUFSIZEUART2];
volatile DWORD CntRXNEX = 0;
volatile BYTE Uart2DecYes = 0;
volatile BYTE ReadyNEX=0; // готовность управления NEXTION
volatile BYTE g_WtRdyNEX = 1;
volatile BYTE ProgFW_LCD =0; // признак программирования индикатора Nexion
volatile WORD CountTimerUart2 = 0;
char wrd[10];
BYTE RSOptYes = 0;
volatile BYTE g_GetStr; // признак ожидания получения строки из редактора. 



void CheckStrNEX (void) // проверка принятой строки
{
     if(g_GetStr==1)//вроде как надо принять строку, и переключится назад 
   {
     if((CntRXNEX>1)&&(RX_BufNEX[CntRXNEX-1]==0xa7)&&(RX_BufNEX[0]==0xa4))
     g_GetStr=2;
   }
}
//#pragma optimize=size  
char GetStringNEX(char* NEXStr, int Size)
{

    char Indx =(CntRXNEX<Size+2)?(CntRXNEX-2):(Size-1);
    memcpy(NEXStr,(void*)&RX_BufNEX[1],Indx); 
    NEXStr[Size-1]=0;
    return Indx;
}

// это в прерывании

void StartRecievNEX (uint32_t TimeOut) // подготовка ожидания ответа от Nextionв некторых случаях, с установкой времени ожидания
{
  RX_BufNEX[0] = 0; // обнуляем буффер приемный от NEXTION
  CntRXNEX=0; // чистим счетчик принятых байт
  RecievNEX=STOP_UART;// переводим в режим приеема первого байта
  g_WtRdyNEX = 0;// чистим признак окончания ожидания по таймауту
  ReadyNEX = 0; // чистим признак принятой команды с дешифровкой ( если разпознали ответ установливаем его номер)
  CountTimerUart2 = TimeOut;// уСТАНВЛИВАЕМ ВРЕМЯ ЗАДЕРЖКИ в mS 
  uint16_t  Dummy = huart7.Instance->RDR ; // чистим буффер приема от NEXTION
  HAL_UART_Receive_IT(&huart7, RX_BufNEX,1); // ждем принятия первого байта из внешнего мира

}





// Прием байта по UART2 LCD
void RS_LCD (BYTE temp)
{
  //     CDC_Transmit(0, (uint8_t*)(TRD), 1); // echo back on same channel
  
  //GetRstTMUart2(1); // сбросим таймер Uart2
  CountTimerUart2 = 50;// уСТАНВЛИВАЕМ ВРЕМЯ ЗАДЕРЖКИ в mS ожидание следующего байта
  if(ProgFW_LCD)
  {
    RX_BufNEX[CntRXNEX]=temp;
    CntRXNEX++;
  }
  else // обычная работа
  {
    switch (RecievNEX)
    {
    case STOP_UART:                    //Состояние готовности к приёму новой команды
      if (temp)  // что-то приняли попробуем начать прием
      {
        CntRXNEX=0;
        RX_BufNEX[CntRXNEX]=temp;
        RecievNEX=START_UART;
        CntRXNEX++;
        if(temp == 0xa4) // это от ответа индикаторана ОК
          g_NeedChkAnsvNEX=1; // начало приема ответа от индикатора по ОК 
      }
      break;
    case START_UART:                   //Состояние приёма текущей команды (исправлено V <-> W) 02/09/03
      {
        switch (temp)
        {
        case 0xa7:                 //Ежели приняли 
          RX_BufNEX[CntRXNEX++]=temp;
          if(g_NeedChkAnsvNEX == 1) // принимаем ответ от ндикатора
          {
            g_NeedChkAnsvNEX = 2; // конец приема можно обработать
          }
          break;
        case 'я':                 //Ежели приняли 
          //CntRXOpt=0;
          RX_BufNEX[CntRXNEX++]=temp;
          if ((CntRXNEX>3)&&(!memcmp ((void*)&RX_BufNEX[CntRXNEX-3], "яяя",3)))
          {
            RecievNEX=END_UART;
            RX_BufNEX[CntRXNEX]=0x0;
            Uart2DecYes = 1;            //Команда принята полностью, поэтому надо обработать установим признакRSDecode();
            switch(RX_BufNEX[0])
            {
            case 0xFD: // завершили передачу блока!
              ReadyNEX = 1;
              break;
            case 0xFE: // готов принять блок
              ReadyNEX = 2;
              break;
            case 0x1A: // error
              ReadyNEX = 3;
              break;
            case 0x70: // text block
              ReadyNEX = 4;
              break;
            case 'c': // text block ответа на connect
              ReadyNEX = 5;
              break;
            default:
              break;
            }
          }//
          
          break;
        default:                  //Продолжение приёма команды
          RX_BufNEX[CntRXNEX]=temp;
          CntRXNEX++;
          if (CntRXNEX > 80)
          {
            CntRXNEX=0;
            RecievNEX=STOP_UART;
          }
          break;
        }
      }
    case END_UART:                     //Состояние принятой полностью команды
      break;                      //Предыдущая команда не отработана - поэтому ничего не делать
    }
  }
}





/******************************************************************************
**                            End Of File
******************************************************************************/
