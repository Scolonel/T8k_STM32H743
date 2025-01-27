/**
  ******************************************************************************
  * @file    pca9554.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    28 december-2014
  * @brief   This file includes the driver for control PCA9554 .
  ******************************************************************************
  * @attention
  *
   *
  ******************************************************************************
Установленная память 512 = 64к байт
  */ 

/* Includes ------------------------------------------------------------------*/
#include "pca955x.h"
#include "main.h"
/** @addtogroup Component
  * @{
  */ 
  
HAL_StatusTypeDef StatusI2C2 = HAL_ERROR;

uint8_t MemBlock[256]; // блок для перезаписи ячеек памяти

/**
  * @}
  */ 
// "ОБЕРТКИ" для функций работы по I2C что бы не мешали друг другу
HAL_StatusTypeDef TOP_I2C_IsDeviceReady(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Trials, uint32_t Timeout)
{
  HAL_StatusTypeDef status = HAL_I2C_IsDeviceReady(hi2c, DevAddress, Trials, Timeout);
  return status;
}

HAL_StatusTypeDef TOP_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
  HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(hi2c, DevAddress, pData, Size, Timeout);
  return status;
}

HAL_StatusTypeDef TOP_I2C_Master_Receive(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
  HAL_StatusTypeDef status = HAL_I2C_Master_Receive(hi2c, DevAddress, pData, Size, Timeout);
  return status;
}


uint32_t GetExpand (void) // получение состояния пинов Expanders (9554), одним словом 23-16:9554,15-0:9555
{
  uint32_t Result = 0;
//  HAL_StatusTypeDef StatusI2C2 = HAL_ERROR;
  uint8_t BufTX[4];
  uint8_t BufRX[4];
  //GPIOE->BSRRH = GPIO_PIN_3; // (PE3) reset pin

  StatusI2C2 =  TOP_I2C_IsDeviceReady(&hi2c2, (uint16_t)(KEYBOARD<<1) , 1, 10000); // если эта микросхема???
  if (StatusI2C2) return StatusI2C2<<26;
    // read current stats
  BufTX[0] = 0;
  StatusI2C2 = TOP_I2C_Master_Transmit(&hi2c2, (uint16_t)(KEYBOARD<<1), BufTX, 1, 10000);
  StatusI2C2 = TOP_I2C_Master_Receive(&hi2c2, (uint16_t)(KEYBOARD<<1), BufRX, 2, 10000);
// пишем байт в слово
  Result = (uint32_t)(BufRX[0]); // старший
  Result = Result<<8;
  Result += (uint32_t)(BufRX[1]);// младший
  // так как пользуем только для клавиатуры 7 бит (ограничим сразу здесь)
  
  return (Result & 0x3F);

}

//конфигурация PCA9554 ( T7KAR_743) клавиатура адрес 0x38

HAL_StatusTypeDef ConfigKBRD (void)
{
  //uint16_t DevAddr = EXPWRCTRL<<1; // address I2C
  uint8_t BufTX[10];
  //uint8_t BufRX[10];
//    StatusI2C2 =  TOP_I2C_IsDeviceReady(&hi2c2, (uint16_t)(EXPWRCTRL<<1), 2, 1000); // если эта микросхема???
    StatusI2C2 =  TOP_I2C_IsDeviceReady(&hi2c2, (uint16_t)(KEYBOARD<<1), 2, 1000); // если эта микросхема???
  if (StatusI2C2) return StatusI2C2;
  // пишем в порты 1 -
  BufTX[0] = 1;
  BufTX[1] = 0x3F;
  StatusI2C2 = TOP_I2C_Master_Transmit(&hi2c2, (uint16_t)(KEYBOARD<<1), BufTX, 2, 1000);
  if (StatusI2C2) return StatusI2C2;
  // без инверсии -
  BufTX[0] = 2;
  BufTX[1] = 0x0;
  StatusI2C2 = TOP_I2C_Master_Transmit(&hi2c2, (uint16_t)(KEYBOARD<<1), BufTX, 2, 1000);
  if (StatusI2C2) return StatusI2C2;
  // конфигурируем микросхему как порты вывода Р0-Р7 - 
  BufTX[0] = 3;
  BufTX[1] = 0xFF;
  StatusI2C2 = TOP_I2C_Master_Transmit(&hi2c2, (uint16_t)(KEYBOARD<<1), BufTX, 2, 1000);
  //if (StatusI2C2) return StatusI2C2;
  //BufTX[0] = 0;
  //StatusI2C2 = TOP_I2C_Master_Transmit(&hi2c2, (uint16_t)(KEYBOARD<<1), BufTX, 1, 10000);
 // StatusI2C2 = TOP_I2C_Master_Receive(&hi2c2, (uint16_t)(EXPWRCTRL<<1), BufRX, 1, 10000);
  //if (BufRX[0] != 0xff) return BufRX[0];
  return StatusI2C2;
  }
