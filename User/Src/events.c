// подпрограмма расчета событий SRRNXN4MGXT310 
#include "system.h"

#define BC_def 800 // уровень отражения задается в зависимости от линии обычно от 80-60 дБ
#define FPO_def 0
#define NF_def 65535
#define NFSF_def 1000
#define PO_def 0
#define LT_def 200  // 0.2dB порог определения события по затуханию
#define RT_def 65000  // 65dB порог определения события по отражению от 0 до 65 дБ
#define ET_def 3000  // 3dB порог затухания для определения конца линии
// добавка к уровню определения тренда для определения уровня шумов
#define ADDTONOISE 10000 // добавка к порогу для определения уровня шумов


EVENTS EvenTrace[SizeTableEvnt];
// структура окончания блока событий рефлектограммы (текущего измерения)
ENDBLKEVENTS EndEvenBlk;
// структура предварительный событий для расчета в белкор
MY_EVNT MyEvenTrace[3*SizeTableEvnt];

//unsigned short PosEndEvenBlkELMP; // позиция расположения конца линии в отсчетах (надо продублировать для получения данных

static unsigned short CntKeyEvents = 0;
static unsigned short EnaKeyEvents = 0; //признак разрешения вывода событий в файл белкора

void InitMyEventsTable (void) // инициализация структур внутренних событий
{
  for (int i=0;i<3*SizeTableEvnt;++i)
  {
    MyEvenTrace[i].EN = i+1;
    MyEvenTrace[i].EPT = 0;
    MyEvenTrace[i].Volume = 0;
    MyEvenTrace[i].PointInImp = 0;
    MyEvenTrace[i].Type_Evnt = 0;
    MyEvenTrace[i].CompBell = 0;
  }
}

void InitEventsTable (void) // инициализация структур событий
{
  for (int i=0;i<SizeTableEvnt;++i)
  {
    EvenTrace[i].EN = i+1;
    EvenTrace[i].EPT = 0;
    EvenTrace[i].ACI = 0;
    EvenTrace[i].EL = 0;
    EvenTrace[i].ER = 0;
    EvenTrace[i].EC[0] = '2';
    EvenTrace[i].EC[1] = 'O';
    EvenTrace[i].EC[2] = '9';
    EvenTrace[i].EC[3] = '9';
    EvenTrace[i].EC[4] = '9';
    EvenTrace[i].EC[5] = '9';
    EvenTrace[i].LMT[0] = '2';
    EvenTrace[i].LMT[1] = 'P';
  for (int j=0;j<9;++j)
  {
    EvenTrace[i].COMM_EVN[j] = ' ';
  }
    EvenTrace[i].COMM_EVN[9] = 0;
    
  }
  // инициализация конца блока событий
  EndEvenBlk.EEL = 0;
  EndEvenBlk.ELMP[0] = 0;
  EndEvenBlk.ELMP[1] = 1;
  EndEvenBlk.ORL = (unsigned short)(g_VolORL*1000);
  EndEvenBlk.RLMP[0] = 0;
  EndEvenBlk.RLMP[1] = 1;
  //PosEndEvenBlkELMP = 1;
}

DWORD CheckReflParam (void)
{
  DWORD err = 0;
  if ((ReflParam.BC>850)||(ReflParam.BC<450)) {err++; ReflParam.BC = BC_def;}
  if (ReflParam.FPO!=0){err++; ReflParam.FPO = FPO_def;}
  if (ReflParam.NF<40000) {err++;ReflParam.NF = NF_def;}
  if (ReflParam.NFSF!=1000) {err++;ReflParam.NFSF = NFSF_def;}
  if (ReflParam.PO!=0) {err++;ReflParam.PO = PO_def;}
  if ((ReflParam.ET>20000)||((ReflParam.ET<100))) {err++; ReflParam.ET = ET_def;} //end line 0.1 --- 20.0
  if ((ReflParam.LT>5000)||(ReflParam.LT<50)) {err++; ReflParam.LT = LT_def;}  //styk  0.05 --- 5.0
  if ((ReflParam.RT>RT_def)||(ReflParam.RT<10000)) {err++; ReflParam.RT = RT_def;} // refl 10 ... 65
  return err;

}

unsigned short GetNumEvents (void) // возвращает число событий (
{
  return CntKeyEvents;
}

unsigned short GetSetEnaEvents (unsigned Dir) // устанавливает возвращает признак разрешения событий
{
  if (Dir) // изменяем признак
  {
    if (EnaKeyEvents) EnaKeyEvents=0;
    else EnaKeyEvents=1;
  }
  
  return EnaKeyEvents;
}

// расчет среднего отклонения заданного размера с данной точки в массиве
float CalkSCO (unsigned short* array, unsigned short BeginPoint, unsigned short Size)// , float* Base
{
  float b1;
  float Sy=0.0,Sy2=0.0;
  for (int i=BeginPoint; i<BeginPoint+Size; ++i)
  {
    Sy = Sy + array[i];
    Sy2 = Sy2 + array[i]*array[i];
  }
  
  b1 = (Size*Sy2)- Sy*Sy;
  b1 = b1/(Size*(Size-1));
  b1 = sqrtf(b1);
  //Base = (Sy - b1*Sx)/Size;
  return b1;
  
}

