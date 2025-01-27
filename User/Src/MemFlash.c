
// Функции работы с памятью 
// взаимодействует с SSP

#include "system.h"
#include "ff.h"

char NameDir[100][6];
char NameFiles[1000][17];
uint32_t NumNameDir=0; // число имен директорий
uint32_t IndexNameDir=0;// индекс дирректории на которую указываем
uint32_t IndexLCDNameDir=0;// индекс указателя на индикаторе дирректории на которую указываем
uint32_t NumNameFiles=0; // число имен файлов
uint32_t IndexNameFiles=0;// индекс файла на который указываем
uint32_t IndexLCDNameFiles=0;// индекс указателя на индикаторе файла на который указываем
uint32_t PageDir; 
uint32_t PageFiles; // страница файлов

  float TmpACI;
St_File_Sor F_SOR; // содержимое основных параметров файла SOR

// для работы с FatFS и SDCard
  FATFS FatFs;
//  FIL Fil;
//  FRESULT FR_Status,res;
  FATFS *FS_Ptr;
  UINT RWC, WWC; // Read/Write Word Counter
  DWORD FreeClusters;
  FILINFO fno;
  DIR dir;
  UINT    br, bw;         // File R/W count

  char*   fn;
  const  char PathMainDir[9]={"0:/_OTDR\0"}; // 
  char PathF[64];
  //char   path=;
  uint32_t TotalSize, FreeSpace;
    char FileNameS[32]; // имя файла куда сохраняем
    char FileSDir[8]; // директория файла куда сохраняем
    char PathFileS[64]; // полный путь файла куда сохраняем
    
  char RW_Buffer[200];
  char TxBuffer[250]; // тестовый буффер передачи справочной нофрмации в UART3 (во внешний мир)
  // TxBuffer[0] = 0;
  // для подсчета контрольной суммы
   unsigned char *c;
  unsigned char value;
  unsigned short NumEventNow = 0; //без событий
  unsigned short old_crc = 0xffff; 
  unsigned short new_crc = 0xffff;

 
uint32_t StartInitSDcard (void)
{
  uint32_t ret=0;
  // попробуем проинициализировать SD Card
  // пустое подключение с первого раза не берет
  FR_Status = f_mount(&FatFs, SDPath, 1);
  HAL_Delay(2);
  
  FR_Status = f_mount(NULL, "", 0);
  //
  HAL_Delay(100);
  FR_Status = f_mount(&FatFs, SDPath, 1);
  if (FR_Status != FR_OK) // какие-то проблемы с SD Card
  {
    ret |= ERR_SDCard;
    //sprintf(TxBuffer, "Error! While Mounting SD Card, Error Code: (%i)\r\n", FR_Status);
    //UARTSendExt((void*)TxBuffer,strlen(TxBuffer)); // выдаем 
  }
  HAL_Delay(2);
  
  FR_Status = f_mkdir(PathMainDir);//0:/_OTDR
  if(FR_Status != FR_EXIST)
  {
    ret |= CLR_SDCard;
  }
  HAL_Delay(2);
  
  // почитаем директории...только что созданные 
  FR_Status = f_opendir(&dir, PathMainDir); //0:/_OTDR
  HAL_Delay(2);
  f_closedir(&dir);
  
  sprintf((void*)PathF,"TOP_N%04d",ConfigDevice.NumDevice);
  
  //FR_Status = f_setlabel(PathF);							/* Set volume label */
  HAL_Delay(2);
  
  
  FR_Status = f_mount(NULL, "", 0);
  // создаем или проверяем наличие дирректории _OTDR
  return ret;
}


void SDMMC_SDCard_FILES(void) // прочитаем список файлов в директории
{
  
  do
  {
    //------------------[ Mount The SD Card ]--------------------
    FR_Status = f_mount(&FatFs, SDPath, 1);
    if (FR_Status != FR_OK)
    {
      break;
    }
        HAL_Delay(2);

    // создаем или проверяем наличие дирректории _OTDR
    res = f_mkdir(PathMainDir);//"0:/_OTDR"
    if(res == FR_EXIST)
    {
      //sprintf ((char*)TxBuffer,"Make MainDir Already Is\r");
      res = FR_OK;
    }
//    // тестовое создание дирректории по дате, далее это на надо
//    GetFolder(FileNameS);
//    
//    sprintf(PathF,"%s/%s",PathMainDir,FileNameS);
//    res = f_mkdir(PathF);
//    //res = f_unlink(PathF);
//    if(res == FR_EXIST)
//    {
//      //sprintf ((char*)TxBuffer,"Make MainDir Already Is\r");
//      res = FR_OK;
//    }
//        HAL_Delay(2);
//    // почитаем директории...только что созданные 
//    res = f_opendir(&dir, PathF);
//        HAL_Delay(2);
//    f_closedir(&dir);
    
    
    //else
    //  sprintf ((char*)TxBuffer,"Make MainDir Ok\r");
    //UARTSendExt((void*)TxBuffer,strlen(TxBuffer)); // выдаем 
    // проверка объема флэшки
    //sprintf(TxBuffer, "00000\nSD Card Mounted Successfully! \r\n\n");
    // UARTSendExt((void*)TxBuffer,strlen(TxBuffer)); // выдаем 
    //------------------[ Get & Print The SD Card Size & Free Space ]--------------------
    //f_getfree("", &FreeClusters, &FS_Ptr);
    //TotalSize = (uint32_t)((FS_Ptr->n_fatent - 2) * FS_Ptr->csize * 0.5);
    //FreeSpace = (uint32_t)(FreeClusters * FS_Ptr->csize * 0.5);
    //sprintf(TxBuffer, "Total SD Card Size: %lu Bytes\r\n", TotalSize);
    //  UARTSendExt((void*)TxBuffer,strlen(TxBuffer)); // выдаем 
    //sprintf(TxBuffer, "Free SD Card Space: %lu Bytes\r\n\n", FreeSpace);
    //  UARTSendExt((void*)TxBuffer,strlen(TxBuffer)); // выдаем 
    
    NumNameFiles=0; // число имен 
    IndexNameFiles=0;// индекс файла на который указываем
    IndexLCDNameFiles=0;// индекс указателя на индикаторе файла на который указываем
    //NameDir[IndexNameDir][0] = 0; // обнулим массив  
    memset(&NameFiles,0,sizeof(NameFiles));
    // почитаем список файлов в данной директории (по указателю на директорию берем путь к файлам...
          //создадим полны путь к директории  чтобы её открыть
    sprintf(PathFileS,"%s/%s",PathMainDir,NameDir[IndexNameDir]);
   
    res = f_opendir(&dir, PathFileS);
    if(res == FR_OK)
    {
      while(1)
      {
        res = f_readdir(&dir, &fno);
        
        if(res != FR_OK || fno.fname[0] == 0) // нет дирректорий выходим
          break;
        
        //      if(fno.fname[0] != 0) // вроде это имена файлов
        //      {
        //        fn = fno.fname;
        //              sprintf(TxBuffer, "file/%s\r",fn);
        //      UARTSendExt((void*)TxBuffer,strlen(TxBuffer)); // выдаем 
        //        
        //      }
        //      
        //if(fno.fname[0] == '.')
        //{ // выводим имена дирректорий
        //continue;
        
        fn = fno.fname; 
        
        // поробуем найти файлы
        //1) есть в нужном месте _ и .sor
          if((strlen(fn) == 16)&&(fn[6]=='_')&&(fn[12]=='.')&&(fn[13]=='s')&&(fn[14]=='o')&&(fn[15]=='r')) // наш размер теперь проверим содержимое
          {
            // 2) проверим что в имени цифры
                       // проверка на состав имени файла
            //if((fn[0]>='0'&&fn[0]<='9')&&(fn[1]>='0'&&fn[1]<='9')&&(fn[2]>='0'&&fn[2]<='9')&&(fn[3]>='0'&&fn[3]<='9')&&(fn[4]>='0'&&fn[4]<='9')&&(fn[5]>='0'&&fn[5]<='9')&&(fn[7]>='0'&&fn[7]<='9')&&(fn[8]>='0'&&fn[8]<='9')&&(fn[9]>='0'&&fn[9]<='9')&&(fn[10]>='0'&&fn[10]<='9')&&(fn[11]>='0'&&fn[11]<='9'))
            //{
              memcpy( &NameFiles[NumNameFiles],fn,16);
              NameFiles[NumNameFiles][16]=0;
              NumNameFiles++;
              //            sprintf(TxBuffer, "dir/%s\r",fn);
            //}
 
          }
//        if((fno.fattrib & AM_DIR) == AM_DIR) // дирректории
//        {
//          // это директории - надо проверить имя и записть в список
//          if((strlen(fn) == 5)&&(fn[2]=='_')) // наш размер теперь проверим содержимое
//          {
//            // проверка на состав имени директории
//            if((fn[0]>='0'&&fn[0]<='9')&&(fn[1]>='0'&&fn[1]<='9')&&(fn[3]>='0'&&fn[3]<='9')&&(fn[4]>='0'&&fn[4]<='9'))
//            {
//              memcpy( &NameDir[NumNameDir],fn,5);
//              NameDir[NumNameDir][5]=0;
//              NumNameDir++;
//              //            sprintf(TxBuffer, "dir/%s\r",fn);
//            }
//          }
//          else
//            TxBuffer[0] = 0;
//          
//        }
        //        else // файлы,
        //        {
        //                        sprintf(TxBuffer, "file/%s\r",fn);
        //
        //        }
        //      UARTSendExt((void*)TxBuffer,strlen(TxBuffer)); // выдаем 
        
        //}
      }
    }
    
    f_closedir(&dir);
    
    
  } while(0);
  //------------------[ Test Complete! Unmount The SD Card ]--------------------
  FR_Status = f_mount(NULL, "", 0);
}

