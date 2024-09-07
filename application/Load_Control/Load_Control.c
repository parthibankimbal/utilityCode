#include <stdint.h>
#include "Load_Control.h"
#include "eeprom_storage.h"

uint8_t relay_weld_tamper_logged;
load_control_connect_disconnect_callback post_relay_disconnect_callback = 0;
load_control_connect_disconnect_callback post_relay_connect_callback = 0;

EN_CONTROL_MODE enControlMode = eMODE6; //Default
EN_CONTROL_STATE enControlState = eCONNECTED, enLastControlState = eCONNECTED; // 0 Disconnected, 1 Connected, 2 Ready to connect, 
EN_LATCH_STATE enLatchStatus = eLATCH_CLOSE, enLatchLastStatus = eLATCH_CLOSE;
EN_TRANSITION_STATE enLocalTransitionState = eTRANSITION_CONNECT, enManualTransitionState = eTRANSITION_DISCONNECT, enRemoteTransitionState = eTRANSITION_CONNECT;

ST_LOCAL_CONTROL stLocalCtrl;
ST_LOCAL_CONTROL_CHECK stLocalCtrlCheck;
ST_OVER_LIMIT_TAMPER_STATUS st_over_limit_tamper_status;

//This function is to be called whenever any of the Overload or OverCurrent event occurs
void setLocalControl(void)
{
        if ((stLocalCtrlCheck.u16LoadChkTime == 0) && (stLocalCtrlCheck.u16WaitTime == 0))                        //If Already no previous ongoing disconnect cycle is there
        {
                stLocalCtrlCheck.u16WaitTime = stLocalCtrl.u16DisconnectTimeNormal;
                stLocalCtrlCheck.u8ReconnectCount = 0;
                enLocalTransitionState = eTRANSITION_DISCONNECT;
        }
        //else do nothing
}

//This function is to be called whenever any of the Overload or OverCurrent event restores or whenever enControlMode is changed
void resetLocalControl(void)
{
        if ((!st_over_limit_tamper_status.over_load_registered && !st_over_limit_tamper_status.over_current_registered))// || (0 != u8ContrloModeChanged))              //Both the events are restored already
        {
                stLocalCtrlCheck.u16WaitTime = 0;
                stLocalCtrlCheck.u16LoadChkTime = 0;
                stLocalCtrlCheck.u8ReconnectCount = 0;
                enLocalTransitionState = eTRANSITION_CONNECT;
        }
}

