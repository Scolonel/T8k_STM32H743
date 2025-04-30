//#define MAXWIDESMBL 17 // максимальное число широких символов в строке 
//#define MAXSMALLSMBL 22
//#define WAITWELCOME 100 // число циклов ожидания заставки
//#define PLUS 1
//#define MINUS 0
//#define Pic_Tx 67 // X - смещение картинки при выводе установки времени
//#define Pic_Ty 5 // Y - смещение картинки при выводе установки времени


#include "main.h"

volatile BYTE g_NeedScr=1; // перерисовки экрана ! Аккуратно! надо проверить 
volatile BYTE g_FirstScr=1; // первый вход в экран, нужна полная перерисовка (заполнение), далее изменяем только нужные поля, для NEXTION
volatile BYTE g_NeedChkAnsvNEX=0; // признак получения строки из редактора.и ее проверка
 uint16_t Set_MAX_DB; // предельное значение индикатора дБ в десятых долях (в SHAG долях) для разных диапазонов



static volatile unsigned char CurIndLambda = 0; // указатель на длину волны отображения
static volatile unsigned char FrSetResGrph = 0; // указатель на курсор в граф режиме
static volatile unsigned char FrSetResTabl = 1; // указатель на курсор в табл режиме
static volatile unsigned char FrSetOnOff = 1; // указатель на курсор в режиме выключения
static volatile unsigned char FrSetClear = 1; // указатель на курсор в режиме очистки памяти
static volatile char CntFolder=0; // счетчик папок при удалении

static volatile unsigned char SubModeMem = 0; // подрежим индикации памяти
static volatile  unsigned char FrCurrMem = 0; // курсор в режиме просмотра в графике
//static volatile unsigned char ChannelsMode = UserConfig.ChnMod; // режим отображения каналов
//static char Stroka[20];
//static unsigned char IndexComm=0;
unsigned char SetErrMsg = 0; // указатель какое сообщение об ошибке печатаем
unsigned CntWelcome = 0; // счетчик ожидания заставки

//static unsigned char Ptr; // указатель с какой строкой работаем (чтобы перекопировать)
static void (*ModeFunc)(void);
static void (*ModeFuncPrev)(void) = 0;

static unsigned char FrdBSet=1; //признак включения рамки у рабочего значения затухания (сверху)
static unsigned char FrFreeSet=0; //признак включения рамки основного курсора левый столбец
//static unsigned char FrValSet = 1; //признак включения рамки основного курсора правый столбец
unsigned char ScrRedraw = 1; //признак перерисовки экрана

//static char* globalStringToCopy = 0;

//static char AnlzMode = MOD_ANLZ_TAB;

//static unsigned char TabFiles[16];
//для батареи
//void FrBat(unsigned char SW ,  int X, int Size); //???
//void FrBatNew(int Y ,  int X, int Size); //???
float BatProc; //перем. для уровня заряда в %


//char GetModeDev (void) // возвращает текущий режим
//{
//  return CurrMode;
//}
// функция инициализации страницы по номеру в NEXTION
void CmdInitPage(int Num)
{
  char str[44];
  sprintf(str, "page %dяяя",Num); // < событиe >
  NEX_Transmit((void*)str);    //
  
  //NEX_Transmit((void*)CmdNextion[Num]);
  g_FirstScr=1;
  //CreatDelay(20000);// 177 как в 173
  HAL_Delay(100);
}


void ModeFuncTmp(void)
{
  ModeFunc();
    if(KeyP)
    KeyP = 0;

}

