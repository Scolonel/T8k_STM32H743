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
float BatControl[2048];// контроль разряда батареи 
} BAT_CNTR;




// структура коэффициентов затухания в тиках перебора по 0.05 дБ
typedef struct
{
    float SlopeChADC[2];   // наклон канала АЦП 8
    float OffsetLW[18];   // смещение длины волны 72
} TAB_SET;

// структура конфигурации прибора
typedef struct
{
  uint16_t NumDevice;  // (9999) № прибора,
  uint8_t ID_Device; // прибор 0-ТОПАЗ или 1-MOT_700
  uint8_t Ena_DB;  // разрешение альтернативного имени( 0- разрешено, 1-запрещено)
  char AltName[20]; //  альтернативное имя
} CFG_DEV;



typedef struct  // основные конфигурационные настройки прибора режимы работы
{
  uint8_t CurrLang;  // текущий язык Rus, Eng, Cze
  uint8_t ChnMod;  // режим отображения каналов 18,8R,8T 1
  uint8_t EnaCntFiber;   // авто счет волокна 1
  short int FiberID;    // номер волокна 2
  short int FileNumber;  // номер файла (?) может уже не надо? 2
  short int FileNumberView;   // номер файла (?) может уже не надо? 2
  char FiberName[10];  // имя волокна 10
  char CableID[10];    // идентификатор кабеля 10
  char Comments[20];   // комментарий по умолчанию 20
} CFG_USR;

// структура данных из памяти
#pragma pack(push,1)
typedef struct
{
    short int FileNumber;                           // номер файла (?) может уже не надо? 2
    char DateMem[12];                               // дата сохранения 10
    char TimeMem[12];                               // время сохранения 10
    char CableID[10];                               // идентификатор кабеля 10
    char FiberName[20];                             // имя волокна 10
    char Comments[20];                              // комментарий по умолчанию 20
    float CWDMDataMem[18]; // данные сохранений
} strFILESDATA;                                      // 

unsigned InvalidDevice(); //boolean actually
void InitDevice(unsigned Err);
DWORD CheckUserGonfig (void);  // Проверка пользовательских настроек 
DWORD FindFixErrBatS (void);
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



