//#define MAXWIDESMBL 17 // максимальное число широких символов в строке 
//#define MAXSMALLSMBL 22
//#define WAITWELCOME 100 // число циклов ожидания заставки
//#define PLUS 1
//#define MINUS 0
//#define Pic_Tx 67 // X - смещение картинки при выводе установки времени
//#define Pic_Ty 5 // Y - смещение картинки при выводе установки времени

#define INCR 1
#define DECR -1



#include "main.h"

#include "fatfs.h"

const char *MsgWeek[53]={
"1янв-7янв",
"8янв-14янв",
"15янв-21янв",
"22янв-28янв",
"29янв-4фев",
"5фев-11фев",
"12фев-18фев",
"19фев-25фев",
"26фев-4мар",
"5мар-11мар",
"12мар-18мар",
"19мар-25мар",
"26мар-1апр",
"2апр-8апр",
"9апр-15апр",
"16апр-22апр",
"23апр-29апр",
"30апр-6май",
"7май-13май",
"14май-20май",
"21май-27май",
"28май-3июн",
"4июн-10июн",
"11июн-17июн",
"18июн-24июн",
"25июн-1июл",
"2июл-8июл",
"9июл-15июл",
"16июл-22июл",
"23июл-29июл",
"30июл-5авг",
"6авг-12авг",
"13авг-19авг",
"20авг-26авг",
"27авг-2сен",
"3сен-9сен",
"10сен-16сен",
"17сен-23сен",
"24сен-30сен",
"1окт-7окт",
"8окт-14окт",
"15окт-21окт",
"22окт-28окт",
"29окт-4ноя",
"5ноя-11ноя",
"12ноя-18ноя",
"19ноя-25ноя",
"26ноя-2дек",
"3дек-9дек",
"10дек-16дек",
"17дек-23дек",
"24дек-30дек",
"31дек-31дек"
};
const uint16_t ColorsPCO[18]={56731,40412,40545,52448,65049,63387,65535,50712,0,64872,33808,60445,31,1024,65504,64800,63488,37440};


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

static BYTE PosCurr = 6; // положение курсора устанавливаемых величин в настройках часов

//static unsigned char Ptr; // указатель с какой строкой работаем (чтобы перекопировать)
static void (*ModeFunc)(void);
//static void (*ModeFuncPrev)(void) = 0;

//static unsigned char FrdBSet=1; //признак включения рамки у рабочего значения затухания (сверху)
//static unsigned char FrFreeSet=0; //признак включения рамки основного курсора левый столбец
//static unsigned char FrValSet = 1; //признак включения рамки основного курсора правый столбец
unsigned char ScrRedraw = 1; //признак перерисовки экрана

volatile BYTE NeedKeyB = 0; // необходимость переключения в клавиатуру
char IndxKBCableID = 0; // Индексы указатели для строк редактирования - Имя кабеля
char IndxKBFiberName = 0; // Индексы указатели для строк редактирования - Имя волокна
char IndxKBComments = 0; // Индексы указатели для строк редактирования - Комментарий
volatile int NeedReturn = 0; // необходимость вернуться в окно сохранения
volatile int NeedWinKBReturn = 0; // необходимость вернуться в окно откуда вызвали редактор

static volatile BYTE ViewMod = 1; // режим простотра таблица или график

//static char* globalStringToCopy = 0;
//char Str[64];
char StrN[32];
char St[5];
char StrI[32];
char StrF[24];
//static char AnlzMode = MOD_ANLZ_TAB;

//static unsigned char TabFiles[16];
//для батареи
//void FrBat(unsigned char SW ,  int X, int Size); //???
//void FrBatNew(int Y ,  int X, int Size); //???
float BatProc; //перем. для уровня заряда в %

unsigned short CalkCheckSum (void)// подсчет контрольной суммы конфигурации прибора
{
  unsigned short Sum =0;
  
  Sum = 0x2A56;
  return Sum;
}


   // получение данных от измерителя
   void GetAllDataMeas(int Indx)
   {
     for(int i=0;i<18;i++)
     {
       CWDMData[i] = -0.2*((Indx-16)*(i-18)+i*Indx)+17.6;
     }
   }

//char GetModeDev (void) // возвращает текущий режим
//{
//  return CurrMode;
//}
// функция инициализации страницы по номеру в NEXTION
void CmdInitPage(int Num)
{
  char str[44];
  NumCurrPage = Num;
  sprintf(str, "page %dяяя",Num); // < событиe >
  NEX_Transmit((void*)str);    //
  
  //NEX_Transmit((void*)CmdNextion[Num]);
  g_FirstScr=1;
  //CreatDelay(20000);// 177 как в 173
  HAL_Delay(100);
}