// Новая система работы с файлами, они хранятся на SD card
//Чужая программа теста работы FATFS in SDMMC2 with SD_Card
void SDMMC_SDCard_DIR(void) // прочитаем дирректроии
{
  do
  {
    //------------------[ Mount The SD Card ]--------------------
    FR_Status = f_mount(&FatFs, SDPath, 1);
    if (FR_Status != FR_OK)
    {
      break;
    }
        HAL_Delay(2);

    // создаем или проверяем наличие дирректории _OTDR
    res = f_mkdir(PathMainDir);//"0:/_OTDR"
    if(res == FR_EXIST)
    {
      //sprintf ((char*)TxBuffer,"Make MainDir Already Is\r");
      res = FR_OK;
    }
//    // тестовое создание дирректории по дате, далее это на надо
//    GetFolder(FileNameS);
//    
//    sprintf(PathF,"%s/%s",PathMainDir,FileNameS);
//    res = f_mkdir(PathF);
//    //res = f_unlink(PathF);
//    if(res == FR_EXIST)
//    {
//      //sprintf ((char*)TxBuffer,"Make MainDir Already Is\r");
//      res = FR_OK;
//    }
//        HAL_Delay(2);
//    // почитаем директории...только что созданные 
//    res = f_opendir(&dir, PathF);
//        HAL_Delay(2);
//    f_closedir(&dir);
    
    
    //else
    //  sprintf ((char*)TxBuffer,"Make MainDir Ok\r");
    //UARTSendExt((void*)TxBuffer,strlen(TxBuffer)); // выдаем 
    // проверка объема флэшки
    //sprintf(TxBuffer, "00000\nSD Card Mounted Successfully! \r\n\n");
    // UARTSendExt((void*)TxBuffer,strlen(TxBuffer)); // выдаем 
    //------------------[ Get & Print The SD Card Size & Free Space ]--------------------
    //f_getfree("", &FreeClusters, &FS_Ptr);
    //TotalSize = (uint32_t)((FS_Ptr->n_fatent - 2) * FS_Ptr->csize * 0.5);
    //FreeSpace = (uint32_t)(FreeClusters * FS_Ptr->csize * 0.5);
    //sprintf(TxBuffer, "Total SD Card Size: %lu Bytes\r\n", TotalSize);
    //  UARTSendExt((void*)TxBuffer,strlen(TxBuffer)); // выдаем 
    //sprintf(TxBuffer, "Free SD Card Space: %lu Bytes\r\n\n", FreeSpace);
    //  UARTSendExt((void*)TxBuffer,strlen(TxBuffer)); // выдаем 
    uint32_t OldIndexNameDir = IndexNameDir;
    NumNameDir=0; // число имен директорий
    IndexNameDir=0;// индекс дирректории на которую указываем
    IndexLCDNameDir=0;// индекс указателя на индикаторе дирректории на которую указываем
    //NameDir[IndexNameDir][0] = 0; // обнулим массив  
    memset(&NameDir,0,sizeof(NameDir));
    // почитаем директории...
    res = f_opendir(&dir, PathMainDir);
    if(res == FR_OK)
    {
      while(1)
      {
        res = f_readdir(&dir, &fno);
        
        if(res != FR_OK || fno.fname[0] == 0) // нет дирректорий выходим
          break;
        
        //      if(fno.fname[0] != 0) // вроде это имена файлов
        //      {
        //        fn = fno.fname;
        //              sprintf(TxBuffer, "file/%s\r",fn);
        //      UARTSendExt((void*)TxBuffer,strlen(TxBuffer)); // выдаем 
        //        
        //      }
        //      
        //if(fno.fname[0] == '.')
        //{ // выводим имена дирректорий
        //continue;
        
        fn = fno.fname; 
        
        if((fno.fattrib & AM_DIR) == AM_DIR) // дирректории
        {
          // это директории - надо проверить имя и записть в список
          if((strlen(fn) == 5)&&(fn[2]=='_')) // наш размер теперь проверим содержимое
          {
            // проверка на состав имени директории
            if((fn[0]>='0'&&fn[0]<='9')&&(fn[1]>='0'&&fn[1]<='9')&&(fn[3]>='0'&&fn[3]<='9')&&(fn[4]>='0'&&fn[4]<='9'))
            {
              memcpy( &NameDir[NumNameDir],fn,5);
              NameDir[NumNameDir][5]=0;
              NumNameDir++;
              //            sprintf(TxBuffer, "dir/%s\r",fn);
            }
          }
          else
            TxBuffer[0] = 0;
          
        }
        //        else // файлы,
        //        {
        //                        sprintf(TxBuffer, "file/%s\r",fn);
        //
        //        }
        //      UARTSendExt((void*)TxBuffer,strlen(TxBuffer)); // выдаем 
        
        //}
      }
    }
    if(OldIndexNameDir <= NumNameDir)
      IndexNameDir = OldIndexNameDir;
    f_closedir(&dir);
    
    
    //    //------------------[ Open A Text File For Write & Write Data ]--------------------
    //    //Open the file
    //    //FR_Status = f_open(&Fil, "MyTextFile.txt", FA_WRITE | FA_READ | FA_CREATE_ALWAYS);
    //    sprintf(FileNameS,"MyFile%03d.txt",Num);
    //    FR_Status = f_open(&Fil, FileNameS, FA_WRITE | FA_READ | FA_CREATE_ALWAYS);
    //    if(FR_Status != FR_OK)
    //    {
    //      sprintf(TxBuffer, "Error! While Creating/Opening A New Text File, Error Code: (%i)\r\n", FR_Status);
    //      HAL_UART_Transmit(&huart3, (void*)TxBuffer,strlen(TxBuffer),50); // выдаем 
    //      break;
    //    }
    //    sprintf(TxBuffer, "Text File Created & Opened! Writing Data To The Text File..\r\n\n");
    //    HAL_UART_Transmit(&huart3, (void*)TxBuffer,strlen(TxBuffer),100); // выдаем 
    //    // (1) Write Data To The Text File [ Using f_puts() Function ]
    //    f_puts("Hello! From STM32 To SD Card Over SDMMC, Using f_puts()\n", &Fil);
    //    // (2) Write Data To The Text File [ Using f_write() Function ]
    //    strcpy(RW_Buffer, "Hello! From STM32 To SD Card Over SDMMC, Using f_write()\r\n");
    //    f_write(&Fil, RW_Buffer, strlen(RW_Buffer), &WWC);
    //    // Close The File
    //    f_close(&Fil);
    //    //------------------[ Open A Text File For Read & Read Its Data ]--------------------
    //    // Open The File
    //    //FR_Status = f_open(&Fil, "MyTextFile.txt", FA_READ);
    //    FR_Status = f_open(&Fil, FileNameS, FA_READ);
    //    if(FR_Status != FR_OK)
    //    {
    //      sprintf(TxBuffer, "Error! While Opening (MyTextFile.txt) File For Read.. \r\n");
    //      HAL_UART_Transmit(&huart3, (void*)TxBuffer,strlen(TxBuffer),50); // выдаем 
    //      break;
    //    }
    //    // (1) Read The Text File's Data [ Using f_gets() Function ]
    //    f_gets(RW_Buffer, sizeof(RW_Buffer), &Fil);
    //    sprintf(TxBuffer, "Data Read From (MyTextFile.txt) Using f_gets():%s", RW_Buffer);
    //    //sprintf(TxBuffer, "Data Read From (%s) Using f_gets():%s",FileNameS, RW_Buffer);
    //    HAL_UART_Transmit(&huart3, (void*)TxBuffer,strlen(TxBuffer),50); // выдаем 
    //    // (2) Read The Text File's Data [ Using f_read() Function ]
    //    f_read(&Fil, RW_Buffer, f_size(&Fil), &RWC);
    //    sprintf(TxBuffer, "Data Read From (%s) Using f_read():%s",FileNameS, RW_Buffer);
    //    HAL_UART_Transmit(&huart3, (void*)TxBuffer,strlen(TxBuffer),50); // выдаем 
    //    // Close The File
    //    f_close(&Fil);
    //    sprintf(TxBuffer, "File Closed! \r\n\n");
    //    HAL_UART_Transmit(&huart3, (void*)TxBuffer,strlen(TxBuffer),50); // выдаем 
    //    //------------------[ Open An Existing Text File, Update Its Content, Read It Back ]--------------------
    //    // (1) Open The Existing File For Write (Update)
    //    //FR_Status = f_open(&Fil, "MyTextFile.txt", FA_OPEN_EXISTING | FA_WRITE);
    //    FR_Status = f_open(&Fil, FileNameS, FA_READ); // Open The File For Read
    //    FR_Status = f_lseek(&Fil, f_size(&Fil)); // Move The File Pointer To The EOF (End-Of-File)
    //    if(FR_Status != FR_OK)
    //    {
    //      sprintf(TxBuffer, "Error! While Opening (MyTextFile.txt) File For Update.. \r\n");
    //      HAL_UART_Transmit(&huart3, (void*)TxBuffer,strlen(TxBuffer),50); // выдаем 
    //      break;
    //    }
    //    // (2) Write New Line of Text Data To The File
    //    FR_Status = f_puts("This New Line Was Added During File Update!\r\n", &Fil);
    //    f_close(&Fil);
    //    memset(RW_Buffer,'\0',sizeof(RW_Buffer)); // Clear The Buffer
    //    // (3) Read The Contents of The Text File After The Update
    //    //FR_Status = f_open(&Fil, "MyTextFile.txt", FA_READ); // Open The File For Read
    //    FR_Status = f_open(&Fil, FileNameS, FA_READ); // Open The File For Read
    //    f_read(&Fil, RW_Buffer, f_size(&Fil), &RWC);
    //    sprintf(TxBuffer, "Data Read From (MyTextFile.txt) After Update:\r\n%s", RW_Buffer);
    //    HAL_UART_Transmit(&huart3, (void*)TxBuffer,strlen(TxBuffer),50); // выдаем 
    //    f_close(&Fil);
    //    //------------------[ Delete The Text File ]--------------------
    //    // Delete The File
    //    /*
    //    FR_Status = f_unlink(MyTextFile.txt);
    //    if (FR_Status != FR_OK){
    //        sprintf(TxBuffer, "Error! While Deleting The (MyTextFile.txt) File.. \r\n");
    //        USC_CDC_Print(TxBuffer);
    //    }
    //    */
  } while(0);
  //------------------[ Test Complete! Unmount The SD Card ]--------------------
  FR_Status = f_mount(NULL, "", 0);
  // думал не закрывать флэшку
  //        sprintf(TxBuffer, "\r\nSD Card NO   Un-mounted Successfully! \r\n");
  //    HAL_UART_Transmit(&huart3, (void*)TxBuffer,strlen(TxBuffer),50); // выдаем 
//  if (FR_Status != FR_OK)
//
//  {
//      sprintf(TxBuffer, "\r\nError! While Un-mounting SD Card, Error Code: (%i)\r\n", FR_Status);
//      HAL_UART_Transmit(&huart3, (void*)TxBuffer,strlen(TxBuffer),50); // выдаем 
//  } else{
//      sprintf(TxBuffer, "\r\nSD Card Un-mounted Successfully! \r\n");
//      HAL_UART_Transmit(&huart3, (void*)TxBuffer,strlen(TxBuffer),50); // выдаем 
//  }
}

