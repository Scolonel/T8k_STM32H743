//#define MAXWIDESMBL 17 // ������������ ����� ������� �������� � ������ 
//#define MAXSMALLSMBL 22
//#define WAITWELCOME 100 // ����� ������ �������� ��������
//#define PLUS 1
//#define MINUS 0
//#define Pic_Tx 67 // X - �������� �������� ��� ������ ��������� �������
//#define Pic_Ty 5 // Y - �������� �������� ��� ������ ��������� �������


#include "main.h"

volatile BYTE g_NeedScr=1; // ����������� ������ ! ���������! ���� ��������� 
volatile BYTE g_FirstScr=1; // ������ ���� � �����, ����� ������ ����������� (����������), ����� �������� ������ ������ ����, ��� NEXTION
volatile BYTE g_NeedChkAnsvNEX=0; // ������� ��������� ������ �� ���������.� �� ��������
 uint16_t Set_MAX_DB; // ���������� �������� ���������� �� � ������� ����� (� SHAG �����) ��� ������ ����������



static volatile unsigned char CurIndLambda = 0; // ��������� �� ����� ����� �����������
static volatile unsigned char FrSetResGrph = 0; // ��������� �� ������ � ���� ������
static volatile unsigned char FrSetResTabl = 1; // ��������� �� ������ � ���� ������
static volatile unsigned char FrSetOnOff = 1; // ��������� �� ������ � ������ ����������
static volatile unsigned char FrSetClear = 1; // ��������� �� ������ � ������ ������� ������
static volatile char CntFolder=0; // ������� ����� ��� ��������

static volatile unsigned char SubModeMem = 0; // �������� ��������� ������
static volatile  unsigned char FrCurrMem = 0; // ������ � ������ ��������� � �������
//static volatile unsigned char ChannelsMode = UserConfig.ChnMod; // ����� ����������� �������
//static char Stroka[20];
//static unsigned char IndexComm=0;
unsigned char SetErrMsg = 0; // ��������� ����� ��������� �� ������ ��������
unsigned CntWelcome = 0; // ������� �������� ��������

//static unsigned char Ptr; // ��������� � ����� ������� �������� (����� ��������������)
static void (*ModeFunc)(void);
static void (*ModeFuncPrev)(void) = 0;

static unsigned char FrdBSet=1; //������� ��������� ����� � �������� �������� ��������� (������)
static unsigned char FrFreeSet=0; //������� ��������� ����� ��������� ������� ����� �������
//static unsigned char FrValSet = 1; //������� ��������� ����� ��������� ������� ������ �������
unsigned char ScrRedraw = 1; //������� ����������� ������

//static char* globalStringToCopy = 0;

//static char AnlzMode = MOD_ANLZ_TAB;

//static unsigned char TabFiles[16];
//��� �������
//void FrBat(unsigned char SW ,  int X, int Size); //???
//void FrBatNew(int Y ,  int X, int Size); //???
float BatProc; //�����. ��� ������ ������ � %


