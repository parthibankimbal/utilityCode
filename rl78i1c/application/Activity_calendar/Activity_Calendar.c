#include "Activity_Calendar.h"
#include "eeprom_storage.h"
  
AC_user_callback pas_to_act_event_callback = 0;
AC_time_zone_change_callback_type AC_time_zone_change_callback = 0;

uint8_t get_calendar_name_active(uint8_t* name)
{
  read_page_eeprom(STORAGE_EEPROM_DLMS_CALENDAR_NAME_ACTIVE_ADDR, name, STORAGE_EEPROM_DLMS_CALENDAR_NAME_ACTIVE_SIZE);
  return strlen((const char*)name);
}

uint8_t get_calendar_name_passive(uint8_t* name)
{
  st_tod_passive_to_active_t passvie_cal;
  read_page_eeprom(STORAGE_EEPROM_DLMS_PASSIVE_TO_ACTIVE_ADDR, (uint8_t*)&passvie_cal, sizeof(st_tod_passive_to_active_t));
  memcpy(name, (const char*)passvie_cal.u8CalendarName, STORAGE_EEPROM_DLMS_CALENDAR_NAME_ACTIVE_SIZE);
  return strlen((const char*)name);
}

uint8_t set_calendar_name_passive(uint8_t* name, uint8_t size)
{
  st_tod_passive_to_active_t passvie_cal;
  
  if(size >= STORAGE_EEPROM_DLMS_CALENDAR_NAME_ACTIVE_SIZE)
  {
      return 1;
  }
  read_page_eeprom(STORAGE_EEPROM_DLMS_PASSIVE_TO_ACTIVE_ADDR, (uint8_t*)&passvie_cal, sizeof(st_tod_passive_to_active_t));
  memcpy(passvie_cal.u8CalendarName, (const char*)name, size);
  passvie_cal.u8CalendarName[size] = 0;
  write_page_eeprom(STORAGE_EEPROM_DLMS_PASSIVE_TO_ACTIVE_ADDR, (uint8_t*)&passvie_cal, sizeof(st_tod_passive_to_active_t));
  return 0;
}

void get_season_profile(uint8_t* data, ACTIVE_PASSIVE_CAL eCalendar)
{
    uint8_t season;
    uint32_t num_of_season;
    st_tod_season_t season_data;
    gxByteBuffer bb;
    uint32_t season_loaction = STORAGE_EEPROM_DLMS_ACTIVE_SEASON_BLK_ADDR;
    uint32_t season_count_loaction = STORAGE_EEPROM_DLMS_ACTIVE_SEASON_CNT_ADDR;
    
    bb_attach(&bb, data, 1200);
    bb_reset(&bb);
    
    if(eCalendar == PASSIVE_CAL)
    {
      season_loaction = STORAGE_EEPROM_DLMS_PASSIVE_SEASON_BLK_ADDR;
      season_count_loaction = STORAGE_EEPROM_DLMS_PASSIVE_SEASON_CNT_ADDR;
    }
    num_of_season = read_eeprom(season_count_loaction);
    
    bb_setUInt8(&bb, num_of_season);
    for(season = 0; season < num_of_season; season++)
    {
      read_page_eeprom(season_loaction + (sizeof(st_tod_season_t)*season),(uint8_t*)&season_data, sizeof(st_tod_season_t));
      bb_setUInt8(&bb, 2);
      bb_setUInt8(&bb, 3);
      bb_setUInt8(&bb, 9);
      bb_setUInt8(&bb, strlen((const char*)season_data.u8SeasonName));
      bb_set(&bb, season_data.u8SeasonName, strlen((const char*)season_data.u8SeasonName));
      bb_setUInt8(&bb, 9);
      bb_setUInt8(&bb, 12);
      bb_setUInt16(&bb, 0xFFFF);
      bb_setUInt8(&bb, season_data.u8StartMon);
      bb_setUInt8(&bb, season_data.u8StartDt);
      bb_setUInt8(&bb, 0xFF);
      bb_setUInt8(&bb, season_data.u8StartHr);
      bb_setUInt8(&bb, season_data.u8StartMin);
      bb_setUInt8(&bb, season_data.u8StartSec);
      bb_setUInt8(&bb, 0xFF);
      bb_setUInt16(&bb, (uint16_t)TIME_ZONE);
      bb_setUInt8(&bb, 0);
      bb_setUInt8(&bb, 9);
      bb_setUInt8(&bb, strlen((const char*)season_data.u8WeekName));
      bb_set(&bb, season_data.u8WeekName, strlen((const char*)season_data.u8WeekName));
    }
    
    if (bb.size < 0x80)
    {
      bb_move(&bb, 0, 1, bb.size);
      bb_setUInt8ByIndex(&bb, 0, bb.size-1);
    }
    else if (bb.size < 0x100)
    {
      bb_move(&bb, 0, 2, bb.size);
      bb_setUInt8ByIndex(&bb, 0, 0x81);
      bb_setUInt8ByIndex(&bb, 1, bb.size-1);
    }
    else if (bb.size < 0x10000)
    {
      bb_move(&bb, 0, 3, bb.size);
      bb_setUInt8ByIndex(&bb, 0, 0x82);
      bb_setUInt16ByIndex(&bb, 1, bb.size-1);
    }
    return;
}