// структура конфигурации прибора

BYTE MemTable[MaxMemOTDR+1]; // таблица рефлектограмм ячейки памяти меняется в памяти до MaxMemOTDR
BYTE MemTableExt[MaxMemOTDRExt+1]; // таблица рефлектограмм расширенной ячейки памяти меняется в памяти до MaxMemOTDRExt
static unsigned short NumTrace = 0; // номер трассы (число записей) в памяти рефлектограм

BYTE CheckMemID (void) // прочитаем какая флэшка стоит
{
  BYTE ID_Flash = 0;
//  BYTE Data_Flash_Devices[4];
//  FlashReadID ((unsigned char*)&Data_Flash_Devices);
//  ID_Flash = Data_Flash_Devices[1]&0x1f;
//  if (ID_Flash == 7) ID_Flash = 1;
//  else ID_Flash = 0;
  return ID_Flash;
}


DWORD CheckMemFlash (void)
{ 
  DWORD SPCTR_err=0;
// // чтение и инициализация альтернативного имени прибора
//  FlashReadPageNum (DBNAMESTRUCT, StructPtr(DBNAMESTRUCT), StructSize(DBNAMESTRUCT),0);
//  unsigned ErrPramDBNAME = InvalidDBNAME();
//  if (ErrPramDBNAME)
//  {
//    //SPCTR_err=0x80;
//    InitDBNAME(ErrPramDBNAME); 
//    FlashErasePage(DBNAMESTRUCT); // чистим страницу хранящую конфигурацию alternate name прибора
//    FlashWritePageSM(DBNAMESTRUCT, StructPtr(DBNAMESTRUCT), StructSize(DBNAMESTRUCT), 0);
//  }
// // чтение и инициализация конфирурации совместимости JDSU
//  FlashReadPageNum (JDSUSTRUCT, StructPtr(JDSUSTRUCT), StructSize(JDSUSTRUCT),0);
//  unsigned ErrPramJDSU = InvalidJDSU();
//  if (ErrPramJDSU)
//  {
//    //SPCTR_err=0x80;
//    InitJDSU(ErrPramJDSU); 
//    FlashErasePage(JDSUSTRUCT); // чистим страницу хранящую конфигурацию прибора
//    FlashWritePageSM(JDSUSTRUCT, StructPtr(JDSUSTRUCT), StructSize(JDSUSTRUCT), 0);
//  }
//  
//  // функция контроля конфигурации прибора (проверка)
//  FlashReadPageNum (CFG_DEVICE, StructPtr(CFG_DEVICE), StructSize(CFG_DEVICE),0);
//  unsigned ErrDev = InvalidDevice();
//  if (ErrDev)
//  {
//    SPCTR_err=0x80;
//    InitDevice(ErrDev); 
//    FlashErasePage(CFG_DEVICE); // чистим страницу хранящую конфигурацию прибора
//    FlashWritePageSM(CFG_DEVICE, StructPtr(CFG_DEVICE), StructSize(CFG_DEVICE), 0);
//  }
//  // проверка спектральных коэффициентов
//  FlashReadPageNum (COEF_PM, StructPtr(COEF_PM), StructSize(COEF_PM), 0);
//  SPCTR_err = SPCTR_err + FindErrCoeff ();
//  FixErrCoeff (SPCTR_err); //  проверка таблицы коэффициентов и исправление таблицы коэффициентов
//  if (SPCTR_err)
//  {
//  FlashErasePage(COEF_PM); // чистим страницу хранящую спектральные коэфф. прибора
//  FlashWritePageSM(COEF_PM, StructPtr(COEF_PM), StructSize(COEF_PM), 0);
//  }
//  FlashReadPageNum (CFG_USER, StructPtr(CFG_USER), StructSize(CFG_USER),0);
//  SPCTR_err = SPCTR_err + CheckUserGonfig ();  // Проверка пользовательских настроек 
//  if (SPCTR_err>=0x100)
//  {
//  FlashErasePage(CFG_USER); // чистим страницу установок пользователя прибора
//  FlashWritePageSM(CFG_USER, StructPtr(CFG_USER), StructSize(CFG_USER), 0);
//  }
//  // проверка блока установок контроля рефлектограммы
//  FlashReadPageNum (EVEN_SET, StructPtr(EVEN_SET), StructSize(EVEN_SET),0);
//  DWORD Param_err =  CheckReflParam ();  // Проверка пользовательских настроек 
//  if (Param_err>0)
//  {
//  FlashErasePage(EVEN_SET); // чистим страницу установок пользователя прибора
//  FlashWritePageSM(EVEN_SET, StructPtr(EVEN_SET), StructSize(EVEN_SET), 0);
//  }
//  
  return SPCTR_err;
}

void SaveConfigDevice (void)
{
//    FlashErasePage(CFG_DEVICE); // чистим страницу хранящую конфигурацию прибора
//    FlashWritePageSM(CFG_DEVICE, StructPtr(CFG_DEVICE), StructSize(CFG_DEVICE), 0);
}

void SaveUserConfig (void) // сохранение пользовательских настроек
{
//  FlashErasePage(CFG_USER); // чистим страницу установок пользователя прибора
//  FlashWritePageSM(CFG_USER, StructPtr(CFG_USER), StructSize(CFG_USER), 0);
}