//char GetModeDev (void) // ���������� ������� �����
//{
//  return CurrMode;
//}
// ������� ������������� �������� �� ������ � NEXTION
void CmdInitPage(int Num)
{
  char str[44];
  sprintf(str, "page %d���",Num); // < ������e >
  NEX_Transmit((void*)str);    //
  
  //NEX_Transmit((void*)CmdNextion[Num]);
  g_FirstScr=1;
  //CreatDelay(20000);// 177 ��� � 173
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
void ModeMain(void)// ����� ��������
{
//  static volatile unsigned char FrFreeInd = 0; //��������� �� �������� ������
//  
//  char Str[32];
//  char StrN[32];
//  Set_MAX_DB = (ConfigDevice.PlaceLW[UserSet.iCurrLW]>1300)?(MAX_DB):(MAX_DB/2);
//  
//  //if (PRESS(BTN_OK))
//  if ((PRESS(BTN_OK))&&(getStateButtons(BTN_OK)==SHORT_PRESSED))
//  {
//    myBeep(10);
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
//    g_NeedScr = 1;
//    
//  }
//  // ������ �����
//  if ((PRESS(BTN_UP))&&(getStateButtons(BTN_UP)==SHORT_PRESSED))
//  {
//    myBeep(25);
//    FrFreeInd=(int)(FrFreeInd+3)%4;
//    g_NeedScr = 1;
//    
//  }
//  // ������ ����
//  if ((PRESS(BTN_DOWN))&&(getStateButtons(BTN_DOWN)==SHORT_PRESSED))
//  {
//    myBeep(25);
//    FrFreeInd=(int)(FrFreeInd+1)%4;
//    g_NeedScr = 1;
//    
//  }
//  if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==INF_PRESSED)) // ���� �����
//  {
//    if (FrFreeInd==0)
//    {
//      myBeep(3);
//      DigitSet = 1;
//      if(UserSet.iLvlCurrLW[UserSet.iCurrLW]>DigitSet) UserSet.iLvlCurrLW[UserSet.iCurrLW] -= DigitSet;
//      else UserSet.iLvlCurrLW[UserSet.iCurrLW] = 0;
//      g_NeedScr = 1;
//    }
//    if (FrFreeInd==1 || FrFreeInd==2)
//    {
//      myBeep(3);
//      DigitSet = 1;
//      if(UserSet.iLvlFixLW[UserSet.iCurrLW][FrFreeInd-1]>DigitSet) UserSet.iLvlFixLW[UserSet.iCurrLW][FrFreeInd-1] -= DigitSet;
//      else UserSet.iLvlFixLW[UserSet.iCurrLW][FrFreeInd-1] = 0;
//      g_NeedScr = 1;
//    }   
//  }
//  // ������ ����� , ��������� ��� ����� ������
//  if ((PRESS(BTN_LEFT))&&(getStateButtons(BTN_LEFT)==SHORT_PRESSED))
//  {
//    myBeep(25);
//    if (FrFreeInd==0)
//    { 
//      if(UserSet.iLvlCurrLW[UserSet.iCurrLW]>DigitSet) UserSet.iLvlCurrLW[UserSet.iCurrLW] -= DigitSet;
//      else UserSet.iLvlCurrLW[UserSet.iCurrLW] = 0;
//    }    
//    if (FrFreeInd==1 || FrFreeInd==2)
//    {
//      if(UserSet.iLvlFixLW[UserSet.iCurrLW][FrFreeInd-1]>DigitSet) UserSet.iLvlFixLW[UserSet.iCurrLW][FrFreeInd-1] -= DigitSet;
//      else UserSet.iLvlFixLW[UserSet.iCurrLW][FrFreeInd-1] = 0;
//    }   
//    //UserSet.iLvlCurrLW[UserSet.iCurrLW]/SHAG 
//    if (FrFreeInd==3) // ������������ ����� �����
//    { 
//      if(UserSet.iCurrLW>0) UserSet.iCurrLW--; // ����� ����������� ����� ���� (
//      else UserSet.iCurrLW = 3;
//      if(ConfigDevice.PlaceLW[UserSet.iCurrLW]==0)
//      {
//        if(UserSet.iCurrLW>0) UserSet.iCurrLW--;   // ����� ����������� ����� ����
//        else UserSet.iCurrLW = 3;
//      }
//      if(ConfigDevice.PlaceLW[UserSet.iCurrLW]==0)
//      {
//        if(UserSet.iCurrLW>0) UserSet.iCurrLW--;   // ����� ����������� ����� ����
//        else UserSet.iCurrLW = 3;
//      }
//      if(ConfigDevice.PlaceLW[UserSet.iCurrLW]==0)
//      {
//        if(UserSet.iCurrLW>0) UserSet.iCurrLW--;   // ����� ����������� ����� ����
//        else UserSet.iCurrLW = 3;
//      }
//    }
//    g_NeedScr = 1;
//    
//  }
//  if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==INF_PRESSED)) // ���� �����
//  {
//    if (FrFreeInd==0)
//    { 
//      myBeep(3);
//      DigitSet = 1;
//      if(UserSet.iLvlCurrLW[UserSet.iCurrLW]<=Set_MAX_DB-DigitSet) UserSet.iLvlCurrLW[UserSet.iCurrLW] += DigitSet;
//      else UserSet.iLvlCurrLW[UserSet.iCurrLW] = Set_MAX_DB;
//      g_NeedScr = 1;
//      
//    }    
//    if (FrFreeInd==1 || FrFreeInd==2)
//    {
//      myBeep(3);
//      DigitSet = 1;
//      //UserSet.iLvlFixLW[UserSet.iCurrLW][FrFreeInd-1]
//      if(UserSet.iLvlFixLW[UserSet.iCurrLW][FrFreeInd-1]<=Set_MAX_DB-DigitSet) UserSet.iLvlFixLW[UserSet.iCurrLW][FrFreeInd-1] += DigitSet;
//      else UserSet.iLvlFixLW[UserSet.iCurrLW][FrFreeInd-1] = Set_MAX_DB;
//      g_NeedScr = 1;
//    }   
//    
//  }
//  // ������ ����� , ��������� ��� ����� ������
//  if ((PRESS(BTN_RIGHT))&&(getStateButtons(BTN_RIGHT)==SHORT_PRESSED))
//  {
//    myBeep(25);
//    if (FrFreeInd==0)
//    { 
//      if(UserSet.iLvlCurrLW[UserSet.iCurrLW]<=Set_MAX_DB-DigitSet) UserSet.iLvlCurrLW[UserSet.iCurrLW] += DigitSet;
//      else UserSet.iLvlCurrLW[UserSet.iCurrLW] = Set_MAX_DB;
//    }    
//    if (FrFreeInd==1 || FrFreeInd==2)
//    {
//      //UserSet.iLvlFixLW[UserSet.iCurrLW][FrFreeInd-1]
//      if(UserSet.iLvlFixLW[UserSet.iCurrLW][FrFreeInd-1]<=Set_MAX_DB-DigitSet) UserSet.iLvlFixLW[UserSet.iCurrLW][FrFreeInd-1] += DigitSet;
//      else UserSet.iLvlFixLW[UserSet.iCurrLW][FrFreeInd-1] = Set_MAX_DB;
//    }   
//    //UserSet.iLvlCurrLW[UserSet.iCurrLW]/SHAG 
//    if (FrFreeInd==3) // ������������ ����� �����
//    { 
//      if(UserSet.iCurrLW<3) UserSet.iCurrLW++; // ����� ����������� ����� ���� (
//      else UserSet.iCurrLW = 0;
//      if(ConfigDevice.PlaceLW[UserSet.iCurrLW]==0)
//      {
//        if(UserSet.iCurrLW<3) UserSet.iCurrLW++;   // ����� ����������� ����� ����
//        else UserSet.iCurrLW = 0;
//      }
//      if(ConfigDevice.PlaceLW[UserSet.iCurrLW]==0)
//      {
//        if(UserSet.iCurrLW<3) UserSet.iCurrLW++;   // ����� ����������� ����� ����
//        else UserSet.iCurrLW = 0;
//      }
//      if(ConfigDevice.PlaceLW[UserSet.iCurrLW]==0)
//      {
//        if(UserSet.iCurrLW<3) UserSet.iCurrLW++;   // ����� ����������� ����� ����
//        else UserSet.iCurrLW = 0;
//      }
//    }
//    g_NeedScr = 1;
//    
//  }
//  // ������ ���� , ��������� ��� ����� ������
//  if ((PRESS(BTN_MENU))&&(getStateButtons(BTN_MENU)==SHORT_PRESSED))
//  {
//    if (FrFreeInd==1 || FrFreeInd==2)
//    {
//      myBeep(25);
//      UserSet.iLvlCurrLW[UserSet.iCurrLW] = UserSet.iLvlFixLW[UserSet.iCurrLW][FrFreeInd-1];
//      if(FrFreeInd==1) FrFreeInd=2;
//      else FrFreeInd=1;
//    }
//    g_NeedScr = 1;
//  }
//  
//  // ������� ���� ���� �� ������ � ���� ������� �� ������ 'S'
//  //����� ����
//  if (rawPressKeyS) // key S 
//  {  
//    myBeep(35);
//    
//    UserSet.iLvlCurrLW[UserSet.iCurrLW] = 0;
//    // ���������� �.�. �� 30.04.2025
//    FrFreeInd=0;
//    
//    //    if(UserSet.iCurrLW<3) UserSet.iCurrLW++; // ����� ����������� ����� ���� (
//    //    else UserSet.iCurrLW = 0;
//    //    if(ConfigDevice.PlaceLW[UserSet.iCurrLW]==0)
//    //    {
//    //      if(UserSet.iCurrLW<3) UserSet.iCurrLW++;   // ����� ����������� ����� ����
//    //      else UserSet.iCurrLW = 0;
//    //    }
//    //    if(ConfigDevice.PlaceLW[UserSet.iCurrLW]==0)
//    //    {
//    //      if(UserSet.iCurrLW<3) UserSet.iCurrLW++;   // ����� ����������� ����� ����
//    //      else UserSet.iCurrLW = 0;
//    //    }
//    //    if(ConfigDevice.PlaceLW[UserSet.iCurrLW]==0)
//    //    {
//    //      if(UserSet.iCurrLW<3) UserSet.iCurrLW++;   // ����� ����������� ����� ����
//    //      else UserSet.iCurrLW = 0;
//    //    }
//    // NeedSaveParam |=0x02;// Memory UserSet 
//    
//    rawPressKeyS = 0;  
//    g_NeedScr = 1;
//  }
//  
//  if(g_FirstScr)
//  {
//    
//    // ������������� �� ���������� �������
//    // ������������ �������
//    
//    sprintf(StrN,"%.2f dB",UserSet.iCurrLW/SHAG);
//    sprintf (Str,"t0.txt=\"%s\"���",StrN); // 
//    NEX_Transmit((void*)(void*)Str);//
//    
//    sprintf (Str,"t1.txt=\"%s\"���",MsgMass[31][UserSet.CurrLang]); //Memory 1
//    NEX_Transmit((void*)Str);//
//    // 
//    sprintf (Str,"t2.txt=\"%s\"���",MsgMass[32][UserSet.CurrLang]); // Memory 2
//    NEX_Transmit((void*)Str);//
//    // 
//    sprintf (Str,"t3.txt=\"%s\"���",MsgMass[30][UserSet.CurrLang]); // ����� �����
//    NEX_Transmit((void*)Str);//
//    // 
//    g_FirstScr = 0;
//    g_NeedScr = 1;
//  }
//  if(g_NeedScr) // ����������� ���������� ��� ���������� � �������������
//  {
//    // ������������� ���� ������ 
//    // �������� ����������  � ��������� ���������
//    sprintf(Str, "t0.bco=WHITE���"); // �����
//    NEX_Transmit((void*)Str);//
//    sprintf(Str, "t1.bco=WHITE���"); // �����
//    NEX_Transmit((void*)Str);// 
//    sprintf(Str, "t2.bco=WHITE���"); // �����
//    NEX_Transmit((void*)Str);// 
//    sprintf(Str, "t3.bco=WHITE���"); // �����
//    NEX_Transmit((void*)Str);// 
//    // sprintf(Str, "t%d.bco=GREEN���", (FrFreeInd)?(FrFreeInd+3):(0)); // �������
//    sprintf(Str, "t%d.bco=GREEN���", (FrFreeInd)); // �������
//    NEX_Transmit((void*)Str);// 
//    // ��� ��������� ��������� ������ ���� ���� ���� ������ ������
//    
//    // �������� ������ 1
//    sprintf (Str,"t4.txt=\"%.2f dB\"���",UserSet.iLvlFixLW[UserSet.iCurrLW][0]/SHAG); // 
//    NEX_Transmit((void*)Str);//
//    // �������� ������ 2
//    sprintf (Str,"t5.txt=\"%.2f dB\"���",UserSet.iLvlFixLW[UserSet.iCurrLW][1]/SHAG); // 
//    NEX_Transmit((void*)Str);//
//    // ����� ����� �������
//    sprintf (Str,"t6.txt=\"%d nm\"���",ConfigDevice.PlaceLW[UserSet.iCurrLW]); // 
//    NEX_Transmit((void*)Str);//
//    // ��������� ����� ����� 
//    //if(ConfigDevice.PlaceLW[UserSet.iCurrLW]>1300)
//    sprintf (Str,"t7.txt=\"%s\"���",(ConfigDevice.PlaceLW[UserSet.iCurrLW]>1300)?("SM"):("MM")); // 
//    NEX_Transmit((void*)Str);//
//    // ������ �������� ���� ���������
//    
//    if(ModeWork) // ���������
//    {
//      sprintf(Str,"t8.txt=\"!SET!\"���");
//      NEX_Transmit((void*)Str);//
//      sprintf (Str,"t0.txt=\"%04d\"���",CurrLevelDAC); //  ������� ��������
//      NEX_Transmit((void*)Str);//
//    }
//    else
//    {
//      if(DigitSet<SHAG)
//        sprintf(Str,"t8.txt=\"%2.2f\"���",DigitSet/SHAG);
//      else
//        sprintf(Str,"t8.txt=\"%2.1f\"���",DigitSet/SHAG);
//      NEX_Transmit((void*)Str);//
//      sprintf (Str,"t0.txt=\"%.2f dB\"���",UserSet.iLvlCurrLW[UserSet.iCurrLW]/SHAG); //  ������� ��������
//      NEX_Transmit((void*)Str);//
//      CurrLevelDAC = CoeffLW.SetCoefLW[UserSet.iCurrLW][UserSet.iLvlCurrLW[UserSet.iCurrLW]];
//    }
//    NEX_Transmit((void*)Str);//
//    
//    NeedSaveParam |=0x02;// Memory UserSet 
//    
//    g_NeedScr =  0;
//  }
//  
}

void ModeWelcome(void)// ����� ��������
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
  // ������� ���� ��� ���������� ���� LCD �� ��������
  // ����������� ���� ��� NEXTION
  if(g_FirstScr)
  {
    
    // ������������� �� ���������� �������
    // ������������ �������
    int  Mdl=0; // ���������� ������ �� ������������ ����� ����
//    for (int y=0;y<4;++y)
//    {
//      if((ConfigDevice.PlaceLW[y]>0)&&(ConfigDevice.PlaceLW[y]<1310))
//        Mdl|=1;
//      if(ConfigDevice.PlaceLW[y]>1300)
//        Mdl|=2;
//    }
    
    sprintf(StrN,"%s-%d",DeviceIDN[ConfigDevice.ID_Device],Mdl);
    sprintf (Str,"t0.txt=\"%s\"���",StrN); // 
    NEX_Transmit((void*)(void*)Str);//
    // �������������
    if (GetID_Dev())
    {
      sprintf (Str,"t1.txt=\"%s\"���",MsgMass[12][UserSet.CurrLang+2]); // ����
    }
    else
    {
      sprintf (Str,"t1.txt=\"%s\"���",MsgMass[12][UserSet.CurrLang]); // ��
    }
    NEX_Transmit((void*)Str);//
    // Date
    sprintf (Str,"t2.txt=\"%s\"���",MsgMass[13][UserSet.CurrLang]); // Date
    NEX_Transmit((void*)Str);//
    // Time
    sprintf (Str,"t4.txt=\"%s\"���",MsgMass[15][UserSet.CurrLang]); // time
    NEX_Transmit((void*)Str);//
    // Battery
    sprintf (Str,"t6.txt=\"%s\"���",MsgMass[16][UserSet.CurrLang]); // Battery
    NEX_Transmit((void*)Str);//
    // Version
    // ������ ��
    sprintf (St, "v3.%02d%c", NUM_VER/26, (0x61+(NUM_VER%26))); // ������ ������ ���� �� ���� 2.01 (28.06.2022)
    sprintf (Str,"t8.txt=\"%s(%X)\"���",St,CheckErrMEM); // Version
    NEX_Transmit((void*)Str);//
    // Number
    sprintf (Str,"t9.txt=\"�%d\"���",GetNumDevice()); // Number
    NEX_Transmit((void*)Str);//
    
  }
  // �������� ��������� ������� ��� ��������� (����� � �����������)
  // time
  if(SecOld != current_time.RTC_Sec)
  {
    //����������� �������
    SecOld = current_time.RTC_Sec ; 
    sprintf(Str,"t5.txt=\"%02d:%02d:%02d\"���",current_time.RTC_Hour,current_time.RTC_Min,current_time.RTC_Sec);
    NEX_Transmit((void*)Str);//
    // ����������� ����
    sprintf (Str,"t3.txt=\"%02d.%02d.%02d\"���",current_time.RTC_Mday,current_time.RTC_Mon,current_time.RTC_Year);//,MsgMass[14][CurrLang] ����� ��� 25.12.2013
    NEX_Transmit((void*)Str);//
    // ������� � ��� ������
    if (GETEXTPWR == 0)
    {
      sprintf(Str,"t7.txt=\"%s\"���",MsgMass[11][CurrLang]);// ������� �������
    }
    else
      // �� �������� LvlBatInd 
    {
      sprintf(Str,"t7.txt=\"%d%%\"���", (int)(LvlBatInd*12.5));
    }
    NEX_Transmit((void*)Str);// 
  }
  g_FirstScr=0; // ��������� ������� ������� �������
  
  if ((HAL_GetTick() - TimeBegin) > 4000) //4s
    //if (CntWelcome > WAITWELCOME)
  {
    SetMode(ModeMain);
    CmdInitPage(3);// ������� ������� ������������ ���� �� MainMenu � ��������� �������� ������� �����
    
    myBeep(10);
    //ModeDevice = MODEMENU;
    //SetHeadFileRaw (0); // ����� ��������� ����� ������ ������������� ������
    
    //ADCData++;
    //ClearScreen();
  }   
  
}

