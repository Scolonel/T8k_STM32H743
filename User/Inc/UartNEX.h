/*****************************************************************************
 *   UartNEX.h:  Header file for NEXTION
 *
 *   
 *
 *   
 *   2024.11.05  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#ifndef __UARTNEX_H 
#define __UARTNEX_H


#define BUFSIZEUART2   0x80

#define STOP_UART  0
#define START_UART  1
#define END_UART  2


// Прием байта по UART2 LCD
void RS_LCD (BYTE temp);

void StartRecievNEX (uint32_t TimeOut); // подготовка ожидания ответа от Nextionв некторых случаях, с установкой времени ожидания

void CheckStrNEX (void); // проверка принятой строки 
char GetStringNEX(char* NEXStr, int Size);


#endif /* end __UART_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