//    
//конфигурация PCA9555 ( только - расширение) адресс 0x20
// устанавливаем как  выходы P0-P15! несмотря на то что после сброса уже должно быть как входд
HAL_StatusTypeDef ConfigPWRCtrl (void)
{
    //uint16_t DevAddr = EXPWRCTRL<<1; // address I2C
  uint8_t BufTX[10];
  //uint8_t BufRX[10];
    StatusI2C2 =  TOP_I2C_IsDeviceReady(&hi2c2, (uint16_t)(EXPWRCTRL<<1), 1, 1000); // если эта микросхема???
  if (StatusI2C2) return StatusI2C2;
  // пишем в OUT порты 0 - 
  BufTX[0] = 2;
  BufTX[1] = 0x00;
  BufTX[2] = 0x00;
  StatusI2C2 = TOP_I2C_Master_Transmit(&hi2c2, (uint16_t)(EXPWRCTRL<<1), BufTX, 3, 1000);
  if (StatusI2C2) return StatusI2C2;
  // без инверсии - 
  BufTX[0] = 4;
  BufTX[1] = 0x0;
  BufTX[2] = 0x0;
  StatusI2C2 = TOP_I2C_Master_Transmit(&hi2c2, (uint16_t)(EXPWRCTRL<<1), BufTX, 3, 1000);
  if (StatusI2C2) return StatusI2C2;
  // конфигурируем микросхему как порты вывода Р0-P15 - выход
  BufTX[0] = 6;
  BufTX[1] = 0x00; 
  BufTX[2] = 0x00;
  StatusI2C2 = TOP_I2C_Master_Transmit(&hi2c2, (uint16_t)(EXPWRCTRL<<1), BufTX, 3, 1000);
  if (StatusI2C2) return StatusI2C2;
//  BufTX[0] = 6;
//  BufTX[1] = 0x0;
//  StatusI2C2 = TOP_I2C_Master_Transmit(&hi2c2, (uint16_t)(COMMCTRL<<1), BufTX, 2, 10000);
   return StatusI2C2;

}


// так как у нас один рассширитель и всё работает на выход
HAL_StatusTypeDef CtrlExpand ( uint16_t PinOut, uint16_t Mask) // управление пинами выходов Expanders ( 9555),  P0-P15 
// uint16_t Mask - какие биты меняем,  PinOut - значение которое ставим,
//так как устанавливаем по маске можно нулить биты без признака
{
  //HAL_StatusTypeDef StatusI2C2 = HAL_ERROR;
//  StatusI2C2 = HAL_ERROR;
  uint8_t BufTX[3];
  uint8_t BufRX[3];
  volatile uint16_t InPin1 = PinOut & Mask; // устанавливаем  для младшего байта управления
  //volatile uint16_t InPin2 = PinOut & Mask; // здесь прописано что устанавливаем  
  
    StatusI2C2 =  TOP_I2C_IsDeviceReady(&hi2c2, (uint16_t)(EXPWRCTRL<<1), 1, 1000); // если эта микросхема???
  if (StatusI2C2) return StatusI2C2;
    // сначала младший байт переустанавливаем в соответствии с PinOUT
    // read current stats LOEW byte
  BufTX[0] = 0;   // пишем команду  0 - значит принимаем младший первый 
  StatusI2C2 = TOP_I2C_Master_Transmit(&hi2c2, (uint16_t)(EXPWRCTRL<<1), BufTX, 1, 1000);
  // читаем 2 байта ( вроде как ) младший первый
  StatusI2C2 = TOP_I2C_Master_Receive(&hi2c2, (uint16_t)(EXPWRCTRL<<1), BufRX, 2, 1000); // считываем два байта значений установленных портов
  // модернизируем регистры согласно установок
  BufRX[0] &= ~(uint8_t)(Mask & 0xFF); // 
  BufRX[0] |= (uint8_t)(InPin1 & 0xFF); // 
  BufRX[1] &= ~(uint8_t)(Mask >> 8);
  BufRX[1] |= (uint8_t)(InPin1 >> 8);;
  // теперь надо записать
  BufTX[0] = 2 ; // первый младший
  BufTX[1] = BufRX[0]; 
  BufTX[2] = BufRX[1]; 
  // write NEW stat in IC LOW Byte
   StatusI2C2 = TOP_I2C_Master_Transmit(&hi2c2, (uint16_t)(EXPWRCTRL<<1), BufTX, 3, 1000);
  if (StatusI2C2) return StatusI2C2;
  
  return StatusI2C2 ;
  }


