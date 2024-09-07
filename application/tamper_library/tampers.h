#ifndef TAMPERS_H
#define TAMPERS_H
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "em_type.h"
#include "em_measurement.h"
#include "em_operation.h"
#include "em_energies.h"
#include "inst_read.h"
#include "eeprom.h"
#include "rtc_user.h"
#include "datalog.h"
#include "key.h"
#include "r_lcd_display.h"
#include "Load_Control.h"
#include "factory_settings.h"
#include "em_display.h"
#include "math.h"
#include "wrp_em_adc.h"

#define _BV(x)  (1<<x)


typedef struct
{
    uint32_t u32NextUpdateTime;
    int16_t i16CorrectionTime;
    uint8_t u8UpdateFlag;
}ST_RTC_TIME_SYNC;

#define TOTAL_EVENT_TYPES 23
//TODO: Rakesh factory settings sept 2022 later
#ifdef UTILITY_JNK
  #define OVER_VOLT_PERCENT                           115.0
  #define LOW_VOLT_PERCENT                            70.0
  #define NEUTRAL_DISTURBANCE_CHK_CURRENT             1.0
  #define LOW_PF_VAL                                  30
#elif ((UTILITY_INTELLI) || (UTILITY_APRAAVA) || (UTILITY_PURBANCHAL))
  #define OVER_VOLT_PERCENT                           108.33
  #define LOW_VOLT_PERCENT                            83.33
  #define NEUTRAL_DISTURBANCE_CHK_CURRENT             0.6
  #define LOW_PF_VAL                                  40
#endif // UTILITY_INTELLI


//Tampers
#define DEFAULT_MINIMUM_OVER_THRESOLD_DURATION          300
#define DEFAULT_MINIMUM_UNDER_THRESOLD_DURATION         300
#define MINIMUM_MINIMUM_OVER_THRESOLD_DURATION          10
#define MAXIMUM_MINIMUM_OVER_THRESOLD_DURATION          900
#define MINIMUM_MINIMUM_UNDER_THRESOLD_DURATION         10
#define MAXIMUM_MINIMUM_UNDER_THRESOLD_DURATION         1800

#define MINIMUM_OVER_CURRENT_VALUE                      ((FACTORY_METER_I_REF*(double)0.2))
#ifdef UTILITY_JNK
  #define MAXIMUM_OVER_CURRENT_VALUE                      ((FACTORY_METER_I_MAX*(double)1.5833334))
#elif ((UTILITY_INTELLI) || (UTILITY_APRAAVA) || (UTILITY_PURBANCHAL))
  #define MAXIMUM_OVER_CURRENT_VALUE                      ((FACTORY_METER_I_MAX*(double)1.66666667))
#endif

#define MINIMUM_OVER_LOAD_VALUE                         ((FACTORY_METER_V_REF*FACTORY_METER_I_MAX*(double)0.03333333))
#ifdef UTILITY_JNK
  #define MAXIMUM_OVER_LOAD_VALUE                         ((FACTORY_METER_V_REF*FACTORY_METER_I_MAX*(double)1.59722223))
#elif ((UTILITY_INTELLI) || (UTILITY_APRAAVA) || (UTILITY_PURBANCHAL))
  #define MAXIMUM_OVER_LOAD_VALUE                         ((FACTORY_METER_V_REF*FACTORY_METER_I_MAX*(double)1.66666667))
#endif

#define MINIMUM_BYPASS_VALUE                            ((FACTORY_METER_I_REF*(double)0.25))
#define MAXIMUM_BYPASS_VALUE                            ((FACTORY_METER_I_REF*(double)0.2))

#ifdef UTILITY_JNK
  #define DEFAULT_OVER_CURRENT_VAL                        ((FACTORY_METER_I_MAX*(double)1.0583334))
#elif ((UTILITY_INTELLI) || (UTILITY_APRAAVA) || (UTILITY_PURBANCHAL))
  #define DEFAULT_OVER_CURRENT_VAL                        ((FACTORY_METER_I_MAX*(double)1.05))
#endif

#define DEFAULT_BYPASS_CURRENT_VAL                      ((FACTORY_METER_I_REF*(double)0.1))

#ifdef UTILITY_JNK
  #define DEFAULT_OVER_LOAD_VAL                           ((FACTORY_METER_V_REF*FACTORY_METER_I_MAX*(double)1.006944444444444444444444444444)) // 105% of Imax*Vbasic 