void SetMode( void(f)(void) )
{
    ModeFunc = f;
}
//void SetMode( void(f)(void) )
//{
//  ModeFunc = f;
//}
void ModeMain(void)// режим основной
{
  static volatile unsigned char FrFreeInd = 0; //указатель на основной курсор
  
  char Str[32];
  char StrN[32];
  Set_MAX_DB = (ConfigDevice.PlaceLW[UserSet.iCurrLW]>1300)?(MAX_DB):(MAX_DB/2);
  
  //if (PRESS(BTN_OK))
  if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
  {
    myBeep(10);
    switch(DigitSet)
    {
    case 1:
      DigitSet = 20;
      break;
    case 20:
      DigitSet = 100;
      break;
    case 100:
      DigitSet = 200;
      break;
    default:
      DigitSet = 1;
      break;
    }
    g_NeedScr = 1;
    
  }
  // кнопка вверх
  if ((PRESS(BTN_UP))&&(getStateButtons(BTN_UP)==SHORT_PRESSED))
  {
    myBeep(25);
    FrFreeInd=(int)(FrFreeInd+3)%4;
    g_NeedScr = 1;
    
  }
  // кнопка вниз
  if ((PRESS(BTN_DOWN))&&(getStateButtons(BTN_DOWN)==SHORT_PRESSED))
  {
    myBeep(25);
    FrFreeInd=(int)(FrFreeInd+1)%4;
    g_NeedScr = 1;
    
  }
  if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==INF_PRESSED)) // жмем долго
  {
    if (FrFreeInd==0)
    {
      myBeep(3);
      DigitSet = 1;
      if(UserSet.iLvlCurrLW[UserSet.iCurrLW]>DigitSet) UserSet.iLvlCurrLW[UserSet.iCurrLW] -= DigitSet;
      else UserSet.iLvlCurrLW[UserSet.iCurrLW] = 0;
      g_NeedScr = 1;
    }
    if (FrFreeInd==1 || FrFreeInd==2)
    {
      myBeep(3);
      DigitSet = 1;
      if(UserSet.iLvlFixLW[UserSet.iCurrLW][FrFreeInd-1]>DigitSet) UserSet.iLvlFixLW[UserSet.iCurrLW][FrFreeInd-1] -= DigitSet;
      else UserSet.iLvlFixLW[UserSet.iCurrLW][FrFreeInd-1] = 0;
      g_NeedScr = 1;
    }   
  }
  // кнопка влево , учитываем где стоит курсор
  if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))
  {
    myBeep(25);
    if (FrFreeInd==0)
    { 
      if(UserSet.iLvlCurrLW[UserSet.iCurrLW]>DigitSet) UserSet.iLvlCurrLW[UserSet.iCurrLW] -= DigitSet;
      else UserSet.iLvlCurrLW[UserSet.iCurrLW] = 0;
    }    
    if (FrFreeInd==1 || FrFreeInd==2)
    {
      if(UserSet.iLvlFixLW[UserSet.iCurrLW][FrFreeInd-1]>DigitSet) UserSet.iLvlFixLW[UserSet.iCurrLW][FrFreeInd-1] -= DigitSet;
      else UserSet.iLvlFixLW[UserSet.iCurrLW][FrFreeInd-1] = 0;
    }   
    //UserSet.iLvlCurrLW[UserSet.iCurrLW]/SHAG 
    if (FrFreeInd==3) // переключение длины волны
    { 
      if(UserSet.iCurrLW>0) UserSet.iCurrLW--; // здесь переключаем длины волн (
      else UserSet.iCurrLW = 3;
      if(ConfigDevice.PlaceLW[UserSet.iCurrLW]==0)
      {
        if(UserSet.iCurrLW>0) UserSet.iCurrLW--;   // здесь переключаем длины волн
        else UserSet.iCurrLW = 3;
      }
      if(ConfigDevice.PlaceLW[UserSet.iCurrLW]==0)
      {
        if(UserSet.iCurrLW>0) UserSet.iCurrLW--;   // здесь переключаем длины волн
        else UserSet.iCurrLW = 3;
      }
      if(ConfigDevice.PlaceLW[UserSet.iCurrLW]==0)
      {
        if(UserSet.iCurrLW>0) UserSet.iCurrLW--;   // здесь переключаем длины волн
        else UserSet.iCurrLW = 3;
      }
    }
    g_NeedScr = 1;
    
  }
  if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==INF_PRESSED)) // жмем долго
  {
    if (FrFreeInd==0)
    { 
      myBeep(3);
      DigitSet = 1;
      if(UserSet.iLvlCurrLW[UserSet.iCurrLW]<=Set_MAX_DB-DigitSet) UserSet.iLvlCurrLW[UserSet.iCurrLW] += DigitSet;
      else UserSet.iLvlCurrLW[UserSet.iCurrLW] = Set_MAX_DB;
      g_NeedScr = 1;
      
    }    
    if (FrFreeInd==1 || FrFreeInd==2)
    {
      myBeep(3);
      DigitSet = 1;
      //UserSet.iLvlFixLW[UserSet.iCurrLW][FrFreeInd-1]
      if(UserSet.iLvlFixLW[UserSet.iCurrLW][FrFreeInd-1]<=Set_MAX_DB-DigitSet) UserSet.iLvlFixLW[UserSet.iCurrLW][FrFreeInd-1] += DigitSet;
      else UserSet.iLvlFixLW[UserSet.iCurrLW][FrFreeInd-1] = Set_MAX_DB;
      g_NeedScr = 1;
    }   
    
  }
  // кнопка враво , учитываем где стоит курсор
  if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))
  {
    myBeep(25);
    if (FrFreeInd==0)
    { 
      if(UserSet.iLvlCurrLW[UserSet.iCurrLW]<=Set_MAX_DB-DigitSet) UserSet.iLvlCurrLW[UserSet.iCurrLW] += DigitSet;
      else UserSet.iLvlCurrLW[UserSet.iCurrLW] = Set_MAX_DB;
    }    
    if (FrFreeInd==1 || FrFreeInd==2)
    {
      //UserSet.iLvlFixLW[UserSet.iCurrLW][FrFreeInd-1]
      if(UserSet.iLvlFixLW[UserSet.iCurrLW][FrFreeInd-1]<=Set_MAX_DB-DigitSet) UserSet.iLvlFixLW[UserSet.iCurrLW][FrFreeInd-1] += DigitSet;
      else UserSet.iLvlFixLW[UserSet.iCurrLW][FrFreeInd-1] = Set_MAX_DB;
    }   
    //UserSet.iLvlCurrLW[UserSet.iCurrLW]/SHAG 
    if (FrFreeInd==3) // переключение длины волны
    { 
      if(UserSet.iCurrLW<3) UserSet.iCurrLW++; // здесь переключаем длины волн (
      else UserSet.iCurrLW = 0;
      if(ConfigDevice.PlaceLW[UserSet.iCurrLW]==0)
      {
        if(UserSet.iCurrLW<3) UserSet.iCurrLW++;   // здесь переключаем длины волн
        else UserSet.iCurrLW = 0;
      }
      if(ConfigDevice.PlaceLW[UserSet.iCurrLW]==0)
      {
        if(UserSet.iCurrLW<3) UserSet.iCurrLW++;   // здесь переключаем длины волн
        else UserSet.iCurrLW = 0;
      }
      if(ConfigDevice.PlaceLW[UserSet.iCurrLW]==0)
      {
        if(UserSet.iCurrLW<3) UserSet.iCurrLW++;   // здесь переключаем длины волн
        else UserSet.iCurrLW = 0;
      }
    }
    g_NeedScr = 1;
    
  }
  // кнопка МЕНЮ , учитываем где стоит курсор
  if ((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))
  {
    if (FrFreeInd==1 || FrFreeInd==2)
    {
      myBeep(25);
      UserSet.iLvlCurrLW[UserSet.iCurrLW] = UserSet.iLvlFixLW[UserSet.iCurrLW][FrFreeInd-1];
      if(FrFreeInd==1) FrFreeInd=2;
      else FrFreeInd=1;
    }
    g_NeedScr = 1;
  }
  
  // перебор длин волн по списку в одну сторону по кнопке 'S'
  //длины волн
  if (rawPressKeyS) // key S 
  {  
    myBeep(35);
    
    UserSet.iLvlCurrLW[UserSet.iCurrLW] = 0;
    // требование А.К. от 30.04.2025
    FrFreeInd=0;
    
    //    if(UserSet.iCurrLW<3) UserSet.iCurrLW++; // здесь переключаем длины волн (
    //    else UserSet.iCurrLW = 0;
    //    if(ConfigDevice.PlaceLW[UserSet.iCurrLW]==0)
    //    {
    //      if(UserSet.iCurrLW<3) UserSet.iCurrLW++;   // здесь переключаем длины волн
    //      else UserSet.iCurrLW = 0;
    //    }
    //    if(ConfigDevice.PlaceLW[UserSet.iCurrLW]==0)
    //    {
    //      if(UserSet.iCurrLW<3) UserSet.iCurrLW++;   // здесь переключаем длины волн
    //      else UserSet.iCurrLW = 0;
    //    }
    //    if(ConfigDevice.PlaceLW[UserSet.iCurrLW]==0)
    //    {
    //      if(UserSet.iCurrLW<3) UserSet.iCurrLW++;   // здесь переключаем длины волн
    //      else UserSet.iCurrLW = 0;
    //    }
    // NeedSaveParam |=0x02;// Memory UserSet 
    
    rawPressKeyS = 0;  
    g_NeedScr = 1;
  }
  
  if(g_FirstScr)
  {
    
    // инициализация не изменяемых величин
    // наименование прибора
    
    sprintf(StrN,"%.2f dB",UserSet.iCurrLW/SHAG);
    sprintf (Str,"t0.txt=\"%s\"яяя",StrN); // 
    NEX_Transmit((void*)(void*)Str);//
    
    sprintf (Str,"t1.txt=\"%s\"яяя",MsgMass[31][UserSet.CurrLang]); //Memory 1
    NEX_Transmit((void*)Str);//
    // 
    sprintf (Str,"t2.txt=\"%s\"яяя",MsgMass[32][UserSet.CurrLang]); // Memory 2
    NEX_Transmit((void*)Str);//
    // 
    sprintf (Str,"t3.txt=\"%s\"яяя",MsgMass[30][UserSet.CurrLang]); // длина волны
    NEX_Transmit((void*)Str);//
    // 
    g_FirstScr = 0;
    g_NeedScr = 1;
  }
  if(g_NeedScr) // перерисовка индикатора при изменениях и инициализации
  {
    // раскрашивание поля выбора 
    // закрасим бэкграунды  и установим требуемый
    sprintf(Str, "t0.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);//
    sprintf(Str, "t1.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);// 
    sprintf(Str, "t2.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);// 
    sprintf(Str, "t3.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);// 
    // sprintf(Str, "t%d.bco=GREENяяя", (FrFreeInd)?(FrFreeInd+3):(0)); // зеленый
    sprintf(Str, "t%d.bco=GREENяяя", (FrFreeInd)); // зеленый
    NEX_Transmit((void*)Str);// 
    // код подсветки требуемой строки если есть есть маркер строки
    
    // значение памяти 1
    sprintf (Str,"t4.txt=\"%.2f dB\"яяя",UserSet.iLvlFixLW[UserSet.iCurrLW][0]/SHAG); // 
    NEX_Transmit((void*)Str);//
    // значение памяти 2
    sprintf (Str,"t5.txt=\"%.2f dB\"яяя",UserSet.iLvlFixLW[UserSet.iCurrLW][1]/SHAG); // 
    NEX_Transmit((void*)Str);//
    // длина волны рабочая
    sprintf (Str,"t6.txt=\"%d nm\"яяя",ConfigDevice.PlaceLW[UserSet.iCurrLW]); // 
    NEX_Transmit((void*)Str);//
    // индикатор длины волны 
    //if(ConfigDevice.PlaceLW[UserSet.iCurrLW]>1300)
    sprintf (Str,"t7.txt=\"%s\"яяя",(ConfigDevice.PlaceLW[UserSet.iCurrLW]>1300)?("SM"):("MM")); // 
    NEX_Transmit((void*)Str);//
    // рисуем значение ШАГА изменений
    
    if(ModeWork) // настройка
    {
      sprintf(Str,"t8.txt=\"!SET!\"яяя");
      NEX_Transmit((void*)Str);//
      sprintf (Str,"t0.txt=\"%04d\"яяя",CurrLevelDAC); //  уровень основной
      NEX_Transmit((void*)Str);//
    }
    else
    {
      if(DigitSet<SHAG)
        sprintf(Str,"t8.txt=\"%2.2f\"яяя",DigitSet/SHAG);
      else
        sprintf(Str,"t8.txt=\"%2.1f\"яяя",DigitSet/SHAG);
      NEX_Transmit((void*)Str);//
      sprintf (Str,"t0.txt=\"%.2f dB\"яяя",UserSet.iLvlCurrLW[UserSet.iCurrLW]/SHAG); //  уровень основной
      NEX_Transmit((void*)Str);//
      CurrLevelDAC = CoeffLW.SetCoefLW[UserSet.iCurrLW][UserSet.iLvlCurrLW[UserSet.iCurrLW]];
    }
    NEX_Transmit((void*)Str);//
    
    NeedSaveParam |=0x02;// Memory UserSet 
    
    g_NeedScr =  0;
  }
  
}

void ModeWelcome(void)// режим заставки
{
  char Str[32];
  char StrN[32];

  char St[5];
  CntWelcome++;
  static DWORD SecOld=100;
  //static DWORD BatOld=150;
  current_time = RTCGetTime();
  CurrLang=GetLang(CURRENT);
  //CheckLevelBattery ();
  // Выделим блок для выключения если LCD не пользуем
  // дублирующий блок для NEXTION
  if(g_FirstScr)
  {
    
    // инициализация не изменяемых величин
    // наименование прибора
    int  Mdl=0; // определяем модель по конфигурации длинн волн
    for (int y=0;y<4;++y)
    {
      if((ConfigDevice.PlaceLW[y]>0)&&(ConfigDevice.PlaceLW[y]<1310))
        Mdl|=1;
      if(ConfigDevice.PlaceLW[y]>1300)
        Mdl|=2;
    }
    
    sprintf(StrN,"%s-%d",DeviceIDN[ConfigDevice.ID_Device],Mdl);
    sprintf (Str,"t0.txt=\"%s\"яяя",StrN); // 
    NEX_Transmit((void*)(void*)Str);//
    // производитель
    if (GetID_Dev())
    {
      sprintf (Str,"t1.txt=\"%s\"яяя",MsgMass[12][UserSet.CurrLang+2]); // Чехи
    }
    else
    {
      sprintf (Str,"t1.txt=\"%s\"яяя",MsgMass[12][UserSet.CurrLang]); // Мы
    }
    NEX_Transmit((void*)Str);//
    // Date
    sprintf (Str,"t2.txt=\"%s\"яяя",MsgMass[13][UserSet.CurrLang]); // Date
    NEX_Transmit((void*)Str);//
    // Time
    sprintf (Str,"t4.txt=\"%s\"яяя",MsgMass[15][UserSet.CurrLang]); // time
    NEX_Transmit((void*)Str);//
    // Battery
    sprintf (Str,"t6.txt=\"%s\"яяя",MsgMass[16][UserSet.CurrLang]); // Battery
    NEX_Transmit((void*)Str);//
    // Version
    // версия ПО
    sprintf (St, "v3.%02d%c", NUM_VER/26, (0x61+(NUM_VER%26))); // версия должна быть не ниже 2.01 (28.06.2022)
    sprintf (Str,"t8.txt=\"%s(%X)\"яяя",St,CheckErrMEM); // Version
    NEX_Transmit((void*)Str);//
    // Number
    sprintf (Str,"t9.txt=\"№%d\"яяя",GetNumDevice()); // Number
    NEX_Transmit((void*)Str);//
    
  }
  // проверка изменения величин для изменений (время и аккумулятор)
  // time
  if(SecOld != current_time.RTC_Sec)
  {
    //отображение времени
    SecOld = current_time.RTC_Sec ; 
    sprintf(Str,"t5.txt=\"%02d:%02d:%02d\"яяя",current_time.RTC_Hour,current_time.RTC_Min,current_time.RTC_Sec);
    NEX_Transmit((void*)Str);//
    // отображение даты
    sprintf (Str,"t3.txt=\"%02d.%02d.%02d\"яяя",current_time.RTC_Mday,current_time.RTC_Mon,current_time.RTC_Year);//,MsgMass[14][CurrLang] убран год 25.12.2013
    NEX_Transmit((void*)Str);//
    // уровень и тип заряда
    if (GETEXTPWR == 0)
    {
      sprintf(Str,"t7.txt=\"%s\"яяя",MsgMass[11][CurrLang]);// внешнее питание
    }
    else
      // от батареек LvlBatInd 
    {
      sprintf(Str,"t7.txt=\"%d%%\"яяя", (int)(LvlBatInd*12.5));
    }
    NEX_Transmit((void*)Str);// 
  }
  g_FirstScr=0; // выключаем признак первого прохода
  
  if ((HAL_GetTick() - TimeBegin) > 4000) //4s
    //if (CntWelcome > WAITWELCOME)
  {
    SetMode(ModeMain);
    CmdInitPage(3);// посылка команды переключения окна на MainMenu и установка признака первого входа
    
    myBeep(10);
    //ModeDevice = MODEMENU;
    //SetHeadFileRaw (0); // пишем заголовок файла выдачи необработаных данных
    
    //ADCData++;
    //ClearScreen();
  }   
  
}

// переключимся в режим программирования индикатора (пока на паузу  и сигнал
void UploadFW_Nextion(void) // обновление индикатора NEXTION
{ 
  char Str[32];
  // здесь порисуем для нового индиктора
  if (g_FirstScr)
  {
    // здесь заполняем данными поля нового индикатора
    // не требущие изменения при первичной инициализации
    sprintf(Str, "t0.txt=\"%s\"яяя", MsgMass[57][CurrLang]);
    NEX_Transmit((void*)Str);    // 
    
    sprintf(Str, "t1.txt=\"%s\"яяя", MsgMass[58][CurrLang]);
    NEX_Transmit((void*)Str);    // 
    
    //CreatDelay(500000);// чуть потупим
    HAL_Delay(50);
    ProgFW_LCD = 1; // переключим режим работы UART только здесь когда все заслали
    
    g_FirstScr = 0;
    g_NeedScr = 0;
  }
  
  if(ProgFW_LCD==2) // если сбросисли признак программирования то переключимся  в норм режим
  {
    //123 enable_timer( 0 );
    GetSysTick(1); // получение тиков 10 мС. 0 - получение счетчика от предыдущего сброса 1- сброс
    ProgFW_LCD=0;  
    // посылка команды переключения окна на MainMenu (возврат)  
    // вызовем новое окно!
    //CreatDelay(5000000);
    //SetModeDevice (MODEMENU); // принудительная установка режима прибора
    // начало работы..
    CmdInitPage(0);
    SetMode(ModeWelcome);
    //CmdInitPage(0);
    HAL_Delay(500);// индикатор после сброса, время не понятно!
    TimeBegin = HAL_GetTick();
    myBeep(125);
  }
  
}


//int SF_BatLvl(void) //если всё ок, то продолжаем работу, если нет, то выключаемся. Ф-я возвращает 0 или 1, поэтому int
//{
// // char lvl[5]; //сюда вписываем % заряда из BatProc
//
//  char str1[20];
//  char str2[20];
//  char str3[10];
//  char str4[20];  
//  char str5[10];
//  float BatProc; //перем. для уровня заряда в %
//  
//  ClearScreen();
//  udelay(2000000); //т.к. через секунду уже можно адекватно видеть заряд батареи
//  BatProc = CheckBattery(0); //сняли значение, дальше решаем
//  sprintf(str1,"%s", MsgMass[50][UserSet.CurrLang]);
//
//  if (BatProc>5.)
//  {
//    if (BatProc<40.)
//    {
//    sprintf(str5, "%s", MsgMass[53][UserSet.CurrLang]);
//    sprintf(str2,"%s", MsgMass[51][UserSet.CurrLang]);
//    sprintf(str3,"%s", MsgMass[52][UserSet.CurrLang]);    
//    putString(((MAXSMALLSMBL-strlen(str5))/2)*5,18,str5,1,1);
//    putString(((MAXSMALLSMBL-strlen(str2))/2)*5,30,str2,1,0);
//    putString(((MAXSMALLSMBL-strlen(str3))/2)*5,40,str3,1,0);
//    }
//    else
//    {
//    if (BatProc<80.)
//    {
//    sprintf(str5, "%s", MsgMass[54][UserSet.CurrLang]);
//    putString(((MAXSMALLSMBL-strlen(str5))/2)*5,30,str5,1,1);
//    }
//    else
//    {
//    sprintf(str5, "%s", MsgMass[55][UserSet.CurrLang]);
//    putString(((MAXSMALLSMBL-strlen(str5))/2)*5,30,str5,1,1);
//    }
//    }
//  //  sprintf(lvl, "%.0f %s", BatProc, "%");
//
//    putString(((MAXSMALLSMBL-strlen(str1))/2)*5,10,str1,1,0);
//    PaintLCD();
//    udelay(3000000);
//    return 1;
//  }
//  else
//  {
//    sprintf(str5, "%s", MsgMass[48][UserSet.CurrLang]);
//    sprintf(str2,"%s", MsgMass[51][UserSet.CurrLang]);
//    sprintf(str3,"%s", MsgMass[52][UserSet.CurrLang]);    
//    sprintf(str4,"%s", MsgMass[19][UserSet.CurrLang]);
//    putString(((MAXSMALLSMBL-strlen(str1))/2)*5,3,str1,1,0);
//    putString(((MAXSMALLSMBL-strlen(str5))/2)*5,13,str5,1,1);
//    putString(((MAXSMALLSMBL-strlen(str2))/2)*5,25,str2,1,0);
//    putString(((MAXSMALLSMBL-strlen(str3))/2)*5,35,str3,1,0);
//    putString(((MAXSMALLSMBL-strlen(str4))/2)*5,50,str4,1,0);
//    PaintLCD();
//    udelay(3000000);
//    SetMode(SF_OnOffMode);
//    return 0;
//  }
////  char level[15]; //исследовали время, через которое батарея даёт нормальное напряжение
////  for (int i = 0;i<15;i++)
////  {
////  BatLvl[i] = CheckBattery(1);
////  udelay(100000); //снимаем каждую 0.1 сек
////  sprintf(level,"%.1f", BatLvl[i]); //записали в буффер
////  putString(i%3*40,i/3*10,level,1,0); //выводим значения
////  PaintLCD(); //выводим на экран
////  }
////  udelay(5000000); //смотрим на всё это 5 сек и продолжаем дальше
////  SetMode (SF_StartScreen);
//}
//
//
//
//void SF_StartScreen(void)
//{
//  
//  char tt[40];
//  char lw[8];   
//  CntWelcome ++;
//  //здесь обрабатываем экран и все остальное 
//      
//  ClearScreen();
//    RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
//    RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
//      int Mdl=0; // определяем модель по конфигурации длинн волн
//                for (int y=0;y<4;++y)
//          {
//            if((DataSetting.Lamda[y]>0)&&(DataSetting.Lamda[y]<1310))
//              Mdl|=1;
//            if(DataSetting.Lamda[y]>1300)
//              Mdl|=2;
//          }
//    
//    sprintf(tt,"%s-%d",MsgMass[6][UserSet.CurrLang],Mdl);//[MemDeviceConfig.Manufacture+MemUserConfig.Language]);
//    putString(((MAXWIDESMBL-strlen(tt))/2)*8,0,tt,1,1); // название прибора
//    //putString(10,0,tt,1,1);
//    sprintf(tt,"%s",MsgMass[12][UserSet.CurrLang]);
//    putString(((MAXSMALLSMBL-strlen(tt))/2)*5,15,tt,1,0); // наимнование фирмы
//     sprintf(tt,"*");
//
//    for (int i=0; i<4; ++i)
//    {
//      lw[0]=0;
//      if(DataSetting.Lamda[i]!=0)
//          sprintf(lw,"%d", DataSetting.Lamda[i]);
//      sprintf(tt,"%s:%s", tt, lw);
//
//    }
//    putString(0,35,tt,1,0);
//    //sprintf(tt,"%s : %0.2d.%0.2d.%0.2d", MsgMass[2][UserSet.CurrLang],RTC_DateStructure.RTC_Date, RTC_DateStructure.RTC_Month, RTC_DateStructure.RTC_Year);
//    //putString(0,25,tt,1,0);
//    //sprintf(tt,"%s: %0.2d:%0.2d:%0.2d", MsgMass[15][UserSet.CurrLang],RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds);
//    //putString(0,35,tt,1,0);
//    sprintf(tt,"№ %04d", MemDeviceConfig.SerNum);
//    putString(80,45,tt,1,1);
//    sprintf(tt,"%s(%X)", MemDeviceConfig.VerSW, CheckErr);
//    putString(0,54,tt,1,0);
//    PaintLCD();
//
//   // udelay(10000);
////обработчик клавиш
//     if (KeyEvent==bOK) 
//  {
//    KeyEvent=0; 
//    SetMode(SF_SettingDev);
//  }
//  
//    if (CntWelcome > WAITWELCOME) //ждём и автоматом переходим в меню Т5000
//  {
//    SetMode(SF_MainT5xxx);
//    CntWelcome = 0;
//    KeyEvent = 0;
//  }
//  
//}
//
//void SF_MainT5xxx(void) // прорисовка и управление основного меню
//{
//  static volatile unsigned char FrFreeInd = 0; //указатель на основной курсор
//  char Str[22];
//  //char Str1[22];
//  //char StrW[3]; // type fiber MM SM
//  Set_MAX_DB = (DataSetting.Lamda[iCurrLW]>1300)?(MAX_DB):(MAX_DB/2);
////  ClearScreen();
////  FrFree(FrFreeInd,  0, 63);
////  FrVal(1+iFixLvl,  64, 63);
////  FrdB(0, 16, 63); //номер строки, координата х, ширина рамки по х
//  
// // sprintf(Str,"%s",MsgMass[6][UserSet.CurrLang]);
// // putString(((MAXWIDESMBL-strlen(Str))/2)*8,0,Str,1,1);
//
//  if (KeyEvent==bOK) 
//  {
//    KeyEvent=0;
//    switch(DigitSet)
//    {
//    case 1:
//      DigitSet = 20;
//      break;
//    case 20:
//      DigitSet = 100;
//      break;
//    case 100:
//      DigitSet = 200;
//      break;
//    default:
//      DigitSet = 1;
//      break;
//    }
//    //if (DigitSet<200) DigitSet=DigitSet*10;
//    //else DigitSet=1;
////    KeyEvent=0;
////    if (FrFreeInd==0) 
////    {
////      DataSetting.fFixdB[iFixLvl][iCurrLW] = dBled;
////    }
////    else FrFreeSet=1;
////    if (FrFreeInd==1)
////    {
////      iFixLvl=0;
////      DataSetting.fFixdB[iFixLvl][iCurrLW] = dBled;
////    }
////    if (FrFreeInd==2) 
////    {
////      iFixLvl=1;
////      DataSetting.fFixdB[iFixLvl][iCurrLW] = dBled;
////    }
////
//////    if(dBled<=500) dBled +=100; //накидывали 250
//////    else dBled = 0;
////    
////    //CodeDAC +=256;
////    //CodeDAC &=0x0fff;
//    ScrRedraw = 1;
//  }
//  
//  if (KeyEvent==bUP)
//  {
//    KeyEvent=0;
//    FrFreeInd=(int)(FrFreeInd+3)%4;
//    if (FrFreeInd==0)
//  {
//    FrFreeSet=0;
//    FrdBSet = 1;
//  }
//    else
//    {
//      FrFreeSet=1;
//      FrdBSet = 0;
//    }
//    //if(dBled<=590) dBled +=10;
//    //else dBled = 0;
//    //CodeDAC +=16;
//    //CodeDAC &=0x0fff;
//    ScrRedraw = 1;
//  }
//  
//  if (KeyEvent==bDOWN)
//  {
//    KeyEvent=0;
//    FrFreeInd=(int)(FrFreeInd+1)%4;
//    if (FrFreeInd==0)
//  {
//    FrFreeSet=0;
//    FrdBSet = 1;
//  }
//    else
//    {
//      FrFreeSet=1;
//      FrdBSet = 0;
//    }
////    if(dBled>10) dBled -=10;
////    else dBled = 0;
//    //CodeDAC -=16;
//    //CodeDAC &=0x0fff;
//    ScrRedraw = 1;
//  }
//  
//  if (KeyEvent==bLEFT)
//  {
//    KeyEvent=0;
//    if (FrFreeInd==0)
//    { 
//      if(dBled>DigitSet) dBled -= DigitSet;
//      else dBled = 0;
//    }    
//    if (FrFreeInd==1 || FrFreeInd==2)
//    {
//    //  iFixLvl = FrFreeInd-1;
//      if(DataSetting.fFixdB[FrFreeInd-1][iCurrLW]>DigitSet) DataSetting.fFixdB[FrFreeInd-1][iCurrLW]-=DigitSet;
//      else DataSetting.fFixdB[FrFreeInd-1][iCurrLW] = 0;
//    }
//    //длины волн
//    if (FrFreeInd==3)
//    {        
//     DataSetting.fCurrdB[iCurrLW] = dBled;
//     // по новому
//     //while(
//     if(iCurrLW>0) iCurrLW--;   // здесь переключаем длины волн
//     else iCurrLW = 3;
//     if(DataSetting.Lamda[iCurrLW]==0)
//     {
//     if(iCurrLW>0) iCurrLW--;   // здесь переключаем длины волн
//     else iCurrLW = 3;
//     }
//     if(DataSetting.Lamda[iCurrLW]==0)
//     {
//     if(iCurrLW>0) iCurrLW--;   // здесь переключаем длины волн
//     else iCurrLW = 3;
//     }
//     if(DataSetting.Lamda[iCurrLW]==0)
//     {
//     if(iCurrLW>0) iCurrLW--;   // здесь переключаем длины волн
//     else iCurrLW = 3;
//     }
//     
//     dBled = DataSetting.fCurrdB[iCurrLW];
//    }
//    //CodeDAC -=1;
//    //CodeDAC &=0x0fff;
//    ScrRedraw = 1;
//  }
//  
//  if ((KeyEvent==(bLEFT+LONGP)) || (KeyEvent==(bLEFT+REPEAT))) //изменяем на 1
//  {
//    KeyEvent=0;
//    if (FrFreeInd==0)
//    {
//     if(dBled>DigitSet) dBled -=DigitSet;
//     else dBled = 0;
//    }    
//    if (FrFreeInd==1 || FrFreeInd==2)
//    {
//     // iFixLvl = FrFreeInd-1;
//      if(DataSetting.fFixdB[FrFreeInd-1][iCurrLW]>DigitSet) DataSetting.fFixdB[FrFreeInd-1][iCurrLW]-=DigitSet;
//      else DataSetting.fFixdB[FrFreeInd-1][iCurrLW] = 0;
//    }
//    udelay(200000);
//    ScrRedraw = 1;
//  }
//  
//  if (KeyEvent==bRIGHT)
//  {
//    
//    KeyEvent=0;
//    if (FrFreeInd==0)
//    {      
//      if(dBled<=Set_MAX_DB-DigitSet) dBled +=DigitSet;
//      else dBled = Set_MAX_DB;
//    }
//    if (FrFreeInd==1 || FrFreeInd==2)
//    {
//     // iFixLvl = FrFreeInd-1;
//      if(DataSetting.fFixdB[FrFreeInd-1][iCurrLW]<=Set_MAX_DB-DigitSet) DataSetting.fFixdB[FrFreeInd-1][iCurrLW]+=DigitSet;
//      else DataSetting.fFixdB[FrFreeInd-1][iCurrLW] = Set_MAX_DB;
//    }
//    //длины волн
//    if (FrFreeInd==3)
//    {   
//     DataSetting.fCurrdB[iCurrLW] = dBled; // запоминаем установку для данной длины волны
//     if(iCurrLW<3) iCurrLW++; // здесь переключаем длины волн (
//     else iCurrLW = 0;
//     if(DataSetting.Lamda[iCurrLW]==0)
//     {
//     if(iCurrLW<3) iCurrLW++;   // здесь переключаем длины волн
//     else iCurrLW = 0;
//     }
//     if(DataSetting.Lamda[iCurrLW]==0)
//     {
//     if(iCurrLW<3) iCurrLW++;   // здесь переключаем длины волн
//     else iCurrLW = 0;
//     }
//     if(DataSetting.Lamda[iCurrLW]==0)
//     {
//     if(iCurrLW<3) iCurrLW++;   // здесь переключаем длины волн
//     else iCurrLW = 0;
//     }
//     
//     DataSetting.CurrPlaceLW = iCurrLW;
//     dBled = DataSetting.fCurrdB[iCurrLW]; // востанавливаем установку для данной длины волны
//    }
//    //CodeDAC +=1;
//    //CodeDAC &=0x0fff;    
//    ScrRedraw = 1;
//  }
//  //  if (KeyEvent==bS) //отключаем все источники
//  //    {
//  //      KeyEvent=0;
//  //      dBled = 0;
//  //    }
//  
//  
//  if ((KeyEvent==(bRIGHT+REPEAT)) || (KeyEvent==(bRIGHT+LONGP))) //изменяем на 1
//  {
//    KeyEvent=0;
//    if (FrFreeInd==0)
//    {
//     if(dBled<=Set_MAX_DB-DigitSet) dBled +=DigitSet;
//     else dBled = Set_MAX_DB;
//    }
//    if (FrFreeInd==1 || FrFreeInd==2)
//    {
//      //iFixLvl = FrFreeInd-1;
//      if(DataSetting.fFixdB[FrFreeInd-1][iCurrLW]<=Set_MAX_DB-DigitSet) DataSetting.fFixdB[FrFreeInd-1][iCurrLW]+=DigitSet;
//      else DataSetting.fFixdB[FrFreeInd-1][iCurrLW] = Set_MAX_DB;
//    }
//    udelay(200000);
//    ScrRedraw = 1;
//  }
//  
//  if (KeyEvent==(bS+LONGP)) // вызов функции выбора выключения
//  {
//    KeyEvent=0;
//    FrSetOnOff = 1;
//    SetMode(SF_OnOffMode);
//    ScrRedraw = 1;
//  }
//  
//  if (KeyEvent==bS) //сброс в "0" затухания, без записи в память
//  {
//    KeyEvent=0;
//    FrdBSet = 1;
//    // изменено так как надо переключатся на установки дБ когда были в других строках
//    FrFreeSet = 0;//
//    FrFreeInd = 0;//
//    dBled = 0;
////    DataSetting.fCurrdB[iCurrLW] = dBled;
////    if(iCurrLW<3) iCurrLW++;
////    else iCurrLW = 0;
////    dBled = DataSetting.fCurrdB[iCurrLW];
//    ScrRedraw = 1;
//  }
//  
//  if (KeyEvent==bMENU+LONGP) //возврат к экрану заставки нажатием кнопки МЕНЮ долго
//  {
////    KeyEvent=0;
////    if (DigitSet<100) DigitSet=DigitSet*10;
////    else DigitSet=1;
////    DataSetting.fFixdB[iFixLvl][iCurrLW] = dBled;
////    if(iFixLvl) iFixLvl = 0;
////    else iFixLvl=1;
////    ScrRedraw = 1;
//  }
//  
//  if (KeyEvent==bMENU) //переключение индикатора рамки фиксированных значений
//  {
//    KeyEvent=0;
//    FrdBSet = 1;
//    FrFreeSet = 0;
//    FrFreeInd = 0;
//    dBled = DataSetting.fFixdB[iFixLvl][iCurrLW];
//    
//    if(iFixLvl) iFixLvl = 0;
//    else iFixLvl=1;  
//    ScrRedraw = 1;
//  }
//  
//  if(ScrRedraw) //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//  {
//  ClearScreen();
//  BatProc = CheckBattery(0);
//  FrFree(FrFreeInd,  0, 71);
//  FrVal(1+iFixLvl,  72, 55);
//  FrdB(0, 27, 73);
//  //FrBat(0, 0, 27);
//  FrBatNew(0, 0, (int)(BatProc));
//  // рисуем батарейку
//  //sprintf(Str,"%.0f%s",BatProc,"%");
//  //putString(2,5,Str,1,0);
//  // рисуем значение ШАГА изменений
//  if(DigitSet<SHAG)
//  sprintf(Str,"%2.2f",DigitSet/SHAG);
//  else
//  sprintf(Str,"%2.1f",DigitSet/SHAG);
//  putString(104,5,Str,1,0);  
//  
//  // запомним текущее значение затухания для выбраной длины волны при изменениях
//  DataSetting.fCurrdB[iCurrLW] = dBled;
//  // прорисуем экран
//  
//  //putString(((MAXWIDESMBL-strlen(Str))/2)*8,13,Str,1,1); // значение в дБ
//  // рамка управляемая
//  //FrFree(2 ,  64*iFixLvl, 63);
//  //значения фиксированных уровней
//  sprintf(Str,"%s",MsgMass[31][UserSet.CurrLang]);
//  putString(3,20,Str,1,0);  
//  sprintf(Str,"%2.2f %s",DataSetting.fFixdB[0][iCurrLW]/SHAG, MsgMass[49][UserSet.CurrLang]);
//  putString(75,20,Str,1,0); // значение dB 1-fix
//  
//  sprintf(Str,"%s",MsgMass[32][UserSet.CurrLang]);
//  putString(3,32,Str,1,0); 
//  sprintf(Str,"%2.2f %s",DataSetting.fFixdB[1][iCurrLW]/SHAG, MsgMass[49][UserSet.CurrLang]);
//  putString(75,32,Str,1,0); // значение dB 2-fix
//  
// // прорисовка длины волны
//  sprintf(Str,"%s",MsgMass[30][UserSet.CurrLang]); // надпись Длина Волны
//  putString(3,44,Str,1,0); 
//
//  sprintf(Str,"%d%s %s",DataSetting.Lamda[iCurrLW], MsgMass[38][UserSet.CurrLang],(DataSetting.Lamda[iCurrLW]>1300)?("SM"):("MM"));
//  putString(75,44,Str,1,0); // значение длины волны
//  ScrRedraw = 0;
// // строка значений дБ
//  sprintf(Str,"%2.2f %s",dBled/SHAG, MsgMass[49][UserSet.CurrLang]);// значение в дБ
// 
//  // расчет кодов ЦAП от дБ если работа
//  if(UserConfig.ChnMod)
//  {    
//    sprintf(Str,"%d",CodeDAC);// значение кодов установлных
//
//    putString(((MAXWIDESMBL-strlen(Str))/2)*8+4,3,Str,0,1); // значение в дБ
//    sprintf(Str,"!SET!");
//    putString(0,54,Str,0,0); // режим установки!
//  }
//  else
//  {
//    putString(((MAXWIDESMBL-strlen(Str))/2)*8-1,3,Str,1,1); // значение в дБ
//    // здесь забито в ручном режиме( имеет перегруз - более 4095 при 80 дБ Б нет разных уровней для многомодов и одномодов с разными длинами волн)
//    // dBled - указатель на ячейку в которой хранится код
//    // прочитаем код
//          UB_I2C1_ReadMultiByte(4096*iCurrLW + dBled*2, (void*)&CodeDAC, 2);
//
////    if(dBled<5) CodeDAC = (1008 + (uint16_t)(dBled*35.4));
////    else if(dBled<10) CodeDAC = (1185 + (uint16_t)((dBled-5)*13));
////    else if(dBled<20) CodeDAC = (1250 + (uint16_t)((dBled-10)*6.5));
////    else if(dBled<50) CodeDAC = (1315 + (uint16_t)((dBled-20)*3.9));
////    else if(dBled<100) CodeDAC = (1432 + (uint16_t)((dBled-50)*2.4));
////    else  CodeDAC = (1552 + (uint16_t)((dBled-100)*1.7));
//  }
//  //sprintf(Str,"%04d = %1.2f",CodeDAC,(5.0*CodeDAC)/4095);
//  //putString(((MAXWIDESMBL-strlen(Str))/2)*8,54,Str,1,0); // справочно коды ЦАП и напряжение
//  PaintLCD();
//  }
//}
//
//
//
//
//void SF_ResultTable(void) // прорисовка и управление табличного отображения результатов
//{
//}
//
//
//void SF_ResultGraph(void) // прорисовка и управление графического отображения результатов
//{
//}
//
//void SF_OnOffModeOld (void) // режим выкючения прибора (спящий режим???)
//{
//  
//  char Str[22];
//  
//  ClearScreen();
//  if (KeyEvent==bDOWN)
//  {
//    KeyEvent=0;
//    FrSetOnOff = ChangeFrSet (FrSetOnOff, 1, 0, MINUS);// установка курсора в рамках заданных параметров
//  }
//  if (KeyEvent==bUP)
//  {
//    KeyEvent=0;
//    FrSetOnOff = ChangeFrSet (FrSetOnOff, 1, 0, PLUS);// установка курсора в рамках заданных параметров
//  }
//  if (KeyEvent==bMENU)
//  {
//    KeyEvent=0;
//    SetMode(0); // возврат в предыдущий режим
//  }
//  if (KeyEvent==bOK)
//  {
//    ReWrDataUser (1);// теперь переписать из одной структуры в другую
//    UserCfgWrite ();
//    KeyEvent=0;
//    if (!FrSetOnOff) // выкл. прибора
//    {
// DeviceAllOff (); // выключение прибора
//    }
//    else // отмена
//    {
//      SetMode(0); // возврат в предыдущий режим
//    }
//  }
//  sprintf(Str,"%s",MsgMass[19][UserSet.CurrLang]); //выкл. прибор
//  putString(2,6,Str,1,0);
//  sprintf(Str,"%s",MsgMass[20][UserSet.CurrLang]); // отмена
//  putString(2,18,Str,1,0);
//  FrBig(FrSetOnOff,10); // прорисовка динамической рамки для режима
//  PaintLCD();
//}
//
//void SF_OnOffMode (void) // режим выкючения прибора (спящий режим???)
//{
//  
//    ClearScreen();
//    UserConfig.ChnMod = 0;
//    ReWrDataUser (1);// теперь переписать из одной структуры в другую
//    UserCfgWrite ();
//    ReWrDataSett (1);// теперь переписать из одной структуры в другую Настройки
//    SettingCfgWrite (); // пишем в EEPROM 
//    KeyEvent=0;
//    DeviceAllOff (); // выключение прибора
//
//}
//
//void SF_ClearMem (void) // режим очистки памяти прибора ()
//{  
//}
//
//void DeviceAllOff (void)// выключение прибора (одна функция) для выключения
//{
//        char Str[22];
//
//      ClearScreen(); // очищаем экран
//      sprintf(Str,"%s",MsgMass[26][UserSet.CurrLang]); //вкл. прибор
//      putString(((MAXSMALLSMBL-strlen(Str))/2)*5,15,Str,1,0);
//      sprintf(Str,"%s",MsgMass[27][UserSet.CurrLang]); // ждите
//      putString(((MAXSMALLSMBL-strlen(Str))/2)*5,30,Str,1,0);
//      PaintLCD();
//      //StopMode_Measure();
//      CntWelcome =0;
//      SetMode(SF_StartScreen);
//      vDeepSleep();     // здесь должен быть код засыпания и просыпания прибора
//}
//
//
//void SF_MemoryMain (void) // МЕНЮ работы с памятью
//{
//}
//
//void SF_MainSetting (void) // меню выбора УСТАНОВОК (прибор, дата/время, файл)
//{
//}
//
//void SF_SettingDev (void) // меню УСТАНОВОК ПРИБОРА (каналы, автовыкл, контраст)
//{
// 
//  static volatile unsigned char FrSetSetting = 0; // указатель на курсор
//  char Str[22];
//  
//  ClearScreen();
////  if (KeyEvent==(bS+LONGP)) // вызов функции выбора выключения
////  {
////    KeyEvent=0;
////    FrSetOnOff = 1;
////    SetMode(SF_OnOffMode);
////  }
//  if (KeyEvent==bMENU)
//  {
//    KeyEvent=0;
//    ReWrDataUser (1);// теперь переписать рабочей в память 
////    memcpy ((void*)DeviceConfig.Model,(void*)MsgMass[6][UserSet.CurrLang],13); //записываем текущее значение имени прибора
//    
//    SetMode(0); // возврат в ПРЕДЫДУЩЕЕ меню
//  }
////  if (KeyEvent==(bMENU+LONGP))
////  {
////    KeyEvent=0;
////    ReWrDataUser (1);// теперь переписать рабочей в память  
//////    memcpy ((void*)DeviceConfig.Model,(void*)MsgMass[6][UserSet.CurrLang],13); //записываем текущее значение имени прибора
////    
////    SetMode(SF_MainMenu); // возврат в основное меню
////  }
////  if (KeyEvent==bDOWN)
////  {
////    KeyEvent=0;
////    FrSetSetting = ChangeFrSet (FrSetSetting, 3, 0, PLUS);// установка курсора в рамках заданных параметров
////  }
////  if (KeyEvent==bUP)
////  {
////    KeyEvent=0;
////    FrSetSetting = ChangeFrSet (FrSetSetting, 3, 0, MINUS);// установка курсора в рамках заданных параметров
////  }
//  if (KeyEvent==bRIGHT + REPEAT)
//  {
//    KeyEvent=0;
//    UserConfig.Contrast = ChangeFrSet (UserConfig.Contrast, 100, 0, PLUS);// установка курсора в рамках заданных параметров
//    LDC_Set_Contr(UserConfig.Contrast); // установка текущей контрастности
//  }
//  if (KeyEvent==bRIGHT)
//  {
//    KeyEvent=0;
////    switch(FrSetSetting)
////    {
////    case 0:
////      UserConfig.ChnMod = ChangeFrSet (UserConfig.ChnMod, 2, 0, PLUS);// установка курсора в рамках заданных параметров
////      switch(UserConfig.ChnMod)
////      {
////      case 0: //18 chanel - set cursor of default
////        CurIndLambda = 0; // 1290 ()
////        FrSetResTabl = 1;
////        FrSetResGrph = 0;
////        break;
////      case 1: //8R chanel - set cursor of default
////        CurIndLambda = 2; // 1310 ()
////        FrSetResTabl = 1;
////        FrSetResGrph = 2;
////        break;
////      case 2: //8T chanel - set cursor of default
////        CurIndLambda = 10; // 1470 ()
////        FrSetResTabl = 1;
////        FrSetResGrph = 10;
////        break;
////      }
////      break;
////    case 1:
////      UserConfig.AutoOffDev = ChangeFrSet (UserConfig.AutoOffDev, 1, 0, PLUS);// установка курсора в рамках заданных параметров
////      break;
////    case 2: // контрастность
//      UserConfig.Contrast = ChangeFrSet (UserConfig.Contrast, 100, 0, PLUS);// установка курсора в рамках заданных параметров
//      LDC_Set_Contr(UserConfig.Contrast); // установка текущей контрастности
////      break;
////    case 3: // Выбор языка (Language)
////      UserConfig.Language = ChangeFrSet (UserConfig.Language, 1, 0, PLUS);// установка курсора в рамках заданных параметров
////    memcpy ((void*)DeviceConfig.Model,(void*)MsgMass[6][UserSet.CurrLang],13); //записываем текущее значение имени прибора
////        NeedSaveCfg = 1; 
////      break;
////    }
//  }
//  if (KeyEvent==bLEFT + REPEAT)
//  {
//    KeyEvent=0;
//    UserConfig.Contrast = ChangeFrSet (UserConfig.Contrast, 100, 0, MINUS);// установка курсора в рамках заданных параметров
//    LDC_Set_Contr(UserConfig.Contrast); // установка текущей контрастности
//  }
//  if (KeyEvent==bLEFT)
//  {
//    KeyEvent=0;
////    switch(FrSetSetting)
////    {
////    case 0:
////      UserConfig.ChnMod = ChangeFrSet (UserConfig.ChnMod, 2, 0, MINUS);// установка курсора в рамках заданных параметров
////      break;
////    case 1:
////      UserConfig.AutoOffDev = ChangeFrSet (UserConfig.AutoOffDev, 1, 0, MINUS);// установка курсора в рамках заданных параметров
////      break;
////    case 2: // контрастность
//      UserConfig.Contrast = ChangeFrSet (UserConfig.Contrast, 100, 0, MINUS);// установка курсора в рамках заданных параметров
//      LDC_Set_Contr(UserConfig.Contrast); // установка текущей контрастности
////      break;
////    case 3: // Выбор языка (Language)
////      UserConfig.Language = ChangeFrSet (UserConfig.Language, 1, 0, MINUS);// установка курсора в рамках заданных параметров
////    memcpy ((void*)DeviceConfig.Model,(void*)MsgMass[6][UserSet.CurrLang],13); //записываем текущее значение имени прибора
////            NeedSaveCfg = 1; 
////
////      break;
////    }
//  }
////  
////  
////  sprintf(Str,"%s",MsgMass[21][UserSet.CurrLang]); //каналы
////  putString(2,6,Str,1,0);
////  //ChMd
////  sprintf(Str,"%s",ChMd[UserConfig.ChnMod]); //каналы
////  putString(80,6,Str,1,0);
////  sprintf(Str,"%s",MsgMass[22][UserSet.CurrLang]); // автовыключение
////  putString(2,18,Str,1,0);
////  sprintf(Str,"%s",(UserConfig.AutoOffDev)?(MsgMass[23][UserSet.CurrLang]):(MsgMass[24][UserSet.CurrLang])); // нет/да
////  putString(80,18,Str,1,0);
//  sprintf(Str,"%s",MsgMass[25][UserSet.CurrLang]); // контрастность
//  putString(2,6,Str,1,0);
//  sprintf(Str,"%d",UserConfig.Contrast); // контрастность
//  putString(80,6,Str,1,0);
////  sprintf(Str,"Language"); // язык
////  putString(2,42,Str,1,0);
////  sprintf(Str,"%s",MsgMass[0][UserSet.CurrLang]); // Язык
////  putString(80,42,Str,1,0);
////  
//  FrBig(FrSetSetting,10); // прорисовка динамической рамки для режима
//  FrSmall(FrSetSetting,10); // прорисовка динамической рамки для режима
////  
////  
//  PaintLCD();
//}
//
//void SF_SettingDaTime (void) // меню УСТАНОВОК даты и времени
//{
//
//}
//
//void SF_SettingFile (void) // меню настройки файла ()
//{
//}
//
//
//
//void SF_ErrMsg (void) // Рисовалка отображения ошибок режимов 
//{
//    //static volatile unsigned char SetErrMsg = 0; // указатель на курсор
//  char Str[22];
//  
//  ClearScreen();
//  if (KeyEvent==(bS+LONGP)) // вызов функции выбора выключения
//  {
//    KeyEvent=0;
//    FrSetOnOff = 1;
//    SetErrMsg = 0;
//    SetMode(SF_OnOffMode);
//    ScrRedraw = 1;
//
//    
//  }
//  if (KeyEvent==bMENU)
//  {
//    KeyEvent=0;
//    SetErrMsg = 0;
//    SetMode(SF_MainT5xxx); // возврат в основное меню
//    //SetMode(0); // вр=озврат в предыдущее меню не работает так как может быть двойное понижение при ожидании...
//    ScrRedraw = 1;
//
//  }
//  if (KeyEvent==(bMENU+LONGP))
//  {
//    KeyEvent=0;
//    SetErrMsg = 0;
//    SetMode(SF_MainT5xxx); // возврат в основное меню
//    ScrRedraw = 1;
//
//  }
//  switch (SetErrMsg)
//  {
//  case 1:
//  sprintf(Str,"%s",MsgMass[40][UserSet.CurrLang]); // SD карта не найдена
//      putString(((MAXSMALLSMBL-strlen(Str))/2)*5,6,Str,1,0);
//  sprintf(Str,"%s",MsgMass[41][UserSet.CurrLang]); // или защита от записи
//      putString(((MAXSMALLSMBL-strlen(Str))/2)*5,18,Str,1,0);
//    break;
//  case 2:
//  sprintf(Str,"%s",MsgMass[47][UserSet.CurrLang]); // Батарея
//      putString(((MAXSMALLSMBL-strlen(Str))/2)*5,6,Str,1,0);
//  sprintf(Str,"%s",MsgMass[48][UserSet.CurrLang]); // Разряжена
//      putString(((MAXSMALLSMBL-strlen(Str))/2)*5,18,Str,1,0);
//    sprintf(Str,"%c%c%.0f%%",176,177,BatteryVolt);//CheckBattery()
//      putString(((MAXSMALLSMBL-strlen(Str))/2)*5,30,Str,1,0);
//    break;
//  }
//  
//  PaintLCD();
//
//}
//
//
//
//void KeybCntrl (void) // переключатель указателя в клавиатуре
//{
//     if ((KeyEvent==(bUP))||(KeyEvent==(bUP+REPEAT)))
//      {
//        KeyEvent=0;
//        if (KbPosY > 0) KbPosY--;
//        else KbPosY =3;
//      }
//    if ((KeyEvent==(bDOWN))||(KeyEvent==(bDOWN+REPEAT)))
//      {
//        KeyEvent=0;
//        if (KbPosY < 3) KbPosY++;
//        else KbPosY =0;
//      }
//    if ((KeyEvent==(bLEFT))||(KeyEvent==(bLEFT+REPEAT)))
//      {
//        KeyEvent=0;
//        if (KbPosX > 0) KbPosX--;
//        else KbPosX =11;
//      }
//    if ((KeyEvent==(bRIGHT))||(KeyEvent==(bRIGHT+REPEAT)))
//      {
//        KeyEvent=0;
//        if (KbPosX < 11) KbPosX++;
//        else KbPosX =0;
//      }
//}
//
//// прорисовка клавиатуры с установленным курсором на символ
//void Draw_KeyBoard(unsigned char Cursor, char* Str_first, char* Str_edit, unsigned char Shift ,unsigned char Lang)
//{
//  char CommScr[22] = "                     \0"; // строка редактора для вывода на экран
//  unsigned char i;
//  ClearScreen();
//  //Str_edit[ARRAY_SIZE(Str_edit)-1]=0; // последний элемент в массиве равен 0
//  strcpy(CommScr,Str_edit);
//  if (CommScr[Cursor+1] != 0) // не конец строки
//  {
//    CommScr[Cursor]=0x5F; // курсор на последнем месте
//  }
//  else
//  {   
//  if (CommScr[Cursor] == ' ')CommScr[Cursor]=0x5F; // курсор на последнем месте
//  }
//  putString(2,10,CommScr,1,0);
//  //  sprintf(F_name,"%02X%02X%02X_%02X%02X%X.sor",ds1337[6],ds1337[5],ds1337[4],ds1337[2],ds1337[1],ds1337[0]>>4);
//  putString(0,2,Str_first,1,0);
//
//  
//  
//  for (i=0; i<48; i++)
//  {
//    putChar((i%12)*10, 24+((int)(i/12))*10,Tab_Kb[i+Shift*48+Lang*96],0,1,0);
//  }
//  Knob(KbPosX,KbPosY);
//}
//
//void Knob(unsigned char X,unsigned char Y)
//{
//  
//    DrawLine(X*10,Y*10+23,X*10+10,Y*10+23,1,1);
//    DrawLine(X*10,Y*10+23,X*10,Y*10+33,1,1);
//    DrawLine(X*10,Y*10+33,X*10+10,Y*10+33,1,1);
//    DrawLine(X*10+10,Y*10+23,X*10+10,Y*10+33,1,1);
// 
//}
//
//// Большая рамка до 66
//void FrBig(unsigned char SW,  char Add)
//{
//    //  DrawLine(0,0+SW*12,76,0+SW*12,12,0,screen); // очистка окна ,большого
//    // DrawLine(80,0+SW*12,124,0+SW*12,12,0,screen);
//    if (((66+Add)>127)||((66+Add)<0)) Add = 0;
//
//    DrawLine(0,4+SW*12,66+Add,4+SW*12,1,1);
//    DrawLine(0,4+SW*12,0,15+SW*12,1,1);
//    DrawLine(0,15+SW*12,66+Add,15+SW*12,1,1);
//    DrawLine(66+Add,4+SW*12,66+Add,15+SW*12,1,1);
//}
//
//// Маленькая рамка
//void FrSmall(unsigned char SW ,  char Add)
//{
//    if (((68+Add)>127)||((68+Add)<0)) Add = 0;
//    DrawLine(68+Add,4+SW*12,124,4+SW*12,1,1);
//    DrawLine(68+Add,4+SW*12,68+Add,15+SW*12,1,1);
//    DrawLine(68+Add,15+SW*12,124,15+SW*12,1,1);
//    DrawLine(124,4+SW*12,124,15+SW*12,1,1);
//}
//
//// рамка произвольного размера и места
//void FrFree(unsigned char SW ,  int X, int Size)
//{
//    if (((X+Size)>0)&&((X+Size)<128)&&FrFreeSet)
//    {
//    DrawLine(X,6+SW*12,X+Size,6+SW*12,1,1);
//    DrawLine(X,6+SW*12,X,16+SW*12,1,1);
//    DrawLine(X,16+SW*12,X+Size,16+SW*12,1,1);
//    DrawLine(X+Size,6+SW*12,X+Size,16+SW*12,1,1);
//    }
//    
//}
//
////для батареи
//void FrBatNew(int Y , int X, int Size)// батарея, горизонтальня 12 пикселей , от x y  вниз 
//{
//    if (((X+12)>0)&&((X+12)<128))
//    {
//    DrawLine(X,Y,X+11,Y,1,1); // гориз врехняя
//    DrawLine(X,Y+7,X+11,Y+7,1,1); // гориз нижняя
//    DrawLine(X,Y,X,Y+7,1,1); // вертикальная правая
//    DrawLine(X+11,Y,X+11,Y+2,1,1); // вертикальная правая
//    DrawLine(X+11,Y+5,X+11,Y+7,1,1); // вертикальная правая
//    DrawLine(X+12,Y+2,X+12,Y+5,1,1); // вертикальная правая
//    for(int l=0;l<Size/10;l++)
//         DrawLine(X+l+1,Y,X+l+1,Y+7,1,1); // вертикальная правая
// 
//    //DrawLine(X+Size,3+SW*12,X+Size,13+SW*12,1,1);
//    //DrawLine(X+Size,5+SW*12,3+X+Size,5+SW*12,1,1);
//    //DrawLine(3+X+Size,5+SW*12,3+X+Size,11+SW*12,1,1);
//    //DrawLine(X+Size,11+SW*12,3+X+Size,11+SW*12,1,1);
//    }
//    
//}
//
//void FrBat(unsigned char SW ,  int X, int Size)
//{
//    if (((X+Size)>0)&&((X+Size)<128))
//    {
//    DrawLine(X,3+SW*12,X+Size,3+SW*12,1,1);
//    DrawLine(X,3+SW*12,X,13+SW*12,1,1);
//    DrawLine(X,13+SW*12,X+Size,13+SW*12,1,1);
//    DrawLine(X+Size,3+SW*12,X+Size,13+SW*12,1,1);
//    DrawLine(X+Size,5+SW*12,3+X+Size,5+SW*12,1,1);
//    DrawLine(3+X+Size,5+SW*12,3+X+Size,11+SW*12,1,1);
//    DrawLine(X+Size,11+SW*12,3+X+Size,11+SW*12,1,1);
//    }
//    
//}
//
////рамка для подсветки верхнего рабочего значения затухания
//void FrdB(unsigned char SW ,  int X, int Size)
//{
//    if (((X+Size)>0)&&((X+Size)<128)&&FrdBSet)
//    {
//    DrawLine(X,SW*12,X+Size,SW*12,1,1);
//    DrawLine(X,SW*12,X,16+SW*12,1,1);
//    DrawLine(X,16+SW*12,X+Size,16+SW*12,1,1);
//    DrawLine(X+Size,SW*12,X+Size,16+SW*12,1,1);
//    }
//    
//}
////такая же, но для второго столбца меню
//void FrVal(unsigned char SW ,  int X, int Size)
//{
//    if (((X+Size)>0)&&((X+Size)<128))
//    {
//    DrawLine(X,6+SW*12,X+Size,6+SW*12,1,1);
//    DrawLine(X,6+SW*12,X,16+SW*12,1,1);
//    DrawLine(X,16+SW*12,X+Size,16+SW*12,1,1);
//    DrawLine(X+Size,6+SW*12,X+Size,16+SW*12,1,1);
//    }
//    
//}
//
//
//// рамка длин волн для 18
//void FrLamda18(unsigned char SW)
//{
//  int Y = SW%6;
//  int X = SW/6;
//    DrawLine(X*42,Y*10,X*42+41,Y*10,1,1);
//    DrawLine(X*42,Y*10,X*42,Y*10+10,1,1);
//    DrawLine(X*42,Y*10+10,X*42+41,Y*10+10,1,1);
//    DrawLine(X*42+41,Y*10,X*42+41,Y*10+10,1,1);
//}
//
//// рамка длин волн для 9
//void FrLamda9(unsigned char SW)
//{
//  int Y = SW%3;
//  int X = SW/3;
//    DrawLine(X*42,Y*16+16,X*42+41,Y*16+16,1,1);
//    DrawLine(X*42,Y*16+16,X*42,Y*16+31,1,1);
//    DrawLine(X*42,Y*16+31,X*42+41,Y*16+31,1,1);
//    DrawLine(X*42+41,Y*16+16,X*42+41,Y*16+31,1,1);
//}
//
//
//// рамка произвольного размера и места пиксельной установки шириной 11 точек
//
//void FrFreePx(int X, int Y ,  char* PrntStr)
//{
//  int Len = strlen(PrntStr); // длинна сторки
//    if (X==0)
//    {
//        X =((MAXSMALLSMBL-Len)/2)*6-6;
//      
//    }      
//      if ((Len*6+X+12) < 128) // строка помещается
//      {
//        // чистим поле под надпись
//    DrawLine(X,Y,Len*6+X+12,Y,20,0); // очистка окошек для надписей ,большого
//        // внешняя рамка
//    DrawLine(X+2,Y+2,Len*6+X+10,Y+2,1,1);  
//    DrawLine(X+2,Y+2,X+2,Y+18,1,1);
//    DrawLine(X+2,Y+18,Len*6+X+10,Y+18,1,1);
//    DrawLine(Len*6+X+10,Y+2,Len*6+X+10,Y+18,1,1);
//        // внутреняя рамка
//    DrawLine(X+4,Y+4,Len*6+X+8,Y+4,1,1);  
//    DrawLine(X+4,Y+4,X+4,Y+16,1,1);
//    DrawLine(X+4,Y+16,Len*6+X+8,Y+16,1,1);
//    DrawLine(Len*6+X+8,Y+4,Len*6+X+8,Y+16,1,1);
//    putString(X+6,Y+6,PrntStr,1,0);
//      }
//
//    
//}
//
//unsigned char ChangeFrSet (unsigned char FrSet, unsigned char MaxSet, unsigned char MinSet, unsigned char DirSet)// установка курсора в рамках заданных параметров
//{
//  if (DirSet) // вверх
//  {
//    if (FrSet < MaxSet) FrSet++;
//    else FrSet = MinSet;
//  }
//  else
//  {
//    if (FrSet > MinSet) FrSet--;
//    else FrSet = MaxSet;
//  }
//   return FrSet; 
//}
//
//unsigned char SearchEndStr(char* EditStr, unsigned char FindEnd) // ищем место курсора в редактируемой строке
//{
//unsigned char Find = FindEnd;
//for (int i = 0; i<FindEnd; i++)
//  {
//    if (EditStr[i]<=32) EditStr[i]=' ';
//  }
//  EditStr[FindEnd]=0; // принудительное ограничение строки в заданном диапазоне
//  for (int i = FindEnd-1; i>0; i--)
//  {
//    if (EditStr[i]<=32) Find--;
//    else break;
//  }
//  return Find;
//}
//
