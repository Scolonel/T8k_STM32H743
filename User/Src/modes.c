//#define LANG_NUM 3
#define ENADRAWORL 1
//#define ENAOLDLCD 1 //1 - разрешение прорисовки на старый LCD, 0 - нет старого экрана


#define NO_PARENT 0xFFFFFFFF
#define MAXWIDESMBL 17 // максимальное число широких символов в строке 
#define MAXSMALLSMBL 22
#define WAITWELCOME 100
#define TIMEDELAYINPUT 17000 // задержка при измерении качества разъема (короткий импульс-длинная линия)

#define PNTSNUM 7 // число точек контроля длины линии
#define PNTVERTICALSIZE 4
#define b_MENU 0
#define b_OK 1
#define b_RIGHT 2
#define b_LEFT 3
#define b_DOWN 4
#define b_UP 5

#define PLUS 1
#define MINUS 0

#define VIEWER 0
#define SETSCALE 1

#define WAITING 0
#define MEASUR 1

#define INCR 1
#define DECR -1
// подрежимы просмотра рефлектограмм в памяти
#define VIEWSAVED 7  // просто крутимся 
#define SAVEDTRACE 1 // сообщение о сохранении
#define MEMFULL 2 // сообщение о невозможности сохранится
#define VIEWNEXT 3 // просмотр следующей рефлектограммы
// признаки возврата из просмотра рефлектограммы в установки или в предпросмотр 
#define SETPARAM 0
#define VIEWMEM 1
#define VIEWEVNT 2

#define BEGZONEPNT 12 //число точек когда считаем хороший разъем (для новых AR когда слабая лавина и нет затяжки, смотрим только импульс)30.04.2021
#define BEGZONEAVRG 10 //число накоплений при рассчете хорошиго разъема


#define DELTRACE 4 // запрос подтверждения удаления рефлектограммы
#define DELETEDTRACE 5

// #define NumFiltrs 4

#define Pic_Tx 40 // X - смещение картинки при выводе установки времени
#define Pic_Ty 2 // Y - смещение картинки при выводе установки времени

#include "system.h"
// тест картинка
//static const  unsigned char LOGOS[] = {
//  0xaa, 0xaa, 0xaa, 0xaa, 
//  0x01, 0x00, 0x00, 0x80, 
//  0x02, 0x00, 0x00, 0x40, 
//  0x01, 0xaa, 0x55, 0x80, 
//  0x02, 0xaa, 0x55, 0x40, 
//  0x01, 0x00, 0x00, 0x80, 
//  0x02, 0x00, 0x00, 0x40, 
//  0xaa, 0xaa, 0xaa, 0xaa 
//};

// начало линии
const unsigned char BegLine [40] = {
 #include "BegB.pic"
}
;
// отражающее событие
const unsigned char EvntRefl [40] = {
 #include "ReflEvnt.pic"
}
;
// большое отражение - "перегрузка" в конце линии
const unsigned char BigEnd [40] = {
 #include "BigOvrEnd.pic"
}
;
// без отражательный конец линии
const unsigned char NoRefEnd [40] = {
 #include "NoReflEnd.pic"
}
;
// без отражательныое событие
const unsigned char NoRefEvn [40] = {
 #include "NoReflEvnt.pic"
}
;
//выход в шумы (OutOfRange)
const unsigned char OutOfNoise [40] = {
 #include "OutNoise.pic"
}
;




const int KeyPoints[PNTSNUM] = { 96, 172, 344, 688, 1366, 2048, 4608 }; // порги определения индекса установленной длины 4096
 // всеж таки надо брать по конкретной длине а не по полному массиву, так как пересчитываем на реальную длину
//const int KeyPoints[PNTSNUM] = { 96, 192, 384, 768, 1536, 2304, 4608 }; // порги определения индекса установленной длины для всего массива
//const int VerticalSize[PNTVERTICALSIZE] = { 22000, 16000, 8000, 4000 }; // вертикальные масштабы  при просмотре
// время измерния в uS в зависимости от длины и прореживания
const int TimeRepitOfLN[PNTSNUM] = { 105, 120, 200, 350, 450, 750, 1500 }; // порги определения индекса установленной длины 4096

const DWORD DelayBadLength [LENGTH_LINE_NUM-1][LENGTH_LINE_NUM]= //длина установленная,длина полученная
{{0,1250,4600,11000,18000,12000,26000},
{0,0,3300,9700,16800,10700,24600},
{0,0,0,6400,13500,7400,21350},
{0,0,0,0,7100,950,14900},
{0,0,0,0,0,0,7700},
{0,0,0,0,0,0,13900}};

struct tag_PON PONI;


char CommentsOTDR[20] = {"                   \0"}; //комментарии рефлектометра
char FileNameOTDR[20] = {"                   \0"}; //комментарии рефлектометра
char PrefixFileNm[11] = {"Fiber     \0"}; //комментарии рефлектометра
char IndexPrefix = 0;
static unsigned int NumFiber = 1; // авто счетчик числа жил
char IndexCommOTDR = 0;
char CommentsOLT[16] = {"               \0"}; //комментарии измерителя
char CommentsOLTOld[16] = {"               \0"}; //комментарии измерителя
char IndexCommOLT = 0;
char VerFW_LCD[25] = {"No version LCD          \0"}; //версия ПО индикатора NEXION
char txtout[64];
static void (*ModeFunc)(void);

static unsigned CntWelcome = 0;
static BYTE PosCurr = 6; // положение курсора устанавливаемых величин в настройках часов
static BYTE SubModeMeasOTDR = NOTMODE; // Подрежимы измерения рефлектометра
static DWORD CurrentSumDelay; // итоговая суммарная задержка при изменении длины линии для режима накоплений
static unsigned NumAvrg; // число накоплений (общий по периодам по 3 сек)
static unsigned FinAvrg; // число накоплений всего по времени
static unsigned CntNumAvrg; // счетчик числа накоплений ()
static unsigned ValueDS; // значение DS выбранного режима съема (для расчета значения длины и записи в Белкор)
static char KbPosX=11; // позиция Х в клавиатуре
static char KbPosY=0; // позиция Y в клавиатуре
static BYTE IndexVerSize = 0;
volatile BYTE IndexSmall = 0; // переключатель для 2 км ,0 - 0.5 км, 1 - 2км 
static BYTE ModeMemDraw = VIEWSAVED;
static BYTE ModeDevice = MODEMENU;
static BYTE ModePowerMeter = NOTMODE;
static BYTE ReturnModeViewRefl = SETPARAM;
// переменная разрешения корректировки мертвых зон
static BYTE EnaCorrMZ = 0;
int CntPointPick;
unsigned short PointsPick[16];
unsigned short PointsPickIndx[16];

// переменные режима рисования рефлектограммы
  static char ViewMode = VIEWER;//SETCSALE
  static BYTE TypeCentral = MEDIUM;
// переменные автоматического измерителя
static float RPON[5]; // значения для записи в память PON 0-850,1-1310,2-1490,3-1550,4-1625 
static unsigned short LWPON[5]; //длины волн принятые что бы не привязываться к калиброванным
static unsigned short LWCNT[5]; //счетчик времени не принятия записанной волны
static BYTE ModAutoPM = WAITING; // режим измерителя авто ожидаем
static float LastPower; // значения для записи в память manual
//static BYTE Pow_Yes; //признак приняли длину волны можно измерять спустя некоторое время
static BYTE IndWavePON; //индекс длины волны в перечне калибровочных волн
static WORD TimerAutoPM=0; // счетчик времени
//static BYTE NeedSaveCell = 0;//признак сохранения измерений измерителя
static BYTE FrClearMEM = 1; // указатель на курсор
static WORD NumCellIzm ;// текущее значение ячейки при просмотре
static WORD ProcBat , ProcBatInd; // процент баттареи
static int ADCData; // коды Ацп баттареи
static float Ubat; // напряжение батареи в вольтах
BYTE NeedDrawCRC=0;
static volatile DWORD MeagNoise;
// ошибка измерителя
static DWORD ErrOLT;
// для событий
static unsigned short NumCalkEvents; // число посчитанных событий
static unsigned short IndexEvents; // указатель на событие 
static unsigned short IndexEventsOld; // указатель на событие 
    // теперь нарисуем курсор в рамках Y=0..210
static unsigned CurrRed;
static unsigned CurrBlue;
static BYTE MonitorEna; // признак необходимости по завершению измерения выдать таблицу событий
static BYTE CntLS=0; // признак счетчик перключения лазеров (равен 1 так как уже как бы на него смотрим уже 1 раз выбрали)
// 17.11.2022 в новой редакции, это счетчик измерений при авто сохранении
// признак дистанционного управления (для проверки в конце измерений, устанавливаем при приеме коменды сбрасываем когда отработали)
 unsigned long TimeMeasure3S;
 BYTE RemoutCtrl=0; // признак дист управления
 float CursorScale =1.0;
//переменные настройки измерения ORL 
float g_VolORL = 0.0 ;
float g_UpGrdB = 10.0; // повышающие дБ или разы???
float g_kLog = 10.0; // коэффициент логарифма
float g_OffSetdB = 75.0; // смещение в формуле dB
BYTE g_mem_param = 0; // что выводим при просмотре рефлектограмм в памяти , параметры измерения или полученные значения линии(длина затух orl)
//static BYTE CompJDSU = 0;
volatile BYTE g_GetStr=0; // признак ожидания получения строки из редактора. 
volatile BYTE g_NeedScr=1; // перерисовки экрана ! Аккуратно! надо проверить 
volatile BYTE g_FirstScr=1; // первый вход в экран, нужна полная перерисовка (заполнение), далее изменяем только нужные поля, для NEXTION
volatile BYTE NeedKeyB = 0; // необходимость переключения в клавиатуру
volatile BYTE TypeLCD = 0; // тип индикатора в идентификаторе v-3.2(=0) s-3.5(=1) 
volatile BYTE g_NeedChkAnsvNEX=0; // признак получения строки из редактора.и ее проверка
volatile BYTE g_AutoSave=0; // признак режима авто сохранения измерений на выбраных длинах волн
volatile int NeedReturn = 0; // необходимость вернуться в окно сохранения
static BYTE ReturnMemView = 0; // признак куда возвращаемся по Esc из просмотра памяти, 1- в память, там и устанавливаем
// 0 - из старта измерений 



      // переключатель вывода в зависимости от комбинации
void SetPlaceNew(int Mode)
      {  
        if(Mode)
        g_AutoSave = 0; // сброс авто режима
          
          
      switch (GetCombLS(SettingRefl.SetModeLW))
      {
      case 1:
        SettingRefl.SW_LW = 0; // установка индекса начального рабочего лазера
          GetSetModeLW(-1); // сбрасываем авто режим
        break;
      case 2:
        SettingRefl.SW_LW = 1; // установка индекса начального рабочего лазера
          GetSetModeLW(-1); // сбрасываем авто режим
        break;
      case 3: // комбинации
        g_AutoSave = 1; // дл. волн автосохраненние включено
        SettingRefl.SW_LW = 0; // установка индекса начального рабочего лазера
          GetSetModeLW(1); // устанавливаем групповой режим, переключаем поля
        break;
      case 4:
        SettingRefl.SW_LW = 2; // установка индекса начального рабочего лазера
          GetSetModeLW(-1); // сбрасываем авто режим
        break;
      case 5:
        g_AutoSave = 1; // дл. волн автосохраненние включено
        SettingRefl.SW_LW = 0; // установка индекса начального рабочего лазера
          GetSetModeLW(1); // устанавливаем групповой режим, переключаем поля
        break;
      case 6:
        g_AutoSave = 1; // дл. волн автосохраненние включено
        SettingRefl.SW_LW = 1; // установка индекса начального рабочего лазера
          GetSetModeLW(1); // устанавливаем групповой режим, переключаем поля
        break;
      case 7:
        g_AutoSave = 1; // дл. волн автосохраненние включено
        SettingRefl.SW_LW = 0; // установка индекса начального рабочего лазера
          GetSetModeLW(1); // устанавливаем групповой режим, переключаем поля
        break;
      default: // одиночные длины волн
        g_AutoSave = 0; // длина волны одиночная сброс авто сохранения/
        SettingRefl.SW_LW = 0; // установка индекса начального рабочего лазера
          GetSetModeLW(-1); // сбрасываем авто режим
        break;
      }
      SettingRefl.K_pr = GetWAV_SC(SettingRefl.SW_LW); // в текущий подставляем по месту лазера
      GetPlaceLS(CURRENT); // устанавливаем работу нужного места
    }

// звуковой сигнал типа аварии (проба) 04.04.2024
void AlarmSignal(int type)
{
   HAL_Delay(2);
 for (int i=0; i<type; i++)
 {
   LED_START(1);//On  LED
   myBeep(10);
   HAL_Delay(250);
   LED_START(0);//Off  LED
   myBeep(0);
   HAL_Delay(250);
 }
}


//void SlowON (void) // медленное включение питания
//{
//  POWREF (ON);
//  CreatDelay (10000000); // 0.7 С (с этой задержкой вроде работает от USB, без акк)
//  POWDET(ON);
//  CreatDelay (4000000); // 0.3 С
//  POWALT(ON);
//  CreatDelay (4000000); // 0.3 мС
//}


BYTE SetGetMonEna (BYTE Data) // запрос/установка признака выдачи событий по результату измерения
{
  if(Data != 255)
  {
    MonitorEna = Data;
  }
  return MonitorEna;
}

WORD SetNumCellIzm (WORD Data)
{
  if (Data > GetCellMem(0)-1)NumCellIzm=0;
  else NumCellIzm = Data;
  return NumCellIzm;
}

BYTE GetModeDevice (void)// получение режима прибора
{
  return ModeDevice;
}

unsigned GetCntNumAvrg ( void)// получение текущего значения накоплений
{
  return CntNumAvrg;
}
unsigned GetFinAvrg ( void)// получение конечного значения накоплений (определяется временем усреднения)
{
  return FinAvrg;
}

unsigned SetCntNumAvrg ( unsigned Data)// установка текущего значения накоплений
{
  return (CntNumAvrg = Data);
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
  CheckLevelBattery ();
  // Выделим блок для выключения если LCD не пользуем
  // дублирующий блок для NEXTION
  if(g_FirstScr)
  {
    
    // инициализация не изменяемых величин
    // наименование прибора
    GetDeviceLabel( Str, NameDB.Ena_DB  );
    sprintf (StrN,"t0.txt=\"%s\"яяя",Str); // 
    NEX_Transmit((void*)(void*)StrN);//
    // производитель
    if (GetID_Dev())
    {
      sprintf (Str,"t1.txt=\"%s\"яяя",MsgMass[12][CurrLang]); // Чехи
    }
    else
    {
      sprintf (Str,"t1.txt=\"%s\"яяя",MsgMass[75][CurrLang]); // Мы
    }
    NEX_Transmit((void*)Str);//
    // Date
    sprintf (Str,"t2.txt=\"%s\"яяя",MsgMass[13][CurrLang]); // Date
    NEX_Transmit((void*)Str);//
    // Time
    sprintf (Str,"t4.txt=\"%s\"яяя",MsgMass[15][CurrLang]); // time
    NEX_Transmit((void*)Str);//
    // Battery
    sprintf (Str,"t6.txt=\"%s\"яяя",MsgMass[16][CurrLang]); // Battery
    NEX_Transmit((void*)Str);//
    // Version
    GetNumVer(St);
    sprintf (Str,"t8.txt=\"v%s(%X)\"яяя",St,CheckErrMEM); // Version
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
    if (!(EXT_POW))
    {
      sprintf(Str,"t7.txt=\"%s\"яяя",MsgMass[11][CurrLang]);// внешнее питание
    }
    else
    {
      sprintf(Str,"t7.txt=\"%d%%\"яяя", ProcBatInd);
    }
    NEX_Transmit((void*)Str);// 
  }
  g_FirstScr=0; // выключаем признак первого прохода
  
  if ((HAL_GetTick() - TimeBegin) > 4000) //4s
    //if (CntWelcome > WAITWELCOME)
  {
    SetMode(ModeMainMenu);
    CmdInitPage(1);// посылка команды переключения окна на MainMenu и установка признака первого входа
    
    myBeep(10);
    ModeDevice = MODEMENU;
    SetHeadFileRaw (0); // пишем заголовок файла выдачи необработаных данных
    
    //ADCData++;
    //ClearScreen();
  }   
  
}

void ModeMainMenu(void) // режим основного МЕНЮ
{ 
  static volatile BYTE FrSetMainMenu = 1; // указатель на курсор
  static WORD CntInd = 0;
  //static WORD ProcBatNow = 55;
  static BYTE OnlyBat = 1;
  static BYTE KnowLCD = 0;

  char Str[32];
  char StrN[32];
  //  static long tutu=0;
  //  Rect rct_pic={tutu,0,tutu+128,64};
  //  сначала кнопочки опросим  
  //  if (BUTTON_DOWN(BTN_UP)/*(KeyP & (1<<b_UP)*/)&&(getStateButtons(b_UP)==SHORT_PRESSED))
  if ((PRESS(BTN_UP))&&(getStateButtons(BTN_UP)==SHORT_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1;
    FrSetMainMenu = ChangeFrSet (FrSetMainMenu, 4, 1, MINUS);// установка курсора в рамках заданных параметров
    //ClrKey (BTN_UP);
  }
  if ((PRESS(BTN_DOWN))&&(getStateButtons(BTN_DOWN)==SHORT_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1;
    FrSetMainMenu = ChangeFrSet (FrSetMainMenu, 4, 1, PLUS);// установка курсора в рамках заданных параметров
    //ClrKey (BTN_DOWN);
  }
  
  //  if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))
  //  {
  //    tutu++;
  //    //ClrKey (BTN_LEFT);
  //  }
  if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))
  {
    myBeep(10);
    while (SetIndexLight(1)!=1);
    //Light(GetTimeLight(SetIndexLight(0)));
    //ClrKey (BTN_LEFT);
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
    GetDeviceLabel( Str, NameDB.Ena_DB  );
    sprintf (StrN,"t5.txt=\"%s\"яяя",Str); // 
    NEX_Transmit((void*)StrN);//
    // refl
    sprintf(Str,"t1.txt=\"%s\"яяя",MsgMass[7][CurrLang]);
    NEX_Transmit((void*)Str);//
    GetTesterName(Str ); // Tester
    sprintf (StrN,"t2.txt=\"%s\"яяя",Str); // 
    NEX_Transmit((void*)StrN);//
    sprintf(Str,"t3.txt=\"%s\"яяя",MsgMass[9][CurrLang]);
    NEX_Transmit((void*)Str);//
    sprintf(Str,"t4.txt=\"%s\"яяя",MsgMass[10][CurrLang]);
    //LCD_LIGHT(ON);
    NEX_Transmit((void*)Str);//
    //CreatDelay (3000000); // 168 мС - пока без ответа (подтверждения) 83nS*30000 надо приблизительно 2 мС
    HAL_Delay(300);    // попробуем запросить код версии индикатора
    // выключим опрос индикатора всегда 
     TypeLCD=1;
     
#if 0
     StartRecievNEX (500);
    sprintf(Str,"get t6.txtяяя");
    NEX_Transmit((void*)Str);//
    //NEX_Transmit((void*)CmdBuf);//
    while(!((g_WtRdyNEX)||(ReadyNEX==4)));
    // здесь просто можем повиснуть не дождавшись ответов от индикатора
    // это плохо при плохих индикаторах
    // надо ждать получения ответа
    //CreatDelay (2000000); // 168 мС - пока без ответа (подтверждения) 83nS*30000 надо приблизительно 2 мС
    if(RX_BufNEX[0] == 0x70) // есть ответ! перепишем буффер
    {
      for(int i=0;i<25;++i)VerFW_LCD[i]=RX_BufNEX[i+1];
      VerFW_LCD[23]=0;
      // здесь получим идентификатор индикатора (если его прочтем)
      // он нужен для вариантов отображения при просмотре рефлектограмм и в памяти
      switch(VerFW_LCD[3])
      {
      case '2':
        TypeLCD=0;
        KnowLCD = 1;
        break;
      case '5':
        TypeLCD=1;
        KnowLCD = 1;
        break;
      default:
        TypeLCD=0;
        KnowLCD = 0;
        break;
      }
    }
    if(!KnowLCD)
      // возможно индикатор "больной" надо просигнализировать и попытаться работать дальше
      AlarmSignal(3);
#endif

      //сюда перенесем установки QR code
        if(TypeLCD)
    sprintf(Str,"qr0.pco=BLACKяяя"); // QR черный
    else
    sprintf(Str,"qr0.pco=BLUEяяя"); // QR синий
      
    NEX_Transmit((void*)Str);// 

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
    sprintf(Str,"t4.bco=WHITEяяя"); // белый
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
    // уровень и тип заряда
    if (!(EXT_POW))
    {
      sprintf(Str,"t0.txt=\"%s\"яяя",MsgMass[11][CurrLang]);// внешнее питание
    }
    else
    {
      sprintf(Str,"t0.txt=\"%d%%\"яяя", ProcBatInd);
    }
    NEX_Transmit((void*)Str);// 
    OnlyBat=0;
  }
  // кнопка "ОК" для данного меню означает переход в другое окно, поэтому ее можно перенести сюда!
  if (PRESS(BTN_OK))
  {
    myBeep(10);
    switch (FrSetMainMenu)
    {
    case 1: // рефлектометр
      // переход в режим установки параметров рефлектометра из меню или при включении если ТАБЛЕТКА
      SetMODESettingRef ();
      // инициализация списка комбинаций установок лазеров
      SetSeqCombLW();
      
      CmdInitPage(2);// посылка команды переключения окна на OTDR
      
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
    case 2: // ТЕСТЕР
      
      if (GetCfgPM ()) // есть измеритель
      {
        SetMode(ModeSelectOLT); 
        ErrOLT = CheckErrCoeff ();
        
        
        ModeDevice = MODEOTHER; // чтобы при переходе в выбор измерителя не выкл. источник
        CmdInitPage(3);// посылка команды переключения окна на Tester (выбор авто или нет)
        
      }
      else
      {
        SetMode(ModeSourceOnly); // сразу тестер (просто источник)
        ModeDevice = MODETEST;
        CmdInitPage(6);// посылка команды переключения окна на Просто Тестер
        
      }
      POWDET(ON); // включаем питание измерителя
      // сделаем "тупую задержку" перед вклчением питания, так как на некоторых приборах
      // не успевал прориоваться индикатор, это было на 2529 с 3.2 дюйма
      // 
      HAL_Delay (40); //
      SetupSource ( SetModeLS (Str, CURRENT, CurrLang)); // возвращает номер режима источника, и пишет строку для отображения на индикаторе
      
      break;
    case 3: // ПАМЯТЬ
//123      PWM_LMOD_Init (OFF); // выключаем LMOD от PWM 0 - OFF, 1- 270Hz 2-2kHz  
      SetMode(ModeSelectMEM);
      ModeDevice = MODEOTHER;
//123      SSPInit_Any(MEM_FL1); // Инициализация SSP для управления FLASH (порт 1 та что на плате отладочной)
      CmdInitPage(4);// посылка команды переключения окна на MemoryMenu
      
      break;
    case 4: // УСТАНОВКИ
      SetMode(ModeSetting);
//123      PWM_LMOD_Init (OFF); // выключаем LMOD от PWM 0 - OFF, 1- 270Hz 2-2kHz  
      ModeDevice = MODESETUP;
//123      SSPInit_Any(MEM_FL1); // Инициализация SSP для управления FLASH (порт 1 та что на плате отладочной)
      CmdInitPage(5);// посылка команды переключения окна на Setting
      break;
    }
    //ClrKey (BTN_OK);
  }
  // сделаем "тупую задержку"
  HAL_Delay (20); //
  
  //ClrKey (BTN_MENU + BTN_LEFT + BTN_RIGHT);
}

void ModeSetupOTDR(void) // режим установок рефлектометра CHECK_OFF
{
  static volatile BYTE FrSetSetupOTDR = 0; // указатель на курсор
  char Str[64];
  int NeedReSaveIndex=0; // признак пересохранения текущего индекса в индекс привязвнный к месту лазера
  
  //BYTE CurrLang=GetLang(CURRENT);
  if ((PRESS(BTN_UP))&&(getStateButtons(BTN_UP)==SHORT_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1; // Need reDraw Screen
    FrSetSetupOTDR = ChangeFrSet (FrSetSetupOTDR, 4, 0, MINUS);// установка курсора в рамках заданных параметров
    //ClrKey (BTN_UP);
  }
  if ((PRESS(BTN_DOWN))&&(getStateButtons(BTN_DOWN)==SHORT_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1; // Need reDraw Screen
    FrSetSetupOTDR = ChangeFrSet (FrSetSetupOTDR, 4, 0, PLUS);// установка курсора в рамках заданных параметров
    //ClrKey (BTN_DOWN);
  }
  switch (FrSetSetupOTDR) // обработка выбраных полей установок
  {
  case 0: //длина волны
    // кнопка ОК переключаем АВТО!
    if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
    {
      myBeep(10);
//      if(g_AutoSave) g_AutoSave = 0;
//      else g_AutoSave = 1;
//      if(GetIndexVRM()>3) // время реальное или непрерываное не ставится авто сохранение
//      g_AutoSave = 0; 
      if((g_AutoSave)||(GetIndexVRM()>3)) g_AutoSave = 0;
      else g_AutoSave = 1;
      // переключатель вывода в зависимости от комбинации 
      // если стоим на комбинации то снова включаем авто
      SetPlaceNew(0);
      g_FirstScr = 1; // Need reDraw Screen
      
    }
    // ВЛЕВО <---
    if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))
    {
      myBeep(10);
      g_FirstScr = 1; // Need reDraw Screen
      if(GetIndexVRM()>3) // время реальное или непрерываное не ставится авто сохранение
      {
      if (SettingRefl.SetModeLW>0)SettingRefl.SetModeLW--;
       else
       SettingRefl.SetModeLW =GetSetNumLS(0)-1  ;
      }
      else
      {
      if (SettingRefl.SetModeLW>0)SettingRefl.SetModeLW--;
      else
      {
        SettingRefl.SetModeLW = (1<<GetSetNumLS(0))-2;
      }
      }
      // переключатель вывода в зависимости от комбинации
      SetPlaceNew(1);
    }
    if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))
    {
      myBeep(10);
      g_FirstScr = 1; // Need reDraw Screen
      // 22.11.2022 манипуляции с переключениями длин волн в зависимости от выбранного времени измерения
      if(GetIndexVRM()>3) // время реальное или непрерываное не ставится авто сохранение
      {
        if(SettingRefl.SetModeLW<(GetSetNumLS(0)-1)) SettingRefl.SetModeLW++;
        else         SettingRefl.SetModeLW = 0;

      }
      else
      {
      if ((SettingRefl.SetModeLW+1)<((1<<GetSetNumLS(0))-1))SettingRefl.SetModeLW++;
      else
      {
        SettingRefl.SetModeLW = 0;
      }
      }
      // переключатель вывода в зависимости от комбинации
      SetPlaceNew(1);
    }
    //    if (((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))||((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED)))
    //    {
    //      myBeep(10);
    //      g_FirstScr = 1; // Need reDraw Screen
    //      if ((CntLS) < GetSetNumLS(0))
    //      {
    //        GetSetModeLW(-1); // сбрасываем авто режим
    //        CntLS++;
    //        GetPlaceLS(NEXT);
    //      }
    //      else
    //      {
    //        if (GetIndexVRM()<4)
    //          GetSetModeLW(1); // устанавливаем авто режим, если реальные времена накоплений 15,30,60,180с
    //        else GetPlaceLS(NEXT);
    //        CntLS=0; // для автомата вроде как надо 0
    //      }
    //      SetIndexWAV(GetWAV_SC(GetPlaceLS(CURRENT))); // устанавливаем коэфф. преломления выбранной длины волны
    //      
    //      //ClrKey (BTN_RIGHT);
    //    }
    break; //длина волны
  case 1: //длина линии
    if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
//123      PosCursorMain (-4100); // сброс курсора в начало
      IndexVerSize  = 0;// установка вертикального размера отображения рефлектограммы ( самый крупный)
//123      GetSetHorizontScale (5); // сброс масштаба уст самый крупный
      if (!GetSubModRefl ())// ручной
      { 
        if ((GetIndexLN()==0)&&(IndexSmall==0))
        {
          IndexSmall=1;
        }
        else
          SetIndexLN(GetIndexLN()+1);
      }
      else // 
      {
        SetSubModRefl (MANUAL);
        SettingRefl.SubModeRef = MANUAL;
        SetIndexLN(0);
        IndexSmall=0;
//123        GetSetHorizontScale (-2); // масштаб для 0.5 км
      }
      //ClrKey (BTN_RIGHT);
    }
    if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
//123      PosCursorMain (-4100); // сброс курсора в начало
      IndexVerSize  = 0;// установка вертикального размера отображения рефлектограммы ( самый крупный)
