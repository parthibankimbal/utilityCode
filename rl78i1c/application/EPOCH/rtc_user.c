#include "rtc_user.h"
#include "eeprom_storage.h"
#include "main.h"

st_RTC_time g_RTC_time;
void rtc_interrupt_callback(void);
static void set_rtc_hal_dateTime(date_time_t *dateTime);
static void get_rtc_hal_dateTime(date_time_t *dateTime);

RTC_interrupt_callback rtc_per_second_callback = 0;
RTC_interrupt_callback rtc_per_10_second_callback = 0;
RTC_interrupt_callback rtc_per_minute_callback = 0;
RTC_action_callback rtc_post_set_callback = 0;

static uint8_t convt_byte_to_bcd(uint8_t byte_data)
{
    return ((byte_data/10)<<4)+(byte_data % 10);
}

static uint8_t convt_bcd_to_byte(uint8_t bcd_data)
{
    return((bcd_data&0xF0)>>4)*10 + (bcd_data&0x0F);
}

void set_RTC_dateTime(stTime_struct *dateTime)
{
    date_time_t time;
    uint32_t epoch;
    stTime_struct _dateTime = *dateTime;
    
    epoch = Time_Convert_TO2TS(dateTime);
    UnixToDatteTime(epoch, dateTime);
    _dateTime.weekday = dateTime->weekday;
    if(memcmp(&_dateTime, dateTime,sizeof(stTime_struct)) != 0)
    {
      return;
    }
    
    time.second = convt_byte_to_bcd((uint8_t)dateTime->second);
    time.minute = convt_byte_to_bcd((uint8_t)dateTime->minute);
    time.hour = convt_byte_to_bcd((uint8_t)dateTime->hour);
    time.weekday = convt_byte_to_bcd((uint8_t)dateTime->weekday);
    time.day = convt_byte_to_bcd((uint8_t)dateTime->day);
    time.month = convt_byte_to_bcd((uint8_t)dateTime->month);
    time.year = convt_byte_to_bcd((uint8_t)(dateTime->year - 2000));
    set_rtc_hal_dateTime(&time);
    get_RTC_dateTime_with_epoch(&g_RTC_time);
    if(rtc_post_set_callback != 0)
    {
        rtc_post_set_callback(&g_RTC_time);
    }
    rtc_per_second_callback(&g_RTC_time);
    rtc_per_minute_callback(&g_RTC_time);
}

void get_RTC_dateTime(stTime_struct *dateTime)
{
    date_time_t time;
    
    get_rtc_hal_dateTime(&time);
    dateTime->second = convt_bcd_to_byte(time.second);
    dateTime->minute = convt_bcd_to_byte(time.minute);
    dateTime->hour = convt_bcd_to_byte(time.hour);
    dateTime->weekday = convt_bcd_to_byte(time.weekday);
    dateTime->day = convt_bcd_to_byte(time.day);
    dateTime->month = convt_bcd_to_byte(time.month);
    dateTime->year = 2000 + convt_bcd_to_byte(time.year);
}

void adjust_RTC_dateTime(int8_t i8Correction)
{
    stTime_struct dateTime;
    get_RTC_dateTime(&dateTime);
    dateTime = add_seconds(&dateTime, i8Correction);
    set_RTC_dateTime(&dateTime);
}

void get_RTC_dateTime_with_epoch(st_RTC_time *RTC_time)
{
    get_RTC_dateTime(&RTC_time->dateTime);
    RTC_time->epoch = Time_Convert_TO2TS(&RTC_time->dateTime);
}

void rtc_interrupt_callback(void)
{
    static uint32_t last_epoch;
    g_RTC_time.rtc_tick++;
    if(last_epoch != g_RTC_time.epoch)
    {
        last_epoch = g_RTC_time.epoch;
        if(rtc_per_second_callback != 0)
        {
            rtc_per_second_callback(&g_RTC_time);
        }
        if((g_RTC_time.epoch%10) == 0)
        {
            if(rtc_per_10_second_callback != 0)
            {
                rtc_per_10_second_callback(&g_RTC_time);
            }
        }
        if((g_RTC_time.epoch%60) == 0)
        {
            if(rtc_per_minute_callback != 0)
            {
                rtc_per_minute_callback(&g_RTC_time);
            }
        }
    }
}

uint32_t get_epoch(void)
{
    st_RTC_time RTC_time;
    get_RTC_dateTime_with_epoch(&RTC_time);
    return RTC_time.epoch;
}

uint8_t is_RTC_okay(st_RTC_time *RTC_time)
{
    if((RTC_time->dateTime.year < 2010) || (RTC_time->dateTime.year > 2060) || (RTC_time->dateTime.month > 12) || (RTC_time->dateTime.month < 1)
        || (RTC_time->dateTime.day > 31) || (RTC_time->dateTime.day < 1) || (RTC_time->dateTime.hour > 23) || (RTC_time->dateTime.minute > 59) || (RTC_time->dateTime.second > 59) ||(is_RTC_battery_low())) 
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void set_default_rtc_params(void)
{
    to_eeprom(CLOCK_TIME_ZONE_LOC, (uint64_t)TIME_ZONE, CLOCK_TIME_ZONE_SIZE);
}

void Epoch_To_DLMS_Time_With_RTC_Status(uint32_t epoch, uint8_t* DLMS_Time, st_RTC_time *RTC_time)
{
  Epoch_To_DLMS_Time(epoch, DLMS_Time);
  DLMS_Time[11] = 0;
  if(RTC_time->rtc_status)
  {
    DLMS_Time[11] |= 0x01;
  }
  if(RTC_time->battery_status)
  {
    DLMS_Time[11] |= 0x10;
  }
  else
  {
    DLMS_Time[11] &= ~0x10;
  }
}

static void set_rtc_hal_dateTime(date_time_t *dateTime)
{
    rtc_counter_value_t counter_write_val;
    
    counter_write_val.sec = dateTime->second;
    counter_write_val.min = dateTime->minute;
    counter_write_val.hour = dateTime->hour;
    counter_write_val.week = dateTime->weekday;
    counter_write_val.day = dateTime->day;
    counter_write_val.month = dateTime->month;
    counter_write_val.year = dateTime->year;
    R_RTC_Set_CalendarCounterValue(counter_write_val);
}

static void get_rtc_hal_dateTime(date_time_t *dateTime)
{
    rtc_counter_value_t counter_write_val;
    
    R_RTC_Get_CalendarCounterValue(&counter_write_val);
    dateTime->second = counter_write_val.sec;
    dateTime->minute = counter_write_val.min;
    dateTime->hour = counter_write_val.hour;
    dateTime->weekday = counter_write_val.week;
    dateTime->day = counter_write_val.day;
    dateTime->month = counter_write_val.month;
    dateTime->year = counter_write_val.year;
}

void init_rtc_wrapper(void)
{
  fp_rtc_interrupt_driver_callback = rtc_interrupt_callback;
}
/* End user code. Do not edit comment generated here */