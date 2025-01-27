// Функции управления "железом"
// управление выходными разовыми командами через PCA9555
/*
P7 - LD_R - 0x80 -управление красным лазером (первая половина)
вторая половина микросхемы (P8-P15)
P8 - OnG - 0x01 - сигнал разрешения генератора 80 МГц на аналоговой плате
P9 - CW - 0x02 - сигнал постоянного излучения лазеров
P10 - SW_L - 0x04 - младший разряд сигналов выбора рабочего места лазера 
P11 - 3 - 0x08 - Сигнал переключения диапазонов измертеля
P12 - SWRS1K2 - 0x10 -сигнал переключения выбора оптического RS
P13 - 2 - 0x20 - Сигнал переключения диапазонов измертеля
P14 - SW_H - 0x40 - старший разряд сигналов выбора рабочего места лазера 
P15 - 1 - 0x80 - Сигнал переключения диапазонов измертеля

*/

#include "main.h"
// var
//HAL_StatusTypeDef StatusI2C2 = HAL_ERROR;

DEV_CFG DeviceConfig; // структура конфигурации прибора
TAB_USER UserMeasConfig; // таблица пользовательских данных для прибора
TAB_DEVS SettingPrm; // таблица настроек прибора-  Р2, измеритель длины
MEM_BLK MemoryData; // блок данных сохранения (текущий) в нем содержится счетчик волокна

uint8_t NeedSaveChange=0; // признак необходимости перезаписи блоков памяти
extern UART_HandleTypeDef huart2;

uint8_t StrTXOpt[64]; // буффер передачи данных по оптике

uint8_t TxOptBusy=0; // признак занятости UatrOptic
uint8_t TimeSCWP=0; // счетчик времени для переключения лазеров
static uint16_t BegLW; // индекс первой длины волны в источниках, в зависимости от установок с исключением или нет
// функции проверки данных в ЕЕПРОМ: конфигурация, таблицы настройки измерителя, таблицы пользовательских установок (опоры, пределы), 
/* конфигурация: 
uint16_t NumDevice; //2 number device
uint8_t NameDevice[16];//16 строка наименование прибора
uint8_t CfgPM;//1  конфигурация измерителя 0 - нет, 1- фотодиод, 2 -сфера
uint8_t CfgRE;//1 конфигурация красного глаза 0 - нет, 1 - есть
uint8_t CfgL;//1 конфигурация тестера с автоматом
uint16_t PlaceLS[LSPLACENUM]; //3  Место 1, (1310) Конфигурация установленных лазеров по местам 2byte   
uint8_t CfgOUT[LSPLACENUM];//3 конфигурация выхода в режиме автоматического переключения длин волн 0- не в списке, 1- в списке (буковка "d"), но теперь для каждого лазера свое

*/
void SW_RS232 (int a)
{
  (a)?(CtrlExpand(0x10, 0x10)):(CtrlExpand(0, 0x10)); // управление переключателем RS 
}