#elif ((UTILITY_INTELLI) || (UTILITY_APRAAVA) || (UTILITY_PURBANCHAL))
  #define DEFAULT_OVER_LOAD_VAL                           ((FACTORY_METER_V_REF*FACTORY_METER_I_MAX*(double)1.05)) // 105% of Imax*Vbasic 
#endif

#define DEFAULT_ESWF_BITS                               {0x1A, 0x18, 0, 0, 0, 0, 0x10, 0, 0, 0, 0x7F, 0, 0, 0, 0, 0}
#define DEFAULT_TAMPER_OCCUR_TIME                       300   //time in seconds
#define DEFAULT_TAMPER_RESTORE_TIME                     300   //time in seconds
#define MIN_FREQUENCY                                   4700  //f*100
#define MAX_FREQUENCY                                   5300  //f*100
#define DEFAULT_COVER_OPEN_COUNTER                      4     //time in seconds
#define DEFAULT_COVER_OPEN_DELAY_TIME                   1800  //time in seconds
#define DEFAULT_COVER_OPEN_EPOCH                        get_epoch() + DEFAULT_COVER_OPEN_DELAY_TIME
#define I_BASIC                                         FACTORY_METER_I_REF //milli amp
#define V_BASIC                                         FACTORY_METER_V_REF  //v*100
#define UNITY_PF                                        1.0   //pf*100
#define MAGNET_TAMPER_RESTORATION_TIME                  120    //time in seconds
#define MAGNET_TAMPER_OCCURANCE_TIME                    120    //time in seconds
#define ND_OCCUR_RESTORE_TIME                           120

#ifdef UTILITY_JNK
  #define SINGLE_WIRE_OCCUR_RESTORE_TIME                10
#elif ((UTILITY_INTELLI) || (UTILITY_APRAAVA) || (UTILITY_PURBANCHAL))
  #define SINGLE_WIRE_OCCUR_RESTORE_TIME                30
#endif
#define RF_TEST_TIME                                    3
#define ND_DETECTION_TIME                               6
#define ND_RESTORATION_TIME                             6

#define GET_CUMULATIVE_ENERGY(x)    get_energy(x)
#define GET_U_PH_VOLTAGE()          get_ph_voltage()
#define GET_U_REAL_PH_VOLTAGE()     get_real_ph_voltage()
#define GET_U_PH_CURRENT()          get_signed_ph_current()
#define GET_U_NEU_CURRENT()         get_signed_neu_current()
#define GET_U_ACTIVE_CURRENT()      get_signed_act_current()
#define GET_U_REAL_PH_CURRENT()     get_real_signed_ph_current()
#define GET_U_REAL_NEU_CURRENT()    get_real_signed_neu_current()
#define GET_U_REAL_ACTIVE_CURRENT() get_real_signed_act_current()
#define GET_PF()                    get_signed_pf()
#define GET_REAL_PF()               get_real_signed_pf()
#define SYSTEM_FREQUENCY()          get_freq()
#define TOTAL_REAL_ACTIVE_POWER()   get_real_signed_tot_kw()
#define IS_REVERSE()                is_reverse()
#define EM_SET_MAGNET_TAMPER(x)     INST_SET_MAGNET_TAMPER = x
#define DUMMY_POWER_START(x)        INST_DUMMY_POWER_START = x

#define EPOCH                       g_RTC_time.epoch
#define MAGNET_IN_STAT              (!MAGNET_SENSE)

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
  union
  {
    struct
    {
      uint8_t r_ph_volt_missing : 1;
      uint8_t y_ph_volt_missing : 1;
      uint8_t b_ph_volt_missing : 1;
      uint8_t over_volt : 1;
      uint8_t low_volt : 1;
      uint8_t volt_unbalance : 1;
      uint8_t r_ph_current_reverse : 1;
      uint8_t y_ph_current_reverse : 1;
      uint8_t b_ph_current_reverse : 1;
      uint8_t r_ph_current_missing : 1;
      uint8_t y_ph_current_missing : 1;
      uint8_t b_ph_current_missing : 1;
      uint8_t current_unbalance : 1;
      uint8_t current_bypass : 1;
      uint8_t over_current : 1;
      uint8_t earth_loading : 1;
      uint8_t magnet_detected : 1;
      uint8_t neutral_disturbance : 1;
      uint8_t low_PF : 1;
      uint8_t single_wire : 1;
      uint8_t rf_removed : 1;
      uint8_t over_load : 1;
      uint8_t cover_open : 1;
      uint8_t load_disable : 1;
      uint8_t last_gasp : 1;
      uint8_t first_breath : 1;
      uint8_t billing_counter_increment : 1;
    }bits;
    uint32_t tamper_bits;
  };
}St_tamper_bits;