// расчет тренда по облаку точек заданного размера с данной точки в массиве
float CalkTrend (unsigned short* array, unsigned short BeginPoint, unsigned short Size, unsigned short TrLvlDwn, float b0)// , float* Base
{
  float b1,  bx;//b0,
  float Sx=0.0,Sy=0.0,Sxy=0.0,Sx2=0.0, Smin=0.0;
  for (int i=BeginPoint; i<BeginPoint+Size; ++i)
  {
    if (array[i]>Smin) Smin = array[i];
    Sx = Sx + i;
    Sy = Sy + array[i];
    Sxy = Sxy + (i*array[i]);
    Sx2 = Sx2 + i*i;
  }
  
  b1 = ((Sx*Sy) - (Size*Sxy))/((Sx*Sx)-(Size*Sx2));
  // ветвим алгоритм для просчета простого тренда
  if (b0 != 0.0)
  {
  //b0 = GetLSACoef();// учет наклона рефлектограммы от параметров
  b1 = (b1 - b0)*(Size-1);
  // положительный тренд обозначает снижение
  if (b1 < (float)ReflParam.LT)// порог чуствительности события
  {
    // отрицательное событие
    bx = -b1;
    if ((bx < (float)ReflParam.LT)||(bx > 1500)||(TrLvlDwn<Smin)) // более -1.5 дБ
    // рассмотрим только отрицательные не превышающие пороги контроля отражений и шумов
    b1 = 0.0; 
  }
  else // положит затухание пока какое есть, оценим его в зависимости от уровня сигнала
  {
    if ((array[BeginPoint] > TrLvlDwn)&&(b1 <= (0.3*(Smin-TrLvlDwn)+200.0)))
      b1 = 0.0;

  }
  }
  
  //Base = (Sy - b1*Sx)/Size;
  return b1;
  
}
// Расчет разницы по бегущему шаблону (на растоянии длительности импульса на интервале как в корреляции,
// считаем средние по краям и если превышает порог то выводим значение разницы)
unsigned short CalkDownJump (unsigned short* array, unsigned short BeginPoint, unsigned short Size)
{
  float MedBeg = 0, MedEnd = 0;
  unsigned short SizePlusAdd= 2;
  if (Size<10) SizePlusAdd = 5;
  else SizePlusAdd = Size>>1; // добавка точек измерения (по краям от кривой сравнения)
  // расчет среднеарифметического для данных на краях заданного интервала
  for (int i=0; i<SizePlusAdd; ++i)// цикл на колличество точек (интервалы по краям)
  {
    MedBeg += array[i+BeginPoint];
    MedEnd += array[i+Size+SizePlusAdd+BeginPoint];
  }
  MedBeg = MedBeg/SizePlusAdd;
  MedEnd = MedEnd/SizePlusAdd;
  if ((MedEnd - MedBeg)>(float)(ReflParam.LT))
  {
    return (unsigned short)(MedEnd - MedBeg);
  }
  
return (0);
}
// расчет кореляции  по облаку точек заданного размера с данной точки в массиве
float CalkCorelation32 (DWORD* array, unsigned short BeginPoint, unsigned short Size, BYTE Type)// , float* Base
{
  
  // увеличиваем размер окна на 3 точки , этого доствточно даже для фильтрованных результатов с лавинником
  // так как там шаг минимален 166 нс а фильтр 750 нс думается что 3 точек достаточно
  double Sxy=0.0,Sx2=0.0,Sy2=0.0 ,Sx=0.0,Sy=0.0;//
  double Xi = 0.0, Correl = 0.0;
  double tmp;
  //static double  Alt_Cor = 1.0,Yi = 0.0 ;
  //double Ymed = 0.0;
  double Dxy = 0.0,Dx = 0.0, Dy = 0.0;// 
  double ImExpMax = 0; // расчетный максимум импульса для расчета спада
 // unsigned short Ymin=65535, Ymax=0; // значения минимумов и максимумов на интервале
 // int Imin=0, Imax=0; //индексы минимумов и максимумов
//  unsigned short SizePlus= 2;
//  if (Size<10) SizePlus = 5;
//  else SizePlus = Size>>1; // добавка точек измерения (по краям от кривой сравнения)
  // расчет среднеарифметического для данных на заданном интервале
  for (int i=0; i<Size+SizePlus; ++i)// цикл на колличество точек (длительность импульса плюс интервалы по краям)
  {
    // Суммирование Yi
    tmp = (double)array[i+BeginPoint];
    Sy += tmp; 
    Sy2 += pow(tmp,2);
    // генерация Xi
    switch(Type)
    {
    case 1:
   // контроль перепада  ---\____
//    if ( i<SizePlus || Size+SizePlus<i )
//      Xi = ((i<SizePlus)?(1.0):(-1.0));
//    else 
//      Xi = 1.0-2.0*(i-SizePlus)/(Size+1);
      Xi = 0;
      break;
    default:
// контроль импульса  ____/---\____
     if ( i<=Size )
     {
       Xi = 1000.0*(1.0 - exp(TauImp()*i));
       ImExpMax = Xi;
     }
    else 
      Xi = 1000.0*ImExpMax*(exp(TauImp()*(i-Size)));
      break;
    }
    // суммирование Xi
    Sx += Xi;
    Sx2 += pow(Xi,2);
    Sxy += Xi*tmp;
  }
  Dxy = Sxy - (Sx*Sy)/(Size+SizePlus);
  
  Dx = sqrtf(Sx2 - ((Sx*Sx)/(Size+SizePlus)));
  Dy = sqrtf(Sy2 - ((Sy*Sy)/(Size+SizePlus)));

   Correl = Dxy/(Dx*Dy);
  
  return (float)Correl;
  //return (float)Alt_Cor;
  
}