// очистка рефлектограммы
void TraceERASE(unsigned int Trace)
{
//  int i;
//  //if (Trace <= ((CheckIDMEM)?(0x100+MaxMemOTDRExt):(MaxMemOTDR)))
//  if(CHECKNUMTRACE)
//  {
//    for (i=0; i<NUMPAGESFORREF; i++)
//    {
//      FlashErasePage(Trace*NUMPAGESFORREF+i+1);
//    }
//  }
}
;
// при записи рефлектограммы записываем 16 блоков по 528 байт из них 512 данные рефлектограммы 
// + 16 вспомогательные данные 0-блок = текущее время 1-2 блок = комментарии к файлу
// 15 блок - установочные параметры
void TraceWRITE(unsigned int Trace)
{
//  int i;
//  //extern unsigned char Index_set ; // индекс режима работы (0=выбор, 1=10км, 2=20км, 3=40км, 4=80км) 
//  //extern unsigned char Index_Ln ; // индекс значений длины линии в пользовательском режиме
//  //extern unsigned char Index_Im ; // индекс значений длительности импульса 
//  //extern unsigned char Index_Vrm ; // индекс значений времени работы 
//  //extern unsigned char ds1337 ; // индекс значения таймера реального времени  
//  //extern float K_pr; 
//  unsigned long DateFileName = GetSetCntFiles (0);
//  //if (Trace <= ((CheckIDMEM)?(0x100+MaxMemOTDRExt):(MaxMemOTDR)))
//      if(CHECKNUMTRACE)
//  {
//
//    for (i=0; i<NUMPAGESFORREF; i++)
//    {
//      //SPI1_write_page(Trace*16+i+1, (unsigned char*)&Dat[128*i],512,0);
//      // для 2 байтовой записи в тоже поле новый цикл записи по 2 байта 
//      FlashWritePageTrBuf(Trace*NUMPAGESFORREF+i+1, (unsigned char*)&LogData[256*i]);
//      switch(i)
//      {
//      case 0: // запись текущего времени (первые 4 слова структуры времени)
//        FlashWritePageSM(Trace*NUMPAGESFORREF+i+1, (unsigned char*)&TimeSaveOTDR,16,512);
//      break;
//      case 1:       // запись текущего времени (первые 4 слова структуры времени)
//        FlashWritePageSM(Trace*NUMPAGESFORREF+i+1, (unsigned char*)&TimeSaveOTDR+16,16,512);
//      break;
//      case 2: // запись блока комментариев
//        FlashWritePageSM(Trace*NUMPAGESFORREF+i+1, (unsigned char*)&CommentsOTDR,10,512);
//      break;
//      case 3:
//        
//        FlashWritePageSM(Trace*NUMPAGESFORREF+i+1, (unsigned char*)&CommentsOTDR[10],10,512);
//      // сохраняем сквозной номер сохраненной рефлектограммы
//      break;
//      case 4:
//        FlashWritePageSM(Trace*NUMPAGESFORREF+i+1, (unsigned char*)&DateFileName,4,512);
//      // сохраняем ORL в случае новой прошивки и если делали измерение! А так пишем что-то
//      break;
//      case 5:
//        FlashWritePageSM(Trace*NUMPAGESFORREF+i+1, (unsigned char*)&g_VolORL,4,512);
//      break;
//      // 17.11.2022 сохраняем вариант установки выбора длин волн при измерении
//      case 6:
//        FlashWritePageSM(Trace*NUMPAGESFORREF+i+1, (unsigned char*)&g_SetModeLW,1,512);
//      break;
//      case 15:
//        //в последние значения пишем переменные измерения 
//        FlashWritePageSM(Trace*NUMPAGESFORREF+i+1, (unsigned char*)&SettingRefl,16,512);
//        //Index_Ln  // индекс значений длины линии в пользовательском режиме
//        // Index_Im // индекс значений длительности импульса 
//        // Index_Vrm  // индекс значений времени работы 
//        // SW_LW  // признак длинны волны лазера 
//        // K_pr  // коэфф преломления 
//        // Index_Comm  // местоположения курсора в комментариях 
//        // SubModeRef; // рефлектометр авто или нет ( для востановления после включения)
//        // NumAverag; // число накоплений (реальные по данным)
//      break;
//      //
//      default:
//        break;
//      }
//    }
//  }
}
;
// аварийная перезапись нулевой рефлекторграммы
void AlarmReWriteTrace (void)
{
//  //TraceREAD(0);
//  //CheckSavedTrace();
//  //clear Buf 
//  memset( LogData, 43690, OUTSIZE * sizeof(unsigned short) ); // ZERO_ARRAY( array ) memset( array, 0, sizeof(array) )
//  // Set Comments
//  sprintf(CommentsOTDR,"Empty              ");
//  TimeSaveOTDR = RTCGetTime(); // сохраняем время сбора
//  TraceERASE(0);
//  TraceWRITE(0);
//  TraceREAD(0);
}
// при записи рефлектограммы записываем 16 блоков по 528 байт из них 512 данные рефлектограммы 
// запись 3 сек рефлектограммы при измерении "мощными" импульсами, измеряем на 1000нс, для склейки перегуженной зоны 
// пишем как бы в 250 рефлектограмму (4001 pg...4016)
void SpliceWRITE(unsigned int Trace) 
{
//  int i;
//  if (Trace == 250)
//  {
//    for (i=0; i<NUMPAGESFORREF; i++)
//    {
//      //SPI1_write_page(Trace*16+i+1, (unsigned char*)&Dat[128*i],512,0);
//      // для 2 байтовой записи в тоже поле новый цикл записи по 2 байта 
//      FlashWritePageTrBuf(Trace*NUMPAGESFORREF+i+1, (unsigned char*)&LogData[256*i]);
//    }
//  }
}
;
// почитаем текущую точку Splice рефлекограммы
unsigned short FlashReadSpliceData (unsigned int NumData)
{
    unsigned short OUT_Data = 0;
//    unsigned long PgSplice = 4001;
//    PgSplice  = PgSplice + (NumData>>8);
//  unsigned long CmdSqn = PgSplice << 10; // адрес страницы 
//  CmdSqn = CmdSqn + ((NumData & 0xFF)<<1); // байт положение в странице
//  CmdSqn = CmdSqn + 0xD2000000; //команда 
//  FL_CS(1,0); // CS_0
//  SSP_FlashWriteLong( CmdSqn );
//  SSP_FlashWriteLong( 0 );
//   OUT_Data=SSP_FlashTransmit(1, 0)+\
//     SSP_FlashTransmit(1, 0)*256;
//  
// 
//  FL_CS(1,1); // CS_1
  return OUT_Data;
}


// читаем страницу сохраненной рефлектограммы
void FlashReadPageTrace (unsigned int NumPg, unsigned char *adress, int Block)
{
//    unsigned char *c;
//    int i;
//    BYTE CheckSmb;
//  unsigned long CmdSqn =(unsigned long)NumPg << 10; // команда для передачи для AT45DB161
//  CmdSqn = CmdSqn + 0xD2000000; //команда 
//  FL_CS(1,0); // CS_0
//  SSP_FlashWriteLong( CmdSqn );
//  SSP_FlashWriteLong( 0 );
//   for (int i=0; i<512; ++i)
// {
//   *adress++=SSP_FlashTransmit(1, 0);
// }
//   FL_CS(1,1); // CS_0
//    CreatDelay (40000); // 3.3 мС
//   CmdSqn =(unsigned long)NumPg << 10;
//  CmdSqn = CmdSqn + 0xD2000200; //команда 
//   FL_CS(1,0); // CS_0
//  SSP_FlashWriteLong( CmdSqn );
//  SSP_FlashWriteLong( 0 );
//
// if (Block==0)
// {
//   c = (unsigned char*)&TimeSaveOTDR;
//   for (i=0; i<16; i++)
//   {
//     *c++=SSP_FlashTransmit(1, 0);
//   }
// }
// if (Block==1)
// {
//   c = (unsigned char*)&TimeSaveOTDR+16;
//   for (i=0; i<16; i++)
//   {
//     *c++=SSP_FlashTransmit(1, 0);
//   }
// }
// if (Block==2)
// {
//   c = (unsigned char*)&CommentsOTDR;
//   for (i=0; i<10; i++)
//   {
//     CheckSmb=SSP_FlashTransmit(1, 0);
//     if (CheckSmb < 0x20) CheckSmb =0x20;
//     *c++ = CheckSmb;
////     *c++=SSP_FlashTransmit(1, 0);
//   }
// }
// if (Block==3)
// {
//   c = (unsigned char*)&CommentsOTDR[10];
//   for (i=0; i<10; i++)
//   {
//     CheckSmb=SSP_FlashTransmit(1, 0);
//     if (CheckSmb < 0x20) CheckSmb =0x20;
//     *c++ = CheckSmb;
//     
//     //*c++=SSP_FlashTransmit(1, 0);
//   }
//   CommentsOTDR[19]=0; // принудительно зануляем строку
// }
// // порядковый номер текущего файла (читаем из памяти)
// if (Block==4)
// {
//   c = (unsigned char*)&CurCountFiles;
//   for (i=0; i<4; i++)
//   {
//     *c++=SSP_FlashTransmit(1, 0);
//   }
// }
// if (Block==5) // чтение ORl (если записано) (может быть inf)
// {
//   c = (unsigned char*)&g_VolORL;
//   for (i=0; i<4; i++)
//   {
//     *c++=SSP_FlashTransmit(1, 0);
//   }
//   // проверка на inf и диапазон
//       if (isnan(g_VolORL)||(g_VolORL<14.8)||(g_VolORL>70.0))
//         g_VolORL = 0.0;
// }
// if (Block==6) // чтение g_SetModeLW (BYTE) 17.11.2022
// {
//   c = (unsigned char*)&mg_SetModeLW;
//   for (i=0; i<1; i++)
//   {
//     *c++=SSP_FlashTransmit(1, 0);
//   }
//   // проверка на inf и диапазон
//       if (mg_SetModeLW >7)
//         mg_SetModeLW = 0;
// }
// if (Block==15)
// {
//   c = (unsigned char*)&SettingRefl;
//   for (i=0; i<16; i++)
//   {
//     *c++=SSP_FlashTransmit(1, 0);
//   }
// }
//  FL_CS(1,1); // CS_1
}
// читаем комментарии сохраненной рефлектограммы по номеру
// читаем страницу сохраненной рефлектограммы
unsigned long FlashReadCommTrace (unsigned int Num, unsigned char *adress)
{
//    unsigned char *c;
//    BYTE CheckSmb;
  unsigned long NumWrSq = Num; // номер записи
//  unsigned long NumPg = GetNumTraceSaved(Num)*NUMPAGESFORREF+3; // сразу указатель на 2 блок где комментарии
//  unsigned long CmdSqn =(unsigned long)NumPg << 10; // команда для передачи для AT45DB161
//  CmdSqn = CmdSqn + 0xD2000200; //команда чтения блока с 512 байта 
//  FL_CS(1,0); // CS_0
//  SSP_FlashWriteLong( CmdSqn );
//  SSP_FlashWriteLong( 0 );
//   for (int i=0; i<10; ++i)
// {
//     CheckSmb=SSP_FlashTransmit(1, 0);
//     if (CheckSmb < 0x20) CheckSmb =0x20;
//   *adress++= CheckSmb;
//   //*adress++=SSP_FlashTransmit(1, 0);
// }
//  FL_CS(1,1); // CS_1
//  NumPg++;
//  CmdSqn =(unsigned long)NumPg << 10; // команда для передачи для AT45DB161
//  CmdSqn = CmdSqn + 0xD2000200; //команда чтения блока с 512 байта 
//  FL_CS(1,0); // CS_0
//  SSP_FlashWriteLong( CmdSqn );
//  SSP_FlashWriteLong( 0 );
//   for (int i=0; i<10; ++i)
// {
//     CheckSmb=SSP_FlashTransmit(1, 0);
//     if (CheckSmb < 0x20) CheckSmb =0x20;
//   *adress++= CheckSmb;
//   //*adress++=SSP_FlashTransmit(1, 0);
// }
// *adress--;
// *adress=0;
//  FL_CS(1,1); 
//  NumPg++;
//  CmdSqn =(unsigned long)NumPg << 10; // команда для передачи для AT45DB161
//  CmdSqn = CmdSqn + 0xD2000200; //команда чтения блока с 512 байта 
//  FL_CS(1,0); // // CS_0 4 страница данные о номере записи
//  SSP_FlashWriteLong( CmdSqn );
//  SSP_FlashWriteLong( 0 );
//     c = (unsigned char*)&NumWrSq;
//   for (int i=0; i<4; i++)
//   {
//     *c++=SSP_FlashTransmit(1, 0);
//   }
//  FL_CS(1,1); // CS_1
  if (NumWrSq > 0x100000) NumWrSq = (unsigned long)Num;

  return NumWrSq;
}

