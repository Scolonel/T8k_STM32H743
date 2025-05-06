// структура пользовательских установок
#include "system.h"

char NumVer[10];// = "180      \0";
//const char *NumVer = {
//  #include "version.h"
//}
//;
 const uint8_t* TxGenOpt={"UUUUUUUUUUUUUUUUU"};

const unsigned WAVE_LENGTHS[WAVE_LENGTHS_NUM] = {850,1310,1490,1550,1625};


const char *Ides[2] = {"OOO NPK SvyazServis,\0","OPTOKON Co. Ltd,\0"};
const char *VerFW[2] = {",SW_rev \0",",SW_rev \0"};
//const char *Device[2][LANG_NUM] = {{"ТОПАЗ-7\0","TOPAZ-7\0","TOPAZ-7\0"},{"MOT-700\0","MOT-700\0","MOT-700\0"}};
const char *Device[2][2][2] = {{{"ТОПАЗ-7\0","ТОПАЗ-7\0"},{"TOPAZ-7\0","TOPAZ-7\0"}},{{"MOT-700\0","MOT-950\0"},{"MOT-700\0","MOT-950\0"}}};

const char *PMset[6] = {"10\0","31\0","32\0","-PMH\0","-VFL\0","AR\0"};
const DWORD MultIndex[LENGTH_LINE_NUM]= {1,1,1,1,2,4,8};//множитель уменьшения частоты съема
const DWORD NumPointsPeriod[LENGTH_LINE_NUM]= {8,4,2,1,1,1,1};// число точек на период
const DWORD LengthLine[LENGTH_LINE_NUM]= {2,4,8,16,32,64,128};
const DWORD DelayPeriod[LENGTH_LINE_NUM]= {700,1200,4000,8000,10000,0,0};// задержка периода в тиках CreatDelay()~ 83.33 нс
const WORD WidthPulse[2][WIDTH_PULSE_NUM]= {{20,40,150,500,1000,3000,10000,20000},{10,40,150,500,1000,3000,10000,20000}};
const DWORD TimeAver[TIME_AVR_NUM]= {15,30,60,180,3,600};
const unsigned TimeLight[TIME_LIGHT_NUM] = {0,15,30};
const char *IdnsBC[2]= {"SvyazServis   \0","OPTOKON Co.Ltd\0"};
const char *MfidBC[2][2]= {{"TOPAZ7\0","TOPAZ7\0"},{"MOT700\0","MOT950\0"}};

// структура данных о батарейки, пишем по элементу каждую минуту от включения(0...1023)
BAT_CNTR LvlBatSav;

uint32_t CountBat; // счетчик записей по батареи

// структура установочных коэфф (0...4095)
TAB_SET CoeffLW;
//// структура конфигурации прибора
CFG_DEV ConfigDevice;
//// структура пользоваетьских установок (текущая) для востановления
CFG_USR UserSet;

//char CommentsOTDR[20] = {"                   \0"}; //комментарии рефлектометра перенесено из modes.c
uint8_t StrTXOpt[64]; // буффер передачи данных по оптике

uint8_t TxOptBusy=0; // признак занятости UatrOptic
uint8_t TimeSCWP=0; // счетчик времени для переключения лазеров

WORD ProcBat , ProcBatInd; // процент баттареи


static volatile unsigned short CheckSum = 0;




//
unsigned InvalidDevice() //boolean actually
{
  BYTE N_LS=0;
     unsigned err_str = 0;
  unsigned res = ConfigDevice.NumDevice > 9999; // 0x80
  for( unsigned i = 0; i < LSPLACENUM/*PLACE_LS_NUM*/;++i ) // какие длины волн поддерживает
  {
    if (ConfigDevice.PlaceLW[i]!=0) N_LS++;
  res = (res<<1) | (ConfigDevice.PlaceLW[i] > 1650); //0x40,0x20,0x10,0x08 (1310, 1550, 850, 1300)
 }
  res = (res<<1) | (ConfigDevice.ID_Device >1); // чей прибор (топаз или MOT) 0x04
  res = (res<<1) | (ConfigDevice.Ena_DB > 1); // разрешение альтернативного имени 0x02
    for( unsigned i = 0; i < (sizeof(ConfigDevice.AltName)-1);++i )
  {
    if(ConfigDevice.AltName[i]<0x20) 
    {
      err_str = 1; // в комментариях есть управляющие символы
      break;
    }
  }
  res = (res<<1) | err_str; // 0x01
  // Закроем строку
  ConfigDevice.AltName[19]=0;

  return res;
}

void InitDevice(unsigned Err)
{
  if (Err && 0x01)  
  {
  for( unsigned i = 0; i < (ARRAY_SIZE(ConfigDevice.AltName)-2);++i )
  {
    ConfigDevice.AltName[i]=' '; 
  }
  ConfigDevice.AltName[(ARRAY_SIZE(ConfigDevice.AltName)-1)]=0;
  }
  if (Err && 0x02)  ConfigDevice.Ena_DB = 0; // альтернативного имени нет
  if (Err && 0x04) ConfigDevice.ID_Device = 0; // топаз
  if (Err && 0x08) ConfigDevice.PlaceLW[3] = 0; // 1300
  if (Err && 0x10) ConfigDevice.PlaceLW[2] = 0; // 850
  if (Err && 0x20) ConfigDevice.PlaceLW[1] = 1550; // 1550
  if (Err && 0x40) ConfigDevice.PlaceLW[0] = 1310; // 1310
  if (Err && 0x80) ConfigDevice.NumDevice = 0;
}