// разбор и запись соответствующей структуры
void WriteNeedStruct (int NumStruct)
  {
    if(NumStruct)
    {
      if (NumStruct & 0x01)   EEPROM_write(&ConfigDevice, ADR_ConfigDevice, sizeof(ConfigDevice));; // сохраненеие конфигурации  прибора бит 0(0x01)
      NumStruct = NumStruct>>1;
      if (NumStruct & 0x01) EEPROM_write(&UserSet, ADR_UserMeasConfig, sizeof(UserSet)); // сохрананение структуры установок пользователя бит 2 (0x02)
      NumStruct = NumStruct>>1;
      if (NumStruct & 0x01) EEPROM_write(&CoeffLW, ADR_CoeffPM, sizeof(CoeffLW)); // сохранение коэффициентов настройки измерителя P1 бит 1  (0x04)
      NumStruct = NumStruct>>1;
 
    }
  }



//

uint32_t BeginConfig (void) // начальная конфигурация
{
  uint32_t CodeError = 0; //
    uint32_t ErrDev=0;
    uint32_t SPCTR_err=0;
  // проверка наличия микросхемы управления ключами 95555 на шине I2C(2)
  //конфигурация PCA9555 ( клавитатура ) адрес 0x20
  // устанавливаем как входы все! несмотря на то что после сброса уже должно быть как входд
  StatusI2C2 =  TOP_I2C_IsDeviceReady(&hi2c2, (uint16_t)(KEYBOARD<<1), 2, 1000); // если эта микросхема???
  if (StatusI2C2) CodeError |= NOT_PCA9554_KB; // микросхема не определяется (0x01)
  StatusI2C2 =  ConfigKBRD (); // конфигурируем PCA9554  
  if (StatusI2C2) CodeError |= CFG_ERR_PCA9554_KB; // микросхема не конфигурируется (0x02)
  //
  //конфигурация PCA9554 ( ExtKey ) адрес 0x39
  // 
  StatusI2C2 =  TOP_I2C_IsDeviceReady(&hi2c2, (uint16_t)(EXPWRCTRL<<1), 2, 1000); // если эта микросхема???
  if (StatusI2C2) CodeError |= NOT_PCA9555; // микросхема не определяется (0x04)
  StatusI2C2 =  ConfigPWRCtrl (); // конфигурируем PCA9555  
  if (StatusI2C2) CodeError |= CFG_ERR_PCA9555; // микросхема не конфигурируется (0x08)
  //
  
//  // проверка наличия микросхемы  24lc512 на шине I2C(2) EEPROM
  StatusI2C2 =  TOP_I2C_IsDeviceReady(&hi2c2, (uint16_t)(EEPROM24LC128<<1), 2, 1000); // если эта микросхема???
  if (StatusI2C2) CodeError |= NOT_EEPROM_24LC128; // микросхема не определяется (0x10)
  else // микросхема есть прочитаем конфигурацию таблиц и поправим если надо Взята конфигурация от 9400
  {
    // читаем сохраненные массивы
    // функция контроля конфигурации прибора (проверка)
    EEPROM_read(&ConfigDevice, ADR_ConfigDevice, sizeof(ConfigDevice));
    ErrDev = InvalidDevice();
    if (ErrDev)
    {
      CodeError |= ERR_DEVCFG; // плохая конфигурация прибора (0x80)
      SPCTR_err=0x80; // если плохой прибор, очистим спектральную характеристику
      InitDevice(ErrDev); 
      EEPROM_write(&ConfigDevice, ADR_ConfigDevice, sizeof(ConfigDevice));
    }
    
    // контроль установок пользователя измерений прибора
    EEPROM_read(&UserSet, ADR_UserMeasConfig, sizeof(UserSet));
    SPCTR_err = SPCTR_err + CheckUserGonfig ();  // Проверка пользовательских настроек и тут же исправление
    if (SPCTR_err>=0x100)// были ошибки их уже исправили надо просто перезаписать 
    {
      CodeError |= ERR_USERSET; // плохие пользоваельские настройки (0x200)
      //FixErrUserMC (ErrDev); //  фиксируем  (исправляем настройки)
      EEPROM_write(&UserSet, ADR_UserMeasConfig, sizeof(UserSet));
    }
    // контроль таблицы уровней
    //EEPROM_read(&CoeffLW, ADR_CoeffPM, sizeof(CoeffLW)); //проверка таблицы коэффициентов и исправление таблицы коэффициентов
    EEPROM_read(&CoeffLW, ADR_CoeffPM, 12800); //проверка таблицы коэффициентов и исправление таблицы коэффициентов
    ErrDev = FindErrCoeff ();
    SPCTR_err = SPCTR_err + ErrDev;
    if (ErrDev)
    {
      CodeError |= ERR_SPECTR; // плохая спектралка правим! (0x100)
      //EEPROM_write(&CoeffLW, ADR_CoeffPM, sizeof(CoeffLW));
      EEPROM_write(&CoeffLW, ADR_CoeffPM,12800);
    }
    
  }
  //return (CodeError | (ErrDev<<3)); 
  return (CodeError); 
}