//123      GetSetHorizontScale (5); // сброс масштаба уст самый крупный
      if (!GetSubModRefl ())// ручной
      {
        if (GetIndexLN()>0)
        {
          
          SetIndexLN(GetIndexLN()-1);
        }
        else
        {
          if ((GetIndexLN()==0)&&(IndexSmall==1))
          {
            IndexSmall=0;
//123            GetSetHorizontScale (-2); // масштаб для 0.5 км
          }
          else
          {
            SetSubModRefl (AUTO);
            SettingRefl.SubModeRef = AUTO;
            SetIndexLN(0);
            IndexSmall=1;
          }
        }
      }
      else // 
      {
        SetIndexLN(0);
      }
      //ClrKey (BTN_LEFT);
    }
    break; //длина линии
  case 2: //длительность импульса 
    if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      if (!GetSubModRefl ())// ручной
      {
        SetIndexIM(GetIndexIM()+1);
      }
      else // 
      {
        SetIndexIM(0);
      }
      //ClrKey (BTN_RIGHT);
    }
    if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      if (!GetSubModRefl ())// ручной
      {
        if (GetIndexIM()>0)
        {
          SetIndexIM(GetIndexIM()-1);
        }
        else
        {
          SetIndexIM(0);
        }
      }
      else // 
      {
        SetIndexIM(0);
      }
      //ClrKey (BTN_LEFT);
    }
    break; //длительность импульса
  case 3: //время измерения или для автомата номер волокна
    if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
    // 22.11.2022 изменение индикации строк 4-5 надо привязать к режиму с автоматическим сохранением
    //if (GetSetModeLW(0))
    if (g_AutoSave)
      {
        if (NumFiber<999) NumFiber++;
        else NumFiber=1;
      }
      else
      {
        SetIndexVRM(GetIndexVRM()+1);
        if (GetIndexVRM()>3)// сбрасываем признак авто режима
        {
          GetSetModeLW(-1); // сбрасываем авто режим
          //CntLS=1;
        }
      }
      //ClrKey (BTN_RIGHT);
    }
    if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==INF_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
    // 22.11.2022 изменение индикации строк 4-5 надо привязать к режиму с автоматическим сохранением
    //if (GetSetModeLW(0))
    if (g_AutoSave)
      {
        if (NumFiber<989) NumFiber=NumFiber+10;
        else NumFiber=1;
      }
      //ClrKey (BTN_RIGHT);
    }
    if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==INF_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
    // 22.11.2022 изменение индикации строк 4-5 надо привязать к режиму с автоматическим сохранением
    //if (GetSetModeLW(0))
    if (g_AutoSave)
      {
        if (NumFiber>10) NumFiber=NumFiber-10;
        else NumFiber=990;
      }
      ClrKey (BTN_LEFT);
    }
    if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
    // 22.11.2022 изменение индикации строк 4-5 надо привязать к режиму с автоматическим сохранением
    //if (GetSetModeLW(0))
    if (g_AutoSave)
      {
        if (NumFiber>1) NumFiber--;
        else NumFiber=999;
      }
      else
      {
        if (GetIndexVRM()>0)
        {
          SetIndexVRM(GetIndexVRM()-1);
        }
        else
        {
          SetIndexVRM(0xFF);
        }
        if (GetIndexVRM()>3)// сбрасываем признак авто режима
        {
          GetSetModeLW(-1); // сбрасываем авто режим
          //CntLS=1;
        }
      }
      //ClrKey (BTN_LEFT);
    }
    break; //Время измерения
  case 4: //Коэфф.Преломления 
    // 22.11.2022 изменение индикации строк 4-5 надо привязать к режиму с автоматическим сохранением
    //if (GetSetModeLW(0))
    if (g_AutoSave)
    {
      if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
      {
        g_NeedScr = 1; // Need reDraw Screen
        myBeep(10);
        for (int Ind =ARRAY_SIZE(PrefixFileNm)-2; Ind>=0; Ind--)
        {
          if (PrefixFileNm[Ind]<0x20) PrefixFileNm[Ind]=' ';
          else if (PrefixFileNm[Ind]!=' ' && IndexPrefix == 0) IndexPrefix = Ind;
          //Index_Comm --;
        }
        if (PrefixFileNm[IndexPrefix]!=' ')IndexPrefix ++;
        KbPosX = 11;
        KbPosY = 2;
        SetMode(ModeKeyBoardPrefix);
        ModeDevice = MODEOTHER;
        NeedKeyB =1; // надо переключится в клавиатуру
        //ClrKey (BTN_OK);
      }
    }
    else
    {
      //            if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED)) // запомним изменение коэфф. преломления
      //      {
      //        myBeep(20);
      //        SetWAV_SC(GetIndexWAV(),GetPlaceLS(CURRENT));
      //        SSPInit_Any(MEM_FL1); // Инициализация SSP для записи FLASH 
      //        SaveUserConfig();
      //        //SSPInit_Any(SPI_ALT); // Инициализация SSP для управления ALTERA 
      //
      //      }
      
      if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))
      {
        myBeep(10);
        g_NeedScr = 1; // Need reDraw Screen
        SetIndexWAV(GetIndexWAV()+0.0001);
        NeedReSaveIndex = 1;
        //ClrKey (BTN_RIGHT);
      }
      if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))
      {
        myBeep(10);
        g_NeedScr = 1; // Need reDraw Screen
        SetIndexWAV(GetIndexWAV()-0.0001);
        NeedReSaveIndex = 1;
        //ClrKey (BTN_LEFT);
      }
      if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==INF_PRESSED))
      {
        //myBeep(10);
        g_NeedScr = 1; // Need reDraw Screen
        SetIndexWAV(GetIndexWAV()+0.0001);
        NeedReSaveIndex = 1;
        ////ClrKey (BTN_RIGHT);
      }
      if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==INF_PRESSED))
      {
        //myBeep(10);
        g_NeedScr = 1; // Need reDraw Screen
        SetIndexWAV(GetIndexWAV()-0.0001);
        NeedReSaveIndex = 1;
        //ClrKey (BTN_LEFT);
      }
    }
    break; //Коэфф.преломления
  }
  // если не коэфф преломления 
  if ((FrSetSetupOTDR == 4) && (NeedReSaveIndex))
  {
    ReSaveWAV_SC (); // пересохраняем если есть изменения ИНДЕКСА
    NeedReSaveIndex = 0;
  }
  // заполняем новый индикатор если в первый раз в основном не изменяемые поля
  if(g_FirstScr)
  {
    
    // столбцы названий 
    // переключатель вывода в зависимости от комбинации
    
    switch (GetCombLS(SettingRefl.SetModeLW))
    {
    case 1: // одиночные длины волн
    case 2:
    case 4:
    sprintf(Str,"t0.txt=\"%s\"яяя", MsgMass[17][CurrLang]); // длина волны
      break;
    default: // комбинации
    sprintf(Str,"t0.txt=\"%s\"яяя", MsgMass[125][CurrLang]); // дл. волн авто
      
      break;
    }
    NEX_Transmit((void*)Str);// 
    sprintf(Str,"t2.txt=\"%s\"яяя", MsgMass[19][CurrLang]); // длина линии 
    NEX_Transmit((void*)Str);// 
    sprintf(Str,"t4.txt=\"%s\"яяя", MsgMass[22][CurrLang]); //зонд.Имп.
    NEX_Transmit((void*)Str);// 
    // 22.11.2022 изменение индикации строк 4-5 надо привязать к режиму с автоматическим сохранением
    //sprintf(Str,"t6.txt=\"%s\"яяя", (GetSetModeLW(0))?(MsgMass[61][CurrLang]):(MsgMass[24][CurrLang])); //"Время Изм."//"№ волокна"
    sprintf(Str,"t6.txt=\"%s\"яяя", (g_AutoSave)?(MsgMass[61][CurrLang]):(MsgMass[24][CurrLang])); //"Время Изм."//"№ волокна"
    NEX_Transmit((void*)Str);// 
    // 22.11.2022 изменение индикации строк 4-5 надо привязать к режиму с автоматическим сохранением
    //sprintf(Str,"t8.txt=\"%s\"яяя", (GetSetModeLW(0))? (PrefixFileNm):(MsgMass[27][CurrLang])); //"префикс Коэфф.прелом."
    sprintf(Str,"t8.txt=\"%s\"яяя", (g_AutoSave)? (PrefixFileNm):(MsgMass[27][CurrLang])); //"префикс Коэфф.прелом."
    NEX_Transmit((void*)Str);// 
    
    
    
    g_NeedScr = 1; // для вызова заполнения значений
    g_FirstScr = 0;
  }
  // надо что то изменить в полях установок 
  if(g_NeedScr)
  {
    //FIO1PIN |=LEDSTART;//On  LED
    // первая строка длина волны
    switch (GetCombLS(SettingRefl.SetModeLW))
    {
    case 1: // одиночные длины волн
      sprintf(Str,"t1.txt=\"%d%s\"яяя",GetLengthWaveLS (0),MsgMass[18][CurrLang]); // nm
      break;
    case 2:
      sprintf(Str,"t1.txt=\"%d%s\"яяя",GetLengthWaveLS (1),MsgMass[18][CurrLang]); // nm
      break;
    case 4:
      sprintf(Str,"t1.txt=\"%d%s\"яяя",GetLengthWaveLS (2),MsgMass[18][CurrLang]); // nm
      break;
    case 3:
      sprintf(Str,"t1.txt=\"%02d,%02d\"яяя",(GetLengthWaveLS(0)%1000)/10,(GetLengthWaveLS(1)%1000)/10); // 1 и 2
      break;
    case 5:
      sprintf(Str,"t1.txt=\"%02d,%02d\"яяя",(GetLengthWaveLS(0)%1000)/10,(GetLengthWaveLS(2)%1000)/10); // 1 и 3
      break;
    case 6:
      sprintf(Str,"t1.txt=\"%02d,%02d\"яяя",(GetLengthWaveLS(1)%1000)/10,(GetLengthWaveLS(2)%1000)/10); // 2 и 3
      break;
    case 7:
      sprintf(Str,"t1.txt=\"%02d,%02d,%02d\"яяя",(GetLengthWaveLS(0)%1000)/10,(GetLengthWaveLS(1)%1000)/10,(GetLengthWaveLS(2)%1000)/10); // все 3
      break;
    default: // комбинации
      sprintf(Str,"t1.txt=\"?NON?\"яяя"); // test
      
      break;
    }
    
    // первая строка длина волны
//    if (GetSetModeLW(0)) 
//     // sprintf(Str,"t1.txt=\"%s\"яяя",MsgMass[21][CurrLang]); // auto
//      sprintf(Str,"t1.txt=\"13,15,85\"яяя"); // test
//    else
//      sprintf(Str,"t1.txt=\"%d%s\"яяя",GetLengthWaveLS (GetPlaceLS(CURRENT)),MsgMass[18][CurrLang]); // nm
    NEX_Transmit((void*)Str);// 
    // вторая строка - диапазон
    if (!GetSubModRefl ()) // manual
    {
      if ((GetIndexLN()==0)&&(IndexSmall==0))
        sprintf(Str,"t3.txt=\"0.5%s\"яяя", MsgMass[20][CurrLang]); // km
      else 
        sprintf(Str,"t3.txt=\"%d%s\"яяя", GetLengthLine(GetIndexLN()),MsgMass[20][CurrLang]); // km
    }
    else 
    {
      sprintf(Str,"t3.txt=\"%s\"яяя", MsgMass[21][CurrLang]);
      SetIndexLN (0); // установка индекса длины линии
    }
    NEX_Transmit((void*)Str);// 
    
    // третья сторка - длительность импульса
    if (!GetSubModRefl ()) sprintf(Str,"t5.txt=\"%d%s\"яяя", GetWidthPulse(GetIndexIM()),MsgMass[23][CurrLang]);//ns
    else 
    {
      sprintf(Str,"t5.txt=\"%s\"яяя", MsgMass[21][CurrLang]);
      SetIndexIM (0); // установка индекса длины импульса
    }
    NEX_Transmit((void*)Str);// 
    // четвертая строка в зависимости от положения длины волны
    // строка отображения времени измерения или номера волокна
    // 22.11.2022 изменение индикации строк 4-5 надо привязать к режиму с автоматическим сохранением
    //if (GetSetModeLW(0))
    if (g_AutoSave)
    {
      sprintf(Str,"t7.txt=\"%03d\"яяя", NumFiber);//c
    }
    else
    {
      sprintf(Str,"t7.txt=\"%d%s\"яяя", GetTimeAvrg(GetIndexVRM()),MsgMass[4][CurrLang]);//c
      if (GetIndexVRM() == 5)
        sprintf(Str,"t7.txt=\"%s\"яяя", MsgMass[25][CurrLang]);// "непр."
      if (GetIndexVRM() == 4)
        sprintf(Str,"t7.txt=\"%s\"яяя", MsgMass[26][CurrLang]);//"разов."
    }
    NEX_Transmit((void*)Str);// 
    // пятая строка - индекс измерения или префикс при авто
    // строка отображения коэфф преломления или префикса имани файла
    //if (GetSetModeLW(0))
    //123
    // здесь вызываются функции для прорисовки строки в разных режимах
    // надо разобраться , пока пишу ТЕСТ
    if (g_AutoSave)
    {
      sprintf(Str,"t9.txt=\"%03d/%03d\"яяя",GetNumTraceSaved(0),MAXMEMALL);// сколько занято
    }
    else
    {
      sprintf(Str,"t9.txt=\"%.4f\"яяя",GetIndexWAV ());
    }
    //  sprintf(Str,"t9.txt=\"ТЕСТ\"яяя");// надо убрать
    NEX_Transmit((void*)Str);// 
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
    sprintf(Str,"t%d.bco=GREENяяя",FrSetSetupOTDR<<1); // зеленый ?
    NEX_Transmit((void*)Str);// 
    // покраска поля t1 в зависимости от режима
    if(g_AutoSave)
        sprintf(Str,"t1.bco=YELLOWяяя"); // оранжевый
    else
        sprintf(Str,"t1.bco=WHITEяяя"); // белый

    NEX_Transmit((void*)Str);//
    
    //FIO1PIN &=~LEDSTART;//Off  LED
    g_NeedScr = 0;
  }
  // кнопка возврата в меню
  if ((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))
  {
    SetMode(ModeMainMenu);
    myBeep(10);
    ReSaveWAV_SC (); // пересохраняем если есть изменения
    ModeDevice = MODEMENU;
    CmdInitPage(1);// посылка команды переключения окна на MainMenu (возврат)
  }
  if (rawPressKeyS) // START Measure из режима установок рефлектометра
  {        
    myBeep(10);
    // сохраняем установки измерения если запустили
    EEPROM_write(&SettingRefl, ADR_ReflSetting, sizeof(SettingRefl));
    ReSaveWAV_SC (); // пересохраняем если есть изменения
    LSEL0(0);
    LSEL1(0);
    SlowON();
    ReturnMemView = 0; // зададим признак возврата в режим установок рефлектометра если пришли отсюда
    //POWALT(ON);
    //POWREF (ON);
    //POWDET(ON);
    SetMode(ModeStartOTDR);
    ModeDevice = MODEMEASURE;
    //IndexVerSize  = 0;// установка вертикального размера отображения рефлектограммы ( самый крупный)
    SubModeMeasOTDR = SETPOWER;
//123    SSPInit_Any(SPI_ALT); // Инициализация SSP для управления ALTERA (порт 1 та что на плате отладочной)
    rawPressKeyS=0;
    CntLS=0;
    //CreatDelay (30000); // 3.3 мС
  HAL_Delay(3);
  CmdInitPage(16);// посылка команды переключения окна на начало измерения Measuring
  }
  if(NeedKeyB ) // необходимость переключения в клавиатуру для редактирования PreFix
  {
    // согласно выбранного языка вызывем клавиатуру
    if (CurrLang) 
      // посылка команды переключения окна на EnglishPreF Keyboard 
      CmdInitPage(31); //(22)
    else
      // посылка команды переключения окна на RussianPreF Keyboard 
      CmdInitPage(32); //(23)
    NeedKeyB=0; 
  }
  
  
}

void ModeErrorOTDR(void) // режим отображения "Излучение на входе" CHECK_OFF
{
  char Str[32];
  //BYTE CurrLang;
  
  //CurrLang=GetLang(CURRENT);
  //CreatDelay(300000);
  if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
  {
    myBeep(1);
  }
  //ClrKey (BNS_MASK); // сброс нажатых клавиш
  if (g_FirstScr)
  {
    // здесь заполняем данными поля нового индикатора
    // не требущие изменения при первичной инициализации
    sprintf(Str, "t0.txt=\"%s\"яяя", MsgMass[36][CurrLang]); // Внимание
    NEX_Transmit((void*)Str);    //
    sprintf(Str, "t1.txt=\"%s\"яяя", MsgMass[37][CurrLang]); // Излучение на входе [ДА]
    NEX_Transmit((void*)Str);    //
    sprintf(Str, "t2.txt=\"%s\"яяя", MsgMass[38][CurrLang]); // для продолжения
    NEX_Transmit((void*)Str);    //
    sprintf(Str, "t3.txt=\"%s\"яяя", MsgMass[39][CurrLang]); // Нажмите <S> 
    NEX_Transmit((void*)Str);    //
    
    g_FirstScr = 0;
    g_NeedScr = 1;
  }
  if (g_NeedScr)
  {
    // здесь заполняем данными поля нового индикатора
    // по результатам изменений вызваныйх обработчиком клавиатуры
    //sprintf(Str,"t3.bco=REDяяя"); // белый
    //NEX_Transmit((void*)Str);//
    
    // раскрашивание поля выбора 
    // закрасим бэкграунды  и установим требуемый
    // код подсветки требуемой строки если есть есть маркер строки
    g_NeedScr = 0;
  }
  // пустой контроль клавиатуры (нажатия кнопок)
  
  if (rawPressKeyS)
  { 
    myBeep(10);
    //g_NeedScr = 1; // Need reDraw Screen
    SetMode(ModeSetupOTDR);
    GetSetModeLW(-1); // сброс счетчика длин волн источников, так как ошибка
    ModeDevice = MODESETREFL;
    SubModeMeasOTDR = NOTMODE;
    GetPlaceLS(CURRENT);
    rawPressKeyS=0;
    // посылка команды возврата на OTDR (Установки рефлектометра)
    CmdInitPage(2);
    
  }
  //CreatDelay (400000); // 3.3 мС
    HAL_Delay(40);

}

#pragma optimize=none  
void ModeStartOTDR(void) // режим накопления рефлектометра
{
  DWORD Mean=0;
  DWORD Noise ;
  //static DWORD LengthLine = 0;
  
  DWORD LocalMax = 0;
  static BYTE SizeRepit = 0;// число повторений для данного набора измерений с сохраненеием
  static BYTE InputOK = YES;
  static BYTE LengthOK = YES;
  static BYTE IndxAddBad = 0; //пересчет индекса длины при отражениях далее заявленной длины линии

  static BYTE ShadowIndexLN; //
  static BYTE ShadowIndexIM;
  static BYTE NeedResetIM = 0; // признак необходимости переустановки длительности импульса после первого измерения
  //static DWORD CurrentSumDelay; // итоговая суммарная задержка при изменении длины линии для режима накоплений
  //static unsigned CntAvrg; // счетчик накоплений (общий по периодам по 3 сек)
  char Str[22];
  //BYTE CurrLang;
  //CurrLang=GetLang(CURRENT);
  switch (SubModeMeasOTDR)
  {
  case SETPOWER: // установка питания (запуск прогревочного измерения) (импульса нет)
    // тест вывод определения начала измерения
    //sprintf(Str,"ooo\r");//c
    //UARTSend0 ((BYTE*)Str, strlen (Str));
    
    LSEL0(0);
    LSEL1(0);
    if(g_FirstScr) // рисуем новый индикатор
    {
      sprintf(Str,"t0.txt=\"%s\"яяя", MsgMass[28][CurrLang]); // Wait
      NEX_Transmit((void*)Str);// 
      // остальное почистим
      sprintf(Str,"t1.txt=\"\"яяя"); // 
      NEX_Transmit((void*)Str);// 
      sprintf(Str,"t2.txt=\"\"яяя"); // 
      NEX_Transmit((void*)Str);// 
      sprintf(Str,"t3.txt=\"\"яяя"); // 
      NEX_Transmit((void*)Str);// 
      sprintf(Str,"t4.txt=\"\"яяя"); // 
      NEX_Transmit((void*)Str);// 
      sprintf(Str,"t5.txt=\"\"яяя"); // 
      NEX_Transmit((void*)Str);// 
      sprintf(Str,"t6.txt=\"\"яяя"); // 
      NEX_Transmit((void*)Str);// 
      sprintf(Str,"t7.txt=\"\"яяя"); //
      NEX_Transmit((void*)Str);// 
      sprintf(Str,"t8.txt=\"\"яяя"); //
      NEX_Transmit((void*)Str);// 
      sprintf(Str,"t9.txt=\"\"яяя"); //
      NEX_Transmit((void*)Str);// 
      sprintf(Str,"t10.txt=\"\"яяя"); //
      NEX_Transmit((void*)Str);// 
      g_FirstScr = 0;
      g_NeedScr = 1;
    }
    SubModeMeasOTDR = INPUTOTDR;
    ShadowIndexLN = GetIndexLN(); // запоминаем установленную длину, определяет частоту съема
    ShadowIndexIM = GetIndexIM();
    PointsPerPeriod = NumPointsPeriod[GetIndexLN()]; // SetPointsPerPeriod( ... );
    memset( RawData, 0, RAWSIZE * sizeof(DWORD) );
    myBeep (0);
    //123    disable_timer ( 0 );
    LED_START(1);//On  LED
    CntNumAvrg = 0; // обнуляем счетчик накоплений
    HV_LOW(ON); //ON LOW HIGH VOLT
    HV_SW(ON); // ON HIGH VOLT
    // запустим таймер 2 и посчитаем время
    //123    reset_timer(2);
    //123    enable_timer(2);
    CurrTimeAccum = 0; // текущее время накопления 1 мкС
    EnaTimerAccum = 1; // разрешаем считать время накопления
    
    Averaging (5,0,0);//запуск прогревочного измерения
    break;
  case INPUTOTDR:
    LSEL0(0);
    LSEL1(0);
    if(g_NeedScr) //
    {
      sprintf(Str,"t1.txt=\"%s\"яяя",MsgMass[34][CurrLang]); // 
      NEX_Transmit((void*)Str);// 
      sprintf(Str,"t6.txt=\"???\"яяя"); // 
      NEX_Transmit((void*)Str);// 
      g_NeedScr = 0;  
    }
    PointsPerPeriod = NumPointsPeriod[GetIndexLN()]; // SetPointsPerPeriod( ... );
    memset( RawData, 0, RAWSIZE * sizeof(DWORD) );
    CntNumAvrg = 0; // обнуляем счетчик накоплений
    Averaging (200,0,0);
    for (int i=0; i<RAWSIZE; ++i)
    {
      Mean +=RawData[i];
    }
    Mean = Mean/RAWSIZE;
    Mean = Mean/200;
    //Mean = 200; // тест для проверки батарейки
    if (Mean > 2000)
    {
      POWDET(OFF);
      POWREF (OFF);
      SetMode(ModeErrorOTDR);
      ModeDevice = MODEERREFL;
      SubModeMeasOTDR = NOTMODE;
      //123      SSPInit_Any(MEM_FL1); // Востанавливаем Инициализацию SSP для управления внешней FLASH (порт 1 та что на плате отладочной)
      //123      enable_timer ( 0 );
      myBeep(20);
      LED_START(0);//Off  LED
      // посылка команды переключения окна на Сообщение об излучении на входе ErrPowerInput
      //CmdInitPage(19);
      NeedReturn = 19;
      //g_FirstScr=1;
    }
    else
    {
      HV_LOW(OFF); //ON  HIGH VOLT FULL
      HV_SW(ON); // ON HIGH VOLT
      SubModeMeasOTDR = CALKORLBEG;
    }
    break;
    // добавим измерение ORL (в тест режиме )
    // 40 nS 16 km, не взирая на установки
  case CALKORLBEG:
    // Здесь функция измерения ORL 
    g_VolORL = MeasORL (1000, 0);
    LED_START(1);//On  LED - продолжаем измерение
    // печатаем что НЕТ излучения на входе
    sprintf(Str,"t6.txt=\"%s\"яяя",MsgMass[35][CurrLang]); // 
    NEX_Transmit((void*)Str);// 
    
    g_NeedScr =1;
    SubModeMeasOTDR = QUALITYBEGIN;
    break;
  case QUALITYBEGIN:
    SetIndexWAV(GetWAV_SC(GetPlaceLS(CURRENT))); // устанавливаем коэфф. преломления выбранной длины волны
    
    //GetPlaceLS(CURRENT);
    SetIndexLN(0);
    SetIndexIM(0);
    if(g_NeedScr)
    {
      //sprintf(Str,"t6.txt=\"%s\"яяя",MsgMass[35][CurrLang]); // 
      //NEX_Transmit((void*)Str);// 
      sprintf(Str,"t2.txt=\"%s\"яяя",MsgMass[32][CurrLang]); // 
      NEX_Transmit((void*)Str);// 
      sprintf(Str,"t7.txt=\"%s\"яяя",MsgMass[33][CurrLang]); // 
      NEX_Transmit((void*)Str);// 
      
      g_NeedScr = 0;
    }
    PointsPerPeriod = NumPointsPeriod[GetIndexLN()]; // SetPointsPerPeriod( ... );
    InputOK = YES; // обнуляем признак плохого разъема
    memset( RawData, 0, RAWSIZE * sizeof(DWORD) );
    CntNumAvrg = 0; // обнуляем счетчик накоплений
    Averaging (BEGZONEAVRG,6,0); // измеряем 2км редко (128км)
    LocalMax = 0;
    Mean = (DWORD)(5000.0*log10(1023*BEGZONEAVRG)); // максимальный уровень при данном накопленнн
    for (int i=0; i<BEGZONEPNT; ++i)
    {
      LocalMax +=LogData[i];
    }
    LocalMax=LocalMax/BEGZONEPNT; // средний уровень за 10 точек начала
    if (LocalMax<800) 
      InputOK = NO; // точки лежат выше -3дб = плохой разъем
    g_NeedScr = 1;
    SubModeMeasOTDR = SEARCHENDLINE;
    break;
  case SEARCHENDLINE:
    SetIndexLN(6); //128km
    SetIndexIM(4); //1000ns // вроде как ошибка была (было 5) ??? 31.08.2012
    if(g_NeedScr)
    {
      sprintf(Str,"t7.txt=\"%s\"яяя",((InputOK)?("OK"):("???"))); // 
      NEX_Transmit((void*)Str);// 
      sprintf(Str,"t3.txt=\"%s\"яяя",MsgMass[31][CurrLang]); // длина линии
      NEX_Transmit((void*)Str);// 
      sprintf(Str,"t8.txt=\"???\"яяя"); //  состояние - неопределено
      NEX_Transmit((void*)Str);// 
      
      g_NeedScr = 0;
    }
    PointsPerPeriod = NumPointsPeriod[GetIndexLN()]; // SetPointsPerPeriod( ... );
    memset( RawData, 0, RAWSIZE * sizeof(DWORD) );
    CntNumAvrg = 0; // обнуляем счетчик накоплений
    Averaging (500,0,0);
    CurrentSumDelay = 0;
    Noise = 0;
    LocalMax = 0;
    // считаем текущий и шум и выбираем уровень контроля наличия сигнала  больше на 3дБ
    for (int i = 0;i<100;++i)
    {
      if (RawData[RAWSIZE-100+i]>LocalMax) LocalMax = RawData[RAWSIZE-100+i];
      Noise += RawData[RAWSIZE-100+i];
    }
    Noise = Noise/100;
    RawData[0] = Noise + (LocalMax-Noise)*2;
    // считаем логарифм
    /*
    for (int i= 2; i<RAWSIZE; ++i)
    {
    if (RawData[i]<Noise)RawData[i]=Noise+1;
    RawData[i] = 5000.0*log10((double)(RawData[i]));
    
  }
    */
    LengthOK = YES;
    CurrentSumDelay = 1;
    Mean = Scan( RawData, RAWSIZE , RawData[0] );// точка отражения , как бы конц линии в режиме 128 км
    //LengthLine = Mean;
    //for (Mean=RAWSIZE-1; ((Mean>90) && (RawData[Mean]<RawData[0])); --Mean);
    IndxAddBad = IndexSeek(Mean); // какой индекс надо поставить при этом отражении
    if (IndxAddBad>ShadowIndexLN) 
    {
      LengthOK = NO;
      // надо перестроить измеритель на нужную длину
      // но при этом не трогать частоту съема
      // добавка к периоду в тиках делая (83.33нс)
      DWORD tmp = RAWSIZE/NumPointsPeriod[ShadowIndexLN];
      if (Mean>tmp) CurrentSumDelay = (Mean-tmp)*4;
    }
    else 
      {// Линия выбрана правильно
      Mean = KeyPoints[ShadowIndexLN]-1;
      IndxAddBad = 0; // нужен для установки другого диапазона при измерениях
      }
    // Mean - задает период повторения сборщика одной точки
    // число накоплениий за 3 сек можно сосчитать как 
    // время промежуточного вывода на экран 28 мС
    // время одного прохода в заданой длине Mean*333.33 + 14000*NumPointsPeriod[ShadowIndexLN]
    TimeMeasure3S  = 2750000; // uS
    if (RemoutCtrl) TimeMeasure3S  = 3000000; //uS
    // рассчитаем приблизительное число накоплений
    if(LengthOK) // линия правильная,  
    NumAvrg = (unsigned)(TimeMeasure3S/((NumPointsPeriod[ShadowIndexLN]+1)*TimeRepitOfLN[ShadowIndexLN]+260));//*NumPointsPeriod[ShadowIndexLN])
    // линия плохая
    else
    NumAvrg = (unsigned)(TimeMeasure3S/((NumPointsPeriod[ShadowIndexLN]+1)*TimeRepitOfLN[IndxAddBad]+260));//*NumPointsPeriod[ShadowIndexLN])
      
      
                         // число накоплений как деление времени 2.75 сек на промеренную длину в тиках сбора 333ns 
    //  NumAvrg = (unsigned)(7300000L/(Mean+10)); // было +30
    // установить соотв. задержку для полученной линии 
    if (GetSubModRefl()) // автоматический - добавку не делаем
    {
      ShadowIndexLN = IndexSeek(Mean);
      ShadowIndexIM = IndexSeek(Mean);
     // NumAvrg = (unsigned)(8250000L/(KeyPoints[ShadowIndexLN]+50)); // число накоплений , было +30
    //NumAvrg = (unsigned)(TimeMeasure3S/(NumPointsPeriod[ShadowIndexLN]*TimeRepitOfLN[ShadowIndexLN]));//*NumPointsPeriod[ShadowIndexLN])
    NumAvrg = (unsigned)(TimeMeasure3S/((NumPointsPeriod[ShadowIndexLN]+1)*TimeRepitOfLN[ShadowIndexLN]+260));//*NumPointsPeriod[ShadowIndexLN])
      CurrentSumDelay = 1;
    }
    SetIndexLN(ShadowIndexLN);
    SetIndexIM(ShadowIndexIM);
    PointsPerPeriod = NumPointsPeriod[GetIndexLN()]; // SetPointsPerPeriod( ... );
    //ValueDS = (unsigned)(MultIndex[GetIndexLN()]*(ADCPeriod*50000)/PointsPerPeriod); //  устанавливаем значения DS для установленного режима измерения
    ValueDS = GetValueDS(); //  устанавливаем значения DS для установленного режима измерения
    // корректировка накоплений в соответствии с числом точек на период
    //NumAvrg = NumAvrg/PointsPerPeriod; // расчетное число накоплений за 1 секунду
    // для нового индикатора
    //CurrentSumDelay += DelayPeriod[GetIndexLN()];
    g_NeedScr = 1;
    SubModeMeasOTDR = LASTCHECK;
    break;
  case LASTCHECK:
    // New LCD
    if(g_NeedScr)
    {
      if ((GetIndexLN()==0)&&(IndexSmall==0))
        sprintf(Str,"t8.txt=\"0.5%s%s\"яяя",MsgMass[20][CurrLang],((LengthOK)?(" "):("!")));//Длина линии: XXкм
      else
        sprintf(Str,"t8.txt=\"%d%s%s\"яяя",GetLengthLine(GetIndexLN()),MsgMass[20][CurrLang],((LengthOK)?(" "):("!")));//Длина линии: XXкм
      //sprintf(Str,"t6.txt=\"%s\"яяя",MsgMass[35][CurrLang]); // 
      NEX_Transmit((void*)Str);// 
      sprintf(Str,"t4.txt=\"%s\"яяя",MsgMass[29][CurrLang]); // Длит.импульса: XXнс
      NEX_Transmit((void*)Str);// 
      sprintf(Str,"t9.txt=\"%d%s\"яяя",GetWidthPulse(GetIndexIM()),MsgMass[23][CurrLang]); // Длит.импульса: XXнс
      NEX_Transmit((void*)Str);// 
      sprintf(Str,"t5.txt=\"%s\"яяя",MsgMass[30][CurrLang]); //Идет измерение: XXс 
      NEX_Transmit((void*)Str);// 
      sprintf(Str,"t10.txt=\"%d%s\"яяя",GetTimeAvrg(GetIndexVRM()),MsgMass[4][CurrLang]); //Идет измерение: XXс 
      NEX_Transmit((void*)Str);// 
      
      g_NeedScr = 0;
    }
    
    memset( RawData, 0, RAWSIZE * sizeof(DWORD) ); // обнуляем массив накоплений
    FinAvrg = (GetTimeAvrg(GetIndexVRM())*NumAvrg)/3;
    CntNumAvrg = 0; // обнуляем счетчик накоплений
    if (GetIndexVRM()==4) NumAvrg = NumAvrg-10; // разовый ~1,5 cek
    ShadowIndexIM = GetIndexIM(); // запоминаем индекс установленного импульса
    if ((ShadowIndexIM > 4) && (GetIndexVRM()!=4))
    {
      SetIndexIM(4); //1000ns для первого измерения устанавливаем импульс если задан большой 3...10мкс
      //SetIndexShadowIM (ShadowIndexIM); // установка мертвой зоны для теневого импульса
      SW_FLTR(ON);
      NeedResetIM = 1;
    }
    // поуправляем выходом DAC
    //p0.26 (PHLD) 
    //PINSEL1 &=~0x00300000; //set P0.26 us GIO
    //PINSEL1 |=0x00200000; // set P0.26 us AOUT
    //SetLvlDAC (0);
    //123
    // здесь какой-то тестовый вывод по "кнопке" не обработанных данных
    // во внешний мир 1 накопления, зачем не помню
    //    if (!(FIO2PIN & BTN_DOWN))
    //    {
    //      Averaging (1,CurrentSumDelay,0);// через 3 сек первый результат
    //      for (int i = 0; i<RAWSIZE;++i)
    //      {
    //        sprintf(Str,"%u\n", RawData[i]);//c
    //        UARTSend0 ((BYTE*)Str, strlen (Str));
    //      }
    //      sprintf(Str,"\r");//c
    //      UARTSend0 ((BYTE*)Str, strlen (Str));
    //      
    //      memset( RawData, 0, RAWSIZE * sizeof(DWORD) ); // обнуляем массив накоплений
    //      CntNumAvrg = 0; // обнуляем счетчик накоплений
    //      
    //    }
    // запустим таймер 2 и посчитаем время
    //123    reset_timer(2);
    //123    enable_timer(2);
    CurrTimeAccum = 0;
    EnaTimerAccum = 1;
    
    // можно задать 3000 это как бы время накопления за 3 сек
    //    CmdInitPage(17);// посылка команды переключения окна на DrawOTDR
    //    sprintf(Str,"t0.txt=\"%s\"яяя",MsgMass[30][CurrLang]); //Идет измерение: XXс 
    //    NEX_Transmit((void*)Str);// 
    
    g_NeedScr = 1; // используем его для переключения в режим отображения накоплений
    // переключим окно для нового индиктора и сбросим признак
    Averaging (NumAvrg,IndxAddBad,1);// через 3 сек первый результат
    // установка режима накопления 
    SubModeMeasOTDR = AVERAGING;
    // можно нарисовать окно с рефлектограммой
    
    //PointsPerPeriod = NumPointsPeriod[GetIndexLN()]; // SetPointsPerPeriod( ... );
    //memset( RawData, 0, RAWSIZE * sizeof(DWORD) );
    // расчет счетчика накоплений в зависимости от установленного времени усреднения
    break;
    //  case ONESUMMONLY:
    //    
    //    break;
  case AVERAGING:
    //sprintf(Str,"xxx\r");//c
    //UARTSend0 ((BYTE*)Str, strlen (Str));
    
    // сюда попадаем после первого измерения (первые 3 сек)
    // если есть склейка, сохраняем рефлектограмму для склейки
    //123    
    //    if (NeedResetIM) // восстановим импульс и сохраним рефлектограмму
    //    {
    //      SetIndexIM(ShadowIndexIM);
    //      SSPInit_Any(MEM_FL1); // Востанавливаем Инициализацию SSP для управления внешней FLASH (чтобы записать)
    //      SpliceWRITE(250); 
    //      LogData[12] = FlashReadSpliceData(12);
    //      SSPInit_Any(SPI_ALT); // Инициализация SSP для управления ALTERA (переключаем обратно управление )
    //      memset( RawData, 0, RAWSIZE * sizeof(DWORD) ); // обнуляем массив накоплений
    //      CntNumAvrg = 0; // обнуляем счетчик накоплений
    //      NeedResetIM = 0;
    //    }
    if (GetIndexVRM()==4) // разовый ~1,5 cek ... режим Реал тайм
    {
      CntNumAvrg = 0; // обнуляем счетчик накоплений
      memset( RawData, 0, RAWSIZE * sizeof(DWORD) );
      //123      reset_timer(2); //04.02.2014
      //123      enable_timer(2);
      CurrTimeAccum = 0;
      EnaTimerAccum = 1;
      
    }
    Averaging (NumAvrg,IndxAddBad,1); //запуск текущего накопления
    if ((GetCntNumAvrg() >= FinAvrg)||((CurrTimeAccum/1000)>GetTimeAvrg(GetIndexVRM()))) //закончили накопление рисуем рефлектограмму
    {
      
      RawData[RAWSIZE-1]= CurrTimeAccum;// что-то хотим сохранить в данных рефлектограммы (время съема)
      EnaTimerAccum = 0;
      
      HV_LOW(ON); //ON LOW HIGH VOLT
      HV_SW(OFF); // OFF HIGH VOLT
      POWDET(OFF);
      POWREF (OFF);
      // востановим пин управления лазерами
      //PINSEL1 &=~0x00300000; //set P0.26 us GIO
      
      SetMode(ModeDrawOTDR);
      // вызовем окно просмотра рефлектограммы  
      // посылка команды переключения окна на DrawOTDRview (вызов)  
      CmdInitPage(18);
      ModeDevice = MODEREFL;
      ReturnModeViewRefl = SETPARAM;//VIEWMEM -  чтобы вернуться в в установки
      SubModeMeasOTDR = NOTMODE;
      GetPlaceLS(CURRENT);
      rawPressKeyS=0;
      //123      SSPInit_Any(MEM_FL1); // Востанавливаем Инициализацию SSP для управления внешней FLASH (порт 1 та что на плате отладочной)
      //123      enable_timer ( 0 );
      myBeep(30);
      LED_START(0);//Off  LED
      //Light(GetTimeLight(SetIndexLight(0))); // включаем если была подсветка
      //IndexVerSize  = 0;// установка вертикального размера отображения рефлектограммы ( самый крупный)
      //GetSetHorizontScale (5); // установка Вся картинка на экран
      // закончили полное накопление ... попробуем сшить (если установлены "жирные" импульсы)
      // Поработаем с данными уже отлогарифмированными после накоплений
      if(EnaCorrMZ)// если разрешено корректировка мертвых зон!
      {
        // здесь можно попробовать улучшить мертвую зону по событию, обострить пик импульса
        // только при (PointsPerPeriod==48), алгоритм следующий 
        // условие сигнал менее 18000 но не более 700 ( т.е. не в шумах, может даже 17000)
        // точки дальше 20-
        // ищем пик, смотрим например 7 точек
        CntPointPick=0;
        if((GetIndexIM()==0)&&(PointsPerPeriod==48)) // только на мелком импульсе
        {
          for (int i=0; i<OUTSIZE; i++)
          { 
            if((LogData[i]<16000)&&(LogData[i]>3700)&&(i>20)) // данные в нужном диапазоне
            {
              unsigned short aLD,bLD,cLD,dLD,eLD,fLD,gLD;
              aLD = LogData[i-6];
              bLD = LogData[i-5];
              cLD = LogData[i-4];
              dLD = LogData[i-3];// pick
              eLD = LogData[i-2];
              fLD = LogData[i-1];
              gLD = LogData[i];
              if((CntPointPick<15)&&(dLD<cLD)&&(dLD<eLD))// есть вершинка,
              {
                if((cLD<bLD)&&(eLD<fLD))// посмотрим есть ли склоны от вершинки
                {
                  if((bLD<aLD)&&(fLD<gLD))// основания тоже есть?
                  {
                    if(((gLD-dLD)>1000)&&((aLD-dLD)>1000))// имеем "колокол" и у него есть высота, попробуем его зафиксировать
                    {
                      PointsPick[CntPointPick]=dLD;
                      PointsPickIndx[CntPointPick]=i-3;
                      CntPointPick++;
                      // скорректируем уровень вершинки на 0.75 дБ
                      LogData[i-3]-=753;
                      // может сразу выведем значение 
                    }
                  }
                }
              }
            }
          }
          // тест вывод точек пиков
          //          if(CntPointPick)
          //          {
          //            char BufOut[32];
          //            for(int j=0;j<CntPointPick;j++)
          //            {
          //              sprintf(BufOut,"%d. %d\r",PointsPickIndx[j],PointsPick[j]);//c
          //              UARTSend0 ((BYTE*)BufOut, strlen (BufOut));
          //            }
          //          }
        }
      }
      
      if (GetIndexIM()>4)
      {
        SpliceProg (PointsInImpulse(0));
      }
      TimeSaveOTDR = RTCGetTime(); // сохраняем время сбора
      SetNumAverag (FinAvrg); // Запись числа накоплений в память рефлектограмм
      SaveLogNoise(); // сохраняем уровень шумов(перед импульсом в дБ (NF)
      //123      TraceERASE(0); // стирание трассы (0) в старом для сохранеия параметров измерения, что бы
      //123      TraceWRITE(0); // запись трассы (0) при следующем включении востановить параметры по этой (нулевой)
      
      // рефлектограмме, в новом надо по другому...
      SaveFileSD(0); // сохраняем в НУЛЕВУЮ
      IndexEvents=0; // указатель на событие (характеристика линии)
      if (SetGetMonEna(255)) PrintEventTbl(); // если установлен признак выдаем по RS таблицу событий
      // если установлен признак автоматического измерения, надо повторить измерение на новой длинне волны и сохранить текущее
      //if (GetSetModeLW(0)) // автоматический режим измерения на длинах волн (старое)
      //
      if (g_AutoSave) //17.11.2022 автоматический режим СОХРАНЕНИЯ измерения 
        //на длинах волн или на одной
        
      { 
        // подготавливаем имя файла в коммнтарии
        memcpy( FileNameOTDR, PrefixFileNm, 10 );
        sprintf(Str,"_%03d_%04d",NumFiber,GetLengthWaveLS (GetPlaceLS(CURRENT))); // номер волокна и длинна волны на которой измерялось (8 байт)
        memcpy( &FileNameOTDR[10], &Str, 9 ); // 
        memcpy( CommentsOTDR,FileNameOTDR, 20 );
        // сохраняем рефлектограмму в памяти 
        //123 !!!!        if (SaveTrace())
        SaveFileSD(1);
        if (1)
        {
          CntLS++;
          // проверяем не закончился ли цикл измерений трассу сохранили
          switch(GetCombLS(SettingRefl.SetModeLW))
          {
          case 1:
          case 2:
          case 4:
            SizeRepit=1;
            break;
          case 3:
            SizeRepit=2;
            SettingRefl.SW_LW = 1; // установка следующего индекса рабочего лазера
            
            break;
          case 5:
            SizeRepit=2;
            SettingRefl.SW_LW = 2; // установка следующего индекса рабочего лазера
            break;
          case 6:
            SizeRepit=2;
            SettingRefl.SW_LW = 2; // установка следующего индекса рабочего лазера
            break;
          case 7:
            SizeRepit=3;
            if (CntLS<2)
              SettingRefl.SW_LW = 1; // установка следующего индекса рабочего лазера
            else
              SettingRefl.SW_LW = 2; // установка следующего индекса рабочего лазера
            
            break;
          default:
            SizeRepit=0;
            break;
          }
          
          // для комбинационных измерений можно проверять по установленному признаку
          // выбранной длины волны или комбинации
          //if (++CntLS<GetSetNumLS(0)) // еще есть длина волны на которой можно измерить (старое)
          if (CntLS<SizeRepit) // 17.11.2022 (новое) еще есть длина волны на которой можно измерить
          {
            // новая длина волны, это переключение по старому, без возможных комбинаций 
            //просто все разрешенные подряд
            //
            GetPlaceLS(CURRENT); // выбираем следующую длину волны из списка разрешенныз
            myBeep(10);
            LSEL0(0);
            LSEL1(0);
            SlowON();
            //POWALT(ON);
            //POWREF (ON);
            //POWDET(ON);
            SetMode(ModeStartOTDR);
            ModeDevice = MODEMEASURE;
            CmdInitPage(16);// посылка команды переключения окна на начало измерения Measuring
            //IndexVerSize  = 0;// установка вертикального размера отображения рефлектограммы ( самый крупный)
            SubModeMeasOTDR = SETPOWER;
            //123 SSPInit_Any(SPI_ALT); // Инициализация SSP для управления ALTERA (порт 1 та что на плате отладочной)
          }
          else // выход в режим установок рефлектометра по окончании цикла измерений
          {
            if (NumFiber<999) NumFiber++;
            else NumFiber =1;
            CntLS = 0;// ??? так как автомат то надо оставить 0
            SetMode(ModeSetupOTDR);  //  переход в режим установок рефлектометра (было)
            ModeDevice = MODESETREFL;
            SubModeMeasOTDR = NOTMODE;
            // переключатель вывода в зависимости от комбинации
            SetPlaceNew(0); // без сброса установки авто сохранения
            
            CmdInitPage(2);// посылка команды переключения окна на OTDR (Установки рефлектометра)
          } 
        }
        else // нет места для рефлектограмм - выходим в просмотр с полной памятью
        {
          SetMode(ModeMemoryOTDR);
          ModeDevice = MODEMEMR;
          myBeep(10);
          ModeMemDraw = MEMFULL;
          // посылка команды переключения окна на Mem_OTDR_garaph (вызов)  
          CmdInitPage(13);
          
        }
      }
      if (RemoutCtrl) // выдача окончания сбора если запускали дистанционно
      {
        sprintf(Str,"END\r");//c
        RemoutCtrl = 0;
        UARTSendExt ((BYTE*)Str, strlen (Str));
        ClearRS();
      }
      
    }
    break;
  }
  // прерывание по кнопке S возврат в настройку рефлектометра
  if (rawPressKeyS)
  { 
    myBeep(10);
    HV_LOW(ON); //ON LOW HIGH VOLT
    HV_SW(OFF); // OFF HIGH VOLT
    POWDET(OFF);
    POWREF (OFF);
    // востановим пин управления лазерами
    //PINSEL1 &=~0x00300000; //set P0.26 us GIO
    
    GetPlaceLS(CURRENT);
    rawPressKeyS=0;
    //123 SSPInit_Any(MEM_FL1); // Востанавливаем Инициализацию SSP для управления внешней FLASH (порт 1 та что на плате отладочной)
//123    enable_timer ( 0 );
    myBeep(20);
    LED_START(0);//Off  LED
    //SetVerticalSize (22000);// установка вертикального размера отображения рефлектограммы
//123    Light(GetTimeLight(SetIndexLight(0))); // включаем если была подсветка- not need
    if ((SubModeMeasOTDR==AVERAGING)&&(GetSetModeLW(0)!=1))
    {
      SetMode(ModeDrawOTDR);
      ModeDevice = MODEREFL;
      SubModeMeasOTDR = NOTMODE;
      ReturnModeViewRefl = SETPARAM;//VIEWMEM -  чтобы вернуться в в установки
      
      //IndexVerSize  = 0;// установка вертикального размера отображения рефлектограммы ( самый крупный)
      //GetSetHorizontScale (5); // установка Вся картинка на экран
      /*  Если надо по рерыванию накоплений можно просмотреть то что успели накопить*/
      TimeSaveOTDR = RTCGetTime(); // сохраняем время сбора
      SetNumAverag (GetCntNumAvrg()); // Запись числа накоплений в память рефлектограмм
      SaveLogNoise(); // сохраняем уровень шумов(перед импульсом в дБ (NF)
      SetHeadFileRaw (GetNumAverag()); // пишем заголовок файла выдачи необработаных данных
      
//123      TraceERASE(0); // стирание трассы (0)
//123      TraceWRITE(0); // запись трассы (0)
      SaveFileSD(0); // сохраняем нулевую трассу

      CmdInitPage(18);// посылка команды переключения окна на просмотра рефлектограммы DrawOTDRView
      
    }
    else
    {
      SetMode(ModeSetupOTDR);  //  переход в режим установок рефлектометра (было)
      ModeDevice = MODESETREFL;
      SubModeMeasOTDR = NOTMODE;
      CmdInitPage(2);// посылка команды переключения окна на OTDR (Установки рефлектометра)
    }
    if (RemoutCtrl) // выдача окончания сбора если запускали дистанционно
    {
      sprintf(Str,"END\r");//c
      RemoutCtrl = 0;
      UARTSendExt ((BYTE*)Str, strlen (Str));
    }
  }
  // вызов нового окна, если необходимо (например при ошибке на входе)
  if(NeedReturn)
  {
    CmdInitPage(NeedReturn);
    NeedReturn = 0;
  }
  
  //  if (g_FirstScr)
  //  {
  //    // здесь заполняем данными поля нового индикатора
  //    // не требущие изменения при первичной инициализации
  //    sprintf(Str, "t0.txt=\"%s\"яяя", MsgMass[1][CurrLang]);
  //    NEX_Transmit((void*)Str);    // 
  //
  //    sprintf(Str, "t1.txt=\"%s\"яяя", MsgMass[1][CurrLang]);
  //    NEX_Transmit((void*)Str);    // 
  //
  //    sprintf(Str, "t2.txt=\"%s\"яяя", MsgMass[32][CurrLang]);
  //    NEX_Transmit((void*)Str);    // входной разъем:
  //
  //    sprintf(Str, "t3.txt=\"%s\"яяя", MsgMass[31][CurrLang]);
  //    NEX_Transmit((void*)Str);    // Range
  //
  //    sprintf(Str, "t4.txt=\"%s\"яяя", MsgMass[29][CurrLang]);
  //    NEX_Transmit((void*)Str);    // Pulse width
  //
  //    sprintf(Str, "t5.txt=\"%s\"яяя", MsgMass[30][CurrLang]);
  //    NEX_Transmit((void*)Str);    // Идет измерение
  //
  //    sprintf(Str, "t6.txt=\" \"яяя");
  //    NEX_Transmit((void*)Str);    // Пустая строка
  //
  //    sprintf(Str, "t7.txt=\" \"яяя");
  //    NEX_Transmit((void*)Str);    // Пустая строка
  //
  //    sprintf(Str, "t8.txt=\" \"яяя");
  //    NEX_Transmit((void*)Str);    // Пустая строка
  //
  //    sprintf(Str, "t9.txt=\" \"яяя");
  //    NEX_Transmit((void*)Str);    // Пустая строка
  //
  //    sprintf(Str, "t10.txt=\" \"яяя");
  //    NEX_Transmit((void*)Str);    // Пустая строка
  //    g_FirstScr = 0;
  //    g_NeedScr = 1;
  //  }
  //  if (g_NeedScr)
  //  {
  //    // здесь заполняем данными поля нового индикатора
  //    // по результатам изменений вызваныйх обработчиком клавиатуры
  //
  //    // раскрашивание поля выбора 
  //    // закрасим бэкграунды  и установим требуемый
  //    sprintf(Str, "t1.bco=WHITEяяя"); // белый
  //    NEX_Transmit((void*)Str);//
  //    sprintf(Str, "t1.bco=WHITEяяя"); // белый
  //    NEX_Transmit((void*)Str);// 
  //    sprintf(Str, "t2.bco=WHITEяяя"); // белый
  //    NEX_Transmit((void*)Str);// 
  //    sprintf(Str, "t3.bco=WHITEяяя"); // белый
  //    NEX_Transmit((void*)Str);// 
  //    sprintf(Str, "t4.bco=WHITEяяя"); // белый
  //    NEX_Transmit((void*)Str);//
  //    sprintf(Str, "t5.bco=WHITEяяя"); // белый
  //    NEX_Transmit((void*)Str);//
  //    sprintf(Str, "t6.bco=WHITEяяя"); // белый
  //    NEX_Transmit((void*)Str);//
  //    sprintf(Str, "t7.bco=WHITEяяя"); // белый
  //    NEX_Transmit((void*)Str);//
  //    sprintf(Str, "t8.bco=WHITEяяя"); // белый
  //    NEX_Transmit((void*)Str);//
  //    sprintf(Str, "t9.bco=WHITEяяя"); // белый
  //    NEX_Transmit((void*)Str);//
  //    sprintf(Str, "t10.bco=WHITEяяя"); // белый
  //    NEX_Transmit((void*)Str);//
  //    sprintf(Str, "t%d.bco=GREENяяя", SubModeMeasOTDR); // зеленый - ПОКА так! надо проверить
  //    NEX_Transmit((void*)Str);// 
  //                                       // код подсветки требуемой строки если есть есть маркер строки
  //    g_NeedScr = 0;
  //  }
  
  //ClrKey (BNS_MASK); // сброс нажатых клавиш
}