void get_season_profile_active(uint8_t* data)
{
  get_season_profile(data, ACTIVE_CAL);
}

void get_season_profile_passive(uint8_t* data)
{
  get_season_profile(data, PASSIVE_CAL);
}

void set_season_profile_passive(uint8_t* data, uint8_t num_of_season)
{
    uint8_t season;
    uint8_t length;
    st_tod_season_t season_data;
    gxByteBuffer bb;
    
    bb_attach(&bb, data, 1200);
    write_eeprom(STORAGE_EEPROM_DLMS_PASSIVE_SEASON_CNT_ADDR, num_of_season);
    
    for(season = 0; season < num_of_season; season++)
    {
      bb.position += 3;
      bb_getUInt8(&bb, &length);
      if(length > 9)
      {
          length = 9;
      }
      bb_get(&bb, season_data.u8SeasonName, length);
      season_data.u8SeasonName[length] = 0;
      bb.position += 4;
      bb_getUInt8(&bb, &season_data.u8StartMon);
      bb_getUInt8(&bb, &season_data.u8StartDt);
      bb.position++;
      bb_getUInt8(&bb, &season_data.u8StartHr);
      bb_getUInt8(&bb, &season_data.u8StartMin);
      bb_getUInt8(&bb, &season_data.u8StartSec);
      bb.position += 5;
      bb_getUInt8(&bb, &length);
      if(length > 9)
      {
          length = 9;
      }
      bb_get(&bb, season_data.u8WeekName, length);
      season_data.u8WeekName[length] = 0;
      
      write_page_eeprom(STORAGE_EEPROM_DLMS_PASSIVE_SEASON_BLK_ADDR + (sizeof(st_tod_season_t)*season),(uint8_t*)&season_data, sizeof(st_tod_season_t));
    }
}

void get_week_profile(uint8_t* data, ACTIVE_PASSIVE_CAL eCalendar)
{
    uint8_t week;
    uint32_t num_of_week;
    st_tod_week_t week_data;
    uint8_t day_ID;
    gxByteBuffer bb;
    uint32_t week_loaction = STORAGE_EEPROM_DLMS_ACTIVE_WEEK_BLK_ADDR;
    uint32_t week_count_loaction = STORAGE_EEPROM_DLMS_ACTIVE_WEEK_CNT_ADDR;
    
    bb_attach(&bb, data, 1200);
    bb_reset(&bb);
    
    if(eCalendar == PASSIVE_CAL)
    {
      week_loaction = STORAGE_EEPROM_DLMS_PASSIVE_WEEK_BLK_ADDR;
      week_count_loaction = STORAGE_EEPROM_DLMS_PASSIVE_WEEK_CNT_ADDR;
    }
    num_of_week = read_eeprom(week_count_loaction);
    
    bb_setUInt8(&bb, num_of_week);
    for(week = 0; week < num_of_week; week++)
    {
      read_page_eeprom(week_loaction + (sizeof(st_tod_week_t)*week),(uint8_t*)&week_data, sizeof(st_tod_week_t));
      bb_setUInt8(&bb, 2);
      bb_setUInt8(&bb, 8);
      bb_setUInt8(&bb, 9);
      bb_setUInt8(&bb, strlen((const char*)week_data.u8WeekName));
      bb_set(&bb, week_data.u8WeekName, strlen((const char*)week_data.u8WeekName));
      
      for(day_ID = 0; day_ID < 7; day_ID++)
      {
        bb_setUInt8(&bb, 0x11);
        bb_setUInt8(&bb, week_data.u8DayID[day_ID]);
      }
    }

    if (bb.size < 0x80)
    {
      bb_move(&bb, 0, 1, bb.size);
      bb_setUInt8ByIndex(&bb, 0, bb.size-1);
    }
    else if (bb.size < 0x100)
    {
      bb_move(&bb, 0, 2, bb.size);
      bb_setUInt8ByIndex(&bb, 0, 0x81);
      bb_setUInt8ByIndex(&bb, 1, bb.size-1);
    }
    else if (bb.size < 0x10000)
    {
      bb_move(&bb, 0, 3, bb.size);
      bb_setUInt8ByIndex(&bb, 0, 0x82);
      bb_setUInt16ByIndex(&bb, 1, bb.size-1);
    }
    return;
}