// управление лазерным источником (сигналом CW и FW)
void ControlSC (uint8_t Mode) // 
{
  __HAL_UART_DISABLE_IT(&huart2, UART_IT_PE);

  SWITCH_LW(); // установка требуемой длины волны источника (управление ключами)
  //const uint8_t* TxGenOpt={"UUUUUUUUUUUUUUUUU"};
  switch(Mode)
  {
  case 0: // выключаем источник и станавливаем всякие действия
    CW_S(0);
    HAL_UART_DMAStop(&huart2);
    lSofn = 0; //
    break;
  case 1: // включаем источник и станавливаем всякие действия
    CW_S(1);
    HAL_UART_DMAStop(&huart2);
    lSofn = 1; //
    break;
  case 2: // выключаем источник и перестраиваем UART и запускаем Генерацию меандра 275 Hz
    CW_S(0);
    huart2.Init.BaudRate = 550;
    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
      Error_Handler();
    }
    HAL_UART_DMAStop(&huart2);
    //HAL_UART_Receive_IT(&huart2, RxBufOpt,1); // ждем принятия первого байта из внешнего мира
    //Dummy = huart2.Instance->DR ; // чистим буффер приема 
    
    HAL_UART_Transmit_DMA(&huart2, (void*)TxGenOpt,strlen((void*)TxGenOpt)); // выдаем 
    lSofn = 1; //
    break;
  case 3: // выключаем источник и и перестраиваем UART и запускаем Генерацию меандра 2 kHz
    CW_S(0);
    huart2.Init.BaudRate = 4000;
    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
      Error_Handler();
    }
    HAL_UART_DMAStop(&huart2);
    //HAL_UART_Receive_IT(&huart2, RxBufOpt,1); // ждем принятия первого байта из внешнего мира
    //Dummy = huart2.Instance->DR ; // чистим буффер приема 
    
    HAL_UART_Transmit_DMA(&huart2, (void*)TxGenOpt,strlen((void*)TxGenOpt)); // выдаем 
    lSofn = 1; //
    break;
  case 4: // выключаем источник и перестраиваем UART на скорость 1200
    CW_S(0);
    huart2.Init.BaudRate = 1200;
    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
      Error_Handler();
    }
    HAL_UART_DMAStop(&huart2);
    //HAL_UART_Receive_IT(&huart2, RxBufOpt,1); // ждем принятия первого байта из внешнего мира
    //Dummy = huart2.Instance->DR ; // чистим буффер приема 
    //    HAL_UART_DMAStop(&huart2);
    TimeSCWP = 70; // 3.5sec
    lSofn = 1; //
    break;
  default:
    CW_S(0);
    HAL_UART_DMAStop(&huart2);
    lSofn = 0; //
    break;
  }
}
// функция переключения лазеров по кругу с излучением информации о себе
void Run_SCWP(void) // каждые 50 мС
{
  if(TimeSCWP++<70)
  {
    switch(TimeSCWP)
    {
    case 2:
      sprintf((char*)StrTXOpt, "UUUU####SL%4u %.2f\rUUU",(DeviceConfig.PlaceLS[Ind_LW]),SettingPrm.PowSc[Ind_LW]);//UUU ooo1300
      HAL_UART_Transmit_DMA(&huart2, (void*)StrTXOpt,strlen((void*)StrTXOpt)); // выдаем 
      TxOptBusy = 1;
      //TsT(1);// дернем ногой для проверки
      //  HAL_GPIO_WritePin(test_GPIO_Port, test_Pin, GPIO_PIN_SET); 

      //HAL_Delay(200);
      break;
    case 10: // через 500 мС после старта передачи... 
      CW_S(1); // on CW 
      break;
    default:
      break;
    }
  }
  else
  {
    TimeSCWP = 0; // все сначала
    CW_S(0); // off CW
    // переключаем ЛАЗЕРЫ
      //  GetSetLW_SC (1, 0) ; // так будут все светить
        GetSetLW_SC (1, 1) ; // а так только на одном выходе

//    if(++Ind_LW>2) Ind_LW=0;
//    
//    while(DeviceConfig.PlaceLS[Ind_LW]==0)
//    {
//      if(++Ind_LW>2) Ind_LW=0;
//    }
    //SWITCH_LW(); // переключение лазеров (управление)
    NeedReDraw = 1;
  }
}
// управление красным глазом
void ControlRE (uint8_t Mode) // 
{
        LD_R(Mode);
}
//  static uint8_t CntRE;
//    if(CntRE<20)CntRE++;
//    else CntRE=0;
//    
//  if(DeviceConfig.CfgRE)// есть красный лазер, то управляем
//  {
//    switch(ReMode)
//    {
//    case OFFs:
//      LD_R(0);
//      CntRE=0;
//      break;    
//    case SCW:
//      LD_R(1);
//      CntRE=0;
//      break;    
//    case S27W:
//      if(CntRE<10)
//        LD_R(0);
//      else
//        LD_R(1);
//      break;    
//    }
//  }
//}

