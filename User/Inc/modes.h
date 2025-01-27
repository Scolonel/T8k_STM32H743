#ifndef __MODES_H__
#define __MODES_H__

#include "rtc.h"
// Режимы работы прибора ( для соответсвия комманд для старой платы)
#define MODEMENU 0 // режим МЕНЮ
#define MODESETREFL 2 // режим установок рефлектомтра
#define MODEREFL 1 // режим рефлектометра
#define MODEMEMR 3 // режим работы с памятью рефлектограмм
#define MODETEST 4 // режим ТЕСТЕРА
#define MODESETUP 6 // режим установок пользователя
#define MODEMEASURE 7 // режим измерения рефлектометра (накопление)
#define MODETESTMEM 8 // режим ТЕСТЕРА view memory
#define MODEERREFL 85 // режим ощибки измерения рефлектометрического
#define MODEOTDRPRM 9 // режим установок параметров рефлектометрических измерений

#define MODEOTHER 255 // режимы другие


// наименование подрежимов рефлектометра при накоплении
#define NOTMODE 0 // только что зашли еще ничего не сделали
#define SETPOWER 1 // включаем питание, настраиваем установки накопления
#define INPUTOTDR 2 // проверка сигнала на входе
#define QUALITYBEGIN 3 // проверка качества разъема ( короткий импульс - длиная линия)
#define SEARCHENDLINE 4 // поиск конца линии
#define LASTCHECK 5 // проверка перед запуском накоплений
#define AVERAGING 6 // только накопление таймер запрещен
//#define ONESUMMONLY 7 // только 1 накопление 
#define CALKORLBEG 8 //  расчет ORL
//
#define POWMANUAL 1 // ручной режим измерителя
#define POWAUTO 2 // автоматический режим измерителя

#define PNTVERTICALSIZE 4 // число вертикальных масштабов

#define PRESS( a ) (KeyP & a)

WORD SetNumCellIzm (WORD Data);
float GetLastPower (void);

BYTE SetGetMonEna (BYTE Data); // запрос/установка признака выдачи событий по результату измерения
//
BYTE GetEnaMZ (void); // получение текущего значения разрешения корректировки метрвых зон
BYTE SetEnaMZ (BYTE Mode); //  установка  значения разрешения корректировки метрвых зон

//
BYTE SetModeDevice (BYTE Mode); // принудительная установка режима прибора
BYTE GetCurrentModeDevice (void); // получение текущего режима прибора
BYTE GetModeDevice (void);// получение режима прибора
BYTE GetModePowerMeter (void); // получение текущего режима измерителя
      // переключатель вывода в зависимости от комбинации
void SetPlaceNew(int Mode);


void ModeFuncTmp(void);
void SetMode( void(f)(void) );

void SetBELCORE (void);// установка порогов анализа в BELCORE

void ModeSetOTDRprm(void);// режим установок параметров рефлектометра прибора
void ModeWelcome(void); // режим заставки
void ModeMainMenu(void); // режим основного МЕНЮ
void ModeSetupOTDR(void); // режим установок рефлектометра
void ModeStartOTDR(void); // режим накопления рефлектометра
void ModeErrorOTDR(void); // режим отображения "Излучение на входе"
void ModeDrawOTDR(void); // режим отображения рефлектограммы
void ModeEventsOTDR(void); // режим отображения событий рефлектограммы
void ModeKeyBoardOTDR(void); // режим отображения клавиатуры редактора комментариев рефлектограммы
void ModeMemoryOTDR(void); // режим отображения сохраненных рефлектограмм и работа с ними
void ModeSelectOLT(void); // режим выбора типа тестера
void ModeMeasManualOLT(void); // режим работы тестера в ручном режиме
void ModeMeasAutoOLT(void); // режим работы тестера в автоматическом режиме
void ModeSourceOnly(void); // режим работы тестера только источник
void ModeSaveOLT(void); // режим сохранения результатов измерителя
void ModeKeyBoardOLT(void); // режим отображения клавиатуры редактора комментариев измерителя
void ModeKeyBoardPrefix(void); // режим отображения клавиатуры редактора Prefix File Name
void ModeSelectMEM(void); // режим выбора работы с памятью
void ModeClearMEM(void); // режим освобождения памяти измерителя
void ModeViewMemOLT(void); // режим просмотра памяти измерителя
void ModeSetting(void); // режим установок прибора
void Mode(void); // режим установок прибора
void ModeDateTimeSET(void); // режим установок времени
void ModeCalibrate(void); // режим установки начального смещения
void ModeFileMngDir(void); // режим файл менеджера директорий
void ModeFileMngFiles(void); // режим файл менеджера файлов