void BadBattery(void) // ������ ���������� CHECK_OFF
{ 
  char Str[32];
  if(g_FirstScr)
  {
    
    // ������������� �� ���������� �������
    // ������������ �������
    
    sprintf (Str,"t0.txt=\"%s\"���",MsgMass[47][UserSet.CurrLang]); // 
    NEX_Transmit((void*)(void*)Str);//
    
    sprintf (Str,"t1.txt=\"%s\"���",MsgMass[48][UserSet.CurrLang]); //
    NEX_Transmit((void*)Str);//
    // 
    sprintf (Str,"t2.txt=\"%s\"���",MsgMass[51][UserSet.CurrLang]); // 
    NEX_Transmit((void*)Str);//
    // 
    sprintf (Str,"t3.txt=\"%s\"���",MsgMass[52][UserSet.CurrLang]); //
    NEX_Transmit((void*)Str);//
    // 
    g_FirstScr = 0;
    //g_NeedScr = 1;
  }
  
  
  if(GETEXTPWR == 0) // ���������� ������� ������� ������������ � ���������� �����
  {
    BadLevelBat = 0;
    SetMode(ModeMain);
    CmdInitPage(3);// ������� ������� ������������ ���� �� MainMenu � ��������� �������� ������� �����
    myBeep(100);
    
  }
  
//  while((LevelBat < 60)&&(EXT_POW)) // ���� ���������� ������� ������� �� ������������  � ���� �����
//  {
//    if (CntOff++%60 == 0) myBeep(3);
//    // ������ ��������� �������
//    LevelBat = CheckLevelBattery();
//    // ProcBatInd
//    //sprintf(F_name,"BAT = %d%%", LevelBat );
//    sprintf (F_name,"t3.txt=\"BAT = %d%%(%1.2f)\"���",ProcBatInd,Ubat); // 
//    NEX_Transmit((void*)F_name);//
//    
//  }
//  CmdInitPage(1);// MainMenu
//  myBeep(25);
  
}