uint32_t InvalidDevice() //boolean actually
{
  N_LS=0;
  uint32_t res = 0; 
  uint8_t ChkOUT = 0; // есть ли лазеры и его конфиг по CW*, не должно быть все НУЛИ
  res = (res<<1) | (DeviceConfig.NumDevice > 9999);                     // 10 bit
  //for( unsigned i = 0; i < 16/*PLACE_LS_NUM*/;++i )
  //{
    // if (g_ConfigDevice.CfgCombLS[i]!=0) N_LS++;
    res = (res<<1) | (DeviceConfig.NameDevice[15]);                     // 9 bit
  //}
  res = (res<<1) | (DeviceConfig.CfgPM > 2);   //  (g_ConfigDevice.CfgPM < 1)||                // 8 bit
  res = (res<<1) | (DeviceConfig.CfgRE > 3);                     // 7 bit + NewPalte 03/03/2023
  res = (res<<1) | (DeviceConfig.CfgL >1);                     // 6 bit
  for( unsigned i = 0; i < LSPLACENUM/*PLACE_LS_NUM*/;++i )
  {
    if (DeviceConfig.PlaceLS[i]!=0) N_LS++;
    res = (res<<1) | (DeviceConfig.PlaceLS[i] > 4300);                     // 3-4-5 bit
  }
  
  for( unsigned i = 0; i < LSPLACENUM/*PLACE_LS_NUM*/;++i )
  {
    res = (res<<1) | (DeviceConfig.CfgOUT[i]>1);  
    ChkOUT +=DeviceConfig.CfgOUT[i];// 0-1-2 bit
  }
  if(ChkOUT==0) res = res | 0x7;
  //
      for (int i=0; i<ARRAY_SIZE(DeviceConfig.SmPx); ++i)
    if ((DeviceConfig.SmPx[i]<=800)||(DeviceConfig.SmPx[i]>5000)) res |=0x1000;//смещения (начальное) для измерителя P2 длин волн калибровки
// калибровочные коэффициенты измерителя длины (
    if (isnan(DeviceConfig.KGen)||(DeviceConfig.KGen<=0.5)||(DeviceConfig.KGen>2.0)) res |=0x2000;// 
    if (isnan(DeviceConfig.MZone)||(DeviceConfig.MZone<0.0)||(DeviceConfig.MZone>500.)) res |=0x4000;// 
    if (isnan(DeviceConfig.Kprop)||(DeviceConfig.Kprop<=0.0)||(DeviceConfig.Kprop>10.)) res |=0x8000;// 
    if ((DeviceConfig.YearManuf<22)||(DeviceConfig.YearManuf>99)) res |=0x10000;//год производства
  
  return res;
}