void ModeDrawOTDR(void) // режим отображения рефлектограммы
{
  char Str[48];
  char Stra[24];
  unsigned InitScale; // масштаб инициализации в зависимости от индикатора
  //static char ViewMode = VIEWER;//SETCSALE
  //static BYTE TypeCentral = MEDIUM;
  static BYTE NumCursors = 1;
  float fCD; // переменная для оценки дБ
  static float LvlLSA; // уровень погонного затухания
  BYTE NeedCalkLSA = 0;
  unsigned CursorMain = PosCursorMain (0);
  //unsigned CursorMain = (0);
  float TmpACI;

  //float PosLine;// = ((ValueDS*LIGHTSPEED*1e-14)/GetIndexWAV())*CursorMain;
  //BYTE CurrLang;
  //CurrLang=GetLang(CURRENT);
  switch (ViewMode)
  {
  case VIEWER: // обычный просмотр
    if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==UP_SHORT_PRESSED)) // установка снятие курсора замечание 3 (UP_)
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      //      switch (NumCursors) // откат по однократному нажатию
      //      {
      //      case 2: // два курсора .. slave главный
      //        NumCursors = 1;
      //        break;
      //      default :                 
      //        NumCursors = 2;
      //        NeedCalkLSA = 1;
      //        SetPosCursorSlave (CursorMain);
      //        break;
      //      }
      // у нас как бы всегда 2 курсора, но при ОК совмещаем курсоры....
      NumCursors = 2;
      NeedCalkLSA = 1;
      SetPosCursorSlave (CursorMain);//123
      
      //TypeCentral = BYCUR;
      //ClrKey(BTN_OK);
    }
    if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==LONG_PRESSED)) // переход в режим зума
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      ViewMode = SETSCALE;
      //      if (NumCursors != 2)
      //      switch (NumCursors) // откат по однократному нажатию
      //      {
      //      case 2:
      //        NumCursors = 1;
      //        break;
      //      default :
      //        NumCursors = 2;
      //        SetPosCursorSlave (CursorMain);
      //        break;
      //      }
    }
    // выбор горизонтального масштаба при удерживаемой клавише вниз 
    //    if ((PRESS(BTN_DOWN))&&(getStateButtons(BTN_DOWN)==INF_PRESSED))
    //    {
    //
    //      ClrKey (BTN_DOWN);
    //    }
    // перемещение курсора в право
    if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED)) 
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      PosCursorMain (GetSetHorizontScale (0));//123
      NeedCalkLSA = 1;
    }
    // отпустили долго нажатую кнопку
    if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==UP_INF_PRESSED)) 
    {
      g_NeedScr = 1; // Need reDraw Screen
      NeedCalkLSA = 1; // отпустили долго нажатую кнопку, надо пересчитать
    }
    if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==INF_PRESSED))
    {
     PosCursorMain (GetSetHorizontScale(0)*5);//123 
      NeedCalkLSA = 1;
      g_NeedScr = 1; // перерисовки экрана ! Аккуратно! надо проверить потому что нет звука
    } 
    // перемещение курсора влево
    if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      PosCursorMain (-GetSetHorizontScale(0));//123
      NeedCalkLSA = 1;
    }
    // отпустили долго нажатую кнопку
    if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==UP_INF_PRESSED)) 
    {
      g_NeedScr = 1; // Need reDraw Screen
      NeedCalkLSA = 1; // отпустили долго нажатую кнопку, надо пересчитать
    }
    if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==INF_PRESSED))
    {
     PosCursorMain (-GetSetHorizontScale(0)*5);//123 
      NeedCalkLSA = 1;
      g_NeedScr = 1; // перерисовки экрана ! Аккуратно! надо проверить 
    }
    if ((PRESS(BTN_UP))&&(getStateButtons(BTN_UP)==SHORT_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      TypeCentral = UP10PICS;
    }
    if ((PRESS(BTN_UP))&&(getStateButtons(BTN_UP)==INF_PRESSED))
    {
      g_NeedScr = 1; // Need reDraw Screen
      TypeCentral = UP10PICS;
    }
    if ((PRESS(BTN_DOWN))&&(getStateButtons(BTN_DOWN)==SHORT_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      TypeCentral = DOWN10PICS;
    }
    // вызов событий - долго жмем вниз
    if ((PRESS(BTN_DOWN))&&(getStateButtons(BTN_DOWN)==LONG_PRESSED))
    {
      /*  */ 
      if (IndexVerSize==0)// макс. масштаб можно отобразить таблицу событий
      {
        InitEventsTable (); // инициализация структур событий
        SetMode(ModeEventsOTDR);
        // посылка команды переключения окна на прорисовку таблицы событий (вызов)  
        // надо послать команду ниже
        NeedReturn = 12; // необходимо ренуться в окно 12
        // CmdInitPage(12);
        // предрасчет сообытий по необработанным данным
        // CalkPreEvents (RawData, PointsInImpulse(1));
        // ищем события - данные в определении места событий в точках съема
        NumCalkEvents =  (CalkEventsKeys (LogData, PointsInImpulse(0), 1)); 
        IndexEvents = IndexEventsOld = 0;
      }
      else 
      {
        g_NeedScr = 1; // Need reDraw Screen
        TypeCentral = DOWN10PICS;
      }
    }
    break;
  case SETSCALE: // установка масштаба 
    // перенес ниже
    //  sprintf(Str,"<ZOOM>");// признак кправления горизонтальным зумом
    //  putString(82,44,Str,1,0);
    // изменение горизонтального масштаба
    if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))//&&(getStateButtons(BTN_DOWN)==INF_PRESSED)
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      GetSetHorizontScale (-1);//123
      //ClrKey (BTN_RIGHT);
      KeyP &=~BTN_RIGHT;
    }
    if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))//&&(getStateButtons(BTN_DOWN)==INF_PRESSED)
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      GetSetHorizontScale (1);//123
      //ClrKey (BTN_LEFT);
      KeyP &=~BTN_LEFT;
      
    }
    // переключение вертикального масштаба
    if ((PRESS(BTN_UP))&&(getStateButtons(BTN_UP)==SHORT_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      if (IndexVerSize < PNTVERTICALSIZE-1)IndexVerSize++;
      else IndexVerSize = PNTVERTICALSIZE-1;
      TypeCentral = BYCUR;
      //ClrKey (BTN_UP);
    }
    if ((PRESS(BTN_DOWN))&&(getStateButtons(BTN_DOWN)==SHORT_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      if (IndexVerSize > 0)IndexVerSize--;
      else IndexVerSize = 0;
      TypeCentral = BYCUR;
      if (IndexVerSize == 0)TypeCentral = MEDIUM; // при возврате к макс масштабу привязка по среднему
      //ClrKey (BTN_DOWN);
    }
    
    if (((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==LONG_PRESSED))||((PRESS(BTN_DOWN))&&(getStateButtons(BTN_DOWN)==LONG_PRESSED))) // возврат в полную катинку
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      GetSetHorizontScale (5);//123
      IndexVerSize = 0;
      TypeCentral = MEDIUM;
      ViewMode = VIEWER;
      
    }
    if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==UP_SHORT_PRESSED)) // возврат в режим просмотра
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      //TypeCentral = MEDIUM;
      ViewMode = VIEWER;
    }
    break;
    
  } // от переключателя режимов просмотра
  // устнавливаем параметры отображения
  GraphParams params = {GetVerticalSize(IndexVerSize),CursorMain,GetSetHorizontScale(0),NumCursors, TypeCentral};//PosCursorMain (0)//123
  //GraphParams params = {22000,CursorMain,1<<3,NumCursors, TypeCentral};//PosCursorMain (0)
  //UserParams = {GetVerticalSize(IndexVerSize),CursorMain,GetSetHorizontScale(0),NumCursors, TypeCentral};//PosCursorMain (0)
  
  Rect rct={0,0,128,56};
  // режимы отображения
  
  // порисуем график в новый индикатор(данные уже заполнены в МакеГрафик)
  // проверим первый вход  и перерисуем если надо
  // заполняем новый индикатор если в первый раз в основном не изменяемые поля
  if(g_FirstScr)
  {
    
    //sprintf( Str,"xstr 0,208,27,16,3,RED,WHITE,0,1,1,\"A:\"яяя"); // 0
    //NEX_Transmit((void*)Str);// 
    //sprintf( Str,"xstr 0,224,27,16,3,BLUE,WHITE,0,1,1,\"B:\"яяя"); // 0
    //NEX_Transmit((void*)Str);// 
    sprintf(Str,"t4.txt=\"%s\"яяя", MsgMass[118][CurrLang]);// Slope Наклон
    NEX_Transmit((void*)Str);// 
    
    g_NeedScr = 1; // для вызова заполнения значений
    g_FirstScr = 0;
    NeedCalkLSA = 1;// надо пересчитать наклон!
  }
  // здесь будем считать наклон!
    // Если два курсора и признак перемещения курсора есть то считаем LSA
    //if ((NeedCalkLSA)&&(NumCursors == 2))
    // Если признак перемещения курсора есть то считаем LSA
    if (NeedCalkLSA)
    {
      // расчет погонного затухания, двух точечным методом
      NeedCalkLSA = 0;
      unsigned ptA = PosCursorMain (0);//123!!!
      unsigned ptB = PosCursorSlave (0);//123!!!
      //unsigned ptA = 0;
      //unsigned ptB = 128;
      if(ptA!=ptB)
      {
      TmpACI = (GetPosLine(ptA)-GetPosLine(ptB)); // расстояние между точками
      LvlLSA = (LogData[ptA]-LogData[ptB])/(1000.0*TmpACI);
      }
      else
        LvlLSA  = 0.0;
      //LvlLSA = Calc_LSA(PosCursorMain (0),PosCursorSlave (0));
    }
  
  // надо что то изменить в полях установок 
  if(g_NeedScr)
  {
    
    // устнавливаем параметры отображения
    CursorMain = PosCursorMain (0); //123!!!// переустановим курсор после изменений
    //CursorMain =  (0); // переустановим курсор после изменений
    // расчет параметров графика, и курсоры, надо ставить выше расчетов значений!?
    GraphParams paramsN = {GetVerticalSize(IndexVerSize),CursorMain,GetSetHorizontScale(0),NumCursors, TypeCentral};//123!!!//PosCursorMain (0)
    //GraphParams paramsN = {22000,CursorMain,1<<3,NumCursors, TypeCentral};//PosCursorMain (0)
    // прорисовка(заполнение) данных для нового индикатора после обработки кнопок
    // здесь надо переопределить установки для разных типоразмеров индикаторов 400*240(3.2") или 480*320(3.5")
    if(TypeLCD)
    {
      rct.right=480;
      rct.bottom=255;// здесь надо сделать 255, а индикаторе масштаб 270/255 1.058
      InitScale = 8; // 4608/InitScale>480
      // курсорный масштаб
      CursorScale = 1.06;
    }
    else
    {
      rct.right=400;
      rct.bottom=210;
      InitScale = 10; // 4608/InitScale>480
      CursorScale = 1.0;

    }
    
    rct.left=0;
    rct.top=0;
    paramsN.scale=(paramsN.scale>=16)?(InitScale):(paramsN.scale);
    unsigned CurrSM = MakeGraphNext( &rct, LogData, POINTSIZE, &paramsN );// тут пересчитываем данные для графика (его вывод) в зависимости от параметров
    // теперь нарисуем курсор в рамках Y=0..210
    CurrRed = (CurrSM & 0xffff);
    CurrBlue = (CurrSM>>16);
    //CreatDelay (20000);
      HAL_Delay(2);

    SendDrawNex(NexData,2, rct.right);
    
    //CreatDelay (30000); // на большом индикаторе с 30000 "глючило"
      HAL_Delay(3);
// Main Cursor
    if (GetIndexLN()) // не 2 км ( значит в км)
    {
      if (GetIndexLN()==6) // 128 km? 2 sign
        sprintf(Str,"t0.txt=\"A:%6.2f%s\"яяя", GetPosLine(CursorMain) ,MsgMass[20][CurrLang]);//км
      else
        sprintf(Str,"t0.txt=\"A:%6.3f%s\"яяя", GetPosLine(CursorMain) ,MsgMass[20][CurrLang]);//км
    }
    else
      sprintf(Str,"t0.txt=\"A:%.1f%s\"яяя", GetPosLine(CursorMain)*1000.0 ,MsgMass[78][CurrLang]);//м - метры
    NEX_Transmit((void*)Str);// 
    // Slave Cursor  
    if (GetIndexLN()) // не 2 км ( значит в км)
    {
      if (GetIndexLN()==6) // 128 km? 2 sign
        sprintf(Str,"t1.txt=\"B:%6.2f%s\"яяя", GetPosLine(PosCursorSlave (0)) ,MsgMass[20][CurrLang]);//км
      else
        sprintf(Str,"t1.txt=\"B:%6.3f%s\"яяя", GetPosLine(PosCursorSlave (0)) ,MsgMass[20][CurrLang]);//км
    }
    else
      sprintf(Str,"t1.txt=\"B:%.1f%s\"яяя", GetPosLine(PosCursorSlave (0))*1000.0 ,MsgMass[78][CurrLang]);//м - метры
    NEX_Transmit((void*)Str);// 
    
    if (GetIndexLN()) // не 2 км ( значит в км)
    {
      if (GetIndexLN()==6) // 128 km? 2 sign
        sprintf(Str,"t2.txt=\"A-B:%.2f%s\"яяя",fabs(GetPosLine(CursorMain)-GetPosLine(PosCursorSlave (0))) ,MsgMass[20][CurrLang]);//км
      else
        sprintf(Str,"t2.txt=\"A-B:%.3f%s\"яяя",fabs(GetPosLine(CursorMain)-GetPosLine(PosCursorSlave (0))) ,MsgMass[20][CurrLang]);//км
    }
    else
      sprintf(Str,"t2.txt=\"A-B:%.1f%s\"яяя",fabs((GetPosLine(CursorMain)-GetPosLine(PosCursorSlave (0)))*1000.0) ,MsgMass[78][CurrLang]);//м - метры
    NEX_Transmit((void*)Str);// 
    // разница дБ между курсорами
    if ((int)(PosCursorSlave(0) - PosCursorMain(0))>0)
    {
      fCD=((unsigned short)(GetLevelCurs(1)>>16)-(unsigned short)(GetLevelCurs(1)&0xFFFF))/1000.0;
      if((fCD>-10.0)&&(fCD<10.0))
        sprintf(Str,"t3.txt=\"L:%.3f%s\"яяя",fCD,MsgMass[47][CurrLang]);//дБ
      else
        sprintf(Str,"t3.txt=\"L:%.2f%s\"яяя",fCD,MsgMass[47][CurrLang]);//дБ
      
    }
    else
    {
      fCD= ((unsigned short)(GetLevelCurs(1)&0xFFFF)-(unsigned short)(GetLevelCurs(1)>>16))/1000.0;
      if((fCD>-10.0)&&(fCD<10.0))
        sprintf(Str,"t3.txt=\"L:%.3f%s\"яяя",fCD,MsgMass[47][CurrLang]);//дБ
      else
        sprintf(Str,"t3.txt=\"L:%.2f%s\"яяя",fCD,MsgMass[47][CurrLang]);//дБ
    }
    NEX_Transmit((void*)Str);// 
    // расчет и индикация погонного затухания
//    if(LvlLSA<0.0)
//      sprintf(Str,"t5.txt=\"<0 %s/%s\"яяя",MsgMass[47][CurrLang],MsgMass[20][CurrLang]);//дБ/км
//    else if(LvlLSA<10.0)
    if((LvlLSA<10.0)&&(LvlLSA>-10.0)) // три знака
      sprintf(Str,"t5.txt=\"%.3f%s/%s\"яяя",LvlLSA,MsgMass[47][CurrLang],MsgMass[20][CurrLang]);//дБ/км
    else if((LvlLSA<=30.0)&&(LvlLSA>-30.0))
      sprintf(Str,"t5.txt=\"%.2f%s/%s\"яяя",LvlLSA,MsgMass[47][CurrLang],MsgMass[20][CurrLang]);//дБ/км
    else 
      sprintf(Str,"t5.txt=\">< %s/%s\"яяя",MsgMass[47][CurrLang],MsgMass[20][CurrLang]);//дБ/км
    
    NEX_Transmit((void*)Str);// 
    
    // курсоры, попробуем прорисовть бирюзовым если они вместе
    
    if(CurrBlue==CurrRed)
    {
      sprintf( Str,"line %d,0,%d,%d,63519яяя",CurrRed,CurrRed,(int)(rct.bottom*CursorScale)-1); // 0
      NEX_Transmit((void*)Str);// 
    }
    else
    {
      sprintf( Str,"line %d,0,%d,%d,REDяяя",CurrRed,CurrRed,(int)(rct.bottom*CursorScale)-1); // 0
      NEX_Transmit((void*)Str);// 
      //CreatDelay (20000);
      HAL_Delay(2);
      if(CurrBlue<rct.right)
      {
        sprintf( Str,"line %d,0,%d,%d,BLUEяяя",CurrBlue,CurrBlue,(int)(rct.bottom*CursorScale)-1); // 0
      }
      else
      {
        if(CurrBlue==rct.right) // справа
        {
          if(TypeLCD)
            sprintf( Str,"xstr 440,192,26,40,2,BLUE,BLACK,0,1,3,\">\"яяя"); // 03_5(2-40) (№ шрифта-Размер) 0
          else
            sprintf( Str,"xstr 387,192,16,24,2,BLUE,BLACK,0,1,3,\">\"яяя"); // 03_2(2-24) (№ шрифта-Размер) 0
        }
        else
        {
          if(TypeLCD)
            sprintf( Str,"xstr 1,192,26,40,2,BLUE,BLACK,0,1,3,\"<\"яяя"); // 03_5(2-40) (№ шрифта-Размер) 0
          else
            sprintf( Str,"xstr 1,192,16,24,2,BLUE,BLACK,0,1,3,\"<\"яяя"); // 03_2(2-24) (№ шрифта-Размер) 0
        }
      }
      NEX_Transmit((void*)Str);// 
      
    }
    // рисуем признак активного курсора (пока это 1 или 2)
    //   if(NumCursors==1)
    //        sprintf( Str,"xstr 387,0,12,16,3,RED,BLACK,0,1,1,\"A\"яяя"); // 0
    //    else 
    //        sprintf( Str,"xstr 387,0,12,16,3,BLUE,BLACK,0,1,1,\"B\"яяя"); // 0
    //    NEX_Transmit((void*)Str);
    // индикация масштабов
    sprintf(Stra,"Y-%.1f%s/div",(float)GetVerticalSize(IndexVerSize)/7000.0,MsgMass[47][1] );//дБ
    
    if(TypeLCD)
      sprintf( Str,"xstr 360,0,120,24,3,GREEN,BLACK,0,1,3,\"%s\"яяя",Stra); // 3_5(3-24) (№ шрифта-Размер) 0
    else
      sprintf( Str,"xstr 260,0,140,24,2,GREEN,BLACK,0,1,3,\"%s\"яяя",Stra); // 3_2(2-24) (№ шрифта-Размер) 0
    NEX_Transmit((void*)Str);// 
    //CreatDelay (20000);
      HAL_Delay(2);

    sprintf(Stra,"X-1:%d",GetSetHorizontScale(0));// признак управления горизонтальным зумом
    if(TypeLCD)
      sprintf( Str,"xstr 360,24,120,24,3,GREEN,BLACK,0,1,3,\"%s\"яяя",Stra); // 3_5(24)  0
    else
      sprintf( Str,"xstr 260,16,140,24,2,GREEN,BLACK,0,1,3,\"%s\"яяя",Stra); // 3_2(24)  0
    NEX_Transmit((void*)Str);// 
    
    // если ЗУМ, перенесем сюда из кнопок
    if(ViewMode == SETSCALE)
    {
      sprintf(Stra,"<ZOOM>");// признак кправления горизонтальным зумом
      //sprintf(Stra,"<Z1:%d>",GetSetHorizontScale(0));// признак управления горизонтальным зумом
      if(TypeLCD)
        sprintf( Str,"xstr 240,0,100,40,3,2047,BLACK,0,1,3,\"%s\"яяя",Stra); //3_5(40) 0 последнее поле вид Бэкграунда, 0- обрезка, 1- цельный , 2 -картинка, 3 -без Бэкграунда
      else
        sprintf( Str,"xstr 160,0,100,24,2,2047,BLACK,0,1,3,\"%s\"яяя",Stra); //3_2(24) 0 последнее поле вид Бэкграунда, 0- обрезка, 1- цельный , 2 -картинка, 3 -без Бэкграунда
      NEX_Transmit((void*)Str);// 
    }
    
    g_NeedScr = 0; 
  }
  
  if ((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))
  {
    myBeep(10);
    switch (ReturnModeViewRefl)
    {
    case VIEWMEM:
      SetMode(ModeMemoryOTDR);
      ModeDevice = MODEMEMR;
      ModeMemDraw = VIEWNEXT;
      // посылка команды переключения окна на Mem_OTDR_graph (возврат)  
      CmdInitPage(13);
      //надо время что бы переключится
      //CreatDelay(1000000);
        HAL_Delay(100);

      break;
    case VIEWEVNT:
      SetMode(ModeEventsOTDR);
      // посылка команды переключения окна на Event_window (возврат)  
      CmdInitPage(12);
      break;
    default:
      SetMode(ModeSetupOTDR);
      GetSetModeLW(-1); // сброс счетчика, так как из просмотра 
      ClrKey (BTN_MENU);
      ModeDevice = MODESETREFL;
      ViewMode = VIEWER; // если есть зумм то выключаем его
      // посылка команды переключения окна на OTDR (возврат)  
      CmdInitPage(2);
      break;
    }
  }
  //ClrKey (BNS_MASK);
  if (rawPressKeyS)// переход в редактор комментариев сохранения
  { 
    myBeep(10);
    SetMode(ModeKeyBoardOTDR);
    ModeDevice = MODEOTHER;
    myBeep(10);
    CheckCommOTDR ();// проверка и корректировка строки комментариев OTDR

    // подготовка строки комментария для редакции
    IndexCommOTDR = 0;
    CommentsOTDR[ARRAY_SIZE(CommentsOTDR)-1]=0; // последний элемент в массиве равен 0
    for (int Ind =ARRAY_SIZE(CommentsOTDR)-2; Ind>=0; Ind--) // идем с последнего
    {
      if (CommentsOTDR[Ind]<0x20) CommentsOTDR[Ind]=' '; //если управляющие, то делаем их "пробелом"
      else if (CommentsOTDR[Ind]!=' ' && IndexCommOTDR == 0)IndexCommOTDR = Ind; // фиксируем длину строки до первого НЕ"прбела"
      //Index_Comm --;
    }
    if ((CommentsOTDR[IndexCommOTDR]!=' ')&&(IndexCommOTDR!=18))IndexCommOTDR ++;// если указатель на не пробел и это не последний, то увеличиваем указатель
    KbPosX = 11;
    KbPosY = 2;
    // было старт терперь снова редактор сохранения
    //        POWALT(ON);
    //        POWREF (ON);
    //        POWDET(ON);
    //        Light(0); // Выключаем подсветку
    //        SetMode(ModeStartOTDR);
    //        ModeDevice = MODEMEASURE;
    //        //IndexVerSize  = 0;// установка вертикального размера отображения рефлектограммы ( самый крупный)
    //        SubModeMeasOTDR = SETPOWER;
    //        SSPInit_Any(SPI_ALT); // Инициализация SSP для управления ALTERA (порт 1 та что на плате отладочной)
    //        rawPressKeyS=0;
    // посылка команды переключения окна на Клавиатуру редактирования Keyboard (вызов)  OTDR
    if(CurrLang)// не русский
    {
      CmdInitPage(22);
    }
    else
    {
      CmdInitPage(23);
    }
    
    
  }
  // вызов нового окна, если необходимо
  if(NeedReturn)
  {
    CmdInitPage(NeedReturn);
    NeedReturn = 0;
  }
  
  //SetVerticalSize(VerticalSize[IndexVerSize]);
}