//void FrBig(unsigned char SW,  char Add); //Большая рамка
//void FrSmall(unsigned char SW,  char Add);// Маленькая рамка
// рамка произвольного размера и места
//void FrFree(unsigned char SW ,  int X, int Size);

BYTE ChangeFrSet (BYTE FrSet, BYTE MaxSet, BYTE MinSet, BYTE DirSet);// установка курсора в рамках заданных параметров
// Рисуем источник и красный глаз
BYTE DrawSourceVFL (BYTE* frameInx,  char* Str, DWORD KeyP, BYTE Lang, BYTE FrmMax, BYTE FrmMin);
BYTE Measuring ( void);
int SearchShiftBeg (int Size);// поиск мертвой зоны 
void DrawCalibrating (DWORD Data);// рисование окна калибровки
BYTE IndexSeek( int Data );// поиск индекса длины при проверке длины линии

// программа сканирования на поиск конца линии
DWORD Scan( DWORD* array, unsigned size, DWORD level );

void StartSettingBegShift (void); // старт измерения мертвых зон

void InitAutoPM (void); // начальные установки измерителя автомата

unsigned GetCntNumAvrg ( void);// получение текущего значения накоплений
unsigned GetFinAvrg ( void);// получение конечного значения накоплений (определяется временем усреднения)
unsigned SetCntNumAvrg ( unsigned Data);// установка текущего значения накоплений

float GetPosLine (unsigned Cursor); // получение длины от позиции курсора

// прорисовка клавиатуры с установленным курсором на символ
//void Draw_KeyBoard(unsigned char Cursor, char* Str_first, char* Str_edit, BYTE Shift ,BYTE Lang);
//void Knob(unsigned char X,unsigned char Y);
void SavePowerMeter(float Pow_mW);// функция сохранения в памяти ИЗмерений
void ReturnToTester (BYTE EnClr); // возврат в измеритель
void WriteMemPow(void); // запись в память непосредственно
void DrawLevelStr(char* Str); //???? вывод признака в чем выводим W/ dB/ dBm
void SetStringPM(char *Str, float mWatt) ;          // устанавливает строку данных измерения
void ReLoadCommOLT (void); // перезагружаем комментарии для измерителя

void KeybCntrl (void); // переключатель указателя в клавиатуре

WORD CheckLevelBattery (void); // контроль заряда баттареи
//123 void SetContrast (float Data, WORD UserContr); // установка авто контроаста с учетом пользовательских установок
void BadBattery(void); // плохая баттарейка
// переключимся в режим программирования индикатора (пока на паузу  и сигнал
void UploadFW_Nextion(void); //

BYTE CheckPONI (WORD TmpCellMem);  // проверка текущей ячейки и ее презапись если что не так
//void TestScreen (void); // 
// предрасчет сообытий по необработанным данным
void CalkPreEvents (DWORD* array, unsigned short PII);
// сохраение текущих шумов
void SaveNoise (DWORD Noise);

// сохранение рефлектограммы (по режиму от кнопок и по команде от UART)
int SaveNewOTDRTrace (BYTE Mode);

// получение вертикального масштаба
BYTE GetUserVerSize (void);
// получение пользовательского уровня центровки графика
BYTE GetUserTypeCentral (void);
void PrintEventTbl (void); // вывод по RS таблицы событий
// расчет склейки
unsigned short SpliceProg (unsigned short PII);
// функция установки уровня DAC на выходе P0.26
//123 void SetLvlDAC (unsigned int DAC); // не нужна!
// установка заголовка файла передачи необработанных данных
void SetHeadFileRaw (DWORD NAV);
// переход в режим установки параметров рефлектометра из меню или при включении если ТАБЛЕТКА
void SetMODESettingRef (void);
// расчет ORL по массиву необработанных данных от измерения в режиме 16км 40 нс
float MeasORL(int NumAvrgThis, int EnaReport);
// функция инициализации страницы по номеру в NEXTION
void CmdInitPage(int Num);

void SlowON (void); // медленное включение питания

void AlarmSignal(int type);

void CreatDelay (unsigned long a); // перенесли из LCD.h

//void TetrisGame (void);
//void ArcanoidGame (void);
//void KeyTestGame (void);

//extern RTCTime TimeSaveOTDR; // время сохраненной рефлектограммы
//extern RTCTime TimeSaveOLT; // время сохраненной рефлектограммы
extern char CommentsOTDR[20]; //комментарии рефлектометра
extern char IndexCommOTDR ;
extern BYTE RSOptYes; // признак приема команды по оптическому UART
extern BYTE RemoutCtrl; // признак дист управления

//переменные настройки измерения ORL 
extern float g_VolORL;
extern float g_UpGrdB ; // повышающие дБ
extern float g_kLog; // коэффициент логарифма
extern float g_OffSetdB; // смещение в формуле dB



#endif  /* __MODES_H__ */