// чтение времени сохраненной рефлектограммы(поле минут)
unsigned long FlashReadTimeTrace (unsigned int Num)
{
  unsigned long NumPg =1; // сразу указатель на 2 блок где комментарии

//  unsigned char *c;
//  // время сохраненной рефлектограмм
//    RTCTime TimeReadOTDR;
//  unsigned long NumWrSq; // получение беззнакового целего поля минут ( для анализа писалось туда что нить или нет)
//  unsigned long NumPg = GetNumTraceSaved(Num)*NUMPAGESFORREF+1; // сразу указатель на 2 блок где комментарии
//  unsigned long CmdSqn =(unsigned long)NumPg << 10; // команда для передачи для AT45DB161
//  CmdSqn = CmdSqn + 0xD2000200; //команда чтения блока с 512 байта 
//  FL_CS(1,0); // CS_0
//  SSP_FlashWriteLong( CmdSqn );
//  SSP_FlashWriteLong( 0 );
//   c = (unsigned char*)&TimeReadOTDR;
//   for (int i=0; i<16; i++)
//   {
//     *c++=SSP_FlashTransmit(1, 0);
//   }
//  FL_CS(1,1); // CS_1 прочитали перый блок
//  NumPg++;
//  CmdSqn =(unsigned long)NumPg << 10; // команда для передачи для AT45DB161
//  CmdSqn = CmdSqn + 0xD2000200; //команда чтения блока с 512 байта 
//  FL_CS(1,0); // CS_0
//  SSP_FlashWriteLong( CmdSqn );
//  SSP_FlashWriteLong( 0 );
//   c = (unsigned char*)&TimeReadOTDR+16;
//   for (int i=0; i<16; i++)
//   {
//     *c++=SSP_FlashTransmit(1, 0);
//   }
//  FL_CS(1,1); // CS_1 прочитали перый блок
//  //
//  sprintf(NameReadFile,"%02d%02d%02d_%02d%02d%01d.sor",TimeReadOTDR.RTC_Year%100,
//          TimeReadOTDR.RTC_Mon,
//          TimeReadOTDR.RTC_Mday,
//          TimeReadOTDR.RTC_Hour,
//          TimeReadOTDR.RTC_Min,
//          TimeReadOTDR.RTC_Sec/10 );
  
  return NumPg;
}

// чтение рефлектограммы
void TraceREAD(unsigned int Trace)
{
//  int i;
//  //if (Trace <= MaxMemOTDR)
//    if(CHECKNUMTRACE)
//  {
//    for (i=0; i<NUMPAGESFORREF; i++)
//    {
//      FlashReadPageTrace(Trace*NUMPAGESFORREF+i+1, (unsigned char*)&LogData[256*i], i);
//    }
//  }
//   //CheckCommOTDR (); // проверка и корректировка строки комментариев OTDR
//
//  if (CurCountFiles > 0x100000) // empty data
//  {
//    CurCountFiles = (unsigned long)Trace;
//  }
//  if (Trace == 0) // если читаем установочную рефлектограмму переустанавливаем режим рефлектометра
//  {
//    SetSubModRefl (SettingRefl.SubModeRef);
//    GetSetCntFiles(CurCountFiles);
//  }
}

unsigned short SetNumTrace (unsigned short Num) // установка номера трассы
{
//  //просто порядковый номер трассы, надо модифициролвать в зависимости от заполнения памяти
//  // указание на той или иной банк в зависимости от конфигурации
//  if (CheckIDMEM) // ести расширенная память
//  {
//    if (Num > (MaxMemOTDRExt + MaxMemOTDR) )
//    {
//      Num = 0;
//    }
//  }
//  else
//  {
//    if (Num > MaxMemOTDR) Num = 0;
//  }
  NumTrace = Num;
  return NumTrace;
}

unsigned short GetNumTrace (void) // получение номера трассы 
{
  return NumTrace;
}
// получаем "реальный адрес" хранения рефлектограммы
unsigned short GetNumTraceSaved (unsigned short Index) // чтение значения таблицы рефлектограм по индексу (основного блока)
{
  unsigned short RealNum = 0;
//  if (CheckIDMEM) // есть расширенная память
//  {
//    if (Index > (MaxMemOTDRExt + MaxMemOTDR) )
//    {
//      Index = 0;
//    }
//    else
//    {
//      if (Index>MemTable[0]) // число больше чем записано в 1 банке
//      {
//        RealNum = (unsigned short)(0x100 + MemTableExt[Index - MemTable[0]]); 
//      }
//      else
//      {
//        RealNum = (unsigned short)MemTable[Index];
//      }
//    }
//    if (Index == 0)      RealNum = (unsigned short)MemTable[Index]+(unsigned short)MemTableExt[Index]; // сколько ячеек заполнено
//
//  }
//  else // нет расширенной памяти
//  {
//    if (Index > MaxMemOTDR)
//    {
//      Index = 0;
//    }
//      RealNum = (unsigned short)MemTable[Index];
//  }
  return  RealNum;

}

void CheckTableMemOTDR (BYTE MemErr) // проверка таблицы сохранения рефлктограмм 0 - проверяем 1 - обновляем
{
//  //BYTE MemErr=0;
////  unsigned short AllTrace, RTrace;
//  if (!MemErr)
//  {
//    FlashReadPageNum (TABLEMEMOTDR, MemTable , ARRAY_SIZE(MemTable),0);
//  for (int i = 0; i <= MaxMemOTDR; i++)  if (MemTable[i] > MaxMemOTDR)MemErr = 1;
//    FlashReadPageNum (TABLEMEMOTDREXT, MemTableExt , ARRAY_SIZE(MemTableExt),0);
//  for (int i = 0; i <= MaxMemOTDRExt; i++)  if (MemTableExt[i] > MaxMemOTDRExt)MemErr |= 2;
//  
//  
//  }
//  if ((MemErr & 0x01) == 1)
//  {
//    for (int i = 0; i <= MaxMemOTDR; ++i) MemTable[i] = i;
//    FlashErasePage(TABLEMEMOTDR);
//    FlashWritePageSM(TABLEMEMOTDR, MemTable, ARRAY_SIZE(MemTable),0);
//  }
//  if ((MemErr & 0x02) == 2)
//  {
//    for (int i = 0; i <= MaxMemOTDRExt; ++i) MemTableExt[i] = i;
//    FlashErasePage(TABLEMEMOTDREXT);
//    FlashWritePageSM(TABLEMEMOTDREXT, MemTableExt, ARRAY_SIZE(MemTableExt),0);
//  }
////  AllTrace =  (unsigned short)MemTable[0] +  (unsigned short)MemTableExt[0]; // сколько всего рефлектограмм записано
////  // проверяем правильность сохраненных рефлектограмм по полю минут в файле сохренения если плохо удаляем эту рефлектограмму
////  for (int i = AllTrace; i > 0 ; i--) 
////  {
////    GetNumTraceSaved
////    
////  }
  
  
}