unsigned short CalkEventsKeys (unsigned short* array, unsigned short PII, BYTE Type) // вычисление событий для текущей рефлектограммы
{
#define DELTACHNG 400 // изменение сигнала 0.4 дБ
#define MORED 8 // добавка к PII для правильного расчета безотражающего события после отражения
  
  unsigned short CurLvlVol = DELTACHNG; // текущий уровень сигнала 
  unsigned short NewMinLvlVol = DELTACHNG; // текущий минимальный уровень сигнала 
  unsigned short IndexCurLvlVol = 0; // индекс текущего уровеня сигнала 
  unsigned short IndexCurBeg = 0; // индекс первого оt текущего уровеня сигнала 
  unsigned short NumEvents = 0; // счетчик событий
  //long ReflEvents = 0; // значение отражения в отражающем событии
  static unsigned short Count = 0; // длинна события - долгий высокий уровень 
  unsigned short EventsType1 = 0; // признак наличия отражающего события
  //unsigned short EventsType2 = 0; // признак наличия отражающего события
  unsigned short PosEventsType1 = 0; // позиция отражающего события чтобы не перепутать с концом линии
  //unsigned short PosEventsType2 = 0; // позиция отражающего события чтобы не перепутать с концом линии
  unsigned short PointsShift = GetPointsShift(); // сдвиг измерения 
  unsigned short LvlTrendUp = GetLvlTrend(); // начальный уровень оценки шумов при расчете тренда 
  float LSACoefIN = GetLSACoef();
  int FindBeg = (int)(LSACoefIN*PII); //+ (DELTACHNG/2) погонное затухание в зависимости от уст. параметов
  int ChkLvlUp ; // начальный уровень контроля отражающего события
  long BegTemp; // пересчет начала добавка
  
  //float AutoRef = 200;
  float Trend = 1000;
  //float CurrTrend = 1000;
  int temp=0, temp1 = 0;
  //char UartStr[40];
  //float SumBuff=0;
  //float Correlations;
  float Buff[160];
  memset( Buff, 0, 160 * sizeof(int) );
  //  InitEventsTable (); // инициализация структур событий
  Count = 0;
  int CurrBig = 0;
  //int YesEvents = 0; // признак обнаружения события
  //int YesCorrFix = 0; // признак необходимости фиксации
  //int CorrFix = 0; // фиксация корреляции
  //int CountEvents = 0; // счетчик длительности события
  //unsigned short Sobytie;
  unsigned short LevlRefl; // уровень отражающего события при отражении
  volatile unsigned short i;
  unsigned short DeadPoint = 0; // точка до которой не считаем тренд если было отражение
  unsigned short DeadPointUp = 0; // точка до которой не смотрим следующее отражающее событие
//  unsigned short EndDeadLine = CalkEndRefl ( array,  PII,   LvlTrendUp);

  // тестовый вывод параметров установки
  //    sprintf (UartStr, "PII=%d Shift=%d \n",PII,PointsShift);
  //   UARTSend0 ((BYTE*)UartStr, strlen (UartStr));
  float TrOld=0.0, TrCurr=0.0, TrNew=0.0;
  // поиск конца линии , с конца рефлектограммы

  for (i = PII; i<OUTSIZE-2*PII; i++)
  
  //for (i = PII; i<=EndDeadLine; i++)
  {
    // тест вывод для контроля тренда
    //  sprintf (UartStr, "%d %.3f \n",array[i],CalkTrend(array, i, 2*PII, LvlTrendUp, 0.0));
    // UARTSend0 ((BYTE*)UartStr, strlen (UartStr));
    
    // контроль длительности высокого сигнала
    if ((array[i] < 230)) // &&(i<4*PII)0.23 дБ и начало 4 длительностей импульса
    {
      if ((i-CurrBig)>1) // новое превышение уровня 3дБ 
      {
        CurrBig = i;
        Count = 0;
      }
      else // повторяется превышение... считаем
      {
        CurrBig = i;
        Count++;
      }
      if (Count > PointsInImpulse(1)) // 
      {// большое отражение принимаем за конец линиии
        NumEvents++;    
        //if ((GetIndexLN()==0)&&(IndexCurLvlVol>0)) IndexCurLvlVol--; // фиксируем позицию начала события// для 2 км берем предыдущую
        
        EvenTrace[NumEvents-1].EPT = IndexCurLvlVol;
        EvenTrace[NumEvents-1].EC[0] = '2';//2
        EvenTrace[NumEvents-1].EC[1] = 'E';//E
        EndEvenBlk.ELMP[1] = IndexCurLvlVol;//записываем значение точки конца линии ... надо пересчитать в реальное

        memcpy (&EvenTrace[NumEvents-1].COMM_EVN, "BigOvrEnd\0",10);
        if ((NumEvents>1)&&((EvenTrace[NumEvents-1].EPT>=EvenTrace[NumEvents-2].EPT-2)&&(EvenTrace[NumEvents-1].EPT<=EvenTrace[NumEvents-2].EPT+2)))// было отражение - пересчитаем Точки совпали
        {
          NumEvents--;    
          //EvenTrace[NumEvents-1].EC[0] = '2';//берем значени от определенного при анализе события
          EvenTrace[NumEvents-1].EC[1] = 'E';//E
          EvenTrace[NumEvents-1].EL = 0;  
          memcpy (&EvenTrace[NumEvents-1].COMM_EVN, "LineEnd_0\0",10);
          // значение ER посчитано раньше...
        }
        EndEvenBlk.EEL = array[EndEvenBlk.ELMP[1]]-EndEvenBlk.EEL;
        
        if (IndexCurLvlVol == 0)// не было снижения сигнала более чем на 0.4 дБ
        {
          // скорей всего без линии большое отражение в начале
          // так как отражение посчитаем его параметры
          EvenTrace[NumEvents-1].EPT=0;
          EvenTrace[NumEvents-1].EC[0] = '2';
          EvenTrace[NumEvents-1].EC[1] = 'E';
          //EvenTrace[NumEvents-1].ER =  CalkEventER (array[0] - CalkMINMAX (&array[i], PII+1, 0)); // вычисление отражения
          EvenTrace[NumEvents-1].EL =  0; // вычисление отражения
          
          //EndEvenBlk.ELMP[1] = i;//записываем значение точки конца линии ... надо пересчитать в реальное
          memcpy (&EvenTrace[NumEvents-1].COMM_EVN, "NotFiber1\0",10);
          NumEvents=0;    // нет событий!!!
          
        }
        
        break; //большое отражение выходим
      }
    }
    
    //  проверка уровня на порог шумов
    if (array[i]> (LvlTrendUp+ADDTONOISE))//GetNoiseLvl()
    {
      //IndexCurLvlVol = i;
      NumEvents++;
      
      EvenTrace[NumEvents-1].EPT=IndexCurLvlVol;
      EvenTrace[NumEvents-1].EC[0] = '0';//0
      EvenTrace[NumEvents-1].EC[1] = 'O';//O - out of range finding end of fiber
      // сигнал ниже уровня контрольруемых шумов
      // для простого 18.5 дБ, для лавинного 25 дБ
      EndEvenBlk.ELMP[1] = IndexCurLvlVol;//записываем значение точки конца линии ... надо пересчитать в реальное
      EvenTrace[NumEvents-1].EL=0;
      memcpy (&EvenTrace[NumEvents-1].COMM_EVN, "OutOfRng \0",10);
      // было отражение - пероформим событие  получим 1E, 
      if (i<DeadPoint+3) // было недавно отражающее событие
      {
        NumEvents--;
        EndEvenBlk.ELMP[1] = PosEventsType1;
        EvenTrace[NumEvents-1].EL = 0; //array[i]-array[PosEventsType1];// Found by software
        EvenTrace[NumEvents-1].EC[0] = '1';// reflective events with end fiber
        EvenTrace[NumEvents-1].EC[1] = 'E';// Found by software
        memcpy (&EvenTrace[NumEvents-1].COMM_EVN, "FbEndRefl\0",10);
      }
      EndEvenBlk.EEL = array[EndEvenBlk.ELMP[1]]-EndEvenBlk.EEL;
      break;// ушли в шумы - выходим (определили вариант конца линии)
    }
    // установка нового текущего индекса
    temp = (int)(array[i])-(int)(array[i-1]);  // считаем разницу текущей точки и предыдущей
    // проверка на провал больше порога оценки и ниже уровня заданных шумов в 15.000 дБ
    // имеем провал больше чем на заданный порог, от текущего уровня и при этом сигнл уже ниже 15 дБ
    if ((temp>0)&&(temp>ReflParam.ET)&&(array[i]>CurLvlVol)&&(CurLvlVol>15000))
    {// без отражательный конец - сигнал упал на уровень больше порога
      NumEvents++;
      memcpy (&EvenTrace[NumEvents-1].COMM_EVN, "BigJmpDwn\0",10);
      if ((i - IndexCurLvlVol)> (2*PII))
      {
        IndexCurLvlVol = i; // если точка далее последнего изменения чем на 2 длит. импульса 
      memcpy (&EvenTrace[NumEvents-1].COMM_EVN, "BigJmpDnN\0",10);
        // установим новую точку
      }
        
      EvenTrace[NumEvents-1].EPT = IndexCurLvlVol;
      EvenTrace[NumEvents-1].EC[0] = '0';//0
      EvenTrace[NumEvents-1].EC[1] = 'E';//E
      EvenTrace[NumEvents-1].EL=0;//array[IndexCurLvlVol]-array[IndexCurLvlVol-1];
      EndEvenBlk.ELMP[1] = IndexCurLvlVol;//записываем значение точки конца линии ... надо пересчитать в реальное
      if (i<DeadPoint+3) // было недавно отражающее событие
      {
        NumEvents--;
        EndEvenBlk.ELMP[1] = PosEventsType1;
        EvenTrace[NumEvents-1].EL = 0;//array[i]-array[PosEventsType1];// Found by software // уровень затухания в событии
        EvenTrace[NumEvents-1].EC[0] = '1';// reflective events with end fiber
        EvenTrace[NumEvents-1].EC[1] = 'E';// Found by software
        memcpy (&EvenTrace[NumEvents-1].COMM_EVN, "FbEndJmp \0",10);
        
      }
      EndEvenBlk.EEL = array[EndEvenBlk.ELMP[1]]-EndEvenBlk.EEL;
      
      break;
    }
    // прыгнули ниже или не далеко от текущего уровня,
    // отслеживание тенденции сснижения сигнала
    // как бы бежим по сигналу не реагируя на "всплески"
    if ((array[i]>(CurLvlVol-DELTACHNG))&&(array[i]>(NewMinLvlVol-DELTACHNG))&&(abs(temp)<DELTACHNG)&&(temp!=0))
    {
      if (NewMinLvlVol<array[i])
      {// новый минимум посмотрим если перепад вниз через длительность импульса???
        NewMinLvlVol=array[i];
      }
      CurLvlVol = array[i]; // установим новое значение
      IndexCurLvlVol = i;
    }
    // контроль уровня сигнала при не найденом горизонтальном участке
    // 18 дБ, универсальный уровень оценки шумов при поиске горизонтального участка после импульса
      if ((array[i]>18000)&&(IndexCurBeg == 0)) // not fiber - нет событий
      {
      NumEvents++;    
      EvenTrace[NumEvents-1].EPT=0;
      EvenTrace[NumEvents-1].EC[0] = '0';
      EvenTrace[NumEvents-1].EC[1] = 'E';
      EvenTrace[NumEvents-1].EL = 0;// Found by software // уровень затухания в событии
          //EndEvenBlk.ELMP[1] = i;//записываем значение точки конца линии ... надо пересчитать в реальное
          memcpy (&EvenTrace[NumEvents-1].COMM_EVN, "NotFiber2\0",10);
      NumEvents = 0;   // нет событий!!!!
        
        break; //нет линии - выходим
        
      }
    
    // поиск начала линии после действия зондирующего импульса
    temp1 = array[i+PII] - array[i];
    // если есть две точки различающиеся меньше чем на 0.2 дБ и не было таких еще 
    if ((temp1<FindBeg)&&(IndexCurBeg == 0)&&(array[i] > 1000))//&&(temp1>0)
      //if ((temp>DELTACHNG/2)&&(IndexCurBeg == 0)&&(array[i] > 3000))
    {
      // проверим уровень сигнала на горизонтальном участке
      IndexCurBeg = i;
      NumEvents++;    
      //EvenTrace[NumEvents-1].EPT=IndexCurLvlVol;
      EvenTrace[NumEvents-1].EPT = 0;
      EvenTrace[NumEvents-1].EC[0] = '2';// 3 -начало линии сам придумал !!!!
      EvenTrace[NumEvents-1].EC[1] = 'F';// Found by software
      EndEvenBlk.ELMP[0] = 0;//записываем значение точки ... надо пересчитать в реальное
      // расчитаем заначение в начале то есть добавку
      // (unsigned short)(((unsigned int)((640*i)/NumPointsPeriod[GetIndexLN()]))<<6)
      // приблизительный расчет значения в начале линии, изменяем значение в первой точке,
      // 
      
      BegTemp = (long)(i*LSACoefIN); // затухание на участке данного размера
      //BegTemp =  BegTemp/NumPointsPeriod[GetIndexLN()];
      //BegTemp =  BegTemp>>6;
      
      EndEvenBlk.EEL = (long)(array[i])- BegTemp ;//записываем значение точки .в начале линии
      array[0] = (unsigned short)(EndEvenBlk.EEL);// не переписываем так как уже сохранили в EndEvenBlk.EEL
      EvenTrace[NumEvents-1].EL =  array[0]; // начальное затухание в точке
      
      // поиск отражения
      //unsigned short tempMIN = CalkMINMAX (&array[0], i, 0);
      EvenTrace[NumEvents-1].ER =  CalkEventER (EndEvenBlk.EEL - CalkMINMAX (&array[0], i, 0));
      memcpy (&EvenTrace[NumEvents-1].COMM_EVN, "FirstBeg \0",10);
      NumEvents--;    
      
    }
    // поиск других событий 
    if ((Type)&&(IndexCurBeg))
    {
      unsigned short PIIM = PointsInImpulse(0);
      // Фиксация максим тренда на участке
      // значение тренда определяет затухание на участке
      if ((i > DeadPoint)&&(i > DeadPointUp))
      {
        Trend = CalkTrend(array, i, PIIM, LvlTrendUp, LSACoefIN);
      }
      else
        Trend = 0.0;
      
      
      // модуль вывода вспомогательной информации по определению событий  
      //sprintf (UartStr, "%d %d    %.3f \n",PIIM,i,Trend);
      //UARTSend0 ((BYTE*)UartStr, strlen (UartStr));
      
      // определение экстремума по 3-м точкам
      TrOld = TrCurr;
      TrCurr = TrNew;
      TrNew = Trend;
      if ((TrOld>0)&&(TrCurr>0)&&(TrNew>0)) // имеем не нулевые значения тренда, определим экстремум
      {
        if((TrCurr>=TrOld)&&(TrCurr>=TrNew)&&(NumEvents<(SizeTableEvnt-1))&&(TrCurr<ReflParam.ET))// можно зафиксировать экстремум, он меньше порога конца линии
        {
          // оценим уровень события
          //if ((LvlTrendUp > array[i-1])||(TrCurr >= (0.3*(array[i-1]-LvlTrendUp)+200.0)))
            // если сигнал выше порогга оценки тренда - фиксируем событие (отрицат затухание)
          //{
            NumEvents++;    
          EvenTrace[NumEvents-1].EPT = i-1;// запомним позицию события
          EvenTrace[NumEvents-1].EC[0] = '0';// NO reflective events
          EvenTrace[NumEvents-1].EC[1] = 'F';// Found by software
          EvenTrace[NumEvents-1].EL = (short int)TrCurr;// Found by software          
          EvenTrace[NumEvents-1].ER = 0;
          memcpy (&EvenTrace[NumEvents-1].COMM_EVN, "NoReflEvt\0",10);
          DeadPoint = i + PII;
          //}
        }
      }
      // определение неотражающего события при отрицательном затухании
      if ((TrOld<0)&&(TrCurr<0)&&(TrNew<0)) // имеем не нулевые значения тренда, определим экстремум
      {
        if((TrCurr<=TrOld)&&(TrCurr<=TrNew)&&(NumEvents<(SizeTableEvnt-1))&&(-TrCurr<ReflParam.ET))// можно зафиксировать экстремум, он меньше порога конца линии
        {
          //if (LvlTrendUp > array[i-1])// если сигнал выше порогга оценки тренда - фиксируем событие (отрицат затухание)
          //{
          NumEvents++;    
          EvenTrace[NumEvents-1].EPT = i-1;// запомним позицию события
          EvenTrace[NumEvents-1].EC[0] = '0';// NO reflective events
          EvenTrace[NumEvents-1].EC[1] = 'F';// Found by software
          EvenTrace[NumEvents-1].EL = (short int)TrCurr;// Found by software          
          EvenTrace[NumEvents-1].ER = 0;
          memcpy (&EvenTrace[NumEvents-1].COMM_EVN, "NpReflEvt\0",10);
          DeadPoint = i + PII;
          //}
        }
      }
      
      /*
      Trend = (array[i+PointsShift]-array[i])/1.0;
      int a = i - PII;
      // 50*е(0.0002*level) - авто порог оценки отражающего события
      AutoRef = -50*exp(0.0002*array[i]) - ReflParam.LT;
      //
      SumBuff = SumBuff + Trend;  
      if( a - PII >= 0 ) SumBuff = SumBuff - Buff[ a % PII ];
      Buff[ a % PII ] = Trend;
      
      Correlations = CalkCorelation(array, i, PII); // расчет коррреляции неотражающего затухания
      // расчет ведется вперед на PII точек, надо определить крайние точки оценки события применительно к реальным данным
      // при уровне оцкенки перепада 0.2 дБ, анализируем перход через 20 единиц корреляции,
      // при уровне в 1дБ переход через 100единиц, и так далее
      Sobytie = CalkDownJump(array, i, PII);
      CorrFix = 0;
      if (Correlations<-0.7)
      {
      if (!YesEvents)// есть событие begin
      {
      YesEvents = 1;
      CountEvents = 0;
    }
      else
      {
      CountEvents++;
      if (CountEvents>PII)// событие длительное
      {
      YesCorrFix = 1; // необходимо зафиксировать событие
    }
    }
    }
      else
      {
      YesEvents = 0;
      if (YesCorrFix)
      {
      YesCorrFix = 0;
      CorrFix = i - PII;
    }
    }
      
      
      if ((Trend < (AutoRef))&&(!EventsType2)) // имеем изменение 
      {
      EventsType2 = 1;
    }
      if ((EventsType2) && (Trend < AutoRef))
      {
      if (Trend<CurrTrend) 
      {
      CurrTrend = Trend;
      PosEventsType2 = i;
    }
    }
      if ((Trend > 0) && (EventsType2) && (NumEvents<(SizeTableEvnt-1)))
      {
      NumEvents++;    
      EvenTrace[NumEvents-1].EPT = PosEventsType2;// запомним позицию события
      EvenTrace[NumEvents-1].EC[0] = '1';// reflective events
      EvenTrace[NumEvents-1].EC[1] = 'F';// Found by software
      CurrTrend = 1000;
      EventsType2  = 0;
      memcpy (&EvenTrace[NumEvents-1].COMM_EVN, "NoRefEvnt\0",10);
    } 
      */
      // контроль отражающего события
      // порог контроля оценки отражающего события
      ChkLvlUp = (int)array[i] - (int)LvlTrendUp;
      if (ChkLvlUp>0) ChkLvlUp =  (ChkLvlUp>>1) + DELTACHNG;
      else ChkLvlUp =  DELTACHNG;
      
      if (((array[i]-array[i+2])>ChkLvlUp)&&(!EventsType1)&&(NumEvents<(SizeTableEvnt-1))&&(i>DeadPointUp))// скачек вверх более чем на 0.4 дБ
      {
        EventsType1=1;
        if (GetIndexLN()) IndexCurLvlVol = i; // фиксируем позицию начала события
        else IndexCurLvlVol = i-1; // для 2 км берем предыдущую
        PosEventsType1 = IndexCurLvlVol;
        DeadPoint = i + PII + MORED;  // устанавливаем возможную точку конца отражающего события
        // Надо проверить нет ли провала после события до завершения мертвой зоны
        
      }
      // обсчитаем событие....
      if (EventsType1) // время счета события
      {
//        if (EventsType1<(PII+1))
//          EventsType1++;
//        else // фиксируем событие 
        {
          if (NumEvents<(SizeTableEvnt-1))
          {
            // имеем отражающее событие переустанавливаем значение неопределяния тренда
            EventsType1 = 0;
            // здесь надо проверить нет ли предварительно отрицательного затухания для "малых" отражений
            // 01.03.2022 Это можно АККУРАТНО исключить чтобы не потерять Отрицательное затухание!
            // попробуем убрать
            //if( (EvenTrace[NumEvents-1].EL < 0)&&(((EvenTrace[NumEvents-1].EPT)>(PosEventsType1-PII/2-2))&&((EvenTrace[NumEvents-1].EPT)<(PosEventsType1-PII/2+2))))
            //{
            //  NumEvents--; // стираем событие - если было опознано событие с отрицательным затуханием....
            //}
            
            NumEvents++;    
            EvenTrace[NumEvents-1].EPT = PosEventsType1;// запомним позицию события
            EvenTrace[NumEvents-1].EC[0] = '1';// reflective events
            EvenTrace[NumEvents-1].EC[1] = 'F';// Found by software
            int step = 0;
            // 28.02.2022 на тестовой линии фиксанули  отражающее событие с "отрицательным" затуханием
            // Здесь надо обработать событие - это может быть с отрицательным затуханием
            // Д.Б. предложил начинать поиск выхода при условии поиска ровного участка даже выше точки начала события
            // например на 1 дБ
            // поиск участка после отражения ниже уровня до отражения, так было и есть ПОКА!
            do
            { // ищем точку ниже события и имеющей гор. участок из 2 точек не более
              int trew = array[PosEventsType1+ PII + step+1]-array[PosEventsType1+ PII + step];
              //if (((array[PosEventsType1+ PII + step]-array[PosEventsType1])>0)&&(trew<(DELTACHNG>>2))&&(trew>0)) break;
              // Добавим к первой точке  1 дБ для поиска выхода на горизонтальный участок
              if (((array[PosEventsType1+ PII + step]-(array[PosEventsType1]+1000))>0)&&(trew<(DELTACHNG>>2))&&(trew>0)) break;
            }
            while (step++ < (4095-(PosEventsType1 + 2*PII))); 
            //  if (step>50)
            //  {
            DeadPointUp = PosEventsType1 + PII + step;
            EvenTrace[NumEvents-1].EL = array[PosEventsType1+PII+step+1]-array[PosEventsType1];// Found by software - уровень затухания
            // проверим не ОТРИЦАТЕЛЬНЫЙ ЛИ перепад
            if(EvenTrace[NumEvents-1].EL < 0)
            {
          EvenTrace[NumEvents-1].EC[0] = '0';// NO reflective events
          EvenTrace[NumEvents-1].EC[1] = 'F';// Found by software
          EvenTrace[NumEvents-1].ER = 0;
          memcpy (&EvenTrace[NumEvents-1].COMM_EVN, "NegEvt   \0",10);
             
            }
            // проверим значение тренда на тройной длительности импульса для определения конца линии
             if( (array[PosEventsType1+3*PII+step+1]-array[PosEventsType1+PII+step+1]) > ReflParam.ET)// более 3 дБ
             {
               memcpy (&EvenTrace[NumEvents-1].COMM_EVN, "ReflEvEnd\0",10);
               EvenTrace[NumEvents-1].EPT = IndexCurLvlVol;
               EvenTrace[NumEvents-1].EC[0] = '2';//2
               EvenTrace[NumEvents-1].EC[1] = 'E';//E
               EvenTrace[NumEvents-1].EL = 0;
               EndEvenBlk.ELMP[1] = PosEventsType1;//записываем значение точки конца линии ... надо пересчитать в реальное
               EndEvenBlk.EEL = array[EndEvenBlk.ELMP[1]]-EndEvenBlk.EEL;
               break;
             }
             
            //ReflEvents = array[PosEventsType1];
            memcpy (&EvenTrace[NumEvents-1].COMM_EVN, "ReflEvtS \0",10);
            LevlRefl = CalkMINMAX (&array[PosEventsType1], PII+1, 0);
            if (LevlRefl < 100)
            {
              EvenTrace[NumEvents-1].EC[0] = '2';// reflective events - с перегрузкой
              memcpy (&EvenTrace[NumEvents-1].COMM_EVN, "ReflEvtB \0",10);
            }
            
            EvenTrace[NumEvents-1].ER =  CalkEventER (array[PosEventsType1] - LevlRefl); // вычисление отражения
            //EvenTrace[NumEvents-1].ER=array[PosEventsType1]-ReflEvents;
            // проверка что событие не последнее и это не конец линии (только для отражающих)
          }
          
        }
      }
    }
  }
  // проверка последнего события 
//  if (EvenTrace[NumEvents-1].EPT < EndDeadLine) // последнее событие раньше , найденго конца
//  {
//    // установим свое событие конец линии
//               memcpy (&EvenTrace[NumEvents-1].COMM_EVN, "FindEnd  \0",10);
//               EvenTrace[NumEvents-1].EPT = IndexCurLvlVol;
//               EvenTrace[NumEvents-1].EC[0] = '2';//2
//               EvenTrace[NumEvents-1].EC[1] = 'E';//E
//               EvenTrace[NumEvents-1].EL = 0;
//               EndEvenBlk.ELMP[1] = PosEventsType1;//записываем значение точки конца линии ... надо пересчитать в реальное
//               EndEvenBlk.EEL = array[EndEvenBlk.ELMP[1]]-EndEvenBlk.EEL;
//  }
//     

  // 
  return NumEvents;
}

