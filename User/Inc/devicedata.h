#ifndef __DEVICEDATA_H__
#define __DEVICEDATA_H__

#define WAVE_LENGTHS_NUM 5
#define LENGTH_LINE_NUM 7
#define WIDTH_PULSE_NUM 8
#define TIME_AVR_NUM 6
#define TIME_LIGHT_NUM 3
#define LSPLACENUM 4
#define CURRENT 0
#define NEXT 1
#define MODELSNUM 5
#define MODERENUM 3



// ��������� ������������� ��������� � ����� �������� �� 0.05 ��
typedef struct
{
float BatControl[2048];// �������� ������� ������� 
} BAT_CNTR;




// ��������� ������������� ��������� � ����� �������� �� 0.05 ��
typedef struct
{
    float SlopeChADC[2];   // ������ ������ ��� 8
    float OffsetLW[18];   // �������� ����� ����� 72
} TAB_SET;

// ��������� ������������ �������
typedef struct
{
  uint16_t NumDevice;  // (9999) � �������,
  uint8_t ID_Device; // ������ 0-����� ��� 1-MOT_700
  uint8_t Ena_DB;  // ���������� ��������������� �����( 0- ���������, 1-���������)
  char AltName[20]; //  �������������� ���
} CFG_DEV;



typedef struct  // �������� ���������������� ��������� ������� ������ ������
{
  uint8_t CurrLang;  // ������� ���� Rus, Eng, Cze
  uint8_t ChnMod;  // ����� ����������� ������� 18,8R,8T 1
  uint8_t EnaCntFiber;   // ���� ���� ������� 1
  short int FiberID;    // ����� ������� 2
  short int FileNumber;  // ����� ����� (?) ����� ��� �� ����? 2
  short int FileNumberView;   // ����� ����� (?) ����� ��� �� ����? 2
  char FiberName[10];  // ��� ������� 10
  char CableID[10];    // ������������� ������ 10
  char Comments[20];   // ����������� �� ��������� 20
} CFG_USR;

// ��������� ������ �� ������
#pragma pack(push,1)
typedef struct
{
    short int FileNumber;                           // ����� ����� (?) ����� ��� �� ����? 2
    char DateMem[12];                               // ���� ���������� 10
    char TimeMem[12];                               // ����� ���������� 10
    char CableID[10];                               // ������������� ������ 10
    char FiberName[20];                             // ��� ������� 10
    char Comments[20];                              // ����������� �� ��������� 20
    float CWDMDataMem[18]; // ������ ����������
} strFILESDATA;                                      // 

unsigned InvalidDevice(); //boolean actually
void InitDevice(unsigned Err);
DWORD CheckUserGonfig (void);  // �������� ���������������� �������� 
DWORD FindFixErrBatS (void);
DWORD FindErrCoeff (void); // �������� ������� �������������
void SetLang( BYTE lang );
BYTE GetID_Dev(void); // ��������� �������������� ��� ���� ������
BYTE SetID_Dev(BYTE Data); // ��������� �������������� ��� ���� ������
BYTE GetLang(int Dir); // ��������� �������� �����/ ��� ������������
WORD GetNumDevice(void); // ��������� ������  �������
WORD SetNumDevice(WORD Num); // ��������� ������  �������

// ��������� ������ � ���������, ����� �� �������� ������ ������ �� ���������(0...1023)
extern BAT_CNTR LvlBatSav;
extern uint32_t CountBat; // ������� ������� �� �������

// ��������� ������������� ���������� ��������
extern TAB_SET CoeffLW;
// ������������ �������
extern CFG_DEV ConfigDevice;
// ���������������� ��������� (�������) ��� ������������� ����� ���������
extern CFG_USR UserSet;


#endif  /* __DEVICEDATA_H__ */



