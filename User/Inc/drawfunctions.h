#ifndef DRAWFUNCTIONS_H
#define DRAWFUNCTIONS_H

#include "system.h"

#define NOCHANGE 0
#define UPPICS 1
#define DOWNPICS 2
#define UP10PICS 3
#define DOWN10PICS 4
#define MEDIUM 5
#define BYCUR 6
#define MEMDRAW 7
#define FIXSIZE 8

#define ScrH_N  (210)  // Nexion
#define ScrW_N  (390) // Nexion


// Структура для удобства - прямойгольник в который вписывать график
typedef struct tagRect
{
    long    left;
    long    top;
    long    right;
    long    bottom;
} Rect;

// курсорная струтура определяет рисование
typedef struct tagGraphParams
{
  unsigned maxy;      // Максимум по Y
  unsigned cursor;     // Местоположение курсора [0, 4095]
  unsigned scale;     //  Мастштаб: 1 - когда 1 сдвиг курсора смещает всего на 1 точку 
  BYTE numcur;     // число курсоров 0- нет,1- основной 2- еще дополнительный
  BYTE typecentral;     // какую центровку используем 0- нет (текущую),1- персчитываем тип1 2- еще дополнительный
} GraphParams;



//unsigned short MakeGraph( Rect* rct, unsigned short* array, unsigned size, GraphParams* prms );
unsigned MakeGraphNext( Rect* rct, unsigned short* array, unsigned size, GraphParams* prms );
// Рисование картинки из последовательности байтов, длинной определяемой гоиз. размером окна,
// высота соотв. вертикальным точка пивязки также в параметрах окна.
//void DrawPicture (Rect* rct, const unsigned char* file);

unsigned GetLevelCurs (int Dir);

//void DecrBinDec(unsigned char *nm);
//void InitRect (Rect* rct, long left, long top, long right, long bottom);
//void InitParams (GraphParams* prms, unsigned cursor, unsigned scale, unsigned maxy, BYTE numcur, BYTE typecentral);
//void DrawDigitScore(BYTE Digit, int x, int y);
unsigned MinMax( unsigned short* array, unsigned nums );
unsigned MinMaxNEX( unsigned short* array, unsigned nums );
unsigned short Adjust4Nex( int val, int center, int range );
//unsigned short Adjust2( int center, int min, int max, int rng );


typedef struct 
{
  unsigned int x;
  unsigned int y;
}Magnifier;

//void DrawMgn(Magnifier M,char whereto[1024]);

//void MoveTo(Magnifier M,int x,int y,char whereto[1024]);

typedef struct
{
  Magnifier M;
  unsigned int start;
  unsigned int fixcur;// координаты фиксированного курсора
  char mode;
  char *statusbar;
}Desktop;

//void DrawDskt(Desktop D,char whereto[1024], short unsigned int func[OUTSIZE]);//short int

float Calc_LSA (unsigned int start_P,unsigned int end_P);// вычисление погонного затухания для режима 2
void SendDrawNex (unsigned char* array, int ID_lcd, int Points);

#endif