// ������������ � ����� ���������������� ���������� (���� �� �����  � ������
void UploadFW_Nextion(void) // ���������� ���������� NEXTION
{ 
  char Str[32];
  // ����� �������� ��� ������ ���������
  if (g_FirstScr)
  {
    // ����� ��������� ������� ���� ������ ����������
    // �� �������� ��������� ��� ��������� �������������
    sprintf(Str, "t0.txt=\"%s\"���", MsgMass[57][CurrLang]);
    NEX_Transmit((void*)Str);    // 
    
    sprintf(Str, "t1.txt=\"%s\"���", MsgMass[58][CurrLang]);
    NEX_Transmit((void*)Str);    // 
    
    //CreatDelay(500000);// ���� �������
    HAL_Delay(50);
    ProgFW_LCD = 1; // ���������� ����� ������ UART ������ ����� ����� ��� �������
    
    g_FirstScr = 0;
    g_NeedScr = 0;
  }
  
  if(ProgFW_LCD==2) // ���� ��������� ������� ���������������� �� ������������  � ���� �����
  {
    //123 enable_timer( 0 );
    GetSysTick(1); // ��������� ����� 10 ��. 0 - ��������� �������� �� ����������� ������ 1- �����
    ProgFW_LCD=0;  
    // ������� ������� ������������ ���� �� MainMenu (�������)  
    // ������� ����� ����!
    //CreatDelay(5000000);
    //SetModeDevice (MODEMENU); // �������������� ��������� ������ �������
    // ������ ������..
    CmdInitPage(0);
    SetMode(ModeWelcome);
    //CmdInitPage(0);
    HAL_Delay(500);// ��������� ����� ������, ����� �� �������!
    TimeBegin = HAL_GetTick();
    myBeep(125);
  }
  
}


