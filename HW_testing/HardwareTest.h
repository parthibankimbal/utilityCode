#ifndef _HARDWARE_TESTING_H
#define _HARDWARE_TESTING_H

#include <stdio.h>
#include "typedef.h"
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "rtc_user.h"
#include "tampers.h"
#include "main.h"
#include "r_cg_it8bit.h"
#include "eeprom_storage.h"
#include "rtc_compensation.h"

void relayTest(uint8_t byConnectState);
uint8_t TestMem(uint8_t byMemType);
uint8_t rfPinsCheck(uint8_t byInput);
uint8_t rfPinsCheck_OnBoard(uint8_t key_index);
uint8_t getInstantParam(uint32_t dAddress, uint8_t *buff, uint16_t byLength);
uint8_t setInstantParam(uint32_t dAddress, uint8_t *buff, uint16_t byLength);
static uint8_t convt_byte_to_bcd(uint8_t byte_data);
static uint8_t convt_bcd_to_byte(uint8_t bcd_data);
void accumulateEnergy_Callback(void);
void display_check_routine(void);

typedef enum
{
  Meter_Number,
  RMS_Volt_Current,
  Active_kwh,
  RTC_Clock,
  Meter_Version_No,
  RF_Key_1,
  RF_Key_2,
  RF_Key_3,
  RF_Key_4,
  RF_Key_5,
  RF_Key_6,

  Board_Number,
  RF_Keys_All,
  Network_Config_RF,
  Network_Config_4G,
  
  HW_Func_Test_Release = 18,
  HW_Switch_Connect_Chk = 19,
  HW_Switch_Release_Chk = 20,
  HW_Func_Test,
  HW_Energy_Acc_Start,
  HW_Energy_Acc_Read,

  Clear_CoverOpen = 30,
  Single_Wire_Offset,
  RTC_calibrate,
  RTC_Comp_Pulse_start,
  RTC_Comp_Pulse_stop,
  RTC_Comp_10_Plus,
  RTC_Comp_10_Less,
  Display_Check,
  Int_Firmware_Version_No,
}E_Set_Get_Instant_Param;

typedef struct 
{
  uint8_t u8AccEnergyStart;
  uint32_t u32AccTime_set;
  float32_t fPhaseEnergy;
  float32_t fNeutralEnergy;
  uint32_t u32AccTime_ms;
  double dEnergyDelta;
  float32_t fPhaseEnergy_kVAh;
  float32_t fNeutralEnergy_kVAh;
  float32_t fPhaseEnergy_kVArh;
  float32_t fNeutralEnergy_kVArh;
}st_Accumulate_Energies;


#endif//RF_COMM_H