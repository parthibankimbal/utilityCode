//
// --------------------------------------------------------------------------
//  Gurux Ltd
//
//
//
// Filename:        $HeadURL:  $
//
// Version:         $Revision:  $,
//                  $Date:  $
//                  $Author: $
//
// Copyright (c) Gurux Ltd
//
//---------------------------------------------------------------------------

#ifndef DATE_H
#define DATE_H

#include "bytebuffer.h"
#include "enums.h"

//If OS
#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)
#include <time.h>
#else //If no OS.

#ifndef _TM_DEFINED
struct tm
{
    unsigned char tm_sec;     /* seconds after the minute - [0,59] */
    unsigned char tm_min;     /* minutes after the hour - [0,59] */
    unsigned char tm_hour;    /* hours since midnight - [0,23] */
    unsigned char tm_mday;    /* day of the month - [1,31] */
    unsigned char tm_mon;     /* months since January - [0,11] */
    unsigned short tm_year;    /* years since 1900 */
    unsigned char tm_wday;    /* days since Sunday - [0,6] */
    unsigned short tm_yday;    /* days since January 1 - [0,365] */
    unsigned char tm_isdst;   /* daylight savings time flag */
    unsigned short tm_deviation;
};
#define _TM_DEFINED
#endif

#endif

// DataType enumerates skipped fields from date time.
typedef enum
{
    // Nothing is skipped from date time.
    DATETIME_SKIPS_NONE = 0x0,
    // Year part of date time is skipped.
    DATETIME_SKIPS_YEAR = 0x1,
    // Month part of date time is skipped.
    DATETIME_SKIPS_MONTH = 0x2,
    // Day part is skipped.
    DATETIME_SKIPS_DAY = 0x4,
    // Day of week part of date time is skipped.
    DATETIME_SKIPS_DAYOFWEEK = 0x8,
    // Hours part of date time is skipped.
    DATETIME_SKIPS_HOUR = 0x10,
    // Minute part of date time is skipped.
    DATETIME_SKIPS_MINUTE = 0x20,
    // Seconds part of date time is skipped.
    DATETIME_SKIPS_SECOND = 0x40,
    // Hundreds of seconds part of date time is skipped.
    DATETIME_SKIPS_MS = 0x80,
    //Devitation is skipped on write.
    DATETIME_SKIPS_DEVITATION = 0x100
} DATETIME_SKIPS;

typedef struct
{
    unsigned char skip; //DATETIME_SKIPS
    struct tm value;
    unsigned char daylightSavingsBegin;
    unsigned char daylightSavingsEnd;
    unsigned char status;//DLMS_CLOCK_STATUS
} gxtime;

// Constructor.
void time_init(
    gxtime* time,
    int year,
    int month,
    int day,
    int hour,
    int minute,
    int second,
    int millisecond,
    int devitation);

void time_init2(
    gxtime* time,
    struct tm* value);

void time_clear(
    gxtime* time);

void time_copy(
    gxtime* trg,
    gxtime* src);

//Returns current time.
void time_now(
    gxtime* value);

void time_addDays(
    gxtime* value,
    int days);

void time_clearDate(
    gxtime* value);

void time_clearTime(
    gxtime* value);

unsigned char date_daysInMonth(
    int year,
    short month);

int time_toString(
    gxtime* time,
    gxByteBuffer* arr);

void time_addTime(
    gxtime* time,
    int hours,
    int minutes,
    int seconds);

char time_compare(
    struct tm* value1,
    struct tm* value2);

#endif //DATE_H