unsigned short DeletingTrace (unsigned short Trace) // "Удаление" рефлектограммы, просто переносим 
{
//  unsigned short IndexMem;
//      if (CheckIDMEM) // прочитаем какая флэшка стоит
//      {
//        // определяем в какой части лежит указанная трасса
//        if (Trace <= MemTable[0]) // main memory
//        {
//          IndexMem = MemTable[Trace];
//          for (int i=Trace; i<MaxMemOTDR; i++)
//          {
//            MemTable[i] = MemTable[i+1];
//          }
//          MemTable[MaxMemOTDR] = IndexMem;
//          MemTable[0]--;
//          Trace--;
//    FlashErasePage(TABLEMEMOTDR);
//    FlashWritePageSM(TABLEMEMOTDR, MemTable, ARRAY_SIZE(MemTable),0);
//        }
//        else // трасса во втором банке
//        {
//          IndexMem = MemTableExt[Trace - MemTable[0]];
//          for (int i=(Trace - MemTable[0]); i<MaxMemOTDRExt; i++)
//          {
//            MemTableExt[i] = MemTableExt[i+1];
//          }
//          MemTableExt[MaxMemOTDRExt] = IndexMem;
//          MemTableExt[0]--;
//          Trace--;
//    FlashErasePage(TABLEMEMOTDREXT);
//    FlashWritePageSM(TABLEMEMOTDREXT, MemTableExt, ARRAY_SIZE(MemTableExt),0);
//          
//        }
//      }
//      else  // для простой памяти по-старому
//      {
//    IndexMem = MemTable[Trace];
//          for (int i=Trace; i<MaxMemOTDR; i++)
//          {
//            MemTable[i] = MemTable[i+1];
//          }
//          MemTable[MaxMemOTDR] = IndexMem;
//          MemTable[0]--;
//          Trace--;
//    FlashErasePage(TABLEMEMOTDR);
//    FlashWritePageSM(TABLEMEMOTDR, MemTable, ARRAY_SIZE(MemTable),0);
//      }
         return Trace; 
          

}

unsigned short ReanimateTrace (unsigned short ReanT) // "реанимация" заданного числа трасс
{ 
  unsigned short Trace=0; // возвращаем число реанимированных трасс
//  if (CheckIDMEM) // прочитаем какая флэшка стоит, если есть большая, то удалем в ней верхнюю часть
//  {
//    if(ReanT>200)
//    {
//      // можно реанимировать трассы в верхнем блоке
//      MemTableExt[0] = ReanT - 200;
//    }
//    else
//      MemTableExt[0] = 0;
//    //Trace = (unsigned short)MemTable[0] + (unsigned short)MemTableExt[0] ; // сколько трасс удалим в верхнем блоке
//    Trace =  (unsigned short)MemTableExt[0] ; // сколько трасс восстановим в верхнем блоке
//    // сохраняем таблицу сохранений
//    FlashErasePage(TABLEMEMOTDREXT);
//    FlashWritePageSM(TABLEMEMOTDREXT, MemTableExt, ARRAY_SIZE(MemTableExt),0);
//  }
//  // этот блок нижней части можем востановить только не более 200 
//  //      else
//  //      {
//  if(ReanT>200)
//    MemTable[0] = 200;
//  else
//    MemTable[0] = ReanT;
//  Trace = Trace + (unsigned short)MemTable[0];
//  SetNumTrace (0); // установка номера трассы
//  FlashErasePage(TABLEMEMOTDR);
//  FlashWritePageSM(TABLEMEMOTDR, MemTable, ARRAY_SIZE(MemTable),0);
//  //      }
  return Trace; 
}

unsigned short DeletingAllTrace (void) // "Удаление"всех рефлектограммы, просто обнуляем счетчик сохранения
{ 
  unsigned short Trace=0; // возвращаем число удаленных трасс
//      if (CheckIDMEM) // прочитаем какая флэшка стоит, если есть большая, то удалем в ней верхнюю часть
//      {
//  //Trace = (unsigned short)MemTable[0] + (unsigned short)MemTableExt[0] ; // сколько трасс удалим в верхнем блоке
//  Trace =  (unsigned short)MemTableExt[0] ; // сколько трасс удалим в верхнем блоке
//  
//  //SetNumTrace (0); // установка номера трассы
//  MemTable[0] = 0;
//  MemTableExt[0] = 0;
//  // переиндексируем блок
//    for (int i = 0; i <= MaxMemOTDRExt; ++i) MemTableExt[i] = i;
////  
////  FlashErasePage(TABLEMEMOTDR);
////  FlashWritePageSM(TABLEMEMOTDR, MemTable, ARRAY_SIZE(MemTable),0);
//        // сохраняем таблицу сохранений
//        FlashErasePage(TABLEMEMOTDREXT);
//        FlashWritePageSM(TABLEMEMOTDREXT, MemTableExt, ARRAY_SIZE(MemTableExt),0);
//     }
//     // этот блок переиндексируем всегда!!!
////      else
////      {
//  Trace = Trace + (unsigned short)MemTable[0];
//  SetNumTrace (0); // установка номера трассы
//  MemTable[0] = 0;
//  // переиндексируем блок
//    for (int i = 0; i <= MaxMemOTDR; ++i) MemTable[i] = i;
//  FlashErasePage(TABLEMEMOTDR);
//  FlashWritePageSM(TABLEMEMOTDR, MemTable, ARRAY_SIZE(MemTable),0);
////      }
  return Trace; 
}

// КАРТИНКА ПАМЯТИ - рисуем картинку памяти рефлектограмм
void DrawMemoryRefl(unsigned short Trace, BYTE CurrLang, BYTE Param)
{
//  char Str[32];
//  char StrI[32];
//  char s_orl[6];
//  //BYTE CurrLang=GetLang(CURRENT);
//  if(g_FirstScr) // если первый раз пришли читаем трассу
//  {
//    if (Trace)
//      TraceREAD(GetNumTraceSaved(Trace)); // читаем данные из памяти по номеру котораяуказана в списке
//    else TraceREAD(0); // если 0 то ее и читаем
//  }
//      CheckCommOTDR (); // проверка и корректировка строки комментариев OTDR перед записью...
//
//  // проверка считанных данных
//  if(CheckSavedTrace()) // функция контроля сохраненной рефлектограммы 
//  {
//    // рефлектограмма текущая плохая (надо удалить)
//    Trace = DeletingTrace (Trace);
//    SetNumTrace(Trace);
//    TraceREAD(0);
//  }
//  //объявления графических установок для старого индикатора, они же используются для нового, НО Ниже
//  GraphParams params = {27000,0,48,0,MEMDRAW};//PosCursorMain (0) // масштаб 48 ( для уменьшенной картинки)
//  Rect rct={43,18,128,63};
//  //для нового индикатора(рисуем график)
//  if(g_FirstScr)
//  {
//    if(TypeLCD)
//    {
//    params.scale = 13;// for 3.5
//    rct.right=313;//
//    rct.bottom=216;//
//    }
//    else
//    {
//    params.scale = 16;//for 3.2
//    rct.right=256;//
//    rct.bottom=128;//
//    }
//   rct.left=0;
//    rct.top=0;
//    MakeGraphNext( &rct, LogData, 4096, &params );
//    SendDrawNex(NexData,6,rct.right); // ID=6 для графика в просмотре
//    // заполним нужные поля, для нового индикатора в функции прорисовки, не снимая признак первого входа
//  sprintf(Str,"%02d%02d%02d_%02d%02d%01d.sor(%03d)",TimeSaveOTDR.RTC_Year%100,
//          TimeSaveOTDR.RTC_Mon,
//          TimeSaveOTDR.RTC_Mday,
//          TimeSaveOTDR.RTC_Hour,
//          TimeSaveOTDR.RTC_Min,
//          TimeSaveOTDR.RTC_Sec/10,
//          Trace);
//    sprintf(StrI, "t0.txt=\"%s\"яяя", Str);
//    UARTSend2((BYTE*)StrI, strlen(StrI));    // Date/Time/Num записи
//    sprintf(Str, "t5.txt=\"%s\"яяя",CommentsOTDR); 
//    UARTSend2((BYTE*)Str, strlen(Str));    // 1 строка комментарии
//  if (Param)
//  {
//    // ищем события
//    CalkEventsKeys (LogData, PointsInImpulse(0), 1);
//    if (GetIndexLN()) // не 2 км ( значит в км)
//      sprintf(Str,"t1.txt=\"%.1f%s\"яяя",fabs(GetPosLine(EndEvenBlk.ELMP[1])) ,MsgMass[20][CurrLang]);//км
//    else
//      sprintf(Str,"t1.txt=\"%.1f%s\"яяя",fabs((GetPosLine(EndEvenBlk.ELMP[1]))*1000.0) ,MsgMass[78][CurrLang]);//м - метры
//    UARTSend2((BYTE*)Str, strlen(Str));    // 1 строка комментарии
//    sprintf(Str,"t2.txt=\"%.2f%s\"яяя",(LogData[EndEvenBlk.ELMP[1]]-LogData[EndEvenBlk.ELMP[0]])/1000.0,MsgMass[47][CurrLang]);//дБ
//    UARTSend2((BYTE*)Str, strlen(Str));    // 1 строка комментарии
//    if(g_VolORL!=0)sprintf(s_orl,"%.1f",g_VolORL);
//    else sprintf(s_orl,"???");
//    sprintf(Str,"t3.txt=\"%s%s\"яяя",s_orl,MsgMass[47][CurrLang]);//ORL, если измерялось!
//    UARTSend2((BYTE*)Str, strlen(Str));    // 1 строка комментарии
//    sprintf(Str,"t4.txt=\"\"яяя");// Empty
//    UARTSend2((BYTE*)Str, strlen(Str));    
//    sprintf(Str,"t6.txt=\"%d\"яяя",mg_SetModeLW);// Empty
//    UARTSend2((BYTE*)Str, strlen(Str));    
//  }
//  else
//  {
//    sprintf(Str,"t1.txt=\"%d%s\"яяя", GetLengthLine(GetIndexLN()),MsgMass[20][CurrLang]);
//    UARTSend2((BYTE*)Str, strlen(Str));    
//    sprintf(Str,"t2.txt=\"%d%s\"яяя", GetWidthPulse(GetIndexIM()),MsgMass[23][CurrLang]);//ns
//    UARTSend2((BYTE*)Str, strlen(Str));    
//    sprintf(Str,"t3.txt=\"%d%s\"яяя",GetLengthWaveLS (GetPlaceLS(CURRENT)),MsgMass[18][CurrLang]); // nm
//    UARTSend2((BYTE*)Str, strlen(Str));    
//    sprintf(Str,"t4.txt=\"%d%s\"яяя", GetTimeAvrg(GetIndexVRM()),MsgMass[4][CurrLang]);//c
//    UARTSend2((BYTE*)Str, strlen(Str));    
//    //sprintf(Str,"t6.txt=\"%c/%c\"яяя",(SettingRefl.SubModeRef)?('A'):('M'),(GetSubModRefl())?('A'):('M'));// Empty
//    sprintf(Str,"t6.txt=\"%.4f\"яяя",GetWAV_SC(GetPlaceLS(CURRENT)));// коэфф преломления для данной длины волны
//    UARTSend2((BYTE*)Str, strlen(Str));    
//  }
//  }
//  // 
//  //sprintf(Str,"%d",  PointsInImpulse ());//ns
//  //putString(102,19,Str,1,0);//нс
  
}