//This function is to be called periodically (each second)
EN_LATCH_STATE load_control_polling_process(uint8_t u8OverloadEvent, uint8_t u8OverCurrentEvent)
{
        #define SKIT_TIME             5
        static uint8_t skip_counter = 0;
        uint8_t u8OverloadRestoreFlag = 1;
        uint8_t u8OvercurrentRestoreFlag = 1;
        //Local Connect control mechanism to follow here
        if(skip_counter < SKIT_TIME)
        {
          skip_counter++;
        }
        if(skip_counter < SKIT_TIME)
        {
          return enLatchStatus;
        }
        if(enLatchStatus == eLATCH_CLOSE)
        {
          e_LCD_icon_status.latch_stat &= ~(1 << 0);
        }
        else
        {
          e_LCD_icon_status.latch_stat |= (1 << 0);
        }
        if (stLocalCtrlCheck.u16LoadChkTime != 0)
        {
                //check load condition here
                if (u8OverloadEvent)
                {
                        if (st_over_limit_tamper_status.over_load_live)
                        {
                                u8OverloadRestoreFlag = 0;
                        }
                }
                if (u8OverCurrentEvent)
                {
                        if (st_over_limit_tamper_status.over_current_live)
                        {
                                u8OvercurrentRestoreFlag = 0;
                        }
                }

                if (u8OverloadRestoreFlag && u8OvercurrentRestoreFlag)
                {
                        stLocalCtrlCheck.u16LoadChkTime = stLocalCtrl.u16LoadChkTime;
                }
                else
                {
                        stLocalCtrlCheck.u16LoadChkTime--;
                }

                if (stLocalCtrlCheck.u16LoadChkTime == 0)
                {
                        //Reconnect load here
                        stLocalCtrlCheck.u8ReconnectCount++;
                        if (stLocalCtrlCheck.u8ReconnectCount < stLocalCtrl.u8ReconnectCount)
                        {
                                stLocalCtrlCheck.u16WaitTime = stLocalCtrl.u16DisconnectTimeNormal;
                        }
                        else if (stLocalCtrlCheck.u8ReconnectCount == stLocalCtrl.u8ReconnectCount)
                        {
                                stLocalCtrlCheck.u16WaitTime = stLocalCtrl.u16DisconnectTimeLockout;
                                #ifdef FACTORY_DEBUG
                                    stLocalCtrlCheck.u16WaitTime = 300;
                                #endif
                        }
                        else
                        {
                                stLocalCtrlCheck.u8ReconnectCount = 0;
                                stLocalCtrlCheck.u16WaitTime = stLocalCtrl.u16DisconnectTimeNormal;
                        }
                        enLocalTransitionState = eTRANSITION_DISCONNECT;
                }
        }
        if (stLocalCtrlCheck.u16WaitTime != 0)
        {
                stLocalCtrlCheck.u16WaitTime--;
                if (0 == stLocalCtrlCheck.u16WaitTime)
                {
                        stLocalCtrlCheck.u16LoadChkTime = stLocalCtrl.u16LoadChkTime;
                        enLocalTransitionState = eTRANSITION_CONNECT;
                }
        }
        //////////////End Of Local Control////////////////////////////

        //Control states follows here
        switch (enControlMode)
        {
        case eMODE0:
                enLatchStatus =  eLATCH_CLOSE;
                enControlState = eCONNECTED;
                break;
        case eMODE1:
                switch (enControlState)
                {
                case eDISCONNECTED:
                        if (enRemoteTransitionState == eTRANSITION_CONNECT)                 //d - Ready to connect(latch disconnected)
                        {
                                enControlState = eREADY_TO_CONNECT;
                        }
                        break;
                case eREADY_TO_CONNECT:
                        if (enManualTransitionState == eTRANSITION_CONNECT)                 //e - connect
                        {
                                enLatchStatus = eLATCH_CLOSE;
                                enControlState = eCONNECTED;
                        }
                        if (enRemoteTransitionState == eTRANSITION_DISCONNECT)              //c - disconnect
                        {
                                enLatchStatus = eLATCH_OPEN;
                                enControlState = eDISCONNECTED;
                        }
                        break;
                case eCONNECTED:
                        if (enRemoteTransitionState == eTRANSITION_DISCONNECT)              //b - disconnect
                        {
                                enLatchStatus = eLATCH_OPEN;
                                enControlState = eDISCONNECTED;
                        }
                        if ((enManualTransitionState == eTRANSITION_DISCONNECT) || (enLocalTransitionState == eTRANSITION_DISCONNECT))                 //f - ready to connect(latch disconnected)       //g - ready to connect(latch disconnected)
                        {
                                enLatchStatus = eLATCH_OPEN;
                                enControlState = eREADY_TO_CONNECT;
                        }
                        break;
                default:
                        break;
                }
                break;
        case eMODE2:
                switch (enControlState)
                {
                case eDISCONNECTED:
                        if (enRemoteTransitionState == eTRANSITION_CONNECT)
                        {
                                enLatchStatus = eLATCH_CLOSE;
                                enControlState = eCONNECTED;
                        }
                        break;
                case eREADY_TO_CONNECT:
                        if (enManualTransitionState == eTRANSITION_CONNECT)
                        {
                                enLatchStatus = eLATCH_CLOSE;
                                enControlState = eCONNECTED;
                        }
                        if (enRemoteTransitionState == eTRANSITION_DISCONNECT)
                        {
                                enLatchStatus = eLATCH_OPEN;
                                enControlState = eDISCONNECTED;
                        }
                        break;
                case eCONNECTED:
                        if (enRemoteTransitionState == eTRANSITION_DISCONNECT)
                        {
                                enLatchStatus = eLATCH_OPEN;
                                enControlState = eDISCONNECTED;
                        }
                        if ((enManualTransitionState == eTRANSITION_DISCONNECT) || (enLocalTransitionState == eTRANSITION_DISCONNECT))
                        {
                                enLatchStatus = eLATCH_OPEN;
                                enControlState = eREADY_TO_CONNECT;
                        }
                        break;
                default:
                        break;
                }
                break;
        case eMODE3:
                switch (enControlState)
                {
                case eDISCONNECTED:
                        if (enRemoteTransitionState == eTRANSITION_CONNECT)
                        {
                                enControlState = eREADY_TO_CONNECT;
                        }
                        break;
                case eREADY_TO_CONNECT:
                        if (enManualTransitionState == eTRANSITION_CONNECT)
                        {
                                enLatchStatus = eLATCH_CLOSE;
                                enControlState = eCONNECTED;
                        }
                        if (enRemoteTransitionState == eTRANSITION_DISCONNECT)
                        {
                                enLatchStatus = eLATCH_OPEN;
                                enControlState = eDISCONNECTED;
                        }
                        break;
                case eCONNECTED:
                        if (enRemoteTransitionState == eTRANSITION_DISCONNECT)
                        {
                                enLatchStatus = eLATCH_OPEN;
                                enControlState = eDISCONNECTED;
                        }
                        if (enManualTransitionState == eTRANSITION_DISCONNECT)
                        {
                                enLatchStatus = eLATCH_OPEN;
                                enControlState = eREADY_TO_CONNECT;
                        }
                        break;
                default:
                        break;
                }
                break;
        case eMODE4:
                switch (enControlState)
                {
                case eDISCONNECTED:
                        if (enRemoteTransitionState == eTRANSITION_CONNECT)
                        {
                                enLatchStatus = eLATCH_CLOSE;
                                enControlState = eCONNECTED;
                        }
                        break;
                case eREADY_TO_CONNECT:
                        if (enManualTransitionState == eTRANSITION_CONNECT)
                        {
                                enLatchStatus = eLATCH_CLOSE;
                                enControlState = eCONNECTED;
                        }
                        if (enRemoteTransitionState == eTRANSITION_DISCONNECT)
                        {
                                enLatchStatus = eLATCH_OPEN;
                                enControlState = eDISCONNECTED;
                        }
                        break;
                case eCONNECTED:
                        if (enRemoteTransitionState == eTRANSITION_DISCONNECT)
                        {
                                enLatchStatus = eLATCH_OPEN;
                                enControlState = eDISCONNECTED;
                        }
                        if (enManualTransitionState == eTRANSITION_DISCONNECT)
                        {
                                enLatchStatus = eLATCH_OPEN;
                                enControlState = eREADY_TO_CONNECT;
                        }
                        break;
                default:
                        break;
                }
                break;
        case eMODE5:
                switch (enControlState)
                {
                case eDISCONNECTED:
                        if (enRemoteTransitionState == eTRANSITION_CONNECT)
                        {
                                enControlState = eREADY_TO_CONNECT;
                        }
                        break;
                case eREADY_TO_CONNECT:
                        if ((enManualTransitionState == eTRANSITION_CONNECT) || (enLocalTransitionState == eTRANSITION_CONNECT))
                        {
                                enLatchStatus = eLATCH_CLOSE;
                                enControlState = eCONNECTED;
                        }
                        if (enRemoteTransitionState == eTRANSITION_DISCONNECT)
                        {
                                enLatchStatus = eLATCH_OPEN;
                                enControlState = eDISCONNECTED;
                        }
                        break;
                case eCONNECTED:
                        if (enRemoteTransitionState == eTRANSITION_DISCONNECT)
                        {
                                enLatchStatus = eLATCH_OPEN;
                                enControlState = eDISCONNECTED;
                        }
                        if ((enManualTransitionState == eTRANSITION_DISCONNECT) || (enLocalTransitionState == eTRANSITION_DISCONNECT))
                        {
                                enLatchStatus = eLATCH_OPEN;
                                enControlState = eREADY_TO_CONNECT;
                        }
                        break;
                default:
                        break;
                }
                break;
        case eMODE6:
                switch (enControlState)
                {
                case eDISCONNECTED:
                        if (enRemoteTransitionState == eTRANSITION_CONNECT)
                        {
                                enControlState = eREADY_TO_CONNECT;
                        }
                        break;
                case eREADY_TO_CONNECT:
                        if ((enManualTransitionState == eTRANSITION_CONNECT) || (enLocalTransitionState == eTRANSITION_CONNECT))
                        {
                                enLatchStatus = eLATCH_CLOSE;
                                enControlState = eCONNECTED;
                        }
                        if (enRemoteTransitionState == eTRANSITION_DISCONNECT)
                        {
                                enLatchStatus = eLATCH_OPEN;
                                enControlState = eDISCONNECTED;
                        }
                        break;
                case eCONNECTED:
                        if (enRemoteTransitionState == eTRANSITION_DISCONNECT)
                        {
                                enLatchStatus = eLATCH_OPEN;
                                enControlState = eDISCONNECTED;
                        }
                        if ((enLocalTransitionState == eTRANSITION_DISCONNECT))
                        {
                                enLatchStatus = eLATCH_OPEN;
                                enControlState = eREADY_TO_CONNECT;
                        }
                        break;
                default:
                        break;
                }
                break;
        default:
                break;
        }

        if (enLastControlState != enControlState)
        {
                enLastControlState = enControlState;
                write_page_eeprom(STORAGE_EEPROM_DLMS_RELAY_CONTROL_STATE_ADDR, (uint8_t *)&enControlState, STORAGE_EEPROM_DLMS_RELAY_CONTROL_STATE_SIZE);
        }
        
        if (enLatchLastStatus != enLatchStatus)
        {
                //Connect Latch Here
                enLatchLastStatus = enLatchStatus;
                set_relay_output_state(enLatchStatus);
                if (enLatchStatus == eLATCH_OPEN)
                {
                        RELAY_SwitchOff();
                        if(post_relay_disconnect_callback != 0)
                        {
                          post_relay_disconnect_callback();
                        }
                }
                else
                {
                        RELAY_SwitchOn();
                        if(post_relay_connect_callback != 0)
                        {
                          post_relay_connect_callback();
                        }
                }
        }
        return enLatchStatus;
}

