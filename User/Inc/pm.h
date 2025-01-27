#ifndef __PM_H__
#define __PM_H__

// состояния работы АЦП - AD7782
#define FREEADC 0 // ацп можно запускать
#define READYDATA 1 // данные готовы
#define BUSYADC 2 // ацп занято


// Для "внутренненго" использования"
int GetPMData(void);              // Читает данные из АЦП измерителя
float GetCoeffSpctr(unsigned int Lambda);   // Получение спектрального коэффициента в зависимости от длины волны
                                            // диапазоны 800-900, 1210-1390, 1410-1650 
float GetCoefPointKlb(unsigned int Lambda);              // Возвращает дополнительный коэффициент для калиброванных длин волн


// "Внешние" функции
void SetSwitchPMMode(BYTE SwMode);          // Устанавливает режим переключения 
                                            // коэффициентов усиления измерителя мощности (ручной или автоматический)
void SetRange(unsigned char Rn);            // Установка ключей согласно выбранного диапазона (для ручного режима)
BYTE GetRange(void);                        // Возвращает текущий диапазон работы измерительного усилителя
float GetPower(unsigned int Lambda);       // Получаем мощность в мВт в зависимости от длины волны
unsigned int GetPMWavelenght(signed char DeltaLambda); //Возвращает, либо изменяет текущую длину волн
unsigned int SetPMWavelenght (unsigned int CurrPMWavelenght); // принудительная установка текущей длины волны
float Watt2dB(char *str, float mWatt,BYTE TypeRslt);    // Преобразует мВт в мкВт (нВт), дБ или дБм
int GetTypeRslt();                          // Возвращает текущее значение типа ед. изерения
int SetTypeRslt(BYTE type);                 // Устанавливает новое значение типа ед.измерения
int AcquireShZeroRng(void);                 // Измерение уровней смещения
int AcquireShZeroLowRng(void);                          // Измерение уровней смещения на нижнем диапазоне (переустановка нуля)
int SetDataADC (int Data); // установка данных АЦП в прерывании
void SetStateADC (BYTE State); // установка режима АЦП
BYTE GetStateADC (void); // получение режима АЦП
int AcquireCoefStykRange(BYTE Rng, float* PrevRng, float* CurrRng);         // Вычисляет стыковычный коэффициент текущего диапазона с предыдущим
float GetCoefSpctr(WORD Lambda, float RealPower);     // Возвращает спектральный коэффициент для текущей длины волны
float GetCoefSpctrKlb(WORD index, float RealPower);    // Возвращает спектральный коэффициент для калибровочных длин волн
// установка длинны волны если включен Wide и JDSU поддержка ?????????? Получение
void SetAutoLamdaJDSU (DWORD Freq);
// хотим получить длинну волны по частоте
unsigned int GetLambdaJDSUFreq (unsigned int Freq);


#endif  /* __PM_H__ */