void InitDevice(unsigned Err)
{
  uint8_t InitName[16]={'Т','О','П','А','З','-','7','3','1','6',0,0,0,0,0,0};

    if (Err & 0x1000) 
      for (int i=0; i<ARRAY_SIZE(DeviceConfig.SmPx); ++i) DeviceConfig.SmPx[i]=3000+i;
    if (Err & 0x2000)
      DeviceConfig.KGen=1.38;// 72 MHz
    if (Err & 0x4000)
       DeviceConfig.MZone=0.0;
    if (Err & 0x8000)
      DeviceConfig.Kprop=0.02; // 
    if (Err & 0x10000)
      DeviceConfig.YearManuf=23; // 
  
  if (Err & 0x01) DeviceConfig.CfgOUT[2] = 0; //         // 0 bit .. для формирования оъединений
  Err = Err>>1;
  if (Err & 0x01)  DeviceConfig.CfgOUT[1] = 1; //  // 1 bit
  Err = Err>>1;
  if (Err & 0x01)  DeviceConfig.CfgOUT[0] = 0; //    2 bit
  Err = Err>>1;
  if (Err & 0x01) DeviceConfig.PlaceLS[2] = 0;                     // 3 bit
  Err = Err>>1;
  if (Err & 0x01) DeviceConfig.PlaceLS[1] = 1310;                     // 4 bit
  Err = Err>>1;
  if (Err & 0x01) DeviceConfig.PlaceLS[0] = 0;                     // 5 bit
  Err = Err>>1;
  if (Err & 0x01) DeviceConfig.CfgL = 0;                     // 6 bit
  Err = Err>>1;
  if (Err & 0x01) DeviceConfig.CfgRE = 0; //                     // 7 bit
  Err = Err>>1;
  if (Err & 0x01) DeviceConfig.CfgPM = 1;                     // 8 bit
  Err = Err>>1;
  if (Err & 0x01)                    // 9 bit заполняем строку наименования
  {
  for( unsigned i = 0; i < 16/*PLACE_LS_NUM*/;++i )
   DeviceConfig.NameDevice[i] = InitName[i]; 
  }
    if (Err & 0x01) DeviceConfig.NumDevice = 0;                     // 8 bit
  Err = Err>>1;

}

void SetRange (uint8_t Rng) // установка сигналов переключения ключей для выбранного диапазона Range = 3 - самый грубый, Range = 0 - Самый чувствительный
{
  uint16_t Temp;
  Range = Rng;
  switch (Rng)
  {
  case 0:
    Temp=K2; // K1=0, K2=1, K3=0 
    break;
  case 1:
    Temp=K1|K2; // K1=1, K2=1, K3=0 K1- optic switch add 4,7MOm
    break;
  case 2:
    Temp=K1|K2|K3; // K1=1, K2=1, K3=1 K3 - on 47kOm
    break;
  case 3:
    Temp=K1; // K1=1, K2=0, K3=0 K2 -set inverse if 0 - on 390 Om, and shift PHOTODIOD 
    break;
  }
  CtrlExpand(Temp, K1|K2|K3);
}

void SWITCH_LW(void) // Функция установки сигналов для выбора лазерного места, 
{
    switch (Ind_LW)
    {
    case 0:
    CtrlExpand(0, SW_H|SW_L);
      break;
    case 1:
    CtrlExpand(SW_L, SW_H|SW_L);
      break;
    case 2: // 1490 - третье место
    CtrlExpand(SW_H, SW_H|SW_L);
      break;
    default:
    CtrlExpand(SW_L, SW_H|SW_L);
      break;
    }
    
}
// поиск/установка индекса длины волны источника для режимов, с признаком на один вход
uint8_t GetSetLW_SC (uint8_t Next, uint8_t Choice) // Next 1-Следующий,0- первый из присутствущих;
                                                     //Choice 1-с контролем по разрешению, 0-без контроля
// Ind_LW - внешняя глобальная
// используем глобальную переменную Ind_LW (индекс рабочего места установки лазера)
{
  uint8_t Mask[3]={1,1,1};
  // перепишем маску в зависимости от CHOICE
  if(Choice)
  {
    memcpy(Mask, &DeviceConfig.CfgOUT, 3); //потому как три байта
  }
  if(Next)
  {
    if(++Ind_LW>2) Ind_LW=0;
  }
  else
  {
    Ind_LW=0; // ставим на начало
  }
    
    while((DeviceConfig.PlaceLS[Ind_LW]==0)||(Mask[Ind_LW]==0))
    {
      if(++Ind_LW>2) Ind_LW=0;
    }
   SWITCH_LW(); // аппаратную выборку подтвердим.
   return Ind_LW;                                                
 }
// поиск и установка индекса длины волны калибровки по длине волны
int GetIndxLW (uint16_t in_LW)
{
  int ret = 2; // вывод по умолчанию (1310)
  int i;
  for(i=0;i<WAVE_LENGTHS_NUM;i++)
  {
    if(CalibrLW[i] == in_LW) ret = i;
  }
    return ret;
}