void ModeFuncTmp(void)
{
  char StrL[64];
  ModeFunc();
    // если ошибка индикатора напишем сообщение
  if(g_ErrFW_LCD && TimerDraw)
  {
    sprintf( StrL,"xstr 10,120,460,40,2,RED,WHITE,1,1,1,\"%s\"яяя","ОШИБКА! ПО LCD "); //  сообщение об ошибке FW LCD
    NEX_Transmit((void*)StrL);//
    sprintf( StrL,"xstr 10,160,460,40,2,RED,WHITE,1,1,1,\"%s\"яяя","от другого прибора"); //  сообщение об ошибке FW LCD
    NEX_Transmit((void*)StrL);//
    TimerDraw = 0;
  }
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
//-------------------------------------------------------------------------------------------------------------
void ModeWelcome(void)// режим заставки
{
  //char StrN[32];
  
  //char St[5];
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
//        for (int y=0;y<4;++y)
//        {
//          if((ConfigDevice.PlaceLW[y]>0)&&(ConfigDevice.PlaceLW[y]<1310))
//            Mdl|=1;
//          if((ConfigDevice.PlaceLW[y]>1300)||(ConfigDevice.PlaceLW[y]==1064))
//            Mdl|=2;
//        }
    
    sprintf(StrN,"%s",DeviceIDN[ConfigDevice.ID_Device]);
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
    sprintf (St, "v4.%02d%c", NUM_VER/26, (0x61+(NUM_VER%26))); // версия должна быть не ниже 2.01 (28.06.2022)
    sprintf (Str,"t8.txt=\"%s(%X)\"яяя",St,CheckErrMEM); // Version
    NEX_Transmit((void*)Str);//
    // Number
    sprintf (Str,"t9.txt=\"№%d\"яяя",GetNumDevice()); // Number
    NEX_Transmit((void*)Str);//
    ModeReDrawLCD = 1;
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
    CmdInitPage(1);// посылка команды переключения окна на MainMenu и установка признака первого входа
    myBeep(10);
    //ModeDevice = MODEMENU;
    //SetHeadFileRaw (0); // пишем заголовок файла выдачи необработаных данных
    //ADCData++;
    //ClearScreen();
  }   
}
//-------------------------------------------------------------------------------------------------------
void ModeMain(void)// режим основной
{
  static volatile BYTE FrSetMainMenu = 1; // указатель на курсор
  static WORD CntInd = 0;
  //static WORD ProcBatNow = 55;
  static BYTE OnlyBat = 1;
  
  //char Str[32];
  //char StrN[32];
  current_time = RTCGetTime();
  
  //  static long tutu=0;
  //  Rect rct_pic={tutu,0,tutu+128,64};
  //  сначала кнопочки опросим  
  //  if (BUTTON_DOWN(BTN_UP)/*(KeyP & (1<<b_UP)*/)&&(getStateButtons(b_UP)==SHORT_PRESSED))
  if ((PRESS(BTN_UP))&&(getStateButtons(BTN_UP)==SHORT_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1;
    FrSetMainMenu = ChangeFrSet (FrSetMainMenu, 3, 1, MINUS);// установка курсора в рамках заданных параметров
    //ClrKey (BTN_UP);
  }
  if ((PRESS(BTN_DOWN))&&(getStateButtons(BTN_DOWN)==SHORT_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1;
    FrSetMainMenu = ChangeFrSet (FrSetMainMenu, 3, 1, PLUS);// установка курсора в рамках заданных параметров
    //ClrKey (BTN_DOWN);
  }
  if ((CntInd++%10)==0)
  {
    //ProcBatNow=ProcBatInd;
    OnlyBat = 1;
  }
  
  // Выделим блок для выключения если LCD не пользуем
  // Прорисовка нового индикатора
  if(g_FirstScr) // заполнение незменяемых полей
  {
    // наименование прибора
    sprintf (StrN,"t5.txt=\"%s\"яяя",MsgMass[6][CurrLang]); // 
    NEX_Transmit((void*)StrN);//
    // Анализатор
    sprintf(Str,"t1.txt=\"%s\"яяя",MsgMass[7][CurrLang]);
    NEX_Transmit((void*)Str);//
    // память
    sprintf(Str,"t2.txt=\"%s\"яяя",MsgMass[9][CurrLang]);
    NEX_Transmit((void*)Str);//
    // установки
    sprintf(Str,"t3.txt=\"%s\"яяя",MsgMass[10][CurrLang]);
    NEX_Transmit((void*)Str);//
    
    
    ModeReDrawLCD = 0;
    
    g_FirstScr = 0;
    g_NeedScr = 1;
    OnlyBat=1;
  }
  if(g_NeedScr) // перерисовки экрана ! Аккуратно! надо проверить 
  {
    // раскрашивание поля выбора 
    // закрасим бэкграунды  и установим требуемый
    sprintf(Str,"t1.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);// 
    sprintf(Str,"t2.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);// 
    sprintf(Str,"t3.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);// 
    sprintf(Str,"t%d.bco=GREENяяя",FrSetMainMenu); // зеленый
    NEX_Transmit((void*)Str);// 
    //    if(TypeLCD)
    //    sprintf(Str,"qr0.pco=BLACKяяя"); // QR черный
    //    else
    //    sprintf(Str,"qr0.pco=BLUEяяя"); // QR синий
    //      
    //    NEX_Transmit((void*)Str);// 
    
    g_NeedScr=0;
  }
  if (OnlyBat)
  {
    //отображение времени
    //SecOld = current_time.RTC_Sec ; 
    sprintf(Str,"t4.txt=\"%02d:%02d:%02d\"яяя",current_time.RTC_Hour,current_time.RTC_Min,current_time.RTC_Sec);
    NEX_Transmit((void*)Str);//
    // отображение даты
    sprintf (Str,"t7.txt=\"%02d.%02d.%02d\"яяя",current_time.RTC_Mday,current_time.RTC_Mon,current_time.RTC_Year);//,MsgMass[14][CurrLang] убран год 25.12.2013
    NEX_Transmit((void*)Str);//
    // уровень и тип заряда
    if (GETEXTPWR == 0)
    {
      sprintf(Str,"t0.txt=\"%s\"яяя",MsgMass[11][CurrLang]);// внешнее питание
    }
    else
      // от батареек LvlBatInd 
    {
      sprintf(Str,"t0.txt=\"%d%%\"яяя", (int)(LvlBatInd*12.5));
    }
    NEX_Transmit((void*)Str);// 
    OnlyBat=0;
    //g_NeedScr = 1;
  }
  
  if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
  {
    myBeep(10);
    switch (FrSetMainMenu)
    {
    case 1: // Анализатор
      // переход в режим установки параметров рефлектометра из меню или при включении если ТАБЛЕТКА
      SetMode(ModeDrawMeasure);
      // инициализация списка комбинаций установок лазеров
      if(UserSet.ChnMod)
        CmdInitPage(2);// посылка команды переключения окна на Анализатор
      else
        CmdInitPage(3);// посылка команды переключения окна на Анализатор
      //     SetIndexLN(GetIndexLN());//устанавливаем текущие установки по длинам и импульсам
      //     SetMode(ModeSetupOTDR);
      //     SetIndexWAV(GetWAV_SC(GetPlaceLS(CURRENT))); // устанавливаем коэфф. преломления выбранной длины волны
      //
      //     GetSetModeLW(-1); // сбрасываем счетчик длин волн источников
      //     CntLS = 1;
      //     PosCursorMain (-4100); // сброс курсора в начало
      //     IndexVerSize  = 0;// установка вертикального размера отображения рефлектограммы ( самый крупный)
      //     GetSetHorizontScale (5); // сброс масштаба уст самый крупный
      //     if (IndexSmall==0)GetSetHorizontScale (-2);
      //     // устанавливаем для 2 км 
      //     ModeDevice = MODESETREFL;
      //     SSPInit_Any(SPI_ALT); // Инициализация SSP для управления ALTERA (порт 1 та что на плате отладочной)
      //     
      //     PWM_LMOD_Init (OFF); // выключаем LMOD от PWM 0 - OFF, 1- 270Hz 2-2kHz  
      break;
    case 2: // ПАМЯТЬ
      
//    if(ModeUSB) // запрещаем работу с памятью, так как занята 
//    {
//    myBeep(500);
//    }
//    else
//    {
//      SetMode(ModeFileMngDir);
//      CmdInitPage(10);// посылка команды переключения окна на MemoryMenu
      SetMode(ModeSelectMEM);
      CmdInitPage(14);// посылка команды переключения окна на SelectMemoryMenu
//    } 
      break;
    case 3: // УСТАНОВКИ
      SetMode(ModeSetting);
      //123      PWM_LMOD_Init (OFF); // выключаем LMOD от PWM 0 - OFF, 1- 270Hz 2-2kHz  
      //ModeDevice = MODESETUP;
      //123      SSPInit_Any(MEM_FL1); // Инициализация SSP для управления FLASH (порт 1 та что на плате отладочной)
      CmdInitPage(4);// посылка команды переключения окна на Setting
      break;
    }
    ClrKey (BTN_OK);
    //KeyP &=~BTN_OK;

  }  // кнопка "ОК" для данного меню означает переход в другое окно, поэтому ее можно перенести сюда!
  //  
}
//-------------------------------------------------------------------------------------------------------
// окно индикации анализатора, тут будем измерять
void ModeDrawMeasure(void) // режим отображения графического
{
  //char Str[32];
  //LED_START(1);

  static volatile BYTE FrSetIndex = 0; // указатель на курсор
  int Res;
  // получение данных от измерителя
  //GetAllDataMeas((g_IndexMeas)&0xF);
  // Прорисовка нового индикатора
  if(g_FirstScr) // заполнение незменяемых полей
  {
    //    // наименование прибора
    //    sprintf (StrN,"t5.txt=\"%s\"яяя",MsgMass[6][CurrLang]); // 
    //    NEX_Transmit((void*)StrN);//
    //    // Анализатор
    //    sprintf(Str,"t1.txt=\"%s\"яяя",MsgMass[7][CurrLang]);
    //    NEX_Transmit((void*)Str);//
    //    // память
    //    sprintf(Str,"t2.txt=\"%s\"яяя",MsgMass[9][CurrLang]);
    //    NEX_Transmit((void*)Str);//
    //    // установки
    
        g_EnaQuickReDraw =1;
    ModeReDrawLCD = 1;

    rawPressKeyS=0;// если вдруг кто нажимал это до этого
    g_FirstScr = 0;
    g_NeedScr = 1;
  }
  // обработка клавиатуры кнопки Влево Вправо
  if ((PRESS(BTN_LEFT))&&((getStateButtons(BTN_LEFT)==SHORT_PRESSED)||(getStateButtons(BTN_LEFT)==INF_PRESSED)))//
  {
    if(UserSet.ChnMod) // Graph
    {
    myBeep(10);
    if(g_IndexLW>0)g_IndexLW--;
    else g_IndexLW = 17;
    g_NeedScr = 1; // Need reDraw Screen
    }
  }  
  if ((PRESS(BTN_RIGHT))&&((getStateButtons(BTN_RIGHT)==SHORT_PRESSED)||(getStateButtons(BTN_RIGHT)==INF_PRESSED)))//
  {
    if(UserSet.ChnMod) // Graph
    {
    myBeep(10);
    if(g_IndexLW<17)g_IndexLW++;
    else g_IndexLW = 0;
    g_NeedScr = 1; // Need reDraw Screen
    }
  }
// поиск минимального по кнопке вниз и установка курсора
  if ((PRESS(BTN_DOWN))&&(getStateButtons(BTN_DOWN)==SHORT_PRESSED))//
  {
    float Min_F = 100.;
    if(UserSet.ChnMod) // Graph
    {
    myBeep(10);
      for(int i=0;i<18;i++)
      {
        if(CWDMData[i]<=Min_F)
        {
          Min_F = CWDMData[i];
          g_IndexLW = i;
        }
      }
    g_NeedScr = 1; // Need reDraw Screen
    }
  }
// поиск максимального по кнопке вверх и установка курсора
  if ((PRESS(BTN_UP))&&(getStateButtons(BTN_UP)==SHORT_PRESSED))//
  {
    float Max_F = -100.;
    if(UserSet.ChnMod) // Graph
    {
    myBeep(10);
      for(int i=0;i<18;i++)
      {
        if(CWDMData[i]>=Max_F)
        {
          Max_F = CWDMData[i];
          g_IndexLW = i;
        }
      }
    g_NeedScr = 1; // Need reDraw Screen
    }
  }
  
  if(g_NeedScr)
    
  {
    /*
Это в таймере
//line 0,300,340,300,YELLOW
//draw 0,270,340,270,GREEN
//draw 0,240,340,240,YELLOW
//draw 0,210,340,210,GREEN
//draw 0,180,340,180,YELLOW
//draw 0,150,340,150,GREEN
//draw 0,120,340,120,YELLOW
draw 0,90,340,90,GREEN
draw 0,60,340,60,WHITE
draw 0,30,340,30,GREEN
draw 0,0,340,0,YELLOW
xstr 345,290,35,20,3,BLUE,WHITE,0,1,1,"-40"
xstr 345,230,35,20,3,BLUE,WHITE,0,1,1,"-30"
xstr 345,170,35,20,3,BLUE,WHITE,0,1,1,"-20"
xstr 345,110,35,20,3,BLUE,WHITE,0,1,1,"-10"
xstr 345,50,35,20,3,BLUE,WHITE,0,1,1,"0"
xstr 345,20,35,20,3,BLUE,WHITE,0,1,1,"5"

*/
    
    
    // рисуем
//        line 0,300,340,300,YELLOW
//    draw 0,270,340,270,GREEN
//    draw 0,240,340,240,YELLOW
//    draw 0,210,340,210,GREEN
//    draw 0,180,340,180,YELLOW
//    draw 0,150,340,150,GREEN
//    draw 0,120,340,120,YELLOW
//    draw 0,90,340,90,GREEN
//    draw 0,60,340,60,WHITE
//    draw 0,30,340,30,GREEN
//    draw 0,0,340,0,YELLOW
    //xstr 345,290,35,20,3,BLUE,WHITE,0,1,1,"-40"
    //xstr 345,230,35,20,3,BLUE,WHITE,0,1,1,"-30"
    //xstr 345,170,35,20,3,BLUE,WHITE,0,1,1,"-20"
    //xstr 345,110,35,20,3,BLUE,WHITE,0,1,1,"-10"
    //xstr 345,50,35,20,3,BLUE,WHITE,0,1,1,"0"
    //xstr 345,20,35,20,3,BLUE,WHITE,0,1,1,"5"
    //   xstr 380,90,100,55,2,BLACK,WHITE,0,1,1,"-40.2
    if(UserSet.ChnMod) // Graph
    {
      //sprintf(Str,"fill 0,0,341,300,10857яяя"); // fill
      //  NEX_Transmit((void*)Str);//

      for(int i=0;i<18;i++)
      {
        Res=0;
        if(CWDMData[i]>-40.0)
        {
          Res=(int)((CWDMData[i]+40.0)*6);
          if(Res>300) Res=300;
          // если выше -40, то рисуем столбик
        }
        // рассчет Y
        int Y_H = 300 - Res;
        //sprintf(Str,"fill %d,%d,%d,%d,%dяяя",i*19,0,18,Y_H,6371); // fill темный фон
        sprintf(Str,"fill %d,%d,%d,%d,%dяяя",i*19,0,19,Y_H,45901); // fill темный фон
        //sprintf(Str,"fill %d,%d,%d,%d,%dяяя",i*19,0,18,Y_H,10857); // fill темный фон
        //sprintf(Str,"fill %d,%d,%d,%d,%dяяя",i*19,0,18,Y_H,59292); // fill светлый фон
        NEX_Transmit((void*)Str);//
        
        sprintf(Str,"fill %d,%d,%d,%d,%dяяя",i*19,Y_H,19,Res,ColorsPCO[i]); // fill
        //sprintf(Str,"j%d.val=%dяяя",i,Res); // progress bar
        //sprintf(Str,"h%d.val=%dяяя",i,Res); // slider
        NEX_Transmit((void*)Str);//
        //HAL_Delay(25);
//        sprintf(Str,"draw %d,270,%d,270,GREENяяя",i*19, (i+1)*19); //
//        NEX_Transmit((void*)Str);//
//        sprintf(Str,"draw %d,210,%d,210,GREENяяя",i*19, (i+1)*19); //
//        NEX_Transmit((void*)Str);//
//        sprintf(Str,"draw %d,150,%d,150,GREENяяя",i*19, (i+1)*19); //
//        NEX_Transmit((void*)Str);//
//        sprintf(Str,"draw %d,90,%d,90,GREENяяя",i*19, (i+1)*19); //
//        NEX_Transmit((void*)Str);//
//        sprintf(Str,"draw %d,30,%d,30,GREENяяя",i*19, (i+1)*19); //
//        NEX_Transmit((void*)Str);//
//        sprintf(Str,"draw %d,60,%d,60,WHITEяяя",i*19, (i+1)*19); //
//        NEX_Transmit((void*)Str);//
//        sprintf(Str,"draw %d,0,%d,0,YELLOWяяя",i*19, (i+1)*19); //
//        NEX_Transmit((void*)Str);//
//        sprintf(Str,"draw %d,120,%d,120,YELLOWяяя",i*19, (i+1)*19); //
//        NEX_Transmit((void*)Str);//
//        sprintf(Str,"draw %d,180,%d,180,YELLOWяяя",i*19, (i+1)*19); //
//        NEX_Transmit((void*)Str);//
//        sprintf(Str,"draw %d,240,%d,240,YELLOWяяя",i*19, (i+1)*19); //
//        NEX_Transmit((void*)Str);//
//        sprintf(Str,"draw %d,300,%d,300,YELLOWяяя",i*19, (i+1)*19); //
//        NEX_Transmit((void*)Str);//
        
      }
      // попробуем тут сразу сетку нарисовать
//      sprintf(Str,"draw 0,0,340,0,YELLOWяяя"); //
//      NEX_Transmit((void*)Str);//
//      sprintf(Str,"draw 0,60,340,60,WHITEяяя"); //
//      NEX_Transmit((void*)Str);//
//      sprintf(Str,"draw 0,150,340,150,GREENяяя"); //
//      NEX_Transmit((void*)Str);//
//      sprintf(Str,"draw 0,180,340,180,YELLOWяяя"); //
//      NEX_Transmit((void*)Str);//
//      sprintf(Str,"line 0,300,340,300,YELLOWяяя"); //
//      NEX_Transmit((void*)Str);//

      //      sprintf(Str,"tm0.en=1яяя");
      //      NEX_Transmit((void*)Str);//
      sprintf(Str,"t0.txt=\"%d%s\"яяя",1270+g_IndexLW*20,MsgMass[38][CurrLang]);// LW_nm
      NEX_Transmit((void*)Str);//
      sprintf(Str,"t0.bco=%dяяя",ColorsPCO[g_IndexLW]);// LW_nm
      NEX_Transmit((void*)Str);//
      sprintf(Str,"t1.txt=\"%.1f\"яяя",CWDMData[g_IndexLW]);// значение
      NEX_Transmit((void*)Str);//
      sprintf(Str,"t2.txt=\"%s\"яяя",MsgMass[18][CurrLang]);// дБм
      NEX_Transmit((void*)Str);//
      if((g_IndexLW==8)||(g_IndexLW==12))
      {
      sprintf(Str,"t0.pco=65535яяя");// LW_nm
      NEX_Transmit((void*)Str);//
      }
      else
      {
      sprintf(Str,"t0.pco=0яяя");// LW_nm
      NEX_Transmit((void*)Str);//
      }
      //sprintf(Str,"h0.val=%dяяя",g_IndexLW);
      //NEX_Transmit((void*)Str);//
      sprintf(Str,"p1.pic=%dяяя",g_IndexLW+17); //курсор
      NEX_Transmit((void*)Str);//
      //      sprintf(Str,"tm0.en=0яяя");
      //      NEX_Transmit((void*)Str);//
      //sprintf(Str,"line 0,300,340,300,YELLOWяяя"); //курсор
      //NEX_Transmit((void*)Str);//

      
    }
    else // Table
    {
      for(int i=0;i<18;i++)
      {
        //sprintf(Str,"t%d.bco=65535яяя",i+40);// цвет фона
        //NEX_Transmit((void*)Str);//
        
        sprintf(Str,"t%d.txt=\"%.2fdBm\"яяя",i+40,CWDMData[i]); // 
        NEX_Transmit((void*)Str);// 
      }
        //sprintf(Str,"t%d.bco=59292яяя",g_IndexLW+40);// цвет фона на который указывает курсор
        //NEX_Transmit((void*)Str);//
      
    }
    g_NeedScr = 0;
  }
  if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==UP_SHORT_PRESSED)) // возврат в режим просмотра
  {
    myBeep(10);
    if(UserSet.ChnMod) // Graph
    {
      UserSet.ChnMod = 0;
      CmdInitPage(3);// посылка команды переключения окна на Анализатор
    }
    else
    {
      UserSet.ChnMod = 1;
      CmdInitPage(2);// посылка команды переключения окна на Анализатор
    }
    g_FirstScr = 1;
    //g_NeedScr = 1; // Need reDraw Screen
  }
  if ((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))
  {
    SetMode(ModeMain);
    CmdInitPage(1);// посылка команды переключения окна на MainMenu и установка признака первого входа
    g_EnaQuickReDraw =0;
  }
  // вызов сохранения файла (его меню)
  if (rawPressKeyS) // 
  { 
//    if(ModeUSB) // запрещаем запись так как 
//    {
//    myBeep(500);
//    }
//    else
//    {
    myBeep(10);
    //  SaveFileSD(0);
    SetMode(ModeSaverFILE);
    //CreatDelay (30000); // 3.3 мС
    HAL_Delay(3);
    CmdInitPage(7);// посылка команды переключения окна на Меню Сохранения
    g_EnaQuickReDraw =0;
//    }
    rawPressKeyS=0;
  }
  //    HAL_Delay(500);
  //LED_START(0);

  
}
//------------------------------------------------------------------------------------------------------------
void BadBattery(void) // плохая баттарейка CHECK_OFF
{ 
  //char Str[32];
  if(g_FirstScr)
  {
    
    // инициализация не изменяемых величин
    // наименование прибора
    
    sprintf (Str,"t0.txt=\"%s\"яяя",MsgMass[47][UserSet.CurrLang]); // 
    NEX_Transmit((void*)(void*)Str);//
    
    sprintf (Str,"t1.txt=\"%s\"яяя",MsgMass[48][UserSet.CurrLang]); //
    NEX_Transmit((void*)Str);//
    // 
    sprintf (Str,"t2.txt=\"%s\"яяя",MsgMass[51][UserSet.CurrLang]); // 
    NEX_Transmit((void*)Str);//
    // 
    sprintf (Str,"t3.txt=\"%s\"яяя",MsgMass[52][UserSet.CurrLang]); //
    NEX_Transmit((void*)Str);//
    // 
    g_FirstScr = 0;
    //g_NeedScr = 1;
  }
  
  
  if(GETEXTPWR == 0) // подключили внешнее питание возвращаемся в нормальный режим
  {
    BadLevelBat = 0;
    SetMode(ModeMain);
    CmdInitPage(3);// посылка команды переключения окна на MainMenu и установка признака первого входа
    myBeep(100);
    
  }
  
//  while((LevelBat < 60)&&(EXT_POW)) // если подключили внешнее питание то переключимся  в норм режим
//  {
//    if (CntOff++%60 == 0) myBeep(3);
//    // читаем состояние батареи
//    LevelBat = CheckLevelBattery();
//    // ProcBatInd
//    //sprintf(F_name,"BAT = %d%%", LevelBat );
//    sprintf (F_name,"t3.txt=\"BAT = %d%%(%1.2f)\"яяя",ProcBatInd,Ubat); // 
//    NEX_Transmit((void*)F_name);//
//    
//  }
//  CmdInitPage(1);// MainMenu
//  myBeep(25);
  
}

//----------------------------------------------------------------------------------------------------
void ModeSetting(void)// режим установок прибора CHECK_IN
{
  static BYTE FrSetting = 0; // указатель на курсор
  //char Str[64];
    char Stro[32];
  char SetNewWinIfOut = 0; // устнанавливаем признак перхода в другое окно если надо выйти
  //BYTE CurrLang=GetLang(CURRENT);
  //DWORD KeyP = SetBtnStates( KEYS_REG, 1 );
  if ((PRESS(BTN_UP))&&(getStateButtons(BTN_UP)==SHORT_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1; // Need reDraw Screen
    FrSetting = ChangeFrSet (FrSetting, (2), 0, MINUS);// установка курсора в рамках заданных параметров
    //ClrKey (BTN_UP);
  }
  if ((PRESS(BTN_DOWN))&&(getStateButtons(BTN_DOWN)==SHORT_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1; // Need reDraw Screen
    FrSetting = ChangeFrSet (FrSetting, (2), 0, PLUS);// установка курсора в рамках заданных параметров
    //ClrKey (BTN_DOWN);
  }
  switch (FrSetting) // сделаем перестановку полей
  {
  case 1: // Data_Time_Set
    if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      PosCurr = 6;
      SetMode(ModeDateTimeSET);
      //    NeedDrawCRC = 1;
      
      // посылка команды переключения окна на Set_datetime (вызов)  
      // вызовем позже!
      SetNewWinIfOut = 5; // устнанавливаем признак перхода в другое окно если надо выйти
      //CmdInitPage(9);
      //ClrKey (BTN_OK);
    }
    break;
    //  case 4: // BlackLight (1)
    //  if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))
    //  {
    //    myBeep(10);
    //    g_NeedScr = 1; // Need reDraw Screen
    //    //ClrKey (BTN_RIGHT);
    //  }
    //  if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))
    //  {
    //    myBeep(10);
    //    g_NeedScr = 1; // Need reDraw Screen
    //    //ClrKey (BTN_LEFT);
    //  }
    //  break;// BlackLight
  case 0: // Language (2)
    if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))
    {
      myBeep(10);
      g_FirstScr = 1; // Need reDraw Screen
      CurrLang=GetLang(INCR);
      //ClrKey (BTN_RIGHT);
    }
    if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))
    {
      myBeep(10);
      g_FirstScr = 1; // Need reDraw Screen
      CurrLang=GetLang(DECR);
      //ClrKey (BTN_LEFT);
    }
    break;// Language
  case 2: //Файл - параметры файла (3)
    if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
    {
      myBeep(10);
      SetMode(ModeSetupFILE);
      //    NeedDrawCRC = 1;
      // посылка команды переключения окна на Set_OTDRparams (вызов)  
      SetNewWinIfOut = 6; // устнанавливаем признак перхода в другое окно если надо выйти
      //CmdInitPage(6);
      //  StartSettingBegShift (); // старт измерения мертвых зон
      ClrKey (BTN_OK);
    }
    break;// Файл
//  case 3: //память (4)
//    if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))
//    {
//      myBeep(10);
//      g_NeedScr = 1; // Need reDraw Screen
//      //xxx    ChangeUserContr (1); // изменеие пользовательской контрастности
//      //ClrKey (BTN_RIGHT);
//    }
//    if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))
//    {
//      myBeep(10);
//      g_NeedScr = 1; // Need reDraw Screen
//      //xxx    ChangeUserContr (-1); // изменеие пользовательской контрастности
//      //ClrKey (BTN_LEFT);
//    }
//    if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==INF_PRESSED))
//    {
//      myBeep(10);
//      g_NeedScr = 1; // Need reDraw Screen
//      //xxx    ChangeUserContr (1); // изменеие пользовательской контрастности
//      //ClrKey (BTN_RIGHT);
//    }
//    if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==INF_PRESSED))
//    {
//      myBeep(10);
//      g_NeedScr = 1; // Need reDraw Screen
//      //xxx    ChangeUserContr (-1); // изменеие пользовательской контрастности
//      //ClrKey (BTN_LEFT);
//    }
//    //  if (((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==LONG_PRESSED))&&(ChangeUserContr (0)==66))
//    //  {
//    //    myBeep(10);
//    //    SetMode(TetrisGame);
//    //    InitTetris();
//    //  }
//    //  if (((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==LONG_PRESSED))&&(ChangeUserContr (0)==67))
//    //  {
//    //    myBeep(10);
//    //    SetMode(ArcanoidGame);
//    //    InitArcanoid();
//    //  }
//    //  if (((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==LONG_PRESSED))&&(ChangeUserContr (0)==68))
//    //  {
//    //    myBeep(10);
//    //    SetMode(KeyTestGame);
//    //    
//    //  }
//    break;// Contrast
  }

  
  if (g_FirstScr)
  {
    ModeReDrawLCD = 0;
    //HAL_Delay(25);
    // здесь заполняем данными поля нового индикатора
    // не требущие изменения при первичной инициализации
    sprintf(Stro, "t1.txt=\"%s\"яяя", MsgMass[1][CurrLang]);
    NEX_Transmit((void*)Stro);    // Дата / Время
  
  
    sprintf(Stro, "t0.txt=\"Language\"яяя"); //!
    NEX_Transmit((void*)Stro);    // Язык
  
    sprintf(Stro, "t2.txt=\"%s\"яяя", MsgMass[29][CurrLang]); //!
    NEX_Transmit((void*)Stro);    // Файл
  
    //sprintf(Stro, "t3.txt=\"%s\"яяя", MsgMass[9][CurrLang]); //!
    sprintf(Stro, "t3.txt=\"\"яяя"); //!
    NEX_Transmit((void*)Stro);    // Память
  
    g_FirstScr = 0;
    g_NeedScr = 1;
  }
  if (g_NeedScr)
  {
    // здесь заполняем данными поля нового индикатора
    // по результатам изменений вызваныйх обработчиком клавиатуры
    sprintf(Stro, "t6.txt=\"%s\"яяя", MsgMass[0][CurrLang]);
    NEX_Transmit((void*)Stro);    // English
  
    sprintf(Stro, "t7.txt=\"%d\"яяя", 55);
    NEX_Transmit((void*)Stro);    // ???
  
    // раскрашивание поля выбора 
    // закрасим бэкграунды  и установим требуемый
    sprintf(Stro, "t0.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Stro);// 
    sprintf(Stro, "t1.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Stro);// 
    sprintf(Stro, "t2.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Stro);// 
    sprintf(Stro, "t3.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Stro);//
    sprintf(Stro, "t%d.bco=GREENяяя", FrSetting); // зеленый
    NEX_Transmit((void*)Stro);// 
  
  									 // код подсветки требуемой строки если есть есть маркер строки
    g_NeedScr = 0;
  }
    
 if(SetNewWinIfOut) // устнанавливаем признак перхода в другое окно если надо выйти
 {
        // посылка команды переключения окна на Set_datetime (вызов)  
    CmdInitPage(SetNewWinIfOut);
    
 }
    if ((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))
  {
    myBeep(10);
    WriteNeedStruct(0x04);//SaveUserConfig();
    SetMode(ModeMain);
        // посылка команды переключения окна на MainMenu (возврат)  
    CmdInitPage(1);
  }
  
}
//-------------------------------------------------------------------------------------------------
void ModeDateTimeSET(void) // режим установок времени CHECK_IN
{
  //static BYTE PosCurr = 6; // положение курсора устанавливаемых величин
  static RTCTime NowTime;
  //char Str[32];
  //char St[5];
  static DWORD old_sec;
  static DWORD new_sec;
  //BYTE CurrLang=GetLang(CURRENT);
  //DWORD KeyP = SetBtnStates( KEYS_REG, 1 );
  if (PosCurr == 6)  NowTime=RTCGetTime(); // фиксируем время
  new_sec = NowTime.RTC_Sec;
  if(new_sec != old_sec)
  {
   old_sec = new_sec;
   g_NeedScr = 1;
  }

  if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1; // надо перерисовать что то на новом индикаторе
    if (PosCurr>0) PosCurr--;
    else PosCurr=5;
    //ClrKey (BTN_RIGHT);
  }
  if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1; // надо перерисовать что то на новом индикаторе
    if (PosCurr<5) PosCurr++;
    else PosCurr=0;
    //ClrKey (BTN_LEFT);
  }
  // установка величин
  if ((PRESS(BTN_UP))&&((getStateButtons(BTN_UP)==SHORT_PRESSED)||(getStateButtons(BTN_UP)==INF_PRESSED)))
  {
    myBeep(10);
    g_NeedScr = 1; // надо перерисовать что то на новом индикаторе
    switch (PosCurr)
    {
    case 0:// Редактор секунд
      if (NowTime.RTC_Sec<59) NowTime.RTC_Sec++;
      else NowTime.RTC_Sec = 0;
      break;
    case 1:// Редактор минут
      if (NowTime.RTC_Min<59) NowTime.RTC_Min++;
      else NowTime.RTC_Min = 0;
      break;
    case 2:// Редактор часов
      if (NowTime.RTC_Hour<23) NowTime.RTC_Hour++;
      else NowTime.RTC_Hour = 0;
      break;
    case 3:// Редактор дней месяца
      if (NowTime.RTC_Mday<31) NowTime.RTC_Mday++;
      else NowTime.RTC_Mday = 1;
      break;
    case 4:// Редактор  месяца
      if (NowTime.RTC_Mon<12) NowTime.RTC_Mon++;
      else NowTime.RTC_Mon = 1;
      break;
    case 5:// Редактор  года
      if (NowTime.RTC_Year<2050) NowTime.RTC_Year++;
      else NowTime.RTC_Year = 2024;
      break;
    }
    //ClrKey (BTN_UP);
  }
  if ((PRESS(BTN_DOWN))&&((getStateButtons(BTN_DOWN)==SHORT_PRESSED)||(getStateButtons(BTN_DOWN)==INF_PRESSED)))
  {
    myBeep(10);
    g_NeedScr = 1; // надо перерисовать что то на новом индикаторе
    switch (PosCurr)
    {
    case 0:// Редактор секунд
      if (NowTime.RTC_Sec>0) NowTime.RTC_Sec--;
      else NowTime.RTC_Sec = 59;
      break;
    case 1:// Редактор минут
      if (NowTime.RTC_Min>0) NowTime.RTC_Min--;
      else NowTime.RTC_Min = 59;
      break;
    case 2:// Редактор часов
      if (NowTime.RTC_Hour>0) NowTime.RTC_Hour--;
      else NowTime.RTC_Hour = 23;
      break;
    case 3:// Редактор дней месяца
      if (NowTime.RTC_Mday>1) NowTime.RTC_Mday--;
      else NowTime.RTC_Mday = 31;
      break;
    case 4:// Редактор  месяца
      if (NowTime.RTC_Mon>1) NowTime.RTC_Mon--;
      else NowTime.RTC_Mon = 12;
      break;
    case 5:// Редактор  года
      if (NowTime.RTC_Year>2025) NowTime.RTC_Year--;
      else NowTime.RTC_Year = 2050;
      break;
    }
    //ClrKey (BTN_DOWN);
  }
  
  if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1; // надо перерисовать что то на новом индикаторе
    RTCSetTime( NowTime );
    //  if (GetID_Dev()==0)
    //  {
    //  sprintf(Str,"(%04X)", CalkCheckSum()); // 
    //  putString(2,42,Str,1,0);
    //  }
    PosCurr = 7;
    //ClrKey (BTN_OK);
    // 18.10.2011 - индикация CRC при входе в редактор часов
    //    if (NeedDrawCRC)
    //  {
    //
    //  if (GetID_Dev()==0)
    //  {
    //  GetNumVer(St);
    //  }
    //  NeedDrawCRC=0;
    //  }
    
  }
  // Прорисовка нового индикатора
if(g_FirstScr)
{
  // здесь заполняем данными поля нового индикатора
  // не требущие изменения при первичной инициализации
      sprintf(Str,"t0.txt=\"%s\"яяя", MsgMass[2][CurrLang]); // "Дата"" 
    NEX_Transmit((void*)Str);// 
      sprintf(Str,"t1.txt=\"%s\"яяя", MsgMass[15][CurrLang]); // "Время"
    NEX_Transmit((void*)Str);// 
      sprintf(Str,"t2.txt=\"/\"яяя"); // "слэшь"
    NEX_Transmit((void*)Str);// 
      sprintf(Str,"t3.txt=\"/\"яяя"); // "слэшь"
    NEX_Transmit((void*)Str);// 
    sprintf(Str,"t4.txt=\":\"яяя"); // "двоеточие"
    NEX_Transmit((void*)Str);// 
    sprintf(Str,"t5.txt=\":\"яяя"); // "двоеточие"
    NEX_Transmit((void*)Str);// 
    sprintf(Str,"t6.txt=\" \"яяя"); // "пустая строка (пробел)"
    NEX_Transmit((void*)Str);// 
    sprintf(Str,"t7.txt=\" \"яяя"); // "пустая строка (пробел)"
    NEX_Transmit((void*)Str);// 

    ModeReDrawLCD = 0;
  g_FirstScr=0;
  g_NeedScr=1;
}
if(g_NeedScr)
{
   // здесь заполняем данными поля нового индикатора
  // по результатам изменений вызваныйх обработчиком клавиатуры
    sprintf(Str,"n5.val=%dяяя",NowTime.RTC_Year%100); // "Год"
    NEX_Transmit((void*)Str);// 
    sprintf(Str,"n4.val=%dяяя",NowTime.RTC_Mon); // "месяц"
    NEX_Transmit((void*)Str);// 
    sprintf(Str,"n3.val=%dяяя",NowTime.RTC_Mday); // "день"
    NEX_Transmit((void*)Str);// 
    sprintf(Str,"n2.val=%dяяя",NowTime.RTC_Hour); // "часы"
    NEX_Transmit((void*)Str);// 
    sprintf(Str,"n1.val=%dяяя",NowTime.RTC_Min); // "минуты"
    NEX_Transmit((void*)Str);// 
    sprintf(Str,"n0.val=%dяяя",NowTime.RTC_Sec); // "секунды"
    NEX_Transmit((void*)Str);// 
 
    
 
  // код подсветки требуемой строки если есть есть маркер строки
    // закрасим бэкграунды  и установим требуемый
  // забеляем поля редактирования
    sprintf(Str,"n0.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);// 
    sprintf(Str,"n1.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);// 
    sprintf(Str,"n2.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);// 
    sprintf(Str,"n3.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);// 
    sprintf(Str,"n4.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);// 
    sprintf(Str,"n5.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);// 
  if(PosCurr<6)
  {
    sprintf(Str,"n%d.bco=GREENяяя",PosCurr); // зеленый
    NEX_Transmit((void*)Str);// 
  }
  g_NeedScr = 0;
  if(PosCurr==7)
  {
      GetNumVer(St);
    sprintf(Str,"t6.txt=\"v%s (%04X)\"яяя",St, CalkCheckSum()); // "версия и контрольная сумма"
    NEX_Transmit((void*)Str);// 
    sprintf(Str,"t7.txt=\"%s\"яяя", MsgMass[59][CurrLang]); // Up-to DATE"
    NEX_Transmit((void*)Str);// 

  }
}
  
  if ((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))
  {
    myBeep(10);
    SetMode(ModeSetting);
        // посылка команды переключения окна на Settings (возврат)  
    CmdInitPage(4);
  }
}
  // -------------------------------------------------------------------------------------------
  void ModeSetupFILE(void) // режим установок FILES (6 окно)
  {
    static volatile BYTE FrSetSetupFile = 0; // указатель на курсор
    //char Str[64];
    int NeedReSave=0; // признак пересохранения текущих изменений если были
    
    //BYTE CurrLang=GetLang(CURRENT);
    if ((PRESS(BTN_UP))&&(getStateButtons(BTN_UP)==SHORT_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      FrSetSetupFile = ChangeFrSet (FrSetSetupFile, 3, 0, MINUS);// установка курсора в рамках заданных параметров
      //ClrKey (BTN_UP);
    }
    if ((PRESS(BTN_DOWN))&&(getStateButtons(BTN_DOWN)==SHORT_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      FrSetSetupFile = ChangeFrSet (FrSetSetupFile, 3, 0, PLUS);// установка курсора в рамках заданных параметров
      //ClrKey (BTN_DOWN);
    }
    switch (FrSetSetupFile) // обработка выбраных полей установок
    {
    case 0: //Имя кабеля
      // кнопка ОК переключаемся в редактор!
      if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
      {
        g_NeedScr = 1; // Need reDraw Screen
        myBeep(10);
        for (int Ind =ARRAY_SIZE(UserSet.CableID)-2; Ind>=0; Ind--)
        {
          if (UserSet.CableID[Ind]<0x20) UserSet.CableID[Ind]=' ';
          else if (UserSet.CableID[Ind]!=' ' && IndxKBCableID == 0) IndxKBCableID = Ind;
          //Index_Comm --;
        }
        if (UserSet.CableID[IndxKBCableID]!=' ')IndxKBCableID ++;
        //KbPosX = 11;
        //KbPosY = 2;
        SetMode(ModeKBCableID); // редактор имя кабеля
        NeedKeyB = 1; // надо переключится в клавиатуру
        //ClrKey (BTN_OK);
      }
      break; //Имя кабеля
    case 1: //Имя волокна
      // кнопка ОК переключаемся в редактор!
      if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
      {
        g_NeedScr = 1; // Need reDraw Screen
        myBeep(10);
        for (int Ind =ARRAY_SIZE(UserSet.FiberName)-2; Ind>=0; Ind--)
        {
          if (UserSet.FiberName[Ind]<0x20) UserSet.FiberName[Ind]=' ';
          else if (UserSet.FiberName[Ind]!=' ' && IndxKBFiberName == 0) IndxKBFiberName = Ind;
          //Index_Comm --;
        }
        if (UserSet.FiberName[IndxKBCableID]!=' ')IndxKBFiberName ++;
        //KbPosX = 11;
        //KbPosY = 2;
        SetMode(ModeKBFiberName); // редактор имя волокна
        NeedKeyB =1; // надо переключится в клавиатуру
        //ClrKey (BTN_OK);
      }
      break; //Имя волокна
    case 2: //счетчик номера волокна
      if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))
      {
        myBeep(10);
        g_NeedScr = 1; // Need reDraw Screen
        if (UserSet.FiberID<999) UserSet.FiberID++;
        else UserSet.FiberID=1;
        NeedReSave = 1;
        //ClrKey (BTN_RIGHT);
      }
      if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==INF_PRESSED))
      {
        myBeep(10);
        g_NeedScr = 1; // Need reDraw Screen
        if (UserSet.FiberID<989) UserSet.FiberID=UserSet.FiberID+10;
        else UserSet.FiberID=1;
        //ClrKey (BTN_RIGHT);
      }
      if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==INF_PRESSED))
      {
        myBeep(10);
        g_NeedScr = 1; // Need reDraw Screen
        if (UserSet.FiberID>10) UserSet.FiberID=UserSet.FiberID-10;
        else UserSet.FiberID=990;
        //ClrKey (BTN_LEFT);
      }
      if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))
      {
        myBeep(10);
        if (UserSet.FiberID>1) UserSet.FiberID--;
        else UserSet.FiberID=999;
        NeedReSave = 1;
        
        //ClrKey (BTN_LEFT);
      }
      if (((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==UP_INF_PRESSED))||((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==INF_PRESSED)))
      {
        NeedReSave = 1;
      }
      break; //счетчик номера волокна
    case 3: //Разрешение счета волкна
      if (((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))||((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED)))
      {
        myBeep(10);
        if (UserSet.EnaCntFiber) UserSet.EnaCntFiber=0;
        else UserSet.EnaCntFiber=1;
        
        g_NeedScr = 1; // Need reDraw Screen
        NeedReSave = 1;
        
      }
      break; //Разрешение счета волкна
      //    case 4: //Тестовый вызов окна редактирвания (имя Кабеля) 
      //      if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
      //      {
      //        g_NeedScr = 1; // Need reDraw Screen
      //        myBeep(10);
      //        for (int Ind =ARRAY_SIZE(UserSet.CableID)-2; Ind>=0; Ind--)
      //        {
      //          if (UserSet.CableID[Ind]<0x20) UserSet.CableID[Ind]=' ';
      //          else if (UserSet.CableID[Ind]!=' ' && IndxKBCableID == 0) IndxKBCableID = Ind;
      //          //Index_Comm --;
      //        }
      //        if (UserSet.CableID[IndxKBCableID]!=' ')IndxKBCableID ++;
      //        //KbPosX = 11;
      //        //KbPosY = 2;
      //        SetMode(ModeKBCableID); // редактор имя кабеля
      //        NeedKeyB =1; // надо переключится в клавиатуру
      //        //ClrKey (BTN_OK);
      //      }
      //      break; //Коэфф.преломления
    }
    // если не коэфф преломления 
    if (NeedReSave)
    {
      //WriteNeedStruct(0x02);//
      EEPROM_write(&UserSet, ADR_UserMeasConfig, sizeof(UserSet));
      NeedReSave = 0;
    }
    // заполняем новый индикатор если в первый раз в основном не изменяемые поля
    if(g_FirstScr)
    {
      
      // столбцы названий 
      sprintf(Str,"t0.txt=\"%s\"яяя", MsgMass[31][CurrLang]); // Имя кабеля
      NEX_Transmit((void*)Str);// 
      sprintf(Str,"t2.txt=\"%s\"яяя", MsgMass[30][CurrLang]); // Имя волокна 
      NEX_Transmit((void*)Str);// 
      sprintf(Str,"t4.txt=\"%s\"яяя", MsgMass[32][CurrLang]); //№ волокна
      NEX_Transmit((void*)Str);// 
      sprintf(Str,"t6.txt=\"%s\"яяя", MsgMass[33][CurrLang]); //счет волокон
      NEX_Transmit((void*)Str);// 
      //      sprintf(Str,"t8.txt=\"%s\"яяя", MsgMass[34][CurrLang]); //ОК
      //      NEX_Transmit((void*)Str);// 
    ModeReDrawLCD = 0;
      
      g_NeedScr = 1; // для вызова заполнения значений
      g_FirstScr = 0;
    }
    // надо что то изменить в полях установок 
    if(g_NeedScr)
    {
      // первая строка имя Кабеля
      sprintf(Str,"t1.txt=\"%s\"яяя",UserSet.CableID); // 
      NEX_Transmit((void*)Str);// 
      // вторая строка - Имя Волокна
      sprintf(Str,"t3.txt=\"%s\"яяя", UserSet.FiberName);
      NEX_Transmit((void*)Str);// 
      
      // третья сторка - Счетчик волокна
      sprintf(Str,"t5.txt=\"%d\"яяя", UserSet.FiberID );//
      NEX_Transmit((void*)Str);// 
      // четвертая строка в зависимости от положения длины волны
      sprintf(Str,"t7.txt=\"%s\"яяя", (UserSet.EnaCntFiber)?(MsgMass[23][CurrLang]):(MsgMass[24][CurrLang]));//"разов."
      NEX_Transmit((void*)Str);// 
      // пятая строка - индекс измерения или префикс при авто
      //      sprintf(Str,"t9.txt=\"ТЕСТ\"яяя");// надо убрать
      //      NEX_Transmit((void*)Str);// 
      // закрасим бэкграунды  и установим требуемый
      sprintf(Str,"t0.bco=WHITEяяя"); // белый
      NEX_Transmit((void*)Str);// 
      sprintf(Str,"t2.bco=WHITEяяя"); // белый
      NEX_Transmit((void*)Str);// 
      sprintf(Str,"t4.bco=WHITEяяя"); // белый
      NEX_Transmit((void*)Str);// 
      sprintf(Str,"t6.bco=WHITEяяя"); // белый
      NEX_Transmit((void*)Str);// 
      sprintf(Str,"t8.bco=WHITEяяя"); // белый
      NEX_Transmit((void*)Str);//
      sprintf(Str,"t%d.bco=GREENяяя",FrSetSetupFile<<1); // зеленый ?
      NEX_Transmit((void*)Str);// 
      
      //FIO1PIN &=~LEDSTART;//Off  LED
      g_NeedScr = 0;
    }
    // кнопка возврата в меню
    if ((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))
    {
      SetMode(ModeSetting);
      // посылка команды переключения окна на Settings (возврат)  
      CmdInitPage(4);
    }
    if(NeedKeyB ) // необходимость переключения в клавиатуру для редактирования PreFix
    {
      // согласно выбранного языка вызывем клавиатуру
      if (CurrLang) 
        // посылка команды переключения окна на EnglishPreF Keyboard 
        CmdInitPage(12); //(22)
      else
        // посылка команды переключения окна на RussianPreF Keyboard 
        CmdInitPage(13); //(23)
      NeedKeyB=0; 
      NeedWinKBReturn = 0;
    }
  }

// --------------------------------------------------------------------------------------------------------- 
// обработка клавиатуры при редакторе Имени Кабеля
void ModeKBCableID(void) // режим отображения клавиатуры редактора CableID
{
  //char Str[32];
  //char StrI[32];
  //static BYTE Shift = 0; // регистр 
  // тут для нового индикатора
  if (g_FirstScr)
  {
    sprintf(Str, "t0.txt=\"%s\"яяя",MsgMass[31][CurrLang]); 
    NEX_Transmit((void*)Str);    // 

    // здесь заполняем данными поля нового индикатора
    // надо обрезать до последнего пробела....
    memcpy(StrI,UserSet.CableID,IndxKBCableID+1); 
    StrI[IndxKBCableID+1]=0;// No more 11 byte size
    sprintf(Str, "t1.txt=\"%s\"яяя",StrI); 
    NEX_Transmit((void*)Str);    // 1 строка комментарии
    g_GetStr=1; // взведем признак  необходимости прочитать строчку! из индикатора.
    g_FirstScr = 0;
    g_NeedScr = 1;
    ModeReDrawLCD = 0;
  }
  if (g_NeedScr)
  {
    g_NeedScr=0;
  }
  
  if(g_GetStr==2) // УРА! что то приняли назад, можно переписать и сбросить признак
  {
    // здесь обработаем строку на приеме
    IndxKBCableID = GetStringNEX(UserSet.CableID, ARRAY_SIZE(UserSet.CableID));
    //memcpy(CommentsOLT,&RX_BufNEX[1],(CntRXNEX<18)?(CntRXNEX-2):(15)); 
    //CommentsOLT[15]=0;
    for(int i=IndxKBCableID; i< ARRAY_SIZE(UserSet.CableID); ++i) UserSet.CableID[i]=' ';
    UserSet.CableID[ARRAY_SIZE(UserSet.CableID)-1]=0;
    // сохраним
      EEPROM_write(&UserSet, ADR_UserMeasConfig, sizeof(UserSet));
    NeedReturn=1;
  }
    // сохранение по кнопке OK на клавиатуре прибора (точка)
  if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
    // обработка кнопки Ок
  {
    myBeep(7);
    //  sprintf(Str, "click brok,1яяя"); // тест кнопка ок на клавиатуре
    //NEX_Transmit((void*)Str);    //
    StartRecievNEX (10);// время ожидания начала ответов от индикатора
    sprintf(Str, "click bok,1яяя"); // тест кнопка ок на клавиатуре ENGLISH
    NEX_Transmit((void*)Str);    //
    // здесь реально отвечает через не более 2 мС
    
  }

  if (((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))||(NeedReturn))
  {
    myBeep(10);
    if(NeedWinKBReturn)
    {
    SetMode(ModeSaverFILE);  //  переход в режим сохранения ФАЙЛА
    CmdInitPage(7);
    }
    else
    {
    SetMode(ModeSetupFILE);  //  переход в режим установок ФАЙЛА
    CmdInitPage(6);
    }
    NeedWinKBReturn = 0;
    // вернемся в окно
    // важно тут 11 байт и последние пробелы
    g_GetStr=0; // сбросим признак  необходимости прочитать строчку! из индикатора.
    // вернемся в окно установок FILES
    NeedReturn = 0;
  }
}

// --------------------------------------------------------------------------------------------------------- 
// обработка клавиатуры при редакторе Имени Кабеля
void ModeKBFiberName(void) // режим отображения клавиатуры редактора FiberName
{
  //char Str[32];
  //char StrI[32];
  //static BYTE Shift = 0; // регистр 
  // тут для нового индикатора
  if (g_FirstScr)
  {
    sprintf(Str, "t0.txt=\"%s\"яяя",MsgMass[30][CurrLang]); 
    NEX_Transmit((void*)Str);    // 

    // здесь заполняем данными поля нового индикатора
    // надо обрезать до последнего пробела....
    memcpy(StrI,UserSet.FiberName,IndxKBFiberName+1); 
    StrI[IndxKBFiberName+1]=0;// No more 11 byte size
    sprintf(Str, "t1.txt=\"%s\"яяя",StrI); 
    NEX_Transmit((void*)Str);    // 1 строка комментарии
    g_GetStr=1; // взведем признак  необходимости прочитать строчку! из индикатора.
    g_FirstScr = 0;
    g_NeedScr = 1;
    ModeReDrawLCD = 0;
  }
  if (g_NeedScr)
  {
    g_NeedScr=0;
  }
  
  if(g_GetStr==2) // УРА! что то приняли назад, можно переписать и сбросить признак
  {
    // здесь обработаем строку на приеме
    IndxKBFiberName = GetStringNEX(UserSet.FiberName, ARRAY_SIZE(UserSet.FiberName));
    //memcpy(CommentsOLT,&RX_BufNEX[1],(CntRXNEX<18)?(CntRXNEX-2):(15)); 
    //CommentsOLT[15]=0;
    for(int i=IndxKBFiberName; i< ARRAY_SIZE(UserSet.FiberName); ++i) UserSet.FiberName[i]=' ';
    UserSet.FiberName[ARRAY_SIZE(UserSet.FiberName)-1]=0;
    // сохраним
      EEPROM_write(&UserSet, ADR_UserMeasConfig, sizeof(UserSet));
    NeedReturn=1;
  }
    // сохранение по кнопке OK на клавиатуре прибора (точка)
  if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
    // обработка кнопки Ок
  {
    myBeep(7);
    //  sprintf(Str, "click brok,1яяя"); // тест кнопка ок на клавиатуре
    //NEX_Transmit((void*)Str);    //
    StartRecievNEX (10);// время ожидания начала ответов от индикатора
    sprintf(Str, "click bok,1яяя"); // тест кнопка ок на клавиатуре ENGLISH
    NEX_Transmit((void*)Str);    //
    // здесь реально отвечает через не более 2 мС
    
  }
  
  if (((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))||(NeedReturn))
  {
    myBeep(10);
    if(NeedWinKBReturn)
    {
    SetMode(ModeSaverFILE);  //  переход в режим сохранения ФАЙЛА
    CmdInitPage(7);
    }
    else
    {
    SetMode(ModeSetupFILE);  //  переход в режим установок ФАЙЛА
    CmdInitPage(6);
    }
    NeedWinKBReturn = 0;
    // вернемся в окно
    // важно тут 11 байт и последние пробелы
    g_GetStr=0; // сбросим признак  необходимости прочитать строчку! из индикатора.
    // вернемся в окно установок FILES
    NeedReturn = 0;
  }
}

//-----------------------------------------------------------------------------------------------------------------
// обработка клавиатуры при редакторе Comments
void ModeKBComments(void) // режим отображения клавиатуры редактора Comments
{
  //char Str[32];
  //char StrI[32];
  //static BYTE Shift = 0; // регистр 
  // тут для нового индикатора
  if (g_FirstScr)
  {
    sprintf(Str, "t0.txt=\"%s\"яяя",MsgMass[37][CurrLang]); 
    NEX_Transmit((void*)Str);    // 

    // здесь заполняем данными поля нового индикатора
    // надо обрезать до последнего пробела....
    memcpy(StrI,UserSet.Comments,IndxKBComments+1); 
    StrI[IndxKBComments+1]=0;// No more 11 byte size
    sprintf(Str, "t1.txt=\"%s\"яяя",StrI); 
    NEX_Transmit((void*)Str);    // 1 строка комментарии
    g_GetStr=1; // взведем признак  необходимости прочитать строчку! из индикатора.
    g_FirstScr = 0;
    g_NeedScr = 1;
    ModeReDrawLCD = 0;
  }
  if (g_NeedScr)
  {
    g_NeedScr=0;
  }
  
  if(g_GetStr==2) // УРА! что то приняли назад, можно переписать и сбросить признак
  {
    // здесь обработаем строку на приеме
    IndxKBComments = GetStringNEX(UserSet.Comments, ARRAY_SIZE(UserSet.Comments));
    //memcpy(CommentsOLT,&RX_BufNEX[1],(CntRXNEX<18)?(CntRXNEX-2):(15)); 
    //CommentsOLT[15]=0;
    for(int i=IndxKBComments; i< ARRAY_SIZE(UserSet.Comments); ++i) UserSet.Comments[i]=' ';
    UserSet.Comments[ARRAY_SIZE(UserSet.Comments)-1]=0;
    // сохраним
      EEPROM_write(&UserSet, ADR_UserMeasConfig, sizeof(UserSet));
    NeedReturn=1;
  }
  
    // сохранение по кнопке OK на клавиатуре прибора (точка)
  if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
    // обработка кнопки Ок
  {
    myBeep(7);
    //  sprintf(Str, "click brok,1яяя"); // тест кнопка ок на клавиатуре
    //NEX_Transmit((void*)Str);    //
    StartRecievNEX (10);// время ожидания начала ответов от индикатора
    sprintf(Str, "click bok,1яяя"); // тест кнопка ок на клавиатуре ENGLISH
    NEX_Transmit((void*)Str);    //
    // здесь реально отвечает через не более 2 мС
  }
  
  if (((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))||(NeedReturn))
  {
    myBeep(10);
    if(NeedWinKBReturn)
    {
    SetMode(ModeSaverFILE);  //  переход в режим сохранения ФАЙЛА
    CmdInitPage(7);
    }
    else
    {
    SetMode(ModeSetupFILE);  //  переход в режим установок ФАЙЛА
    CmdInitPage(6);
    }
    NeedWinKBReturn = 0;
    // вернемся в окно
    // важно тут 11 байт и последние пробелы
    g_GetStr=0; // сбросим признак  необходимости прочитать строчку! из индикатора.
    // вернемся в окно установок FILES
    NeedReturn = 0;
  }
}

  // -------------------------------------------------------------------------------------------
  void ModeSaverFILE(void) // режим сохранения FILES
  {
    static volatile BYTE FrSetSaverFile = 3; // указатель на курсор
    //char Str[64];
    //char StrF[24];
    int NeedReSave=0; // признак пересохранения текущих изменений если были
    
    //BYTE CurrLang=GetLang(CURRENT);
    if ((PRESS(BTN_UP))&&(getStateButtons(BTN_UP)==SHORT_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      FrSetSaverFile = ChangeFrSet (FrSetSaverFile, 3, 1, MINUS);// установка курсора в рамках заданных параметров
      //ClrKey (BTN_UP);
    }
    if ((PRESS(BTN_DOWN))&&(getStateButtons(BTN_DOWN)==SHORT_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      FrSetSaverFile = ChangeFrSet (FrSetSaverFile, 3, 1, PLUS);// установка курсора в рамках заданных параметров
      //ClrKey (BTN_DOWN);
    }
    switch (FrSetSaverFile) // обработка выбраных полей установок
    {
    case 1: //Имя кабеля
      // кнопка ОК переключаемся в редактор!
      if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
      {
        g_NeedScr = 1; // Need reDraw Screen
        myBeep(10);
        for (int Ind =ARRAY_SIZE(UserSet.CableID)-2; Ind>=0; Ind--)
        {
          if (UserSet.CableID[Ind]<0x20) UserSet.CableID[Ind]=' ';
          else if (UserSet.CableID[Ind]!=' ' && IndxKBCableID == 0) IndxKBCableID = Ind;
          //Index_Comm --;
        }
        if (UserSet.CableID[IndxKBCableID]!=' ')IndxKBCableID ++;
        //KbPosX = 11;
        //KbPosY = 2;
        SetMode(ModeKBCableID); // редактор имя кабеля
        NeedKeyB =1; // надо переключится в клавиатуру
        NeedWinKBReturn = 7;
        //ClrKey (BTN_OK);
      }
      break; //Имя кабеля
    case 2: //Имя волокна
      // кнопка ОК переключаемся в редактор!
      if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
      {
        g_NeedScr = 1; // Need reDraw Screen
        myBeep(10);
        for (int Ind =ARRAY_SIZE(UserSet.FiberName)-2; Ind>=0; Ind--)
        {
          if (UserSet.FiberName[Ind]<0x20) UserSet.FiberName[Ind]=' ';
          else if (UserSet.FiberName[Ind]!=' ' && IndxKBFiberName == 0) IndxKBFiberName = Ind;
          //Index_Comm --;
        }
        if (UserSet.FiberName[IndxKBFiberName]!=' ')IndxKBFiberName ++;
        //KbPosX = 11;
        //KbPosY = 2;
        SetMode(ModeKBFiberName); // редактор имя волокна
        NeedKeyB =1; // надо переключится в клавиатуру
        NeedWinKBReturn = 7;
        //ClrKey (BTN_OK);
      }
      break; //Имя волокна
    case 3: //Комментарий
      // кнопка ОК переключаемся в редактор!
      if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
      {
        g_NeedScr = 1; // Need reDraw Screen
        myBeep(10);
        for (int Ind =ARRAY_SIZE(UserSet.Comments)-2; Ind>=0; Ind--)
        {
          if (UserSet.Comments[Ind]<0x20) UserSet.Comments[Ind]=' ';
          else if (UserSet.Comments[Ind]!=' ' && IndxKBComments == 0) IndxKBComments = Ind;
          //Index_Comm --;
        }
        if (UserSet.Comments[IndxKBComments]!=' ')IndxKBComments ++;
        //KbPosX = 11;
        //KbPosY = 2;
        SetMode(ModeKBComments); // редактор имя волокна
        NeedKeyB =1; // надо переключится в клавиатуру
        //ClrKey (BTN_OK);
      }
    }
    //  
    if (NeedReSave)
    {
      //WriteNeedStruct(0x02);//
      EEPROM_write(&UserSet, ADR_UserMeasConfig, sizeof(UserSet));
      NeedReSave = 0;
    }
    // заполняем новый индикатор если в первый раз в основном не изменяемые поля
    if(g_FirstScr)
    {
        TimeSaveOTDR = RTCGetTime(); // сохраняем время сохранения

      // столбцы названий 
      sprintf(Str,"t0.txt=\"%s\"яяя", MsgMass[29][CurrLang]); // Имя Файла
      NEX_Transmit((void*)Str);// 
      sprintf(Str,"t2.txt=\"%s\"яяя", MsgMass[35][CurrLang]); // Кабель 
      NEX_Transmit((void*)Str);// 
      sprintf(Str,"t4.txt=\"%s\"яяя", MsgMass[36][CurrLang]); //Волокно
      NEX_Transmit((void*)Str);// 
      sprintf(Str,"t6.txt=\"%s\"яяя", MsgMass[37][CurrLang]); //счет волокон
      NEX_Transmit((void*)Str);// 
      //      sprintf(Str,"t8.txt=\"%s\"яяя", MsgMass[34][CurrLang]); //ОК
      //      NEX_Transmit((void*)Str);// 
      
      g_NeedScr = 1; // для вызова заполнения значений
      g_FirstScr = 0;
    ModeReDrawLCD = 0;
    }
    // надо что то изменить в полях установок 
    if(g_NeedScr)
    {
      //первая имя ФАЙЛА
      // имя файла из времени
            sprintf(StrF,"%02d%02d%02d_%02d%02d%02d.t8k",TimeSaveOTDR.RTC_Year%100,
          TimeSaveOTDR.RTC_Mon,
          TimeSaveOTDR.RTC_Mday,
          TimeSaveOTDR.RTC_Hour,
          TimeSaveOTDR.RTC_Min,
          TimeSaveOTDR.RTC_Sec );

      //sprintf(Str,"t1.txt=\"T8K%04d\"яяя",UserSet.FileNumber); // 
      sprintf(Str,"t1.txt=\"%s\"яяя",StrF); // 
      NEX_Transmit((void*)Str);// 
      
      // вторая строка имя Кабеля
      sprintf(Str,"t3.txt=\"%s\"яяя",UserSet.CableID); // 
      NEX_Transmit((void*)Str);// 
      //  третья строка - Имя Волокна
      sprintf(Str,"t5.txt=\"%s\"яяя", UserSet.FiberName);
      NEX_Transmit((void*)Str);// 
      
      // четвертая сторка - Comments
      sprintf(Str,"t7.txt=\"%s\"яяя", UserSet.Comments );//
      NEX_Transmit((void*)Str);// 
      // пятая строка Comments
      sprintf(Str,"t8.txt=\"%s\"яяя", (UserSet.Comments));//
      NEX_Transmit((void*)Str);// 
      //  строка - индекс измерения или префикс при авто
      //      sprintf(Str,"t9.txt=\"ТЕСТ\"яяя");// надо убрать
      //      NEX_Transmit((void*)Str);// 
      // закрасим бэкграунды  и установим требуемый
      sprintf(Str,"t0.bco=WHITEяяя"); // белый
      NEX_Transmit((void*)Str);// 
      sprintf(Str,"t2.bco=WHITEяяя"); // белый
      NEX_Transmit((void*)Str);// 
      sprintf(Str,"t4.bco=WHITEяяя"); // белый
      NEX_Transmit((void*)Str);// 
      sprintf(Str,"t6.bco=WHITEяяя"); // белый
      NEX_Transmit((void*)Str);// 
      sprintf(Str,"t8.bco=WHITEяяя"); // белый
      NEX_Transmit((void*)Str);//
      sprintf(Str,"t%d.bco=GREENяяя",FrSetSaverFile<<1); // зеленый ?
      NEX_Transmit((void*)Str);// 
      
      //FIO1PIN &=~LEDSTART;//Off  LED
      g_NeedScr = 0;
    }
    // кнопка возврата в меню
    if ((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))
    {
      // возврат в режим АНАЛИЗАТОР
      SetMode(ModeDrawMeasure);
      // инициализация списка комбинаций установок лазеров
      if(UserSet.ChnMod)
        CmdInitPage(2);// посылка команды переключения окна на Анализатор
      else
        CmdInitPage(3);// посылка команды переключения окна на Анализатор
    }
    if(NeedKeyB ) // необходимость переключения в клавиатуру для редактирования PreFix
    {
      // согласно выбранного языка вызывем клавиатуру
      if (CurrLang) 
        // посылка команды переключения окна на EnglishPreF Keyboard 
        CmdInitPage(12); //(22)
      else
        // посылка команды переключения окна на RussianPreF Keyboard 
        CmdInitPage(13); //(23)
      NeedWinKBReturn = 1; // для возврата в это же окно
      
      NeedKeyB=0; 
    }
    // кнопка "S"
    // вызов сохранения файла (его меню)
    if (rawPressKeyS) // START Measure из режима установок рефлектометра
    {        
//    if(ModeUSB) // запрещаем работу с памятью, так как занята 
//    {
//    myBeep(500);
//    }
//    else
//    {
      myBeep(100);
      SaveFileSD(1);
      sprintf(Str,"xstr 80,145,350,60,2,BLACK,RED,0,1,1,\"%s\"яяя",MsgMass[60][CurrLang]); // зеленый ?
      NEX_Transmit((void*)Str);//
      // здесь сохраняем файл, затем меняем счетчик
      if(UserSet.FileNumber<999)UserSet.FileNumber++;
      else UserSet.FileNumber=1;
    // сохраним
      EEPROM_write(&UserSet, ADR_UserMeasConfig, sizeof(UserSet));
      //
      HAL_Delay(500);
      // возврат в режим АНАЛИЗАТОР
      SetMode(ModeDrawMeasure);
      // инициализация списка комбинаций установок лазеров
      if(UserSet.ChnMod)
        CmdInitPage(2);// посылка команды переключения окна на Анализатор
      else
        CmdInitPage(3);// посылка команды переключения окна на Анализатор
      //CreatDelay (30000); // 3.3 мС
//    }
      rawPressKeyS=0;
    }
    //    HAL_Delay(500);
    
    
  }
//----------------------------------------------------------------------------------
void ModeSelectMEM(void) // режим выбора работы с памятью CHECK_OFF (окно 14)
{
  static volatile BYTE FrSelectMEM = 1; // указатель на курсор
  char Str[32];
  //BYTE CurrLang=GetLang(CURRENT);
  //DWORD KeyP = SetBtnStates( KEYS_REG, 1 );
  if ((PRESS(BTN_UP))&&(getStateButtons(BTN_UP)==SHORT_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1; // Need reDraw Screen
    FrSelectMEM = ChangeFrSet (FrSelectMEM, 2, 1, MINUS);// установка курсора в рамках заданных параметров
    //ClrKey (BTN_UP);
  }
  if ((PRESS(BTN_DOWN))&&(getStateButtons(BTN_DOWN)==SHORT_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1; // Need reDraw Screen
    FrSelectMEM = ChangeFrSet (FrSelectMEM, 2, 1, PLUS);// установка курсора в рамках заданных параметров
    //ClrKey (BTN_DOWN);
  }
  
  
  if (g_FirstScr)
  {
    // здесь заполняем данными поля нового индикатора
    // не требущие изменения при первичной инициализации
    // Здесь практичски все поля 
    sprintf(Str, "t0.txt=\"%s :\"яяя", MsgMass[9][CurrLang]);
    NEX_Transmit((void*)Str);    // Память
    
    //sprintf(Str, "t1.txt=\"%s\"яяя", MsgMass[43][CurrLang]);
    //NEX_Transmit((void*)Str);    // свободно
    
    sprintf(Str, "t1.txt=\"%s\"яяя", MsgMass[68][CurrLang]);
    NEX_Transmit((void*)Str);    // внутренняя (SD Card)
    
    //sprintf(Str, "t3.txt=\"%4d\"яяя", MAXMEMALL-GetNumTraceSaved(0));
    //NEX_Transmit((void*)Str);    // сколько свободно
      sprintf(Str, "t2.txt=\"USB\"яяя");
      NEX_Transmit((void*)Str);    // USB 
    
    
    g_FirstScr = 0;
    g_NeedScr = 1;
  }
  if (g_NeedScr)
  {
    // здесь заполняем данными поля нового индикатора
    // по результатам изменений вызваныйх обработчиком клавиатуры
    
    // раскрашивание поля выбора 
    // закрасим бэкграунды  и установим требуемый
    sprintf(Str, "t1.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);//
    HAL_Delay(5);
    sprintf(Str, "t2.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);// 
    HAL_Delay(5);
    sprintf(Str, "t%d.bco=GREENяяя", FrSelectMEM); // зеленый
    NEX_Transmit((void*)Str);// 
    HAL_Delay(5);
    // код подсветки требуемой строки если есть есть маркер строки
    g_NeedScr = 0;
  }
  
  if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
  {
    switch (FrSelectMEM) // выбор по кнопке "ОК"
    {
    case 1: // переход в память SD Card
      myBeep(10);
      
      //SetMode(ModeMemoryOTDR);
      SetMode(ModeFileMngDir);
      // посылка команды переключения окна на Mem_OTDR_garaph (вызов)  
      ClrKey(BTN_OK);
      CmdInitPage(10); // новое окно лист бокс перечня директорий
      //CreatDelay(1000000);
      HAL_Delay(100);
      
      
      break;
    case 2: // переход в режим работы с памятью по USB
        myBeep(10);
        SetMode(ModeReadUSB);
        //FrClearMEM = 2 + PowerMeter;
        // посылка команды переключения окна на Select_MEM_Clr(вызов)  
        MSC_or_CDC = 1; // признак активности MSC для инициализации разрешим, и как только сразу запретимпо умолчанию запрещно
        MemMsgModeUSB = 2; // так как первый вход, карточка не подключена
        ClrKey(BTN_OK);
        CmdInitPage(15);
        //NeedReturn = 4; // что бы вернутся сюда же
      break;
    }
    
  }
  if ((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))
  {
    myBeep(10);
    SetMode(ModeMain);
    //ModeDevice = MODEMENU;
    // посылка команды переключения окна на MainMenu (возврат)  
    CmdInitPage(1);
    
  }
}
//---------------------------------------------------------------------------------------
void ModeReadUSB(void) // режим чтения по USB памяти флэшки установка признака (окно 15)
{
  char Str[32];
  
  if (g_FirstScr)
  {
    // здесь заполняем данными поля нового индикатора
    // не требущие изменения при первичной инициализации   
    // сообщение о том что надо переподключить провод USB
    sprintf(Str, "t0.txt=\"%s\"яяя", MsgMass[62][CurrLang]); 
    NEX_Transmit((void*)Str);    // Переподключите
    
    sprintf(Str, "t1.txt=\"%s\"яяя", MsgMass[63][CurrLang]);
    NEX_Transmit((void*)Str);    // кабель USB
    
    sprintf(Str, "t2.txt=\"%s\"яяя", MsgMass[64][CurrLang]);
    NEX_Transmit((void*)Str);    // для чтения     
    
    sprintf(Str, "t3.txt=\"%s\"яяя", MsgMass[65][CurrLang]); 
    NEX_Transmit((void*)Str);    // карты памяти

    if(g_CardSD) // признак подключенной карты для правильной индикации
    MemMsgModeUSB = 1; // так как карточка подключена, но идет перерисовка то нужно зеленое
 
    //MemMsgModeUSB = 2; // так как первый вход, карточка не подключена
    g_FirstScr = 0;
    g_NeedScr = 1;
  }

  // когда переподключили кабель и прочитали флэшку
  if(MemMsgModeUSB)
  {
// признак работы USB для индикации доп строчки
    // изменить цвет и надпись однократно
    if(MemMsgModeUSB == 1) // зеленый
    {
    sprintf(Str, "t4.txt=\"%s\"яяя", MsgMass[67][CurrLang]); 
    NEX_Transmit((void*)Str);    // отключено
    sprintf(Str, "t4.bco=GREENяяя"); // зеленый
    NEX_Transmit((void*)Str);// 
    g_CardSD = 1; // признак подключенной карты для правильной индикации
    }
    else
    {
    sprintf(Str, "t4.txt=\"%s\"яяя", MsgMass[66][CurrLang]); 
    NEX_Transmit((void*)Str);    // отключено
    sprintf(Str, "t4.bco=64800яяя"); // оранжевый
    NEX_Transmit((void*)Str);// 
    g_CardSD = 0; // признак подключенной карты для правильной индикации
    }
    MemMsgModeUSB = 0;
  }
  
  if (g_NeedScr)
  {
    // здесь заполняем данными поля нового индикатора
    // по результатам изменений вызваныйх обработчиком клавиатуры
    
    // раскрашивание поля выбора 
    // закрасим бэкграунды  и установим требуемый
    sprintf(Str, "t1.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);// 
    sprintf(Str, "t2.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);// 
    sprintf(Str, "t3.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);//
    
    // код подсветки требуемой строки если есть есть маркер строки
    g_NeedScr = 0;
    MSC_or_CDC = 1;
  }
  
  if (((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))||(NeedReturn))
  {
    myBeep(10);
    g_NeedScr = 1; // Need reDraw Screen
    if(!NeedReturn)
    {
    SetMode(ModeSelectMEM);
      NeedReturn = 14;
    }
    //MX_USB_DEVICE_Init();

    // посылка команды переключения окна на Memory (возврат)  
    CmdInitPage(NeedReturn);
    NeedReturn = 0;
    MSC_or_CDC = 0;
    g_CardSD = 0; // сбросим признак подключенной карты для правильной индикации

    //ModeDevice = MODEMENU;
  }
}

//----------------------------------------------------------------------------------
// вызываем чтение SD Card для поиска директорий, составляем список
// "правильных" директорий, устанавливаем курсоры если они не изменились,
// если не совпадают с полученными размерами сбрасываем в начало
// вызываем ОКНО 10
void ModeFileMngDir(void) // режим файл менеджера директорий
{
  //char Str[32];
  int Nweek;
  if ((PRESS(BTN_UP))&&(getStateButtons(BTN_UP)==SHORT_PRESSED)) 
  {
    myBeep(10);
    if(IndexNameDir>0)IndexNameDir--;
    g_NeedScr=1;
  }
  if ((PRESS(BTN_DOWN))&&(getStateButtons(BTN_DOWN)==SHORT_PRESSED))
  {
    myBeep(10);
    if((IndexNameDir+1)<NumNameDir)IndexNameDir++;
    g_NeedScr=1;
  }
  if (g_FirstScr)
  {
    SDMMC_SDCard_DIR(); // прочитаем дирректроии
    // здесь заполняем данными поля нового индикатора
    // не требущие изменения при первичной инициализации
    sprintf(Str, "t0.txt=\"0:/_T8KN\"яяя"); // < событиe >
    NEX_Transmit((void*)Str);    //
    sprintf(Str, "t14.txt=\"%d\"яяя", NumNameDir); // < сколько папок нашли >
    NEX_Transmit((void*)Str);    //
    
    ModeReDrawLCD = 0;
    
    g_FirstScr = 0;
    g_NeedScr = 1;
  }
  if (g_NeedScr)
  {
    sprintf(Str, "t13.txt=\"%d\"яяя", IndexNameDir+1); // < какая папка выбрана >
    NEX_Transmit((void*)Str);    //
    if (IndexNameDir > NumNameDir) IndexNameDir = 0; 
    // тут нужен сложный подсчет указателя на папки в индикации
    // из выполнения условий текущий индекс папки должен быть меньше
    // числа паПок,и индикационный тндекс должен устанавливаться в соответствии с 
    // текущим индексом выбранной папки
    IndexLCDNameDir = IndexNameDir%12; // как как у нас 12 полей
    PageDir = IndexNameDir/12; // получим страницу перечня директорий котрую нужно отображать
    // заполним поля индикатора именами директорий
    for (int i=0; i<12; i++)
    {
      Nweek=0;
      sprintf(Str, "t%d.txt=\"%s\"яяя",i+1 ,NameDir[PageDir*12+i]); // < событиe >
      NEX_Transmit((void*)Str);    //
      // можно получить строчку дат объявленной папки
      Nweek = atoi(&NameDir[PageDir*12+i][3]);
//      if(Nweek)
//      {
//      sprintf(Str, "t%d.txt=\"%s\"яяя",i+15 ,MsgWeek[Nweek-1]); // < событиe >
//      NEX_Transmit((void*)Str);    //
//      }
    }
    for (int i=0; i<12; i++)
    {
      // закрасим бэкграунды  и установим требуемый
      sprintf(Str,"t%d.bco=WHITEяяя",i+1); // белый
      NEX_Transmit((void*)Str);// 
    }
    sprintf(Str,"t%d.bco=GREENяяя",IndexLCDNameDir+1); // GREEN
    NEX_Transmit((void*)Str);    //
    // код подсветки требуемой строки если есть есть маркер строки
    g_NeedScr = 0;
  }
  // обработка кнопки "OK"
  if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==UP_SHORT_PRESSED)) // переход в режим просмотра с переключением зума
  {
//    if(ModeUSB) // запрещаем работу с памятью, так как занята 
//    {
//    myBeep(500);
//    }
//    else
//    {
    
    myBeep(10);
      //SetMode(ModeFileMngFiles);
      SetMode(ModeFileMngDirD);

      IndexNameFiles=0;// индекс файла на который указываем
      //ModeDevice = MODEMEMR;
      //ModeMemDraw = VIEWNEXT;
      //ReturnMemView = 1; // надо вернуться сюда же по ESC
         // посылка команды переключения окна на Mem_OTDR_garaph (вызов)  
      //KeyP = 0;
      CmdInitPage(10); // новое окно лист бокс перечня файлов в текущей дирректории
       //CreatDelay(1000000);
//    }
      ClrKey(BTN_OK);
      HAL_Delay(100);
  }

  if ((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))
  {
    // здесь над витвится в зависимости от признака откуда пришли
    //if(ReturnMemView)
    //{
      //DeleteTrace = 0;
      SetMode(ModeMain);
      //ModeDevice = MODEOTHER;
      myBeep(10);
      // посылка команды переключения окна на MainMenu (возврат)  
      CmdInitPage(1);
    //}
  }
}
//--------------------------------------------------------------------------------
// второй уровень подпапок по дате
// папка года_месяц и папка числа введем переменную ???LevelDir - определяющую что рисовать и откуда

void ModeFileMngDirD(void) // режим файл менеджера директорий
{
  char Str[32];
  
  if ((PRESS(BTN_UP))&&(getStateButtons(BTN_UP)==SHORT_PRESSED)) 
  {
    myBeep(10);
    if(IndexNameDirD>0)IndexNameDirD--;
    g_NeedScr=1;
  }
  if ((PRESS(BTN_DOWN))&&(getStateButtons(BTN_DOWN)==SHORT_PRESSED))
  {
    myBeep(10);
    if((IndexNameDirD+1)<NumNameDirD)IndexNameDirD++;
    g_NeedScr=1;
  }
  if (g_FirstScr)
  {
    SDMMC_SDCard_DIRD(); // прочитаем дирректории
    // здесь заполняем данными поля нового индикатора
    // не требущие изменения при первичной инициализации
    sprintf(Str, "t0.txt=\"0:/_T8KN/%s\"яяя",NameDir[IndexNameDir]); // < событиe >
    NEX_Transmit((void*)Str);    //
    sprintf(Str, "t14.txt=\"%d\"яяя", NumNameDirD); // < сколько папок нашли >
    NEX_Transmit((void*)Str);    //
    
    
    g_FirstScr = 0;
    g_NeedScr = 1;
  }
  if (g_NeedScr)
  {
    sprintf(Str, "t13.txt=\"%d\"яяя", IndexNameDirD+1); // < какая папка выбрана >
    NEX_Transmit((void*)Str);    //
    if (IndexNameDirD > NumNameDirD) IndexNameDirD = 0; 
    // тут нужен сложный подсчет указателя на папки в индикации
    // из выполнения условий текущий индекс папки должен быть меньше
    // числа паПок,и индикационный тндекс должен устанавливаться в соответствии с 
    // текущим индексом выбранной папки
    IndexLCDNameDirD = IndexNameDirD%12; // как как у нас 12 полей
    PageDirD = IndexNameDirD/12; // получим страницу перечня директорий котрую нужно отображать
    // заполним поля индикатора именами директорий
    for (int i=0; i<12; i++)
    {
      
      sprintf(Str, "t%d.txt=\"%s\"яяя",i+1 ,NameDirD[PageDirD*12+i]); // < событиe >
      NEX_Transmit((void*)Str);    //
      
    }
    for (int i=0; i<12; i++)
    {
      // закрасим бэкграунды  и установим требуемый
      sprintf(Str,"t%d.bco=WHITEяяя",i+1); // белый
      NEX_Transmit((void*)Str);// 
    }
    sprintf(Str,"t%d.bco=GREENяяя",IndexLCDNameDirD+1); // GREEN
    NEX_Transmit((void*)Str);    //
    // код подсветки требуемой строки если есть есть маркер строки
    g_NeedScr = 0;
  }
  // обработка кнопки "OK"
  if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==UP_SHORT_PRESSED)) // переход в режим просмотра с переключением зума
  {
    // вызов окна выбора файлов при просмотре памяти
    myBeep(10);
      SetMode(ModeFileMngFiles);
//      ReturnMemView = 1; // надо вернуться сюда же по ESC
         // посылка команды переключения окна на Mem_OTDR_garaph (вызов)  
      //KeyP = 0;
      ClrKey(BTN_OK);
      CmdInitPage(11); // новое окно лист бокс перечня файлов в текущей дирректории
       //CreatDelay(1000000);
      HAL_Delay(100);
  }

  if ((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))
  {
    // здесь над витвится в зависимости от признака откуда пришли
//    if(ReturnMemView)
//    {
      //DeleteTrace = 0;
      SetMode(ModeFileMngDir);
      //ModeDevice = MODEOTHER;
      myBeep(10);
      // посылка команды переключения окна на Memory (возврат)  
      CmdInitPage(10);
//    }
  }
}

//--------------------------------------------------------------------------------
// вызываем чтение SD Card для поиска файлов , составляем список
// "правильных" файлов, устанавливаем курсоры если они не изменились,
// если не совпадают с полученными размерами сбрасываем в начало
// вызываем ОКНО 11
void ModeFileMngFiles(void) // режим файл менеджера файлов (Окно 34)
{
  // таблица экрана поля ввода, верхняя строка t0, левая колонка t1-t12
  //        t0
  // t13  t14   t15
  //            t16
  // t20        t17  
  // t21        t18
  //        t19
  char StrR[64];
  char FilPath[64];
//  uint32_t BlkSz; // размер блока заголовка
//  uint32_t EvntSz=0; // размер блока событий, пока не читаем события просто для смещения
//  uint32_t PosDataLog=0xe1; // позиция начала блока данных для копирования в 
  // на начало данных
//  UINT RWC;
  FATFS FatFs;
  FIL Fil;
  FRESULT FR_Status;
  
  if ((PRESS(BTN_UP))&&(getStateButtons(BTN_UP)==SHORT_PRESSED)) 
  { 
//    if(ModeUSB) // запрещаем работу с памятью, так как занята 
//    {
//      myBeep(500);
//    }
//    else
//    {
      myBeep(10);
      if(IndexNameFiles>0)IndexNameFiles--;
      g_NeedScr=1;
//    }
  }
  if ((PRESS(BTN_DOWN))&&(getStateButtons(BTN_DOWN)==SHORT_PRESSED))
  {
//    if(ModeUSB) // запрещаем работу с памятью, так как занята 
//    {
//      myBeep(500);
//    }
//    else
//    {
      myBeep(10);
      if((IndexNameFiles+1)<NumNameFiles)IndexNameFiles++;
      g_NeedScr=1;
//    }
  }
  if (g_FirstScr)
  {
    SDMMC_SDCard_FILES(); // прочитаем файлы, 
    // здесь заполняем данными поля нового индикатора
    // не требущие изменения при первичной инициализации
    sprintf(Str, "t0.txt=\"0:/_T8KN/%s\"яяя",NameDir[IndexNameDir]); // < событиe >
    NEX_Transmit((void*)Str);    //
    HAL_Delay(1);
    sprintf(Str, "t14.txt=\"%d\"яяя", NumNameFiles); // < сколько файлов нашли >
    NEX_Transmit((void*)Str);    //
    
    ModeReDrawLCD = 0;
    
    g_FirstScr = 0;
    g_NeedScr = 1;
  }
  if (g_NeedScr)
  {
    sprintf(Str, "t13.txt=\"%d\"яяя", IndexNameFiles+1); // < какой файл выбран >
    NEX_Transmit((void*)Str);    //
    if (IndexNameFiles > NumNameFiles) IndexNameFiles = 0; 
    // тут нужен сложный подсчет указателя на папки в индикации
    // из выполнения условий текущий индекс папки должен быть меньше
    // числа паПок,и индикационный тндекс должен устанавливаться в соответствии с 
    // текущим индексом выбранной папки
    IndexLCDNameFiles = IndexNameFiles%12; // как как у нас 12 полей
    PageFiles = IndexNameFiles/12; // получим страницу перечня файлов который нужно отображать
    // заполним поля индикатора именами файлов
    for (int i=0; i<12; i++)
    {
      
      sprintf(Str, "t%d.txt=\"%s\"яяя",i+1 ,NameFiles[PageFiles*12+i]); // < имена файлов >
      NEX_Transmit((void*)Str);    //
      
    }
    FR_Status = f_mount(&FatFs, SDPath, 1);
    
    // здесь можно прочитать файл на котрый указываем и разобрать его
    // перепишем егов структуру что бы второй раз не читаить
    sprintf(FilPath, "0:/_T8KN/%s/%s/%s",NameDir[IndexNameDir],NameDirD[IndexNameDirD],NameFiles[IndexNameFiles]); // путь к файлу
    // откроем файл и прочитаем размер блока
    FR_Status = f_open(&Fil, FilPath, FA_READ);
    if(FR_Status == FR_OK)
    {
      int i, j;
      int len, Res;
      float fDt; 
      f_gets(StrR, 64,&Fil); // read 1 string (DATA)
      // разбор строки, 
      len = strlen(StrR);
      for(i=0;i<len;i++)
      {
        if(StrR[i]==';') break;
      }
      i++;
      i++;
      StrR[len-2]=0;
      sprintf(MemD.DateMem, "%s", &StrR[i]); // < дата >
      
      f_gets(StrR, 64,&Fil); // read 2 string (Time)
      // разбор строки, 
      len = strlen(StrR);
      for(i=0;i<len;i++)
      {
        if(StrR[i]==';') break;
      }
      i++;
      i++;
      StrR[len-2]=0;
      sprintf(MemD.TimeMem, "t16.txt=\"%s\"яяя", &StrR[i]); // < время >

      f_gets(StrR, 64,&Fil); // read 3 string (name, version, Serial number)
      f_gets(StrR, 64,&Fil); // read 4 string (CableID)
      // разбор строки, 
      len = strlen(StrR);
      for(i=0;i<len;i++)
      {
        if(StrR[i]==';') break;
      }
      i++;
      i++;
      StrR[len-2]=0;
      sprintf(MemD.CableID, "%s", &StrR[i]); // < имя кабеля >
      sprintf(Str, "t16.txt=\"%s\"яяя", &StrR[i]); // < имя кабеля >
      NEX_Transmit((void*)Str);    //
      
      f_gets(StrR, 64,&Fil); // read 5 string (Fiber Name)
      // разбор строки, 
      len = strlen(StrR);
      for(i=0;i<len;i++)
      {
        if(StrR[i]==';') break;
      }
      i++;
      i++;
      StrR[len-2]=0;
      sprintf(MemD.FiberName, "%s", &StrR[i]); // < имя волокна >
       sprintf(Str, "t17.txt=\"%s\"яяя", &StrR[i]); // < имя волокна >
       NEX_Transmit((void*)Str);    //
      f_gets(StrR, 64,&Fil); // read 6 string (Comments)
      // разбор строки, 
      len = strlen(StrR);
      for(i=0;i<len;i++)
      {
        if(StrR[i]==';') break;
      }
      i++;
      i++;
      StrR[len-2]=0;
      sprintf(MemD.Comments, "%s", &StrR[i]); // < комметарий >
      sprintf(Str, "t18.txt=\"%s\"яяя", &StrR[i]); // < комметарий >
       NEX_Transmit((void*)Str);    //

      f_gets(StrR, 64,&Fil); // read 7 string (Шапка данных)
       
for(j=0; j<18; j++)
{
        f_gets(StrR, 64,&Fil); // read 8..25 string (Данные сохраненные)
        fDt = atof(&StrR[5]);
        if((fDt<-70.)&&(fDt>25.)) fDt = -70.;
        MemD.CWDMDataMem[j]= fDt;
       // заполним картинку
        Res=0;
        if(MemD.CWDMDataMem[j]>-40.0)
        {
          Res=(int)((MemD.CWDMDataMem[j]+40.0)*2);
          if(Res>100) Res=100;
        }
        sprintf(Str,"j%d.val=%dяяя",j,Res);
        NEX_Transmit((void*)Str);//
}
      // ТУТ надо разобрать окрытый файл    
      //     f_lseek (&Fil, 2); // переместимся на 2 байта
      //     f_read (&Fil, (void*)&BlkSz, 4, &RWC);
      //     if(BlkSz==98) // есть события
      //     {
      //     f_lseek (&Fil, 0x44); // переместимся на 0x44 байта чтобы прочитать размер блока событий 
      //     f_read (&Fil, (void*)&EvntSz, 4, &RWC);
      //     PosDataLog = 0xe1 + 16 + EvntSz;
      //     }
      //     f_lseek (&Fil, BlkSz); // переместимся на  байта
      //     f_read (&Fil, (void*)&F_SOR, 142, &RWC);
      //      // читаем блок данных из файла
      //     f_lseek (&Fil, PosDataLog); // переместимся на начало блока данных байта
      //     f_read (&Fil, (void*)&LogData, F_SOR.NPPW*2, &RWC);
      
    }
    f_close(&Fil);
    FR_Status = f_mount(NULL, "", 0);
    
    for (int i=0; i<12; i++)
    {
      // закрасим бэкграунды  и установим требуемый
      sprintf(Str,"t%d.bco=WHITEяяя",i+1); // белый
      NEX_Transmit((void*)Str);// 
    }
    sprintf(Str,"t%d.bco=GREENяяя",IndexLCDNameFiles+1); // GREEN
    NEX_Transmit((void*)Str);    //
    // код подсветки требуемой строки если есть есть маркер строки
    
    // тут надозаполнить пля из файла
    //        sprintf(Str, "t15.txt=\"%d %d\"яяя", BlkSz, EvntSz); // < ракзмер заголовка, есть ли там события >
    //    NEX_Transmit((void*)Str);    //
    //        sprintf(Str, "t16.txt=\"%dnm\"яяя", F_SOR.AW/10); // < длина волны >
    //    NEX_Transmit((void*)Str);    //
    //        sprintf(Str, "t17.txt=\"%d\"яяя", F_SOR.NPPW); // < число точек >
    //    NEX_Transmit((void*)Str);    //
    //        //sprintf(Str, "t18.txt=\"%d\"яяя", F_SOR.AR); // < какой файл выбран >
    //        sprintf(Str, "t18.txt=\"%d\"яяя", F_SOR.NAV); // < число накоплений >
    //    NEX_Transmit((void*)Str);    //
    //        sprintf(Str, "t19.txt=\"%s\"яяя", F_SOR.CMT); // < комметарий >
    //    NEX_Transmit((void*)Str);    //
    // надо нарисовать заполненную таблицу измерений в виде графика...
    
    g_NeedScr = 0;
  }
  if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==UP_SHORT_PRESSED)) // переход в режим просмотра с переключением зума
  {
      SetMode(ModeViewMemory);
      if(ViewMod)
        CmdInitPage(2);// посылка команды переключения окна на Анализатор
      else
        CmdInitPage(3);// посылка команды переключения окна на Анализатор
    
    myBeep(10);
  }
  if ((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))
  {
//    if(ModeUSB) // запрещаем работу с памятью, так как занята 
//    {
//      myBeep(500);
//    }
//    else
//    {
      
      // здесь над витвится в зависимости от признака откуда пришли
      //if(ReturnMemView)
      //{
      SetMode(ModeFileMngDirD);
      //ModeDevice = MODEMEMR;
      //ModeMemDraw = VIEWNEXT;
      //ReturnMemView = 1; // надо вернуться сюда же по ESC
      // посылка команды переключения окна на Mem_OTDR_garaph (вызов)  
      //KeyP = 0;
      CmdInitPage(10); // новое окно лист бокс перечня директорий
//    }
    //CreatDelay(1000000);
    ClrKey(BTN_MENU);
    HAL_Delay(100);
    //}
  }
}

//-------------------------------------------------------------------------------------------------------
// окно индикации результатов анализатора из памяти, тут будем просматривать
void ModeViewMemory(void) // режим отображения из файла (памяти)
{
  //char Str[32];
  
  static volatile BYTE FrSetIndx = 0; // указатель на курсор
  static volatile BYTE IndxViewLW = 0; // указатель на длину волны
  int Res;
  // получение данных от измерителя
  //GetAllDataMeas((g_IndexMeas)&0xF);
  // Прорисовка нового индикатора
  if(g_FirstScr) // заполнение незменяемых полей
  {
    //    // наименование прибора
    //    sprintf (StrN,"t5.txt=\"%s\"яяя",MsgMass[6][CurrLang]); // 
    //    NEX_Transmit((void*)StrN);//
    //    // Анализатор
    //    sprintf(Str,"t1.txt=\"%s\"яяя",MsgMass[7][CurrLang]);
    //    NEX_Transmit((void*)Str);//
    //    // память
    //    sprintf(Str,"t2.txt=\"%s\"яяя",MsgMass[9][CurrLang]);
    //    NEX_Transmit((void*)Str);//
    //    // установки
    
    //    g_EnaQuickReDraw =1;
    //ModeReDrawLCD = 1;

    rawPressKeyS=0;// если вдруг кто нажимал это до этого
    g_FirstScr = 0;
    g_NeedScr = 1;
  }
  // обработка клавиатуры кнопки Влево Вправо
  if ((PRESS(BTN_LEFT))&&((getStateButtons(BTN_LEFT)==SHORT_PRESSED)||(getStateButtons(BTN_LEFT)==INF_PRESSED)))//
  {
    if(ViewMod) // Graph
    {
    myBeep(10);
    if(IndxViewLW>0)IndxViewLW--;
    else IndxViewLW = 17;
    g_NeedScr = 1; // Need reDraw Screen
    }
  }  
  if ((PRESS(BTN_RIGHT))&&((getStateButtons(BTN_RIGHT)==SHORT_PRESSED)||(getStateButtons(BTN_RIGHT)==INF_PRESSED)))//
  {
    if(ViewMod) // Graph
    {
    myBeep(10);
    if(IndxViewLW<17)IndxViewLW++;
    else IndxViewLW = 0;
    g_NeedScr = 1; // Need reDraw Screen
    }
  }
// поиск минимального по кнопке вниз и установка курсора
  if ((PRESS(BTN_DOWN))&&(getStateButtons(BTN_DOWN)==SHORT_PRESSED))//
  {
    float Min_F = 100.;
    if(ViewMod) // Graph
    {
    myBeep(10);
      for(int i=0;i<18;i++)
      {
        if(MemD.CWDMDataMem[i]<=Min_F)
        {
          Min_F = MemD.CWDMDataMem[i];
          IndxViewLW = i;
        }
      }
    g_NeedScr = 1; // Need reDraw Screen
    }
  }
// поиск максимального по кнопке вверх и установка курсора
  if ((PRESS(BTN_UP))&&(getStateButtons(BTN_UP)==SHORT_PRESSED))//
  {
    float Max_F = -100.;
    if(ViewMod) // Graph
    {
    myBeep(10);
      for(int i=0;i<18;i++)
      {
        if(MemD.CWDMDataMem[i]>=Max_F)
        {
          Max_F = MemD.CWDMDataMem[i];
          IndxViewLW = i;
        }
      }
    g_NeedScr = 1; // Need reDraw Screen
    }
  }
  if(g_NeedScr)
  {
    // рисуем
    //    line 0,300,340,300,YELLOW
    //draw 0,270,340,270,GREEN
    //draw 0,240,340,240,YELLOW
    //draw 0,210,340,210,GREEN
    //draw 0,180,340,180,YELLOW
    //draw 0,150,340,150,GREEN
    //draw 0,120,340,120,YELLOW
    //draw 0,90,340,90,GREEN
    //draw 0,60,340,60,WHITE
    //draw 0,30,340,30,GREEN
    //draw 0,0,340,0,YELLOW
    //xstr 345,290,35,20,3,BLUE,WHITE,0,1,1,"-40"
    //xstr 345,230,35,20,3,BLUE,WHITE,0,1,1,"-30"
    //xstr 345,170,35,20,3,BLUE,WHITE,0,1,1,"-20"
    //xstr 345,110,35,20,3,BLUE,WHITE,0,1,1,"-10"
    //xstr 345,50,35,20,3,BLUE,WHITE,0,1,1,"0"
    //xstr 345,20,35,20,3,BLUE,WHITE,0,1,1,"5"
    //   xstr 380,90,100,55,2,BLACK,WHITE,0,1,1,"-40.2
    if(ViewMod) // Graph
    {
      for(int i=0;i<18;i++)
      {
        Res=0;
        //MemD.CWDMDataMem
        if(MemD.CWDMDataMem[i]>-40.0)
        {
          Res=(int)((MemD.CWDMDataMem[i]+40.0)*6);
          if(Res>300) Res=300;
        }
        //sprintf(Str,"j%d.val=%dяяя",i,Res);
        //NEX_Transmit((void*)Str);//
                // рассчет Y
        int Y_H = 300 - Res;
        //sprintf(Str,"fill %d,%d,%d,%d,%dяяя",i*19,0,18,Y_H,6371); // fill темный фон
        sprintf(Str,"fill %d,%d,%d,%d,%dяяя",i*19,0,19,Y_H,45901); // fill темный фон
        //sprintf(Str,"fill %d,%d,%d,%d,%dяяя",i*19,0,18,Y_H,10857); // fill темный фон
        //sprintf(Str,"fill %d,%d,%d,%d,%dяяя",i*19,0,18,Y_H,59292); // fill светлый фон
        NEX_Transmit((void*)Str);//
        
        sprintf(Str,"fill %d,%d,%d,%d,%dяяя",i*19,Y_H,19,Res,ColorsPCO[i]); // fill
        //sprintf(Str,"j%d.val=%dяяя",i,Res); // progress bar
        //sprintf(Str,"h%d.val=%dяяя",i,Res); // slider
        NEX_Transmit((void*)Str);//

      }
      //      sprintf(Str,"tm0.en=1яяя");
      //      NEX_Transmit((void*)Str);//
      sprintf(Str,"t0.txt=\"%d%s\"яяя",1270+IndxViewLW*20,MsgMass[38][CurrLang]);// LW_nm
      NEX_Transmit((void*)Str);//
      sprintf(Str,"t0.bco=%dяяя",ColorsPCO[IndxViewLW]);// LW_nm
      NEX_Transmit((void*)Str);//
      sprintf(Str,"t1.txt=\"%.1f\"яяя",MemD.CWDMDataMem[IndxViewLW]);// значение
      NEX_Transmit((void*)Str);//
      sprintf(Str,"t2.txt=\"%s\"яяя",MsgMass[18][CurrLang]);// дБм
      NEX_Transmit((void*)Str);//
      if((IndxViewLW==8)||(IndxViewLW==12))
      {
      sprintf(Str,"t0.pco=65535яяя");// LW_nm
      NEX_Transmit((void*)Str);//
      }
      else
      {
      sprintf(Str,"t0.pco=0яяя");// LW_nm
      NEX_Transmit((void*)Str);//
      }
      //sprintf(Str,"h0.val=%dяяя",g_IndexLW);
      //NEX_Transmit((void*)Str);//
      sprintf(Str,"p1.pic=%dяяя",IndxViewLW+17);
      NEX_Transmit((void*)Str);//
      //      sprintf(Str,"tm0.en=0яяя");
      //      NEX_Transmit((void*)Str);//
    }
    else // Table
    {
      for(int i=0;i<18;i++)
      {
        sprintf(Str,"t%d.txt=\"%.2fdBm\"яяя",i+40,MemD.CWDMDataMem[i]); // зеленый
        NEX_Transmit((void*)Str);// 
      }
    }
    g_NeedScr = 0;
  }
  if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==UP_SHORT_PRESSED)) // возврат в режим просмотра
  {
    myBeep(10);
    if(ViewMod) // Graph
    {
      ViewMod = 0;
      CmdInitPage(3);// посылка команды переключения окна на Анализатор
    }
    else
    {
      ViewMod = 1;
      CmdInitPage(2);// посылка команды переключения окна на Анализатор
    }
    g_FirstScr = 1;
    //g_NeedScr = 1; // Need reDraw Screen
  }
  if ((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))// нужно вернутся к памяти
  {
    myBeep(10);
      SetMode(ModeFileMngFiles);
      CmdInitPage(11); // новое окно лист бокс перечня файлов в текущей дирректории
  }
  // вызов сохранения файла (его меню)
  if (rawPressKeyS) // 
  { 
////    if(ModeUSB) // запрещаем запись так как 
////    {
////    myBeep(500);
////    }
////    else
////    {
//    myBeep(10);
//    //  SaveFileSD(0);
//    SetMode(ModeSaverFILE);
//    //CreatDelay (30000); // 3.3 мС
//    HAL_Delay(3);
//    CmdInitPage(7);// посылка команды переключения окна на Меню Сохранения
//    g_EnaQuickReDraw =0;
////    }
    rawPressKeyS=0;
  }
  //    HAL_Delay(500);
  
  
}
//-------------------------------------------------------------------------------------------------------------------
// переключимся в режим программирования индикатора (пока на паузу  и сигнал
void UploadFW_Nextion(void) // обновление индикатора NEXTION
{ 
  //char Str[32];
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
    
    ModeReDrawLCD = 0;
    g_FirstScr = 0;
    g_NeedScr = 0;
  }
  
  if(ProgFW_LCD==0) // если сбросисли признак программирования то переключимся  в норм режим
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
unsigned char ChangeFrSet (unsigned char FrSet, unsigned char MaxSet, unsigned char MinSet, unsigned char DirSet)// установка курсора в рамках заданных параметров
{
  if (DirSet) // вверх
  {
    if (FrSet < MaxSet) FrSet++;
    else FrSet = MinSet;
  }
  else
  {
    if (FrSet > MinSet) FrSet--;
    else FrSet = MaxSet;
  }
   return FrSet; 
}

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
