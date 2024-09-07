#ifndef __Load_Control_h__
#define __Load_Control_h__

#include "em_type.h"
#include "inst_read.h"
#include "relay.h"
#include "eeprom.h"
#include "em_display.h"
#include "factory_settings.h"


typedef void(*load_control_connect_disconnect_callback)(void);
extern load_control_connect_disconnect_callback post_relay_disconnect_callback;
extern load_control_connect_disconnect_callback post_relay_connect_callback;

#define DEFAULT_DISCONNECT_TIME_NORMAL                  300
#define DEFAULT_DISCONNECT_TIME_LOCKOUT                 1800
#define DEFAULT_LOAD_CHECK_TIME                         300
#define DEFAULT_RECONNECT_COUNT                         3

#define MINIMUM_RELAY_NORMAL_DISCONNECT_TIME            10
#define MAXIMUM_RELAY_NORMAL_DISCONNECT_TIME            1800
#define MINIMUM_RELAY_LOCKOUT_DISCONNECT_TIME           120
#define MAXIMUM_RELAY_LOCKOUT_DISCONNECT_TIME           3600
#define MINIMUM_RELAY_LOAD_CHECK_TIME                   10
#define MAXIMUM_RELAY_LOAD_CHECK_TIME                   300
#define MINIMUM_RELAY_RECONNECT_COUNT                   0
#define MAXIMUM_RELAY_RECONNECT_COUNT                   5

#define RELAY_WELD_THERSHOLD        FACTORY_DEFAULT_RELAY_WELD_THERSHOLD      /*Amp*/
#define RELAY_WELD_RECORD_TIME      FACTORY_DEFAULT_RELAY_WELD_RECORD_TIME    /*Sec*/
#define RELAY_WELD_RECOVER_TIME     FACTORY_DEFAULT_RELAY_WELD_RECOVER_TIME   /*Sec*/
#define RELAY_WELD_TAG              0xAB

typedef enum 
{
        eDISCONNECTED,
        eCONNECTED,
        eREADY_TO_CONNECT
}EN_CONTROL_STATE;


typedef enum 
{
        eLATCH_OPEN,
        eLATCH_CLOSE
}EN_LATCH_STATE;

typedef enum 
{
        eTRANSITION_DISCONNECT,
        eTRANSITION_CONNECT
}EN_TRANSITION_STATE;

typedef enum 
{
        eMODE0,
        eMODE1,
        eMODE2,
        eMODE3,
        eMODE4,
        eMODE5,
        eMODE6
}EN_CONTROL_MODE;

typedef struct
{
        uint8_t u8ReconnectCount;
        uint16_t u16LoadChkTime;
        uint16_t u16DisconnectTimeNormal;
        uint16_t u16DisconnectTimeLockout;
}ST_LOCAL_CONTROL;

typedef struct
{
        uint8_t u8ReconnectCount;
        uint16_t u16LoadChkTime;
        uint16_t u16WaitTime;
}ST_LOCAL_CONTROL_CHECK;

typedef struct
{
  uint8_t over_load_live : 1;
  uint8_t over_current_live : 1;
  uint8_t over_load_registered : 1;
  uint8_t over_current_registered : 1;
}ST_OVER_LIMIT_TAMPER_STATUS;

extern ST_OVER_LIMIT_TAMPER_STATUS st_over_limit_tamper_status;
extern EN_LATCH_STATE enLatchStatus;
EN_LATCH_STATE load_control_polling_process(uint8_t u8OverloadEvent, uint8_t u8OverCurrentEvent);
void load_control_init(void);
void setLocalControl(void);
void resetLocalControl(void);
EN_LATCH_STATE get_relay_output_state(void);
EN_LATCH_STATE relay_output_state(void);
void set_relay_output_state(EN_LATCH_STATE state);
EN_CONTROL_STATE get_relay_control_state(void);
uint8_t get_relay_control_mode(void);
void set_relay_control_mode(uint8_t mode);
EN_TRANSITION_STATE get_relay_remote_state(void);
void set_relay_remote_state(uint8_t state);
uint16_t get_relay_normal_disconnect_time(void);
void set_relay_normal_disconnect_time(uint16_t time);
uint16_t get_relay_lockout_disconnect_time(void);
void set_relay_lockout_disconnect_time(uint16_t time);
uint16_t get_relay_load_check_time(void);
void set_relay_load_check_time(uint16_t time);
uint16_t get_relay_reconnect_count(void);
void set_relay_reconnect_count(uint8_t count);
void set_default_load_control_values(void);
void switch_weld_tamper_scan(uint32_t epoch);
#endif // !__Load_Control_h__
