#ifndef __BUTTONS_H
#define __BUTTONS_H





#define BTNS_NUM 6

//#define NOT_PRESSED 0
//#define RAW_PRESSED 4 // возможно уже нажали а может дребезг
//#define M_SHORT_PRESSED 7
//#define M_LONG_PRESSED 8
//#define SHORT_PRESSED 1
//#define LONG_PRESSED 2
//#define INF_PRESSED 3
//#define UP_SHORT_PRESSED 5
//#define UP_LONG_PRESSED 6
#define NOT_PRESSED 0
#define RAW_PRESSED 1 // возможно уже нажали а может дребезг
#define M_SHORT_PRESSED 2
#define M_LONG_PRESSED 3
#define SHORT_PRESSED 4
#define LONG_PRESSED 5
#define INF_PRESSED 6
#define UP_SHORT_PRESSED 7
#define UP_LONG_PRESSED 8
#define UP_INF_PRESSED 9

#define SHORT_TIME 3
#define LONG_TIME 25
#define INF_TIME 30

#define BTN_UP  0x20
#define BTN_DOWN  0x01
#define BTN_LEFT  0x08
#define BTN_RIGHT  0x04
#define BTN_START  0x04
#define BTN_OK      0x10
#define BTN_MENU    0x02
#define PRESS( a ) (KeyP & a)


unsigned short SetBtnStates( unsigned int states, unsigned int timeStep );
char getStateButtons ( int a);
void ClrKey (uint16_t Button); // сброс бита обработанной клавиши
void SetKeyMd (unsigned char Keys, unsigned char States);


void InitBtns(void);



#endif
