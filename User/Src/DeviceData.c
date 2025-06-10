// ��������� ���������������� ���������
#include "system.h"

char NumVer[10];// = "180      \0";
//const char *NumVer = {
//  #include "version.h"
//}
//;
 const uint8_t* TxGenOpt={"UUUUUUUUUUUUUUUUU"};

const unsigned WAVE_LENGTHS[WAVE_LENGTHS_NUM] = {850,1310,1490,1550,1625};


const char *Ides[2] = {"OOO NPK SvyazServis,\0","OPTOKON Co. Ltd,\0"};
const char *VerFW[2] = {",SW_rev \0",",SW_rev \0"};
//const char *Device[2][LANG_NUM] = {{"�����-7\0","TOPAZ-7\0","TOPAZ-7\0"},{"MOT-700\0","MOT-700\0","MOT-700\0"}};
const char *Device[2][2][2] = {{{"�����-7\0","�����-7\0"},{"TOPAZ-7\0","TOPAZ-7\0"}},{{"MOT-700\0","MOT-950\0"},{"MOT-700\0","MOT-950\0"}}};

const char *PMset[6] = {"10\0","31\0","32\0","-PMH\0","-VFL\0","AR\0"};
const DWORD MultIndex[LENGTH_LINE_NUM]= {1,1,1,1,2,4,8};//��������� ���������� ������� �����
const DWORD NumPointsPeriod[LENGTH_LINE_NUM]= {8,4,2,1,1,1,1};// ����� ����� �� ������
const DWORD LengthLine[LENGTH_LINE_NUM]= {2,4,8,16,32,64,128};
const DWORD DelayPeriod[LENGTH_LINE_NUM]= {700,1200,4000,8000,10000,0,0};// �������� ������� � ����� CreatDelay()~ 83.33 ��
const WORD WidthPulse[2][WIDTH_PULSE_NUM]= {{20,40,150,500,1000,3000,10000,20000},{10,40,150,500,1000,3000,10000,20000}};
const DWORD TimeAver[TIME_AVR_NUM]= {15,30,60,180,3,600};
const unsigned TimeLight[TIME_LIGHT_NUM] = {0,15,30};
const char *IdnsBC[2]= {"SvyazServis   \0","OPTOKON Co.Ltd\0"};
const char *MfidBC[2][2]= {{"TOPAZ7\0","TOPAZ7\0"},{"MOT700\0","MOT950\0"}};

// ��������� ������ � ���������, ����� �� �������� ������ ������ �� ���������(0...1023)
BAT_CNTR LvlBatSav;

uint32_t CountBat; // ������� ������� �� �������

// ��������� ������������ ����� (0...4095)
TAB_SET CoeffLW;
//// ��������� ������������ �������
CFG_DEV ConfigDevice;
//// ��������� ��������������� ��������� (�������) ��� �������������
CFG_USR UserSet;

//char CommentsOTDR[20] = {"                   \0"}; //����������� ������������� ���������� �� modes.c
uint8_t StrTXOpt[64]; // ������ �������� ������ �� ������

uint8_t TxOptBusy=0; // ������� ��������� UatrOptic
uint8_t TimeSCWP=0; // ������� ������� ��� ������������ �������

WORD ProcBat , ProcBatInd; // ������� ��������


static volatile unsigned short CheckSum = 0;




//
unsigned InvalidDevice() //boolean actually
{
  BYTE N_LS=0;
     unsigned err_str = 0;
  unsigned res = ConfigDevice.NumDevice > 9999; // 0x80
  res = (res<<1) | (ConfigDevice.ID_Device >1); // ��� ������ (����� ��� MOT) 0x04
  res = (res<<1) | (ConfigDevice.Ena_DB > 1); // ���������� ��������������� ����� 0x02
    for( unsigned i = 0; i < (sizeof(ConfigDevice.AltName)-1);++i )
  {
    if(ConfigDevice.AltName[i]<0x20) 
    {
      err_str = 1; // � ������������ ���� ����������� �������
      break;
    }
  }
  res = (res<<1) | err_str; // 0x01
  // ������� ������
  ConfigDevice.AltName[19]=0;

  return res;
}

