#include "system.h"

//#define AUTO      1
//#define MANUAL    0

static BYTE Range = 0;                  // Диапазон работы усилителя (коэффициент усиления)
static char SwitchRangeMode=AUTO;       // Режим переключения коэффициентов усиления: автоматический или ручной
static BYTE TypeRslt=0;                 // Единицы измерений: дБм - 0, дБ - 1, Вт-2
static DWORD Watchdog;
volatile BYTE StateADC = FREEADC;
static int DataADC_PM=0x00;
//static BOOL Ready=TRUE;
static int FreeCodeADC_PM=0x00;       // приведенное значение кодов АЦП для текущего измерения


static int Res_Old=0;                 // Предыдущее усредненное значение АЦП с учетом смещения
static int Level[40];         // Накопленные значения АЦП с учетом смещения
static unsigned int PMWavelenght=1310;  // Текущая длина волны

unsigned int GetPMWavelenght(signed char DeltaLambda) //Возвращает, либо изменяет текущую длину волны
{
  BYTE i=0;     // Индекс
  if(!DeltaLambda) return PMWavelenght;         // Если DeltaLambda ноль,  то возращам текущую длину волны
  
  if(DeltaLambda==13)                           // Если DeltaLambda равна 13, то переключамся по калиброваным длинам волн
  {
    for(i=0;i<WAVE_LENGTHS_NUM;i++)
      if(PMWavelenght<CoeffPM.PointKalib[i])
      {
        PMWavelenght=CoeffPM.PointKalib[i];
        return CoeffPM.PointKalib[i];
      }
      
      PMWavelenght=CoeffPM.PointKalib[0];
      
    return  PMWavelenght;    
  };
  // блок для обычных измерителей
  if (WIDE_VER)  // если нет перемычки для расширенного диапазона
  {
  if((PMWavelenght+DeltaLambda)<800) 
  {
    PMWavelenght=1650;
    return  PMWavelenght;    
  }
  
  if((PMWavelenght+DeltaLambda)>1650) 
  {
    PMWavelenght=800;
    return  PMWavelenght;    
  }
  
  if(((PMWavelenght+DeltaLambda>900)&&(PMWavelenght+DeltaLambda<1210))&&(PMWavelenght+DeltaLambda<PMWavelenght))
  {
    PMWavelenght=900;
    return PMWavelenght;
  }

  if(((PMWavelenght+DeltaLambda>900)&&(PMWavelenght+DeltaLambda<1210))&&(PMWavelenght+DeltaLambda>PMWavelenght))
  {
    PMWavelenght=1210;
    return PMWavelenght;
  }
  }
  else // расширенный диапазон
  {
      if((PMWavelenght+DeltaLambda)<800) 
  {
    PMWavelenght=1700;
    return  PMWavelenght;    
  }
  if((PMWavelenght+DeltaLambda)>1700) 
  {
    PMWavelenght=800;
    return  PMWavelenght;    
  }

  }
  
  PMWavelenght+=DeltaLambda; 
  
  return PMWavelenght;

  
}
// установка длинны волны если включен Wide и JDSU поддержка
void SetAutoLamdaJDSU (DWORD Freq)
{
    if (!(WIDE_VER))  // если  расширенный диапазон
    {
      if (SetJDSU.CompMode)
      {
        PMWavelenght = GetLambdaJDSUFreq(Freq);
        GetCurrLvldB(PMWavelenght);
      }
    }
}
// хотим получить длинну волны по частоте
unsigned int GetLambdaJDSUFreq (unsigned int Freq)
{
  for (int i=0;i<6;++i)
  {
    if ((Freq > SetJDSU.FreqLambda[i]-20)&&(Freq < SetJDSU.FreqLambda[i]+20))
    {
      return SetJDSU.LambdaNum[i];
    }
  }
  return PMWavelenght;
}



unsigned int SetPMWavelenght (unsigned int CurrPMWavelenght) // принудительная установка текущей длины волны
{
  if (WIDE_VER)  // если нет перемычки для расширенного диапазона
  {
  if ((CurrPMWavelenght<800)||(CurrPMWavelenght>1650)||((CurrPMWavelenght>900)&&(CurrPMWavelenght<1210))) CurrPMWavelenght = 1310;
  }
  else // расширенный диапазон
  {
  if ((CurrPMWavelenght<800)||(CurrPMWavelenght>1700)) CurrPMWavelenght = 1310;
  }
  PMWavelenght = CurrPMWavelenght;
  return PMWavelenght;
}