void ModeEventsOTDR(void) // режим отображения событий рефлектограммы CHECK_OFF
{
  char Str[32];
  char s_orl[16];
  //Rect rct_pic={77,17,109,27};
  int type_pic=0;
  //static unsigned short IndexEventsOld = 0; // указатель на событие Для однократного перерисовывания индикатора

  if (IndexEvents == 0) // начало просмотра рефлектограмм
  {
  }
  else
  {
    if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==UP_SHORT_PRESSED)) // переход в режим просмотра с переключением зума
    {
      myBeep(10);
      GetSetHorizontScale (-6); // обнуляем масштаб
      GetSetHorizontScale (0); // устанавливаем масштаб = 4
      //ClrKey (BTN_LEFT);
      // переключение вертикального масштаба
      // if (IndexVerSize < PNTVERTICALSIZE-1)IndexVerSize++;
      // else IndexVerSize = PNTVERTICALSIZE-1;
      IndexVerSize=1;
      SetPosCursorMain (EvenTrace[IndexEvents-1].EPT);     
      TypeCentral = MEDIUM;
      ViewMode = VIEWER;
      SetMode(ModeDrawOTDR);
      // с 22.11.2022 надо пересмотреть вариант возврата
      ReturnModeViewRefl = VIEWEVNT;//VIEWMEM -  чтобы вернуться в в установки
      KeyP &=~BTN_OK;
    // посылка команды переключения окна на DrawOTDRview (возврат)  
      // через признак 
      // надо послать команду ниже
      NeedReturn = 18; // необходимо ренуться в окно 18
      //CmdInitPage(18);
      
    }
    
    
  }
  // перемещение курсора в право
  if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED)) 
  {
    myBeep(10);
    g_NeedScr = 1; // Need reDraw Screen
    if (IndexEvents<NumCalkEvents)IndexEvents++;
    else 
      IndexEvents=0; 
    if((IndexEvents!=0) && (IndexEventsOld==0))g_FirstScr=1;
    if((IndexEvents==0) && (IndexEventsOld!=0))g_FirstScr=1;
    IndexEventsOld = IndexEvents;
  }
  // перемещение курсора влево
  if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1; // Need reDraw Screen
    if (IndexEvents>0)IndexEvents--;
    else IndexEvents=NumCalkEvents; 
    if((IndexEvents!=0) && (IndexEventsOld==0))g_FirstScr=1;
    if((IndexEvents==0) && (IndexEventsOld!=0))g_FirstScr=1;
    IndexEventsOld = IndexEvents;
  }
  // проверим изменение IndexEvents
  
  if (g_FirstScr)
  {
    // здесь заполняем данными поля нового индикатора
    // не требущие изменения при первичной инициализации
    sprintf(Str, "t0.txt=\"< %s >\"яяя", MsgMass[83][CurrLang]); // < событиe >
    NEX_Transmit((void*)Str);    //
    // далее в зависимости от указателя
    if(IndexEvents) // отображаем событие
    {
    sprintf(Str, "t1.txt=\"%s\"яяя", MsgMass[84][CurrLang]); //Type Event
    NEX_Transmit((void*)Str);    // 
    sprintf(Str, "t2.txt=\"%s\"яяя", MsgMass[85][CurrLang]); // положение
    NEX_Transmit((void*)Str);    //
    sprintf(Str, "t3.txt=\"%s\"яяя", MsgMass[86][CurrLang]); //Затухание
    NEX_Transmit((void*)Str);    //
    sprintf(Str, "t8.txt=\"%s\"яяя", MsgMass[87][CurrLang]); // Отражение
    NEX_Transmit((void*)Str);    //
    }
    else // заголовок таблицы событий (результат линии)
    {
    sprintf(Str, "t1.txt=\"%s\"яяя", MsgMass[31][CurrLang]); //длинна линии
    NEX_Transmit((void*)Str);    // 
    sprintf(Str, "t2.txt=\"%s\"яяя", MsgMass[93][CurrLang]); // полные потери
    NEX_Transmit((void*)Str);    //
    sprintf(Str, "t3.txt=\"ORL\"яяя" ); //ORL, если измерялось! 
    NEX_Transmit((void*)Str);    //
    sprintf(Str, "t8.txt=\" \"яяя" ); //Empty str
    NEX_Transmit((void*)Str);    //
      
    }
       



    g_FirstScr = 0;
    g_NeedScr = 1;
  }
  if (g_NeedScr)
  {
    // здесь заполняем данными поля нового индикатора
    // по результатам изменений вызваныйх обработчиком клавиатуры
    // счетчик указатель событий
    sprintf(Str, "t7.txt=\"%d/%d \"яяя", IndexEvents,NumCalkEvents); //Таблица событий
    NEX_Transmit((void*)Str);    //
    // далее в зависимости от указателя
    type_pic = 0;
          sprintf(Str,"p0.pic=%1dяяя",type_pic);//затираем картинку
      //sprintf(Str,"pic 330,70,%1dяяя",type_pic);//тип картинки
      NEX_Transmit((void*)Str);    // 
      //CreatDelay(5000);
  HAL_Delay(1);

    if(IndexEvents) // отображаем событие
    {
      // строчка прорисовки типа событий пока просто номер!
      //sprintf(Str,"t4.txt=\"%c%c\"яяя",EvenTrace[IndexEvents-1].EC[1],EvenTrace[IndexEvents-1].EC[0]);// event Code
      // обработка признаков событий для вывода в поле Т4 названий или картинок?
          switch (EvenTrace[IndexEvents-1].EC[1])
    {
    case 'F': // 
      switch (EvenTrace[IndexEvents-1].EC[0])
      {
      case '0':
      sprintf(s_orl,"nRef");//N - неотражающее событие
      type_pic = 1;
        break;
      case '1':
      sprintf(s_orl,"Ref");//R - отражающее событие
      type_pic = 2;
        break;
      case '2':
      sprintf(s_orl,"Ref");//S - начало линии
        break;
    default:
    sprintf(s_orl,"%s ",EvenTrace[IndexEvents-1].COMM_EVN);//комментарий
      break;
        
      }
      break;
    case 'E':
      switch (EvenTrace[IndexEvents-1].EC[0])
      {
      case '0':
      sprintf(s_orl,"EOFn");//E - конец линии без отражения
      type_pic = 4;
        break;
      case '2':
      sprintf(s_orl,"EOFr");//E - конец линии как перегрузка по отражающему событию
      type_pic = 5;
        break;
    default:
    sprintf(s_orl,"%s ",EvenTrace[IndexEvents-1].COMM_EVN);//комментарий
      break;
      }
      break;
    case 'O':
      switch (EvenTrace[IndexEvents-1].EC[0])
      {
      case '0':
      sprintf(s_orl,"ENF");//E - конец линии без отражения
      type_pic = 6;
        break;
//      case '2':
//      DrawPicture (&rct_pic, BigEnd);
//      sprintf(Str,"E");//E - конец линии как перегрузка по отражающему событию
//      putString(120,18,Str,1,0);
//        break;
    default:
    sprintf(s_orl,"%s ",EvenTrace[IndexEvents-1].COMM_EVN);//комментарий
      break;
      }
      break;
    default:
    sprintf(s_orl,"%s ",EvenTrace[IndexEvents-1].COMM_EVN);//комментарий
      break;
    }
    // сначала текст потом картинку
      sprintf(Str,"t4.txt=\"%s\"яяя",s_orl);//км
      NEX_Transmit((void*)Str);    // 
      if(type_pic)
      {
      //CreatDelay(50000);
  HAL_Delay(5);
      // в Модуль картинки
      sprintf(Str,"p0.pic=%1dяяя",type_pic);//тип картинки
      //sprintf(Str,"pic 330,70,%1dяяя",type_pic);//тип картинки
      NEX_Transmit((void*)Str);    // 
      //CreatDelay(50000);
  HAL_Delay(5);
      // чуток надо потупить!
      }
      // вывод других полей
      if (GetIndexLN()) // не 2 км ( значит в км)
        sprintf(Str,"t5.txt=\"%.3f%s\"яяя",fabs(GetPosLine(EvenTrace[IndexEvents-1].EPT)) ,MsgMass[20][CurrLang]);//км
      else
        sprintf(Str,"t5.txt=\"%.1f%s\"яяя",fabs((GetPosLine(EvenTrace[IndexEvents-1].EPT))*1000.0) ,MsgMass[78][CurrLang]);//м - метры
      NEX_Transmit((void*)Str);    // 
      // затухание
      sprintf(Str, "t6.txt=\"%.2f%s\"яяя",(EvenTrace[IndexEvents-1].EL)/1000.0,MsgMass[47][CurrLang]); // дб
      NEX_Transmit((void*)Str);    //
      // отражение
      // 16/04/2015 прочерки при неотражающем событии
      if (EvenTrace[IndexEvents-1].ER !=0)
      {
        sprintf(Str, "t9.txt=\"%.2f%s\"яяя",EvenTrace[IndexEvents-1].ER/1000.0,MsgMass[47][CurrLang]); // дб
      }
      else
      {
        sprintf(Str, "t9.txt=\"------\"яяя"); // дб
      }
      NEX_Transmit((void*)Str);    //
      
    }
    else // заголовок таблицы событий (результат линии)
    {
      // зачистим область вывода картинки
     // sprintf(Str,"pic 330,70,0яяя");//пустая картинка
     //NEX_Transmit((void*)Str);    // 
      // чуток надо потупить!
     // CreatDelay(1000);
    if (GetIndexLN()) // не 2 км ( значит в км)
      sprintf(Str,"t4.txt=\"%.3f%s\"яяя",fabs(GetPosLine(EndEvenBlk.ELMP[1])) ,MsgMass[20][CurrLang]);//км
    else
      sprintf(Str,"t4.txt=\"%.1f%s\"яяя",fabs((GetPosLine(EndEvenBlk.ELMP[1]))*1000.0) ,MsgMass[78][CurrLang]);//м - метры
    NEX_Transmit((void*)Str);    // 
    sprintf(Str, "t5.txt=\"%.2f%s\"яяя", (LogData[EndEvenBlk.ELMP[1]]-LogData[EndEvenBlk.ELMP[0]])/1000.0,MsgMass[47][CurrLang]); // дб
    NEX_Transmit((void*)Str);    //
    if(g_VolORL!=0)sprintf(s_orl,"%.1f",g_VolORL);
    else sprintf(s_orl,"???");
      sprintf(Str,"t6.txt=\"%s%s\"яяя",s_orl,MsgMass[47][CurrLang]);////ORL, если измерялось!
    NEX_Transmit((void*)Str);    // 
    sprintf(Str, "t9.txt=\" \"яяя" ); //Empty str
    NEX_Transmit((void*)Str);    //
    }
    
                                           // код подсветки требуемой строки если есть есть маркер строки
    g_NeedScr = 0;
  }

  if ((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))
  {
    SetMode(ModeDrawOTDR);
// 22.11.2022 надо вернутся туда откуда пришли по новой переменной  ReturnMemView
    if(ReturnMemView)
      ReturnModeViewRefl = VIEWMEM;//SETPARAM -  чтобы вернуться в предпросмотр
      else
      ReturnModeViewRefl = SETPARAM;//-  чтобы вернуться в предпросмотр
    myBeep(10);
    // посылка команды переключения окна на DrawOTDRview (возврат)  
      CmdInitPage(18);
  }
  // вызов нового окна, если необходимо
  if(NeedReturn)
  {
   CmdInitPage(NeedReturn);
   NeedReturn = 0;
  }
  //ClrKey (BNS_MASK); // сброс нажатых клавиш
  
}

void ModeKeyBoardOTDR(void) // режим отображения клавиатуры редактора комментариев рефлектограммы
{
  char Str[32];
  char StrI[32];
  //BYTE CurrLang=GetLang(CURRENT);
  //static BYTE Shift = 0; // регистр 
  BYTE NeedSaveTr = 0;
  if (g_FirstScr)
  {
    // здесь заполняем данными поля нового индикатора
    // не требущие изменения при первичной инициализации
  SetIndexCommOTDR (IndexCommOTDR);
    
    // надо обрезать до последнего пробела....
    memcpy(StrI,CommentsOTDR,IndexCommOTDR+1); 
    StrI[IndexCommOTDR+1]=0;// No more 20 byte size
    sprintf(Str, "t1.txt=\"%s\"яяя",StrI); 
    NEX_Transmit((void*)Str);    // 1 строка комментарии
    g_GetStr=1; // взведем признак  необходимости прочитать строчку! из индикатора.
    g_FirstScr = 0;
    g_NeedScr = 1;
  }
  if (g_NeedScr)
  {
  sprintf(Str,"%02d%02d%02d_%02d%02d%01d.sor",TimeSaveOTDR.RTC_Year%100,
          TimeSaveOTDR.RTC_Mon,
          TimeSaveOTDR.RTC_Mday,
          TimeSaveOTDR.RTC_Hour,
          TimeSaveOTDR.RTC_Min,
          TimeSaveOTDR.RTC_Sec/10 );
    sprintf(StrI, "t0.txt=\"%s\"яяя", Str);
    NEX_Transmit((void*)StrI);    // Date/Time записи
    g_NeedScr=0;
  }
    if(g_GetStr==2) // УРА! что то приняли назад, можно переписать и сбросить признак
  {
    // здесь обработаем строку на приеме
    IndexCommOTDR = GetStringNEX(CommentsOTDR, ARRAY_SIZE(CommentsOTDR));
    //memcpy(CommentsOLT,&RX_BufNEX[1],(CntRXNEX<18)?(CntRXNEX-2):(15)); 
    //CommentsOLT[15]=0;
    for(int i=IndexCommOTDR; i<ARRAY_SIZE(CommentsOTDR); i++) CommentsOTDR[i]=' ';
    CommentsOTDR[ARRAY_SIZE(CommentsOTDR)-1]=0;
    // сохраним комментарий
   memcpy(NameDB.UserComm,CommentsOTDR,20); 
    //for(int i=0; i<20; i++) 
    //NameDB.UserComm[i]=CommentsOTDR[i];

    WriteNeedStruct(0x10);
  
    //CommentsOTDR[ARRAY_SIZE(CommentsOTDR)-1]=0xd;
    //    memcpy (PONI.CommUserPM, CommentsOLT,16);
    //    RX_BufNEX[31]=0xd;
    //    UARTSend0 ((BYTE*)RX_BufNEX, 32);
    //    UARTSend0 ((BYTE*)CommentsOTDR, strlen (CommentsOTDR));

    NeedSaveTr=1; // надо сохранится
  }
  // сохранение по кнопке OK на клавиатуре прибора (точка)
  if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
    // обработка кнопки Ок
  {
    myBeep(7);
    //  sprintf(Str, "click brok,1яяя"); // тест кнопка ок на клавиатуре
    //NEX_Transmit((void*)Str);    //
      sprintf(Str, "click bok,1яяя"); // тест кнопка ок на клавиатуре ENGLISH
    NEX_Transmit((void*)Str);    //
    // здесь реально отвечает через не более 2 мС
    // StartRecievNEX (10);// время ожидания начала ответов от индикатора

  }
  // выход без сохранения
  if (((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))||(NeedReturn))
  {
    SetMode(ModeDrawOTDR);
    ReturnModeViewRefl = SETPARAM;//VIEWMEM -  чтобы вернуться в в установки
    ModeDevice = MODEREFL;
    myBeep(10);
    // посылка команды переключения окна на DrawOTDRview (возврат)  
      CmdInitPage(18);
      NeedReturn = 0;
  }
  if ((rawPressKeyS)||(NeedSaveTr))// сохранение если в редакторе по кнопке S  кроме кнопки отмена
  {
    if ((!((KbPosY == 2)&&(KbPosX == 11)))||(NeedSaveTr))  SaveNewOTDRTrace (0);
    rawPressKeyS = 0;
    NeedSaveTr = 0;
  }
}

// вызываем чтение SD Card для поиска директорий, составляем список
// "правильных" директорий, устанавливаем курсоры если они не изменились,
// если не совпадают с полученными размерами сбрасываем в начало
// вызываем ОКНО 33
void ModeFileMngDir(void) // режим файл менеджера директорий
{
  char Str[32];
  
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
    sprintf(Str, "t0.txt=\"0:/_OTDR\"яяя"); // < событиe >
    NEX_Transmit((void*)Str);    //
    sprintf(Str, "t14.txt=\"%d\"яяя", NumNameDir); // < сколько папок нашли >
    NEX_Transmit((void*)Str);    //
    
    
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
      
      sprintf(Str, "t%d.txt=\"%s\"яяя",i+1 ,NameDir[PageDir*12+i]); // < событиe >
      NEX_Transmit((void*)Str);    //
      
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
    myBeep(10);
      SetMode(ModeFileMngFiles);
      ModeDevice = MODEMEMR;
      ModeMemDraw = VIEWNEXT;
      ReturnMemView = 1; // надо вернуться сюда же по ESC
         // посылка команды переключения окна на Mem_OTDR_garaph (вызов)  
      //KeyP = 0;
      ClrKey(BTN_OK);
      CmdInitPage(34); // новое окно лист бокс перечня директорий
       //CreatDelay(1000000);
      HAL_Delay(100);
  }

  if ((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))
  {
    // здесь над витвится в зависимости от признака откуда пришли
    if(ReturnMemView)
    {
      //DeleteTrace = 0;
      SetMode(ModeSelectMEM);
      ModeDevice = MODEOTHER;
      myBeep(10);
      // посылка команды переключения окна на Memory (возврат)  
      CmdInitPage(4);
    }
  }
}

// вызываем чтение SD Card для поиска файлов , составляем список
// "правильных" файлов, устанавливаем курсоры если они не изменились,
// если не совпадают с полученными размерами сбрасываем в начало
// вызываем ОКНО 34
void ModeFileMngFiles(void) // режим файл менеджера файлов
{
  char Str[32];
  char FilPath[64];
  uint32_t BlkSz; // размер блока заголовка
  UINT RWC;
  FATFS FatFs;

  if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==UP_SHORT_PRESSED)) // переход в режим просмотра с переключением зума
  {
    myBeep(10);
  }
  if ((PRESS(BTN_UP))&&(getStateButtons(BTN_UP)==SHORT_PRESSED)) 
  {
    myBeep(10);
    if(IndexNameFiles>0)IndexNameFiles--;
    g_NeedScr=1;
  }
  if ((PRESS(BTN_DOWN))&&(getStateButtons(BTN_DOWN)==SHORT_PRESSED))
  {
    myBeep(10);
    if((IndexNameFiles+1)<NumNameFiles)IndexNameFiles++;
    g_NeedScr=1;
  }
  if (g_FirstScr)
  {
    SDMMC_SDCard_FILES(); // прочитаем файлы, 
    // здесь заполняем данными поля нового индикатора
    // не требущие изменения при первичной инициализации
    sprintf(Str, "t0.txt=\"0:/_OTDR/%s\"яяя",NameDir[IndexNameDir]); // < событиe >
    NEX_Transmit((void*)Str);    //
    sprintf(Str, "t14.txt=\"%d\"яяя", NumNameFiles); // < сколько файлов нашли >
    NEX_Transmit((void*)Str);    //
    
    
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
    PageFiles = IndexNameFiles/12; // получим страницу перечня директорий котрую нужно отображать
    // заполним поля индикатора именами директорий
    for (int i=0; i<12; i++)
    {
      
      sprintf(Str, "t%d.txt=\"%s\"яяя",i+1 ,NameFiles[PageFiles*12+i]); // < событиe >
      NEX_Transmit((void*)Str);    //
      
    }
        FR_Status = f_mount(&FatFs, SDPath, 1);

    // здесь можно прочитать файл на котрый указываем и разобрать его
    sprintf(FilPath, "0:/_OTDR/%s/%s",NameDir[IndexNameDir],NameFiles[IndexNameFiles]); // путь к файлу
    // откроем файл и прочитаем размер блока
        FR_Status = f_open(&Fil, FilPath, FA_READ);
    if(FR_Status == FR_OK)
    {
     f_lseek (&Fil, 2); // переместимся на 2 байта
     f_read (&Fil, (void*)&BlkSz, 4, &RWC);
     f_lseek (&Fil, BlkSz); // переместимся на  байта
     f_read (&Fil, (void*)&F_SOR, 142, &RWC);

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
        sprintf(Str, "t15.txt=\"%d\"яяя", BlkSz); // < какой файл выбран >
    NEX_Transmit((void*)Str);    //
        sprintf(Str, "t16.txt=\"%dnm\"яяя", F_SOR.AW/10); // < какой файл выбран >
    NEX_Transmit((void*)Str);    //
        sprintf(Str, "t17.txt=\"%d\"яяя", F_SOR.NPPW); // < какой файл выбран >
    NEX_Transmit((void*)Str);    //
        sprintf(Str, "t18.txt=\"%d\"яяя", F_SOR.AR); // < какой файл выбран >
    NEX_Transmit((void*)Str);    //
        sprintf(Str, "t19.txt=\"%d\"яяя", F_SOR.NAV); // < какой файл выбран >
    NEX_Transmit((void*)Str);    //

    g_NeedScr = 0;
  }
  if ((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))
  {
    // здесь над витвится в зависимости от признака откуда пришли
    if(ReturnMemView)
    {
      SetMode(ModeFileMngDir);
      ModeDevice = MODEMEMR;
      ModeMemDraw = VIEWNEXT;
      ReturnMemView = 1; // надо вернуться сюда же по ESC
         // посылка команды переключения окна на Mem_OTDR_garaph (вызов)  
      //KeyP = 0;
      ClrKey(BTN_MENU);
      CmdInitPage(34); // новое окно лист бокс перечня директорий
       //CreatDelay(1000000);
      HAL_Delay(100);
    }
  }
}

void ModeMemoryOTDR(void) // режим отображения сохраненных рефлектограмм и работа с ними
{
  
  char Str[64];
  char Stra[32];
  //BYTE CurrLang=GetLang(CURRENT);
  unsigned Trace = GetNumTrace();
  static BYTE DeleteTrace = 0; //признак удаления выбранной трассы
  static BYTE MsgDel = 0; //сообщения при удалении выбранной трассы
  BYTE NeedCallView = 0; //признак необходимости открыть окно просмотра рефлектограммы
  //BYTE NeedReStartMeasure = 0; //признак необходимости снова запустить измерение из окна предросмотра рефлектограмм
  // обработка клавиатуры  с изменениями
    if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED)&&!DeleteTrace)
    // обработка кнопки Ок (подтверждение перехода в окно просмотра рефлекторгаммы
  {
      TraceERASE(0);
      TraceWRITE(0); // записываем рефлектограмму с параметрами сьема  в 0 ячейку
      DeleteTrace = 0;
      GetSetHorizontScale (5);// вся картинка на экран
      SetMode(ModeDrawOTDR);
      if(ReturnMemView)
      ReturnModeViewRefl = VIEWMEM;//SETPARAM -  чтобы вернуться в предпросмотр
      else
      ReturnModeViewRefl = SETPARAM;//-  чтобы вернуться в предпросмотр
        
      ModeDevice = MODEREFL;
      myBeep(10);
      NeedCallView = 1;
    }
  if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED)&&DeleteTrace)
    // обработка кнопки Ок При удалении
  {
      // удаляем выбранную рефлектограмму
      Trace = DeletingTrace (Trace);
      SetNumTrace(Trace);
      g_FirstScr = 1; // надо перечитать трассу
      ModeMemDraw = VIEWNEXT;

      //ModeMemDraw = VIEWSAVED;
      MsgDel = 2;
      g_NeedScr = 1;
      DeleteTrace = 0;
      myBeep(20);
  }
  
  if ((PRESS(BTN_DOWN))&&(getStateButtons(BTN_DOWN)==SHORT_PRESSED))
  {
    if ( ModeMemDraw == VIEWSAVED) // можно удалять если выбрана рефлектограмма
    {
      myBeep(10);
      if ((Trace != 0)&&(GetNumTraceSaved(0)>0))
      {
        if (DeleteTrace == 0)
        {
          DeleteTrace = 1;
          MsgDel = 1;
          g_NeedScr = 1;
        }
      }
    }
  }
  
  if ((PRESS(BTN_UP))&&((getStateButtons(BTN_UP)==SHORT_PRESSED)||(getStateButtons(BTN_UP)==INF_PRESSED)))
  {
    
    myBeep(5);
    DeleteTrace = 0;
    if(g_mem_param) g_mem_param = 0; // переключаем что рисуем про измерения
    else g_mem_param = 1;
    // перерисуем картинку!
    g_FirstScr = 1; // надо перечитать трассу
    DrawMemoryRefl(Trace, CurrLang, g_mem_param);
    ModeMemDraw = VIEWSAVED;
   
  }
  if ((PRESS(BTN_LEFT))&&((getStateButtons(BTN_LEFT)==SHORT_PRESSED)||(getStateButtons(BTN_LEFT)==INF_PRESSED)))
  {
    
    myBeep(5);
    DeleteTrace = 0;
    if (Trace > 0) Trace--;
    else Trace = GetNumTraceSaved(0);
    SetNumTrace(Trace);
    ModeMemDraw = VIEWNEXT;
    g_FirstScr = 1; // надо перечитать трассу
  }
  if ((PRESS(BTN_RIGHT))&&((getStateButtons(BTN_RIGHT)==SHORT_PRESSED)||(getStateButtons(BTN_RIGHT)==INF_PRESSED)))
  {
    myBeep(5);
    DeleteTrace = 0;
    if (Trace < GetNumTraceSaved(0)) Trace++;
    else Trace =0;
    SetNumTrace(Trace);
    ModeMemDraw = VIEWNEXT;
    g_FirstScr = 1; // надо перечитать трассу
  }
// переключатель отображения по режимам
  switch (ModeMemDraw)
  {
  case SAVEDTRACE:
    //ClearScreen(); внесем в функцию прорисовки окна
    DrawMemoryRefl(Trace, CurrLang, g_mem_param);
    ModeMemDraw = VIEWSAVED;
    break;
  case MEMFULL:
    //ClearScreen();
    DrawMemoryRefl(0, CurrLang, g_mem_param); //вызываем нулевую рефлектограмму и пишем сообщение о полной памяти 
    sprintf(Stra,"%s    %d/%d",MsgMass[63][CurrLang], GetNumTraceSaved(0), MAXMEMALL);//  "НЕТ ПАМЯТИ" 
    sprintf( Str,"xstr 200,150,200,40,2,2047,BLACK,0,1,3,\"%s\"яяя",Stra); // 0для обоих индикаторов 
    NEX_Transmit((void*)Str);//

    ModeMemDraw = VIEWSAVED;
    break;
  case VIEWNEXT:
    //ClearScreen();
    DrawMemoryRefl(Trace, CurrLang, g_mem_param);
    // востанавливаем расчетные значения (при отображении рефлектограммы)
    PointsPerPeriod = NumPointsPeriod[GetIndexLN()]; // SetPointsPerPeriod( ... );
    //ValueDS = (unsigned)((ADCPeriod*50000)/PointsPerPeriod); //  устанавливаем значения DS для установленного режима измерения
    ValueDS = GetValueDS(); //  устанавливаем значения DS для установленного режима измерения
    ModeMemDraw = VIEWSAVED;
    
    break;
  case DELTRACE:
    
    break;
  case VIEWSAVED:
    // здесь 14.11.2022 можно запустить измерение с установленными параметрами из окна просмотра рефлектограмм по кнопке "S"
    //NeedReStartMeasure = 1;
    //ClearScreen();
    //DrawMemoryRefl(Trace, CurrLang, g_mem_param);
    break;
  }
  //
  if(g_FirstScr)
  {
    g_FirstScr = 0;
    g_NeedScr = 1;
  }
  if(g_NeedScr)
  {
    switch(MsgDel)
    {
    case 1:
      sprintf(Stra,"%s (Ok)", MsgMass[66][CurrLang]);//"удаляем?"
      sprintf( Str,"xstr 200,150,200,40,2,2047,BLACK,0,1,3,\"%s\"яяя",Stra); // 0для обоих индикаторов
      NEX_Transmit((void*)Str);// 
      MsgDel = 0;
      break;
    case 2:
      sprintf(Stra,"%s    %d/%d",MsgMass[67][CurrLang], GetNumTraceSaved(0), MAXMEMALL);
      sprintf( Str,"xstr 200,130,200,40,2,2047,BLACK,0,1,3,\"%s\"яяя",Stra); // 0для обоих индикаторов
      NEX_Transmit((void*)Str);//
      MsgDel = 0;
      break;
    default:
      break;
    }
    // надо добавить записи: - вопрос про удаление, - подтверждение удаления
    //      sprintf(Stra,"<ZOOM>");// признак кправления горизонтальным зумом
      //sprintf(Stra,"<Z1:%d>",GetSetHorizontScale(0));// признак кправления горизонтальным зумом

    g_NeedScr = 0;
  }
  if(NeedCallView)
  {
    // посылка команды переключения окна на DrawOTDRview (возврат)  
    CmdInitPage(18);
         //надо время что бы переключится
    // возможно здесь многовато будет... 13.02.2024
    // ставил в 10 раз меньше не влияет
    //CreatDelay(1000000); //  до 173
    //CreatDelay(100000); //  173
    //CreatDelay(800000);// 176
    CreatDelay(100000);// 177  вернул к 173

    NeedCallView = 0;
  }
  
  if ((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))
  {
    // здесь над витвится в зависимости от признака откуда пришли
    if(ReturnMemView)
    {
   DeleteTrace = 0;
   SetMode(ModeSelectMEM);
   ModeDevice = MODEOTHER;
   myBeep(10);
    // посылка команды переключения окна на Memory (возврат)  
      CmdInitPage(4);
    }
    else
    {// окно просмотра рефлектограммы
      TraceERASE(0);
      TraceWRITE(0); // записываем рефлектограмму с параметрами сьема  в 0 ячейку
      DeleteTrace = 0;
      GetSetHorizontScale (5);// вся картинка на экран
      SetMode(ModeDrawOTDR);
      ReturnModeViewRefl = SETPARAM;//SETPARAM -  чтобы вернуться в предпросмотр
      ModeDevice = MODEREFL;
      myBeep(10);
    // посылка команды переключения окна на DrawOTDRview (возврат)  
    CmdInitPage(18);
      
    }
  }
 // if (((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==INF_PRESSED))||
 //    ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==INF_PRESSED))) // если продолжительное нажатие Left Right
    //ClrKey (BNS_MASK-BTN_RIGHT-BTN_LEFT);
 // else
    //ClrKey (BNS_MASK);
  //CalkEventsKeys (LogData, PointsInImpulse(0), 1)
  // в версии 154 убрана обработка
//  if(NeedReStartMeasure)
//  {
    if (rawPressKeyS) // 17.11.2022 хотим запустить измерение
    {        
      myBeep(10);
      // если стартуем перепишем выбранную длину волны в соответствии с памяти
      SetIndxSeqLS();
       //g_SetModeLW = SettingRefl.SW_LW;
      g_AutoSave = 0; // длина волны одиночная сброс авто сохранения/
        
      ReSaveWAV_SC (); // пересохраняем если есть изменения
      LSEL0(0);
      LSEL1(0);
      SlowON();
      //POWALT(ON);
      //POWREF (ON);
      //POWDET(ON);
      SetMode(ModeStartOTDR);
      ModeDevice = MODEMEASURE;
      ReturnMemView = 0; // как бы запустились из установок
      ReturnModeViewRefl = SETPARAM;//SETPARAM -  чтобы вернуться в установки
      //IndexVerSize  = 0;// установка вертикального размера отображения рефлектограммы ( самый крупный)
      SubModeMeasOTDR = SETPOWER;
      //123 SSPInit_Any(SPI_ALT); // Инициализация SSP для управления ALTERA (порт 1 та что на плате отладочной)
      rawPressKeyS=0;
      
      //CreatDelay (30000); // 3.3 мС
  HAL_Delay(4);
      CmdInitPage(16);// посылка команды переключения окна на начало измерения Measuring
    }
//    NeedReStartMeasure = 0;
//  }
  
      // сделаем "тупую задержку"
    // похоже задержка слишком велика, пропускаем команды по UART 13.02.2024?
    // ставил 20000 - не поменялось
    //CreatDelay (20000); //173 и 176,177
  HAL_Delay(3);

}


