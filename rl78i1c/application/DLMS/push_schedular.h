#include <stdint.h>

#ifndef PUSH_SCHEDULAR
#define PUSH_SCHEDULAR
#define INSTANT_MAX_PUSH_SCHEDULE 4
#define DEBUG

#ifdef WIN32
    #pragma pack(push,1)
#else 
    #ifdef __CCRL__
        #pragma pack
    #else
        #error please define proper packing.
    #endif
#endif

typedef struct
{
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
}st_time_sturct;

typedef struct
{
  uint8_t total_actions;
  st_time_sturct push_time[INSTANT_MAX_PUSH_SCHEDULE];
  uint8_t is_push_action;
  uint8_t is_push_send;
  uint32_t last_epoch_push_time;
  uint16_t crc;
}st_push_schedular;

#ifdef WIN32
    #pragma pack(pop)
#else
    #ifdef __CCRL__
        #pragma unpack
    #else
        #error please define proper unpacking.
    #endif
#endif

#ifdef OLD_METHOD
void check_push_time(st_push_schedular* st_push, uint32_t epoch_time);
#else
void check_push_time(st_push_schedular* st_push, uint32_t epoch);
#endif
uint8_t check_greater_time(uint32_t epoch_time, st_time_sturct* st_actionTime);
#endif //PUSH_SCHEDULAR