typedef struct
{
  int16_t counter;
  uint32_t enable_time;
}St_miss_event;

typedef struct
{
  uint16_t id;
  uint32_t time;
}st_latest_event;

typedef struct  
{
  uint16_t event_presistent_time[TOTAL_EVENT_TYPES];
  uint16_t event_restoration_time[TOTAL_EVENT_TYPES];
  uint16_t _event_presistent_time[TOTAL_EVENT_TYPES];
  uint16_t _event_restoration_time[TOTAL_EVENT_TYPES];
  uint32_t Trigger_Byte;
  float32_t over_current_value;
  float32_t over_load_value;
  uint32_t over_bypass_value;
  uint32_t minimum_thresold_over_duration;
  uint32_t minimum_thresold_under_duration;
  uint32_t Cum_Power_On_Time;
  uint32_t Cum_Power_Off_Time;
  St_miss_event St_cover_open;
  St_miss_event St_RF_removal;
  St_tamper_bits tampers_bits; //current status
  St_tamper_bits g_tampers_bits; // occured or restored global status
}St_tamper;

typedef struct
{
    uint8_t byMagSense;
    uint8_t byMagSense2;
    uint8_t byMagStat;
    uint8_t byMagPulsesCnt;
    uint16_t wMagTime;
}ST_MAGNET_STAT;

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
  VOLT_EVENT,
  AMP_EVENT,
  PFAIL_EVENT,
  TRANSACT_EVENT,
  OTHER_EVENT,
  NOROLL_EVENT,
  CTRL_EVENT,
}Event_Type;

typedef enum
{
  VOLTAGE_TAMPER_BIT,
  CURRENT_TAMPER_BIT,
  POWER_TAMPER_BIT,
  TRANSACTION_TAMPER_BIT,
  OTHER_TAMPER_BIT,
  NON_ROLLOVER_TAMPER_BIT,
  CONNECT_DISCONNECT_TAMPER_BIT,
  EVENTS_BIT,
  INSTANT_PUSH_BIT,
  SMO1_BIT,
  SMO2_BIT,
  IHD_MSG_BIT,
  LOAD_PROFILE_BIT,
  DAILY_PROFILE_BIT,
  BILLING_PROFILE_BIT,
}e_tamper_bit_map;

typedef enum
{
  TAMPER_R_PH_VOLTAGE_MISSING_STAT,
  TAMPER_Y_PH_VOLTAGE_MISSING_STAT,
  TAMPER_B_PH_VOLTAGE_MISSING_STAT,

  TAMPER_OVER_VOLTAGE_STAT,
  TAMPER_LOW_VOLTAGE_STAT,
  TAMPER_VOLTAGE_UNBALANCE,
  
  TAMPER_R_PH_CURRENT_REVERSE_STAT,
  TAMPER_Y_PH_CURRENT_REVERSE_STAT,  
  TAMPER_B_PH_CURRENT_REVERSE_STAT,
  
  TAMPER_R_PH_CURRENT_MISSING_STAT,
  TAMPER_Y_PH_CURRENT_MISSING_STAT,
  TAMPER_B_PH_CURRENT_MISSING_STAT,
  
  TAMPER_CURRENT_UNBALANCE_STAT,
  TAMPER_CURRENT_BYPASS_STAT,
  TAMPER_OVER_CURRENT_STAT,
  TAMPER_EARTH_LOADING_STAT,
  
  TAMPER_MAGNET_STAT,
  TAMPER_NEUTRAL_DISTURBANCE_STAT,
  TAMPER_LOW_PF_STAT,
  TAMPER_SINGLE_WIRE_STAT,
  TAMPER_RF_REMOVAL_STAT,
  TAMPER_OVER_LOAD_STAT,
  
  TAMPER_METER_COVER_OPEN_STAT,
  TAMPER_METER_LOAD_CONNECTION_STAT,
  TAMPER_LAST_GASP_STAT,
  TAMPER_FIRST_BREATH_STAT,
  TAMPER_BILLING_COUNT_INCREMENT_STAT,
}tamper_stats;