// Устанавливает режим переключения коэффициентов усиления 
// измерителя мощности (ручной или автоматический)
void SetSwitchPMMode(BYTE SwMode)       
{
  SwitchRangeMode=SwMode;
  return;
}

void MyDe_t (int lin) // подпрограмма формирования программной задержки
{
  for (int i=0;i<lin;i++);
}


// ногоДрыганное чтение данных из АЦП
uint32_t ReadDataADC7782 (void)
{
  uint32_t Data=0;
  for (int i=0; i<24; i++)
  {
    PM_CLK(0);
    MyDe_t(2);
    if ((GET_PM_DATA)!=0) Data++;
    PM_CLK(1);  
    Data = Data<<1;
    MyDe_t(4);
  }
  return Data>>1;
}

// Читает данные из АЦП измерителя
// Читает данные из АЦП измерителя
// в основном цикле запускаем измерение АЦП
// ждем готовность данных (до этого выдаем  старые данные) диапазоны не переключать!!!
// по готовым данным пересчитываем  показанния и переключаем диапазоны
int GetPMData(void)       
{
  
  
  switch (StateADC)
  {
  case FREEADC: 
    PM_CS(0);               // Запускаем цикл измерения выбрав CS
    Watchdog=0; // обнуляем счетчик от зависонов (25 мС)
    StateADC = BUSYADC;
    break;
  case BUSYADC: // 
    Watchdog++; // ждем готовности АЦП
    
    if (Watchdog > 10) // не дождались более 250 мС
    {
      // чистим буфер на всякий случай (делаем пустое чтение
      ReadDataADC7782 ();
      StateADC = FREEADC;
      PM_CS(1);
    }
    else
    {
      if (!(GET_PM_DATA)) // данные готовы можно считывать
      {
        DataADC_PM =  ReadDataADC7782 ();
        Watchdog=0; //
        StateADC = READYDATA;
      }
    }
    
    break;
  case READYDATA: // данные готовы (здесь попробуем фильтрануть их)
    Watchdog=0; // 
    PM_CS(1);
    //CurrRange = Range;
    //PMCurrValue = GetPower(PMWavelenght);      // Получаем мощность в мВт не зависимо от длины волны
    StateADC = FREEADC;
    break;
  }
  
  //FreeCodeADC_PM = 0x800000-DataADC_PM;
  //FreeCodeADC_PM = 0xFFFFFF-DataADC_PM; // для маленьких тесторов
//  if(DeviceConfig.CfgRE>>1) // если новая плата аналоговая 03.03.2023
//    FreeCodeADC_PM = 0x800000 - DataADC_PM;
//  else // старая аналоговая плата
    FreeCodeADC_PM = DataADC_PM-0x800000;
  return  FreeCodeADC_PM;                           // 
}
void SetStateADC (BYTE State) // установка режима АЦП
{
 StateADC = State; 
}

BYTE GetStateADC (void) // получение режима АЦП
{
 return StateADC; 
}

int SetDataADC (int Data) // установка данных АЦП в прерывании
{
  PM_CS(1);
  DataADC_PM = Data;
  Watchdog=0; // обнуляем счетчик сторожевого таймера (такт около 30 мС)
  StateADC = READYDATA;
  return DataADC_PM;
  
}

