#ifndef _R_DLMS_STORAGE_MAP_H
#define _R_DLMS_STORAGE_MAP_H

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
#include "Activity_Calendar.h"
//Same defined in r_dlms_push.h due to compiler issue.
#define PUSH_OBJECT_NUMBER          15
#define PUSH_MAX_WINDOW_SUPPORT		(1)
/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
/* Start user code for macro. Do not edit comment generated here */
                                                /* 0xFE000000 + (Version = v02.01.04) */
#define STORAGE_EEPROM_CURRENT_VERISON			(0xFE020104)
                                                /* 0xFE000000 + (Version = v02.01.03) */
#define STORAGE_DATAFLASH_CURRENT_VERISON		(0xFE020103)

/* End user code. Do not edit comment generated here */
/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/* Start user code for typedef. Do not edit comment generated here */

#define ONE_EVENT_FULL_LOG         ONE_EVENT_ENERGY_DATA_LOG
#define ONE_EVENT_SHORT_LOG        ONE_EVENT_ID_LOG

/* RTC Date Time */
typedef struct tag_STORAGE_RTC_DATE_TIME
{
  /* Total: 8 bytes */
  uint8_t Sec;        /* Second */
  uint8_t Min;        /* Minute */
  uint8_t Hour;       /* Hour */
  uint8_t Day;        /* Day */
  uint8_t Week;       /* Day of week */
  uint8_t Month;      /* Month */
  uint8_t Year;       /* Year (ony 2 ending digit) */

  uint8_t reserved;   /* Reserved Byte (Padding) - NO USE */
} STORAGE_RTC_DATE_TIME;
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
EEPROM MEMORY MAP
***********************************************************************************************************************/

                                                                      /* 15 Bytes */                  /*592 Bytes */


#endif