void get_week_profile_active(uint8_t* data)
{
    get_week_profile(data, ACTIVE_CAL);
}

void get_week_profile_passive(uint8_t* data)
{
    get_week_profile(data, PASSIVE_CAL);
}

void set_week_profile_passive(uint8_t* data, uint8_t num_of_week)
{
    uint8_t week;
    uint8_t week_name_length;
    st_tod_week_t week_data;
    uint8_t day_ID;
    gxByteBuffer bb;
    uint32_t location;
    
    bb_attach(&bb, data, 1200);
    write_eeprom(STORAGE_EEPROM_DLMS_PASSIVE_WEEK_CNT_ADDR, num_of_week);
    
    for(week = 0; week < num_of_week; week++)
    {
      bb.position += 3;
      bb_getUInt8(&bb, &week_name_length);
      if(week_name_length > 9)
      {
          week_name_length = 9;
      }
      bb_get(&bb, week_data.u8WeekName, week_name_length);
      
      week_data.u8WeekName[week_name_length] = 0;
      for(day_ID = 0; day_ID < 7; day_ID++)
      {
        bb.position++;
        bb_getUInt8(&bb, &week_data.u8DayID[day_ID]);
      }
      location = STORAGE_EEPROM_DLMS_PASSIVE_WEEK_BLK_ADDR + (sizeof(st_tod_week_t)*week);
      write_page_eeprom(location,(uint8_t*)&week_data, sizeof(st_tod_week_t));
    }
}

void get_day_profile(uint8_t* data, ACTIVE_PASSIVE_CAL eCalendar)
{
    uint8_t day;
    uint8_t num_of_days;
    uint8_t zone;
    gxByteBuffer bb;
    st_tod_day_t tod_day;
    uint32_t day_loaction = STORAGE_EEPROM_DLMS_ACTIVE_DAY_BLK_ADDR;
    uint32_t day_count_loaction = STORAGE_EEPROM_DLMS_ACTIVE_DAY_CNT_ADDR;
    uint8_t Tod_struct[] = {0x02, //tag struct
                                0x03, //num of elements
                                    0x09, // tag octat string 
                                        0x04, // num of uint8 
                                            0x02, 0x00, 0x00, 0xFF, //hr:min:sec:MS
                                    0x09,  // tag octat string 
                                        0x06, // num of uint8 
                                            0x00, 0x00, 0x0A, 0x00, 0x64, 0xFF, //dummy OBIS code, 0.0.10.0.0.255
                                    0x12, //Tag DLMS long-Uint
                                        0x00, 0x01 // value script selector ID
                            };
    
    bb_attach(&bb, data, 1200);
    bb_reset(&bb);
    
    if(eCalendar == PASSIVE_CAL)
    {
      day_loaction = STORAGE_EEPROM_DLMS_PASSIVE_DAY_BLK_ADDR;
      day_count_loaction = STORAGE_EEPROM_DLMS_PASSIVE_DAY_CNT_ADDR;
    }
    
    num_of_days = read_eeprom(day_count_loaction);
    bb_setUInt8(&bb, num_of_days);
    for(day = 0; day < num_of_days; day++)
    {
      read_page_eeprom(day_loaction + (sizeof(st_tod_day_t)*(day)),(uint8_t*)&tod_day, sizeof(st_tod_day_t));
      bb_setUInt8(&bb, 2);// Tag struct
      bb_setUInt8(&bb, 2);//num of elements
      bb_setUInt8(&bb, 0x11);//tag uint
      bb_setUInt8(&bb, tod_day.u8DayID);//day ID
      bb_setUInt8(&bb, 1);// tag array
      bb_setUInt8(&bb, tod_day.u8ActiveZones);//num of zones
      for(zone = 0; zone < tod_day.u8ActiveZones; zone++)
      {
        Tod_struct[4] = tod_day.stTimeZones[zone].u8StartHr; //hr
        Tod_struct[5] = tod_day.stTimeZones[zone].u8StartMin;//min
        Tod_struct[18] = tod_day.stTimeZones[zone].u8ID;//sec, MS skipped to 0xFF
        bb_set(&bb, Tod_struct, sizeof(Tod_struct));//fill data to buffer
      }
    }
    
    if (bb.size < 0x80)
    {
      bb_move(&bb, 0, 1, bb.size);
      bb_setUInt8ByIndex(&bb, 0, bb.size-1);
    }
    else if (bb.size < 0x100)
    {
      bb_move(&bb, 0, 2, bb.size);
      bb_setUInt8ByIndex(&bb, 0, 0x81);
      bb_setUInt8ByIndex(&bb, 1, bb.size-1);
    }
    else if (bb.size < 0x10000)
    {
      bb_move(&bb, 0, 3, bb.size);
      bb_setUInt8ByIndex(&bb, 0, 0x82);
      bb_setUInt16ByIndex(&bb, 1, bb.size-1);
    }
    return;
}