float GetPower(unsigned int Lambda)      // Получаем мощность в нВт в зависимости от длины волны
{
//  
//  char M_PON;
//  float clck_K;
//  clkP++;
  
  int i;                          // Счётчик
  int Res_Now;                    // Текущее значение АЦП с учетом смещения
  float Lev_x;                    // Усредненные значения АЦП: среднее значение Level[0..39]
  static float Lev_W=1e-9;
  
  
  
  Res_Now = GetPMData() - CoeffPM.ShZeroRng[Range];        // Получаем результат с учетом смещения
  
  if (StateADC == READYDATA)                               // Если данные готовы
  {
    if (abs(Res_Old - Res_Now)>10000)                      // Перезапишем буфера при скачке показаний измерения
    {
      for (i=0; i<=39; i++)
        Level[i]=Res_Now;    
    }
    
    for (i=39; i>=1; i--)
    Level[i]=Level[i-1];
    Level[0] = Res_Now; 
    
    
    Lev_x = 0;                                            // Считаем среднее значение показаний АЦП
    for (i=0; i<40; i++)Lev_x +=Level[i];
    Lev_x = Lev_x/40.0;
                             // замена строк местами, сначала проверяем потом устанавливаем
    Res_Old = (int)Lev_x;                                      // Запомнили значение, как "старое"
    if (Lev_x<=0)     Lev_x = 1;                          // Если вдруг значение меньше или равно нулю
    Lev_W = ( 7.9261495712e-7 * Lev_x  * CoeffPM.CoefStykRange[Range] * GetCoeffSpctr(Lambda)) / pow( 100, 3-Range ); // собственно расчет мощности в миливаттах 
    // корректируем если длина волны 800-900 и простой фотодиод, а также коэфф не равен 0.0)
    if(( Lambda>=800)&&( Lambda<900)&&(ConfigDevice.CfgPM==1)&&(NameDB.ph_A[0]!=0.0))
      Lev_W = pow(Lev_W, 1.0+NameDB.ph_A[0]);
    if (SwitchRangeMode == AUTO)                          // Автоматическое переключение диапазонов
    {
      // Границы гистерезиса переключений
      // чтобы работал с 270 Гц надо снизить уровень переключения до 6000000 = 5B8000
      if ((Lev_x>=0x5B8000) && (Range<3))                     //Ежели переполнение не на самом грубом диапазоне, (более 8000000 с вычетом смещения)
      {
        Range++;                                          //переключаемся на менее чувствительный диапазон
        SetRange(Range);
        //CreatDelay (5000000);
        
      }
      if (Lev_x < 40000 && Range>0)                       //Ежели получена слишком малая величина не на самом чувствительном пределе,
      {
        Range--;                                          //переключаемся на более чувствительный диапазон
        SetRange(Range);
        //CreatDelay (5000000);
      }
    }
    
    
    
    
    
//    switch(GetCfgPM())
//    {
//    case 1:
//      if (Lev_W<1e-9)Lev_W = 1e-9;                          //ограничение показаний на уровне 1pW 
//      break;
//    case 2:
//      if (Lev_W<1e-6)Lev_W = 1e-6;                          //ограничение показаний на уровне 1pW 
//      break;
//    }
    StateADC = FREEADC;
  }
  return Lev_W;

}

void SetRange(unsigned char Rn)                            // установка ключей согласно выбранного диапазона
{
   if (Rn>3) Rn = 0;
   Range=Rn;
  switch(Rn)
  {
  case 0:
    KEY1(0);    KEY2(1);    KEY3(0);
    break;
  case 1:
    KEY1(1);    KEY2(1);    KEY3(0);
    break;
  case 2:
    KEY1(1);    KEY2(1);    KEY3(1);
    break;
  case 3:
    KEY1(1);    KEY2(0);    KEY3(0);
    break;
  default:
    KEY1(0);    KEY2(1);    KEY3(0);
    break;
  }

  return;
}

BYTE GetRange(void)                                        // Возвращает текущий диапазон работы измерительного усилителя
{
  return Range;
}


// Получение спектрального коэффициента в зависимости от длины волны
// диапазоны 800-900, 1210-1390, 1410-1650 