// проверка на возможное затухание без отражательного события
// 
void CalkNonReflDwn (void)
{
  
}
// поиск конца линии , с конца рефлектограммы
unsigned short CalkEndRefl (unsigned short* array, unsigned short PII,  unsigned short TrLvlDwn)
{
  
  unsigned short EndLine = OUTSIZE-1;
  float Fir = 0.0, Two = 0.0, Tre = 0.0; 
  float Tst = GetLSACoef()*PII ;
  for (int i=OUTSIZE-1; i>PII; --i)
  {
    Tre = Two;
    Two = Fir;
    Fir = 0.0;
    if (array[i]<(TrLvlDwn) )// зона измерения тренда - посчитаем тренд на участке
    {
      Fir = CalkTrend(array, i-PII, PII, TrLvlDwn, 0.0);
      if ((Fir>0) && (Fir < (Tst+100.0))&& (Fir > (Tst-100.0)))
      EndLine = (unsigned short)i;
      else Fir = 0.0;
    }
    if (Fir>0 && Two>0 && Tre>0) 
    {
      EndLine = (unsigned short)(i+2);
      break;
    }
  }
  return EndLine;
}



// расчет значений EPT для событий от положения курсора
long CalkEPT (unsigned long Position)
{
        float NowDS;
        float EvntEPT;
            // востанавливаем расчетные значения (при отображении рефлектограммы)
        PointsPerPeriod = NumPointsPeriod[GetIndexLN()]; // SetPointsPerPeriod( ... );
        //NowDS = (ADCPeriod*50000)/PointsPerPeriod; //  устанавливаем значения DS для установленного режима измерения
        NowDS = GetValueDS(); //  устанавливаем значения DS для установленного режима измерения
        EvntEPT = Position*NowDS;
        EvntEPT = EvntEPT/10000;
        return (long)(EvntEPT);

}

