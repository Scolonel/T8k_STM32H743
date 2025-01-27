#include "system.h"

#define TICS 48
#define MAXPOSCURSOR 4095

/*
static unsigned int PointInPeriod = 0; // указатель точки в периоде при накоплении
unsigned int PreWait; 
static int CntAccumulat; // счетчик накоплений
static unsigned int PointsPerPeriod; // „исло точек в периоде 
BYTE NeedCalkData = 0;
*/
const unsigned VerticalSize[2][PNTVERTICALSIZE] = {{ 22000, 16000, 8000, 4000 },{ 32000, 16000, 8000, 4000 }}; // вертикальные масштабы  при просмотре

  static unsigned PosCursor = 0; // позици€ курсора
  static unsigned PosCursorSl = 0; // позици€ курсора второго
  static int NumAccumPerSec = 100; //число накоплениий на врем€ перерисовывани€
//  static unsigned VerticalSize = 35000; //вертикальный размер окна отображени€ ирефлектограммы

int GetNumAccumPerSec (void) //получение значени€ накоплений в данном режиме
{
  return NumAccumPerSec;
}

void SetNumAccumPerSec (int Data)// установка значени€ числа накоплений
{
  NumAccumPerSec = Data;
}

unsigned GetVerticalSize (BYTE Index)//получение вертикального размера отображени€ рефлектограммы
{ 
  return VerticalSize[GetApdiSet()][Index];
}
unsigned GetSetHorizontScale (int Dir)//получение горизонтального масштаба отображени€ рефлектограммы
{
  int MaxIndexSc;
 if(ENAOLDLCD)
   MaxIndexSc=5; // старый черно белый 128 точек (мах =32)
 else
 {
   if(TypeLCD)
     MaxIndexSc=3; // ÷ветной 3.5" 480 точек (мах=8)
   else
     MaxIndexSc=4; // ÷ветной 3.2" 400 точек (мах=16)
 }
  
  static int IndexScale=5;
  IndexScale +=Dir;
  if (IndexScale>MaxIndexSc)IndexScale=MaxIndexSc;
  if (IndexScale<0)IndexScale=0;
  return (1<<IndexScale);
}


//void GetPointInPeriod
DWORD CalkZondImpuls (void)
{
    DWORD Imp=0;
  switch (GetIndexIM()) //шаг 4.166666 нс
  {
    case 0: // 10нс- (real 8.33 ns)
    Imp=0x2;
    break;
    case 1: // 40нс (real 39,9 ns)
    Imp=0x9;//10
    break;
    case 2: // 150нс (real 152.68)
    Imp=35;//36
    break;
    case 3: // 500нс (485.8ns)
    Imp=119;//120
    break;
    case 4: // 1000нс (999.36ns)
    Imp=239;//240
    break;
    case 5:  //3000  (2998.08) шаг 13.88*2=27.76
    Imp=719; //720
    break;
    case 6://10000  (10215.68) шаг 13.88*8=111.4
    Imp=2399; //2400
    break;
    case 7://20000  (10215.68) шаг 13.88*8=111.4
    Imp=4799; //4800
    break;
    default:
    Imp=0;
    break;
  }
  return Imp;
}

void SendSetALT ( WORD Data)
{
}

WORD CalkSetALT (unsigned int NumPnt, unsigned int CurrPos)
{
  WORD   PreWait = (CalkZondImpuls())<<6;
  WORD Factor = ((CalkZondImpuls()&0x4)>>2)+((CalkZondImpuls()&0x10)>>3);
  Factor = 0x1<<Factor;
  PreWait |= (((CurrPos)*(TICS/(Factor*NumPnt))) & 0x3f); // —читаем индекс первой записи массива дл€ счета с прореживанием!!!!!!!!!!! 
  //PreWait = (PreWait & 0x3f);
  //PreWait |= 0x1800;  дописываем устанавливаем частоту 72 ћ√ц
  return PreWait;
}

// функции управлени€ индикатором при отображении рефлектограмм

unsigned PosCursorMain (int Dir)//установка и получение значени€ курсора - главного
{
  PosCursor +=Dir;
    if (PosCursor>0x80000000) PosCursor = 0;
    if (PosCursor>MAXPOSCURSOR) PosCursor = MAXPOSCURSOR;
return PosCursor;  
}
unsigned PosCursorSlave (int Dir)//установка и получение значени€ курсора - второго
{
  PosCursorSl +=Dir;
    if (PosCursorSl>0x80000000) PosCursorSl = 0;
    if (PosCursorSl>MAXPOSCURSOR) PosCursorSl = MAXPOSCURSOR;
return PosCursorSl;  
}

unsigned SetPosCursorMain (unsigned Point)//установка  курсора - главного
{
    if (Point>4095) PosCursor = 4095;
    else PosCursor = Point;
return PosCursor;  
  
}
unsigned SetPosCursorSlave (unsigned Point)//установка  курсора - второго
{
    if (Point>4095) PosCursorSl = 4095;
    else PosCursorSl = Point;
return PosCursorSl;  
  
}

//void ButtonsForOTDR (void)
//{
//
//    if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))
//  {
//    Sound(10);
//      PosCursorMain (1);
//    ClrKey (BTN_RIGHT);
//  }
//  if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))
//  {
//    Sound(10);
//      PosCursorMain (-1);
//    ClrKey (BTN_LEFT);
//  }
//  if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==INF_PRESSED))
//  {
//    //Sound(10);
//      PosCursorMain (20);
//    //ClrKey (BTN_RIGHT);
//  }
//  if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==INF_PRESSED))
//  {
//    //Sound(10);
//      PosCursorMain (-20);
//    //ClrKey (BTN_LEFT);
//  }
//}

float Calc_LSA (unsigned int start_P,unsigned int end_P)// вычисление погонного затухани€ дл€ режима 2
{
 unsigned int x; 
  float pp;
  float qq;
  float pq;
  float b1;
  float b2;
  float d1;
  float d2;
  float t1;
  float t2;
  float phi;
  float c;
  float s;
  float m;
  int i;
  float b;
  float gg = GetPosLine(1);
  if (start_P >= end_P)
  {
    x=start_P;
    start_P = end_P;
    end_P = x+1;
  }
  pp = end_P-start_P+1;
  qq = 0;
  pq = 0;
  b1 = 0;
  b2 = 0;
  for (i=0; i<pp; i++)
  {
    pq = pq + (start_P+i)*gg;
    qq = qq + ((start_P+i)*gg)*((start_P+i)*gg);
    b1 = b1 + LogData[start_P+i]/1000.0;
    b2 = b2 + (LogData[start_P+i]/1000.0)*((start_P+i)*gg);
  }
  phi = atan2(2*pq, qq-pp)/2;
  c = cos(phi);
  s = sin(phi);
  d1 = c*c*pp + s*s*qq - 2*s*c*pq;
  d2 = s*s*pp + c*c*qq + 2*s*c*pq;
  if (fabs(d1) > fabs(d2))
  {
    m= fabs(d1);
  }
  else 
  {
    m= fabs(d2);
  }
  t1 = c*b1-s*b2;
  t2 = s*b1+c*b2;
  if (fabs(d1) > m*5e-10)
  {
    t1 = t1/d1;
  }
  else 
  {
    t1 =0;
  }  
  if (fabs(d2) > m*5e-10)
  {
    t2 = t2/d2;
  }
  else 
  {
    t2 =0;
  }  
  b=-s*t1+c*t2;
  return b;
}