float GetCoeffSpctr(unsigned int Lambda)
{
  float k_c =1.0;    // Спектральный коэффициент
  unsigned int i;    // Индекс
  double a,b;        // индексы по которым расчитывают коэфф.  a*x+b=y
  
    if (WIDE_VER)  // если нет перемычки для расширенного диапазона
  {

  if((Lambda<800) || ((Lambda>900)&&(Lambda<1210)) || (Lambda>1650)) // Если указана неверная длина волны    
    return k_c;
  
  if(Lambda<=900)                      // Первый диапазон L-диапазон 800...900нм
  {
    i=(int)((Lambda-800)/20);          // Шаг между спектральными коэффициетами - 20 нм
    
    
    a = (CoeffPM.CoefSpctrL[i+1]-CoeffPM.CoefSpctrL[i])/20;   //a = (y2-y1)/(x2-x1)=(y2-y1)/20
    b = CoeffPM.CoefSpctrL[i]-a*(800+i*20);                   //b = y1-a*x1 = y - a*(820нм)
    
    k_c = a*Lambda+b;
    k_c = k_c*GetCoefPointKlb(850);
    
    return k_c;
  }
  
  if(Lambda<=1390)                     // Второй диапазон M-диапазон 1210...1390нм
  {
    i=(int)((Lambda-1210)/20);          // Шаг между спектральными коэффициетами - 20 нм
    
    
    a = (CoeffPM.CoefSpctrM[i+1]-CoeffPM.CoefSpctrM[i])/20;   //a = (y2-y1)/(x2-x1)=(y2-y1)/20
    b = CoeffPM.CoefSpctrM[i]-a*(1210+i*20);                   //b = y1-a*x1 = y - a*(820нм)
    
    k_c = a*Lambda+b;
    k_c = k_c*GetCoefPointKlb(1310);
    
    return k_c;
  }

  if(Lambda<1410)                       // "Окно" 1391...1409 нм
  {
    a = (CoeffPM.CoefSpctrH[0]-CoeffPM.CoefSpctrM[9])/20;   //a = (y2-y1)/(x2-x1)=(y2-y1)/20
    b = CoeffPM.CoefSpctrM[9]-a*1390;                   //b = y1-a*x1 = y - a*(820нм)
    
    k_c = a*Lambda+b;
    k_c = k_c*GetCoefPointKlb(1310);

    return k_c;
  }  

  if(Lambda<=1650)                     // Третий диапазон H-диапазон 1410...1650нм
  {
    i=(int)((Lambda-1410)/20);          // Шаг между спектральными коэффициетами - 20 нм
    
    
    a = (CoeffPM.CoefSpctrH[i+1]-CoeffPM.CoefSpctrH[i])/20;   //a = (y2-y1)/(x2-x1)=(y2-y1)/20
    b = CoeffPM.CoefSpctrH[i]-a*(1410+i*20);                   //b = y1-a*x1 = y - a*(820нм)
    
    k_c = a*Lambda+b;
    k_c = k_c*GetCoefPointKlb(1550);
    
    return k_c;
  }
  }
else // расширенный диапазон
{
    if((Lambda<800) || (Lambda>1700)) // Если указана неверная длина волны    
    return k_c;
   // 800 ... 900 
  if(Lambda<=900)                      // Первый диапазон L-диапазон 800...900нм
  {
    i=(int)((Lambda-800)/20);          // Шаг между спектральными коэффициетами - 20 нм
    
    
    a = (CoeffPM.CoefSpctrL[i+1]-CoeffPM.CoefSpctrL[i])/20;   //a = (y2-y1)/(x2-x1)=(y2-y1)/20
    b = CoeffPM.CoefSpctrL[i]-a*(800+i*20);                   //b = y1-a*x1 = y - a*(820нм)
    
    k_c = a*Lambda+b;
    k_c = k_c*GetCoefPointKlb(850);
    
    return k_c;
  }
  //не существующий диапазон расчитаем из точек 900 - 1210 тремя интервалами
  // 900... 960
  if(Lambda<=960)                     // 
  {
    i=(int)(Lambda-900);          // Шаг между спектральными коэффициетами - 20 нм
    
    
    a = ((CoeffPM.CoefSpctrL[5]/1.8) - CoeffPM.CoefSpctrL[5])/60;   //a = (y2-y1)/(x2-x1)=(y2-y1)/20
    b = CoeffPM.CoefSpctrL[5];                   //b = y1-a*x1 = y - a*(820нм)
    
    k_c = a*i+b;
    
    return k_c;
    
  }
  // 960... 1000
  
  if(Lambda<=1000)                     // не существующий диапазон расчитаем из точек 960 - 1000 прямой апроксимацией
  {
    
    k_c = CoeffPM.CoefSpctrL[5]/1.8;
    
    return k_c;
    
  }
  // 1000... 1210
  
  if(Lambda<=1210)                     // не существующий диапазон расчитаем из точек 900 - 1210 прямой апроксимацией
  {
    i=(int)(Lambda-1000);          // Шаг между спектральными коэффициетами - 20 нм
    
    
    a = (CoeffPM.CoefSpctrM[0]-(CoeffPM.CoefSpctrL[5]/1.8))/210;   //a = (y2-y1)/(x2-x1)=(y2-y1)/20
    b = CoeffPM.CoefSpctrL[5]/1.8;                   //b = y1-a*x1 = y - a*(820нм)
    
    k_c = a*i+b;
    
    return k_c;
    
  }
  // 1210.... 1390
  if(Lambda<=1390)                     // Второй диапазон M-диапазон 1210...1390нм
  {
    i=(int)((Lambda-1210)/20);          // Шаг между спектральными коэффициетами - 20 нм
    
    
    a = (CoeffPM.CoefSpctrM[i+1]-CoeffPM.CoefSpctrM[i])/20;   //a = (y2-y1)/(x2-x1)=(y2-y1)/20
    b = CoeffPM.CoefSpctrM[i]-a*(1210+i*20);                   //b = y1-a*x1 = y - a*(820нм)
    
    k_c = a*Lambda+b;
    k_c = k_c*GetCoefPointKlb(1310);
    
    return k_c;
  }
  // 1390... 1410
  if(Lambda<1410)                       // "Окно" 1391...1409 нм
  {
    a = (CoeffPM.CoefSpctrH[0]-CoeffPM.CoefSpctrM[9])/20;   //a = (y2-y1)/(x2-x1)=(y2-y1)/20
    b = CoeffPM.CoefSpctrM[9]-a*1390;                   //b = y1-a*x1 = y - a*(820нм)
    
    k_c = a*Lambda+b;
    k_c = k_c*GetCoefPointKlb(1310);

    return k_c;
  }  
    // 1410... 1650
  if(Lambda<=1650)                     // Третий диапазон H-диапазон 1410...1650нм
  {
    i=(int)((Lambda-1410)/20);          // Шаг между спектральными коэффициетами - 20 нм
    
    
    a = (CoeffPM.CoefSpctrH[i+1]-CoeffPM.CoefSpctrH[i])/20;   //a = (y2-y1)/(x2-x1)=(y2-y1)/20
    b = CoeffPM.CoefSpctrH[i]-a*(1410+i*20);                   //b = y1-a*x1 = y - a*(820нм)
    
    k_c = a*Lambda+b;
    k_c = k_c*GetCoefPointKlb(1550);
    
    return k_c;
  }
  // 1650... 1700
  if(Lambda<=1700)                     // не существующий диапазон расчитаем из точек 1650 - 1700(1650*1.1) прямой апроксимацией
  {
    i=(int)(Lambda-1650);          // Шаг между спектральными коэффициетами - 20 нм
    
    
    a = ((CoeffPM.CoefSpctrH[12]*1.1) - CoeffPM.CoefSpctrH[12])/50;   //a = (y2-y1)/(x2-x1)=(y2-y1)/20
    b = CoeffPM.CoefSpctrH[12];                   //b = y1-a*x1 = y - a*(820нм)
    
    k_c = a*i+b;
    
    return k_c;
    
  }
}
  

  return k_c;
} 