unsigned short GetLvlTrend (void) // расчет начального уровня с которго считаем
{
  double Data = (5000.0*log10((double)(sqrt((float)(GetNumAverag()/8770.0)))))+16000.0; //расчет начальной точки контроля
  if (GetApdiSet ())// c лавиником (есть фильтр)
  {
    if (GetIndexIM()>4) // filtr ON
    {
      Data = Data + 8000.0;
    }
  }
  else
      Data = Data - 3000.0;
  if ( Data <0) Data = 1000.0;
  return (unsigned short)(Data);
    
    
}

// расчет Обратного отражения по данным рефлектограммы

// Вычисление отражения в зависимости от всплеска сигнала и длительности импульса
// в формате х1000 - то есть тысячных децибела
// исходные данные BC - 80 дБ(800),значение импульса в дБ*1000, длительность импульса
long CalkEventER (long LevelER)
//
{
  if (LevelER<=0.0) LevelER = 1.0;
  float CalkR = 10.0*log10(pow(10, (LevelER/5000.0))-1); // 10^(f/5)
  float Calk = GetWidthPulse(GetIndexIM());// получаем длительность импульса в нс
  Calk = (ReflParam.BC/-10.0)+10.0*log10(Calk); // получаем Q в зависимости от длител. импульса
  Calk = Calk + CalkR;
  return (long)(Calk*1000);
}

unsigned short CalkMINMAX (unsigned short* array, int size, BYTE type)
{
  unsigned short MINi = 65535;
  unsigned short MAXi = 0;
  for (int j = 0; j< size; ++j)
  {
    if (array[j] < MINi) MINi = array[j];
    if (array[j] > MAXi) MAXi = array[j];
    
  }
  if (type) return MAXi;
  else return MINi;
}