void ModeSelectOLT(void) // режим выбора типа тестера CHECK_OFF
{
  static volatile BYTE FrSelectOLT = 0; // указатель на курсор
  BYTE RedEye = GetCfgRE(); // проверим есть ли кр. глаз
  char Str[32];
  char StrI[32];
  //BYTE CurrLang=GetLang(CURRENT);
  //DWORD KeyP = SetBtnStates( KEYS_REG, 1 );
  if ((PRESS(BTN_UP))&&(getStateButtons(BTN_UP)==SHORT_PRESSED))
  {
    myBeep(10);
    FrSelectOLT = ChangeFrSet (FrSelectOLT, 1+RedEye, 0, MINUS);// установка курсора в рамках заданных параметров
    g_NeedScr = 1; // Need reDraw Screen
    //ClrKey (BTN_UP);
  }
  if ((PRESS(BTN_DOWN))&&(getStateButtons(BTN_DOWN)==SHORT_PRESSED))
  {
    myBeep(10);
    FrSelectOLT = ChangeFrSet (FrSelectOLT, 1+RedEye, 0, PLUS);// установка курсора в рамках заданных параметров
    g_NeedScr = 1; // Need reDraw Screen
    //ClrKey (BTN_DOWN);
  }
  if((RedEye)&&(FrSelectOLT==1+RedEye)) //если красный глаз и стоим на нем
  {
    if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      SetModeRE(Str,INCR,CurrLang );
      //ClrKey (BTN_RIGHT);
    }
    if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      SetModeRE(Str,DECR,CurrLang );
      //ClrKey (BTN_LEFT);
    }
  }
  if(g_FirstScr)
  {
    // здесь заполняем данными поля нового индикатора
    // не требущие изменения при первичной инициализации
    sprintf(Str, "t0.txt=\"%s\"яяя", MsgMass[49][CurrLang]); // manual
    NEX_Transmit((void*)Str);    //
    
    sprintf(Str, "t1.txt=\"%s\"яяя", MsgMass[50][CurrLang]); // auto
    NEX_Transmit((void*)Str);    // 
    // про красный глаз
    if(RedEye)
    {
      sprintf(Str,"t2.txt=\"VFL,    0.65 \"яяя"); // Red Eye
      NEX_Transmit((void*)Str);    // 
      // ветвление в зависимости от типа индикатора
      if(TypeLCD) //3.5
      {
      sprintf(Str,"draw 272,113,384,156,BLACKяяя"); // Red Eye
      NEX_Transmit((void*)Str);    // 
      sprintf(Str,"draw 271,112,385,157,BLACKяяя"); // Red Eye
      NEX_Transmit((void*)Str);    // 
        
      }
      else
      {
      sprintf(Str,"draw 270,87,382,118,BLACKяяя"); // Red Eye
      NEX_Transmit((void*)Str);    // 
      sprintf(Str,"draw 269,86,383,119,BLACKяяя"); // Red Eye
      NEX_Transmit((void*)Str); 
      }   // 
    }
    else
    {
      sprintf(Str,"t2.txt=\" \"яяя"); // пустая строка
      NEX_Transmit((void*)Str);    // 
      sprintf(Str,"t6.txt=\" \"яяя"); // пустая строка
      NEX_Transmit((void*)Str);    // 
    }
    // индикация ошибки измериттеля
    if (ErrOLT)  //
    {
      sprintf(Str,"t5.txt=\"%s :(%2X)\"яяя", MsgMass[115][CurrLang], ErrOLT); // 
    }
    else
      sprintf(Str,"t5.txt=\" \"яяя"); // пустая строка
    NEX_Transmit((void*)Str);    // 
    
    
    // совместимость с широкодиапазонным вариантом
    // индикация расширенного диапазона
    if (!(WIDE_VER))  // если есть перемычка для расширенного диапазона
    {
      // переключаем совместимость
      if (((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))||((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED)))
      {
        myBeep(10);
        if(SetJDSU.CompMode) SetJDSU.CompMode = 0;
        else SetJDSU.CompMode = 1;
      }
      sprintf(Str,"t3.txt=\"%s: %c\"яяя", MsgMass[117][CurrLang],(SetJDSU.CompMode)?('J'):('T')); // 
      NEX_Transmit((void*)Str);    // 
      
      sprintf(Str,"t4.txt=\"W\"яяя"); // 
      NEX_Transmit((void*)Str);    // 
      
    }
    else
    {
      sprintf(Str,"t3.txt=\" \"яяя"); // пустые строки
      NEX_Transmit((void*)Str);    // 
      sprintf(Str,"t4.txt=\" \"яяя"); // 
      NEX_Transmit((void*)Str);    // 
    }
    
    
    g_FirstScr=0;
    g_NeedScr=1;
    
  }
  if(g_NeedScr)
  {
    // закрасим бэкграунды  и установим требуемый
    sprintf(Str,"t0.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);// 
    sprintf(Str,"t1.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);// 
    sprintf(Str,"t2.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);// 
    sprintf(Str,"t%d.bco=GREENяяя",FrSelectOLT); // зеленый
    NEX_Transmit((void*)Str);// 
    
    // про красный глаз
    if(RedEye)
    {
      SetModeRE (Str, CURRENT, CurrLang);
      sprintf(StrI,"t6.txt=\"%s\"яяя",Str); //режим
      NEX_Transmit((void*)StrI);    // 
    }
    
    g_NeedScr=0;
    
  }
  if ((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))
  {
    myBeep(10);
    SetMode(ModeMainMenu);
    POWDET(OFF); 
    SetupSource (0); // выключаем источник 
    ModeDevice = MODEMENU;
    //123 SSPInit_Any(MEM_FL1); // Востанавливаем Инициализацию SSP для управления внешней FLASH (порт 1 та что на плате отладочной)
    // посылка команды переключения окна на MainMenu (возврат)  
    CmdInitPage(1);
    
  }
  if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
  {
    switch (FrSelectOLT)
    {
    case 0:
      myBeep(10);
      //POWDET(ON);
      SetMode(ModeMeasManualOLT);
      //123 enable_timer(3);  /* Enable Interrupt Timer3 */
      //123 enable_timer(1);  /* Enable  Timer1 JDSU */
      ModeDevice = MODETEST;
      ModePowerMeter = POWMANUAL;
      //123 SSPInit_Any(SPI_PM);      //Перевели SPI на АЦП
      // посылка команды переключения окна на Test_manual (вызов)  
      CmdInitPage(7);
      
      break;
    case 1: // автомат
      myBeep(10);
      //POWDET(ON);
      //UARTInit1 (1200);
      //VICINTENCLEAR | = 1 << UART1_INT; /* Disable Interrupt */
//123      VICINTENABLE = 1 << UART1_INT;  /* Enable Interrupt */
      SetMode(ModeMeasAutoOLT);
//123      enable_timer(3);  /* Enable Interrupt Timer3 */
//123      enable_timer(1);  /* Enable  Timer1 JDSU */
      ModeDevice = MODETEST;
      ModePowerMeter = POWAUTO;
      InitAutoPM (); // начальные установки измерителя автомата
//123      SSPInit_Any(SPI_PM);      //Перевели SPI на АЦП
      // посылка команды переключения окна на Test_auto (вызов)  
      CmdInitPage(8);
      break;
    }
//123    P08_Init();
  }
  
}

float GetLastPower (void)// возвращение померенной мощности 
{
  return LastPower;
}

void ModeMeasManualOLT(void) // режим работы тестера в ручном режиме CHECK_OFF
{
  static BYTE FrManualOLT = 1; // указатель на курсор
  static BYTE MemIndexLW;
  BYTE NowIndexLW;

  //BYTE AddRed;
  char Str[32];
  char Stra[32];
  char Strb[32];
  char Strc[32];
  BYTE RedEye = GetCfgRE();
  //BYTE CurrLang=GetLang(CURRENT);
  //DWORD KeyP = SetBtnStates( KEYS_REG, 1 );
  static float tmp=0;
  static BYTE i=0;                 //счетчик для выводна на экран
  
  if(i==0)
  {
    tmp =  GetPower(GetPMWavelenght(0)); // Получаем мощность в мВт
    LastPower = tmp;
    i=10;
   //      g_NeedScr = 1; // Need reDraw Screen
  }
  else
  {
   // LastPower = GetPower(GetPMWavelenght(0)); // Получаем мощность в мВт
 
  }
    
  i--;
  
//      switch(GetCfgPM())
//    {
//    case 1:
//      if (tmp<1e-9)tmp = 1e-9;                          //ограничение показаний на уровне 1pW 
//      break;
//    case 2:
//      if (tmp<1e-6)tmp = 1e-6;                          //ограничение показаний на уровне 1pW 
//      break;
//    }
//
  
  //Обрабтываем кнопки :
  
  //Кнопка "Меню"  
  switch(FrManualOLT)
  {
  case 1:   // Опора REF
    if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))
    {
      myBeep(10);
      SetTypeRslt(GetTypeRslt()+1);
    }
    if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
    {
     
    }
    
    if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))// перепривязка на выбранной длине волны
    {
      myBeep(15);
      SetCurrLvldB(GetPMWavelenght(0),10*log10(tmp)); // здесь записываем в память
      SetTypeRslt(1); // set dB
      g_NeedScr = 1; // Need reDraw Screen
    }
    
    if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==LONG_PRESSED))
    {
      if (GetRange() == 0)
      {
      myBeep(85);
      AcquireShZeroLowRng();                 // руная переустановка нуля на чувствительном диапазоне
      }
      //AcquireCoefStykRange(1);
    }
    
    break;
  case 2:// Изменение длины волны
    if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      GetCurrLvldB(GetPMWavelenght(-1));
    };
    
    if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==INF_PRESSED))
    { 
      g_NeedScr = 1; // Need reDraw Screen
      GetCurrLvldB(GetPMWavelenght(-1));
    };
    
    if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))
    {    
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      GetCurrLvldB(GetPMWavelenght(1));
    }
    
    if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==INF_PRESSED))
    {    
      g_NeedScr = 1; // Need reDraw Screen
      GetCurrLvldB(GetPMWavelenght(1));
    }
    
    
    
    if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
    {    
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      GetCurrLvldB(GetPMWavelenght(13));
    }
    
    break;
    
  }
  
  // установка длинны волны если включен Wide и JDSU поддержка
  SetAutoLamdaJDSU (TimerValueJDSU);
  // Для источника и красного лазера