void get_day_profile_active(uint8_t* data)
{
  get_day_profile(data, ACTIVE_CAL);
}

void get_day_profile_passive(uint8_t* data)
{
  get_day_profile(data, PASSIVE_CAL);
}

void set_day_profile_passive(uint8_t* data, uint8_t num_of_days)
{
    uint8_t day;
    uint8_t zone;
    gxByteBuffer bb;
    st_tod_day_t tod_day;
    
    bb_attach(&bb, data, 1200);
    write_eeprom(STORAGE_EEPROM_DLMS_PASSIVE_DAY_CNT_ADDR, num_of_days);
    for(day = 0; day < num_of_days; day++)
    {
      bb.position += 3;
      bb_getUInt8(&bb, &tod_day.u8DayID);
      bb.position++;
      bb_getUInt8(&bb, &tod_day.u8ActiveZones);
      if(tod_day.u8ActiveZones > MAX_TOD_ZONES)
      {
        tod_day.u8ActiveZones = MAX_TOD_ZONES;
      }
      for(zone = 0; zone < tod_day.u8ActiveZones; zone++)
      {
        bb.position += 4;
        bb_getUInt8(&bb, &tod_day.stTimeZones[zone].u8StartHr);
        bb_getUInt8(&bb, &tod_day.stTimeZones[zone].u8StartMin);
        bb.position += 12;
        bb_getUInt8(&bb, &tod_day.stTimeZones[zone].u8ID);
        if(tod_day.stTimeZones[zone].u8ID > MAX_TOD_ZONES)
        {
          tod_day.stTimeZones[zone].u8ID = MAX_TOD_ZONES;
        }
      }
      write_page_eeprom(STORAGE_EEPROM_DLMS_PASSIVE_DAY_BLK_ADDR + (sizeof(st_tod_day_t)*day),(uint8_t*)&tod_day, sizeof(st_tod_day_t));
    }
}
uint32_t get_calendar_activation_time(void)
{
  st_tod_passive_to_active_t passvie_cal;
  read_page_eeprom(STORAGE_EEPROM_DLMS_PASSIVE_TO_ACTIVE_ADDR, (uint8_t*)&passvie_cal, sizeof(st_tod_passive_to_active_t));
  return passvie_cal.u32ActTime;
}

void set_calendar_activation_time(uint32_t epoch)
{
  st_tod_passive_to_active_t passvie_cal;
  read_page_eeprom(STORAGE_EEPROM_DLMS_PASSIVE_TO_ACTIVE_ADDR, (uint8_t*)&passvie_cal, sizeof(st_tod_passive_to_active_t));
  passvie_cal.u32ActTime = epoch;
  passvie_cal.u8ActPending = 1;
  write_page_eeprom(STORAGE_EEPROM_DLMS_PASSIVE_TO_ACTIVE_ADDR, (uint8_t*)&passvie_cal, sizeof(st_tod_passive_to_active_t));
  return;
}
//#pragma section ACAL_DATA
const __far uint8_t season_StartTime[][5] = DEFAULT_SEASON_START_TIME;
const __far uint8_t week_profiles[][7] = DEFAULT_WEEK_PROFILES;
const __far st_tod_day_t const_day_profile[] = DEFAULT_TOD_DAY_PROFILE_ACTIONS;
//#pragma section
const char* season_Names[] = DEFAULT_SEASON_NAME;
const char* week_bound[] = DEFAULT_SEASON_WEEK_BOUND;
const char* week_names[] = DEFAULT_WEEK_NAMES;