//функция записи EEPROM
HAL_StatusTypeDef EEPROM_write(void *buff, uint16_t address, uint32_t byte)
{ 
#define size_pg 128
  uint8_t BufTX[140];
  uint8_t *ptr = buff;
  uint16_t n_pg = byte/size_pg ; // число страниц
  uint16_t last_byte=byte - (n_pg*size_pg); // остаток кусок
  
  
  HAL_StatusTypeDef StatusI2C2 = HAL_ERROR;
  StatusI2C2 =  TOP_I2C_IsDeviceReady(&hi2c2, (uint16_t)(EEPROM24LC128<<1), 2, 1000); // если эта микросхема???
  if (StatusI2C2) return StatusI2C2;
  //пишем не более 64 байт  за раз
  for (int j=0; j<n_pg;j++)
  {
    BufTX[0] = (uint8_t)((address+(j*size_pg))>>8 & 0xFF);
    BufTX[1] = (uint8_t)((address+(j*size_pg)) & 0xFF);
    for (int i=0; i<size_pg; ++i)
    {
      BufTX[2+i] = *ptr++;
    }
    StatusI2C2 = TOP_I2C_Master_Transmit(&hi2c2, (uint16_t)(EEPROM24LC128<<1), BufTX, size_pg+2, 10000); // собственно запись блока не более 64 байт
     HAL_Delay(10);
  }
  
  BufTX[0] = (uint8_t)((address+(n_pg*size_pg))>>8 & 0xFF);
  BufTX[1] = (uint8_t)((address+(n_pg*size_pg)) & 0xFF);
  for (int i=0; i<last_byte; ++i)
  {
    BufTX[2+i] = *ptr++;
  }
  StatusI2C2 = TOP_I2C_Master_Transmit(&hi2c2, (uint16_t)(EEPROM24LC128<<1), BufTX, last_byte+2, 10000); // собственно запись блока не более 64 байт
  HAL_Delay(10);

  
  return StatusI2C2;
}

//функция чтения EEPROM
HAL_StatusTypeDef EEPROM_read(void *buff, uint16_t address, uint32_t byte)
{
  uint8_t BufTX[3];
  HAL_StatusTypeDef StatusI2C2 = HAL_ERROR;
  
    StatusI2C2 =  TOP_I2C_IsDeviceReady(&hi2c2, (uint16_t)(EEPROM24LC128<<1), 2, 1000); // если эта микросхема???
  if (StatusI2C2) return StatusI2C2;
  // читаем нужное число байт - 
  // пробуем прочитать
  // устанавливаем адрес
  BufTX[0] = (uint8_t)(address>>8 & 0xFF);
  BufTX[1] = (uint8_t)(address & 0xFF);
  StatusI2C2 = TOP_I2C_Master_Transmit(&hi2c2, (uint16_t)(EEPROM24LC128<<1), BufTX, 2, 1000);
  TOP_I2C_Master_Receive(&hi2c2, (uint16_t)(EEPROM24LC128<<1), buff, byte, 10000);

  return StatusI2C2;
  }

