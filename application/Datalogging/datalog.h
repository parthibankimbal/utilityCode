#ifndef _DATALOG_H_
#define _DATALOG_H_

#undef RETURN_ERROR

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include "em_energies.h"
#include "rtc_user.h"
#include "em_type.h"
#include "em_measurement.h"
#include "em_operation.h"
#include "inst_read.h"
#include "tampers.h"
#include "factory_settings.h"
/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
typedef void(*pre_datalog_user_callback)(void);

/* Configure */
#define member_size(type, member) sizeof(((type *)0)->member)
#define DEFAULT_BILLING_DATE                            1
#define DEFAULT_BILLING_HOUR                            0
#define DEFAULT_BILLING_MINUTE                          0
#define DEFAULT_BILLING_SECOND                          0
#ifdef UTILITY_JNK
	#define DEFAULT_BLOCK_LOAD_CAPTURE_PERIOD               3600
#elif ((UTILITY_INTELLI) || (UTILITY_APRAAVA) || (UTILITY_PURBANCHAL))
	#define DEFAULT_BLOCK_LOAD_CAPTURE_PERIOD               1800
#endif
#define DEFAULT_MD_INTEGRATION_PERIOD                   1800

#define BLOCKLOAD_MAX_ENTRIES                           (70*48)
#define DAILYLOAD_MAX_ENTRIES                           (70)
#define BILLING_MAX_ENTRIES			        (12)

#define EVENT_MAX_ENTRIES_VOLTAGE			(50)
#define EVENT_MAX_ENTRIES_CURRENT			(50)
#define EVENT_MAX_ENTRIES_POWER				(50)
#define EVENT_MAX_ENTRIES_TRANS				(50)
#define EVENT_MAX_ENTRIES_OTHER				(50)
#define EVENT_MAX_ENTRIES_NON_ROLLER                    (2)
#define EVENT_MAX_ENTRIES_CONTROL			(50)

#define EPR_OK                               0 /* Normal end */

typedef void(*address_return_type)(uint32_t);

typedef enum tag_dlms_app_return_error_code_t
{
  RLT_SUCCESS = 0,
  RLT_ERR,
  RLT_ERR_STORAGE_ERROR,
  RLT_ERR_ACCESS_FAIL,
  RLT_ERR_READ_FAIL,
  RLT_ERR_WRITE_FAIL,
  RLT_ERR_NULL,
  RLT_ERR_OBJECT_NOT_FOUND,
  RLT_ERR_EMPTY,
  RLT_ERR_INVALID_PARAMETER,
  RLT_ERR_INVALID_FUNC,
  RLT_ERR_INVALID_DATA,
  RLT_ERR_NOT_ENOUGH_MEM,
  RLT_ERR_CRC,
  RLT_ERR_OPERATION_IN_PROGRESS,
  RLT_ERR_OPERATION_CORRUPTED,
  RLT_ERR_DATA_CORRUPTED,
  RLT_ERR_INIT,
  RLT_ERR_NOT_FOUND,
  RLT_ERR_STORAGE_NOT_FORMATTED,
  RLT_ERR_STORAGE_FAULT,
  RLT_ERR_STORAGE_WEAK_PASSWORD,
  RLT_OTHER = 255,
} dlms_app_return_error_code_t;

typedef enum Tag_dlms_profile_type_t
{
  PROFILE_TYPE_VOLTAGE_RELATED,
  PROFILE_TYPE_CURRENT_RELATED,
  PROFILE_TYPE_POWER_RELATED,
  PROFILE_TYPE_TRANSACTION_RELATED,
  PROFILE_TYPE_OTHERS,
  PROFILE_TYPE_NON_ROLLOVER_EVENTS,
  PROFILE_TYPE_CONTROL_EVENTS,

  PROFILE_TYPE_BILLING,
  PROFILE_TYPE_DAILYLOAD,
  PROFILE_TYPE_BLOCKLOAD,
} tag_dlms_profile_type_t;
#ifdef WIN32
#pragma pack(push,1)
#else
#ifdef __CCRL__
#pragma pack
#else
#error please define proper packing.
#endif
#endif

typedef enum
{
    LP_Active_Imp,
    LP_Active_Exp,
    LP_Apparent_Imp,
    LP_Apparent_Exp,
    LP_Reactive_Ind_Imp,
    LP_Reactive_Ind_Exp,
    LP_Reactive_Cap_Imp,
    LP_Reactive_Cap_Exp,
}ENERGY_LP_TYPE;

typedef struct
{
  uint16_t from;
  uint16_t num;
}range_entries;

typedef struct tag_DLMS_STORAGE
{
  uint32_t address;
  uint32_t length;
} DLMS_STORAGE;

