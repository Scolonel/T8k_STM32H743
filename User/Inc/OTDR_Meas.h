// OTDR_Measure
// 

#ifndef __OTDR_MEAS_H__
#define __OTDR_MEAS_H__


#define PNTVERTICALSIZE 4 // число вертикальных масштабов

void SendSetALT ( WORD Data); // передача слова в Альтеру
WORD CalkSetALT (unsigned int NumPnt, unsigned int CurrPos);
unsigned PosCursorMain (int Dir);//установка и получение значения курсора - главного
unsigned SetPosCursorMain (unsigned Point);//установка  курсора - главного
unsigned PosCursorSlave (int Dir);//установка и получение значения курсора - главного
unsigned SetPosCursorSlave (unsigned Point);//установка  курсора - главного
int GetNumAccumPerSec (void); //получение значения накоплений в данном режиме
void SetNumAccumPerSec (int Data);// установка значения числа накоплений
unsigned GetVerticalSize (BYTE Index);//получение вертикального размера отображения рефлектограммы
//void  SetVerticalSize (unsigned Size);// установка вертикального размера отображения рефлектограммы
unsigned GetSetHorizontScale (int Dir);//получение-установка горизонтального масштаба отображения рефлектограммы
//void ButtonsForOTDR (void);
float Calc_LSA (unsigned int start_P,unsigned int end_P);// вычисление погонного затухания для режима 2


#endif