DWORD CheckUserGonfig (void)  // Проверка пользовательских настроек И исправление
{
  DWORD Err =0;
  if ((UserSet.CurrLang > MAX_LANG )||((ConfigDevice.ID_Device > 0)&&(UserSet.CurrLang == Rus))) //LANG_NUM-1
  {
    Err |=0x100;
    if ((ConfigDevice.ID_Device > 0)&&(UserSet.CurrLang == Rus)) UserSet.CurrLang = Eng;// для Чехов чисто английский
    else UserSet.CurrLang = Rus;
    
  }
  if (UserSet.iCurrLW>3) // текущий рабочий индекс длины волны по ней определяем откуда берем уровень
  {
    Err |=0x200;
    UserSet.iCurrLW=0;
  }
  for( int i=0; i<4; i++) // проверка групп по длинам волн 
  {
    
    if (UserSet.iFixLvl[i]>1)
    {
      Err |=0x400;
      UserSet.iFixLvl[i]=0;
    }
    // проверка текущих уровней (в сохранении для востановления)
    if (UserSet.iLvlCurrLW[i]>1600)
    {
      Err |=0x800;
      UserSet.iLvlCurrLW[i]=0;
    }
    // проверка фиксированных уровней 1 закладки(в сохранении для востановления)
    if (UserSet.iLvlFixLW[i][0]>1600)
    {
      Err |=0x1000;
      UserSet.iLvlFixLW[i][0]=0;
    }
    // проверка фиксированных уровней 1 закладки(в сохранении для востановления)
    if (UserSet.iLvlFixLW[i][1]>1600)
    {
      Err |=0x2000;
      UserSet.iLvlFixLW[i][1]=0;
    }
  }
  
  return Err; 
}

DWORD FindErrCoeff (void)
{
  DWORD Err = 0;
  for (int i=0; i<=1600; ++i) 
  {
    if (CoeffLW.SetCoefLW[0][i]>4095) Err |= 0x4000 ; // плохое первое место
    if (CoeffLW.SetCoefLW[1][i]>4095) Err |= 0x8000 ; // плохое второе место
    if (CoeffLW.SetCoefLW[2][i]>4095) Err |= 0x10000 ; // плохое третье место
    if (CoeffLW.SetCoefLW[3][i]>4095) Err |= 0x20000 ; // плохое четвертое место
  }
  if(Err) // надо исправлять
  {
    if(Err && 0x4000)
    {
      for (int i=0; i<=1600; ++i) 
      {
        CoeffLW.SetCoefLW[0][i]=i; // плохое первое место
      }
    }
    if(Err && 0x8000)
    {
      for (int i=0; i<=1600; ++i) 
      {
        CoeffLW.SetCoefLW[1][i]=i; // плохое второе  место
      }
    }
    if(Err && 0x10000)
    {
      for (int i=0; i<=1600; ++i) 
      {
        CoeffLW.SetCoefLW[2][i]=i; // плохое третье место
      }
    }
    if(Err && 0x20000)
    {
      for (int i=0; i<=1600; ++i) 
      {
        CoeffLW.SetCoefLW[3][i]=i; // плохое четвертое место
      }
    }
  }
  return Err;
}




void SetLang( BYTE lang ) // функция не рабочая
{
  if ( lang < MAX_LANG )//LANG_NUM-1
  {
    lang = 0;
  }
  if (lang==0)lang=ConfigDevice.ID_Device;
    UserSet.CurrLang = lang;
}

BYTE GetID_Dev(void) // получение идентификатора для кого прибор
{
  return ConfigDevice.ID_Device;
}
BYTE SetID_Dev(BYTE Data) // установка идентификатора для кого прибор
{
  if (Data>1) Data=1;
  UserSet.CurrLang = Eng; // всегда при установке принадлежности устанавливаем Eng
  return ConfigDevice.ID_Device = Data;
}

BYTE GetLang(int Dir) // получение текущего языка, и переустановка по циклу
{
  BYTE Lang = UserSet.CurrLang;
  if (Dir) // переключаем языки
  {
    if (Dir>0)
    {
    Lang++;
    //Lang = GetID_Dev()*2 - Lang + 1;
    if (Lang > ((GetID_Dev())?(MAX_LANG):(1))) Lang = GetID_Dev(); //LANG_NUM-1
    }
    else
    {
      if ((Lang>1 && (GetID_Dev()))||(Lang>0 && !(GetID_Dev())))
        Lang--;
      else
        Lang=((GetID_Dev())?(MAX_LANG):(1));//LANG_NUM-1
    }
  }
  
    UserSet.CurrLang = Lang;
  
/*  Lang += Dir;
  if (Lang > MAX_LANG) 
  {
      if(ConfigDevice.ID_Device) UserSet.CurrLang = Eng;
      else UserSet.CurrLang = Rus;
  }
  else
  {
      if ( Lang < GetID_Dev() ) UserSet.CurrLang = MAX_LANG;
    else UserSet.CurrLang = Lang;
  }
  */
  return UserSet.CurrLang;
}
    

WORD GetNumDevice(void) // получение номера  прибора
{
  return ConfigDevice.NumDevice;
}

WORD SetNumDevice(WORD Num) // установка номера  прибора
{
  if (Num>9999) Num=9999;
  return ConfigDevice.NumDevice = Num;
}
// получение идентификатора "железа" , что и как сконфигурировано
void GetDeviceHW( char* name ) // from Computer
{
  char Str[100];
  
  sprintf(Str,"STM32H743 DEVICE\r");
  strcat( name,Str);
}


void GetNumVer (char* Str) // получение номера версии
{
  char out[5];
  strcpy( out, NumVer );//, strlen(Ides[device->ID_Device])
  out[4] = 0;
  int Num = atoi (out);
  //Num=Num/5;
  sprintf (Str, "6%c%c%c", (BYTE)(0x30+(Num/260)), (BYTE)(0x30+(Num/26)), (BYTE)(0x61+(Num%26)));
}



