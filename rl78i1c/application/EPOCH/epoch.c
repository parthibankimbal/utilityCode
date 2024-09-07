#include "epoch.h"

DWORD Time_Convert_TO2TS(stTime_struct * time) {
    if (time)
    {
        const short mth[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
        const short mthb[12] = { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 };
        unsigned long int leaps = 0;
        unsigned short i;
        DWORD timestamp;
        for (i = 1970; i < time->year; i++)
        {
            if (((!(i % 4)) && (i % 100)) || (!(i % 400)))
                leaps++;
        }
        timestamp =
            (((!(time->year % 4)) && (time->year % 100)) || (!(time->year % 400))) ?
            (leaps + (time->year - 1970) * 365 + mthb[time->month - 1] + (time->day - 1)) * 86400 + time->hour * 3600 + time->minute * 60 + time->second :
            (leaps + (time->year - 1970) * 365 + mth[time->month - 1] + (time->day - 1)) * 86400 + time->hour * 3600 + time->minute * 60 + time->second;
        return timestamp;
    }
    else
    {
        return 0;
    }
}

void UnixToDatteTime(DWORD time, stTime_struct *ToTime)
{
    unsigned long i = 0, adj = 0, j;
    const unsigned short  mthl[13] = { 0,31, 60, 91, 121, 152, 182, 213,244, 274, 305, 335,366 };
    const unsigned short mth[13] = { 0, 31, 59, 90, 120, 151, 181, 212,243, 273, 304, 334, 365 };
    ToTime->second = time % 60;
    time /= 60;
    ToTime->minute = time % 60;
    time /= 60;
    ToTime->hour = time % 24;
    time /= 24;
    ToTime->weekday = ((time + 3) % 7) + 1;
    for (i = 1970; i < 2100; i++)
    {
        if (((!(i % 4)) && (i % 100)) || (!(i % 400))) { j = 366; }
        else { j = 365; }
        if ((adj + j) > time) { break; }
        else { adj += j; }
    }
    if (adj == time)
    {
        ToTime->day = 1;
        ToTime->month = 1;
        ToTime->year = i;
    }
    else
    {
        ToTime->year = i;
        time -= adj;
        j = 0;
        for (j = 1; j < 13; j++)
        {
            if (((!(i % 4)) && (i % 100)) || (!(i % 400)))
            {
                if (time < mthl[j]) { time = time - mthl[j - 1]; break; }
            }
            else {
                if (time < mth[j]) { time = time - mth[j - 1]; break; }
            }
        }
        ToTime->month = j;
        ToTime->day = time + 1;
    }
    return;
}

void Epoch_To_DLMS_Time(uint32_t epoch, uint8_t* DLMS_Time)
{
    stTime_struct Time;
    UnixToDatteTime(epoch, &Time);
    DLMS_Time[0] = (Time.year>>8) & 0xFF;
    DLMS_Time[1] = (Time.year) & 0xFF;
    DLMS_Time[2] = Time.month;
    DLMS_Time[3] = Time.day;
    DLMS_Time[4] = 0xFF;
    DLMS_Time[5] = Time.hour;
    DLMS_Time[6] = Time.minute;
    DLMS_Time[7] = Time.second;
    DLMS_Time[8] = 0xFF;
    DLMS_Time[9] = (TIME_DEVIATION>>8) & 0xFF;
    DLMS_Time[10] = (TIME_DEVIATION) & 0xFF;
    DLMS_Time[11] = 0;
    return;
}

DWORD DLMS_Time_To_Epoch(uint8_t* DLMS_Time)
{
    stTime_struct Time;
    Time.year = ((((unsigned short)DLMS_Time[0] << 8) & 0xFF00) | DLMS_Time[1]);
    Time.month = DLMS_Time[2];
    Time.day = DLMS_Time[3];
    Time.hour = DLMS_Time[5];
    Time.minute = DLMS_Time[6];
    Time.second = DLMS_Time[7];
    return Time_Convert_TO2TS(&Time);
}

DWORD Time_To_Epoch(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second)
{
    stTime_struct Time;
    Time.year = year +2000;
    Time.month = month;
    Time.day = day;
    Time.hour = hour;
    Time.minute = minute;
    Time.second = second;
    return Time_Convert_TO2TS(&Time);
}

stTime_struct DLMS_Time_To_StTime(uint8_t* DLMS_Time)
{
    stTime_struct Time;
    DWORD epoch;
    Time.year = ((((unsigned short)DLMS_Time[0] << 8) & 0xFF00) | DLMS_Time[1]);
    Time.month = DLMS_Time[2];
    Time.day = DLMS_Time[3];
    Time.hour = DLMS_Time[5];
    Time.minute = DLMS_Time[6];
    Time.second = DLMS_Time[7];
    epoch = Time_Convert_TO2TS(&Time);
    UnixToDatteTime(epoch, &Time);
    return Time;
}
void convert_to_local_time(stTime_struct *Time, signed short Deviation)
{
    UnixToDatteTime(Time_Convert_TO2TS(Time) - (Deviation), Time);
    return;
};
void convert_to_unix_time(stTime_struct *Time, signed short Deviation)
{
    UnixToDatteTime(Time_Convert_TO2TS(Time) + (Deviation), Time);
    return;
};

void stTime_To_DLMS_Time(stTime_struct stTime, uint8_t * DLMS_Time)
{
    DWORD epoch;
    epoch = Time_Convert_TO2TS(&stTime);
    UnixToDatteTime(epoch, &stTime);
    DLMS_Time[0] = (unsigned char)((stTime.year & 0xFF00) >> 8);
    DLMS_Time[1] = (unsigned char)((stTime.year & 0xFF));
    DLMS_Time[2] = (unsigned char)stTime.month;
    DLMS_Time[3] = (unsigned char)stTime.day;
    DLMS_Time[4] = (unsigned char)0xFF;
    DLMS_Time[5] = (unsigned char)stTime.hour;
    DLMS_Time[6] = (unsigned char)stTime.minute;
    DLMS_Time[7] = (unsigned char)stTime.second;
    DLMS_Time[8] = (unsigned char)0xFF; // milli seconds;
    DLMS_Time[9] = (unsigned char)((TIME_DEVIATION & 0xFF00) >> 8);
    DLMS_Time[10] = (unsigned char)(TIME_DEVIATION & 0xFF);
    return;
}

stTime_struct add_year(stTime_struct *Time, unsigned short years)
{
    stTime_struct Temp_Time = *Time;
    Temp_Time.year += years;
    return Temp_Time;
}

stTime_struct add_months(stTime_struct *Time, unsigned long months)
{
    DWORD epoch;
    stTime_struct Temp_Time = *Time;
    const unsigned short  mthl[13] = { 0,31, 60, 91, 121, 152, 182, 213,244, 274, 305, 335,366 };
    const unsigned short mth[13] = { 0, 31, 59, 90, 120, 151, 181, 212,243, 273, 304, 334, 365 };

    epoch = Time_Convert_TO2TS(&Temp_Time);
    while (months)
    {
        if (((!(Temp_Time.year % 4)) && (Temp_Time.year % 100)) || (!(Temp_Time.year % 400)))
        {
            epoch += (mthl[Temp_Time.month] - mthl[Temp_Time.month - 1]) * 86400;
        }
        else
        {
            epoch += (mth[Temp_Time.month] - mth[Temp_Time.month - 1]) * 86400;
        }
        UnixToDatteTime(epoch, &Temp_Time);
        months--;
    }
    return Temp_Time;
}

stTime_struct add_days(stTime_struct *Time, unsigned long days)
{
    DWORD epoch;
    stTime_struct Temp_Time;
    epoch = Time_Convert_TO2TS(Time) + 86400 * days;
    UnixToDatteTime(epoch, &Temp_Time);
    return Temp_Time;
}

stTime_struct add_hours(stTime_struct *Time, unsigned long hours)
{
    DWORD epoch;
    stTime_struct Temp_Time;
    epoch = Time_Convert_TO2TS(Time) + 3600 * hours;
    UnixToDatteTime(epoch, &Temp_Time);
    return Temp_Time;
}

stTime_struct add_minutes(stTime_struct *Time, unsigned long minutes)
{
    DWORD epoch;
    stTime_struct Temp_Time;
    epoch = Time_Convert_TO2TS(Time) + 60 * minutes;
    UnixToDatteTime(epoch, &Temp_Time);
    return Temp_Time;
}

stTime_struct add_seconds(stTime_struct *Time, long seconds)
{
    DWORD epoch;
    stTime_struct Temp_Time;
    epoch = Time_Convert_TO2TS(Time) + seconds;
    UnixToDatteTime(epoch, &Temp_Time);
    return Temp_Time;
}

unsigned long days_since_epoch(unsigned long epoch_value)
{
  return epoch_value/86400;
}

unsigned long hours_since_epoch(unsigned long epoch_value)
{
  return epoch_value/3600;
}