// контроль установок настройки измерителя  + спектралка
uint32_t FindErrCoeff (void)
{
  static uint32_t Err = 0;
  uint32_t NewFlash = 0;
  for (int i=0; i<ARRAY_SIZE(CoeffPM.ShZeroRng); ++i) 
  {
    if (CoeffPM.ShZeroRng[i]== -1) NewFlash++ ; // чистая ли память
    if (CoeffPM.ShZeroRng[i]>300000) Err |=0x01; // смещения на диапазонах
  }
  for (int i=0; i<ARRAY_SIZE(CoeffPM.CoefStykRange); ++i)
    if (isnan(CoeffPM.CoefStykRange[i])||(CoeffPM.CoefStykRange[i]<=0.1)||(CoeffPM.CoefStykRange[i]>100.0)) Err |=0x40;// коэффициенты стыковки диапазонов
  for (int i=0; i<ARRAY_SIZE(CoeffPM.CoefPointKlb); ++i)
    if (isnan(CoeffPM.CoefPointKlb[i])||(CoeffPM.CoefPointKlb[i]<=0.1)||(CoeffPM.CoefPointKlb[i]>800.0)) Err |=0x04;// коэффициенты привязки длин волн калибровки
  for (int i=0; i<ARRAY_SIZE(CoeffPM.CoefSpctrL); ++i) 
    if (isnan(CoeffPM.CoefSpctrL[i])||(CoeffPM.CoefSpctrL[i]<=0.1)||(CoeffPM.CoefSpctrL[i]>8000.0)) Err |=0x08;// коэффициенты спектральной харр. 800-900
  for (int i=0; i<ARRAY_SIZE(CoeffPM.CoefSpctrM); ++i)
    if (isnan(CoeffPM.CoefSpctrM[i])||(CoeffPM.CoefSpctrM[i]<=0.1)||(CoeffPM.CoefSpctrM[i]>8000.0)) Err |=0x10;// коэффициенты спектральной харр. 1210-1370
  for (int i=0; i<ARRAY_SIZE(CoeffPM.CoefSpctrH); ++i)
    if (isnan(CoeffPM.CoefSpctrH[i])||(CoeffPM.CoefSpctrH[i]<=0.1)||(CoeffPM.CoefSpctrH[i]>8000.0)) Err |=0x20;// коэффициенты спектральной харр. 1410-1650
  for (int i=0; i<ARRAY_SIZE(CoeffPM.CoefTune); ++i)
    if (isnan(CoeffPM.CoefTune[i])||(CoeffPM.CoefTune[i]<0.1)||(CoeffPM.CoefTune[i]>10.0)) Err |=0x80;// подстроечные коэфф по основным диапазонам .0 до +12.0 (Д.Б. будет возражать про)
  for (int i=0; i<WAVE_LENGTHS_NUM; ++i) // не смотря что их 7 пользуем только 3 по местам лазеров
    if (isnan(CoeffPM.BaseRet[i])||(CoeffPM.BaseRet[i]<=-9.0)||(CoeffPM.BaseRet[i]>9.0)) Err |=0x100;// базовые уровни ORL от -9.0 до +9.0 мощности источника
    if (isnan(CoeffPM.Pow_mW_Clbr)||(CoeffPM.Pow_mW_Clbr<=0.0)||(CoeffPM.Pow_mW_Clbr>0.00001)) Err |=0x200;// смещение ORL до 10нВт
  
  if(NewFlash == ARRAY_SIZE(CoeffPM.ShZeroRng)) Err |=0x7F;
  return Err;
}