void set_default_activity_calendar(void)
{
    uint8_t loop, inner_loop;
    
    typedef union{
        st_tod_season_t season_data;
        st_tod_week_t week_data;
        st_tod_day_t tod_day;
    }un;
    un union_st;
    //cal name
    write_page_eeprom(STORAGE_EEPROM_DLMS_CALENDAR_NAME_ACTIVE_ADDR, (uint8_t*)DEFAULT_CALENDAR_NAME, sizeof(DEFAULT_CALENDAR_NAME));
    write_page_eeprom(STORAGE_EEPROM_DLMS_PASSIVE_TO_ACTIVE_ADDR + offsetof(st_tod_passive_to_active_t, u8CalendarName), (uint8_t*)DEFAULT_CALENDAR_NAME, member_size(st_tod_passive_to_active_t, u8CalendarName));
    
    //season
    write_eeprom(STORAGE_EEPROM_DLMS_ACTIVE_SEASON_CNT_ADDR, DEFAULT_SEASON_COUNT);
    write_eeprom(STORAGE_EEPROM_DLMS_PASSIVE_SEASON_CNT_ADDR, DEFAULT_SEASON_COUNT);
    for(loop = 0; loop < DEFAULT_SEASON_COUNT; loop++)
    {
        memcpy(union_st.season_data.u8SeasonName, season_Names[loop], strlen(season_Names[loop]) + 1);
        memcpy(union_st.season_data.u8WeekName, week_bound[loop], strlen(week_bound[loop]) + 1);
        union_st.season_data.u8StartMon = season_StartTime[loop][0];
        union_st.season_data.u8StartDt =  season_StartTime[loop][1];
        union_st.season_data.u8StartHr =  season_StartTime[loop][2];
        union_st.season_data.u8StartMin = season_StartTime[loop][3];
        union_st.season_data.u8StartSec = season_StartTime[loop][4];
        write_page_eeprom(STORAGE_EEPROM_DLMS_ACTIVE_SEASON_BLK_ADDR + loop*sizeof(st_tod_season_t),(uint8_t*)&union_st.season_data, sizeof(st_tod_season_t));
        write_page_eeprom(STORAGE_EEPROM_DLMS_PASSIVE_SEASON_BLK_ADDR + loop*sizeof(st_tod_season_t),(uint8_t*)&union_st.season_data, sizeof(st_tod_season_t));
    }
    
    
    //week
    write_eeprom(STORAGE_EEPROM_DLMS_ACTIVE_WEEK_CNT_ADDR, DEFAULT_WEEK_COUNT);
    write_eeprom(STORAGE_EEPROM_DLMS_PASSIVE_WEEK_CNT_ADDR, DEFAULT_WEEK_COUNT);
    for(loop = 0; loop < DEFAULT_WEEK_COUNT; loop++)
    {
        memcpy(union_st.week_data.u8WeekName, (uint8_t*)week_names[loop], strlen(week_names[loop]) + 1);
        for(inner_loop = 0; inner_loop < 7; inner_loop++)
        {
            union_st.week_data.u8DayID[inner_loop] = week_profiles[loop][inner_loop];
        }
        write_page_eeprom(STORAGE_EEPROM_DLMS_ACTIVE_WEEK_BLK_ADDR + loop*sizeof(st_tod_week_t),(uint8_t*)&union_st.week_data, sizeof(st_tod_week_t));
        write_page_eeprom(STORAGE_EEPROM_DLMS_PASSIVE_WEEK_BLK_ADDR + loop*sizeof(st_tod_week_t),(uint8_t*)&union_st.week_data, sizeof(st_tod_week_t));
    }
    //day
    write_eeprom(STORAGE_EEPROM_DLMS_ACTIVE_DAY_CNT_ADDR, DEFAULT_DAY_ID_TOTAL);
    write_eeprom(STORAGE_EEPROM_DLMS_PASSIVE_DAY_CNT_ADDR, DEFAULT_DAY_ID_TOTAL);
    for(loop = 0; loop < DEFAULT_DAY_ID_TOTAL; loop++)
    {
        union_st.tod_day.u8DayID = const_day_profile[loop].u8DayID;
        union_st.tod_day.u8ActiveZones = const_day_profile[loop].u8ActiveZones;
        
        //REVIEW: Rakesh, Active zone should be <= MAX_TOD_ZONES.
        
        for(inner_loop = 0; inner_loop < const_day_profile[loop].u8ActiveZones; inner_loop++)
        {
            union_st.tod_day.stTimeZones[inner_loop].u8ID = const_day_profile[loop].stTimeZones[inner_loop].u8ID;
            if(union_st.tod_day.stTimeZones[inner_loop].u8ID > MAX_TOD_ZONES)
            {
              union_st.tod_day.stTimeZones[inner_loop].u8ID = MAX_TOD_ZONES;
            }
            union_st.tod_day.stTimeZones[inner_loop].u8StartHr = const_day_profile[loop].stTimeZones[inner_loop].u8StartHr;
            union_st.tod_day.stTimeZones[inner_loop].u8StartMin = const_day_profile[loop].stTimeZones[inner_loop].u8StartMin;
        }
        write_page_eeprom(STORAGE_EEPROM_DLMS_ACTIVE_DAY_BLK_ADDR + loop*sizeof(st_tod_day_t),(uint8_t*)&union_st.tod_day, sizeof(st_tod_day_t));
        write_page_eeprom(STORAGE_EEPROM_DLMS_PASSIVE_DAY_BLK_ADDR + loop*sizeof(st_tod_day_t),(uint8_t*)&union_st.tod_day, sizeof(st_tod_day_t));
    }
}