EN_LATCH_STATE get_relay_output_state(void)
{
  read_page_eeprom(STORAGE_EEPROM_DLMS_RELAY_OUTPUT_STATE_ADDR, (uint8_t *)&enLatchStatus, STORAGE_EEPROM_DLMS_RELAY_OUTPUT_STATE_SIZE);
  enLatchLastStatus = enLatchStatus;
  return enLatchStatus;
}

EN_LATCH_STATE relay_output_state(void)
{
  return enLatchStatus;
}

void set_relay_output_state(EN_LATCH_STATE state)
{
  enLatchStatus = state;
  write_page_eeprom(STORAGE_EEPROM_DLMS_RELAY_OUTPUT_STATE_ADDR, (uint8_t *)&enLatchStatus, STORAGE_EEPROM_DLMS_RELAY_OUTPUT_STATE_SIZE);
}

EN_CONTROL_STATE get_relay_control_state(void)
{
  read_page_eeprom(STORAGE_EEPROM_DLMS_RELAY_CONTROL_STATE_ADDR, (uint8_t *)&enControlState, STORAGE_EEPROM_DLMS_RELAY_CONTROL_STATE_SIZE);
  return enControlState;
}

void set_relay_control_state(EN_CONTROL_STATE state)
{
  enControlState = state;
  write_page_eeprom(STORAGE_EEPROM_DLMS_RELAY_CONTROL_STATE_ADDR, (uint8_t *)&enControlState, STORAGE_EEPROM_DLMS_RELAY_CONTROL_STATE_SIZE);
}