void FixErrCoeff (uint32_t Err)
{
  if (Err != 0)
  {
    if (Err & 0x01) for (int i=0; i<ARRAY_SIZE(CoeffPM.ShZeroRng); ++i) CoeffPM.ShZeroRng[i]=0; // смещение плохое
    if (Err & 0x04) 
      for (int i=0; i<ARRAY_SIZE(CoeffPM.CoefPointKlb); ++i) CoeffPM.CoefPointKlb[i]=1.0;
    if (Err & 0x08) 
      for (int i=0; i<ARRAY_SIZE(CoeffPM.CoefSpctrL); ++i) CoeffPM.CoefSpctrL[i]=1.0;
    if (Err & 0x10)
      for (int i=0; i<ARRAY_SIZE(CoeffPM.CoefSpctrM); ++i) CoeffPM.CoefSpctrM[i]=1.0;
    if (Err & 0x20)
      for (int i=0; i<ARRAY_SIZE(CoeffPM.CoefSpctrH); ++i) CoeffPM.CoefSpctrH[i]=1.0;
    if (Err & 0x40)
      for (int i=0; i<ARRAY_SIZE(CoeffPM.CoefStykRange); ++i) CoeffPM.CoefStykRange[i]=1.0;
    if (Err & 0x80)
      for (int i=0; i<ARRAY_SIZE(CoeffPM.CoefTune); ++i) CoeffPM.CoefTune[i]=1.0;
    if (Err & 0x100)
      for (int i=0; i<WAVE_LENGTHS_NUM; ++i) CoeffPM.BaseRet[i]=-1.0-i; // не смотря что их 7 пользуем только 3 по местам лазеров
        if (Err & 0x200) CoeffPM.Pow_mW_Clbr = 0.0000004; // 400пВт
      

  }
}
 
// контроль таблицы спец длин волн
uint32_t FindErrSpec (void)
{
  static uint32_t Err = 0;
  for (int i=0; i<ARRAY_SIZE(SpecCoeffPM.PointSpecKalib); ++i)
    if (SpecCoeffPM.PointSpecKalib[i]>1700) Err |=0x01;// длины волн специальных коэфф. не правильные
  for (int i=0; i<ARRAY_SIZE(SpecCoeffPM.CoefSpecKlb); ++i)
    if (isnan(SpecCoeffPM.CoefSpecKlb[i])||(SpecCoeffPM.CoefSpecKlb[i]<=0.1)||(SpecCoeffPM.CoefSpecKlb[i]>800.0)) Err |=0x02;// коэффициенты спектральные спец волн
  return Err;
}

void FixErrSpec (uint32_t Err)
{
  if (Err != 0)
  {
    if (Err & 0x01) for (int i=0; i<ARRAY_SIZE(SpecCoeffPM.PointSpecKalib); ++i) SpecCoeffPM.PointSpecKalib[i]=0; // смещение плохое
    if (Err & 0x02) 
      for (int i=0; i<ARRAY_SIZE(SpecCoeffPM.CoefSpecKlb); ++i) SpecCoeffPM.CoefSpecKlb[i]=1.0;
  }
}

    // контроль установок пользователя измерений прибора