uint8_t activity_calendar_polling_process(uint32_t u32Epoch)
{
  uint8_t u8GCTIndex;
  st_tod_passive_to_active_t* stTodPasToAct;
  uint8_t u8TempReadBuf[64];
  uint8_t zone_id;

  //Passive to active check
  read_page_eeprom(STORAGE_EEPROM_DLMS_PASSIVE_TO_ACTIVE_ADDR, u8TempReadBuf, sizeof(st_tod_passive_to_active_t));
  stTodPasToAct = (st_tod_passive_to_active_t*)u8TempReadBuf;
  if ((0 != stTodPasToAct->u8ActPending) && (u32Epoch >= stTodPasToAct->u32ActTime))
  {
    activate_passive_calendar(u8TempReadBuf);
    //clear flag and update passive
    read_page_eeprom(STORAGE_EEPROM_DLMS_PASSIVE_TO_ACTIVE_ADDR, u8TempReadBuf, sizeof(st_tod_passive_to_active_t));
    stTodPasToAct = (st_tod_passive_to_active_t*)u8TempReadBuf;
    stTodPasToAct->u8ActPending = 0;
    write_page_eeprom(STORAGE_EEPROM_DLMS_PASSIVE_TO_ACTIVE_ADDR, u8TempReadBuf, sizeof(st_tod_passive_to_active_t));
  }
  
  //Get Current Season
  u8GCTIndex = get_season(u32Epoch, &u8TempReadBuf[0]);

  //Get WeekID from it
  u8GCTIndex = getWeekID(u32Epoch, u8GCTIndex, &u8TempReadBuf[0]);

  //Get Active day ID
  u8GCTIndex = getDayID(u32Epoch, u8GCTIndex, &u8TempReadBuf[0]);

  //Get Active zone ID
  u8GCTIndex = getZoneID(u32Epoch, u8GCTIndex, &u8TempReadBuf[0]);
  
  //read current active zone ID
  zone_id = read_eeprom(STORAGE_EEPROM_DLMS_RUNNING_ZONE_ID_ADDR);
  if(u8GCTIndex != zone_id)
  {
      if((AC_time_zone_change_callback != NULL) && (zone_id != 0))
      {
          AC_time_zone_change_callback(zone_id);
      }
      zone_id = u8GCTIndex;
      write_eeprom(STORAGE_EEPROM_DLMS_RUNNING_ZONE_ID_ADDR, zone_id);
  }
  return zone_id;
}

