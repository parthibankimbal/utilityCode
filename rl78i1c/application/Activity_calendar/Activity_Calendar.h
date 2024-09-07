#ifndef __Activity_Calendar_h__
#define __Activity_Calendar_h__

#define MAX_TOD_ZONES           8     //8
#define MAX_DAY_PROFILES        10    //2         //EEPROM
#define MAX_WEEK_PROFILES       5     //2
#define MAX_SEASON_PROFILES     4     //2         //These number of day profiles.. time on which it will be activated

#include <stdint.h>
#include <string.h>
#include "eeprom.h"
#include "push_schedular.h"
#include "rtc_user.h"
#include "bytebuffer.h"
#include "datalog.h"

typedef void(*AC_user_callback)(void); // in header file
typedef void(*AC_time_zone_change_callback_type)(uint8_t zone_id);
extern AC_user_callback pas_to_act_event_callback;
extern AC_time_zone_change_callback_type AC_time_zone_change_callback;
//Pragma Pack TODO:

typedef enum
{
    ACTIVE_CAL,
    PASSIVE_CAL,
}ACTIVE_PASSIVE_CAL;

#ifdef WIN32
#pragma pack(push,1)
#else
#ifdef __CCRL__
#pragma pack
#else
#error please define proper packing.
#endif
#endif

typedef struct tag_tod_passive_to_active
{
  uint8_t  u8ActPending;
  uint32_t u32ActTime;
  uint8_t u8CalendarName[16];
}st_tod_passive_to_active_t;

typedef struct tag_tod_zone
{
  uint8_t u8ID;
  uint8_t u8StartHr;
  uint8_t u8StartMin;
}st_tod_zone_t;

typedef struct tag_tod_day
{
  uint8_t u8DayID;
  uint8_t u8ActiveZones;
  st_tod_zone_t stTimeZones[MAX_TOD_ZONES];
}st_tod_day_t;

typedef struct tag_tod_week
{
  //uint8_t u8WeekID;
  uint8_t u8DayID[7];
  uint8_t u8WeekName[10];
}st_tod_week_t;

typedef struct tag_tod_season
{
  uint8_t u8StartDt;
  uint8_t u8StartMon;
  uint8_t u8StartHr;
  uint8_t u8StartMin;
  uint8_t u8StartSec;
  //uint8_t u8WeekID;
  uint8_t u8WeekName[10];
  uint8_t u8SeasonName[10];
}st_tod_season_t;

#ifdef WIN32
#pragma pack(pop)
#else
#ifdef __CCRL__
#pragma unpack
#else
#error please define proper unpacking.
#endif
#endif

#define DEFAULT_CALENDAR_NAME                   "Calendar_1"

#define TOD_DAY_1_ID                            1
#define TOD_DAY_2_ID                            2


#define TOD_TARIFF_REG_1                        1
#define TOD_TARIFF_REG_2                        2
#define TOD_TARIFF_REG_3                        3
#define TOD_TARIFF_REG_4                        4
#define TOD_TARIFF_REG_5                        5
#define TOD_TARIFF_REG_6                        6
#define TOD_TARIFF_REG_7                        7
#define TOD_TARIFF_REG_8                        8

#define DEFAULT_DAY_ID_TOTAL             		1//2

#define DEFAULT_TOD_DAY_PROFILE_ACTIONS         {TOD_DAY_1_ID, \
                                                           3,  \
                                                            {TOD_TARIFF_REG_3,  6,  0, \
                                                             TOD_TARIFF_REG_1, 17,  0, \
                                                             TOD_TARIFF_REG_2, 22,  0, \
                                                                            0,  0,  0, \
                                                                            0,  0,  0, \
                                                                            0,  0,  0, \
                                                                            0,  0,  0, \
                                                                            0,  0,  0},\
                                                TOD_DAY_2_ID, \
                                                          3,  \
                                                            {TOD_TARIFF_REG_1,  0,  0, \
                                                             TOD_TARIFF_REG_2,  8,  0, \
                                                             TOD_TARIFF_REG_3, 16,  0, \
                                                                            0,  0,  0, \
                                                                            0,  0,  0, \
                                                                            0,  0,  0, \
                                                                            0,  0,  0, \
                                                                            0,  0,  0},\
                                                }

#define WEEK_1                                  "Week_1"
#define WEEK_2                                  "Week_2"
#define DEFAULT_WEEK_COUNT                  	1//2
#define DEFAULT_WEEK_NAMES                   	{WEEK_1, WEEK_2}
#define DEFAULT_WEEK_PROFILES				    {{TOD_DAY_1_ID, TOD_DAY_1_ID, TOD_DAY_1_ID, TOD_DAY_1_ID, TOD_DAY_1_ID, TOD_DAY_1_ID, TOD_DAY_1_ID},\
                                                 {TOD_DAY_2_ID, TOD_DAY_2_ID, TOD_DAY_2_ID, TOD_DAY_2_ID, TOD_DAY_2_ID, TOD_DAY_2_ID, TOD_DAY_2_ID}\
                                                }
//REVIEW:Rakesh, Check if passive calendar okay to activate.

#define DEFAULT_SEASON_COUNT                    1//2
#define DEFAULT_SEASON_NAME                     {"Season_1", "Season_2"}
#define DEFAULT_SEASON_START_TIME               {{ 1, 1, 0, 0, 0},{ 7, 1, 0, 0, 0}} //month-day hr:min:sec
#define DEFAULT_SEASON_WEEK_BOUND               {WEEK_1, WEEK_2}

///
void set_default_activity_calendar(void);
///
uint8_t activity_calendar_polling_process(uint32_t u32Epoch);

void activate_passive_calendar(uint8_t * u8TempReadBuf);
uint8_t get_season(uint32_t u32Epoch, uint8_t* u8LocBuf);
uint8_t getWeekID(uint32_t u32Epoch, uint8_t u8SeasonID, uint8_t* u8LocBuf);
uint8_t getDayID(uint32_t u32Epoch, uint8_t u8WeekID, uint8_t* u8LocBuf);
uint8_t getZoneID(uint32_t u32Epoch, uint8_t u8DayID, uint8_t* u8LocBuf);

uint8_t get_calendar_name_active(uint8_t* name);
uint8_t get_calendar_name_passive(uint8_t* name);
uint8_t set_calendar_name_passive(uint8_t* name, uint8_t size);
void get_season_profile_active(uint8_t* data);
void get_season_profile_passive(uint8_t* data);
void set_season_profile_passive(uint8_t* data, uint8_t num_of_season);
void get_week_profile_active(uint8_t* data);
void get_week_profile_passive(uint8_t* data);
void set_week_profile_passive(uint8_t* data, uint8_t num_of_week);
void get_day_profile_active(uint8_t* data);
void get_day_profile_passive(uint8_t* data);
void set_day_profile_passive(uint8_t* data, uint8_t num_of_days);
uint32_t get_calendar_activation_time(void);
void set_calendar_activation_time(uint32_t epoch);
#endif // !__Activity_Calendar_h__