float GetCoefPointKlb(unsigned int Lambda)                                  // Возвращает дополнительный коэффициент для калиброванных длин волн
{
  BYTE i=0;                 // индекс 
  
  for(i=0;i<WAVE_LENGTHS_NUM;i++)   
    if(Lambda==CoeffPM.PointKalib[i]) return CoeffPM.CoefPointKlb[i];     // Если точка калибровки, то возвращаем подстроечный коэффициент
  
  return 1;                                                               // Если Lambda 
  
}

float Watt2dB(char *str, float mWatt,BYTE TypeRslt)           // Преобразует мВт в мкВт (нВт), дБ или дБм
{
    float Data;
    
    switch(GetCfgPM())
    {
    case 1:
      if (mWatt<1e-9)mWatt = 5e-10;                          //ограничение показаний на уровне 1pW простой фотодиод
      break;
    case 2:
    if (WIDE_VER)  // если нет перемычки для расширенного диапазона
      if (mWatt<1e-6)mWatt = 5e-7;                          //ограничение показаний на уровне 1nW  сфера
    else 
      if (mWatt<1e-8)mWatt = 5e-9;                          //ограничение показаний на уровне 1nW  сфера
      
      break;
    }
    
  switch(TypeRslt)
  {
  case 0:// дБм
    Data = 10*log10(mWatt);
    sprintf(str,"% 7.3f",Data);
    break;   
  case 1:// дБ
    Data = 10*log10(mWatt)-GetCurrLvldB(0);
    sprintf(str,"% 7.3f",Data);
    break;
  case 2:// Вт
    Data = mWatt;
    sprintf(str,"% 6.2f",Data);
    break;
  }
  return Data;
}