uint8_t get_relay_control_mode(void)
{
  read_page_eeprom(STORAGE_EEPROM_DLMS_RELAY_CONTROL_MODE_ADDR, (uint8_t*)&enControlMode, STORAGE_EEPROM_DLMS_RELAY_CONTROL_MODE_SIZE);
  return (uint8_t)enControlMode;
}

void set_relay_control_mode(uint8_t mode)
{
  enControlMode = (EN_CONTROL_MODE)mode;
  if((enControlMode == eMODE0) && (enRemoteTransitionState != eTRANSITION_CONNECT))
  {
    set_relay_remote_state((uint8_t)eTRANSITION_CONNECT);
  }
  write_page_eeprom(STORAGE_EEPROM_DLMS_RELAY_CONTROL_MODE_ADDR, (uint8_t*)&enControlMode, STORAGE_EEPROM_DLMS_RELAY_CONTROL_MODE_SIZE);
}

EN_TRANSITION_STATE get_relay_remote_state(void)
{
  read_page_eeprom(STORAGE_EEPROM_DLMS_RELAY_REMOTE_CONNECTION_STATUS_ADDR, (uint8_t*)&enRemoteTransitionState, STORAGE_EEPROM_DLMS_RELAY_REMOTE_CONNECTION_STATUS_SIZE);
  return enRemoteTransitionState;
}