void activate_passive_calendar(uint8_t * u8TempReadBuf)
{
    st_tod_passive_to_active_t* stTodPasToAct;
    uint8_t u8GCTIndex;
    uint8_t u8GWITemp;
    
    if(pas_to_act_event_callback != 0)
    {
      pas_to_act_event_callback();
    }
    //Copy Name
    read_page_eeprom(STORAGE_EEPROM_DLMS_PASSIVE_TO_ACTIVE_ADDR, u8TempReadBuf, sizeof(st_tod_passive_to_active_t));
    stTodPasToAct = (st_tod_passive_to_active_t*)u8TempReadBuf;
    write_page_eeprom(STORAGE_EEPROM_DLMS_CALENDAR_NAME_ACTIVE_ADDR, (uint8_t*)stTodPasToAct->u8CalendarName, STORAGE_EEPROM_DLMS_CALENDAR_NAME_ACTIVE_SIZE);
    
    //Copy Seasons
    u8GCTIndex = read_eeprom(STORAGE_EEPROM_DLMS_PASSIVE_SEASON_CNT_ADDR);//num of season
    write_eeprom(STORAGE_EEPROM_DLMS_ACTIVE_SEASON_CNT_ADDR, u8GCTIndex);
    for (u8GCTIndex = 0; u8GCTIndex < MAX_SEASON_PROFILES; u8GCTIndex++)
    {
      read_page_eeprom(STORAGE_EEPROM_DLMS_PASSIVE_SEASON_BLK_ADDR + (u8GCTIndex * sizeof(st_tod_season_t)), u8TempReadBuf, sizeof(st_tod_season_t));
      write_page_eeprom(STORAGE_EEPROM_DLMS_ACTIVE_SEASON_BLK_ADDR + (u8GCTIndex * sizeof(st_tod_season_t)), u8TempReadBuf, sizeof(st_tod_season_t));
    }
    
    //copy weeks
    u8GWITemp = read_eeprom(STORAGE_EEPROM_DLMS_PASSIVE_WEEK_CNT_ADDR);//num of week
    write_eeprom(STORAGE_EEPROM_DLMS_ACTIVE_WEEK_CNT_ADDR, u8GWITemp);
    for (u8GCTIndex = 0; u8GCTIndex < MAX_WEEK_PROFILES; u8GCTIndex++)
    {
      read_page_eeprom(STORAGE_EEPROM_DLMS_PASSIVE_WEEK_BLK_ADDR + (u8GCTIndex * sizeof(st_tod_week_t)), u8TempReadBuf, sizeof(st_tod_week_t));
      write_page_eeprom(STORAGE_EEPROM_DLMS_ACTIVE_WEEK_BLK_ADDR + (u8GCTIndex * sizeof(st_tod_week_t)), u8TempReadBuf, sizeof(st_tod_week_t));
    }
    
    //copy days
    u8GWITemp = read_eeprom(STORAGE_EEPROM_DLMS_PASSIVE_DAY_CNT_ADDR);//num of day
    write_eeprom(STORAGE_EEPROM_DLMS_ACTIVE_DAY_CNT_ADDR, u8GWITemp);
    for (u8GCTIndex = 0; u8GCTIndex < MAX_DAY_PROFILES; u8GCTIndex++)
    {
      read_page_eeprom(STORAGE_EEPROM_DLMS_PASSIVE_DAY_BLK_ADDR + (u8GCTIndex * sizeof(st_tod_day_t)), u8TempReadBuf, sizeof(st_tod_day_t));
      write_page_eeprom(STORAGE_EEPROM_DLMS_ACTIVE_DAY_BLK_ADDR + (u8GCTIndex * sizeof(st_tod_day_t)), u8TempReadBuf, sizeof(st_tod_day_t));
    }
}

uint8_t get_season(uint32_t u32Epoch, uint8_t* u8LocBuf)
{
  uint8_t u8GWITemp;
  uint8_t u8GWIIndex;
  st_tod_season_t* stTodSeason;
  st_time_sturct stActionTime;

  u8GWITemp = read_eeprom(STORAGE_EEPROM_DLMS_ACTIVE_SEASON_CNT_ADDR);//num of season
  for (u8GWIIndex = 0; u8GWIIndex < u8GWITemp; u8GWIIndex++)
  {
    read_page_eeprom(STORAGE_EEPROM_DLMS_ACTIVE_SEASON_BLK_ADDR + (u8GWIIndex * sizeof(st_tod_season_t)), u8LocBuf, sizeof(st_tod_season_t));
    stTodSeason = (st_tod_season_t*)u8LocBuf;

    stActionTime.day = stTodSeason->u8StartDt;
    stActionTime.month = stTodSeason->u8StartMon;
    stActionTime.year = 0xFF;
    stActionTime.hour = stTodSeason->u8StartHr;
    stActionTime.minute = stTodSeason->u8StartMin;
    stActionTime.second = stTodSeason->u8StartSec;
    if (0 == check_greater_time(u32Epoch, &stActionTime))
    {
      break;
    }
  }

  if (0 == u8GWIIndex)
  {
    u8GWIIndex = u8GWITemp - 1;
  }
  else
  {
    u8GWIIndex--;
  }
  return u8GWIIndex;
}

