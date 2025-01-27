/*****************************************************************************
 *   uart.h:  Header file for NXP LPC23xx Family Microprocessors
 *
 *   Copyright(C) 2006, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2006.09.01  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#ifndef __UARTEXT_H 
#define __UARTEXT_H

#
#define BUFSIZE   0x80
#define BUFSIZEUART1   0x80
#define BUFSIZEUART2   0x80

#define STOP_UART  0
#define START_UART  1
#define END_UART  2


void RS_com (BYTE temp);
//  очистка приема по UART0 что бы ждать начала
void ClearRS (void);

void DecodeCommandRS (void);
void SendCfgOTDR (char* BufStr); // передача конфигурации рефлектометра (настройки)
// тестотвое заполнение таблицы событий
   void  TestLoadEvents (int Num);
extern char wrd[10];
extern volatile DWORD Reciev;
extern volatile DWORD CntRX;
extern volatile BYTE RX_Buf[BUFSIZEUART1];


#endif /* end __UART_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
