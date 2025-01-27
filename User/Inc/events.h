#ifndef __EVENTS_H
#define __EVENTS_H

#define SizeTableEvnt 20

// Структура событий в KeyEvents
#pragma pack(2)
typedef struct   // Size 32 Byte
{
  unsigned short EN;      // EN - Event Number
  unsigned int EPT;       // EPT Event Propagation Time 0
  short int ACI ;    // ACI Attenuation Coefficient Lead-in-Fiber 0
  short int EL ;     // EL Events Loss 0
  long ER ;               // ER Events Reflectance -45000 = -45.0 dB 
  char EC[6] ;            // EC Event Code 0/1/2-A/M/E/F/O-9999  
  char LMT[2];            // 2P/LS 2-point LSA 
  char COMM_EVN[10] ;     // коммментарии к событию
} EVENTS;  // параметры рефлектограммы

// Структура окончания (курсоры , LOSS и ORL) событий в KeyEvents

#pragma pack(2)
typedef struct   // Size 22 Byte
{
  long EEL;     // EEL - End-to_End Loss
  long ELMP[2];          // End-to_End Marker Positions
  unsigned short ORL ;   // optical Return Loss
  long RLMP[2];          // optical Return Loss Marker Positions
} ENDBLKEVENTS;  // конец блока событий

// структура поиска событий внутренняя, из нее сделаем таблицу для белкора
// массив крупнее в 3 раза 
#pragma pack(2)
typedef struct   // Size 12 Byte
{
  unsigned short EN;      // EN - Event Number (2)
  unsigned int EPT;       // положение события в точках - потом его пересчитать для белкора (4)
  unsigned short Volume ;    // значение события по LOGDATA (2)
  unsigned short PointInImp ;     // число точек в импульсе PII (2)
  char Type_Evnt ;               // тип события (1)
  char CompBell ;            // признак совместимости с белкором значит это событие можно перенести в белкор (1)
} MY_EVNT;  // Внутренний массив событий

// структура событий рефлектограммы ( текущего измерения)
extern EVENTS EvenTrace[SizeTableEvnt];
// структура окончания блока событий рефлектограммы (текущего измерения)
extern ENDBLKEVENTS EndEvenBlk;
// структура событий рефлектограммы ( текущего измерения) внутреняя структура
extern MY_EVNT MyEvenTrace[3*SizeTableEvnt];



DWORD CheckReflParam (void);
unsigned short GetNumEvents (void); // возвращает число событий (
unsigned short GetSetEnaEvents (unsigned Dir); // устанавливает возвращает признак разрешения событий
void InitEventsTable (void); // инициализация структур событий
unsigned short CalkEventsKeys (unsigned short* array, unsigned short PII, BYTE Type); // вычисление событий для текущей рефлектограммы
// Расчет разницы по бегущему шаблону (на растоянии длительности импульса на интервале как в корреляции,
// считаем средние по краям и если превышает порог то выводим значение разницы)
unsigned short CalkDownJump (unsigned short* array, unsigned short BeginPoint, unsigned short Size);
// расчет среднего отклонения заданного размера с данной точки в массиве
float CalkSCO (unsigned short* array, unsigned short BeginPoint, unsigned short Size);// , float* Base
// расчет тренда по облаку точек заданного размера с данной точки в массиве
float CalkTrend (unsigned short* array, unsigned short BeginPoint, unsigned short Size, unsigned short TrLvlDwn, float b0); //, float* Base
// расчет кореляции  по облаку точек заданного размера с данной точки в массиве
float CalkCorelation32 (DWORD* array, unsigned short BeginPoint, unsigned short Size, BYTE Type);// , float* Base
// расчет значений EPT для событий от положения курсора
long CalkEPT (unsigned long Position);

unsigned short GetLvlTrend (void); // расчет начального уровня с которго считаем

// поиск конца линии , с конца рефлектограммы
unsigned short CalkEndRefl (unsigned short* array, unsigned short PII,  unsigned short TrLvlDwn );

// Вычисление отражения в зависимости от всплеска сигнала и длительности импульса
// в формате х1000 - то есть тысячных децибела
// исходные данные BC - 80 дБ(800),значение импульса в дБ*1000, длительность импульса
long CalkEventER (long LevelER);
unsigned short CalkMINMAX (unsigned short* array, int size, BYTE type);

#endif