/* buffer info for data logging */
typedef struct tag_buffer_info_t
{
  uint16_t write_pos;
  uint16_t read_pos;
  uint16_t max_entries;
  uint16_t entries_in_used;

  uint8_t  is_non_roller;
  uint8_t  reserved;
} buffer_info_t;

/* Event ID log */
typedef struct TAG_ONE_EVENT_ID_LOG
{
  uint32_t u32_epoch;
  uint16_t u16_event_code;
} ONE_EVENT_ID_LOG;

/* Event ID log */
typedef struct TAG_ONE_EVENT_ENERGY_DATA_LOG
{
  uint32_t u32_epoch;
  uint16_t u16_Tamper_ID;
  uint32_t u32_current_phase;
  uint32_t u32_current_neutral;
  uint16_t u16_volt;
  uint8_t  u8_pf;
  uint64_t u64_energy_val[4];
  uint64_t u64_kwh_expo;
  uint32_t u32_tot_kw;
  uint32_t u32_tamper_count;
} ONE_EVENT_ENERGY_DATA_LOG;

/* monthly bill log */
typedef struct tagOneMonthEnergyDataLog
{
  uint32_t u32_epoch;
  uint8_t  u8_Sys_Power_Factor;
  uint64_t u64_Cumm_Energy_KWh;
  uint64_t u64_Cumm_Energy_KWh_TZ[8];
  uint64_t u64_Cumm_Energy_KVAh;
  uint64_t u64_Cumm_Energy_KVAh_TZ[8];
  uint32_t u32_MD_KW;
  uint32_t u32_MD_KW_Epoch;
  uint32_t u32_MD_KW_TZ[8];
  uint32_t u32_MD_KW_TZ_Epoch[8];
  uint32_t u32_MD_KVA;
  uint32_t u32_MD_KVA_Epoch;
  uint32_t u32_MD_KVA_TZ[8];
  uint32_t u32_MD_KVA_TZ_Epoch[8];
  uint32_t u32_Pwr_on_duration;
  uint64_t u64_Cumm_Energy_KWh_expo;
  uint64_t u64_Cumm_Energy_KVAh_expo;
  uint64_t u64_Cumm_Energy_KVarh_Q1;
  uint64_t u64_Cumm_Energy_KVarh_Q2;
  uint64_t u64_Cumm_Energy_KVarh_Q3;
  uint64_t u64_Cumm_Energy_KVarh_Q4;
  uint32_t u32_tamper_count;
} ONE_MONTH_ENERGY_DATA_LOG;

/* daily load log */
typedef struct tagOneDailyEnergyDataLog
{
  uint32_t u32_epoch;
  uint64_t u64_energy_val[4];
  uint32_t u32_mdKw;
  uint32_t u32_mdKva;
} ONE_DAILY_ENERGY_DATA_LOG;

/* block load log */
typedef struct tagOneBlockEnergyDataLog
{
  uint32_t u32_epoch;
  uint16_t u16_current_phase;
  uint16_t u16_current_neutral;
  uint16_t u16_volt;
  //uint8_t u8_pf;
#ifdef SINGLE_PHASE_METER
  uint32_t u32_block_energy_val[4];
#else
  uint32_t u32_block_energy_val[8];
#endif
} ONE_BLOCK_ENERGY_DATA_LOG;

/* backup block load log */
typedef struct tagOneBlockBackupEnergyDataLog
{
  uint32_t u32_epoch;
#ifdef SINGLE_PHASE_METER
  uint32_t u64_block_energy_val[4];
#else
  uint32_t u64_block_energy_val[8];
#endif
} ONE_BLOCK_BACKUP_ENERGY_DATA_LOG;

/* block load log */
typedef struct tagBlockEnergyAvgDataLog
{
  float    fl_current_phase;
  float    fl_current_neutral;
  uint16_t u16_current_counter_phase;
  uint16_t u16_current_counter_neutral;
  float    fl_volt;
  uint16_t u16_volt_counter;
  float    fl_pf;
  uint16_t u16_pf_counter;
} BLOCK_ENERGY_AVG_DATA_LOG;

typedef struct tagOneMdLog
{
  uint32_t u32_epoch;
  uint64_t u64_energy_kwh;
  uint64_t u64_energy_kvah;
  uint64_t u64_energy_kwh_export;
  uint64_t u64_energy_kvah_export;
  uint32_t u32_MD_KW_Export_epoch;
  uint32_t u32_MD_KW_Export;
  uint32_t u32_MD_KVA_Export_epoch;
  uint32_t u32_MD_KVA_Export;
} ONE_MD_LOG;

typedef struct tagBillingDateTimeAction
{
  uint8_t u8_day;
  uint8_t u8_hour;
  uint8_t u8_minute;
  uint8_t u8_second;
} BILLING_DATE_TIME_ACTION;