unsigned short SaveTrace(void) // сохранение рефлектограммы
        // 
      // пока сохраняем без разговоров
      
{ 
  //char Str[22];
  //ClearScreen(screen);
  
//  if (MemTable[0]<MaxMemOTDR)
//  {
//    MemTable[0]++;
//    CurCountFiles = GetSetCntFiles (1); // изменяем счетчик рефлектограмм
//    // сохраняем таблицу сохранений
//    FlashErasePage(TABLEMEMOTDR);
//    FlashWritePageSM(TABLEMEMOTDR, MemTable, ARRAY_SIZE(MemTable),0);
//    
//    // для востановления настроек при включении прибора 
//    // только здесь пишем в память - поэтому здесь необходимы проверки какая память и куда писать
//    TraceERASE(MemTable[MemTable[0]]);
//    TraceWRITE(MemTable[MemTable[0]]); // записываем рефлектограмму с параметрами сьема  в N ячейку
//    //Total_Time = Total_Sec(ds1337); // считаем секунды
//    // пишем информацию о настройке в заголовок BellCore файла
//    //Set_Bel_core(Total_Time,(unsigned short int)(Im_set[Index_Im]) , (unsigned int)(0x07F281*DS_Code), (unsigned int)(K_pr*100000), sycl, (unsigned int)(0x01A0AA*DS_Code) );
//    //SPI1_read_page0(HEAD); // заголовок файла BELCORE в память
//    
//    NumTrace = MemTable[0];   // указатель памяти рефлектограмм на вновь сохраненную
//  }
//  else// память заполнена (проверим можем ли писать есть ли расширенная память и ее состояние)
//  {
//    if (CheckIDMEM) // прочитаем какая флэшка стоит
//      // есть расширенная память - пишем в расширенную область
//    {
//      if (MemTableExt[0]<MaxMemOTDRExt)// есть место в расширенной области
//      {
//        MemTableExt[0]++;
//        CurCountFiles = GetSetCntFiles (1); // изменяем счетчик рефлектограмм
//        // сохраняем таблицу сохранений
//        FlashErasePage(TABLEMEMOTDREXT);
//        FlashWritePageSM(TABLEMEMOTDREXT, MemTableExt, ARRAY_SIZE(MemTableExt),0);
//        
//        // для востановления настроек при включении прибора 
//        // только здесь пишем в память - поэтому здесь необходимы проверки какая память и куда писать
//        TraceERASE(0x100 + MemTableExt[MemTableExt[0]]);
//        TraceWRITE(0x100 + MemTableExt[MemTableExt[0]]); // записываем рефлектограмму с параметрами сьема  в N ячейку
//        NumTrace = MemTable[0] + MemTableExt[0];   // устанвливаем номер на сохраненную  памяти рефлектограмм на вновь сохраненную
//        
//      }
//      else
//      {
//        NumTrace = 0;  // указатель памяти рефлектограмм на нулевую// нет места в расширенной памяти
//      }
//    }
//    else // вся память заполнена 
//    {
//      NumTrace = 0;
//    }
//  }
//  TraceERASE(0);
//  TraceWRITE(0); // записываем рефлектограмму с параметрами сьема  в 0 ячейку
  // там же сохраняем номер текущего файла
  return NumTrace;
  //PaintLCD();
}

