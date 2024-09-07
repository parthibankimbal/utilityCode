#include "push_schedular_user.h"
#include "eeprom_storage.h"

const st_push_schedular const_st_push_schedular = DEFAULT_INSTANT_PROFILE_PUSH_ACTIONS;
st_push_schedular st_Instant_push_time;

uint16_t volatile flag = 0;
void set_default_instant_push_profile_time(void)
{
  memcpy((uint8_t*)&st_Instant_push_time, (uint8_t*)&const_st_push_schedular, STORAGE_DLMS_INSTANT_PUSH_TIME_SIZE);
  set_instant_push_profile_time(&st_Instant_push_time);
}

void set_instant_push_profile_time(st_push_schedular* Instant_push_time)
{
  R_CRC_Set(0xFFFF);
  st_Instant_push_time.crc = R_CRC_Calculate((uint8_t*)&st_Instant_push_time, STORAGE_DLMS_INSTANT_PUSH_TIME_SIZE - 2);
  write_page_eeprom(STORAGE_DLMS_INSTANT_PUSH_TIME_ADDR, (uint8_t*)Instant_push_time, STORAGE_DLMS_INSTANT_PUSH_TIME_SIZE);
}

void get_instant_push_profile_time(st_push_schedular* Instant_push_time)
{
  read_page_eeprom(STORAGE_DLMS_INSTANT_PUSH_TIME_ADDR, (uint8_t*)Instant_push_time, STORAGE_DLMS_INSTANT_PUSH_TIME_SIZE);
  R_CRC_Set(0xFFFF);
  if(Instant_push_time->crc != R_CRC_Calculate((uint8_t*)Instant_push_time, STORAGE_DLMS_INSTANT_PUSH_TIME_SIZE - 2))
  {
    set_default_instant_push_profile_time();
  }
}