//AddRed =  DrawSourceVFL (&FrManualOLT, Str, KeyP, CurrLang, 3, 1);
DrawSourceVFL (&FrManualOLT, Str, KeyP, CurrLang, 3, 1);
  
  // Первая строка  
  
    SetStringPM(Str, tmp);           // устанавливает строку данных измерения
    sprintf(Stra, "t0.txt=\"%s\"яяя", Str);
    NEX_Transmit((void*)Stra);    // ORL


  
  if (g_FirstScr)
  {
    // здесь заполняем данными поля нового индикатора
    // не требущие изменения при первичной инициализации
    
    sprintf(Str, "t2.txt=\"REF, %s\"яяя", MsgMass[48][CurrLang]);
    NEX_Transmit((void*)Str);    // REF
    
    sprintf(Str, "t4.txt=\"%s\"яяя", MsgMass[17][CurrLang]);
    NEX_Transmit((void*)Str);    // длина волны
    
    sprintf(Str, "t6.txt=\"%s\"яяя", MsgMass[68][CurrLang]);
    NEX_Transmit((void*)Str);    // источник
    
    sprintf(Str, "t8.txt=\"ORL, %s\"яяя", MsgMass[47][CurrLang]);
    NEX_Transmit((void*)Str);    // ORL
    // пустышки
    // зачистим не РАБОЧИЕ поля
        sprintf(Str, "t10.txt=\"\"яяя");
    NEX_Transmit((void*)Str);    // 
        sprintf(Str, "t13.txt=\"\"яяя");
    NEX_Transmit((void*)Str);    // 
    // номер заполненой ячейки
  sprintf(Str,"t9.txt=\"№ %04d\"яяя",GetCellMem(0));
  NEX_Transmit((void*)Str);    // № ячейки
    
    // индикация расширенного диапазона
    if (!(WIDE_VER))  // если есть перемычка для расширенного диапазона
    {
      sprintf(Str,"t12.txt=\"%c\"яяя",(SetJDSU.CompMode)?('J'):('T')); // 
    }
    else
    {
      sprintf(Str, "t12.txt=\" \"яяя"); // пустая строка
    }
    NEX_Transmit((void*)Str);    // W
    
    
    g_FirstScr = 0;
    g_NeedScr = 1;
  }
  if (g_NeedScr)
  {
    // раскрашивание поля выбора 
    // закрасим бэкграунды  и установим требуемый
    sprintf(Str, "t2.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);//
    sprintf(Str, "t4.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);// 
    sprintf(Str, "t6.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);// 
    sprintf(Str, "t8.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);//
    sprintf(Str, "t10.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);//
    sprintf(Str, "t%d.bco=GREENяяя", (FrManualOLT)<<1); // зеленый
    NEX_Transmit((void*)Str);// 
    // здесь заполняем данными поля нового индикатора
    // по результатам изменений вызваныйх обработчиком клавиатуры
  sprintf(Str,"t1.txt=\"%2.3f\"яяя",GetCurrLvldB(0)); // dBm REF
    NEX_Transmit((void*)Str);    //

  sprintf(Str,"t3.txt=\"%d%s\"яяя",GetPMWavelenght(0),MsgMass[18][CurrLang]); // nm
    NEX_Transmit((void*)Str);    //
  
    if(g_VolORL!=0)sprintf(Str,"t7.txt=\"%.2f\"яяя",g_VolORL);
    else sprintf(Str,"t7.txt=\"???\"яяя");
    NEX_Transmit((void*)Str);    // значение ORL
    
    // строка про источники ()
        sprintf(Str, "t6.txt=\"%s\"яяя", MsgMass[68][CurrLang]);// source
        SetModeLS (Strb, CURRENT, CurrLang); // получаем режим источника
        sprintf(Strc,"t5.txt=\"%s\"яяя",Strb);
    NEX_Transmit((void*)Str);    // источник
    
    NEX_Transmit((void*)Strc);    // источник
    
    // запомним индекс длины волны источника
    MemIndexLW = GetPlaceLS(CURRENT);
    //NowIndexLW = MemIndexLW;
        sprintf(Stra,"t11.txt=\"%.2f\"яяя",GetLengthWaveLS (MemIndexLW)/1000.0);
    NEX_Transmit((void*)Stra);    // источник
    
                                       // код подсветки требуемой строки если есть есть маркер строки
    g_NeedScr = 0;
  }

    // порисуем длину воны источника если изменилась
  NowIndexLW = GetPlaceLS(CURRENT);
  if(MemIndexLW != NowIndexLW)
  {
    sprintf(Stra,"t11.txt=\"%.2f\"яяя",GetLengthWaveLS (NowIndexLW)/1000.0);
    NEX_Transmit((void*)Stra);    // источник
    MemIndexLW = NowIndexLW;
  }

  if ((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))
  {
//123    P08_IRQ_EN(OFF); // запрещаем прерывание по готовности АЦП
    myBeep(10);
    SetMode(ModeSelectOLT);
    ModeDevice = MODEOTHER;
    ModePowerMeter = NOTMODE;
          // посылка команды переключения окна на Tester (возврат)  
      CmdInitPage(3);
  }
  if (rawPressKeyS) // key S первый заход в редактор сохр.
  {        
    myBeep(10);
    SetMode(ModeSaveOLT);
    SavePowerMeter(tmp);
    ReLoadCommOLT (); // перезагружаем комментарии для измерителя
    ModeDevice = MODEOTHER;
    rawPressKeyS=0;
          // посылка команды переключения окна на Tester (возврат)  
      CmdInitPage(24);
  }
  
}

void SetStringPM(char *Str, float mWatt)           // устанавливает строку данных измерения
{
  BYTE TypeRslt = GetTypeRslt();
  switch (TypeRslt)
  {
  case 1: //dB
    Watt2dB(Str, mWatt,1);
    sprintf(Str,"%s %s", Str,MsgMass[47][CurrLang]); // Значение dB
    break;
  case 2: // mW
    if(mWatt>1.0)                                  // Если мощность больше 1мВт     
      sprintf(Str,"% 6.2f %s", mWatt, MsgMass[97][CurrLang]);            // Значение в мВт
    else if(mWatt*1e3>1.0)                        // Если больше 1 мкВт
      sprintf(Str,"% 6.2f %s", mWatt*1e3, MsgMass[96][CurrLang]);       // Значение в мкВт
    else if(mWatt*1e6>1.0)                        // Если больше 1 нВт
      sprintf(Str,"% 6.2f %s", mWatt*1e6, MsgMass[95][CurrLang]);       // Значение в нВт
    else 
      // индикация в зависимости от сферы
          switch(GetCfgPM())
    {
    case 1:
      if (mWatt<1e-9)mWatt = 1e-9;                          //ограничение показаний на уровне 1pW 
      sprintf(Str,"% 6.2f %s", mWatt*1e9, MsgMass[94][CurrLang]);       // Значение в пВт
      break;
    case 2:
          if (WIDE_VER)  // если нет перемычки для расширенного диапазона
          {
      if (mWatt<=1e-6)mWatt = 1e-6;                          //ограничение показаний на уровне 1nW 
      sprintf(Str,"<%6.2f %s", mWatt*1e6, MsgMass[95][CurrLang]);       // Значение в нВт
          }
          else
          {
      if (mWatt<=1e-8)mWatt = 1e-8;                          //ограничение показаний на уровне 10pW 
      sprintf(Str,"<%6.2f %s", mWatt*1e9, MsgMass[95][CurrLang]);       // Значение в пВт
          }
          
      break;
    }

    break;
  default: // показания в дБм...
    Watt2dB(Str, mWatt,0);
          switch(GetCfgPM())
    {
    case 1:
      if (mWatt<=1e-9)
      {
        mWatt = 1e-9;                          //ограничение показаний на уровне 1pW  -90 dBm
      sprintf(Str,"<-90.00 %s", MsgMass[48][CurrLang]); // Значение dBm
      }
      else
      sprintf(Str,"%s %s", Str,MsgMass[48][CurrLang]); // Значение dBm
      break;
    case 2:
          if (WIDE_VER)  // если нет перемычки для расширенного диапазона
          {
            if (mWatt<=1e-6)
            {
              mWatt = 1e-6;                          //ограничение показаний на уровне 1nW -60 dBm
              sprintf(Str,"<-60.00 %s", MsgMass[48][CurrLang]); // Значение dBm
            }
            else
              sprintf(Str,"%s %s", Str,MsgMass[48][CurrLang]); // Значение dBm
          }
          else
          {
            if (mWatt<=1e-8)
            {
              mWatt = 1e-8;                          //ограничение показаний на уровне 10pW -80 dBm
              sprintf(Str,"<-80.00 %s", MsgMass[48][CurrLang]); // Значение dBm
            }
            else
              sprintf(Str,"%s %s", Str,MsgMass[48][CurrLang]); // Значение dBm
          }
      break;
    }
    break;
  }
}

void ModeMeasAutoOLT(void) // режим работы тестера в автоматическом режиме
{
  
  static BYTE FrAutoOLT = 3; // указатель на курсор
  char Str[32];
  char Stra[32];
  char Strb[32];
  char Strc[32];
  static WORD NumWave=1310;
  static WORD TempNumWave=1310;
  static float tmpPOW;
  static BYTE MemIndexLW;
  //char wrd[10];
  BYTE NowIndexLW;
  //static BYTE NeedSetNewLVL = 0;
  BYTE RedEye = GetCfgRE();
  //BYTE AddRed;
  //BYTE CurrLang=GetLang(CURRENT);
  //DWORD KeyP = SetBtnStates( KEYS_REG, 1 );
  TimerAutoPM = TimerPA(0); // ~100mS период
  //AddRed = DrawSourceVFL (&FrAutoOLT, Str, KeyP, CurrLang, 3, 3);
  DrawSourceVFL (&FrAutoOLT, Str, KeyP, CurrLang, 3, 3);
  /* */  
  if ((PRESS(BTN_UP))&&(getStateButtons(BTN_UP)==LONG_PRESSED))
  {
    myBeep(10);
    for (int i=0; i<5; i++)
    {
      if ( RPON[i]>-100.0)
      {
        SetNewLeveldB (RPON[i], GetIndxPON(LWPON[i])); // установка нового уровня привязки по принятым волнам
      }
    }
    //123 SSPInit_Any(MEM_FL1); // Инициализация SSP для управления FLASH (порт 1 та что на плате отладочной)
    WriteNeedStruct (0x04); // перезапись страницу установок пользователя прибора
    //FlashErasePage(CFG_USER); // чистим страницу установок пользователя прибора
    //FlashWritePageSM(CFG_USER, StructPtr(CFG_USER), StructSize(CFG_USER), 0);
    //123 SSPInit_Any(SPI_PM); // востановление SSP для управления PM (порт 1 та что на плате отладочной)
  }
  //SetRange(1);
  if (RSOptYes) // получаем длину волны кoторая сейчас будет излучаться
  {
//123!!!    TempNumWave = GetNumWaveOpt (); // получаем номер длины волны
    TempNumWave = 1310; // получаем номер длины волны
    if (TempNumWave)
    { 
      // 1. посчитаем к какому диапазону приходится волна 
       IndWavePON=GetIndxPON(TempNumWave);
       LWPON[IndWavePON]=TempNumWave; // запишем длину волны в ячейку (для индикации)
       LWCNT[IndWavePON] = 0; // clear cnt текущей волны
      // myBeep(5); //писк подтверждения приема
      for (int i=0; i<5; i++)
      {
       if (LWPON[i] != 0) // если есть запись и совпадает с пришедшей
       {
          if(LWCNT[i]++>3)
            {
              RPON[i]=-100.0;
              LWPON[i] = 0; // обнуляем список длин волн
              LWCNT[i] = 0; // clear cnt 
            }
       }
       else
       {
              RPON[i]=-100.0;
              LWPON[i] = 0; // обнуляем список длин волн
              LWCNT[i] = 0; // clear cnt 
       
       }
      }
//      for (int i=0; i<5; i++)
//      {
//        if ((LWPON[i] != 0)&&(TempNumWave==LWPON[i])) // если есть запись и совпадает с пришедшей
//          // if (TempNumWave == CoeffPM.PointKalib[i])
//        {
//          IndWavePON = i;// индекс ячейки памяти RPON где хранятся промежуточные результаты
//          LWCNT[i] = 0; // clear cnt 
//          break;
//        }// установка индекса принятой волны индексируется согласно длин волн калибровки
//        else
//        {
//          if(LWPON[i] == 0) // нет волны запишем новую
//          {
//            LWPON[i]=TempNumWave;
//            IndWavePON = i;
//            break;
//          }
//          // волна есть но долго не обновляется
//          else
//          {
//            if(LWCNT[i]++>3)
//            {
//              RPON[i]=-100.0;
//              LWPON[i] = 0; // обнуляем список длин волн
//              LWCNT[i] = 0; // clear cnt 
//            }
//          }
//        }
//      }
      
      NumWave = TempNumWave;
      GetCurrLvldB(NumWave);
      //SetPMWavelenght (NumWave); // принудительная установка текущей длины волны
      
      ModAutoPM = MEASUR;
      TimerAutoPM = TimerPA(1); // обнуляем таймер
      SetRange(3); // самый грубый
      SetSwitchPMMode(AUTO);  // Устанавливает режим переключения диапазонов автомат
    }

  }
        // test code
    //sprintf(Str, "t8.txt=\"%s\"яяя",wrd);
    //sprintf(Str, "t8.txt=\"%d %d\"яяя",TimerAutoPM,ModAutoPM);
    //NEX_Transmit((void*)Str);    // 
  
  tmpPOW =  GetPower(NumWave); // Получаем мощность в мВт
  switch (ModAutoPM)
  {
  case WAITING:
    if (TimerAutoPM > 1000) //~10S
    {
      myBeep(10);
      InitAutoPM();
    }
    
    break;
    // измерение в автомате
    // возможно время  велико или мало, так как плохо работаем с простыми тесторами 06.06.2011
  case MEASUR:
    if (TimerAutoPM >= 150) //~1.5S
    {
      RPON[IndWavePON] = Watt2dB(Str, tmpPOW , 1);
      ModAutoPM = WAITING; // режим измерителя авто ожидаем
      TimerAutoPM = TimerPA(1); // счетчик времени (обнуляем)
      SetSwitchPMMode(MANUAL);  // Устанавливает режим переключения диапазонов в ручную
      SetRange(1);
    }
    
    break;
  }
  BYTE   M_PON =0;
  // рисуем значения в новый индикатор
 // каждый раз!? может надо сделать реже
    // Рисуем строки измерения
  M_PON = 0;
  for (int i=0; i<5; ++i)
  {
    if ((RPON[i]>-100.0)&&(M_PON<3)) // есть значение для индикации
    {
      //DrawLine(2,12*M_PON+5,114,12*M_PON+5,11,0); //чистка индикатора показаний
      sprintf(Str,"t%d.txt=\"%6.3f %s\"яяя",M_PON+1, RPON[i], MsgMass[47][CurrLang]);//дБ
    NEX_Transmit((void*)Str);    // значение
      //sprintf(Str,"t%d.txt=\"%d%s\"яяя",M_PON+4,CoeffPM.PointKalib[i], MsgMass[18][CurrLang]);//нм 1300
      sprintf(Str,"t%d.txt=\"%d%s\"яяя",M_PON+4,LWPON[i], MsgMass[18][CurrLang]);//нм
    NEX_Transmit((void*)Str);    // длина волны
      M_PON++;
    }
  }
  for(int i=M_PON; i<3;++i)
  {
    //DrawLine(2,i*12+5,114,i*12+5,11,0); //чистка индикатора показаний
    sprintf(Str,"t%d.txt=\"--------\"яяя",i+1);
    NEX_Transmit((void*)Str);    // значение
    sprintf(Str,"t%d.txt=\"----\"яяя",i+4);
    NEX_Transmit((void*)Str);    // длина волны
  }
  
  
  if (g_FirstScr)
  {
    // здесь заполняем данными поля нового индикатора
    // не требущие изменения при первичной инициализации
    
 //   sprintf(Str, "t2.txt=\"REF, %s\"яяя", MsgMass[48][CurrLang]);
 //   NEX_Transmit((void*)Str);    // REF
    
 //   sprintf(Str, "t4.txt=\"%s\"яяя", MsgMass[17][CurrLang]);
 //   NEX_Transmit((void*)Str);    // длина волны
    
    sprintf(Str, "t7.txt=\"%s\"яяя", MsgMass[68][CurrLang]);
    NEX_Transmit((void*)Str);    // источник
    
    sprintf(Str, "t8.txt=\"ORL, %s\"яяя", MsgMass[47][CurrLang]);
    NEX_Transmit((void*)Str);    // ORL
    // пустышки
    // зачистим не РАБОЧИЕ поля
        sprintf(Str, "t9.txt=\"\"яяя");
    NEX_Transmit((void*)Str);    // 
    // test code
    //memcpy(wrd ,RX_BufOpt,9);
        sprintf(Str, "t13.txt=\"\"яяя");
   //wrd[9]=0;
    //    sprintf(Str, "t13.txt=\"%s\"яяя",wrd);
    NEX_Transmit((void*)Str);    // 
    // номер заполненой ячейки
  sprintf(Str,"t12.txt=\"№ %04d\"яяя",GetCellMem(0));
  NEX_Transmit((void*)Str);    // № ячейки
    
    // индикация расширенного диапазона
    if (!(WIDE_VER))  // если есть перемычка для расширенного диапазона
    {
      sprintf(Str,"t0.txt=\"%c\"яяя",(SetJDSU.CompMode)?('J'):('T')); // 
    }
    else
    {
      sprintf(Str, "t0.txt=\" \"яяя"); // пустая строка
    }
    NEX_Transmit((void*)Str);    // W
    
    
    g_FirstScr = 0;
    g_NeedScr = 1;
  }
  if (g_NeedScr)
  {
    // здесь заполняем данными поля нового индикатора
    // по результатам изменений вызваныйх обработчиком клавиатуры
 // sprintf(Str,"t1.txt=\"%2.2f\"яяя",GetCurrLvldB(0)); // dBm REF
 //   NEX_Transmit((void*)Str);    //

 // sprintf(Str,"t3.txt=\"%d%s\"яяя",GetPMWavelenght(0),MsgMass[18][CurrLang]); // nm
 //   NEX_Transmit((void*)Str);    //
    // раскрашивание поля выбора 
    // закрасим бэкграунды  и установим требуемый
    sprintf(Str, "t7.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);//
    sprintf(Str, "t8.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);// 
    sprintf(Str, "t9.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);// 
    sprintf(Str, "t%d.bco=GREENяяя", (FrAutoOLT)+4); // зеленый
    NEX_Transmit((void*)Str);// 
                                       // код подсветки требуемой строки если есть есть маркер строки
  
    // строка про источники ()
        sprintf(Str, "t7.txt=\"%s\"яяя", MsgMass[68][CurrLang]);// source
        //sprintf(Stra,"t14.txt=\"%.2f\"яяя",GetLengthWaveLS (GetPlaceLS(CURRENT))/1000.0);
        SetModeLS (Strb, CURRENT, CurrLang); // получаем режим источника
        sprintf(Strc,"t10.txt=\"%s\"яяя",Strb);
    NEX_Transmit((void*)Str);    // источник
    
    //NEX_Transmit((void*)Stra);    // источник
    NEX_Transmit((void*)Strc);    // источник
  
    if(g_VolORL!=0)sprintf(Str,"t11.txt=\"%.2f\"яяя",g_VolORL);
    else sprintf(Str,"t11.txt=\"???\"яяя");
    NEX_Transmit((void*)Str);    // значение ORL
    // запомним индекс длины волны источника
    MemIndexLW = GetPlaceLS(CURRENT);
    //NowIndexLW = MemIndexLW;
        sprintf(Stra,"t14.txt=\"%.2f\"яяя",GetLengthWaveLS (MemIndexLW)/1000.0);
    NEX_Transmit((void*)Stra);    // источник
    
    
    g_NeedScr = 0;
  }
    //sprintf(Str,"%d",TimerValueJDSU);
    //putString(82,12*2+6,Str,1,0);
    // порисуем длину воны источника если изменилась
  NowIndexLW = GetPlaceLS(CURRENT);
  if(MemIndexLW != NowIndexLW)
  {
    sprintf(Stra,"t14.txt=\"%.2f\"яяя",GetLengthWaveLS (NowIndexLW)/1000.0);
    NEX_Transmit((void*)Stra);    // источник
    MemIndexLW = NowIndexLW;
  }
  
  
  if ((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))
  {
    myBeep(10);
    //UARTInit1(1200);//, OFF
//123    VICINTENCLEAR  = 1 << UART1_INT; /* Disable Interrupt */
//123    disable_timer(3); /* Disable Interrupt Timer3*/
//123    disable_timer(1); /* Disable  Timer1 JDSU*/
    
    SetMode(ModeSelectOLT);
    ModeDevice = MODEOTHER;
    ModePowerMeter = NOTMODE;
    SetSwitchPMMode(AUTO);  // Устанавливает режим переключения диапазонов в ручную
          // посылка команды переключения окна на Tester (возврат)  
      CmdInitPage(3);
  }
  
  if (rawPressKeyS) // key S 
  {        
    myBeep(10);
    SetMode(ModeSaveOLT);
    SavePowerMeter(tmpPOW);
    ReLoadCommOLT (); // перезагружаем комментарии для измерителя

    ModeDevice = MODEOTHER;
    //IndexVerSize  = 0;// установка вертикального размера отображения рефлектограммы ( самый крупный)
    rawPressKeyS=0;
          // посылка команды переключения окна на Tester (возврат)  
      CmdInitPage(24);
  }
}

void ModeSourceOnly(void) // режим работы тестера только источник CHECK_OFF
{
  static BYTE FrSourceOnly = 1; // указатель на курсор
  BYTE RedEye = GetCfgRE();
  static BYTE MemIndexLW;
  BYTE NowIndexLW;
  char Str[32];
  char Stra[32];
  char Strb[32];
  char Strc[32];  //BYTE CurrLang=GetLang(CURRENT);
  //DWORD KeyP = SetBtnStates( KEYS_REG, 1 );
  DrawSourceVFL (&FrSourceOnly, Str, KeyP, CurrLang, 1, 1);
  if (g_FirstScr)
  {
    // здесь заполняем данными поля нового индикатора
    // не требущие изменения при первичной инициализации
    GetDeviceLabel( Str, NameDB.Ena_DB  );
    sprintf(Stra, "t0.txt=\"%s\"яяя", Str);
    NEX_Transmit((void*)Stra);    // наименование прибора
    
    sprintf(Str, "t2.txt=\"ORL, %s\"яяя", MsgMass[47][CurrLang]);
    NEX_Transmit((void*)Str);    // ORL
    
    // красный глаз
    if(RedEye)
    {
      sprintf(Str,"t3.txt=\"VFL,    0.65 \"яяя"); // Red Eye
      NEX_Transmit((void*)Str);    // 
      sprintf(Str,"draw 284,159,383,188,BLACKяяя"); // Red Eye
      NEX_Transmit((void*)Str);    // 
      sprintf(Str,"draw 285,160,382,187,BLACKяяя"); // Red Eye
      NEX_Transmit((void*)Str);    // 
    }
    else
    {
      sprintf(Str,"t3.txt=\" \"яяя"); // пустая строка
      NEX_Transmit((void*)Str);    // 
      sprintf(Str,"t5.txt=\" \"яяя"); // пустая строка
      NEX_Transmit((void*)Str);    // 
    }
    
    
    
    g_FirstScr = 0;
    g_NeedScr = 1;
  }
  if (g_NeedScr)
  { 
    // раскрашивание поля выбора 
    // закрасим бэкграунды  и установим требуемый
    sprintf(Str, "t1.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);// 
    sprintf(Str, "t2.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);//
    sprintf(Str, "t3.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);//
    sprintf(Str, "t%d.bco=GREENяяя", (FrSourceOnly)); // зеленый
    NEX_Transmit((void*)Str);// 
    // здесь заполняем данными поля нового индикатора
    // по результатам изменений вызваныйх обработчиком клавиатуры
    // строка про источники ()
    // запомним индекс длины волны источника
    MemIndexLW = GetPlaceLS(CURRENT);
    NowIndexLW = MemIndexLW;
    sprintf(Str, "t1.txt=\"%s\"яяя", MsgMass[68][CurrLang]);// source
    sprintf(Stra,"t4.txt=\"%.2f\"яяя",GetLengthWaveLS (MemIndexLW)/1000.0);
    SetModeLS (Strb, CURRENT, CurrLang); // получаем режим источника
    sprintf(Strc,"t6.txt=\"%s\"яяя",Strb);
    NEX_Transmit((void*)Str);    // источник
    
    NEX_Transmit((void*)Stra);    // источник
    NEX_Transmit((void*)Strc);    // источник
    
    if(RedEye) // Red eye
    {
      SetModeRE (Strb, CURRENT, CurrLang);
      sprintf(Strc,"t5.txt=\"%s\"яяя",Strb);
      NEX_Transmit((void*)Strc);    // Red Eye
    }
    
    
    if(g_VolORL!=0)sprintf(Str,"t7.txt=\"%.2f\"яяя",g_VolORL);
    else sprintf(Str,"t7.txt=\"???\"яяя");
    NEX_Transmit((void*)Str);    // значение ORL
    
    
    // код подсветки требуемой строки если есть есть маркер строки
    g_NeedScr = 0;
  }
  // порисуем длину воны источника если изменилась
  NowIndexLW = GetPlaceLS(CURRENT);
  if(MemIndexLW != NowIndexLW)
  {
    sprintf(Stra,"t4.txt=\"%.2f\"яяя",GetLengthWaveLS (NowIndexLW)/1000.0);
    NEX_Transmit((void*)Stra);    // источник
    MemIndexLW = NowIndexLW;
  }
  
  if ((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))
  {
    myBeep(10);
    SetMode(ModeMainMenu);
    ModeDevice = MODEMENU;
    POWDET(OFF); 
    SetupSource (0); // выключаем источник 
    // посылка команды переключения окна на MainMenu (возврат)  
    CmdInitPage(1);
  }
}

void ModeSaveOLT(void) // режим сохранения результатов измерителя CHECK_OFF
{
  static BYTE FrSaveOLT = 1; // указатель на курсор
  char Str[32];
  static BYTE ErrMemOlt = 0; // указатель на курсор
//  static BYTE NeedKeyB = 0; // необходимость переключения в клавиатуру
  
  if (GetCellMem(0) <MaxMemPM)
  {
    if ((PRESS(BTN_UP))&&(getStateButtons(BTN_UP)==SHORT_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      FrSaveOLT = ChangeFrSet (FrSaveOLT, 4, 1, MINUS);// установка курсора в рамках заданных параметров
      //ClrKey (BTN_UP);
    }
    if ((PRESS(BTN_DOWN))&&(getStateButtons(BTN_DOWN)==SHORT_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      FrSaveOLT = ChangeFrSet (FrSaveOLT, 4, 1, PLUS);// установка курсора в рамках заданных параметров
      //ClrKey (BTN_DOWN);
    }
    switch (FrSaveOLT) // обработка выбраных полей установок
    {
    case 1: //вызов редактора комментария
      if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
      {
        myBeep(10);
        g_NeedScr = 1; // Need reDraw Screen
        SetMode(ModeKeyBoardOLT);
        ModeDevice = MODEOTHER;
        NeedKeyB = 1; // необходимость переключения в клавиатуру
      }
      break;
    case 2: //изменение счетчика волокон
      if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))
      {
        myBeep(10);
        g_NeedScr = 1; // Need reDraw Screen
        if (PONI.NumFix<9999)PONI.NumFix++;
        else PONI.NumFix = 0;
        //ClrKey (BTN_RIGHT);
      }
      if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))
      {
        myBeep(10);
        g_NeedScr = 1; // Need reDraw Screen
        if (PONI.NumFix>0)PONI.NumFix--;
        else PONI.NumFix = 9999;
        //ClrKey (BTN_LEFT);
      }
      if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==INF_PRESSED))
      {
        myBeep(10);
        g_NeedScr = 1; // Need reDraw Screen
        if (PONI.NumFix<9999)PONI.NumFix++;
        else PONI.NumFix = 0;
        //ClrKey (BTN_RIGHT);
      }
      if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==INF_PRESSED))
      {
        myBeep(10);
        g_NeedScr = 1; // Need reDraw Screen
        if (PONI.NumFix>0)PONI.NumFix--;
        else PONI.NumFix = 9999;
        //ClrKey (BTN_LEFT);
      }
      break;
    case 3: //перемена настройки автоинкремента
      if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))
      {
        myBeep(10);
        g_NeedScr = 1; // Need reDraw Screen
        GetEnIncFiber(1);
        //ClrKey (BTN_RIGHT);
      }
      if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))
      {
        myBeep(10);
        g_NeedScr = 1; // Need reDraw Screen
        GetEnIncFiber(1);
        //ClrKey (BTN_LEFT);
      }
      break;
    case 4: //сброс счетчика 
      if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
      {
        myBeep(10);
        g_NeedScr = 1; // Need reDraw Screen
        PONI.NumFix = 0;
        //ClrKey (BTN_OK);
      }
      break;
    }
  }
  else
  {
    if(!ErrMemOlt)
    {
      ErrMemOlt = 1;
      // перескочим в новое окно об ошибке и заполним тут ЖЕ!
      // посылка команды переключения окна на MainMenu (возврат)  
      CmdInitPage(25);
      if (g_FirstScr)
      {
        
        sprintf(Str, "t0.txt=\"%s\"яяя", MsgMass[36][CurrLang]);
        NEX_Transmit((void*)Str);    // В Н И М А Н И Е
        sprintf(Str, "t1.txt=\"%s\"яяя", MsgMass[59][CurrLang]);
        NEX_Transmit((void*)Str);    // " Запись невозможна"
        sprintf(Str, "t2.txt=\"%s\"яяя", MsgMass[60][CurrLang]);
        NEX_Transmit((void*)Str);    // "нет свободной памяти"
        sprintf(Str, "t3.txt=\"%s\"яяя", MsgMass[38][CurrLang]);
        NEX_Transmit((void*)Str);    // "  для продолжения  "
        sprintf(Str, "t4.txt=\"%s < S >\"яяя", MsgMass[39][CurrLang]);
        NEX_Transmit((void*)Str);    // "    нажмите \"S\"   "
        g_FirstScr = 0;
      }
      
    }
  }
  
  //ClrKey (BNS_MASK);
  if (g_FirstScr)
  {
    // здесь заполняем данными поля нового индикатора
    // не требущие изменения при первичной инициализации
    
    sprintf(Str, "t0.txt=\"%s\"яяя",PONI.CommUserPM);
    NEX_Transmit((void*)Str);    // 1 строка комментарии
    
    sprintf(Str, "t1.txt=\"%04d\"яяя", PONI.NumFix);
    NEX_Transmit((void*)Str);    // номер записи
    
    sprintf(Str, "t2.txt=\"%s\"яяя", MsgMass[54][CurrLang]);
    NEX_Transmit((void*)Str);    // комментариим
    
    sprintf(Str, "t3.txt=\"%s\"яяя", MsgMass[61][CurrLang]);
    NEX_Transmit((void*)Str);    // номер волокна
    
    sprintf(Str, "t4.txt=\"%s\"яяя", MsgMass[55][CurrLang]);
    NEX_Transmit((void*)Str);    // тип счетчика волокона
    
    sprintf(Str, "t5.txt=\"%s\"яяя", MsgMass[58][CurrLang]);
    NEX_Transmit((void*)Str);    // Сброс счетчика
    
    sprintf(Str, "t8.txt=\"№ %04d\"яяя", GetCellMem(0));
    NEX_Transmit((void*)Str);    // Сброс счетчика
    
    g_FirstScr = 0;
    g_NeedScr = 1;
  }
  if (g_NeedScr)
  {
    // здесь заполняем данными поля нового индикатора
    // по результатам изменений вызваныйх обработчиком клавиатуры
    
    // раскрашивание поля выбора 
    // закрасим бэкграунды  и установим требуемый
    sprintf(Str, "t2.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str); //
    sprintf(Str, "t3.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);// 
    sprintf(Str, "t4.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);//
    sprintf(Str, "t5.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);//
    sprintf(Str, "t%d.bco=GREENяяя", FrSaveOLT+1); // зеленый
    NEX_Transmit((void*)Str);// 
    
    sprintf(Str, "t1.txt=\"%04d\"яяя", PONI.NumFix);
    NEX_Transmit((void*)Str);    // номер записи
    
    sprintf(Str, "t6.txt=\"%04d\"яяя",PONI.NumFix);
    NEX_Transmit((void*)Str);    ////счетчик волокна
    
    if (!GetEnIncFiber(0))  sprintf(Str,"t7.txt=\"%s\"яяя", MsgMass[56][CurrLang]);//ручной
    else   sprintf(Str,"t7.txt=\"%s\"яяя", MsgMass[57][CurrLang]);//авто
    NEX_Transmit((void*)Str);    //
    
    // код подсветки требуемой строки если есть есть маркер строки
    
    
    g_NeedScr = 0;
  }
  
  if ((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1; // Need reDraw Screen
    ReturnToTester (0); // возврат в измеритель
  }
  if (rawPressKeyS) // key S
  {        
    myBeep(10);
    g_NeedScr = 1; // Need reDraw Screen
    memcpy (PONI.CommUserPM, CommentsOLT,16);
    WriteMemPow(); // от туда же возвращаемся в измеритель
    rawPressKeyS=0;
    ErrMemOlt=0;
  }
  if(NeedKeyB ) // необходимость переключения в клавиатуру OLT
  {
    // согласно выбранного языка вызывем клавиатуру
    if (CurrLang) 
      // посылка команды переключения окна на EnglishOLT Keyboard 
      CmdInitPage(20); //(22)
    else
      // посылка команды переключения окна на RussianOLT Keyboard 
      CmdInitPage(30); //(23)
    NeedKeyB=0; 
  }
}

void ModeKeyBoardOLT(void) // режим отображения клавиатуры редактора комментариев измерителя
{
  char Str[32];
  char StrI[32];
  //char StrOld[22]; // старая строка комментария
  //BYTE CurrLang=GetLang(CURRENT);
  //DWORD KeyP = SetBtnStates( KEYS_REG, 1 );
  //static BYTE Shift = 0; // регистр 
//  static BYTE NeedReturn = 0; // необходимость вернуться в окно сохранения
  if (g_FirstScr)
  {
    // здесь заполняем данными поля нового индикатора
    // не требущие изменения при первичной инициализации
    sprintf(Str,"%02d/%02d/%02d %02d:%02d:%02d",TimeSaveOLT.RTC_Year%100,
            TimeSaveOLT.RTC_Mon,
            TimeSaveOLT.RTC_Mday,
            TimeSaveOLT.RTC_Hour,
            TimeSaveOLT.RTC_Min,
            TimeSaveOLT.RTC_Sec );
    
    sprintf(StrI, "t0.txt=\"%s\"яяя", Str);
    NEX_Transmit((void*)StrI);    // Date/Time записи
    // надо обрезать до последнего пробела....
    memcpy(StrI,CommentsOLT,IndexCommOLT+1); 
    StrI[IndexCommOLT+1]=0;// No more 16 byte size
    sprintf(Str, "t1.txt=\"%s\"яяя",StrI); 
    NEX_Transmit((void*)Str);    // 1 строка комментарии
    g_GetStr=1; // взведем признак  необходимости прочитать строчку! из индикатора.
    g_FirstScr = 0;
    g_NeedScr = 1;
  }
  if (g_NeedScr)
  {
    g_NeedScr=0;
  }
  if ((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))
  {
    SetMode(ModeSaveOLT);
    ModeDevice = MODEOTHER;
    NeedReturn = 1;
    myBeep(10);
  }
  if(g_GetStr==2) // УРА! что то приняли назад, можно переписать и сбросить признак
  {
    // здесь обработаем строку на приеме
    IndexCommOLT = GetStringNEX(CommentsOLT, ARRAY_SIZE(CommentsOLT));
    //memcpy(CommentsOLT,&RX_BufNEX[1],(CntRXNEX<18)?(CntRXNEX-2):(15)); 
    //CommentsOLT[15]=0;
    for(int i=IndexCommOLT; i<ARRAY_SIZE(CommentsOLT); ++i) CommentsOLT[i]=' ';
    CommentsOLT[ARRAY_SIZE(CommentsOLT)-1]=0;
        memcpy (PONI.CommUserPM, CommentsOLT,16);
        SetMode(ModeSaveOLT);
        
    NeedReturn=1;
  }
  if(NeedReturn)
  {
    // посылка команды переключения окна на SaveOLT (возврат) 
    // надо переписать то что направили в редакторе !!!
    // важно тут 16 байт и последние пробелы
    g_GetStr=0; // сбросим признак  необходимости прочитать строчку! из индикатора.
    CmdInitPage(24);
    
    NeedReturn = 0;
  }
  
  
  
}

// обработка клавиатуры при редакторе префикса имени файла
void ModeKeyBoardPrefix(void) // режим отображения клавиатуры редактора prefixNameFile
{
  char Str[32];
  char StrI[32];
  //static BYTE Shift = 0; // регистр 
  // тут для нового индикатора
  if (g_FirstScr)
  {
    // здесь заполняем данными поля нового индикатора
    // надо обрезать до последнего пробела....
    memcpy(StrI,PrefixFileNm,IndexPrefix+1); 
    StrI[IndexPrefix+1]=0;// No more 11 byte size
    sprintf(Str, "t1.txt=\"%s\"яяя",StrI); 
    NEX_Transmit((void*)Str);    // 1 строка комментарии
    g_GetStr=1; // взведем признак  необходимости прочитать строчку! из индикатора.
    g_FirstScr = 0;
    g_NeedScr = 1;
  }
  if (g_NeedScr)
  {
    g_NeedScr=0;
  }
  
  if(g_GetStr==2) // УРА! что то приняли назад, можно переписать и сбросить признак
  {
    // здесь обработаем строку на приеме
    IndexPrefix = GetStringNEX(PrefixFileNm, ARRAY_SIZE(PrefixFileNm));
    //memcpy(CommentsOLT,&RX_BufNEX[1],(CntRXNEX<18)?(CntRXNEX-2):(15)); 
    //CommentsOLT[15]=0;
    for(int i=IndexPrefix; i< ARRAY_SIZE(PrefixFileNm); ++i) PrefixFileNm[i]=' ';
    PrefixFileNm[ARRAY_SIZE(PrefixFileNm)-1]=0;
    NeedReturn=1;
  }
  
  if (((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))||(NeedReturn))
  {
    myBeep(10);
    SetMode(ModeSetupOTDR);  //  переход в режим установок рефлектометра (было)
    ModeDevice = MODESETREFL;
    SubModeMeasOTDR = NOTMODE;
    // вернемся в окно
    // важно тут 11 байт и последние пробелы
    g_GetStr=0; // сбросим признак  необходимости прочитать строчку! из индикатора.
    // вернемся в окно устанвок рефлектометра
    CmdInitPage(2);
    NeedReturn = 0;
  }
  
  
  
}


void ModeSelectMEM(void) // режим выбора работы с памятью CHECK_OFF
{
  static volatile BYTE FrSelectMEM = 1; // указатель на курсор
  char Str[32];
  BYTE PowerMeter=0;
  if (GetCfgPM()) PowerMeter=1;
  //BYTE CurrLang=GetLang(CURRENT);
  //DWORD KeyP = SetBtnStates( KEYS_REG, 1 );
  if ((PRESS(BTN_UP))&&(getStateButtons(BTN_UP)==SHORT_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1; // Need reDraw Screen
    FrSelectMEM = ChangeFrSet (FrSelectMEM, 2+PowerMeter, 1, MINUS);// установка курсора в рамках заданных параметров
    //ClrKey (BTN_UP);
  }
  if ((PRESS(BTN_DOWN))&&(getStateButtons(BTN_DOWN)==SHORT_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1; // Need reDraw Screen
    FrSelectMEM = ChangeFrSet (FrSelectMEM, 2+PowerMeter, 1, PLUS);// установка курсора в рамках заданных параметров
    //ClrKey (BTN_DOWN);
  }
  
  
  if (g_FirstScr)
  {
    // здесь заполняем данными поля нового индикатора
    // не требущие изменения при первичной инициализации
    // Здесь практичски все поля 
    sprintf(Str, "t0.txt=\"%s\"яяя", MsgMass[42][CurrLang]);
    NEX_Transmit((void*)Str);    // Память
    
    sprintf(Str, "t1.txt=\"%s\"яяя", MsgMass[43][CurrLang]);
    NEX_Transmit((void*)Str);    // свободно
    
    sprintf(Str, "t2.txt=\"%s\"яяя", MsgMass[7][CurrLang]);
    NEX_Transmit((void*)Str);    // РЕФЛЕКТОМЕТР
    
    sprintf(Str, "t3.txt=\"%4d\"яяя", MAXMEMALL-GetNumTraceSaved(0));
    NEX_Transmit((void*)Str);    // сколько свободно
    
    if (PowerMeter) // есть измеритель
    {
      
      sprintf(Str, "t4.txt=\"%s\"яяя", MsgMass[74][CurrLang]);
      NEX_Transmit((void*)Str);    // ИЗМЕРИТЕЛЬ
      
      sprintf(Str, "t5.txt=\"%4d\"яяя",MaxMemPM-GetCellMem(0));
      NEX_Transmit((void*)Str);    // ИЗМЕРИТЕЛЬ (число ячеек)
      
      sprintf(Str, "t6.txt=\"%s\"яяя", MsgMass[44][CurrLang]);
      NEX_Transmit((void*)Str);    // ОЧИСТКА
    }
    else  // измерителя нет
    {
      sprintf(Str, "t4.txt=\"%s\"яяя", MsgMass[44][CurrLang]);
      NEX_Transmit((void*)Str);    // ОЧИСТКА
      sprintf(Str, "t5.txt=\"\"яяя");
      NEX_Transmit((void*)Str);    // пустые
      
      sprintf(Str, "t6.txt=\"\"яяя");
      NEX_Transmit((void*)Str);    // пустые
      
    }
    
    g_FirstScr = 0;
    g_NeedScr = 1;
  }
  if (g_NeedScr)
  {
    // здесь заполняем данными поля нового индикатора
    // по результатам изменений вызваныйх обработчиком клавиатуры
    
    // раскрашивание поля выбора 
    // закрасим бэкграунды  и установим требуемый
    sprintf(Str, "t2.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);//
    sprintf(Str, "t4.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);// 
    sprintf(Str, "t6.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);//
    sprintf(Str, "t%d.bco=GREENяяя", FrSelectMEM<<1); // зеленый
    NEX_Transmit((void*)Str);// 
    // код подсветки требуемой строки если есть есть маркер строки
    g_NeedScr = 0;
  }
  
  if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
  {
    switch (FrSelectMEM) // выбор по кнопке "ОК"
    {
    case 1: // переход в память рефлектограмм
      myBeep(10);
      
      //SetMode(ModeMemoryOTDR);
      SetMode(ModeFileMngDir);
      ModeDevice = MODEMEMR;
      ModeMemDraw = VIEWNEXT;
      ReturnMemView = 1; // надо вернуться сюда же по ESC
      // посылка команды переключения окна на Mem_OTDR_garaph (вызов)  
      ClrKey(BTN_OK);
      CmdInitPage(33); // новое окно лист бокс перечня директорий
      //CreatDelay(1000000);
      HAL_Delay(100);
      
      
      break;
    case 2: // переход в  памяти измерителя
      if (PowerMeter) // есть измеритель
      {
        // go to wiev memPM
        if (GetCellMem(0))
        {
          //123!!!       ReadCellIzm(GetCellMem(0)-1,(unsigned char*)&PONI);//  читаем из памяти(flash) в PONI ячейку сохранения измерителя
          
          Sec2Date (PONI.TotalTimeCell, &TimeSaveOLT);
          myBeep(10);
          NumCellIzm = GetCellMem(0)-1;
          SetMode(ModeViewMemOLT);
          SetModeDevice(MODETESTMEM);
          // посылка команды переключения окна на Mem_OLT_view (вызов)  
          CmdInitPage(20);
        }
      }
      else
      {
        myBeep(10);
        SetMode(ModeClearMEM);
        FrClearMEM = 2 + PowerMeter;
        // посылка команды переключения окна на Select_MEM_Clr(вызов)  
        CmdInitPage(21);
        //NeedReturn = 4; // что бы вернутся сюда же
      }
      break;
    case 3: // переход в режим выбора стирания памяти
      myBeep(10);
      SetMode(ModeClearMEM);
      FrClearMEM = 2 + PowerMeter;
      // посылка команды переключения окна на Select_MEM_Clr(вызов)  
      CmdInitPage(21);
      //NeedReturn = 4; // что бы вернутся сюда же
      break;
    }
    
  }
  if ((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))
  {
    myBeep(10);
    SetMode(ModeMainMenu);
    ModeDevice = MODEMENU;
    // посылка команды переключения окна на MainMenu (возврат)  
    CmdInitPage(1);
    
  }
}

void ModeClearMEM(void) // режим освобождения памяти измерителя CHECK_OFF
{
  char Str[32];
  //(InputOK)?("OK"):("???")
    BYTE PowerMeter=((GetCfgPM())?(1):(0));
  //static BYTE FrClearMEM = 1; // указатель на курсор
  if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
  {
    switch (FrClearMEM)
    {
    case 1: // refl
      DeletingAllTrace (); // "Удаление"всех рефлектограммы, просто обнуляем счетчик сохранения
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      NeedReturn = 4; // индекс окна возврата
      //SetMode(ModeSelectMEM);
      break;
    case 2:
      if (PowerMeter)
      {
        DeletedAllCell (); // удаление всех записей измерителя
        myBeep(10);
        g_NeedScr = 1; // Need reDraw Screen
        NeedReturn = 4; // индекс окна возврата
        //SetMode(ModeSelectMEM);
      }
      else
      {
        myBeep(10);
        g_NeedScr = 1; // Need reDraw Screen
        NeedReturn = 4; // индекс окна возврата
       // SetMode(ModeSelectMEM);
      }
      break;
    case 3:
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      NeedReturn = 4; // индекс окна возврата
      //SetMode(ModeSelectMEM);
      break;
    }
  }
  if ((PRESS(BTN_UP))&&(getStateButtons(BTN_UP)==SHORT_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1; // Need reDraw Screen
    FrClearMEM = ChangeFrSet (FrClearMEM, 2+PowerMeter, 1, MINUS);// установка курсора в рамках заданных параметров
    //ClrKey (BTN_UP);
  }
  if ((PRESS(BTN_DOWN))&&(getStateButtons(BTN_DOWN)==SHORT_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1; // Need reDraw Screen
    FrClearMEM = ChangeFrSet (FrClearMEM, 2+PowerMeter, 1, PLUS);// установка курсора в рамках заданных параметров
    //ClrKey (BTN_DOWN);
  }

  if (g_FirstScr)
  {
    // здесь заполняем данными поля нового индикатора
    // не требущие изменения при первичной инициализации     
    sprintf(Str, "t0.txt=\"%s\"яяя", MsgMass[45][CurrLang]); 
    NEX_Transmit((void*)Str);    // Очистка памяти
                                                             
    sprintf(Str, "t1.txt=\"%s\"яяя", MsgMass[7][CurrLang]);
    NEX_Transmit((void*)Str);    // РЕФЛЕКТОМЕТР 

          if (PowerMeter)
          {
    sprintf(Str, "t2.txt=\"%s\"яяя", MsgMass[74][CurrLang]);
    NEX_Transmit((void*)Str);    // ИЗМЕРИТЕЛЬ     

    sprintf(Str, "t3.txt=\"%s\"яяя", MsgMass[46][CurrLang]); 
    NEX_Transmit((void*)Str);    // ОТМЕНА
          }
          else
          {
    sprintf(Str, "t2.txt=\"%s\"яяя", MsgMass[46][CurrLang]); 
    NEX_Transmit((void*)Str);    // ОТМЕНА
    sprintf(Str, "t3.txt=\"\"яяя");
    NEX_Transmit((void*)Str);    // Empty string    

          }
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
    sprintf(Str, "t2.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);// 
    sprintf(Str, "t3.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);//

    sprintf(Str, "t%d.bco=GREENяяя", FrClearMEM); // зеленый
    NEX_Transmit((void*)Str);// 
                                       // код подсветки требуемой строки если есть есть маркер строки
    g_NeedScr = 0;
  }

  if (((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))||(NeedReturn))
  {
    myBeep(10);
    g_NeedScr = 1; // Need reDraw Screen
    SetMode(ModeSelectMEM);
    if(!NeedReturn) NeedReturn = 4;
            // посылка команды переключения окна на MainMenu (возврат)  
    CmdInitPage(NeedReturn);
    NeedReturn = 0;
    
    //ModeDevice = MODEMENU;
  }
}

void ModeViewMemOLT(void) // режим просмотра памяти измерителя CHECK_OFF
{
  char Str[32];
  char StrO[64];
  if ((PRESS(BTN_LEFT))&&((getStateButtons(BTN_LEFT)==SHORT_PRESSED)||(getStateButtons(BTN_LEFT)==INF_PRESSED)))
      {
        if (NumCellIzm > 0) NumCellIzm--;
        else NumCellIzm = GetCellMem(0)-1;
//123!!!       ReadCellIzm(NumCellIzm,(unsigned char*)&PONI);//  читаем из памяти(flash) в PONI ячейку сохранения измерителя
       Sec2Date (PONI.TotalTimeCell, &TimeSaveOLT);
       g_NeedScr = 1; // для вызова заполнения значений
       myBeep(10);
        
      }
  if ((PRESS(BTN_RIGHT))&&((getStateButtons(BTN_RIGHT)==SHORT_PRESSED)||(getStateButtons(BTN_RIGHT)==INF_PRESSED)))
      {
        if (NumCellIzm < GetCellMem(0)-1) NumCellIzm++;
        else NumCellIzm =0;
//123!!!       ReadCellIzm(NumCellIzm,(unsigned char*)&PONI);//  читаем из памяти(flash) в PONI ячейку сохранения измерителя
       Sec2Date (PONI.TotalTimeCell, &TimeSaveOLT);
       g_NeedScr = 1; // для вызова заполнения значений
       myBeep(10);
      }
  
  if (g_FirstScr)
  {
    // здесь заполняем данными поля нового индикатора
    // не требущие изменения при первичной инициализации   
    sprintf(Str, "t0.txt=\"%s\"яяя", MsgMass[0][CurrLang]); // 
    NEX_Transmit((void*)Str);// 
    sprintf(Str, "t1.txt=\"%s\"яяя", MsgMass[0][CurrLang]); // 
    NEX_Transmit((void*)Str);// 
    sprintf(Str, "t2.txt=\"%s\"яяя", MsgMass[17][CurrLang]); // Длина волны
    NEX_Transmit((void*)Str);// 
    sprintf(Str, "t3.txt=\"%s\"яяя", MsgMass[0][CurrLang]); //  
    NEX_Transmit((void*)Str);// 
    sprintf(Str, "t4.txt=\"%s\"яяя", MsgMass[0][CurrLang]); //
    NEX_Transmit((void*)Str);// 
    sprintf(Str, "t5.txt=\"%s\"яяя", MsgMass[52][CurrLang]); // Потери  
    NEX_Transmit((void*)Str);// 
    sprintf(Str, "t6.txt=\"%s\"яяя", MsgMass[24][CurrLang]); // нм
    NEX_Transmit((void*)Str);// 
    sprintf(Str, "t7.txt=\"%s\"яяя", MsgMass[0][CurrLang]); //
    NEX_Transmit((void*)Str);// 
    sprintf(Str, "t8.txt=\"%s\"яяя", MsgMass[0][CurrLang]); //
    NEX_Transmit((void*)Str);// 
    sprintf(Str, "t9.txt=\"%s\"яяя", MsgMass[47][CurrLang]); // дБ
    NEX_Transmit((void*)Str);// 


    g_NeedScr = 1; // для вызова заполнения значений
    g_FirstScr = 0;
  }
  if (g_NeedScr)
  {
    // здесь заполняем данными поля нового индикатора
    // по результатам изменений вызваныйх обработчиком клавиатуры
  // первая сторока дата и время файла и его номер в памяти
  sprintf(Str,"%02d.%02d.%02d_%02d:%02d:%02d (%04d)",TimeSaveOLT.RTC_Year%100,
          TimeSaveOLT.RTC_Mon,
          TimeSaveOLT.RTC_Mday,
          TimeSaveOLT.RTC_Hour,
          TimeSaveOLT.RTC_Min,
          TimeSaveOLT.RTC_Sec,
          NumCellIzm+1); // поправлено 23.04.2013 - номер в памяти и порядковый номер сохранения 
    sprintf(StrO, "t0.txt=\"%s\"яяя", Str); // 
   NEX_Transmit((void*)StrO);// 
  sprintf(Str,"%s",PONI.CommUserPM);
    sprintf(StrO, "t1.txt=\"%s\"яяя", Str); // 
   NEX_Transmit((void*)StrO);// 
  sprintf(Str,"%04d",PONI.NumFix);
    sprintf(StrO, "t10.txt=\"%s\"яяя", Str); // 
   NEX_Transmit((void*)StrO);// 
  switch (PONI.Rez) // далее рисуем в зависимости от того какая каритнка
  {
  case MANUAL:
    sprintf(Str,"%s", MsgMass[17][CurrLang]);//Длина волны
    sprintf(StrO, "t2.txt=\"%s\"яяя", Str); // 
   NEX_Transmit((void*)StrO);// 
    sprintf(Str,"%d%s",PONI.LenWaveMeas,MsgMass[18][CurrLang]);//нм
    sprintf(StrO, "t6.txt=\"%s\"яяя", Str); // 
   NEX_Transmit((void*)StrO);// 
    sprintf(Str,"%s", MsgMass[51][CurrLang]);//Уровень, 
    sprintf(StrO, "t3.txt=\"%s\"яяя", Str); // 
   NEX_Transmit((void*)StrO);// 
    DrawLevelStr(Str); //???? вывод признака в чем выводим W/ dB/ dBm
    sprintf(StrO, "t7.txt=\"%s\"яяя", Str); // 
   NEX_Transmit((void*)StrO);// 
    sprintf(Str,"REF,     %s", MsgMass[48][CurrLang]);//дБм
    sprintf(StrO, "t4.txt=\"%s\"яяя", Str); // 
   NEX_Transmit((void*)StrO);// 
    sprintf(Str,"%.2f",PONI.BaseLvl[0]);
    sprintf(StrO, "t8.txt=\"%s\"яяя", Str); // 
   NEX_Transmit((void*)StrO);// 
    sprintf(Str,"%s,  %s", MsgMass[52][CurrLang], MsgMass[47][CurrLang]);//Потери,  дБ
    sprintf(StrO, "t5.txt=\"%s\"яяя", Str); // 
   NEX_Transmit((void*)StrO);// 
    sprintf(Str,"%.2f",PONI.PowLevel[1]);
    sprintf(StrO, "t9.txt=\"%s\"яяя", Str); // 
   NEX_Transmit((void*)StrO);// 
    
    break;
  case AUTO:
    sprintf(Str,"%s", MsgMass[53][CurrLang]);//Дл.волны    
    sprintf(StrO, "t2.txt=\"%s\"яяя", Str); // 
   NEX_Transmit((void*)StrO);// 
    sprintf(Str,"%s", MsgMass[52][CurrLang]);//  Потери
    sprintf(StrO, "t6.txt=\"%s\"яяя", Str); // 
   NEX_Transmit((void*)StrO);// 
        for (int i=0; i<3; i++)
        {
          if (PONI.PowLevel[i]>-100.0)
          {
    sprintf(Str,"%04d %s  ",PONI.LenWaveKlb[i], MsgMass[18][CurrLang]);
    sprintf(StrO, "t%01d.txt=\"%s\"яяя",3+i, Str); // 
   NEX_Transmit((void*)StrO);// 
    sprintf(Str,"%.2f %s",PONI.PowLevel[i], MsgMass[47][CurrLang]);
    sprintf(StrO, "t%01d.txt=\"%s\"яяя",7+i, Str); // 
   NEX_Transmit((void*)StrO);// 
          }
    else 
    {
    sprintf(Str,"-------");
    sprintf(StrO, "t%01d.txt=\"%s\"яяя",3+i, Str); // 
   NEX_Transmit((void*)StrO);// 
    sprintf(Str,"--------");
    sprintf(StrO, "t%01d.txt=\"%s\"яяя",7+i, Str); // 
   NEX_Transmit((void*)StrO);// 
    }      

        }
    break;
  }

    // раскрашивание поля выбора 

    g_NeedScr = 0;
  }


    if ((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))
  {
    myBeep(10);
    SetMode(ModeSelectMEM);
    ModeDevice = MODEOTHER;
    // посылка команды переключения окна на Memory (возврат)  
      CmdInitPage(4);
  }
  //ClrKey (BNS_MASK);

}

void ModeSetting(void)// режим установок прибора CHECK_IN
{
  static BYTE FrSetting = 0; // указатель на курсор
  char Str[32];
  char SetNewWinIfOut = 0; // устнанавливаем признак перхода в другое окно если надо выйти
  //BYTE CurrLang=GetLang(CURRENT);
  //DWORD KeyP = SetBtnStates( KEYS_REG, 1 );
  if ((PRESS(BTN_UP))&&(getStateButtons(BTN_UP)==SHORT_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1; // Need reDraw Screen
    FrSetting = ChangeFrSet (FrSetting, (ENAOLDLCD)?(4):(3), 0, MINUS);// установка курсора в рамках заданных параметров
    //ClrKey (BTN_UP);
  }
  if ((PRESS(BTN_DOWN))&&(getStateButtons(BTN_DOWN)==SHORT_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1; // Need reDraw Screen
    FrSetting = ChangeFrSet (FrSetting, (ENAOLDLCD)?(4):(3), 0, PLUS);// установка курсора в рамках заданных параметров
    //ClrKey (BTN_DOWN);
  }
  switch (FrSetting) // сделаем перестановку полей
  {
  case 0: // Data_Time_Set
  if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1; // Need reDraw Screen
    PosCurr = 6;
    SetMode(ModeDateTimeSET);
    NeedDrawCRC = 1;
    ModeDevice = MODEOTHER;
 
        // посылка команды переключения окна на Set_datetime (вызов)  
    // вызовем позже!
       SetNewWinIfOut = 9; // устнанавливаем признак перхода в другое окно если надо выйти
  //CmdInitPage(9);
    //ClrKey (BTN_OK);
  }
  break;
  case 4: // BlackLight (1)
  if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1; // Need reDraw Screen
    //ClrKey (BTN_RIGHT);
  }
  if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1; // Need reDraw Screen
    //ClrKey (BTN_LEFT);
  }
  break;// BlackLight
  case 1: // Language (2)
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
  case 2: //рефлектометр (3)
  if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
  {
    myBeep(10);
    SetMode(ModeSetOTDRprm);
    NeedDrawCRC = 1;
    ModeDevice = MODEOTDRPRM;
        // посылка команды переключения окна на Set_OTDRparams (вызов)  
       SetNewWinIfOut = 10; // устнанавливаем признак перхода в другое окно если надо выйти
    //CmdInitPage(10);
    //  StartSettingBegShift (); // старт измерения мертвых зон
        //ClrKey (BTN_OK);
  }
  break;// рефлектометр
  case 3: //Contrast (4)
  if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1; // Need reDraw Screen
//xxx    ChangeUserContr (1); // изменеие пользовательской контрастности
    //ClrKey (BTN_RIGHT);
  }
  if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1; // Need reDraw Screen
//xxx    ChangeUserContr (-1); // изменеие пользовательской контрастности
    //ClrKey (BTN_LEFT);
  }
  if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==INF_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1; // Need reDraw Screen
//xxx    ChangeUserContr (1); // изменеие пользовательской контрастности
    //ClrKey (BTN_RIGHT);
  }
  if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==INF_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1; // Need reDraw Screen
//xxx    ChangeUserContr (-1); // изменеие пользовательской контрастности
    //ClrKey (BTN_LEFT);
  }
//  if (((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==LONG_PRESSED))&&(ChangeUserContr (0)==66))
//  {
//    myBeep(10);
//    SetMode(TetrisGame);
//    InitTetris();
//  }
//  if (((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==LONG_PRESSED))&&(ChangeUserContr (0)==67))
//  {
//    myBeep(10);
//    SetMode(ArcanoidGame);
//    InitArcanoid();
//  }
//  if (((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==LONG_PRESSED))&&(ChangeUserContr (0)==68))
//  {
//    myBeep(10);
//    SetMode(KeyTestGame);
//    
//  }
  break;// Contrast
  }

  
  if (g_FirstScr)
  {
    // здесь заполняем данными поля нового индикатора
    // не требущие изменения при первичной инициализации
    sprintf(Str, "t0.txt=\"%s\"яяя", MsgMass[1][CurrLang]);
    NEX_Transmit((void*)Str);    // Дата / Время
  
  
    sprintf(Str, "t1.txt=\"Language\"яяя"); //!
    NEX_Transmit((void*)Str);    // Язык
  
    sprintf(Str, "t2.txt=\"%s\"яяя", MsgMass[100][CurrLang]); //!
    NEX_Transmit((void*)Str);    // OTDR Params
  
    sprintf(Str, "t3.txt=\"%s\"яяя", MsgMass[121][CurrLang]); //!
    NEX_Transmit((void*)Str);    // Яркость
  
    g_FirstScr = 0;
    g_NeedScr = 1;
  }
  if (g_NeedScr)
  {
    // здесь заполняем данными поля нового индикатора
    // по результатам изменений вызваныйх обработчиком клавиатуры
    sprintf(Str, "t6.txt=\"%s\"яяя", MsgMass[0][CurrLang]);
    NEX_Transmit((void*)Str);    // English
  
    sprintf(Str, "t7.txt=\"%d\"яяя", 55);
    NEX_Transmit((void*)Str);    // ???
  
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
    sprintf(Str, "t4.bco=WHITEяяя"); // белый
    NEX_Transmit((void*)Str);//
    sprintf(Str, "t%d.bco=GREENяяя", FrSetting); // зеленый
    NEX_Transmit((void*)Str);// 
  
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
    SetMode(ModeMainMenu);
    ModeDevice = MODEMENU;
        // посылка команды переключения окна на MainMenu (возврат)  
    CmdInitPage(1);
  }
  
}

void ModeSetOTDRprm(void)// режим установок параметров рефлектометра прибора CHECK_IN
{
  static BYTE FrSetOTDRprm = 0; // указатель на курсор
  char Str[32];
  int IfJumpNewWin = 0;
  //BYTE CurrLang=GetLang(CURRENT);
  //DWORD KeyP = SetBtnStates( KEYS_REG, 1 );
  if ((PRESS(BTN_UP))&&(getStateButtons(BTN_UP)==SHORT_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1; // Need reDraw Screen
    FrSetOTDRprm = ChangeFrSet (FrSetOTDRprm, 2, 0, MINUS);// установка курсора в рамках заданных параметров
    //ClrKey (BTN_UP);
  }
  if ((PRESS(BTN_DOWN))&&(getStateButtons(BTN_DOWN)==SHORT_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1; // Need reDraw Screen
    FrSetOTDRprm = ChangeFrSet (FrSetOTDRprm, 2, 0, PLUS);// установка курсора в рамках заданных параметров
    //ClrKey (BTN_DOWN);
  }
  switch (FrSetOTDRprm)
  {
  case 0: // установка разрешения отображениея и счета событий
  if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1; // Need reDraw Screen
    GetSetEnaEvents(1);
    //ClrKey (BTN_RIGHT);
  }
  if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1; // Need reDraw Screen
    GetSetEnaEvents(1);
    //ClrKey (BTN_LEFT);
  }
  break;// Events
  case 1: // установка порогов белкора
  if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
  {
    myBeep(10);
    SetMode(SetBELCORE);
    ModeDevice = MODEOTHER;
        // посылка команды переключения окна на Set_OTDR_event(вызов)  
    IfJumpNewWin = 11;
    //CmdInitPage(11);
    //ClrKey (BTN_OK);
  }
  break;
  case 2: //Калибровка
  if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
  {
        // посылка команды переключения окна на Calibrate (вызов)  
    StartSettingBegShift (); // старт измерения мертвых зон
        //ClrKey (BTN_OK);
        // посылка команды переключения окна на Set_OTDRparams (возврат)  
    IfJumpNewWin = 14;
    //CmdInitPage(10);
  }
  break;// калибровка
//  case 3:
//  if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))
//  {
//    myBeep(10);
//    if (FltrMod<NumFiltrs) FltrMod++;
//    else FltrMod=0;
//    //ClrKey (BTN_RIGHT);
//  }
//  if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))
//  {
//    myBeep(10);
//    if (FltrMod>0) FltrMod--;
//    else FltrMod=NumFiltrs;
//    //ClrKey (BTN_LEFT);
//  }
//    
//    break; // filtr
  }

  if (g_FirstScr)
  {
	  // здесь заполняем данными поля нового индикатора
	  // не требущие изменения при первичной инициализации
	  sprintf(Str, "t0.txt=\"%s\"яяя", MsgMass[111][CurrLang]);
	  NEX_Transmit((void*)Str);    // запись событий

	  sprintf(Str, "t1.txt=\"%s\"яяя", MsgMass[112][CurrLang]);
	  NEX_Transmit((void*)Str);    // параметры поиска

	  sprintf(Str, "t2.txt=\"%s\"яяя", MsgMass[5][CurrLang]);
	  NEX_Transmit((void*)Str);    // калибровка

	  g_FirstScr = 0;
	  g_NeedScr = 1;
  }
  if (g_NeedScr)
  {
	  // здесь заполняем данными поля нового индикатора
	  // по результатам изменений вызваныйх обработчиком клавиатуры
  if (GetSetEnaEvents(0)) 
	  sprintf(Str, "t3.txt=\"%s\"яяя", MsgMass[113][CurrLang]);
  else
	  sprintf(Str, "t3.txt=\"%s\"яяя", MsgMass[114][CurrLang]);
	  NEX_Transmit((void*)Str);// 

	  // раскрашивание поля выбора 
	  // закрасим бэкграунды  и установим требуемый
	  sprintf(Str, "t0.bco=WHITEяяя"); // белый
	  NEX_Transmit((void*)Str);// 
	  sprintf(Str, "t1.bco=WHITEяяя"); // белый
	  NEX_Transmit((void*)Str);// 
	  sprintf(Str, "t2.bco=WHITEяяя"); // белый
	  NEX_Transmit((void*)Str);// 
	  sprintf(Str, "t%d.bco=GREENяяя", FrSetOTDRprm); // зеленый
	  NEX_Transmit((void*)Str);// 
		// код подсветки требуемой строки если есть есть маркер строки
	  g_NeedScr = 0;
  }

  if(IfJumpNewWin)
  {
       CmdInitPage(IfJumpNewWin);
    IfJumpNewWin =0;
  }
  if ((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))// возврат в установки
  {
    myBeep(10);
    SetMode(ModeSetting);
    ModeDevice = MODESETUP;
        // посылка команды переключения окна на Settings (возврат)  
    CmdInitPage(5);
  }
  //ClrKey (BNS_MASK);
}
// установка порогов анализа в BELCORE CHECK_IN
void SetBELCORE (void)
{
  float OutSign;                         // переменная для вывода параметров
  static volatile BYTE FrSetBelcore = 0; // указатель на курсор
  char Str[25];
  //BYTE CurrLang=GetLang(CURRENT);
  //DWORD KeyP = SetBtnStates( KEYS_REG, 1 );
  if ((PRESS(BTN_UP))&&(getStateButtons(BTN_UP)==SHORT_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1; // Need reDraw Screen
    FrSetBelcore = ChangeFrSet (FrSetBelcore, 3, 0, MINUS);// установка курсора в рамках заданных параметров
    //ClrKey (BTN_UP);
  }
  if ((PRESS(BTN_DOWN))&&(getStateButtons(BTN_DOWN)==SHORT_PRESSED))
  {
    myBeep(10);
    g_NeedScr = 1; // Need reDraw Screen
    FrSetBelcore = ChangeFrSet (FrSetBelcore, 3, 0, PLUS);// установка курсора в рамках заданных параметров
    //ClrKey (BTN_DOWN);
  }
  switch (FrSetBelcore) // обработка выбраных полей установок
  {
  case 0: // коэффициент обратного рассеяния
    if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      GetSet_BC (0.1);
    }
    if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      GetSet_BC (-0.1);
    }
    if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==INF_PRESSED))
    {
      g_NeedScr = 1; // Need reDraw Screen
      GetSet_BC (0.3);
    }
    if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==INF_PRESSED))
    {
      g_NeedScr = 1; // Need reDraw Screen
      GetSet_BC (-0.3);
    }
    //OutSign = GetSet_BC(0.0)/10.0;
    //sprintf(Str,"%2.1f%s",OutSign, MsgMass[47][CurrLang]);//дБ
    break;
    // Кофф.обратного рассеяния
  case 1: // порог контроля затухания в соединении
    if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      GetSet_LT (0.1);
    }
    if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      GetSet_LT (-0.1);
    }
    if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==INF_PRESSED))
    {
      g_NeedScr = 1; // Need reDraw Screen
      GetSet_LT (0.3);
    }
    if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==INF_PRESSED))
    {
      g_NeedScr = 1; // Need reDraw Screen
      GetSet_LT (-0.3);
    }
    //OutSign = GetSet_LT(0.0)/1000.0;
    //sprintf(Str,"%2.2f%s",OutSign, MsgMass[47][CurrLang]);//дБ
    break;
  case 2: // порог контроля отражающего события
    if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      GetSet_RT (1.0);
    }
    if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      GetSet_RT (-1.0);
    }
    if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==INF_PRESSED))
    {
      g_NeedScr = 1; // Need reDraw Screen
      GetSet_RT (3.0);
    }
    if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==INF_PRESSED))
    {
      g_NeedScr = 1; // Need reDraw Screen
      GetSet_RT (-3.0);
    }
    //OutSign = GetSet_RT(0.0)/1000.0;
    //sprintf(Str,"%2.2f%s",-OutSign, MsgMass[47][CurrLang]);//дБ
    break;
  case 3: // порог контроля определения конца линии
    if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      GetSet_ET (0.1);
    }
    if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      GetSet_ET (-0.1);
    }
    if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==INF_PRESSED))
    {
      g_NeedScr = 1; // Need reDraw Screen
      GetSet_ET (0.3);
    }
    if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==INF_PRESSED))
    {
      g_NeedScr = 1; // Need reDraw Screen
      GetSet_ET (-0.3);
    }
    //OutSign = GetSet_ET(0.0)/1000.0;
    //sprintf(Str,"%2.2f%s",OutSign, MsgMass[47][CurrLang]);//дБ
    break;
  }
