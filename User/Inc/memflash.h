#ifndef __MEMFLASH_H__
#define __MEMFLASH_H__



#define NUMPAGESFORREF 16
#define NUMWORDINPAGE 256
#define CHECKNUMTRACE Trace<=((CheckIDMEM)?(0x100+MaxMemOTDRExt):(MaxMemOTDR))

extern DWORD CheckMemFlash (void);
BYTE CheckMemID (void);
//extern CFG_DEV ConfigDevice;
void SaveConfigDevice (void);
void SaveUserConfig (void); // сохранение пользовательских настроек

// очистка рефлектограммы
void TraceERASE(unsigned int Trace);
void TraceWRITE(unsigned int Trace); // запись трассы
// чтение рефлектограммы
void TraceREAD(unsigned int Trace);
// читаем страницу сохраненной рефлектограммы
unsigned long FlashReadCommTrace (unsigned int Num, unsigned char *adress);
// чтение времени сохраненной рефлектограммы(поле минут)
unsigned long FlashReadTimeTrace (unsigned int Num);

unsigned short SetNumTrace (unsigned short Num); // установка номера трассы
unsigned short GetNumTrace (void); // получение номера трассы
void CheckTableMemOTDR (BYTE MemErr); // проверка таблицы сохранения рефлктограмм 0 - проверка 1 - очистка
unsigned short GetNumTraceSaved (unsigned short Index); // чтение значения таблицы рефлектограм по индексу
// КАРТИНКА ПАМЯТИ - рисуем картинку памяти рефлектограмм
void DrawMemoryRefl(unsigned short Trace, BYTE CurrLang, BYTE Param); // добавим переключатель вывода длины затухания и ORL
unsigned short SaveTrace(void); // сохранение рефлектограммы
unsigned short DeletingTrace (unsigned short Trace); // "Удаление" рефлектограммы, просто переносим 
unsigned short ReanimateTrace (unsigned short ReanT); // "реанимация" заданного числа трасс

unsigned short DeletingAllTrace (void); // "Удаление"всех рефлектограммы, просто обнуляем счетчик сохранения
// функции работы со склеивающими режимами
// почитаем текущую точку Splice рефлекограммы
unsigned short FlashReadSpliceData (unsigned int NumData);
// пишем как бы в 250 рефлектограмму (4001 pg...4016)
void SpliceWRITE(unsigned int Trace); 
// аварийная перезапись нулевой рефлекторграммы
void AlarmReWriteTrace (void);

void SDMMC_SDCard_DIR(void); // прочитаем дирректроии
void SDMMC_SDCard_FILES(void); // прочитаем files
void SaveFileSD(int Mod); // запись 0.sor на карточку в главную дирректорию
uint32_t StartInitSDcard (void); 



#endif  /* __MemFlash_H__ */