typedef enum
{
  IS_15959_TAMPER_R_PH_VOLTAGE_MISSING_STAT,
  IS_15959_TAMPER_Y_PH_VOLTAGE_MISSING_STAT,
  IS_15959_TAMPER_B_PH_VOLTAGE_MISSING_STAT,
  IS_15959_TAMPER_OVER_VOLTAGE_STAT,
  IS_15959_TAMPER_LOW_VOLTAGE_STAT,
  IS_15959_TAMPER_VOLTAGE_UNBALANCE,
  IS_15959_TAMPER_R_PH_CURRENT_REVERSE_STAT,
  IS_15959_TAMPER_Y_PH_CURRENT_REVERSE_STAT,
  IS_15959_TAMPER_B_PH_CURRENT_REVERSE_STAT,
  IS_15959_TAMPER_R_PH_CURRENT_MISSING_STAT = 13,
  IS_15959_TAMPER_Y_PH_CURRENT_MISSING_STAT = 14,
  IS_15959_TAMPER_B_PH_CURRENT_MISSING_STAT =15,
  IS_15959_TAMPER_CURRENT_UNBALANCE_STAT = 9,
  IS_15959_TAMPER_CURRENT_BYPASS_STAT = 10,
  IS_15959_TAMPER_OVER_CURRENT_STAT = 11,
  IS_15959_TAMPER_MAGNET_STAT = 81,
  IS_15959_TAMPER_NEUTRAL_DISTURBANCE_STAT = 82,
  IS_15959_TAMPER_LOW_PF_STAT = 12,
  IS_15959_TAMPER_EARTH_LOADING_STAT = 51,
  IS_15959_TAMPER_METER_COVER_OPEN_STAT = 83,
  IS_15959_TAMPER_METER_LOAD_STAT = 84,
  IS_15959_TAMPER_LAST_GASP_STAT = 85,
  IS_15959_TAMPER_FIRST_BREATH_STAT = 86,
  IS_15959_TAMPER_BILLING_COUNT_INCREMENT_STAT = 87
}IS_15959_tamper_stats;
//

extern ST_RTC_TIME_SYNC stRTCTimeSync;
extern const unsigned short event_ids[];
extern St_tamper st_tamper;
extern const unsigned short event_ids[];

float32_t max_current(void);
float32_t min_current(void);
float32_t cal_percentage(float32_t percentage, float32_t value);

void set_default_tampers_params(void);
void init_tampers_params(void);
void get_voltage_missing_tamper(void);
void get_over_voltage_tamper(void);
void get_low_voltage_tamper(void);
void get_voltage_unbalance_tamper(void);
void get_current_reverse_tamper(void);
void get_current_miss_tamper(void);
void get_current_unbalance_tamper(void);
void get_current_bypass_tamper(void);
void get_over_current_tamper(void);
void get_magnet_tamper(void);
void get_neutral_disturbance(void);
void get_low_PF_tamper(void);
//
void call_tamper_func(uint32_t epoch, uint8_t mains_stat);
void store_event_data(Event_Type event_type, unsigned short event_id, uint32_t event_time);
void ESW_function(void);
void get_cover_open_tamper(uint32_t epoch);
void get_RF_removal_tamper(uint32_t epoch);
uint32_t get_tamper_counts(void);
uint32_t get_programming_counts(void);
void clear_cover_open(void);
void update_cumu_power_on_time(uint32_t epoch);
void update_cumu_power_off_time(uint32_t epoch);
uint32_t get_cumu_power_on_time(void);
void set_cumu_power_on_time(uint32_t time);
uint32_t get_cumu_power_off_time(void);
void set_cumu_power_off_time(uint32_t time);
float32_t get_over_current_value(void);
void set_over_current_value(float32_t value);
float32_t get_over_load_value(void);
void set_over_load_value(float32_t value);
uint32_t get_over_bypass_value(void);
void set_over_bypass_value(uint32_t value);
uint32_t get_minimum_over_duration(void);
void set_minimum_over_duration(uint32_t value);
uint32_t get_minimum_under_duration(void);
void set_minimum_under_duration(uint32_t value);
void register_disconnect_event_callback(st_RTC_time* RTC_time);
void register_connect_event_callback(st_RTC_time* RTC_time);
uint8_t is_RF_missing(void);
void set_tamper_occurance_time(uint16_t time);
void set_tamper_restoration_time(uint16_t time);
uint8_t is_cover_open(void);
void reset_cover_open_time(void);
void ac_magnet_test(void);
uint8_t check_res_cap_ND(void);
void restore_over_limit_events(void);
void log_bill_bit(uint8_t set_bit_value);
void reset_cover_open_time(void);
#endif //TAMPERS_H