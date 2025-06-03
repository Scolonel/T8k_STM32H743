#ifndef __ATT_MODE_H__
#define __ATT_MODE_H__

#define MAXWIDESMBL 17 // максимальное число широких символов в строке 
#define MAXSMALLSMBL 22
#define WAITWELCOME 100 // число циклов ожидания заставки
#define PLUS 1
#define MINUS 0
#define Pic_Tx 67 // X - смещение картинки при выводе установки времени
#define Pic_Ty 5 // Y - смещение картинки при выводе установки времени

//char GetModeDev (void); // возвращает текущий режим
void CmdInitPage(int Num);

void ModeFuncTmp(void);
void SetMode( void(f)(void) );


void ModeWelcome(void); // стартовая заставка
void ModeMain(void); // прорисовка и управление основного меню
// переключимся в режим программирования индикатора (пока на паузу  и сигнал
void UploadFW_Nextion(void); // обновление индикатора NEXTION
void BadBattery(void); // плохая батарейка

//void SF_MainT4000(void); // прорисовка и управление основного меню
//void SF_MainT5xxx(void); // прорисовка и управление основного меню
//void SF_ResultTable(void); // прорисовка и управление табличного отображения результатов
//void SF_ResultGraph(void); // прорисовка и управление графического отображения результатов
//void SF_OnOffMode (void); // режим выкючения прибора (спящий режим???)
//void DeviceAllOff (void);// выключение прибора (одна функция) для выключения
//void SF_MemoryMain (void); // МЕНЮ работы с памятью
//void SF_MainSetting (void); // меню выбора УСТАНОВОК (прибор, дата/время, файл)
//void SF_SettingDev (void); // меню УСТАНОВОК прибора
//void SF_SettingDaTime (void); // меню УСТАНОВОК даты и времени
//void SF_SettingFile (void); // меню настройки фала ()
//void SF_ErrMsg (void); // Рисовалка отображения ошибок режимов 
////void vSetCHKey(char* Dims);
//
//// прорисовка клавиатуры с установленным курсором на символ
//void KeybCntrl (void); // переключатель указателя в клавиатуре
//void Draw_KeyBoard(unsigned char Cursor, char* Str_first, char* Str_edit, unsigned char Shift ,unsigned char Lang);
//void Knob(unsigned char X,unsigned char Y);
//unsigned char SearchEndStr(char* EditStr, unsigned char FindEnd); // ищем место курсора в редактируемой строке
//void FrBig(unsigned char SW,  char Add); //Большая рамка
//void FrLamda9(unsigned char SW);
//void FrLamda18(unsigned char SW);
//void FrSmall(unsigned char SW,  char Add);// Маленькая рамка
//void FrFree(unsigned char SW ,  int X, int Size);// рамка произвольного размера и места
//void FrdB(unsigned char SW ,  int X, int Size); //рамка для подсветки верхнего рабочего значения затухания
//void FrVal(unsigned char SW ,  int X, int Size);
//void FrFreePx(int X, int Y ,  char* PrntStr);
//unsigned char ChangeFrSet (unsigned char FrSet, unsigned char MaxSet, unsigned char MinSet, unsigned char DirSet);// установка курсора в рамках заданных параметров

#endif  /* __ATT_MODE_H__ */