uint8_t getWeekID(uint32_t u32Epoch, uint8_t u8SeasonID, uint8_t* u8LocBuf)
{
  st_tod_season_t* stTodSeason;
  uint8_t num_of_week, week;
  st_tod_week_t week_data;
  
  stTodSeason = (st_tod_season_t*)u8LocBuf;
  
  num_of_week = read_eeprom(STORAGE_EEPROM_DLMS_ACTIVE_WEEK_CNT_ADDR);// num of week
  read_page_eeprom(STORAGE_EEPROM_DLMS_ACTIVE_SEASON_BLK_ADDR + (u8SeasonID * sizeof(st_tod_season_t)), (uint8_t *)stTodSeason, sizeof(st_tod_season_t));
  
  for(week = 0; week < num_of_week; week++)
  {
    read_page_eeprom(STORAGE_EEPROM_DLMS_ACTIVE_WEEK_BLK_ADDR + (sizeof(st_tod_week_t)*week),(uint8_t*)&week_data, sizeof(st_tod_week_t));
    if(memcmp((const char*)week_data.u8WeekName, (const char*)stTodSeason->u8WeekName, strlen((const char*)stTodSeason->u8WeekName)) == 0)
    {
        break;
    }
  }
  return week;
}

uint8_t getDayID(uint32_t u32Epoch, uint8_t u8WeekID, uint8_t* u8LocBuf)
{
  uint8_t u8GDIIndex;
  st_tod_week_t* stTodWeek = NULL;
  stTime_struct stTimeStruct;

  read_page_eeprom(STORAGE_EEPROM_DLMS_ACTIVE_WEEK_BLK_ADDR + (u8WeekID * sizeof(st_tod_week_t)), u8LocBuf, sizeof(st_tod_week_t));
  stTodWeek = (st_tod_week_t*)u8LocBuf;
  UnixToDatteTime(u32Epoch, &stTimeStruct);

  u8GDIIndex = stTimeStruct.weekday - 1;
  return stTodWeek->u8DayID[u8GDIIndex];
}

uint8_t getZoneID(uint32_t u32Epoch, uint8_t u8DayID, uint8_t* u8LocBuf)
{
  uint8_t u8ZIIndex;
  st_tod_day_t* stTodDay = NULL;
  st_time_sturct stActionTime;
  st_tod_zone_t* tod;

  stTodDay = (st_tod_day_t*)u8LocBuf;
  //todo: avl count
  for (u8ZIIndex = 0; u8ZIIndex < MAX_DAY_PROFILES; u8ZIIndex++)
  {
    read_page_eeprom(STORAGE_EEPROM_DLMS_ACTIVE_DAY_BLK_ADDR + (u8ZIIndex * sizeof(st_tod_day_t)), u8LocBuf, sizeof(st_tod_day_t));
    if (stTodDay->u8DayID == u8DayID)
    {
      break;
    }
  }
  if (u8ZIIndex < MAX_DAY_PROFILES)
  {
      for (u8ZIIndex = 0; u8ZIIndex < stTodDay->u8ActiveZones; u8ZIIndex++)
      {
        stActionTime.day = 0xFF;
        stActionTime.month = 0xFF;
        stActionTime.year = 0xFF;
        stActionTime.hour = stTodDay->stTimeZones[u8ZIIndex].u8StartHr;
        stActionTime.minute = stTodDay->stTimeZones[u8ZIIndex].u8StartMin;
        stActionTime.second = 0xFF;
        if (0 == check_greater_time(u32Epoch, &stActionTime))
        {
          break;
        }
      }

      if (0 == u8ZIIndex)
      {
        u8ZIIndex = stTodDay->u8ActiveZones - 1;
      }
      else
      {
        u8ZIIndex--;
      }
      tod = &stTodDay->stTimeZones[u8ZIIndex];
      if(tod->u8ID > 8)
      {
        tod->u8ID = 1;
      }
      return tod->u8ID;
  }
  else
  {
    return 1;
  }
}