int GetTypeRslt()                                    // Возвращает текущее значение типа ед. изерения
{
  return TypeRslt;
}

int SetTypeRslt(BYTE type)                           // Устанавливает новое значение типа ед.измерения
{
  if(type>2) type=0;
  TypeRslt=type;
  return TypeRslt;
}

int AcquireShZeroLowRng(void)                          // Измерение уровней смещения на нижнем диапазоне (переустановка нуля)
{
  BYTE i=0;                                     // Счетчики
 // static unsigned long data;                             // Для усредненния данных
  
  
  int AverageData=0;                              // Усредненые данные
  
    if (GetRange()==0)
      // установлен диапазон 0  ?
    {
    CreatDelay(1.2e6);    //~ 0.1 S
    for(i=0;i<3;i++)
    {    
      if(StateADC==FREEADC) GetPMData();
      MyDe_t(1); // микро задержка
      while(StateADC!=READYDATA) 
      {
      MyDe_t(1); // микро задержка
      }     

      SetStateADC(FREEADC);
    }     
    //AverageData=0;
    
    
    
    for(i=0;i<20;i++)
    {    
      if(StateADC==FREEADC) GetPMData();
      MyDe_t(1); // микро задержка
      while(StateADC!=READYDATA) 
      {
      MyDe_t(1); // микро задержка
      }
      
      AverageData+=GetPMData();
      SetStateADC(FREEADC);
    }
    AverageData = (AverageData/20);
    if ( AverageData < 300000)
    CoeffPM.ShZeroRng[0]=(int)(AverageData);
    
   
  CreatDelay(1e7);  // ~ 0.8 S
    }
  return 1;
}

int AcquireShZeroRng(void)                          // Измерение уровней смещения
{
  BYTE i=0, j=0;                                    // Счетчики
 // static unsigned long data;                             // Для усредненния данных
  
  
  float AverageData=0;                              // Усредненые данные
  char Str[20];
  
  //ClearScreen();
  
  for(j=0;j<4;j++)
  {
    SetRange(j);                      // установили диапазон 0  
    CreatDelay(1.2e6);    //~ 0.1 S
    for(i=0;i<3;i++)
    {    
      if(StateADC==FREEADC) GetPMData();
      
      while(StateADC!=READYDATA) 
      {
                        
      }     
    CreatDelay(2e5);    //~ 0.02 S

      SetStateADC(FREEADC);
    }     
    sprintf(Str,"t%d.txt=\"измеряю...\"яяя",2*j+2); //
    //UARTSend2((BYTE*)Str, strlen(Str));    //
    NEX_Transmit((void*)Str);// 

    AverageData=0;
    
    
    
    for(i=0;i<20;i++)
    {    
      if(StateADC==FREEADC) GetPMData();
      
      while(StateADC!=READYDATA) 
      {
        
        CreatDelay(2e5);    //~ 0.02 S
      }
      
      AverageData+=GetPMData();
      SetStateADC(FREEADC);
    } 
    
    CoeffPM.ShZeroRng[j]=(int)(AverageData/20);
    
    sprintf(Str,"t%d.txt=\"%d\"яяя",2*j+1,CoeffPM.ShZeroRng[j]); //
    //UARTSend2((BYTE*)Str, strlen(Str));    //
      NEX_Transmit((void*)Str);// 

  }
  
  
  CreatDelay(1e7);  // ~ 0.8 S
  return 1;
}


