#include "eeprom_storage.h"
#include "factory_settings.h"

#ifdef FACTORY_DEBUG_MEMORY_ADDRESS_MAP_LOOK
typedef struct{
  uint32_t memory_start_block_1;
  uint32_t memory_block_1;
  uint32_t memory_start_block_2;
  uint32_t memory_block_2;
  uint32_t memory_start_block_3;
  uint32_t memory_block_3;
  uint32_t memory_start_block_4;
  uint32_t memory_block_4;
  uint32_t memory_start_block_5;
  uint32_t memory_block_5;
  uint32_t memory_start_block_6;
  uint32_t memory_block_6;
  uint32_t memory_start_block_7;
  uint32_t memory_block_7;
  uint32_t memory_start_block_8;
  uint32_t memory_block_8;
  uint32_t memory_start_block_9;
  uint32_t memory_block_9;
  uint32_t memory_start_block_10;
  uint32_t memory_block_10;
  uint32_t memory_start_block_11;
  uint32_t memory_block_11;
  uint32_t memory_start_block_12;
  uint32_t memory_block_12;
  uint32_t memory_start_block_13;
  uint32_t memory_block_13;
  uint32_t memory_start_block_14;
  uint32_t memory_block_14;
  uint32_t memory_start_block_15;
  uint32_t memory_block_15;
  uint32_t memory_block_verification;
  uint32_t memory_end;
}memory_org;

memory_org st_memory_org;

uint8_t check_memory_overlap(void)
{
   
  st_memory_org.memory_start_block_1 = MEMORY_SECTION_1_START_LOC;
  st_memory_org.memory_start_block_2 = MEMORY_SECTION_2_START_LOC;
  st_memory_org.memory_start_block_3 = MEMORY_SECTION_3_START_LOC;
  st_memory_org.memory_start_block_4 = MEMORY_SECTION_4_START_LOC;
  st_memory_org.memory_start_block_5 = MEMORY_SECTION_5_START_LOC;
  st_memory_org.memory_start_block_6 = MEMORY_SECTION_6_START_LOC;
  st_memory_org.memory_start_block_7 = MEMORY_SECTION_7_START_LOC;
  st_memory_org.memory_start_block_8 = MEMORY_SECTION_8_START_LOC;
  st_memory_org.memory_start_block_9 = MEMORY_SECTION_9_START_LOC;
  st_memory_org.memory_start_block_10 = MEMORY_SECTION_10_START_LOC;
  st_memory_org.memory_start_block_11 = MEMORY_SECTION_11_START_LOC;
  st_memory_org.memory_start_block_12 = MEMORY_SECTION_12_START_LOC;
  st_memory_org.memory_start_block_13 = MEMORY_SECTION_13_START_LOC;
  st_memory_org.memory_start_block_14 = MEMORY_SECTION_14_START_LOC;
  st_memory_org.memory_start_block_15 = MEMORY_SECTION_15_START_LOC;
  
  st_memory_org.memory_block_1 = MEMORY_SECTION_1_END_LOC;
  st_memory_org.memory_block_2 = MEMORY_SECTION_2_END_LOC;
  st_memory_org.memory_block_3 = MEMORY_SECTION_3_END_LOC;
  st_memory_org.memory_block_4 = MEMORY_SECTION_4_END_LOC;
  st_memory_org.memory_block_5 = MEMORY_SECTION_5_END_LOC;
  st_memory_org.memory_block_6 = MEMORY_SECTION_6_END_LOC;
  st_memory_org.memory_block_7 = MEMORY_SECTION_7_END_LOC;
  st_memory_org.memory_block_8 = MEMORY_SECTION_8_END_LOC;
  st_memory_org.memory_block_9 = MEMORY_SECTION_9_END_LOC;
  st_memory_org.memory_block_10 = MEMORY_SECTION_10_END_LOC;
  st_memory_org.memory_block_11 = MEMORY_SECTION_11_END_LOC;
  st_memory_org.memory_block_12 = MEMORY_SECTION_12_END_LOC;
  st_memory_org.memory_block_13 = MEMORY_SECTION_13_END_LOC;
  st_memory_org.memory_block_14 = MEMORY_SECTION_14_END_LOC;
  st_memory_org.memory_block_15 = MEMORY_SECTION_15_END_LOC;
  st_memory_org.memory_end = STORAGE_EEPROM_DLMS_LAST_ADDR;
  st_memory_org.memory_block_verification = STORAGE_EEPROM_LAST_VARIFICATION_ADDR;

  if (st_memory_org.memory_block_1 > st_memory_org.memory_start_block_2)
  {
    return 1;
  }
  if (st_memory_org.memory_block_2 > st_memory_org.memory_start_block_3)
  {
    return 1;
  }
  if (st_memory_org.memory_block_3 > st_memory_org.memory_start_block_4)
  {
    return 1;
  }
  if (st_memory_org.memory_block_4 > st_memory_org.memory_start_block_5)
  {
    return 1;
  }
  if (st_memory_org.memory_block_5 > st_memory_org.memory_start_block_6)
  {
    return 1;
  }
  if (st_memory_org.memory_block_6 > st_memory_org.memory_start_block_7)
  {
    return 1;
  }
  if (st_memory_org.memory_block_7 > st_memory_org.memory_start_block_8)
  {
    return 1;
  }
  if (st_memory_org.memory_block_8 > st_memory_org.memory_start_block_9)
  {
    return 1;
  }
  if (st_memory_org.memory_block_9 > st_memory_org.memory_start_block_10)
  {
    return 1;
  }
  if (st_memory_org.memory_block_10 > st_memory_org.memory_start_block_11)
  {
    return 1;
  }
  if (st_memory_org.memory_block_11 > st_memory_org.memory_start_block_12)
  {
    return 1;
  }
  if (st_memory_org.memory_block_12 > st_memory_org.memory_start_block_13)
  {
    return 1;
  }
  if (st_memory_org.memory_block_13 > st_memory_org.memory_start_block_14)
  {
    return 1;
  }
  if (st_memory_org.memory_block_14 > st_memory_org.memory_start_block_15)
  {
    return 1;
  }
  if (st_memory_org.memory_block_15 > st_memory_org.memory_end)
  {
    return 1;
  }
  //TODO: Rakesh, validation for last reserve block required.
  return 0;
}
#endif //MEMORY_ADDRESS_MAP_LOOK