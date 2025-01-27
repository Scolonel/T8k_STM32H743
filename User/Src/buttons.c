#include "main.h"

static volatile unsigned short KeyPressed = 0;
volatile unsigned char rawPressKeyS = 0; // ѕоказатель того, что нажатие клавиши не обработано программой "сырое нажатие"

typedef struct DeviceBtn
{
  unsigned char State;
  unsigned Time;
  uint16_t Bind;
} BTNS;


static volatile BTNS Buttons[BTNS_NUM];
//struct BTNS Buttons[BTNS_NUM];

void InitBtns(void)
{
  for( unsigned i = 0; i < BTNS_NUM; ++i )
  {
    Buttons[i].State = NOT_PRESSED;
    Buttons[i].Time = 0;
  }
  // корректировка кнопочного управлени€ дл€ плат с пленочной клавиатурой
  Buttons[5].Bind = BTN_UP;
  Buttons[4].Bind = BTN_DOWN;
  Buttons[3].Bind = BTN_LEFT;
  Buttons[2].Bind = BTN_RIGHT;
  Buttons[1].Bind = BTN_OK;
  Buttons[0].Bind = BTN_MENU;
}



char getStateButtons ( int a)
{
  for(unsigned i = 0; i < BTNS_NUM; ++i )
    if( a == Buttons[i].Bind )
      return Buttons[i].State;
  return 0;
 // if (a<BTNS_NUM) return (Buttons[a].State);
 // else return (0);
}

void SetKeyMd (unsigned char Keys, unsigned char States)
{
  char Str[32];
  unsigned BtnName[6] = {BTN_MENU,BTN_OK,BTN_RIGHT,BTN_LEFT,BTN_DOWN,BTN_UP};
  unsigned char StateName[3] = {SHORT_PRESSED,LONG_PRESSED,INF_PRESSED};
  if ((Keys < 6)&&(States<3))
  {
    for(unsigned i = 0; i < BTNS_NUM; ++i )
    {
      if( BtnName[Keys] == Buttons[i].Bind )
      {
        Buttons[i].State = StateName[States];
        KeyP |= BtnName[Keys];
      }
    }
  }
  if(Keys == 6) // press key "S"
              rawPressKeyS = 1;
 if(Keys == 7) // presskey "OK" in LCD
 {
    sprintf(Str, "click brok,1€€€"); // тест кнопка ок на клавиатуре
    NEX_Transmit((void*)Str);    //
 }
   
  // Buttons[1].State = 4;
  // KeyP |= BTN_OK;
}

void ClrKey (uint16_t Button) // сброс бита обработанной клавиши
{
  KeyPressed &= ~Button;
  for(unsigned i = 0; i < BTNS_NUM; ++i )
  {
    if( (Button & Buttons[i].Bind) && (Buttons[ i ].State > M_LONG_PRESSED) )
    {
         Buttons[ i ].State = NOT_PRESSED;
         Buttons[ i ].Time = 0;
   }
  }
}


uint16_t SetBtnStates( unsigned int states, unsigned int timeStep )
{
  //static volatile DWORD NewKeyS =0;
  static volatile BTNS* cb;
  KeyPressed = 0;
  for( unsigned i = 0; i < BTNS_NUM; ++i )
  {
    cb = (BTNS*)&Buttons[i]; //current button
    if (!( cb->Bind & states ))
    {
      if( cb->State == NOT_PRESSED )
        cb->State = RAW_PRESSED;
      else
      {
        cb->Time += timeStep;
        if( cb->Time == SHORT_TIME )
        {
          //Sound(3);
          cb->State = SHORT_PRESSED;
          KeyPressed |= (uint16_t)(cb->Bind);// // нажали кнопку
        }
        if( cb->Time == LONG_TIME )
        {
          //Sound(3);
          cb->State = LONG_PRESSED;
          KeyPressed |= cb->Bind;// // нажали кнопку
        }
        if( cb->Time >= INF_TIME )
        {
          cb->Time = INF_TIME;
          cb->State = INF_PRESSED;
          KeyPressed |= cb->Bind;// // нажали кнопку
          // м б что-то вызываем или зажигаем (fireEvent)
        }
      }
    }
    else  // кнопку отпустили
    {
      if( cb->State != NOT_PRESSED )
      {
        switch( cb->State )
        {
        case SHORT_PRESSED: // отпустили коротко нажатую кнопку
        cb->State = UP_SHORT_PRESSED;
        cb->Time = 0;
        KeyPressed |= cb->Bind;// // нажали кнопку
          break;
        case LONG_PRESSED: // отпустили долго нажатую кнопку
        cb->State = UP_LONG_PRESSED;
        cb->Time = 0;
        KeyPressed |= cb->Bind;// // нажали кнопку
         break;
        case INF_PRESSED: // отпустили непрерывно нажатую кнопку
        cb->State = UP_INF_PRESSED;
        cb->Time = 0;
        KeyPressed |= cb->Bind;// // нажали кнопку
         break;
        case RAW_PRESSED:
        case UP_SHORT_PRESSED: // отпустили коротко нажатую кнопку
        case UP_LONG_PRESSED: // отпустили долго нажатую кнопку
        //case INF_PRESSED: // отпустили непрерывно нажатую кнопку
        case UP_INF_PRESSED: // отпустили непрерывно нажатую кнопку
        cb->State = NOT_PRESSED;
        cb->Time = 0;
          break;
        }
        //cb->State = NOT_PRESSED;
        //cb->Time = 0;
      }
    }
  }
  KeyP = KeyPressed;
  return (KeyPressed);
}