void set_relay_remote_state(uint8_t state)
{
  if(enRemoteTransitionState != (EN_TRANSITION_STATE)state)
  {
    //TODO: Rakesh taken due to old implementation, in actual it is mode0 - mode6
    if(state == eTRANSITION_DISCONNECT)
    {
      store_event_data(TRANSACT_EVENT, 160, g_RTC_time.epoch);
    }
    else
    {
      store_event_data(TRANSACT_EVENT, 159, g_RTC_time.epoch);
    }
  }
  enRemoteTransitionState = (EN_TRANSITION_STATE)state;
  write_page_eeprom(STORAGE_EEPROM_DLMS_RELAY_REMOTE_CONNECTION_STATUS_ADDR, (uint8_t*)&enRemoteTransitionState, STORAGE_EEPROM_DLMS_RELAY_REMOTE_CONNECTION_STATUS_SIZE);
}

uint16_t get_relay_normal_disconnect_time(void)
{
  read_page_eeprom(STORAGE_EEPROM_DLMS_RELAY_NORMAL_DISCONNECT_TIME_ADDR, (uint8_t*)&stLocalCtrl.u16DisconnectTimeNormal, STORAGE_EEPROM_DLMS_RELAY_NORMAL_DISCONNECT_TIME_SIZE);
  return stLocalCtrl.u16DisconnectTimeNormal;
}

void set_relay_normal_disconnect_time(uint16_t time)
{
  if(time < MINIMUM_RELAY_NORMAL_DISCONNECT_TIME)
  {
    time = MINIMUM_RELAY_NORMAL_DISCONNECT_TIME;
  }
  if(time > MAXIMUM_RELAY_NORMAL_DISCONNECT_TIME)
  {
    time = MAXIMUM_RELAY_NORMAL_DISCONNECT_TIME;
  }
  stLocalCtrl.u16DisconnectTimeNormal = time;
  write_page_eeprom(STORAGE_EEPROM_DLMS_RELAY_NORMAL_DISCONNECT_TIME_ADDR, (uint8_t*)&stLocalCtrl.u16DisconnectTimeNormal, STORAGE_EEPROM_DLMS_RELAY_NORMAL_DISCONNECT_TIME_SIZE);
}

uint16_t get_relay_lockout_disconnect_time(void)
{
  read_page_eeprom(STORAGE_EEPROM_DLMS_RELAY_LOCKOUT_DISCONNECT_TIME_ADDR, (uint8_t*)&stLocalCtrl.u16DisconnectTimeLockout, STORAGE_EEPROM_DLMS_RELAY_LOCKOUT_DISCONNECT_TIME_SIZE);
  return stLocalCtrl.u16DisconnectTimeLockout;
}

void set_relay_lockout_disconnect_time(uint16_t time)
{
  if(time < MINIMUM_RELAY_LOCKOUT_DISCONNECT_TIME)
  {
    time = MINIMUM_RELAY_LOCKOUT_DISCONNECT_TIME;
  }
  if(time > MAXIMUM_RELAY_LOCKOUT_DISCONNECT_TIME)
  {
    time = MAXIMUM_RELAY_LOCKOUT_DISCONNECT_TIME;
  }
  stLocalCtrl.u16DisconnectTimeLockout = time;
  write_page_eeprom(STORAGE_EEPROM_DLMS_RELAY_LOCKOUT_DISCONNECT_TIME_ADDR, (uint8_t*)&stLocalCtrl.u16DisconnectTimeLockout, STORAGE_EEPROM_DLMS_RELAY_LOCKOUT_DISCONNECT_TIME_SIZE);
}

uint16_t get_relay_load_check_time(void)
{
  read_page_eeprom(STORAGE_EEPROM_DLMS_RELAY_LOAD_CHECK_TIME_ADDR, (uint8_t*)&stLocalCtrl.u16LoadChkTime, STORAGE_EEPROM_DLMS_RELAY_LOAD_CHECK_TIME_SIZE);
  return stLocalCtrl.u16LoadChkTime;
}

void set_relay_load_check_time(uint16_t time)
{
  stLocalCtrl.u16LoadChkTime = time;
  write_page_eeprom(STORAGE_EEPROM_DLMS_RELAY_LOAD_CHECK_TIME_ADDR, (uint8_t*)&stLocalCtrl.u16LoadChkTime, STORAGE_EEPROM_DLMS_RELAY_LOAD_CHECK_TIME_SIZE);
}

uint16_t get_relay_reconnect_count(void)
{
  read_page_eeprom(STORAGE_EEPROM_DLMS_RELAY_RECONNECT_COUNT_ADDR, (uint8_t*)&stLocalCtrl.u8ReconnectCount, STORAGE_EEPROM_DLMS_RELAY_RECONNECT_COUNT_SIZE);
  return stLocalCtrl.u8ReconnectCount;
}