int AcquireCoefStykRange(BYTE Rng, float* PrevRng, float* CurrRng)         // Вычисляет стыковычный коэффициент текущего диапазона с предыдущим
{
  // пользуется только в режиме UART - настройка диапазонов, выводы на экран не нужны
  //float PrevRng=0;                         // Значение кодов АЦП на предыдущем диапазоне
  //float CurrRng=0;                         // Значение кодов АЦП на текущем диапазоне
  BYTE i;                                    // Счётчик
  //char Str[20];                              // Строка для вывода на экран
  
  if((Rng<1)||(Rng>3)) return 0;             // Если указан неверный диапазон
  
  //ClearScreen();
  SetRange(Rng-1);                            // установили предыдущий диапазон
  
  //sprintf(Str,"Стык. коэфф.");       
  //putString(0,0,Str,1,1);

  //sprintf(Str,"Диапазон %d", Rng-1); 
  //putString(2,18,Str,1,0);
  //sprintf(Str,"измеряю..."); 
  //putString(82,18,Str,1,0);
  
  //PaintLCD();
  HAL_Delay(100);
  //CreatDelay(1.2e6);  // ~ 0.1 S

  // пустое чтение
  for(i=0;i<3;i++)
  {    
    if(StateADC==FREEADC) GetPMData();
    while(StateADC!=READYDATA);
    SetStateADC(FREEADC);
  }     

  // Измеряем   
  for(i=0;i<20;i++)
  {    
    if(StateADC==FREEADC) GetPMData();
    
    while(StateADC!=READYDATA) 
    {
      //sprintf(Str,"%d",0x800000-DataADC_PM); // nm
      //putString(82,54,Str,1,0);
      //PaintLCD();  
      HAL_Delay(10);
    }
    
    *PrevRng+=GetPMData();
    SetStateADC(FREEADC);
  } 
      MyDe_t(1); // микро задержка
  *PrevRng=*PrevRng/20.0;
  *PrevRng= *PrevRng - (float)CoeffPM.ShZeroRng[Rng];
  
  SetRange(Rng);
  //sprintf(Str,"%.0f",*PrevRng); 
  //putString(82,18,Str,1,0);
 
  //sprintf(Str,"Диапазон %d",Rng); // опора
  //putString(2,30,Str,1,0);
  
  //sprintf(Str,"измеряю...");
  //putString(82,30,Str,1,0);
  HAL_Delay(100);

  //PaintLCD();   
  // пустое чтение
  for(i=0;i<3;i++)
  {    
    if(StateADC==FREEADC) GetPMData();
    while(StateADC!=READYDATA);
    SetStateADC(FREEADC);
  }     
  
  // Измеряем
  for(i=0;i<20;i++)
  {    
    if(StateADC==FREEADC) GetPMData();
    
    while(StateADC!=READYDATA) 
    {
      //sprintf(Str,"%d",0x800000-DataADC_PM); // nm
      //putString(82,54,Str,1,0);
      //PaintLCD(); 
      HAL_Delay(10);
    }
    
    *CurrRng+=GetPMData();
    SetStateADC(FREEADC);
  } 
  
      MyDe_t(1); // микро задержка
  
  *CurrRng=*CurrRng/20.0;
  *CurrRng =*CurrRng - (float)CoeffPM.ShZeroRng[Rng];
  CoeffPM.CoefStykRange[0] = 1.0;
  CoeffPM.CoefStykRange[Rng]=CoeffPM.CoefStykRange[Rng-1]*(*PrevRng)/(*CurrRng*100);

  //sprintf(Str,"%.0f",*PrevRng);
  //putString(82,30,Str,1,0);
  
  //sprintf(Str,"Коэффициент"); // опора
  //putString(2,42,Str,1,0);
  //sprintf(Str,"%.2f",CoeffPM.CoefStykRange[Rng]); // nm
  //putString(82,42,Str,1,0);
      HAL_Delay(100);
  
  
  return 1;
}

