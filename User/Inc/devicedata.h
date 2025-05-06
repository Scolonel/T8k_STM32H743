#ifndef __DEVICEDATA_H__
#define __DEVICEDATA_H__

#define WAVE_LENGTHS_NUM 5
#define LENGTH_LINE_NUM 7
#define WIDTH_PULSE_NUM 8
#define TIME_AVR_NUM 6
#define TIME_LIGHT_NUM 3
#define LSPLACENUM 4
#define CURRENT 0
#define NEXT 1
#define MODELSNUM 5
#define MODERENUM 3



// структура коэффициентов затухания в тиках перебора по 0.05 дБ
typedef struct
{
float BatControl[1024];// контроль разряда батареи 
} BAT_CNTR;




// структура коэффициентов затухания в тиках перебора по 0.05 дБ
typedef struct
{
uint16_t SetCoefLW[4][2048];// коэфф установки затухания по длинам волн
} TAB_SET;

// структура конфигурации прибора
typedef struct
{
  uint16_t NumDevice;  // (9999) № прибора,
  uint16_t PlaceLW[LSPLACENUM]; //  Конфигурация установленных аттенюаторов по длинам волн 
  uint8_t ID_Device; // прибор 0-ТОПАЗ или 1-MOT_700
  uint8_t Ena_DB;  // разрешение альтернативного имени( 0- разрешено, 1-запрещено)
  unsigned char AltName[20]; //  альтернативное имя
} CFG_DEV;



typedef struct  // основные конфигурационные настройки прибора режимы работы
{
  uint8_t CurrLang;  // текущий язык Rus, Eng, Cze
  char iCurrLW ; // индекс текущей длины волны 
  char iFixLvl[4] ; // индекс фиксированного уровня, указатель на рамку (для кждой длины волны)
  uint16_t iLvlCurrLW[4]; // текущий коэфф. для каждой длины волны
  uint16_t iLvlFixLW[4][2]; // сохраненые фиксированные коэфф. для каждой длины волны
  
} CFG_USR;


unsigned InvalidDevice(); //boolean actually
void InitDevice(unsigned Err);
DWORD CheckUserGonfig (void);  // Проверка пользовательских настроек 
DWORD FindErrCoeff (void); // проверка таблицы коэффициентов
void SetLang( BYTE lang );
BYTE GetID_Dev(void); // получение идентификатора для кого прибор
BYTE SetID_Dev(BYTE Data); // установка идентификатора для кого прибор
BYTE GetLang(int Dir); // получение текущего языка/ или переключение
WORD GetNumDevice(void); // получение номера  прибора
WORD SetNumDevice(WORD Num); // установка номера  прибора

// структура данных о батарейки, пишем по элементу каждую минуту от включения(0...1023)
extern BAT_CNTR LvlBatSav;
extern uint32_t CountBat; // счетчик записей по батареи

// структура коэффициентов измерителя мощности
extern TAB_SET CoeffLW;
// конфигурация прибора
extern CFG_DEV ConfigDevice;
// пользовательские настройки (текущие) для востановления после включения
extern CFG_USR UserSet;


#endif  /* __DEVICEDATA_H__ */



