/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HardSetDevice_H
#define __HardSetDevice_H


#ifdef __cplusplus
 extern "C" {
#endif 

#define K1 0x80
#define K2 0x20
#define K3 0x08
// source
#define SW_L  0x04
#define SW_H  0x40
#define ON_GEN(a) (a)?(CtrlExpand(1, 1)):(CtrlExpand(0, 1)) // управление сигналом OnG
#define LD_R(a) (a)?(CtrlExpand(0x100, 0x100)):(CtrlExpand(0, 0x100)) // управление красным глазом? Хитро потому как старший разряд младшего байта
#define CW_S(a) (a)?(CtrlExpand(0x02, 0x02)):(CtrlExpand(0, 0x02)) // управление постоянным излучением лазеров
// Ext mem 24lc512
#define EEPROM24LC512 0x50   // базовый адрес EEPROM24LC512

extern uint8_t TimeSCWP; // счетчик времени для переключения лазеров

   
uint32_t InvalidDevice(); //boolean actually
void InitDevice(unsigned Err);
   
void SetRange (uint8_t Rng); // установка сигналов переключения ключей для выбранного диапазона Range = 0 - самый грубый, Range = 3 - Самый чувствительный
// поиск и установка индекса длины волны калибровки по длине волны
int GetIndxLW (uint16_t in_LW);

void SWITCH_LW(void); // Функция установки сигналов для выбора лазерного места, 
// управление красным глазом
void ControlRE (uint8_t Mode); // 
// функция переключения лазеров по кругу с излучением информации о себе
void Run_SCWP(void);
// поиск/установка индекса длины волны источника для режимов, с признаком на один вход
uint8_t GetSetLW_SC (uint8_t Next, uint8_t Choice); // Next 1-Следующий,0- первый из присутствущих;
                                                     //Choice 1-с контролем по разрешению, 0-без контроля

// управление лазерным источником (сигналом CW и FW)
void ControlSC (uint8_t Mode); // 

uint32_t FindErrCoeff (void); // проверка таблицы коэффициентов
void FixErrCoeff (uint32_t Err); // исправление таблицы коэффициентов
// контроль установок пользователя измерений прибора
uint32_t FindErrUserMC (void);
void FixErrUserMC (uint32_t Err); //  фиксируем  (исправляем настройки)
    // функции контроля содержимого настройки P2 и измерителя длины
uint32_t   FindErrSettingPrm (void);
void  FixErrSettingPrm (uint32_t Err); //  фиксируем  (исправляем настройки)
// функции контроля спец коэффициентов
uint32_t FindErrSpec (void);
void FixErrSpec (uint32_t Err); //  фиксируем  (исправляем коэффициенты)

void SW_RS232 (int a);
#ifdef __cplusplus
}
#endif

#endif /* HardSetDevice_H */
   