void set_relay_reconnect_count(uint8_t count)
{
  stLocalCtrl.u8ReconnectCount = count;
  write_page_eeprom(STORAGE_EEPROM_DLMS_RELAY_RECONNECT_COUNT_ADDR, (uint8_t*)&stLocalCtrl.u8ReconnectCount, STORAGE_EEPROM_DLMS_RELAY_RECONNECT_COUNT_SIZE);
}

uint8_t read_relay_weld_status(void)
{
  read_page_eeprom(STORAGE_EEPROM_DLMS_RELAY_WELD_TAMPER_ADDR, (uint8_t*)&relay_weld_tamper_logged, STORAGE_EEPROM_DLMS_RELAY_WELD_TAMPER_SIZE);
  return relay_weld_tamper_logged;
}

uint8_t get_relay_weld_status(void)
{
  return relay_weld_tamper_logged;
}

void set_relay_weld_status(uint8_t value)
{
  relay_weld_tamper_logged = value;
  write_page_eeprom(STORAGE_EEPROM_DLMS_RELAY_WELD_TAMPER_ADDR, (uint8_t*)&relay_weld_tamper_logged, STORAGE_EEPROM_DLMS_RELAY_WELD_TAMPER_SIZE);
}
//Initialize local variables here. To be called on reset or whenever eeprom values are reinitialised in any case
void load_control_init(void)
{
  //Update values from eeprom for all the variables here
  enLastControlState = get_relay_control_state();
  get_relay_normal_disconnect_time();
  get_relay_lockout_disconnect_time();
  get_relay_load_check_time();
  get_relay_reconnect_count();
  get_relay_remote_state();
  get_relay_control_mode();
  get_relay_output_state();
  read_relay_weld_status();
  stLocalCtrlCheck.u16LoadChkTime = 0;
  stLocalCtrlCheck.u16WaitTime = 0;
  stLocalCtrlCheck.u8ReconnectCount = 0;
  
  if (enLatchStatus == eLATCH_OPEN)
  {
          RELAY_SwitchOff();
  }
  else
  {
          RELAY_SwitchOn();
  }
}

void switch_weld_tamper_scan(uint32_t epoch)
{
  static uint16_t relay_weld_counter = 0;
  uint8_t i;
  uint8_t relay_weld;
  
  if(g_inst_read_params.start_dummy_power != 0)
  {
    relay_weld_counter = 0;
  }
  else if((relay_output_state() == eDISCONNECTED) && (get_relay_weld_status() == 0))
  {
    relay_weld = 0;

    if ((get_real_signed_ph_current() >= RELAY_WELD_THERSHOLD) || (get_real_signed_neu_current() >= RELAY_WELD_THERSHOLD))
    {
      relay_weld = 1;
    }
    if(relay_weld)
    {
      relay_weld_counter++;
      if((relay_weld_counter % RELAY_WELD_RECOVER_TIME) == 0)
      {
        RELAY_SwitchOff();
        relay_weld_counter += 3;
      }
      if(relay_weld_counter >= RELAY_WELD_RECORD_TIME)
      {
        store_event_data(NOROLL_EVENT, 253, epoch);
        set_relay_weld_status(RELAY_WELD_TAG);
      }
    }
    else
    {
      relay_weld_counter = 0;
    }
  }
  
  if (get_relay_weld_status() == RELAY_WELD_TAG)
  {
    e_LCD_icon_status.latch_stat |= (1 << 1);
  }
  else
  {
    e_LCD_icon_status.latch_stat &= ~(1 << 1);
  }
}

void set_default_load_control_values(void)
{
  //Update values from eeprom for all the variables here
  set_relay_control_state(eCONNECTED);
  set_relay_normal_disconnect_time(DEFAULT_DISCONNECT_TIME_NORMAL);
  set_relay_lockout_disconnect_time(DEFAULT_DISCONNECT_TIME_LOCKOUT);
  set_relay_load_check_time(DEFAULT_LOAD_CHECK_TIME);
  set_relay_reconnect_count(DEFAULT_RECONNECT_COUNT);
  set_relay_remote_state((uint8_t)eTRANSITION_CONNECT);
  set_relay_control_mode((uint8_t)eMODE6);
  set_relay_output_state(eLATCH_CLOSE);
  set_relay_weld_status(0);
  stLocalCtrlCheck.u16LoadChkTime = 0;
  stLocalCtrlCheck.u16WaitTime = 0;
  stLocalCtrlCheck.u8ReconnectCount = 0;
}