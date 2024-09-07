#include <stdio.h>
#include <stdlib.h>
#ifdef __CCRL__
#include "r_cg_macrodriver.h"
#else
#include <stdint.h>
#endif
#ifndef EPOCH_H
#define EPOCH_H
#define TIME_DEVIATION 0xFEB6
typedef unsigned long DWORD;

#ifdef WIN32
#pragma pack(push,1)
#else
#ifdef __CCRL__
#pragma pack
#else
#error please define proper packing.
#endif
#endif

typedef struct Time {
    unsigned long year;
    unsigned long month;
    unsigned long day;
    unsigned long weekday;
    unsigned long hour;
    unsigned long minute;
    unsigned long second;
}stTime_struct;

typedef struct
{
    unsigned char second; // 0-59
    unsigned char minute; // 0-59
    unsigned char hour;   // 0-23
    unsigned char weekday;   // 0-23
    unsigned char day;    // 1-31
    unsigned char month;  // 1-12
    unsigned char year;   // 0-99 (representing 2000-2099)
} date_time_t;
#ifdef WIN32
#pragma pack(pop)
#else
#ifdef __CCRL__
#pragma unpack
#else
#error please define proper unpacking.
#endif
#endif

void Epoch_To_DLMS_Time(uint32_t epoch, uint8_t* DLMS_Time);
DWORD DLMS_Time_To_Epoch(uint8_t* DLMS_Time);
DWORD Time_To_Epoch(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
void stTime_To_DLMS_Time(stTime_struct stTime, uint8_t * DLMS_Time);
stTime_struct DLMS_Time_To_StTime(uint8_t* DLMS_Time);
DWORD Time_Convert_TO2TS(stTime_struct * time);
void UnixToDatteTime(DWORD time, stTime_struct *ToTime);
void convert_to_local_time(stTime_struct *Time, signed short Deviation);
void convert_to_unix_time(stTime_struct *Time, signed short Deviation);
stTime_struct add_year(stTime_struct *Time, unsigned short years);
stTime_struct add_months(stTime_struct *Time, unsigned long months);
stTime_struct add_days(stTime_struct *Time, unsigned long days);
stTime_struct add_hours(stTime_struct *Time, unsigned long hours);
stTime_struct add_minutes(stTime_struct *Time, unsigned long minutes);
stTime_struct add_seconds(stTime_struct *Time, long seconds);
unsigned long days_since_epoch(unsigned long epoch_value);
unsigned long hours_since_epoch(unsigned long epoch_value);

#endif //EPOCH_H