float GetCoefSpctr(WORD Lambda, float RealPower)     // Возвращает спектральный коэффициент для текущей длины волны
{
  BYTE i=0;                             // Счётчик
  float AcqPower=1e-9;                  // Измеренная мощность
  char Str[64];                         // Строка для вывода
  float Coef=1.0;                       // Спектральный коэффициент
  
  
  if((Lambda<800) || ((Lambda>900)&&(Lambda<1210)) || (Lambda>1650)) // Если указана неверная длина волны    
    return 1.0;
  
  if(Lambda<=900)                      // Первый диапазон L-диапазон 800...900нм
  {
    i=(int)((Lambda-800)/20);          // Шаг между спектральными коэффициетами - 20 нм  
    CoeffPM.CoefSpctrL[i]=1.0;         // Приняли, что коэффициент равен 1
  }
  else if(Lambda<=1390)                // Второй диапазон M-диапазон 1210...1390нм
  {
    i=(int)((Lambda-1210)/20);         // Шаг между спектральными коэффициетами - 20 нм  
    CoeffPM.CoefSpctrM[i]=1.0;         // Приняли, что коэффициент равен 1
  }
  else                                 // Третий диапазон H-диапазон 1410...1650нм
  {
    i=(int)((Lambda-1410)/20);         // Шаг между спектральными коэффициетами - 20 нм  
    CoeffPM.CoefSpctrH[i]=1.0;         // Приняли, что коэффициент равен 1
  }
  
  // возможно надо исключить индикацию для цветного экрана  
  //CreatDelay(1e6);   // ~ 
  
  //CurStateADC=GetStateADC();
  if (GetStateADC()==BUSYADC)
    while(GetStateADC()!=BUSYADC);
  // Ждем, когда АЦП освободится
  
  //if(CurStateADC==READYDATA) GetPower(Lambda);    // Считаем, что АЦП готово к новым измерениям, пустое чтение
  
  SetSwitchPMMode(MANUAL);                // Переключаем измеритель в ручной режим , чтобы вдруг не переключилось 
  // требование Д.Б. 30.10.2015
  
  
  for(i=0;i<60;i++) // 40*30 мС = 1.8сек
  {
    AcqPower=GetPower(Lambda);          // Читаем данные
    // выведем здесь инфу о измерениях
    // по результатам изменений вызваныйх обработчиком клавиатуры
    sprintf(Str,"t1.txt=\"%2.2e\"яяя",AcqPower); // в поле данных dBm REF
    //UARTSend2((BYTE*)Str, strlen(Str));    //
      NEX_Transmit((void*)Str);// 
    
    sprintf(Str,"t3.txt=\"%d  %d\"яяя",i,GetRange() ); // в поле  значение счетчика nm
    //UARTSend2((BYTE*)Str, strlen(Str));    //
      NEX_Transmit((void*)Str);// 
    
    if (GetStateADC()==BUSYADC)
      while(GetStateADC()!=BUSYADC);
    // Ждем, когда АЦП освободится
    
    CreatDelay(3e5);   // ~ 
    
  }
  
  Coef=RealPower/AcqPower;              // Спектральный коэффициент
  sprintf(Str,"t5.txt=\"%.4f\"яяя",Coef);
      NEX_Transmit((void*)Str);// 
  SetSwitchPMMode(AUTO);                // Переключаем измеритель в автоматический режим
  CreatDelay(3e5);   // ~ 
  
  return Coef;
}

float GetCoefSpctrKlb(WORD index, float RealPower)     // Возвращает спектральный коэффициент для калибровочных длин волн
{
  BYTE i=0;                             // Счётчик
  float AcqPower=1e-9;                  // Измеренная мощность
  //char Str[20];                         // Строка для вывода
  CoeffPM.CoefPointKlb[index]=1.0;
  if(index>3) return 1.0;
  
  if (GetStateADC()==BUSYADC)
  while(GetStateADC()!=BUSYADC);
               // Ждем, когда АЦП освободится
 
  //if(CurStateADC==READYDATA) GetPower(Lambda);    // Считаем, что АЦП готово к новым измерениям, пустое чтение
  
  SetSwitchPMMode(MANUAL);                // Переключаем измеритель в ручной режим , чтобы вдруг не переключилось 
  // требование Д.Б. 30.10.2015
  
    
  for(i=0;i<60;i++) // 40*30 мС = 1.8сек
  {
    AcqPower=GetPower(CoeffPM.PointKalib[index]);          // Читаем данные
      CreatDelay(3e5);   // ~ 
      
  }
  SetSwitchPMMode(AUTO);                // Переключаем измеритель в автоматический режим
    
  
  CoeffPM.CoefPointKlb[index]=RealPower/AcqPower;              // Спектральный коэффициент
  
  return CoeffPM.CoefPointKlb[index];

  
}