void SaveFileSD(int Mod)
{
  //        //        unsigned long NowEND;
  //        //        unsigned long NowBEG;
  unsigned short NumEventNow = GetNumEvents();
  //        unsigned short NumTr = (unsigned short)(atoi((char*)&RX_Buf[17]));
  //        if (NumTr > GetNumTraceSaved(0)) NumTr = 0; // заданная рефлектограмма не существует
  //        SetNumTrace (NumTr); // установка номера трассы
  //SetModeDevice (MODEMEMR); // принудительная установка режима прибора 
  //        // надо прочитать указанную рефлектограмму
  //        // первое заполнение надо прочитать файл
  //        if (GetNumTrace()) // если не нулевая то читаем по таблице
  //          TraceREAD(GetNumTraceSaved(GetNumTrace()));//  читаем файл который надо передать// 27/01/2011 неадекватно считывала рефлектограмму
  //        else  TraceREAD(0);
  //        // ищем события в линии (25.05.2011 пока конец линии)
  InitEventsTable (); // инициализация структур событий
  //        // признак разрешения событий при передаче
  if (GetSetEnaEvents (0)) // проверяем признак разрешения событий
  {
    // ищем события и заполняем файл
    NumEventNow =  (CalkEventsKeys (LogData, PointsInImpulse(0), 1)); 
    //          // расчет погонного затухания если есть точка
    //          /**/
    if (NumEventNow)          //имеем события - 
    {
      if (EndEvenBlk.ELMP[0]!=EndEvenBlk.ELMP[1]) // есть линия! занесем параметры
      {
        TmpACI = GetPosLine(EvenTrace[0].EPT);
        //TmpACI = ;
        TmpACI = (LogData[EvenTrace[0].EPT]-LogData[0])/TmpACI;//GetPosLine(EvenTrace[0].EPT);
        EvenTrace[0].ACI = (short int)TmpACI;
        EndEvenBlk.ELMP[1] = CalkEPT (EndEvenBlk.ELMP[1]); // расчет значений ELMP для конца линии от положения курсора
        
      }
      // цикл заполнения событий 
      if (NumEventNow>1)
      {
        for (int i=1;i<NumEventNow;++i)
        {
          TmpACI = GetPosLine(EvenTrace[i].EPT-EvenTrace[i-1].EPT);
          TmpACI = (LogData[EvenTrace[i].EPT]-(LogData[EvenTrace[i-1].EPT]+EvenTrace[i-1].EL))/TmpACI;
          EvenTrace[i].ACI = (short int)TmpACI;
          //EvenTrace[i].ACI = (LogData[EvenTrace[i].EPT]-(LogData[EvenTrace[i-1].EPT]+EvenTrace[i-1].EL))/GetPosLine(EvenTrace[i].EPT-EvenTrace[i-1].EPT);
          
          //EvenTrace[i-1].EPT = CalkEPT (EvenTrace[i-1].EPT); // расчет значений EPT для событий от положения курсора
        }
      }
      // Заполнение события и конец линии
      
      for (int i=0;i<NumEventNow;++i)
      {
        EvenTrace[i].EPT = CalkEPT (EvenTrace[i].EPT); // расчет значений EPT для событий от положения курсора
      }
    }
    
  }
  // тест загрузка формирование событий
  //NumEventNow = 9;
  //TestLoadEvents (NumEventNow);
  //< < < < <  !!! В Н И М А Н И Е !!! > > > > >
  // попробуем записать файл
  // откроем SD Card, сохраняем всегда в файл 0.sor 
  // для простоты пока
  do
  {
    //------------------[ Mount The SD Card ]--------------------
    FR_Status = f_mount(&FatFs, SDPath, 1);
    if (FR_Status != FR_OK)
    {
      break;
    }
    // создаем или проверяем наличие дирректории _OTDR
    res = f_mkdir(PathMainDir);//"0:/_OTDR"
    if(res == FR_EXIST)
    {
      //sprintf ((char*)TxBuffer,"Make MainDir Already Is\r");
      res = FR_OK;
    }
    // откроем файл для записи
    if(Mod) // сохраняем в дирректроию недели и с именем файла по времени
    {
      // подготовим путь
      // папка для сохранения делаем папку  
      GetFolder(FileSDir);

    sprintf(PathF,"%s/%s",PathMainDir,FileSDir);
    res = f_mkdir(PathF);
    //res = f_unlink(PathF);
    if(res == FR_EXIST)
    {
      //sprintf ((char*)TxBuffer,"Make MainDir Already Is\r");
      res = FR_OK;
    }
        HAL_Delay(2);
    // почитаем директории...только что созданные 
    res = f_opendir(&dir, PathF);
        HAL_Delay(2);
    f_closedir(&dir);
    // папка создана
      // имя файла
      sprintf(FileNameS,"%02d%02d%02d_%02d%02d%01d.sor",TimeSaveOTDR.RTC_Year%100,
          TimeSaveOTDR.RTC_Mon,
          TimeSaveOTDR.RTC_Mday,
          TimeSaveOTDR.RTC_Hour,
          TimeSaveOTDR.RTC_Min,
          TimeSaveOTDR.RTC_Sec/10 );
       // имя файла есть
      //создадим полны путь к файлу чтобы его открыть
    sprintf(PathFileS,"%s/%s",PathF,FileNameS);
      
    }
    
    //Open the file
    //FR_Status = f_open(&Fil, "MyTextFile.txt", FA_WRITE | FA_READ | FA_CREATE_ALWAYS);
    else
    sprintf(PathFileS,"0.sor");
    FR_Status = f_open(&Fil, PathFileS, FA_WRITE  | FA_CREATE_ALWAYS);
    //    if(FR_Status != FR_OK)
    //    {
    //      sprintf(TxBuffer, "Error! While Creating/Opening A New Text File, Error Code: (%i)\r\n", FR_Status);
    //      UARTSendExt ((BYTE*)TxBuffer, strlen (TxBuffer));
    //      break;
    //    }
    
    //unsigned short NumEventNow = 0; // пока без событий
    // начинаем передачу трассы (Заголовок)
    uint32_t HowSizeFile = 8419 + ((NumEventNow)?(NumEventNow*32+40):(0));
    
    //sprintf (StartStr, "#4%4d",8419 + ((NumEventNow)?(NumEventNow*32+40):(0)));
    //UARTSendExt ((BYTE*)StartStr, 6);
    // Мар страница белкора с учетом Таблицы событий (блок 0)
    // если есть таблица событий....
    GetHeaderBelcore (BufString, 0, NumEventNow); // заполняем шапку белкора первые 56 байт Block=0
    //UARTSendExt ((BYTE*)BufString, 56+16*((NumEventNow)?(1):(0)));
    FR_Status = f_write(&Fil, (BYTE*)BufString, 56+16*((NumEventNow)?(1):(0)),&WWC);
    
    // подготовка для расчета контрольной суммы
    unsigned short old_crc = 0xffff; 
    unsigned short new_crc = 0xffff;
    c = (unsigned char*)&BufString;
    for (int i=0;i<56+16*((NumEventNow)?(1):(0));i++)
    {
      /* первый вариант подсчета контрольной суммы - табличный                                             */		
      value = *c;
      new_crc = (old_crc << 8) ^ table[((old_crc >> 8) ^ ((unsigned short int)value)) & 0xff];
      old_crc = new_crc;
      c++;
    }
    // записали 56 или +16 байт в файл = 56 (72)
    // заполняем шапку белкора  62 байт Block=1 (продолжение Мар блока + GenParams)
    GetHeaderBelcore (BufString, 1, NumEventNow); 
    //UARTSendExt ((BYTE*)BufString, 62);
    FR_Status = f_write(&Fil, (BYTE*)BufString, 62,&WWC);
    c = (unsigned char*)&BufString;
    for (int i=0;i<62;i++)
    {
      /* Считаем контрольную сумму переданного блока                                             */		
      value = *c;
      new_crc = (old_crc << 8) ^ table[((old_crc >> 8) ^ ((unsigned short int)value)) & 0xff];
      old_crc = new_crc;
      c++;
    }
    // записали 56+62 или +16 байт в файл = 118 (134)
    // заполняем шапку белкора  94 байт Block=2 - (SupParams FxdParam)
    GetHeaderBelcore (BufString, 2, NumEventNow); 
    //UARTSendExt ((BYTE*)BufString, 95);
    FR_Status = f_write(&Fil, (BYTE*)BufString, 95,&WWC);
    c = (unsigned char*)&BufString;
    for (int i=0;i<95;i++)
    {
      /* Считаем контрольную сумму переданного блока                                             */		
      value = *c;
      new_crc = (old_crc << 8) ^ table[((old_crc >> 8) ^ ((unsigned short int)value)) & 0xff];
      old_crc = new_crc;
      c++;
    }
    // записали 118+95 или +16 байт в файл = 213 (229)

    // Проверяем и передаем блок событий если он есть (блок событий)
    if (NumEventNow) // если есть события 2 байта +
      // события в фиксированном размере для каждого 32 байта  +  22 байт общее для всего блока, итого 24+N*32
    {
      // передаем  число событий  2 байта
      //UARTSendExt ((BYTE*)&NumEventNow, 2);
      FR_Status = f_write(&Fil, (BYTE*)&NumEventNow, 2,&WWC);
      c = (unsigned char*)&NumEventNow;
      for (int i=0;i<2;i++)
      {
        /* Считаем контрольную сумму переданного блока                                             */		
        value = *c;
        new_crc = (old_crc << 8) ^ table[((old_crc >> 8) ^ ((unsigned short int)value)) & 0xff];
        old_crc = new_crc;
        c++;
      }
      // передаем информационные блоки событий  N*32
      for (int s=0; s<NumEventNow; s++)
      {
        //UARTSendExt ((BYTE*)&EvenTrace[s], 32);
        FR_Status = f_write(&Fil, (BYTE*)&EvenTrace[s], 32,&WWC);
        c = (unsigned char*)&EvenTrace[s];
        for (int i=0;i<32;i++)
        {
          /* Считаем контрольную сумму переданного блока                                             */		
          value = *c;
          new_crc = (old_crc << 8) ^ table[((old_crc >> 8) ^ ((unsigned short int)value)) & 0xff];
          old_crc = new_crc;
          c++;
        }
        
      }
      // передаем конечный блок событий 22 байта
      //UARTSendExt ((BYTE*)&EndEvenBlk, 22);
      FR_Status = f_write(&Fil, (BYTE*)&EndEvenBlk, 22,&WWC);
      c = (unsigned char*)&EndEvenBlk;
      for (int i=0;i<22;i++)
      {
        /* Считаем контрольную сумму переданного блока                                             */		
        value = *c;
        new_crc = (old_crc << 8) ^ table[((old_crc >> 8) ^ ((unsigned short int)value)) & 0xff];
        old_crc = new_crc;
        c++;
      }
    }
    
    // заполняем шапку белкора 12 байт Block=3 (DataPts)
    GetHeaderBelcore (BufString, 3, NumEventNow); 
    //UARTSendExt ((BYTE*)BufString, 12);
    FR_Status = f_write(&Fil, (BYTE*)BufString, 12,&WWC);
    c = (unsigned char*)&BufString;
    for (int i=0;i<12;i++)
    {
      /* Считаем контрольную сумму переданного блока                                             */		
      value = *c;
      new_crc = (old_crc << 8) ^ table[((old_crc >> 8) ^ ((unsigned short int)value)) & 0xff];
      old_crc = new_crc;
      c++;
    }
    
    // блок данных 
    //UARTSendExt ((BYTE*)LogData, OUTSIZE*2);
    FR_Status = f_write(&Fil, (BYTE*)LogData, OUTSIZE*2,&WWC);
    c = (unsigned char*)&LogData;
    for (int i=0;i<OUTSIZE*2;i++)
    {
      /* первый вариант подсчета контрольной суммы - табличный                                             */		
      value = *c;
      new_crc = (old_crc << 8) ^ table[((old_crc >> 8) ^ ((unsigned short int)value)) & 0xff];
      old_crc = new_crc;
      c++;
    }
    
    //UARTSendExt ((BYTE*)&new_crc, 2);
    FR_Status = f_write(&Fil, (BYTE*)&new_crc, 2,&WWC);
        HAL_Delay(2);

    f_close(&Fil);
    
  } while(0);
      HAL_Delay(2);

  //------------------[ Test Complete! Unmount The SD Card ]--------------------
  FR_Status = f_mount(NULL, "", 0);
  
}

void CopyFileSave (void) // копирование файла "0" в файл который сохраняем
{
  // откроем карту
    do
  {
    //------------------[ Mount The SD Card ]--------------------
    FR_Status = f_mount(&FatFs, SDPath, 1);
    if (FR_Status != FR_OK)
    {
      break;
    }

  // определим время, создадим имя файла
  // зададим пути
  // откроем файлы
  // скопируем
  // закроем файлы
  // закроем карту
      } while(0);
      HAL_Delay(2);

    FR_Status = f_mount(NULL, "", 0);

}
