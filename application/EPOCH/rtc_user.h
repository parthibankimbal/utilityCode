#ifndef RTC_USER_H
#define RTC_USER_H

#include "string.h"
#include "r_cg_macrodriver.h"
#include "r_cg_rtc.h"
#include "epoch.h"
#include "factory_settings.h"

#define TIME_ZONE DEFUALT_TIME_ZONE

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
  stTime_struct dateTime;
  uint32_t epoch;
  uint8_t rtc_tick;
  uint8_t rtc_tick_per_second;
  uint8_t rtc_tick_per_10_second;
  uint8_t rtc_tick_per_minutes;
  uint8_t rtc_tick_per_3_minutes;
  uint8_t rtc_tick_per_5_minutes;
  uint8_t rtc_status;
  uint8_t battery_status;
}st_RTC_time;
#ifdef WIN32
#pragma pack(pop)
#else
#ifdef __CCRL__
#pragma unpack
#else
#error please define proper unpacking.
#endif
#endif

typedef void(*RTC_interrupt_callback)(st_RTC_time* RTC_Time);
typedef void(*RTC_action_callback)(st_RTC_time* RTC_Time);

extern RTC_interrupt_callback rtc_per_second_callback;
extern RTC_interrupt_callback rtc_per_10_second_callback;
extern RTC_interrupt_callback rtc_per_minute_callback;
extern RTC_action_callback rtc_post_set_callback;
extern st_RTC_time g_RTC_time;

static uint8_t convt_byte_to_bcd(uint8_t byte_data);
static uint8_t convt_bcd_to_byte(uint8_t bcd_data);
void rtc_interrupt_callback(void);
void set_RTC_dateTime(stTime_struct *dateTime);
void get_RTC_dateTime(stTime_struct *dateTime);
void adjust_RTC_dateTime(int8_t i8Correction);
void get_RTC_dateTime_with_epoch(st_RTC_time *g_RTC_time);
uint32_t get_epoch(void);
uint8_t is_RTC_okay(st_RTC_time *RTC_time);
void set_default_rtc_params(void);
void Epoch_To_DLMS_Time_With_RTC_Status(uint32_t epoch, uint8_t* DLMS_Time, st_RTC_time *RTC_time);
void init_rtc_wrapper(void);
#endif