uint32_t FindErrUserMC (void)
{
  static uint32_t Err = 0;
    if (isnan(UserMeasConfig.BaseLevel)||(UserMeasConfig.BaseLevel<=-100.0)||(UserMeasConfig.BaseLevel>100.0)) Err |=0x01;// базовый уровень не очень
   for (int i=0; i<ARRAY_SIZE(UserMeasConfig.BaseLvl); ++i)
    if (isnan(UserMeasConfig.BaseLvl[i])||(UserMeasConfig.BaseLvl[i]<=-100.0)||(UserMeasConfig.BaseLvl[i]>100.0)) Err |=0x02;// базовый уровень длин волн калибровки
   for (int i=0; i<ARRAY_SIZE(UserMeasConfig.BaseSpecKlb); ++i)
    if (isnan(UserMeasConfig.BaseSpecKlb[i])||(UserMeasConfig.BaseSpecKlb[i]<=-100.0)||(UserMeasConfig.BaseSpecKlb[i]>100.0)) Err |=0x04;// базовый уровень специфических длин волн
   for (int i=0; i<ARRAY_SIZE(UserMeasConfig.Lim_LS); ++i)
    if (isnan(UserMeasConfig.Lim_LS[i])||(UserMeasConfig.Lim_LS[i]<=-100.0)||(UserMeasConfig.Lim_LS[i]>10.0)) Err |=0x08;// пределы контроля затуханий
   for (int i=0; i<ARRAY_SIZE(UserMeasConfig.Lim_ORL); ++i)
    if (isnan(UserMeasConfig.Lim_ORL[i])||(UserMeasConfig.Lim_ORL[i]<0.0)||(UserMeasConfig.Lim_ORL[i]>60.0)) Err |=0x10;// пределы контроля ORL
    if (isnan(UserMeasConfig.BaseLevelP2)||(UserMeasConfig.BaseLevelP2<=-100.0)||(UserMeasConfig.BaseLevelP2>100.0)) Err |=0x20;// базовый уровень P2 не очень
   for (int i=0; i<ARRAY_SIZE(UserMeasConfig.BaseLvlP2); ++i)
    if (isnan(UserMeasConfig.BaseLvlP2[i])||(UserMeasConfig.BaseLvlP2[i]<=-100.0)||(UserMeasConfig.BaseLvlP2[i]>100.0)) Err |=0x40;// базовый уровень длин волн калибровки P2
    if ((UserMeasConfig.CalibLnAutoMem<20000)||(UserMeasConfig.CalibLnAutoMem>20700)) Err |=0x80;//калибровка измерителя длины в автомате
// параметры установок при выключении
    if (UserMeasConfig.PMWaveLenghtMem>1700) Err |=0x100;//длина волны измерителя Р1
if ((UserMeasConfig.CondWorkMem==0)||(UserMeasConfig.CondWorkMem>9)) Err |=0x200;//текущий режим работы для востановления P1,P1a,Sc,P2 
if (UserMeasConfig.Ind_LWMem>2) Err |=0x400;//индекс длины волны источника из разрешенных
//uint8_t CondWorkMem; // текущий режим работы для востановления P1,P1a,Sc,P2 
//uint8_t ScModeMem; // режим источника, возможно надо выключать
//uint8_t Ind_LWMem; // индекс длины волны источника из разрешенных
if (UserMeasConfig.ModeIMem>2) Err |=0x800;//// режим индикации P1 
if (UserMeasConfig.ModeIP1aMem>2) Err |=0x1000;//// режим индикации P1a 
if (UserMeasConfig.P2ModeMem>1) Err |=0x2000;//// режим индикации P2 
if (UserMeasConfig.IndxP2LWMem>6) Err |=0x4000;//// индекс длины волны измерителя P2 калибровочные длины волн их 7
if (UserMeasConfig.ScModeMem>4) Err |=0x8000;//// индекс длины волны измерителя P2 калибровочные длины волн их 7
if (UserMeasConfig.AutoOff>1) Err |=0x10000;//// признак авто выключения...
if (UserMeasConfig.AutoOffMod>3) Err |=0x20000;//// признак авто выключения...

//uint8_t ModeIP1aMem; // режим индикации P1а
//uint8_t P2ModeMem; // режим индикации P2
//uint8_t IndxP2LWMem; //
    
    return Err;

}