void InitDevice(unsigned Err)
{
  if (Err && 0x01)  
  {
  for( unsigned i = 0; i < (ARRAY_SIZE(ConfigDevice.AltName)-2);++i )
  {
    ConfigDevice.AltName[i]=' '; 
  }
  ConfigDevice.AltName[(ARRAY_SIZE(ConfigDevice.AltName)-1)]=0;
  }
  if (Err && 0x02)  ConfigDevice.Ena_DB = 0; // ��������������� ����� ���
  if (Err && 0x04) ConfigDevice.ID_Device = 0; // �����
  if (Err && 0x08) ConfigDevice.NumDevice = 0;
}

DWORD CheckUserGonfig (void)  // �������� ���������������� �������� � �����������
{
  DWORD Err =0;
  uint8_t Bad_Err = 0;
  if ((UserSet.CurrLang > MAX_LANG )||((ConfigDevice.ID_Device > 0)&&(UserSet.CurrLang == Rus))) //LANG_NUM-1
  {
    Err |=0x10;
    if ((ConfigDevice.ID_Device > 0)&&(UserSet.CurrLang == Rus)) UserSet.CurrLang = Eng;// ��� ����� ����� ����������
    else UserSet.CurrLang = Rus;
    
  }
  if (UserSet.ChnMod>1) // ������� ������� ������ ���� ������ Graph or Table
  {
    Err |=0x20;
    UserSet.ChnMod=0; //Graph
  }
  if (UserSet.EnaCntFiber>1) // ���� ���� ������� 
  {
    Err |=0x40;
    UserSet.EnaCntFiber=0; //
  }
  if (UserSet.FiberID>999) //  // ����� ������� 
  {
    Err |=0x80;
    UserSet.FiberID=0;
  }
  if (UserSet.FileNumber>400) //  ����� ����� (������� ��������� ������)
  {
    Err |=0x100;
    UserSet.FileNumber=0;
  }
  if (UserSet.FileNumberView>400) //  ����� ����� (������� ��������� ������)
  {
    Err |=0x200;
    UserSet.FileNumberView=0;
  }
  // �������� ����� �������
  for( unsigned i = 0; i < (sizeof(UserSet.FiberName)-1);++i )
  {
    if(UserSet.FiberName[i]<0x20) 
    {
      Bad_Err = 1; // � ������������ ���� ����������� �������
      Err |=0x400;
      break;
    }
  }
  if(Bad_Err)
  {
    for( unsigned i = 0; i < (ARRAY_SIZE(UserSet.FiberName)-2);++i )
    {
      UserSet.FiberName[i]=' '; 
    }
    UserSet.FiberName[(ARRAY_SIZE(UserSet.FiberName)-1)]=0;
    Bad_Err = 0;
  }
  // �������� ����� ������
  for( unsigned i = 0; i < (sizeof(UserSet.CableID)-1);++i )
  {
    if(UserSet.CableID[i]<0x20) 
    {
      Bad_Err = 1; // � ������������ ���� ����������� �������
      Err |=0x800;
      break;
    }
  }
  if(Bad_Err)
  {
    for( unsigned i = 0; i < (ARRAY_SIZE(UserSet.CableID)-2);++i )
    {
      UserSet.CableID[i]=' '; 
    }
    UserSet.CableID[(ARRAY_SIZE(UserSet.CableID)-1)]=0;
    Bad_Err = 0;
  }
  // �������� Comments
  for( unsigned i = 0; i < (sizeof(UserSet.Comments)-1);++i )
  {
    if(UserSet.Comments[i]<0x20) 
    {
      Bad_Err = 1; // � ������������ ���� ����������� �������
      Err |=0x1000;
      break;
    }
  }
  if(Bad_Err)
  {
    for( unsigned i = 0; i < (ARRAY_SIZE(UserSet.Comments)-2);++i )
    {
      UserSet.Comments[i]=' '; 
    }
    UserSet.Comments[(ARRAY_SIZE(UserSet.Comments)-1)]=0;
    Bad_Err = 0;
  }
  
  
  return Err; 
}