typedef union
{
  ONE_BLOCK_ENERGY_DATA_LOG block_load;
  ONE_DAILY_ENERGY_DATA_LOG daily_load;
  ONE_MONTH_ENERGY_DATA_LOG billing;
  ONE_EVENT_ENERGY_DATA_LOG event;
}u_st_profiles;
extern u_st_profiles u_s_profile;

#ifdef WIN32
#pragma pack(pop)
#else
#ifdef __CCRL__
#pragma unpack
#else
#error please define proper unpacking.
#endif
#endif

typedef enum
{
 e_entry_none,
 e_latest_entry,
 e_last_entry,
}e_entry_type;

extern tag_dlms_profile_type_t scan_event_type;
extern u_st_profiles u_s_profile;
extern address_return_type address_return;
extern BLOCK_ENERGY_AVG_DATA_LOG avg_blk_load_values;
extern pre_datalog_user_callback log_energies_callback;
extern ONE_MONTH_ENERGY_DATA_LOG	g_Last_bill_log;
#ifdef UTILITY_JNK
extern uint64_t                         g_bill_kwh_h2;
extern uint64_t                         g_bill_kwh_h3;
extern uint32_t                         g_bill_mdkw_h2;
extern uint32_t                         g_bill_mdkw_h3;
extern uint32_t                         g_bill_mdkw_epoch_h2;
extern uint32_t                         g_bill_mdkw_epoch_h3;
#endif

void R_DLMS_DataLog_SetEntry(tag_dlms_profile_type_t datalogProfile_id, uint8_t* pbuff);
void check_and_log_data(st_RTC_time* g_RTC_time);
void log_load_profile(st_RTC_time* g_RTC_time);
void log_daily_load_profile(st_RTC_time* g_RTC_time);
void log_billing_profile(st_RTC_time* g_RTC_time, uint8_t forced);
void generate_bill_on_demand(st_RTC_time* RTC_time);
void set_default_datalog_on_eeprom_erase(st_RTC_time* RTC_time);
void set_data_logging_times(st_RTC_time* RTC_time, uint8_t from_mid_night);
void set_block_load_capture_period(uint32_t RTC_epoch, uint16_t value);
uint16_t get_block_load_capture_period(void);
void set_md_integration_time(uint32_t RTC_epoch, uint16_t value);
uint16_t get_md_integration_time(void);
void add_cumulative_bill_count(void);
uint16_t get_cumulative_bill_count(void);
uint8_t get_avaiable_bill_count(void);
uint16_t get_latest_event_id(tag_dlms_profile_type_t datalogProfile_id);
void get_load_profile(ONE_BLOCK_ENERGY_DATA_LOG* profile_log, 
                    uint16_t index, 
                    e_entry_type entry_type);
void get_daily_load_profile(ONE_DAILY_ENERGY_DATA_LOG* profile_log, uint16_t index);
void get_running_billing_data(ONE_MONTH_ENERGY_DATA_LOG* profile_log);
void get_billing_profile_data(ONE_MONTH_ENERGY_DATA_LOG* profile_log, uint16_t index, uint8_t latest_entry);
uint16_t get_profile_entry_in_use(tag_dlms_profile_type_t datalogProfile_id);
void log_md_data(st_RTC_time* RTC_time, uint8_t zone);
void store_energies_in_zones(uint8_t zone);
uint32_t get_mdkw_value(uint8_t zone);
uint32_t get_mdkva_value(uint8_t zone);
uint32_t get_mdkw_time(uint8_t zone);
uint32_t get_mdkva_time(uint8_t zone);
uint64_t get_kwh_time_zone(uint8_t zone);
uint64_t get_kvah_time_zone(uint8_t zone);
void get_tamper_profile(ONE_EVENT_ENERGY_DATA_LOG *profile_log, tag_dlms_profile_type_t datalogProfile_id, uint32_t index);
uint32_t get_next_bill_date(st_RTC_time* RTC_time);
void set_bill_date(st_RTC_time* RTC_time, uint8_t *data);
void get_bill_date(uint8_t *data);
uint32_t get_running_bill_power_on_time(void);
void average_lp_values(void);
void store_average_lp_values(void);
void read_average_lp_values(void);
range_entries find_entries_by_range(uint32_t epoch_from, uint32_t epoch_to, tag_dlms_profile_type_t datalogProfile_id);
uint32_t get_mdkw_expo_value(void);
uint32_t get_mdkva_expo_value(void);
uint32_t get_mdkw_expo_time(void);
uint32_t get_mdkva_expo_time(void);
void validte_capture_period(void);
void load_last_bill(void);
void clear_non_roll_over(void);
void init_DataLogging(void);
#endif //_DATALOG_H_