if (g_FirstScr)
{
  // здесь заполняем данными поля нового индикатора
  // не требущие изменения при первичной инициализации
  sprintf(Str, "t0.txt=\"%s\"яяя", MsgMass[103][CurrLang]);
  NEX_Transmit((void*)Str);                         // (BC) обратное рассеяние
  
  sprintf(Str, "t1.txt=\"%s\"яяя", MsgMass[105][CurrLang]);
  NEX_Transmit((void*)Str);                         // (LT) потери в стыке
  
  sprintf(Str, "t2.txt=\"%s\"яяя", MsgMass[107][CurrLang]);
  NEX_Transmit((void*)Str);                         // (RT) Коэфф. обратного отражения
  
  sprintf(Str, "t3.txt=\"%s\"яяя", MsgMass[109][CurrLang]);
  NEX_Transmit((void*)Str);                         // (ET) конец линии
  
  
  g_FirstScr = 0;
  g_NeedScr = 1;
}
if (g_NeedScr)
{
  // здесь заполняем данными поля нового индикатора
  // по результатам изменений вызваныйх обработчиком клавиатуры

  OutSign = GetSet_BC(0.0)/10.0;
  sprintf(Str, "t4.txt=\"%2.1f%s\"яяя",OutSign, MsgMass[47][CurrLang]);
  NEX_Transmit((void*)Str);                         // ...
  
  OutSign = GetSet_LT(0.0)/1000.0;
  sprintf(Str, "t5.txt=\"%2.2f%s\"яяя", OutSign, MsgMass[47][CurrLang]);
  NEX_Transmit((void*)Str);                         // ...
  
  OutSign = GetSet_RT(0.0)/1000.0;
  sprintf(Str, "t6.txt=\"%2.1f%s\"яяя",-OutSign, MsgMass[47][CurrLang]);
  NEX_Transmit((void*)Str);                         // ..
  
  OutSign = GetSet_ET(0.0)/1000.0;
  sprintf(Str, "t7.txt=\"%2.2f%s\"яяя",OutSign, MsgMass[47][CurrLang]);
  NEX_Transmit((void*)Str);                         // ..
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
  sprintf(Str, "t%d.bco=GREENяяя", FrSetBelcore); // зеленый
  NEX_Transmit((void*)Str);// 
  
  								   // код подсветки требуемой строки если есть есть маркер строки
  g_NeedScr = 0;
}
  if ((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))// возврат в установки
  {
    myBeep(10);
    CheckReflParam ();  // Проверка пользовательских настроек 
    
    //FlashErasePage(EVEN_SET); // чистим страницу установок пользователя прибора
    //FlashWritePageSM(EVEN_SET, StructPtr(EVEN_SET), StructSize(EVEN_SET), 0);
    WriteNeedStruct(0x08);
    SetMode(ModeSetOTDRprm);
    ModeDevice = MODEOTDRPRM;
        // посылка команды переключения окна на Set_OTDRparams (возврат)  
    CmdInitPage(10);
    //myBeep(10); // убрал, замечание 3
  }
  
}


void ModeDateTimeSET(void) // режим установок времени CHECK_IN
{
  //static BYTE PosCurr = 6; // положение курсора устанавливаемых величин
  static RTCTime NowTime;
  char Str[32];
  char St[5];
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
    if (NeedDrawCRC)
  {

  if (GetID_Dev()==0)
  {
  GetNumVer(St);
  }
  NeedDrawCRC=0;
  }
 
  }
  // Прорисовка нового индикатора
if(g_FirstScr)
{
  // здесь заполняем данными поля нового индикатора
  // не требущие изменения при первичной инициализации
      sprintf(Str,"t0.txt=\"%s\"яяя", MsgMass[65][CurrLang]); // "Дата"" 
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
    sprintf(Str,"t7.txt=\"%s\"яяя", MsgMass[64][CurrLang]); // Up-to DATE"
    NEX_Transmit((void*)Str);// 

  }
}
  
  if ((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))
  {
    myBeep(10);
    SetMode(ModeSetting);
    ModeDevice = MODESETUP;
        // посылка команды переключения окна на Settings (возврат)  
    CmdInitPage(5);
  }
  
  
}

void ModeCalibrate(void) // режим установки начального смещения
{
  SetIndexIM (3); // устанавливаем индекс длительности импульса с корректировкой по длинне (500нс
  SetIndexLN (0); // короткая линия (2км)
  PointsPerPeriod = NumPointsPeriod[GetIndexLN()]; // SetPointsPerPeriod( ... );
  PointInPeriod = 0;
  memset( RawData, 0, RAWSIZE * sizeof(DWORD) );
  myBeep(0);
//123  disable_timer ( 0 );
//123    reset_timer(2);
//123    enable_timer(2);
  CurrTimeAccum = 0;
  EnaTimerAccum = 1;
  
  Averaging(200,0,0);// включение питания и запуск прогревочного накопления 
  for (int i=0; i<NUMSHIFTZONE; ++i)
  {
    if (i<7) SetIndexLN (i);
    if ((i>6)&&(i<12))
    {
      SetIndexLN (i-5); 
      SetIndexIM (5);
    }
    if (i==12)
    {
      SetIndexLN (6); 
      SetIndexIM (6);
    }
    PointsPerPeriod = NumPointsPeriod[GetIndexLN()]; // SetPointsPerPeriod( ... );
    memset( RawData, 0, RAWSIZE * sizeof(DWORD) );
//123    reset_timer(2); // необходимо пересбрасывать таймер!
    //123 enable_timer(2);
    CurrTimeAccum = 0;
    EnaTimerAccum = 1;
    
    Averaging(300,0,0);// включение питания и запуск прогревочного накопления 
    DWORD Data = SetBegShiftZone (i, SearchShiftBeg (500));// запись мертвой зоны по индексу
    DrawCalibrating(Data);
    g_NeedScr=1;
    
  }
  SetIndexLN (0); 
  SetIndexIM (0);
  HV_LOW(ON); //ON LOW HIGH VOLT
  HV_SW(OFF); // OFF HIGH VOLT
  POWDET(OFF);
  POWREF (OFF);
  //123 SSPInit_Any(MEM_FL1); // Востанавливаем Инициализацию SSP для управления внешней FLASH (порт 1 та что на плате отладочной)
  WriteNeedStruct (0x04);
//123  FlashErasePage(CFG_USER); // чистим страницу установок пользователя прибора
//123  FlashWritePageSM(CFG_USER, StructPtr(CFG_USER), StructSize(CFG_USER), 0);
  SetMode(ModeSetOTDRprm);
  ModeDevice = MODEOTDRPRM;
  myBeep(20);
  SubModeMeasOTDR = NOTMODE;
  //123 enable_timer ( 0 );
  // возврат в окно Установки для рефлектометра
          // посылка команды переключения окна на Set_datetime (вызов)  
  CmdInitPage(10);

  //ClrKey (BNS_MASK); // сброс нажатых клавиш
}

//// Большая рамка
//void FrBig(unsigned char SW,  char Add)
//{
//    //  char CmdBuf[30];
//    //  DrawLine(0,0+SW*12,76,0+SW*12,12,0,screen); // очистка окна ,большого
//    // DrawLine(80,0+SW*12,124,0+SW*12,12,0,screen);
//    if (((76+Add)>127)||((76+Add)<0)) Add = 0;
//
//    DrawLine(0,4+SW*12,76+Add,4+SW*12,1,1);
//    DrawLine(0,4+SW*12,0,15+SW*12,1,1);
//    DrawLine(0,15+SW*12,76+Add,15+SW*12,1,1);
//    DrawLine(76+Add,4+SW*12,76+Add,15+SW*12,1,1);
//    // по приколу порисуем рамку на индикаторе
//    //1. очистим 2. нарисуем рамку
//  //sprintf( CmdBuf,"cls WHITEяяя"); // 0xff
//  //NEX_Transmit((void*)CmdBuf);// 
//  //sprintf( CmdBuf,"page 0яяя"); // 0xff
//  //NEX_Transmit((void*)CmdBuf);// 
//  //sprintf( CmdBuf,"draw %d,%d,%d,%d,REDяяя",0,4+SW*12,76+Add,15+SW*12); // 0xff
//  //NEX_Transmit((void*)CmdBuf);// 
//
//}
//// Маленькая рамка
//void FrSmall(unsigned char SW ,  char Add) // до самого края (09.10.2013)
//{
//    if (((80+Add)>127)||((80+Add)<0)) Add = 0;
//    DrawLine(80+Add,4+SW*12,127,4+SW*12,1,1);
//    DrawLine(80+Add,4+SW*12,80+Add,15+SW*12,1,1);
//    DrawLine(80+Add,15+SW*12,127,15+SW*12,1,1);
//    DrawLine(127,4+SW*12,127,15+SW*12,1,1);
//}
//// рамка произвольного размера и места
//void FrFree(unsigned char SW ,  int X, int Size)
//{
//    if (((X+Size)>0)&&((X+Size)<128))
//    {
//    DrawLine(X,4+SW*12,X+Size,4+SW*12,1,1);
//    DrawLine(X,4+SW*12,X,15+SW*12,1,1);
//    DrawLine(X,15+SW*12,X+Size,15+SW*12,1,1);
//    DrawLine(X+Size,4+SW*12,X+Size,15+SW*12,1,1);
//    }
//  
//
//}
//
BYTE ChangeFrSet (BYTE FrSet, BYTE MaxSet, BYTE MinSet, BYTE DirSet)// установка курсора в рамках заданных параметров
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

// Рисуем источник и красный глаз
BYTE DrawSourceVFL (BYTE* frameInx, char* Str, DWORD KeyP, BYTE Lang, BYTE FrmMax, BYTE FrmMin)
{
    BYTE RedEye = GetCfgRE();
#if !ENADRAWORL
  {
    switch (*frameInx-FrmMax)
    {
    case 0: // Source
      if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
      {
        myBeep(10);
        g_NeedScr = 1; // Need reDraw Screen
        GetPlaceLS(NEXT);
        //ClrKey (BTN_RIGHT);
      }
      if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))
      {
        myBeep(10);
        g_NeedScr = 1; // Need reDraw Screen
        SetModeLS(Str,INCR,Lang );
        //ClrKey (BTN_RIGHT);
      }
      if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))
      {
        myBeep(10);
        g_NeedScr = 1; // Need reDraw Screen
        SetModeLS(Str,DECR,Lang );
        //ClrKey (BTN_LEFT);
      }
      break;
    case 1: // VFL
      if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))
      {
        myBeep(10);
        g_NeedScr = 1; // Need reDraw Screen
        SetModeRE(Str,INCR,Lang );
        //ClrKey (BTN_RIGHT);
      }
      if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))
      {
        myBeep(10);
        g_NeedScr = 1; // Need reDraw Screen
        SetModeRE(Str,DECR,Lang );
        //ClrKey (BTN_LEFT);
      }
      break;
    }
    if ((PRESS(BTN_UP))&&(getStateButtons(BTN_UP)==SHORT_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      *frameInx = ChangeFrSet (*frameInx, FrmMax+RedEye, FrmMin, MINUS);// ????????? ??????? ? ?????? ???????? ??????????
      //ClrKey (BTN_UP);
    }
    if ((PRESS(BTN_DOWN))&&(getStateButtons(BTN_DOWN)==SHORT_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      *frameInx = ChangeFrSet (*frameInx, FrmMax+RedEye, FrmMin, PLUS);// ????????? ??????? ? ?????? ???????? ??????????
      //ClrKey (BTN_DOWN);
    }
    

  }
  
#endif
  
#if ENADRAWORL
  // c ORL
  {
    switch (*frameInx-FrmMax)
    {
    case 0: // Source
      if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
      {
        myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
      GetPlaceLS(NEXT);
      }
      if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))
      {
        myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
          SetModeLS(Str,INCR,Lang );
      }
      if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))
      {
        myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
          SetModeLS(Str,DECR,Lang );
      }
      break;
    case 1: // теперь как бы только ORL здесь
      if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
      {
        myBeep(20);
        GetPlaceLS(CURRENT);
        PreSetModeLS(0); // вроде как выключаем источник!
        SetModeLS(Str,0,Lang ); // прорисовываем индикацию
        SetupSource (GetModeLS()); // установка режима работы лазера  
        // Здесь функция измерения ORL 
        g_VolORL = MeasORL (1000, 1);
      g_NeedScr = 1; // Need reDraw Screen
        myBeep(20);
      }
      //    }
      break;
    case 2: // VFL когда только источник ?
      if(FrmMax==1)
      {
        if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))
        {
          myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
          SetModeRE(Str,INCR,Lang );
        }
        if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))
        {
          myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
          SetModeRE(Str,DECR,Lang );
        }
      }
      break;
    }
    if ((PRESS(BTN_UP))&&(getStateButtons(BTN_UP)==SHORT_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
        if(FrmMax==1)
   *frameInx = ChangeFrSet (*frameInx, FrmMax+1+RedEye, FrmMin, MINUS);// ????????? ??????? ? ?????? ???????? ??????????
    else
   *frameInx = ChangeFrSet (*frameInx, FrmMax+1, FrmMin, MINUS);// ????????? ??????? ? ?????? ???????? ??????????
    }
    if ((PRESS(BTN_DOWN))&&(getStateButtons(BTN_DOWN)==SHORT_PRESSED))
    {
      myBeep(10);
      g_NeedScr = 1; // Need reDraw Screen
        if(FrmMax==1)
   *frameInx = ChangeFrSet (*frameInx, FrmMax+1+RedEye, FrmMin, PLUS);// ????????? ??????? ? ?????? ???????? ??????????
    else
   *frameInx = ChangeFrSet (*frameInx, FrmMax+1, FrmMin, PLUS);// ????????? ??????? ? ?????? ???????? ??????????
    }

    
  }
#endif
  return 1;
}

BYTE Measuring ( void)
{
  return SubModeMeasOTDR;
}

int SearchShiftBeg (int Size)// поиск мертвой зоны 
{
  DWORD LocalMax=0;
  static int i;
  int IndexLocMax=0;
  char EndCalc = 0;
  for ( i= 0; i<Size; i++)// поиск локального максимуа
  {
    if( RawData[i]>LocalMax)
    {
      LocalMax = RawData[i];
      IndexLocMax = i;
    }
  }
  for ( i = IndexLocMax ;((i>0)&&(!EndCalc));i--)
  {
    if ((LocalMax-RawData[i]) > 6*RawData[i]) // проверяем порог провала сигнала на "большую" величину
      
      EndCalc = 1;
  }
    return (i+2);// поправлено т.к. изменился алгоритм поиска начала!
  
}

//void (*ModeFunc)(void);

void DrawCalibrating (DWORD Data)// рисование окна калибровки
{
  char Str[22];
  //BYTE CurrLang;
  //CurrLang=GetLang(CURRENT);
  // здесь порисуем для нового индиктора
    if (g_FirstScr)
  {
    // здесь заполняем данными поля нового индикатора
    // не требущие изменения при первичной инициализации
    sprintf(Str, "t0.txt=\"%s\"яяя", MsgMass[28][CurrLang]);
    NEX_Transmit((void*)Str);    // 
  
    sprintf(Str, "t1.txt=\"%s\"яяя", MsgMass[40][CurrLang]);
    NEX_Transmit((void*)Str);    // 
  
    sprintf(Str, "t2.txt=\"%s\"яяя", MsgMass[19][CurrLang]); //!
    NEX_Transmit((void*)Str);    // 
  
    sprintf(Str, "t3.txt=\"%s\"яяя", MsgMass[22][CurrLang]); //!
    NEX_Transmit((void*)Str);    // 
  
  
    g_FirstScr = 0;
    g_NeedScr = 1;
  }
  if (g_NeedScr)
  {
    // здесь заполняем данными поля нового индикатора
    // по результатам изменений вызваныйх обработчиком клавиатуры
    
    sprintf(Str, "t4.txt=\"%d%s\"яяя", GetLengthLine(GetIndexLN()),MsgMass[20][CurrLang]); //!
    NEX_Transmit((void*)Str);    // 
    
     sprintf(Str, "t5.txt=\"%d%s\"яяя", GetWidthPulse(GetIndexIM()),MsgMass[23][CurrLang]); //!
    NEX_Transmit((void*)Str);    // 
  
    sprintf(Str, "t6.txt=\"%d\"яяя",Data);
    NEX_Transmit((void*)Str);    // 
  
  
  									 // код подсветки требуемой строки если есть есть маркер строки
    g_NeedScr = 0;
  }

}

BYTE IndexSeek( int Data )// поиск индекса длины при проверке длины линии
{
  int out = 0;
  for(;(Data >= KeyPoints[out])&&(out<PNTSNUM-1); ++out);
  return out;
}

// программа сканирования на поиск конца линии
DWORD Scan( DWORD* array, unsigned size, DWORD level )
{
  const unsigned step = 10;
  const unsigned inStep = 3;
  //unsigned cntr;
  //unsigned short curPart;
  for( unsigned i = size-1; i >= 90;)
  {
    if( array[i] > level )
    {
      unsigned second = 0;
      unsigned cntr = 1;
      for( unsigned j = 1; j < step; ++j )
      {
        if( array[i-j] > level )
        {
          if(second == 0) second = j;
          if( ++cntr == inStep )
            return i;
        } 
      }  
      i -= (second ? second : step);
    }
    else
      i -= 1;
  } 
  return 90;
}

float GetPosLine (unsigned Cursor) // получение длины от позиции курсора
{
  float PosLine;
  // установим новые значения параметров съема (при чтении файла)
  PointsPerPeriod = NumPointsPeriod[GetIndexLN()]; // SetPointsPerPeriod( ... );
 //ValueDS = (unsigned)((ADCPeriod*50000)/PointsPerPeriod); //  устанавливаем значения DS для установленного режима измерения
  ValueDS = GetValueDS(); //  устанавливаем значения DS для установленного режима измерения

  PosLine = (float)(LIGHTSPEED*1.0E-14);
  PosLine = PosLine*ValueDS;
  PosLine = PosLine/GetIndexWAV();
  PosLine = PosLine*Cursor;
  PosLine = PosLine/1000.0;
  return PosLine;
  
}

// off 02/11/2023
// прорисовка клавиатуры с установленным курсором на символ
//void Draw_KeyBoard(unsigned char Cursor, char* Str_first, char* Str_edit, BYTE Shift ,BYTE Lang)
//{
//  char CommScr[20] = "                   \0"; // строка редактора для вывода на экран
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
BYTE GetEnaMZ (void) // получение текущего значения разрешения корректировки метрвых зон
{
  return EnaCorrMZ;
}
BYTE SetEnaMZ (BYTE Mode) //  установка  значения разрешения корректировки метрвых зон
{
  if(Mode)
    EnaCorrMZ=1;
  else
    EnaCorrMZ=0;
  return EnaCorrMZ;
}
//
BYTE GetCurrentModeDevice (void) // получение текущего режима прибора
{
  return ModeDevice;
}

BYTE GetModePowerMeter (void) // получение текущего режима измерителя
{
  return ModePowerMeter;
}

BYTE SetModeDevice (BYTE Mode) // принудительная установка режима прибора
{
  switch (Mode)
  {
  case MODESETREFL: // режим установок рефлектомтра
    SetIndexLN(GetIndexLN());//устанавливаем текущие установки по длинам и импульсам
    PosCursorMain (-4100); // сброс курсора в начало
    IndexVerSize  = 0;// установка вертикального размера отображения рефлектограммы ( самый крупный)
    GetSetHorizontScale (5); // сброс масштаба уст самый крупный
    // выключаем возможно вкл. источник
    PreSetModeLS(0);
    SetupSource (GetModeLS()); // установка режима работы лазера  
    SetMode(ModeSetupOTDR);
    GetSetModeLW(-1); // сброс счетчика так как принудительная установка
    ModeDevice = MODESETREFL;
    HV_LOW(ON); //ON LOW HIGH VOLT
    HV_SW(OFF); // OFF HIGH VOLT
    POWREF (OFF);
    POWDET(OFF);
//123    SSPInit_Any(SPI_ALT); // Инициализация SSP для управления ALTERA (порт 1 та что на плате отладочной)
      CmdInitPage(2);// посылка команды переключения окна на OTDR
    
    break;
  case MODEREFL :// режим рефлектометра
    //TraceREAD(0);// после проверки установлнеа 0 рефлектограмма ее и читаем
    memset( LogData, 65535, OUTSIZE * sizeof(unsigned short) ); // ZERO_ARRAY( array ) memset( array, 0, sizeof(array) )
    SetMode(ModeDrawOTDR);
    ReturnModeViewRefl = SETPARAM;//VIEWMEM -  чтобы вернуться в в установки
    // выключаем возможно вкл. источник
    PreSetModeLS(0);
    SetupSource (GetModeLS()); // установка режима работы лазера  
    ModeDevice = MODEREFL;
    HV_LOW(ON); //ON LOW HIGH VOLT
    HV_SW(OFF); // OFF HIGH VOLT
    POWREF (OFF);
    POWDET(OFF);
//123    SSPInit_Any(SPI_ALT); // Инициализация SSP для управления ALTERA (порт 1 та что на плате отладочной)
    CmdInitPage(18);// посылка команды переключения окна на OTDR просмотр
    break;
  case MODEMEMR:// режим работы с памятью рефлектограмм
    SetMode(ModeMemoryOTDR);
    // выключаем возможно вкл. источник
    PreSetModeLS(0);
    SetupSource (GetModeLS()); // установка режима работы лазера  
    ModeDevice = MODEMEMR;
    ModeMemDraw = VIEWNEXT;
    HV_LOW(ON); //ON LOW HIGH VOLT
    HV_SW(OFF); // OFF HIGH VOLT
    POWREF (OFF);
    POWDET(OFF);
    //123 SSPInit_Any(MEM_FL1); // Инициализация SSP для FLASH (порт 1 та что на плате отладочной)
      CmdInitPage(13);// посылка команды переключения окна на OTDR память
    break;
  case MODETEST:// режим ТЕСТЕРА
    POWDET(ON); // включаем питание измерителя
    if (GetCfgPM ())
    {
      SetMode(ModeMeasManualOLT);
          CmdInitPage(7);// посылка команды переключения окна на ТЕСТЕР
    }
    else 
    {
      SetMode(ModeSourceOnly); // сразу тестер (просто источник)
          CmdInitPage(7);// посылка команды переключения окна на ТЕСТЕР
    }
    ModeDevice = MODETEST;
    //123 SSPInit_Any(SPI_PM); // Инициализация SSP для управления ALTERA (порт 1 та что на плате отладочной)
    break;
  case MODETESTMEM:// режим ТЕСТЕРА память
    if (GetCfgPM ())
    {
      POWDET(ON); // включаем питание измерителя
      SetMode(ModeViewMemOLT);
      //123 SSPInit_Any(MEM_FL1); // Инициализация SSP для FLASH (порт 1 та что на плате отладочной)
      ModeDevice = MODETESTMEM;
      CmdInitPage(20);// посылка команды переключения окна на память тестера
    }
    else 
    {
    HV_LOW(ON); //ON LOW HIGH VOLT
    HV_SW(OFF); // OFF HIGH VOLT
    POWREF (OFF);
    POWDET(OFF);
    SetMode(ModeMainMenu);
    ModeDevice = MODEMENU;
      CmdInitPage(1);// посылка команды переключения окна на MainMenu
    }
    break;
  case MODESETUP:// режим установок пользователя
    HV_LOW(ON); //ON LOW HIGH VOLT
    HV_SW(OFF); // OFF HIGH VOLT
    POWREF (OFF);
    POWDET(OFF);
    SetMode(ModeSetting);
    ModeDevice = MODESETUP;
    //123 SSPInit_Any(MEM_FL1); // Инициализация SSP для FLASH (порт 1 та что на плате отладочной)
      CmdInitPage(5);// посылка команды переключения окна на Setting

    break;
  case MODEMEASURE:// режим измерения рефлектометра (накопление)
    CmdInitPage(16);// посылка команды переключения окна на начало измерения Measuring
    SlowON();
    //POWALT(ON);
    //POWREF (ON);
    //POWDET(ON);
    //IndexVerSize  = 0;// установка вертикального размера отображения рефлектограммы ( самый крупный)
    SubModeMeasOTDR = SETPOWER;
    SetMode(ModeStartOTDR);
    ModeDevice = MODEMEASURE;
    //123 SSPInit_Any(SPI_ALT); // Инициализация SSP для управления ALTERA (порт 1 та что на плате отладочной)
    
    break;
  default:// режим МЕНЮ
    HV_LOW(ON); //ON LOW HIGH VOLT
    HV_SW(OFF); // OFF HIGH VOLT
    POWREF (OFF);
    POWDET(OFF);
    SetMode(ModeMainMenu);
    ModeDevice = MODEMENU;
    //123 SSPInit_Any(MEM_FL1); // Инициализация SSP для FLASH (порт 1 та что на плате отладочной)
    //  CmdInitPage(1);// посылка команды переключения окна на MainMenu
    CmdInitPage(1);// посылка команды переключения окна MainMenu
    break;
  }
  return ModeDevice;
}

void StartSettingBegShift (void) // старт измерения мертвых зон
{
        myBeep(10);
        SlowON();
        HV_LOW(ON); //ON LOW HIGH VOLT
        HV_SW(ON); // ON HIGH VOLT
        //POWALT(ON);
        //POWREF (ON);
        //POWDET(ON);
        //123 SSPInit_Any(SPI_ALT); // Инициализация SSP для управления ALTERA (порт 1 та что на плате отладочной)
        SetIndexIM (0);
        SetIndexLN (0);
        PointsPerPeriod = NumPointsPeriod[GetIndexLN()]; // SetPointsPerPeriod( ... );
        PointInPeriod = 0;
        memset( RawData, 0, RAWSIZE * sizeof(DWORD) );
        SetNumAccumPerSec (200);// установка значения числа накоплений
        SubModeMeasOTDR = AVERAGING;
        for (int i = 0; i< NUMSHIFTZONE; ++i)
        {
         SetBegShiftZone (i, 0);// запись мертвой зоны = 0
        }
        SetMode(ModeCalibrate);
        ModeDevice = MODEOTHER;
}

void InitAutoPM (void) // начальные установки измерителя автомата
{
      ModAutoPM = WAITING; // режим измерителя авто ожидаем
      IndWavePON = 0; //индекс длины волны в перечне калибровочных волн
      TimerAutoPM = TimerPA(1); // счетчик времени (обнуляем)
      for(int i=0; i<5; i++) 
      {
        RPON[i]=-100.0;
        LWPON[i] = 0; // обнуляем список длин волн
        LWCNT[i] = 0; // clear cnt 
      }
      SetSwitchPMMode(MANUAL);  // Устанавливает режим переключения диапазонов в ручную
      SetRange(1);
}

void SavePowerMeter(float Pow_mW)// функция сохранения в памяти ИЗмерений
{
  char Str[20];
  int j=0;
  for (int i=0; i<3; i++)
  {
    PONI.LenWaveKlb[i]=0;
    PONI.PowLevel[i]=-100.0;
    PONI.BaseLvl[i]=-100.0;
  }
    PONI.CellMod=0;
    TimeSaveOLT = RTCGetTime(); // сохраняем время сбора
    PONI.TotalTimeCell = TotalSec( TimeSaveOLT); // считаем секунды// подсчет общего времени в сек
    PONI.CommUserPM[ARRAY_SIZE(PONI.CommUserPM)-1]=0; // последний элемент в массиве равен 0
      for (int Ind =ARRAY_SIZE(PONI.CommUserPM)-2; Ind>=0; Ind--)
      {
        if (PONI.CommUserPM[Ind]<0x20) PONI.CommUserPM[Ind]=' ';
        //if (PONI.CommUserPM[Ind]!=' ')break;
        //Index_Comm --;
      }
  switch (ModePowerMeter)
  {
case POWMANUAL:
  // заполняем структуру хранения измерений
    PONI.LenWaveMeas =  GetPMWavelenght(0);
    PONI.CellMod = GetTypeRslt();
    PONI.PowLevel[0]= Watt2dB(Str, Pow_mW,0); //dBm
    PONI.PowLevel[1]= Watt2dB(Str, Pow_mW,1); //dB
    PONI.PowLevel[2]= Pow_mW;
    PONI.BaseLvl[0] = GetCurrLvldB(0);
    //if (!En_Inc)  PONI.NumFix++;  // это надо делать только когда сохраняем
    PONI.Rez = MANUAL;
    /*
  if (Cell_Mem<MaxMem) Cell_Mem++;
  else 
  {
    Cell_File = Cell_File + Cell_Mem;
    Cell_Mem = 0;
  }
    */
  break;
  case POWAUTO:
    // запись в ячейки
    PONI.Rez = AUTO;
        for (int i=0; i<5; i++)
        {
          if ((RPON[i]>-100.0)&&(j<3)) // есть значение для индикации
          {
              PONI.PowLevel[j]=RPON[i];
              PONI.BaseLvl[j]=GetCurrLvldB(CoeffPM.PointKalib[i]) ;
              //PONI.LenWaveKlb[j]=CoeffPM.PointKalib[i]; // LWPON[i]
              PONI.LenWaveKlb[j]=LWPON[i]; // LWPON[i] 1300

              j++;
          }
        }

    break;
  default:
    break;
    
  }
    //123 SSPInit_Any(MEM_FL1); // Инициализация SSP для управления FLASH (порт 1 та что на плате отладочной)
    //123 FlashErasePage(CFG_USER); // чистим страницу установок пользователя прибора
    //123 FlashWritePageSM(CFG_USER, StructPtr(CFG_USER), StructSize(CFG_USER), 0);
    //123 SSPInit_Any(SPI_PM); // востановление SSP для управления PM (порт 1 та что на плате отладочной)
  WriteNeedStruct(0x04);
}