DWORD FindFixErrBatS (void)
{
  DWORD Err = 0;
  for (int i=0; i<ARRAY_SIZE(LvlBatSav.BatControl); ++i)
    if (isnan(LvlBatSav.BatControl[i])||(LvlBatSav.BatControl[i]<=0.0)||(LvlBatSav.BatControl[i]>3000)) Err |=0x1;// 
 if(Err)
 {
     for (int i=0; i<ARRAY_SIZE(LvlBatSav.BatControl); ++i)
      LvlBatSav.BatControl[i]=i*0.001;
     //
           EEPROM_write(&LvlBatSav, ADR_BatSave,sizeof(LvlBatSav));

 }
 return Err;
}

DWORD FindErrCoeff (void)
{
  DWORD Err = 0;
  // ����� �������� �� ����� �����
  for (int i=0;i<2;++i)
  {
    if ((CoeffLW.SlopeChADC[i]>0.045)||(CoeffLW.SlopeChADC[i]<0.010))
    {
      CoeffLW.SlopeChADC[i]=0.0257;
              Err |=0x2000;
    }
  }
  // ����� ������� ���
  for (int i=0;i<18;++i)
  {
    if ((CoeffLW.OffsetLW[i]>-40.0)||(CoeffLW.OffsetLW[i]<-80.0))
    {
      CoeffLW.OffsetLW[i]=-63.5+i*0.1;
              Err |=0x4000;
    }
  }
  return Err;
}




void SetLang( BYTE lang ) // ������� �� �������
{
  if ( lang < MAX_LANG )//LANG_NUM-1
  {
    lang = 0;
  }
  if (lang==0)lang=ConfigDevice.ID_Device;
    UserSet.CurrLang = lang;
}

BYTE GetID_Dev(void) // ��������� �������������� ��� ���� ������
{
  return ConfigDevice.ID_Device;
}
BYTE SetID_Dev(BYTE Data) // ��������� �������������� ��� ���� ������
{
  if (Data>1) Data=1;
  UserSet.CurrLang = Eng; // ������ ��� ��������� �������������� ������������� Eng
  return ConfigDevice.ID_Device = Data;
}

BYTE GetLang(int Dir) // ��������� �������� �����, � ������������� �� �����
{
  BYTE Lang = UserSet.CurrLang;
  if (Dir) // ����������� �����
  {
    if (Dir>0)
    {
    Lang++;
    //Lang = GetID_Dev()*2 - Lang + 1;
    if (Lang > ((GetID_Dev())?(MAX_LANG):(1))) Lang = GetID_Dev(); //LANG_NUM-1
    }
    else
    {
      if ((Lang>1 && (GetID_Dev()))||(Lang>0 && !(GetID_Dev())))
        Lang--;
      else
        Lang=((GetID_Dev())?(MAX_LANG):(1));//LANG_NUM-1
    }
  }
  
    UserSet.CurrLang = Lang;
  
/*  Lang += Dir;
  if (Lang > MAX_LANG) 
  {
      if(ConfigDevice.ID_Device) UserSet.CurrLang = Eng;
      else UserSet.CurrLang = Rus;
  }
  else
  {
      if ( Lang < GetID_Dev() ) UserSet.CurrLang = MAX_LANG;
    else UserSet.CurrLang = Lang;
  }
  */
  return UserSet.CurrLang;
}
    

WORD GetNumDevice(void) // ��������� ������  �������
{
  return ConfigDevice.NumDevice;
}

WORD SetNumDevice(WORD Num) // ��������� ������  �������
{
  if (Num>9999) Num=9999;
  return ConfigDevice.NumDevice = Num;
}
// ��������� �������������� "������" , ��� � ��� ����������������
void GetDeviceHW( char* name ) // from Computer
{
  char Str[100];
  
  sprintf(Str,"STM32H743 DEVICE\r");
  strcat( name,Str);
}


void GetNumVer (char* Str) // ��������� ������ ������
{
  char out[5];
  strcpy( out, NumVer );//, strlen(Ides[device->ID_Device])
  out[4] = 0;
  int Num = atoi (out);
  //Num=Num/5;
  sprintf (Str, "6%c%c%c", (BYTE)(0x30+(Num/260)), (BYTE)(0x30+(Num/26)), (BYTE)(0x61+(Num%26)));
}