void FixErrUserMC (uint32_t Err) //  фиксируем  (исправляем настройки)
{
  
    if (Err != 0)
  {
    if (Err & 0x01) UserMeasConfig.BaseLevel=-0.1; // 
    if (Err & 0x02) 
      for (int i=0; i<ARRAY_SIZE(UserMeasConfig.BaseLvl); ++i) UserMeasConfig.BaseLvl[i]=i;
    if (Err & 0x04) 
      for (int i=0; i<ARRAY_SIZE(UserMeasConfig.BaseSpecKlb); ++i) UserMeasConfig.BaseSpecKlb[i]=-i/2.;
    if (Err & 0x08)
      for (int i=0; i<ARRAY_SIZE(UserMeasConfig.Lim_LS); ++i) UserMeasConfig.Lim_LS[i]=-19.0+i;
    if (Err & 0x10)
      for (int i=0; i<ARRAY_SIZE(UserMeasConfig.Lim_ORL); ++i) UserMeasConfig.Lim_ORL[i]=26.0-i;
    if (Err & 0x20) UserMeasConfig.BaseLevelP2=-0.1; // 
    if (Err & 0x40) 
      for (int i=0; i<ARRAY_SIZE(UserMeasConfig.BaseLvlP2); ++i) UserMeasConfig.BaseLvlP2[i]=-i;
    if (Err & 0x80) UserMeasConfig.CalibLnAutoMem=20300; // 
    if (Err & 0x100) UserMeasConfig.PMWaveLenghtMem=1310; // 
    if (Err & 0x200) UserMeasConfig.CondWorkMem=P1; // 
    if (Err & 0x400) UserMeasConfig.Ind_LWMem=1; // 
    if (Err & 0x800) UserMeasConfig.ModeIMem=0; // 
    if (Err & 0x1000) UserMeasConfig.ModeIP1aMem=0; // 
    if (Err & 0x2000) UserMeasConfig.P2ModeMem=0; // 
    if (Err & 0x4000) UserMeasConfig.IndxP2LWMem=2; // 
    if (Err & 0x8000) UserMeasConfig.ScModeMem=0; // 
    if (Err & 0x10000) UserMeasConfig.AutoOff=0; // 
    if (Err & 0x20000) UserMeasConfig.AutoOffMod=0; // 
    
  }

}

uint32_t   FindErrSettingPrm (void)
{
  static uint32_t Err = 0;
     for (int i=0; i<ARRAY_SIZE(SettingPrm.PowSc); ++i)
    if (isnan(SettingPrm.PowSc[i])||(SettingPrm.PowSc[i]<=-20.0)||(SettingPrm.PowSc[i]>10.0)) Err |=0x01;// значения мощности излучения установленных лазеров
    if (SettingPrm.NumberFbrCount>59999) Err |=0x02; // счетчик номера волокна
    if (SettingPrm.EnaAutoInkrNF>1) Err |=0x04; // признак авто инкремента счетчика волокна
    if (SettingPrm.EnaCmpLOSS>1) Err |=0x08; // признак разрешения применять пороги при индикации затухания для 3 длин волн источников  
    if (SettingPrm.EnaCmpORL>1) Err |=0x10; // признак разрешения применять пороги при индикации ORL для 3 длин волн источников  
    if (SettingPrm.EnaWrtMem>7) Err |=0x20; // признак разрешения записи измерений в память 1бит-LOSS, 2-ORL, 3-Ln(длина)
    if (SettingPrm.CountMemory>512) Err |=0x40; // счетчик ячеек памяти 
    
    return Err;
}

void  FixErrSettingPrm (uint32_t Err) //  фиксируем  (исправляем настройки)
{
     if (Err != 0)
  {
    if (Err & 0x01) 
      for (int i=0; i<ARRAY_SIZE(SettingPrm.PowSc); ++i) SettingPrm.PowSc[i]=-i/10.;
    if (Err & 0x02) SettingPrm.NumberFbrCount = 0;
    if (Err & 0x04) SettingPrm.EnaAutoInkrNF = 0;
    if (Err & 0x08) SettingPrm.EnaCmpLOSS = 0;
    if (Err & 0x10) SettingPrm.EnaCmpORL = 0;
    if (Err & 0x20) SettingPrm.EnaWrtMem = 7; // при инициализации разрешаем запись сразу (требование А.К. 02.03.2021)
    if (Err & 0x40) SettingPrm.CountMemory = 0; // 
  }
 
}