//int SF_BatLvl(void) //���� �� ��, �� ���������� ������, ���� ���, �� �����������. �-� ���������� 0 ��� 1, ������� int
//{
// // char lvl[5]; //���� ��������� % ������ �� BatProc
//
//  char str1[20];
//  char str2[20];
//  char str3[10];
//  char str4[20];  
//  char str5[10];
//  float BatProc; //�����. ��� ������ ������ � %
//  
//  ClearScreen();
//  udelay(2000000); //�.�. ����� ������� ��� ����� ��������� ������ ����� �������
//  BatProc = CheckBattery(0); //����� ��������, ������ ������
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
////  char level[15]; //����������� �����, ����� ������� ������� ��� ���������� ����������
////  for (int i = 0;i<15;i++)
////  {
////  BatLvl[i] = CheckBattery(1);
////  udelay(100000); //������� ������ 0.1 ���
////  sprintf(level,"%.1f", BatLvl[i]); //�������� � ������
////  putString(i%3*40,i/3*10,level,1,0); //������� ��������
////  PaintLCD(); //������� �� �����
////  }
////  udelay(5000000); //������� �� �� ��� 5 ��� � ���������� ������
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
//  //����� ������������ ����� � ��� ��������� 
//      
//  ClearScreen();
//    RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
//    RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
//      int Mdl=0; // ���������� ������ �� ������������ ����� ����
//                for (int y=0;y<4;++y)
//          {
//            if((DataSetting.Lamda[y]>0)&&(DataSetting.Lamda[y]<1310))
//              Mdl|=1;
//            if(DataSetting.Lamda[y]>1300)
//              Mdl|=2;
//          }
//    
//    sprintf(tt,"%s-%d",MsgMass[6][UserSet.CurrLang],Mdl);//[MemDeviceConfig.Manufacture+MemUserConfig.Language]);
//    putString(((MAXWIDESMBL-strlen(tt))/2)*8,0,tt,1,1); // �������� �������
//    //putString(10,0,tt,1,1);
//    sprintf(tt,"%s",MsgMass[12][UserSet.CurrLang]);
//    putString(((MAXSMALLSMBL-strlen(tt))/2)*5,15,tt,1,0); // ����������� �����
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
//    sprintf(tt,"� %04d", MemDeviceConfig.SerNum);
//    putString(80,45,tt,1,1);
//    sprintf(tt,"%s(%X)", MemDeviceConfig.VerSW, CheckErr);
//    putString(0,54,tt,1,0);
//    PaintLCD();
//
//   // udelay(10000);
////���������� ������
//     if (KeyEvent==bOK) 
//  {
//    KeyEvent=0; 
//    SetMode(SF_SettingDev);
//  }
//  
//    if (CntWelcome > WAITWELCOME) //��� � ��������� ��������� � ���� �5000
//  {
//    SetMode(SF_MainT5xxx);
//    CntWelcome = 0;
//    KeyEvent = 0;
//  }
//  
//}
//
//void SF_MainT5xxx(void) // ���������� � ���������� ��������� ����
//{
//  static volatile unsigned char FrFreeInd = 0; //��������� �� �������� ������
//  char Str[22];
//  //char Str1[22];
//  //char StrW[3]; // type fiber MM SM
//  Set_MAX_DB = (DataSetting.Lamda[iCurrLW]>1300)?(MAX_DB):(MAX_DB/2);
////  ClearScreen();
////  FrFree(FrFreeInd,  0, 63);
////  FrVal(1+iFixLvl,  64, 63);
////  FrdB(0, 16, 63); //����� ������, ���������� �, ������ ����� �� �
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
//////    if(dBled<=500) dBled +=100; //���������� 250
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
//    //����� ����
//    if (FrFreeInd==3)
//    {        
//     DataSetting.fCurrdB[iCurrLW] = dBled;
//     // �� ������
//     //while(
//     if(iCurrLW>0) iCurrLW--;   // ����� ����������� ����� ����
//     else iCurrLW = 3;
//     if(DataSetting.Lamda[iCurrLW]==0)
//     {
//     if(iCurrLW>0) iCurrLW--;   // ����� ����������� ����� ����
//     else iCurrLW = 3;
//     }
//     if(DataSetting.Lamda[iCurrLW]==0)
//     {
//     if(iCurrLW>0) iCurrLW--;   // ����� ����������� ����� ����
//     else iCurrLW = 3;
//     }
//     if(DataSetting.Lamda[iCurrLW]==0)
//     {
//     if(iCurrLW>0) iCurrLW--;   // ����� ����������� ����� ����
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
//  if ((KeyEvent==(bLEFT+LONGP)) || (KeyEvent==(bLEFT+REPEAT))) //�������� �� 1
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
//    //����� ����
//    if (FrFreeInd==3)
//    {   
//     DataSetting.fCurrdB[iCurrLW] = dBled; // ���������� ��������� ��� ������ ����� �����
//     if(iCurrLW<3) iCurrLW++; // ����� ����������� ����� ���� (
//     else iCurrLW = 0;
//     if(DataSetting.Lamda[iCurrLW]==0)
//     {
//     if(iCurrLW<3) iCurrLW++;   // ����� ����������� ����� ����
//     else iCurrLW = 0;
//     }
//     if(DataSetting.Lamda[iCurrLW]==0)
//     {
//     if(iCurrLW<3) iCurrLW++;   // ����� ����������� ����� ����
//     else iCurrLW = 0;
//     }
//     if(DataSetting.Lamda[iCurrLW]==0)
//     {
//     if(iCurrLW<3) iCurrLW++;   // ����� ����������� ����� ����
//     else iCurrLW = 0;
//     }
//     
//     DataSetting.CurrPlaceLW = iCurrLW;
//     dBled = DataSetting.fCurrdB[iCurrLW]; // �������������� ��������� ��� ������ ����� �����
//    }
//    //CodeDAC +=1;
//    //CodeDAC &=0x0fff;    
//    ScrRedraw = 1;
//  }
//  //  if (KeyEvent==bS) //��������� ��� ���������
//  //    {
//  //      KeyEvent=0;
//  //      dBled = 0;
//  //    }
//  
//  
//  if ((KeyEvent==(bRIGHT+REPEAT)) || (KeyEvent==(bRIGHT+LONGP))) //�������� �� 1
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
//  if (KeyEvent==(bS+LONGP)) // ����� ������� ������ ����������
//  {
//    KeyEvent=0;
//    FrSetOnOff = 1;
//    SetMode(SF_OnOffMode);
//    ScrRedraw = 1;
//  }
//  
//  if (KeyEvent==bS) //����� � "0" ���������, ��� ������ � ������
//  {
//    KeyEvent=0;
//    FrdBSet = 1;
//    // �������� ��� ��� ���� ������������ �� ��������� �� ����� ���� � ������ �������
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
//  if (KeyEvent==bMENU+LONGP) //������� � ������ �������� �������� ������ ���� �����
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
//  if (KeyEvent==bMENU) //������������ ���������� ����� ������������� ��������
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
//  // ������ ���������
//  //sprintf(Str,"%.0f%s",BatProc,"%");
//  //putString(2,5,Str,1,0);
//  // ������ �������� ���� ���������
//  if(DigitSet<SHAG)
//  sprintf(Str,"%2.2f",DigitSet/SHAG);
//  else
//  sprintf(Str,"%2.1f",DigitSet/SHAG);
//  putString(104,5,Str,1,0);  
//  
//  // �������� ������� �������� ��������� ��� �������� ����� ����� ��� ����������
//  DataSetting.fCurrdB[iCurrLW] = dBled;
//  // ��������� �����
//  
//  //putString(((MAXWIDESMBL-strlen(Str))/2)*8,13,Str,1,1); // �������� � ��
//  // ����� �����������
//  //FrFree(2 ,  64*iFixLvl, 63);
//  //�������� ������������� �������
//  sprintf(Str,"%s",MsgMass[31][UserSet.CurrLang]);
//  putString(3,20,Str,1,0);  
//  sprintf(Str,"%2.2f %s",DataSetting.fFixdB[0][iCurrLW]/SHAG, MsgMass[49][UserSet.CurrLang]);
//  putString(75,20,Str,1,0); // �������� dB 1-fix
//  
//  sprintf(Str,"%s",MsgMass[32][UserSet.CurrLang]);
//  putString(3,32,Str,1,0); 
//  sprintf(Str,"%2.2f %s",DataSetting.fFixdB[1][iCurrLW]/SHAG, MsgMass[49][UserSet.CurrLang]);
//  putString(75,32,Str,1,0); // �������� dB 2-fix
//  
// // ���������� ����� �����
//  sprintf(Str,"%s",MsgMass[30][UserSet.CurrLang]); // ������� ����� �����
//  putString(3,44,Str,1,0); 
//
//  sprintf(Str,"%d%s %s",DataSetting.Lamda[iCurrLW], MsgMass[38][UserSet.CurrLang],(DataSetting.Lamda[iCurrLW]>1300)?("SM"):("MM"));
//  putString(75,44,Str,1,0); // �������� ����� �����
//  ScrRedraw = 0;
// // ������ �������� ��
//  sprintf(Str,"%2.2f %s",dBled/SHAG, MsgMass[49][UserSet.CurrLang]);// �������� � ��
// 
//  // ������ ����� �A� �� �� ���� ������
//  if(UserConfig.ChnMod)
//  {    
//    sprintf(Str,"%d",CodeDAC);// �������� ����� �����������
//
//    putString(((MAXWIDESMBL-strlen(Str))/2)*8+4,3,Str,0,1); // �������� � ��
//    sprintf(Str,"!SET!");
//    putString(0,54,Str,0,0); // ����� ���������!
//  }
//  else
//  {
//    putString(((MAXWIDESMBL-strlen(Str))/2)*8-1,3,Str,1,1); // �������� � ��
//    // ����� ������ � ������ ������( ����� �������� - ����� 4095 ��� 80 �� � ��� ������ ������� ��� ���������� � ��������� � ������� ������� ����)
//    // dBled - ��������� �� ������ � ������� �������� ���
//    // ��������� ���
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
//  //putString(((MAXWIDESMBL-strlen(Str))/2)*8,54,Str,1,0); // ��������� ���� ��� � ����������
//  PaintLCD();
//  }
//}
//
//
//
//
//void SF_ResultTable(void) // ���������� � ���������� ���������� ����������� �����������
//{
//}
//
//
//void SF_ResultGraph(void) // ���������� � ���������� ������������ ����������� �����������
//{
//}
//
//void SF_OnOffModeOld (void) // ����� ��������� ������� (������ �����???)
//{
//  
//  char Str[22];
//  
//  ClearScreen();
//  if (KeyEvent==bDOWN)
//  {
//    KeyEvent=0;
//    FrSetOnOff = ChangeFrSet (FrSetOnOff, 1, 0, MINUS);// ��������� ������� � ������ �������� ����������
//  }
//  if (KeyEvent==bUP)
//  {
//    KeyEvent=0;
//    FrSetOnOff = ChangeFrSet (FrSetOnOff, 1, 0, PLUS);// ��������� ������� � ������ �������� ����������
//  }
//  if (KeyEvent==bMENU)
//  {
//    KeyEvent=0;
//    SetMode(0); // ������� � ���������� �����
//  }
//  if (KeyEvent==bOK)
//  {
//    ReWrDataUser (1);// ������ ���������� �� ����� ��������� � ������
//    UserCfgWrite ();
//    KeyEvent=0;
//    if (!FrSetOnOff) // ����. �������
//    {
// DeviceAllOff (); // ���������� �������
//    }
//    else // ������
//    {
//      SetMode(0); // ������� � ���������� �����
//    }
//  }
//  sprintf(Str,"%s",MsgMass[19][UserSet.CurrLang]); //����. ������
//  putString(2,6,Str,1,0);
//  sprintf(Str,"%s",MsgMass[20][UserSet.CurrLang]); // ������
//  putString(2,18,Str,1,0);
//  FrBig(FrSetOnOff,10); // ���������� ������������ ����� ��� ������
//  PaintLCD();
//}
//
//void SF_OnOffMode (void) // ����� ��������� ������� (������ �����???)
//{
//  
//    ClearScreen();
//    UserConfig.ChnMod = 0;
//    ReWrDataUser (1);// ������ ���������� �� ����� ��������� � ������
//    UserCfgWrite ();
//    ReWrDataSett (1);// ������ ���������� �� ����� ��������� � ������ ���������
//    SettingCfgWrite (); // ����� � EEPROM 
//    KeyEvent=0;
//    DeviceAllOff (); // ���������� �������
//
//}
//
//void SF_ClearMem (void) // ����� ������� ������ ������� ()
//{  
//}
//
//void DeviceAllOff (void)// ���������� ������� (���� �������) ��� ����������
//{
//        char Str[22];
//
//      ClearScreen(); // ������� �����
//      sprintf(Str,"%s",MsgMass[26][UserSet.CurrLang]); //���. ������
//      putString(((MAXSMALLSMBL-strlen(Str))/2)*5,15,Str,1,0);
//      sprintf(Str,"%s",MsgMass[27][UserSet.CurrLang]); // �����
//      putString(((MAXSMALLSMBL-strlen(Str))/2)*5,30,Str,1,0);
//      PaintLCD();
//      //StopMode_Measure();
//      CntWelcome =0;
//      SetMode(SF_StartScreen);
//      vDeepSleep();     // ����� ������ ���� ��� ��������� � ���������� �������
//}
//
//
//void SF_MemoryMain (void) // ���� ������ � �������
//{
//}
//
//void SF_MainSetting (void) // ���� ������ ��������� (������, ����/�����, ����)
//{
//}
//
//void SF_SettingDev (void) // ���� ��������� ������� (������, ��������, ��������)
//{
// 
//  static volatile unsigned char FrSetSetting = 0; // ��������� �� ������
//  char Str[22];
//  
//  ClearScreen();
////  if (KeyEvent==(bS+LONGP)) // ����� ������� ������ ����������
////  {
////    KeyEvent=0;
////    FrSetOnOff = 1;
////    SetMode(SF_OnOffMode);
////  }
//  if (KeyEvent==bMENU)
//  {
//    KeyEvent=0;
//    ReWrDataUser (1);// ������ ���������� ������� � ������ 
////    memcpy ((void*)DeviceConfig.Model,(void*)MsgMass[6][UserSet.CurrLang],13); //���������� ������� �������� ����� �������
//    
//    SetMode(0); // ������� � ���������� ����
//  }
////  if (KeyEvent==(bMENU+LONGP))
////  {
////    KeyEvent=0;
////    ReWrDataUser (1);// ������ ���������� ������� � ������  
//////    memcpy ((void*)DeviceConfig.Model,(void*)MsgMass[6][UserSet.CurrLang],13); //���������� ������� �������� ����� �������
////    
////    SetMode(SF_MainMenu); // ������� � �������� ����
////  }
////  if (KeyEvent==bDOWN)
////  {
////    KeyEvent=0;
////    FrSetSetting = ChangeFrSet (FrSetSetting, 3, 0, PLUS);// ��������� ������� � ������ �������� ����������
////  }
////  if (KeyEvent==bUP)
////  {
////    KeyEvent=0;
////    FrSetSetting = ChangeFrSet (FrSetSetting, 3, 0, MINUS);// ��������� ������� � ������ �������� ����������
////  }
//  if (KeyEvent==bRIGHT + REPEAT)
//  {
//    KeyEvent=0;
//    UserConfig.Contrast = ChangeFrSet (UserConfig.Contrast, 100, 0, PLUS);// ��������� ������� � ������ �������� ����������
//    LDC_Set_Contr(UserConfig.Contrast); // ��������� ������� �������������
//  }
//  if (KeyEvent==bRIGHT)
//  {
//    KeyEvent=0;
////    switch(FrSetSetting)
////    {
////    case 0:
////      UserConfig.ChnMod = ChangeFrSet (UserConfig.ChnMod, 2, 0, PLUS);// ��������� ������� � ������ �������� ����������
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
////      UserConfig.AutoOffDev = ChangeFrSet (UserConfig.AutoOffDev, 1, 0, PLUS);// ��������� ������� � ������ �������� ����������
////      break;
////    case 2: // �������������
//      UserConfig.Contrast = ChangeFrSet (UserConfig.Contrast, 100, 0, PLUS);// ��������� ������� � ������ �������� ����������
//      LDC_Set_Contr(UserConfig.Contrast); // ��������� ������� �������������
////      break;
////    case 3: // ����� ����� (Language)
////      UserConfig.Language = ChangeFrSet (UserConfig.Language, 1, 0, PLUS);// ��������� ������� � ������ �������� ����������
////    memcpy ((void*)DeviceConfig.Model,(void*)MsgMass[6][UserSet.CurrLang],13); //���������� ������� �������� ����� �������
////        NeedSaveCfg = 1; 
////      break;
////    }
//  }
//  if (KeyEvent==bLEFT + REPEAT)
//  {
//    KeyEvent=0;
//    UserConfig.Contrast = ChangeFrSet (UserConfig.Contrast, 100, 0, MINUS);// ��������� ������� � ������ �������� ����������
//    LDC_Set_Contr(UserConfig.Contrast); // ��������� ������� �������������
//  }
//  if (KeyEvent==bLEFT)
//  {
//    KeyEvent=0;
////    switch(FrSetSetting)
////    {
////    case 0:
////      UserConfig.ChnMod = ChangeFrSet (UserConfig.ChnMod, 2, 0, MINUS);// ��������� ������� � ������ �������� ����������
////      break;
////    case 1:
////      UserConfig.AutoOffDev = ChangeFrSet (UserConfig.AutoOffDev, 1, 0, MINUS);// ��������� ������� � ������ �������� ����������
////      break;
////    case 2: // �������������
//      UserConfig.Contrast = ChangeFrSet (UserConfig.Contrast, 100, 0, MINUS);// ��������� ������� � ������ �������� ����������
//      LDC_Set_Contr(UserConfig.Contrast); // ��������� ������� �������������
////      break;
////    case 3: // ����� ����� (Language)
////      UserConfig.Language = ChangeFrSet (UserConfig.Language, 1, 0, MINUS);// ��������� ������� � ������ �������� ����������
////    memcpy ((void*)DeviceConfig.Model,(void*)MsgMass[6][UserSet.CurrLang],13); //���������� ������� �������� ����� �������
////            NeedSaveCfg = 1; 
////
////      break;
////    }
//  }
////  
////  
////  sprintf(Str,"%s",MsgMass[21][UserSet.CurrLang]); //������
////  putString(2,6,Str,1,0);
////  //ChMd
////  sprintf(Str,"%s",ChMd[UserConfig.ChnMod]); //������
////  putString(80,6,Str,1,0);
////  sprintf(Str,"%s",MsgMass[22][UserSet.CurrLang]); // ��������������
////  putString(2,18,Str,1,0);
////  sprintf(Str,"%s",(UserConfig.AutoOffDev)?(MsgMass[23][UserSet.CurrLang]):(MsgMass[24][UserSet.CurrLang])); // ���/��
////  putString(80,18,Str,1,0);
//  sprintf(Str,"%s",MsgMass[25][UserSet.CurrLang]); // �������������
//  putString(2,6,Str,1,0);
//  sprintf(Str,"%d",UserConfig.Contrast); // �������������
//  putString(80,6,Str,1,0);
////  sprintf(Str,"Language"); // ����
////  putString(2,42,Str,1,0);
////  sprintf(Str,"%s",MsgMass[0][UserSet.CurrLang]); // ����
////  putString(80,42,Str,1,0);
////  
//  FrBig(FrSetSetting,10); // ���������� ������������ ����� ��� ������
//  FrSmall(FrSetSetting,10); // ���������� ������������ ����� ��� ������
////  
////  
//  PaintLCD();
//}
//
//void SF_SettingDaTime (void) // ���� ��������� ���� � �������
//{
//
//}
//
//void SF_SettingFile (void) // ���� ��������� ����� ()
//{
//}
//
//
//
//void SF_ErrMsg (void) // ��������� ����������� ������ ������� 
//{
//    //static volatile unsigned char SetErrMsg = 0; // ��������� �� ������
//  char Str[22];
//  
//  ClearScreen();
//  if (KeyEvent==(bS+LONGP)) // ����� ������� ������ ����������
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
//    SetMode(SF_MainT5xxx); // ������� � �������� ����
//    //SetMode(0); // ��=������ � ���������� ���� �� �������� ��� ��� ����� ���� ������� ��������� ��� ��������...
//    ScrRedraw = 1;
//
//  }
//  if (KeyEvent==(bMENU+LONGP))
//  {
//    KeyEvent=0;
//    SetErrMsg = 0;
//    SetMode(SF_MainT5xxx); // ������� � �������� ����
//    ScrRedraw = 1;
//
//  }
//  switch (SetErrMsg)
//  {
//  case 1:
//  sprintf(Str,"%s",MsgMass[40][UserSet.CurrLang]); // SD ����� �� �������
//      putString(((MAXSMALLSMBL-strlen(Str))/2)*5,6,Str,1,0);
//  sprintf(Str,"%s",MsgMass[41][UserSet.CurrLang]); // ��� ������ �� ������
//      putString(((MAXSMALLSMBL-strlen(Str))/2)*5,18,Str,1,0);
//    break;
//  case 2:
//  sprintf(Str,"%s",MsgMass[47][UserSet.CurrLang]); // �������
//      putString(((MAXSMALLSMBL-strlen(Str))/2)*5,6,Str,1,0);
//  sprintf(Str,"%s",MsgMass[48][UserSet.CurrLang]); // ���������
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
//void KeybCntrl (void) // ������������� ��������� � ����������
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
//// ���������� ���������� � ������������� �������� �� ������
//void Draw_KeyBoard(unsigned char Cursor, char* Str_first, char* Str_edit, unsigned char Shift ,unsigned char Lang)
//{
//  char CommScr[22] = "                     \0"; // ������ ��������� ��� ������ �� �����
//  unsigned char i;
//  ClearScreen();
//  //Str_edit[ARRAY_SIZE(Str_edit)-1]=0; // ��������� ������� � ������� ����� 0
//  strcpy(CommScr,Str_edit);
//  if (CommScr[Cursor+1] != 0) // �� ����� ������
//  {
//    CommScr[Cursor]=0x5F; // ������ �� ��������� �����
//  }
//  else
//  {   
//  if (CommScr[Cursor] == ' ')CommScr[Cursor]=0x5F; // ������ �� ��������� �����
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
//// ������� ����� �� 66
//void FrBig(unsigned char SW,  char Add)
//{
//    //  DrawLine(0,0+SW*12,76,0+SW*12,12,0,screen); // ������� ���� ,��������
//    // DrawLine(80,0+SW*12,124,0+SW*12,12,0,screen);
//    if (((66+Add)>127)||((66+Add)<0)) Add = 0;
//
//    DrawLine(0,4+SW*12,66+Add,4+SW*12,1,1);
//    DrawLine(0,4+SW*12,0,15+SW*12,1,1);
//    DrawLine(0,15+SW*12,66+Add,15+SW*12,1,1);
//    DrawLine(66+Add,4+SW*12,66+Add,15+SW*12,1,1);
//}
//
//// ��������� �����
//void FrSmall(unsigned char SW ,  char Add)
//{
//    if (((68+Add)>127)||((68+Add)<0)) Add = 0;
//    DrawLine(68+Add,4+SW*12,124,4+SW*12,1,1);
//    DrawLine(68+Add,4+SW*12,68+Add,15+SW*12,1,1);
//    DrawLine(68+Add,15+SW*12,124,15+SW*12,1,1);
//    DrawLine(124,4+SW*12,124,15+SW*12,1,1);
//}
//
//// ����� ������������� ������� � �����
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
////��� �������
//void FrBatNew(int Y , int X, int Size)// �������, ������������� 12 �������� , �� x y  ���� 
//{
//    if (((X+12)>0)&&((X+12)<128))
//    {
//    DrawLine(X,Y,X+11,Y,1,1); // ����� �������
//    DrawLine(X,Y+7,X+11,Y+7,1,1); // ����� ������
//    DrawLine(X,Y,X,Y+7,1,1); // ������������ ������
//    DrawLine(X+11,Y,X+11,Y+2,1,1); // ������������ ������
//    DrawLine(X+11,Y+5,X+11,Y+7,1,1); // ������������ ������
//    DrawLine(X+12,Y+2,X+12,Y+5,1,1); // ������������ ������
//    for(int l=0;l<Size/10;l++)
//         DrawLine(X+l+1,Y,X+l+1,Y+7,1,1); // ������������ ������
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
////����� ��� ��������� �������� �������� �������� ���������
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
////����� ��, �� ��� ������� ������� ����
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
//// ����� ���� ���� ��� 18
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
//// ����� ���� ���� ��� 9
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
//// ����� ������������� ������� � ����� ���������� ��������� ������� 11 �����
//
//void FrFreePx(int X, int Y ,  char* PrntStr)
//{
//  int Len = strlen(PrntStr); // ������ ������
//    if (X==0)
//    {
//        X =((MAXSMALLSMBL-Len)/2)*6-6;
//      
//    }      
//      if ((Len*6+X+12) < 128) // ������ ����������
//      {
//        // ������ ���� ��� �������
//    DrawLine(X,Y,Len*6+X+12,Y,20,0); // ������� ������ ��� �������� ,��������
//        // ������� �����
//    DrawLine(X+2,Y+2,Len*6+X+10,Y+2,1,1);  
//    DrawLine(X+2,Y+2,X+2,Y+18,1,1);
//    DrawLine(X+2,Y+18,Len*6+X+10,Y+18,1,1);
//    DrawLine(Len*6+X+10,Y+2,Len*6+X+10,Y+18,1,1);
//        // ��������� �����
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
//unsigned char ChangeFrSet (unsigned char FrSet, unsigned char MaxSet, unsigned char MinSet, unsigned char DirSet)// ��������� ������� � ������ �������� ����������
//{
//  if (DirSet) // �����
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
//unsigned char SearchEndStr(char* EditStr, unsigned char FindEnd) // ���� ����� ������� � ������������� ������
//{
//unsigned char Find = FindEnd;
//for (int i = 0; i<FindEnd; i++)
//  {
//    if (EditStr[i]<=32) EditStr[i]=' ';
//  }
//  EditStr[FindEnd]=0; // �������������� ����������� ������ � �������� ���������
//  for (int i = FindEnd-1; i>0; i--)
//  {
//    if (EditStr[i]<=32) Find--;
//    else break;
//  }
//  return Find;
//}
//
