void GetHeaderBelcore (char* Name, unsigned short Block, unsigned short NumEvents) // заполняем шапку белкора
{
  char Str[10];
  
  unsigned short DataShort ; // 
  unsigned long DataInt ;
  
      // при чтении переустанавливаем настройки прибора (поэтому перезаполняем длину волны на которой измеряли)
      switch (Block)
      {
      case 0: //0...55 byte
//        // первое заполнение надо прочитать файл
//      if (GetNumTrace()) // если не нулевая то читаем по таблице
//        TraceREAD(GetNumTraceSaved(GetNumTrace()));//  читаем файл который надо передать// 27/01/2011 неадекватно считывала рефлектограмму
//      else  TraceREAD(0);
      memcpy( Name, HeaderBelcore, 56 );
        if (NumEvents)
        {
          //memcpy( &Name[5], 7, 1);
          Name[2]=0x62; // запись нового размера Map блока +16
          Name[6]=7; // запись нового количества полей в Мар блоке
          memcpy( &Name[56], &HeaderBelcore[225], 16 ); // дозапись поля Событий в Мар блок
          DataShort = (unsigned short)(NumEvents*32+24); // получение размера блока в зависимости от числа событий
          memcpy( &Name[68], &DataShort, 2);
          
        }
        break;
      case 1: //56...117 (72...133) 62 byte
      memcpy( Name, &HeaderBelcore[56], 62 );
        // устанавливаем длину волны
      DataShort = (unsigned short)GetLengthWaveLS (GetPlaceLS(CURRENT)); // получение длины волны от индекса установочного места
      // перезапись длинны волны 1300 для х4 приборов!
      
      memcpy( &Name[86-56], &DataShort, 2);
      // заполняем Comments
      memcpy( &Name[98-56], CommentsOTDR, 19);
        break;
      case 2: //118...211 (134...227) 94 byte
      memcpy( Name, &HeaderBelcore[118], 94 );
      // заполняем производителя
      memcpy( &Name[118-118], IdnsBC[ConfigDevice.ID_Device], 14 );
      GetMFID( Str ); // for Belcore
      // Set MDIF
      memcpy( &Name[133-118], Str, 12 );
      // Number Device
      sprintf (Str,"%04d",ConfigDevice.NumDevice);
      memcpy( &Name[146-118], Str, 4 );
      // релиз программы
      GetNumVer(Str);
      memcpy( &Name[153-118], Str, 4 );
      
      // время сохранения рефлектограммы 
      // устанавливаем дату сохраненной рефлектограммы
      DataInt = TotalSec (TimeSaveOTDR);
      memcpy( &Name[159-118], &DataInt, 4);
      // (164)AW длина волны в 0.1 нм (1310нм = 13100) 0x332c 0x3c8c (164)
      DataShort = (unsigned short)GetLengthWaveLS (GetPlaceLS(CURRENT))*10;
      memcpy( &Name[165-118], &DataShort, 2);
      // длина зондирующего импульса 
      DataShort = GetWidthPulse(GetIndexIM());
      memcpy( &Name[173-118], &DataShort, 2);
      // устанавливаем DS
      // ((10000points*10(in 1ns 100ps))/2 = 50000 , 333.333 ns - интервал съема информации
      DataInt = (unsigned long)((ADCPeriod*50000)/NumPointsPeriod[GetIndexLN()]); //  устанавливаем значения DS для установленного режима измерения
      memcpy( &Name[175-118], &DataInt, 4);
      // ###(182) GI коэфф преломления  146583 (1.46583)  
      DataInt = (unsigned long)( GetIndexWAV()*100000);
      memcpy( &Name[183-118], &DataInt, 4);
      // ###(186) BC = 800 предел обратного отражения
      DataInt =  ReflParam.BC;
      memcpy( &Name[187-118], &DataInt, 2);
      // ###(188) NAV число накоплений - зависит от времени накопления и длины линии = FinAvrg
      DataInt = SettingRefl.NumAvrag;
      memcpy( &Name[189-118], &DataInt, 4);
      // ###(192) AR  длина измеряемого участка (грубо число измерений на шаг) DS*NPPW/10000
      DataInt = (unsigned long)((ADCPeriod*5*4096)/NumPointsPeriod[GetIndexLN()]); //  устанавливаем значения DS для установленного режима измерения
      memcpy( &Name[193-118], &DataInt, 4);
      // ###(200) NF нижний уровень шумов равен 65535
      DataInt =  ReflParam.NF;
      memcpy( &Name[201-118], &DataInt, 2);
      // ###(206) LT минимальное значение затухания для события 200 в 0.001 дБ
      DataInt =  ReflParam.LT;
      memcpy( &Name[207-118], &DataInt, 2);
      // ###(208) RT минимальное значение отражения для события 40000 в -0.001дБ
      DataInt =  ReflParam.RT;
      memcpy( &Name[209-118], &DataInt, 2);
      // ###(210) ET минимальное затухание для конца линии 3000 в 0.001 дБ
      DataInt =  ReflParam.ET;
      memcpy( &Name[211-118], &DataInt, 2);
        break;
      case 3: //212...223 (228...239) 12 byte
      memcpy( Name, &HeaderBelcore[213], 12 );
       break; 
      }

}