void WriteMemPow(void) // запись в память непосредственно
{
  WORD TmpCellMem = GetCellMem(0);
    //123 SSPInit_Any(MEM_FL1); // Инициализация SSP для управления FLASH (порт 1 та что на плате отладочной)
  
if (TmpCellMem < MaxMemPM)
      {
//123!!!      WriteCellIzm(TmpCellMem, (unsigned char*)&PONI);
      if (TmpCellMem < MaxMemPM) GetCellMem(1);
      if (GetEnIncFiber(0))    // это надо делать только когда сохраняем
      {
        if (PONI.NumFix<9999) PONI.NumFix++;
        else PONI.NumFix=0;
      }
    //FlashErasePage(CFG_USER); // чистим страницу установок пользователя прибора
    //FlashWritePageSM(CFG_USER, StructPtr(CFG_USER), StructSize(CFG_USER), 0);
    WriteNeedStruct(0x04);
      }
    else myBeep(20);          
    ReturnToTester (1); // возврат в измеритель
    //123 SSPInit_Any(SPI_PM); // востановление SSP для управления PM (порт 1 та что на плате отладочной)
  
}

void ReturnToTester (BYTE EnClr) // возврат в измеритель( здесь же выберем окно нового индикатора куда вернуться)
{
      switch (ModePowerMeter)
    {
    case POWMANUAL:
    SetMode(ModeMeasManualOLT);
          // посылка команды переключения окна на Tester (возврат)  
      CmdInitPage(7);
      break;
    case POWAUTO:
      if (EnClr) InitAutoPM (); // начальные установки измерителя автомата
    SetMode(ModeMeasAutoOLT);
    TimerPA (1); // сброс таймера1
          // посылка команды переключения окна на Tester (возврат)  
      CmdInitPage(8);
      break;
      // если попали в тестер по команде UART то что бы выйти
    default:
    SetMode(ModeMeasManualOLT);
          // посылка команды переключения окна на Tester (возврат)  
      CmdInitPage(7);
      break;
      
    }
    ModeDevice = MODETEST;
}

void  DrawLevelStr(char* Str) //???? вывод признака в чем выводим W/ dB/ dBm
{
  //char Watt[5];
         switch (PONI.CellMod)
    {
    case 0: // дБм
    case 1: // дБ
  switch ( GetCfgPM ()) // получение установки измерителя
  {
    case 1:
      sprintf(Str,"%s  %.2f", MsgMass[48][CurrLang],PONI.PowLevel[0]);//дБм
      if (PONI.PowLevel[0] <= -85.0)
      {
      sprintf(Str,"%s  <%.2f", MsgMass[48][CurrLang], -85.0);//дБм
      }
      break;
    case 2:
      sprintf(Str,"%s  %.2f", MsgMass[48][CurrLang],PONI.PowLevel[0]);//дБм
      if (PONI.PowLevel[0] <= -60.0)
      {
      sprintf(Str,"%s  <%.2f", MsgMass[48][CurrLang],-60.0);//дБм
      }
      break;
    default:
      sprintf(Str,"%s  %.2f", MsgMass[48][CurrLang],PONI.PowLevel[0]);//дБм
      break;
  }
      //putWideString(20,2,screen,Str2,1);
      break;
    case 2: // миливаты
      if (PONI.PowLevel[2] >= 1) // миливатты
      {
        sprintf(Str,"%s  %.2f",MsgMass[97][CurrLang],PONI.PowLevel[2]);
        //putWideString(20,2,screen,Str2,1);
      }
      if ((PONI.PowLevel[2] >= 0.001)&&(PONI.PowLevel[2] < 1.0)) // микроватты
      {
        sprintf(Str,"%s  %.2f",MsgMass[96][CurrLang],PONI.PowLevel[2]*1.0e3);
        //putWideString(20,2,screen,Str2,1);
      }
      
      if ((PONI.PowLevel[2] >= 1e-6)&&(PONI.PowLevel[2] < 0.001)) // нановатты
      {
        sprintf(Str,"%s  %.2f",MsgMass[95][CurrLang],PONI.PowLevel[2]*1.0e6);
        //putWideString(20,2,screen,Str2,1);
      }
      
      if (PONI.PowLevel[2] < 1e-6) // меньше 1 нВт
      {
        if (PONI.PowLevel[2] < 1e-9)PONI.PowLevel[2] = 1e-9;
        sprintf(Str,"%s  %.2f",MsgMass[94][CurrLang],PONI.PowLevel[2]*1.0e9);
        //putWideString(20,2,screen,Str2,1);
      }
      break;
    }
}

void ReLoadCommOLT (void) // перезагружаем комментарии для измерителя
{
      memcpy ( CommentsOLT,PONI.CommUserPM,16); // пишем 
      CommentsOLT[ARRAY_SIZE(CommentsOLT)-1]=0; // последний элемент в массиве равен 0
      IndexCommOLT = 0;
      
      for (int Ind =ARRAY_SIZE(CommentsOLT)-2; Ind>=0; Ind--)
      {
        if (CommentsOLT[Ind]<0x20) CommentsOLT[Ind]=' ';
        else if (CommentsOLT[Ind]!=' ' && IndexCommOLT == 0) IndexCommOLT = Ind;
        //Index_Comm --;
      }
      if (CommentsOLT[IndexCommOLT]!=' ')IndexCommOLT ++;
      KbPosX = 11;
      KbPosY = 2;
}

WORD CheckLevelBattery (void) // контроль заряда баттареи
{
  static volatile WORD BufProcBat[10]={100,100,100,100,100,100,100,100,100,100};
  WORD ProcBatMed, ProcBatSum;
//123  ADCData = ADC0Read(BAT_ADC);//новое правило получения данных АЦП батарейки
  ADCData = 2048;
  Ubat = ADCData * GetSetBatStep(0)* 3 ; // так как делитель на 3, может не надо -0.3- 0.3
    if (Ubat > 5.1) ProcBat = 100;
  //else if (Ubat >= 4.0) ProcBat = (WORD)((Ubat-4.0)*90.91);//  1.1в = 100%
  //else if (Ubat >= 4.3) ProcBat = (WORD)((Ubat-4.3)*125);//  0.8в = 100%
  else if (Ubat >= 4.15) ProcBat = (WORD)((Ubat-4.15)*105.26);//  0.95в = 100%
  if (Ubat<4.15)
  {
    ProcBat = 0; // плохая батарея
  }
  if (!(EXT_POW)) ProcBat = 100; // если внешне питание подсветку включаем

//123  SetContrast (Ubat, GetUserContr()); // возможно не нужна!
  ProcBatSum=0;
  for (int i=9;i>0;--i)
  {
    BufProcBat[i]=BufProcBat[i-1];
    ProcBatSum += BufProcBat[i];
  }
  
  BufProcBat[0]=ProcBat;
  ProcBatSum += BufProcBat[0];
  ProcBatMed = (WORD)(ProcBatSum/10);
  // для индикации
  if (ProcBatMed > 20) ProcBatInd = (WORD)(1.25*ProcBatMed - 25);
  else ProcBatInd = 0;
  //ProcBatInd = ProcBatMed;
  return ProcBatMed;
}

int GetBateryADC (void)
{
  return  ADCData;
}

//123
//void SetContrast (float Data, WORD UserContr) // установка авто контроаста с учетом пользовательских установок
//{
//  static unsigned int NewContrast = 0;
//  if (Data < 5.0) NewContrast = PeriodPWM/2;
//  else if (Data < 5.8) NewContrast = (unsigned int)(PeriodPWM/2 + (Data - 5.0)*6000);
//  else  NewContrast = 15000; 
//    ChangeLenPWM (NewContrast + ((50-UserContr)*USERSTEPONTR));
//}

void BadBattery(void) // плохая баттарейка CHECK_OFF
{ 
  char F_name[32];
  WORD CntOff = 0;
  WORD LevelBat;
  REDEYE(0); // выключаем красный лазер
  HV_LOW(ON); //ON LOW HIGH VOLT
  HV_SW(OFF); // OFF HIGH VOLT
  POWREF (OFF);
  POWDET(OFF);
  SubModeMeasOTDR = NOTMODE;
  SetMode(ModeMainMenu);
  ModeDevice = MODEMENU;
    CmdInitPage(27);// MainMenu

  // Выделим блок для выключения если LCD не пользуем
  // заполним индикатор Nextion
    sprintf (F_name,"t0.txt=\"%s\"яяя",MsgMass[71][CurrLang]); // 
    NEX_Transmit((void*)F_name);    sprintf (F_name,"t1.txt=\"%s\"яяя",MsgMass[72][CurrLang]); // 
    NEX_Transmit((void*)F_name);//
    sprintf (F_name,"t2.txt=\"%s\"яяя",MsgMass[73][CurrLang]); // 
    NEX_Transmit((void*)F_name);//
  
  LevelBat = CheckLevelBattery();
  while((LevelBat < 60)&&(EXT_POW)) // если подключили внешнее питание то переключимся  в норм режим
  {
    if (CntOff++%60 == 0) myBeep(3);
     // читаем состояние батареи
    LevelBat = CheckLevelBattery();
    // ProcBatInd
    //sprintf(F_name,"BAT = %d%%", LevelBat );
    sprintf (F_name,"t3.txt=\"BAT = %d%%(%1.2f)\"яяя",ProcBatInd,Ubat); // 
    NEX_Transmit((void*)F_name);//
    
  }
  CmdInitPage(1);// MainMenu
  myBeep(25);

}

// переключимся в режим программирования индикатора (пока на паузу  и сигнал
void UploadFW_Nextion(void) // обновление индикатора NEXTION
{ 
  char Str[32];
  REDEYE(0); // выключаем красный лазер
  HV_LOW(ON); //ON LOW HIGH VOLT
  HV_SW(OFF); // OFF HIGH VOLT
  POWREF (OFF);
  POWDET(OFF);
  //123 enable_timer ( 0 );
    // здесь порисуем для нового индиктора
    if (g_FirstScr)
  {
    // здесь заполняем данными поля нового индикатора
    // не требущие изменения при первичной инициализации
    sprintf(Str, "t0.txt=\"%s\"яяя", MsgMass[119][CurrLang]);
    NEX_Transmit((void*)Str);    // 
  
    sprintf(Str, "t1.txt=\"%s\"яяя", MsgMass[120][CurrLang]);
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
  SubModeMeasOTDR = NOTMODE;
  SetMode(ModeMainMenu);
  ModeDevice = MODEMENU;
      // посылка команды переключения окна на MainMenu (возврат)  
  // вызовем новое окно!
  //CreatDelay(5000000);
  SetModeDevice (MODEMENU); // принудительная установка режима прибора
  CmdInitPage(1);
  HAL_Delay(500);// индикатор после сброса, время не понятно!
  myBeep(125);
  }

}

void KeybCntrl (void) // переключатель указателя в клавиатуре
{
     if ((PRESS(BTN_UP))&&((getStateButtons(BTN_UP)==SHORT_PRESSED)||(getStateButtons(BTN_UP)==INF_PRESSED)))
      {
        myBeep(7);
        if (KbPosY > 0) KbPosY--;
        else KbPosY =3;
      }
    if ((PRESS(BTN_DOWN))&&((getStateButtons(BTN_DOWN)==SHORT_PRESSED)||(getStateButtons(BTN_DOWN)==INF_PRESSED)))
      {
        myBeep(7);
        if (KbPosY < 3) KbPosY++;
        else KbPosY =0;
      }
    if ((PRESS(BTN_LEFT))&&((getStateButtons(BTN_LEFT)==SHORT_PRESSED)||(getStateButtons(BTN_LEFT)==INF_PRESSED)))
      {
        myBeep(7);
        if (KbPosX > 0) KbPosX--;
        else KbPosX =11;
      }
    if ((PRESS(BTN_RIGHT))&&((getStateButtons(BTN_RIGHT)==SHORT_PRESSED)||(getStateButtons(BTN_RIGHT)==INF_PRESSED)))
      {
        myBeep(7);
        if (KbPosX < 11) KbPosX++;
        else KbPosX =0;
      }
}

BYTE CheckPONI (WORD TmpCellMem) // проверка текущей ячейки и ее презапись если что не так
{
  char Str[16] = {"Error S a v e !\0"};
  BYTE Err = 0;
  if (PONI.TotalTimeCell>0x80000000)Err = 1;
  for (int i=0; i<3; i++)
  {
    if (PONI.LenWaveKlb[i]>1700)Err = 1;
    if (isnan(PONI.PowLevel[i]))Err = 1;
    if (isnan(PONI.BaseLvl[i]))Err = 1;
  }
  if (PONI.CellMod>2) Err=1;
  if (PONI.LenWaveMeas>1700) Err=1;
  if (PONI.NumFix>9999) Err=1;
  if (PONI.Rez>1) Err=1;
  if (PONI.CommUserPM[15]!=0) Err =1;
  if (Err)
  {
    memcpy(&PONI.CommUserPM, Str,16);
  for (int i=0; i<3; i++)
  {
    PONI.LenWaveKlb[i] = 0;
    PONI.PowLevel[i] = -100.0;
    PONI.BaseLvl[i] = -100.0;
  }
  PONI.CellMod=0;
  PONI.LenWaveMeas=1310;
  PONI.NumFix=0;
  PONI.Rez=0;
  PONI.TotalTimeCell = 10*YearSecV-1;
//123!!!  WriteCellIzm(TmpCellMem, (unsigned char*)&PONI);
  
  }
  return Err;
}

// off 02/11/2023
//void TestScreen (void)
//{
//    ClearScreen();
//for (int z = 1; z>=0; z--)
//{
//  for (int x = 0; x<128; ++x)
//  {
//    for (int y = 0; y<64; ++y)
//    {
//      PutPixel(x, y, z);
//    }
//    PaintLCD();
//  }
//}
//}

// сохранение рефлектограммы (по режиму от кнопок и по команде от UART)
int SaveNewOTDRTrace (BYTE Mode)
{
  char BufStrOut[25];
  int Ret=0;
  // получаем новое время, и сохраняем вновь в 0 и следующую
  
  SetMode(ModeMemoryOTDR);
  ModeDevice = MODEMEMR;
  TimeSaveOTDR = RTCGetTime(); // сохраняем время сбора
  myBeep(10);
  
  Ret = 1;
  //  SaveTrace(); // сохраняем текущую трассу(по плученному
    SaveFileSD(1); // сохраняем текущую трассу( on SD Card
  //sprintf(txtout,"%s %d\r",CommentsOTDR,Ret);//c
  //      UARTSend0 ((BYTE*)txtout, strlen (txtout));

  if (Ret) ModeMemDraw = SAVEDTRACE;
  else ModeMemDraw = MEMFULL;
  if (Mode) // сохранение по UART
  {
    if (Ret) sprintf(BufStrOut,"Trace saved %d\r",Ret);//c
    else sprintf(BufStrOut,"Mem full\r");//c
    UARTSendExt ((BYTE*)BufStrOut, strlen (BufStrOut));
    
  }
      // посылка команды переключения окна на Mem_OTDR_graph (возврат)  
  // возможно бы тут надо бы ПОТУПИТЬ
  // 07.11.2022 (148)
     // CreatDelay(100000);
  HAL_Delay(10);
      CmdInitPage(13);
//  myBeep(10);
//      CreatDelay(200000);
//      CmdInitPage(13);
//  myBeep(10);

 return Ret; 
}
// сохраение текущих шумов
void SaveNoise (DWORD Noise)
{
  MeagNoise = Noise;
}
// предрасчет сообытий по необработанным данным
void CalkPreEvents (DWORD* array, unsigned short PII)
{
  float Correlations;
  char OutText[20];

    for (int i = 0 ; i<RAWSIZE-PII-20; i++)
  {
          Correlations = CalkCorelation32(array, i, PII, 0); // расчет коррреляции неотражающего затухания
      // модуль вывода вспомогательной информации по определению событий  
      sprintf (OutText, "%d %d %0.3f %0.6f\n",i,array[i],log10(array[i]),Correlations);
      UARTSendExt ((BYTE*)OutText, strlen (OutText));
      
  }
}
// получение пользовательских настроек отображения рефлектограммы
// получение вертикального масштаба
BYTE GetUserVerSize (void)
{
  return IndexVerSize;
}
// получение пользовательского уровня центровки графика
BYTE GetUserTypeCentral (void)
{
  return TypeCentral;
}

void PrintEventTbl (void) // вывод по RS таблицы событий
{
    char OutText[20];
   unsigned short  NumEventOut =  (CalkEventsKeys (LogData, PointsInImpulse(0), 1)); 
   
   sprintf(OutText,"%.1f %s; ",fabs((GetPosLine(EndEvenBlk.ELMP[1]))*1000.0) ,MsgMass[78][CurrLang]);//м - метры
      UARTSendExt ((BYTE*)OutText, strlen (OutText));
    sprintf(OutText,"%.2f %s; ",(LogData[EndEvenBlk.ELMP[1]]-LogData[EndEvenBlk.ELMP[0]])/1000.0,MsgMass[47][CurrLang]);//дБ
      UARTSendExt ((BYTE*)OutText, strlen (OutText));
      //
      for (int i=1; i<=NumEventOut;++i)
      {
        if(EvenTrace[i-1].EC[1] == 'F')// Found by software
        {
      //sprintf(OutText,"%s ",EvenTrace[i-1].COMM_EVN);//комментарий
      //UARTSend0 ((BYTE*)OutText, strlen (OutText));
      sprintf(OutText,"Evnt %d %.1f ; ",i,fabs((GetPosLine(EvenTrace[i-1].EPT))*1000.0) );//м - метры
      UARTSendExt ((BYTE*)OutText, strlen (OutText));
      sprintf(OutText,"%.2f ; ",(EvenTrace[i-1].EL)/1000.0);//дБ
      UARTSendExt ((BYTE*)OutText, strlen (OutText));
      sprintf(OutText,"%.2f ; ",EvenTrace[i-1].ER/1000.0);//дБ
      UARTSendExt ((BYTE*)OutText, strlen (OutText));
        }
      }
      sprintf(OutText,"\n");//
      UARTSendExt ((BYTE*)OutText, strlen (OutText));
      SetGetMonEna(0);
}
// Программа сшивки рефлектограммы при перегрузке
// если после действия импульса иммем перегрузку (значение близкое к 0) то 
// ищем ближайшие N точек мало отличающиеся друг от друга до 0.05 дБ и 
// имеющие значение до 15 дБ, но больше 0.5 дБ
// программа сшивки работает при измерениях на "больших" импульсах 3 и 10 мкс
// попробуем здесь же фильтрануть шумы в конце линии
unsigned short SpliceProg (unsigned short PII)
{
  unsigned short EnaChange=0; // признак разрешения менять данные ( устанавливаем когда провалимся ниже 40 дБ)
  //int Difrn=0;
  //float MedLog=0;
  //unsigned long Summer=0;
  unsigned short AddSh=0; // добавка к смещению
  unsigned short Index=0;
  static unsigned short I_min=0, I_max=0;;
  static unsigned short min = 0xFFFF, max = 0;
  unsigned short deltaU = 150;
  unsigned long medium =0;
  if(GetLengthWaveLS (GetPlaceLS(CURRENT)) == 850) deltaU = 500;
  for (int i = (int)(PII+(PII>>1)) ; ((i<OUTSIZE)&&(LogData[i]<20000)&&(!Index)); ++i) // 26/02/2014 анализ начинаем позже на 1.5 импульса
  {
    if (LogData[i]>750)
    {
      min = 0xFFFF;
      max = 0;
      medium =0;
      I_min = i;
      I_max = i;
      for( int j = i; j <i+5; ++j )
      {
        medium = medium + LogData[j];
        if( min > LogData[j] )
        {
          I_min = j;
          min = LogData[j];
        }
        if( max < LogData[j] )
        {
          I_max = j;
          max = LogData[j];
        }
      }
      // для 850 нм на 5 точках при128км набегает более 0.366 дБ
      
      if (((max-min)<deltaU)&&(I_max>I_min))
      {
        Index = i+5;
        //medium = medium/5;
        medium = (unsigned long)FlashReadSpliceData(Index)- (unsigned long)LogData[Index];
        for (int o=1; o<Index; ++o) // перезапись от короткого импульса
        {
          LogData[o] = FlashReadSpliceData(o);
        }
        for (int o=Index; o<OUTSIZE; ++o) // изменение "хвоста"
        {
          // новые данные приклееного хвоста ( можно анализировать и изменять)
          if ((GetIndexLN()==6)&&(GetIndexIM()>=6)&&(GetIndexVRM()==3)&&GetApdiSet()) // если установлено время накопления 180с, длина линии 128 км, и длительность импульса 10 мкС
          {
//          case 1:
//            if (LogData[o]>35000)
//            {
//              EnaChange++;
//              Summer +=(unsigned long)LogData[o];
//            }
//            if (EnaChange>2) // можно менять данные
//            {
//              //if (++AddSh > 700) AddSh = 700;
//              AddSh = (unsigned short)((double)Summer/(double)EnaChange); // логарифмическое среднее
//              if (LogData[o]<35000)
//              {
//                LogData[o] = (unsigned short)(((float)LogData[o] + (float)AddSh)/2);
//              }
//            }
//            break;
//          case 2:
//            if (LogData[o]>40000)
//            {
//              EnaChange++;
//            }
//            if (EnaChange>4)
//            {
//              if (LogData[o]<37000)
//              {
//                Difrn = LogData[o-1] - LogData[o];
//                if (Difrn>0)
//                  LogData[o] = LogData[o] + (unsigned short)(Difrn>>1);
//              }
//            }
//            break;
//          case 3:
//            if (LogData[o]>30000)
//            {
//              EnaChange++;
//              Summer +=(unsigned long)LogData[o];
//            }
//            if (EnaChange>4) // можно менять данные
//            {
//              //if (++AddSh > 700) AddSh = 700;
//              AddSh = (unsigned short)((double)Summer/(double)EnaChange); // логарифмическое среднее
//              if (LogData[o]<37000)
//              {
//                LogData[o] = AddSh;
//              }
//            }
//            break;
            if (LogData[o]>27000)
            {
              EnaChange++;
            }
            if (EnaChange>2) // можно менять данные
            {
              if (LogData[o]<24000) AddSh = 700;
              if (++AddSh > 700) AddSh = 700;
              if (LogData[o]<32000)
              {
                LogData[o] = LogData[o] + (unsigned short)(AddSh*3);
              }
            }
          }
          LogData[o] = LogData[o] + (unsigned short)medium;
          
        }
      }
    }
    
  }
  
  //if (Index) LogData[Index]=0;
  return Index;
}

// функция установки уровня DAC на выходе P0.26
//123
//void SetLvlDAC (unsigned int DAC)
//{
//  DACR = (unsigned long)((DAC&0x7)<<9);
//}

// установка заголовка файла передачи необработанных данных
void SetHeadFileRaw (DWORD NAV)
{
  DWORD NumBt = RAWSIZE*sizeof(DWORD) + 24; // число байт в массиве предачи 24 -32 правильный размер 24
  WORD Index;
  WORD siZe = sprintf ((void*)Head_RAW.Head,"%d",NumBt);
  // заголовок
        for (Index=0 ; Index<siZe; Index++)
        {
          Head_RAW.Head[7-Index] =  Head_RAW.Head[siZe-Index-1];
        }
        Head_RAW.Head[7 - Index++] = 0x30 +  siZe;
        Head_RAW.Head[7 - Index++] = '#';
        while (Index < 8) Head_RAW.Head[7 - Index++]=0x20;
// разрешение DS
       //Head_RAW.ValDS = (unsigned int)((ADCPeriod*50000)/(NumPointsPeriod[GetIndexLN()])); //  устанавливаем значения DS для установленного режима измерения
       //Head_RAW.ValDS = (unsigned int)((ADCPeriod*50000)/(NumRepit)); //  устанавливаем значения DS для установленного режима измерения
       Head_RAW.ValDS = (unsigned int)GetValueDS(); //  устанавливаем значения DS для установленного режима измерения
// число накоплений NAV
       Head_RAW.NumAvrg = NAV;
// длина волны источника, длительность импульса зондирования AW PWU
       Index = GetLengthWaveLS (GetPlaceLS(CURRENT)); // получаем длину волны источника
          Head_RAW.AW_PWU = Index<<16; // записываем длину волны источника
          Index  = GetWidthPulse(GetIndexIM());
          Head_RAW.AW_PWU += Index; // записываем длительность импульса
// Формат данных, при прореживании (будет ли здесь не знаю = 0) смещение (начало линии) то что прописано в настройках
          Head_RAW.FormatData = GetCurrentBegShiftZone();
          // размер окна блокировки, сейчас число точек на период 
            Head_RAW.SizeFrm = (NumRepit);
          //число отсчетов NPPW (на выбранный импульс, он у нас один)
              Head_RAW.NumPtsMain = RAWSIZE;//0x1200;
}

// 26.02.2014 
// переход в режим установки параметров рефлектометра из меню или при включении если ТАБЛЕТКА
void SetMODESettingRef (void)
{
     SetIndexLN(GetIndexLN());//устанавливаем текущие установки по длинам и импульсам
     SetMode(ModeSetupOTDR);
     SetIndexWAV(GetWAV_SC(GetPlaceLS(CURRENT))); // устанавливаем коэфф. преломления выбранной длины волны

     GetSetModeLW(-1); // сбрасываем счетчик длин волн источников
     CntLS = 1;
     PosCursorMain (-4100); // сброс курсора в начало
     IndexVerSize  = 0;// установка вертикального размера отображения рефлектограммы ( самый крупный)
     GetSetHorizontScale (5); // сброс масштаба уст самый крупный
     // 25.11.2022 предпроверка при входе в меню установок рефлектометра, 
     //по востановленному индексу длины, не надо ли переключать масштаб
        if ((GetIndexLN()!=0)&&(IndexSmall==0))
        {
          IndexSmall=1;
        }
     if (IndexSmall==0)GetSetHorizontScale (-2);
     // устанавливаем для 2 км 
     ModeDevice = MODESETREFL;
     //123 SSPInit_Any(SPI_ALT); // Инициализация SSP для управления ALTERA (порт 1 та что на плате отладочной)
     
//123     PWM_LMOD_Init (OFF); // выключаем LMOD от PWM 0 - OFF, 1- 270Hz 2-2kHz  

}
// расчет ORL по массиву необработанных данных от измерения в режиме 16км 40 нс (03/05/2017)
float MeasORL(int NumAvrgThis, int EnaReport)
{
#define NumPnoise 40 // число точек для рассчета НУЛЯ
//#define NumAvrgThis 5000 // число точек накопления
  //char BufStr[48];
  double SumNoise = 0.; // "ноль" - смещение
  double SumDatRefl = 0.0; // Сумма данных рефлектограммы 4096 точек
  float Result;
  double CurrDat, MaxNoise=0.;
  double SumBeg = 0, Mnozhitel = 1.0; // выравнивающий множитель
  int NumPereg = 0, pSu = 0; // счетчик перегрузки, число точек суммирования, AllPereg = 0
  LED_START(1);//On  LED
  //myBeep(10);
  SlowON();
  HV_LOW(OFF); //ON HIGH VOLT
  HV_SW(ON); // ON HIGH VOLT
  //POWALT(ON);
  //POWREF (ON);
  //POWDET(ON);
  //123
  //теперь нет в этом необходимости
        if (EnaReport)//типа измеряли из под измерителя
        {
//  SSPInit_Any(SPI_ALT); // Инициализация SSP для управления ALTERA (порт 1 та что на плате отладочной)
//  disable_timer ( 0 );
//  reset_timer(2);
//  enable_timer(2);
          CurrTimeAccum = 0;
          EnaTimerAccum = 1;
        }
  SetIndexIM (1); // 40 ns
  SetIndexLN (3); // 16 km
  PointsPerPeriod = NumPointsPeriod[GetIndexLN()]; // SetPointsPerPeriod( ... );
  PointInPeriod = 0;
  memset( RawData, 0, RAWSIZE * sizeof(DWORD) );
//123  SetNumAccumPerSec (NumAvrgThis);// установка значения числа накоплений
  //CreatDelay (9000000*EnaReport); // 33 мС
  HAL_Delay(900*EnaReport);
  //        ModeDevice = MODEOTHER;
  //
  Averaging(100,0,0);// включение питания и запуск накопления 
   SetNumAccumPerSec (NumAvrgThis);// установка значения числа накоплений
  LED_START(0);//Off  LED
  //        ModeDevice = MODEOTHER;
  //
  //disable_timer ( 0 );
  myBeep(0); // выкл. писк
  //CreatDelay (9000000*EnaReport); // 33 мС
  HAL_Delay(900*EnaReport);
  CntNumAvrg = 0; // обнуляем счетчик накоплений
  memset( RawData, 0, RAWSIZE * sizeof(DWORD) );
//123  reset_timer(2);
//123  enable_timer(2);
  CurrTimeAccum = 0;
  EnaTimerAccum = 1;
  LED_START(0);//On  LED
  Averaging(NumAvrgThis,0,0);// включение питания и запуск накопления c увеличенным интервалом
  //g_UpGrdB, g_kLog, g_OffSetdB)
  //  SetMode(ModeSetOTDRprm);
  //  ModeDevice = MODEOTDRPRM;
  // теперь можно посчитать ORL
  for(int i=0; i<NumPnoise; ++i) 
  {
    SumNoise +=(double)RawData[i];
    if(MaxNoise<(double)RawData[i]) MaxNoise = (double)RawData[i];
  }
  SumNoise = SumNoise/(NumAvrgThis*NumPnoise); // в чистом виде НОЛЬ смещение отнормировано
  MaxNoise = MaxNoise/NumAvrgThis;  // норимированное значение максимального шума  в НУЛЕ
  // расчет МНОЖИТЕЛЯ
  // проверим уровень сигнала после импульса
  SumBeg = (RawData[43]+RawData[44]+RawData[45])/(3.0*NumAvrgThis)-SumNoise;
  if(SumBeg>1.8 && SumBeg<400.)
    Mnozhitel = 10.24/SumBeg;
  else
    Mnozhitel = 1.0;
  // проверим значения и отнормируем их для суммирования
  for(int i=0; i<POINTSIZE; ++i) //4096
  {
    CurrDat = (double)RawData[i+41]/NumAvrgThis; // нормируем показание
    if(CurrDat > 1020) // перегрузка! 4090
    {
      NumPereg++; // посчитаем длительность перегрузки
      //AllPereg++;
      CurrDat = CurrDat*pow(g_UpGrdB,NumPereg)/Mnozhitel; // увеличим уровень (пока так)
    }
    else // нет перегрузки, сбросим счетчик перегрузок
    {
      if(NumPereg )
      {
#if 0
//        {
          sprintf(BufStr,"--p-%d\n", NumPereg);// ORL
  UARTSendExt ((BYTE*)BufStr, strlen (BufStr));
//        }
#endif
    NumPereg=0;
      }
    }
    // условие превышения , если сигнал больше максимального шума НУЛЯ то плюсуем
    if(CurrDat>(MaxNoise))
    {
    SumDatRefl +=CurrDat-SumNoise;
    pSu++;
    }
  }
  if(SumDatRefl<=10) SumDatRefl = 10; // не более 65 дБ
  // увеличиваем сумму на множитель, "выравниваем" значение суммы
  SumDatRefl = SumDatRefl*Mnozhitel ;
  // ограничение на уровне 14.8
  if(SumDatRefl>1047000.)SumDatRefl=1047000.;
  Result = g_OffSetdB - g_kLog*log10(SumDatRefl);
        if (EnaReport)//типа измеряли из под измерителя
        {
          SetIndexLN (0); 
          SetIndexIM (0);
          HV_LOW(ON); //ON LOW HIGH VOLT
          HV_SW(OFF); // OFF HIGH VOLT
          //POWDET(OFF);
          POWREF (OFF);
          POWDET(ON); // включаем питание измерителя
//123          enable_timer(3);  /* Enable Interrupt Timer3 */
//123          enable_timer(1);  /* Enable  Timer1 JDSU */
//123          SSPInit_Any(SPI_PM);      //Перевели SPI на АЦП
//123          SetupSource (GetModeLS()); // востанавливаем режим работы лазера  
          // по следам версии 175, здесь тоже сделаем небольшую задержку
          //CreatDelay (350000); //
  HAL_Delay(35);
          
        }
  // отладочная инфа
#if 0
  //{
  sprintf(BufStr,"O=%.2f\n", Result);// ORL
  UARTSend0 ((BYTE*)BufStr, strlen (BufStr));
  sprintf(BufStr,"N=%.3f\n", SumNoise);// Noise Shift
  UARTSend0 ((BYTE*)BufStr, strlen (BufStr));
  sprintf(BufStr,"M=%.3f\n", MaxNoise);// MaxNoise
  UARTSend0 ((BYTE*)BufStr, strlen (BufStr));
  sprintf(BufStr,"S=%.3f\n", SumDatRefl);// Summa 
  UARTSend0 ((BYTE*)BufStr, strlen (BufStr));
  sprintf(BufStr,"P=%d\n", pSu);// число точек суммирования
  UARTSend0 ((BYTE*)BufStr, strlen (BufStr));
  sprintf(BufStr,"Mn=%.3f\n", Mnozhitel);// множитель
  UARTSend0 ((BYTE*)BufStr, strlen (BufStr));
  sprintf(BufStr,"Sb=%.3f\r", SumBeg);// сумма в начале
  UARTSend0 ((BYTE*)BufStr, strlen (BufStr));
  SubModeMeasOTDR = NOTMODE;
  //}
#endif
  LED_START(0);//Off  LED
  return Result;
}

    
    
//void TetrisGame (void)
//{
//     ClearScreen();
//    OnTimerTetris();
//
//  if ((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))
//  {
//    SetMode(ModeMainMenu);
//    myBeep(10);
//    ModeDevice = MODEMENU;
//  }
//    PaintLCD();
//
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
  HAL_Delay(30);
}

void SlowON (void) // медленное включение питания
{
  // сделал с проверкой уже включенных, для ускорения...
  
  POWREF (ON);
  HAL_Delay (700); // 0.7 С (с этой задержкой вроде работает от USB, без акк)
  POWDET(ON);
  // а так по старому
//  POWALT(ON);
//  CreatDelay (40000); // 3.3 мС
//  POWDET(ON);
//  CreatDelay (40000); // 3.3 мС
//  POWREF (ON);
//  CreatDelay (80000); // 3.3 мС
  
}

#pragma optimize=none

// задержка 83.33 нС
void CreatDelay (unsigned long a) { while (a--!=0); }


//void ArcanoidGame (void)
//{
//     ClearScreen();
//     OnTimerArcanoid();
//  if ((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))
//  {
//    SetMode(ModeMainMenu);
//    myBeep(10);
//    ModeDevice = MODEMENU;
//  }
//    PaintLCD();
//
//}

//void KeyTestGame (void)
//{
//     ClearScreen();
//     OnTimerKeyTest();
//  if ((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==LONG_PRESSED))
//  {
//        POWALT(OFF);
//        POWREF (OFF);
//        POWDET(OFF);
//
//    SetMode(ModeMainMenu);
//    myBeep(10);
//    ModeDevice = MODEMENU;
//  }
//    PaintLCD();
//
//}
