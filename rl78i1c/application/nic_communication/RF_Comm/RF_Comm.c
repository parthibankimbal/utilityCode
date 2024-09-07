#include <stdlib.h>
#include "RF_Comm.h"

#define RF_COMM_BUFFER_SIZE             255
RF_params stRF_params;
RF_Comm_Frame stRF_Comm_Frame;
CMD_Object stCMD_Object;
static uint8_t temp_buff[RF_COMM_BUFFER_SIZE];
uint8_t RSSI_val = 0, execute_frame_gen = 0;
uint8_t RF_data_buffer[RF_COMM_BUFFER_SIZE];
uint8_t RF_data_send_end = 1;

uint32_t rf_app_firmware_version = 1;
uint32_t Tx_timeout = 0;
uint8_t data_len = 0;
RF_callback_func RF_check_forced_stop = 0;
static NicSystemError NIC_error_code = ERR_ALL_GOOD;
static uint16_t rf_comm_timeout_max = RF_COMM_TIMEOUT;
void delay1ms(uint32_t delay)
{
  while(delay--)
  {
    MCU_Delay(1000);
  }
}
void RF_Data_send_end(void)
{
  RF_data_send_end = 1;
}

static uint32_t ASCII_to_numeric(uint8_t* serial)
{
  uint32_t num = 0;
  uint8_t len = 0;
  uint8_t char_;
  while (serial[len] != 0)
  {
    char_ = (serial[len] < 0x30) || (serial[len] > 0x39);
    if (char_ == 0)
    {
      num = num * 10 + (serial[len] - 0x30);
      if (len >= METER_SERIAL_NUMBER_SIZE)
      {
        break;
      }
    }
    len++;
  }
  return num;
}

void RF_Callback(void)
{
  if (IS_RF_LINK_MISSING())
  {
    stRF_params.RF_stat = RF_Comm_Removed;
    stRF_params.RF_struct_init = 0;
    stRF_params.RF_RSSI_val = 0;
    stRF_params.nic_type = NIC_Type_None;
  }
  else if (stRF_params.RF_stat == RF_Comm_Removed)
  {
    stRF_params.RF_stat = RF_Comm_Plugged;
  }
  if(stRF_params.RF_stat == RF_Comm_Protocol_Forced_Stopped)
  {
    return;
  }
  if(RF_COMM_DATA_SENT_TIMEOUT_FLAG < RF_COMM_DATA_SENT_TIMEOUT)
  {
    RF_COMM_DATA_SENT_TIMEOUT_FLAG++;
    return;
  }
  RF_comm_stat_func(RF_COMM_DATA_BUFF,&RF_COMM_DATA_LEN);
  Send_RF_data(RF_COMM_DATA_BUFF, RF_COMM_DATA_LEN, FALSE);
}

void Send_RF_data(uint8_t *data, uint8_t len, uint8_t test)
{
  RF_COMM_DATA_LEN = len;
  if(len <= RF_COMM_BUFFER_SIZE)
  {
    memcpy(RF_COMM_DATA_BUFF, data, len);
    if(len != 0)
    {
      RF_data_send_end = 0;
//      if(test == 0)
//      {
//        RF_COMM_ASSERT;
//      }
      RF_COMM_DATA_SENT_TIMEOUT_FLAG = 0;
      send_message(1, RF_COMM_DATA_BUFF, RF_COMM_DATA_LEN);
      while(RF_data_send_end == 0);
      RF_COMM_DATA_LEN = 0;
      delay1ms(5);
      
//      if(test == 0)
//      {
//        RF_COMM_DEASSERT;
//      }
    }
  }
}

void RF_comm_stat_func(uint8_t *data , uint8_t *len)
{
  uint16_t stopped = 0;
  if(stRF_params.RF_stat == RF_Comm_Removed)
  {
     stRF_params.RF_RSSI_val = 0;
     stRF_params.RF_Comm_skip = 10;
     return;
  }
  if((stRF_params.RF_struct_init == 0) || (g_st_NIC_info.NIC_struct_init == 0))
  {
    if (stRF_params.RF_stat != RF_Comm_Removed)
    {
	    if (RF_check_forced_stop != NULL)
	    {
          stopped = RF_check_forced_stop();
          stRF_params.RF_stat = (stopped == RF_COMM_FORCE_STOP_VALUE) ? RF_Comm_Protocol_Forced_Stopped : RF_Comm_Plugged;
        } 
        else
        {
          stRF_params.RF_stat = RF_Comm_Plugged;
        }
        stRF_params.RF_Comm_skip = RF_COMM_MULTIPLIER_100MS * 5;
    }
    stRF_params.RF_RSSI_val = 0;
    stRF_params.RF_Node_ID = ASCII_to_numeric(&g_st_NIC_info.Meter_number[1]);
    stRF_params.RF_Node_Role = NODE_ROLE1;
    stRF_params.stRF_nvm_params.RF_Network_Address = g_st_NIC_info.nvm_info.network_address;
    stRF_params.stRF_nvm_params.RF_Network_Channel = g_st_NIC_info.nvm_info.network_channel;
    memcpy(&stRF_params.stRF_nvm_params.RF_encryption_key[1], g_st_NIC_info.nvm_info.encryption_key, g_st_NIC_info.nvm_info.encryption_key[0] + 1);
    stRF_params.stRF_nvm_params.RF_encryption_key[0] = stRF_params.stRF_nvm_params.RF_encryption_key[1] + 1;
    stRF_params.RF_Comm_Timeout = 0;
    stRF_params.RF_Comm_Timeout_cnt = 0;
    stRF_params.RF_Comm_Is_Assert = 0;
    stRF_params.RF_struct_init = 0xFF;
    g_st_NIC_info.NIC_struct_init = 0xFF;
    RF_send_end_callback = RF_Data_send_end;
  }
  //test
  //stRF_params.RF_stat = Cmd_Write_Network_Address;
  //
  if(((stRF_params.RF_stat != Cmd_Read_RSSI) && (stRF_params.RF_stat != Cmd_Read_CSQ_4G)) && (stRF_params.RF_Comm_skip > RF_COMM_TIMEOUT*10))
  {
    stRF_params.RF_Comm_skip = RF_COMM_TIMEOUT*10;
  }
  if((stRF_params.RF_Comm_skip != 0) && (RF_COMM_DATA_SENT_TIMEOUT_FLAG >= RF_COMM_DATA_SENT_TIMEOUT))
  {
//    RF_COMM_DEASSERT;
//    stRF_params.RF_Comm_Is_Assert = 0;
    if(stRF_params.RF_Comm_skip != 0)
    {
      stRF_params.RF_Comm_skip--;
    }
    return;
  }
//  if((stRF_params.RF_Comm_Timeout < RF_COMM_ASSERT_TIME) && (stRF_params.RF_Comm_Res_Pending == 0))
//  {
//    RF_COMM_ASSERT;
//    stRF_params.RF_Comm_Is_Assert = 1;
//    stRF_params.RF_Comm_Timeout++;
//    return;
//  }
  if((stRF_params.RF_Comm_Timeout < rf_comm_timeout_max) && (stRF_params.RF_Comm_Res_Pending == 1))
  {
    stRF_params.RF_Comm_Timeout++;
    if(stRF_params.RF_Comm_Timeout >= rf_comm_timeout_max)
    {
      stRF_params.RF_Comm_skip = 0;
      stRF_params.RF_Comm_Timeout_cnt++;
      stRF_params.RF_Comm_Res_Pending = 0;
      stRF_params.RF_Comm_Timeout = 0;
    }
    return;
  }
  if(stRF_params.RF_Comm_Timeout_cnt >= RF_COMM_CNT_TIMEOUT_RESET)
  {
    stRF_params.RF_stat = RF_Comm_Assert_Reset;
    NIC_error_code = ERR_METER_NIC_COMM_FAILURE;
  }
  stRF_params.RF_Comm_skip = RF_COMM_MULTIPLIER_100MS;
  stRF_params.RF_Comm_Timeout = 0;
  stRF_params.RF_Comm_Res_Pending = 1;
  rf_comm_timeout_max = RF_COMM_TIMEOUT;
  
  switch(stRF_params.RF_stat)
  {
    //States
    case RF_Comm_Plugged:
      RF_Plugged_func();
    break;
    case RF_Comm_Assert_Reset:
      RF_Assert_Reset_func();
    break;
    case RF_Comm_Deassert_Reset:
      RF_Deassert_Reset_func();
    break;
    case RF_Comm_Ok:
      RF_Comm_Ok_func();
    break;
    //CMDs
    case Cmd_Ping:
      rf_comm_timeout_max = RF_COMM_TIMEOUT*20;
    case Cmd_Read_Stack_Stat:
    //case Cmd_Stack_Stop:
    //case Cmd_Stack_Start:
    case Cmd_Read_Node_ID:
    case Cmd_Read_Node_Role:
    case Cmd_Read_Network_Address:
    case Cmd_Read_Network_Channel:
    case Cmd_Read_Network_Stat:
    case Cmd_Read_App_Firmware_Ver:
    //case Cmd_Read_Boot_Firmware_Ver:
    case Cmd_Read_Wire_Firmware_Ver:
    case Cmd_Read_Hardware_Ver:
    case Cmd_Read_MAC_Address:
    case Cmd_Read_params_4G:
      RF_Params_get_set_func(data, len, stRF_params.RF_stat, Read_data);
      break;
    case Cmd_Test_RF_IO_HIGH:
    case Cmd_Test_RF_IO_LOW:
        stRF_params.RF_Comm_skip = 10;
        RF_Params_get_set_func(data, len, stRF_params.RF_stat, Read_data);
        break;
    case Cmd_Read_RSSI:
    case Cmd_Read_CSQ_4G:
      stRF_params.RF_Comm_skip = RSSI_CMD_PERIOD;
      RF_Params_get_set_func(data, len, stRF_params.RF_stat, Read_data);
    break;
    case Cmd_Write_Node_ID:
    case Cmd_Write_Node_Role:
    case Cmd_Write_Network_Address:
    case Cmd_Write_Network_Channel:
    case Cmd_Write_Encryption_Key:
    case Cmd_Write_settings_4G:
      RF_Params_get_set_func(data, len, stRF_params.RF_stat, Write_data);
      break;
    
  }
  if(execute_frame_gen != 0)
  {
    execute_frame_gen = 0;
    RF_comm_frame_gen(data , len);
  }
}

void RF_Plugged_func(void)
{
  stRF_params.RF_stat = RF_Comm_Assert_Reset;
  stRF_params.RF_Comm_Res_Pending = 0;
}

void RF_Assert_Reset_func(void)
{
  ASSERT_RESET;
  RF_COMM_ASSERT;
  stRF_params.RF_stat = RF_Comm_Deassert_Reset;
  stRF_params.RF_Comm_Res_Pending = 0;
  stRF_params.RF_Comm_Timeout_cnt = 0;
}

void RF_Deassert_Reset_func(void)
{
  DEASSERT_RESET;
  RF_COMM_DEASSERT;
  stRF_params.RF_stat = RF_Comm_Ok;
  stRF_params.RF_Comm_Res_Pending = 0;
}
void RF_Comm_Ok_func(void)
{
  stRF_params.RF_stat = Cmd_Ping;
  stRF_params.RF_Comm_skip = 50*RF_COMM_MULTIPLIER_100MS;
  stRF_params.RF_Comm_Res_Pending = 0;
}

void RF_Params_get_set_func(uint8_t *data , uint8_t *len, en_RF_stat RF_stat, Direction direction)
{
  uint16_t crc; 
  if(direction == Write_data)
  {
    stCMD_Object.Data_len = 0;
    stCMD_Object.Data = temp_buff;
    switch(RF_stat)
    {
      case Cmd_Write_Node_ID:
        stCMD_Object.Data[stCMD_Object.Data_len++] = stRF_params.RF_Node_ID & 0xFF;
        stCMD_Object.Data[stCMD_Object.Data_len++] = (stRF_params.RF_Node_ID>>8) & 0xFF;
        stCMD_Object.Data[stCMD_Object.Data_len++] = (stRF_params.RF_Node_ID>>16) & 0xFF;
        break;
      case Cmd_Write_Node_Role:
        stCMD_Object.Data[stCMD_Object.Data_len++] = stRF_params.RF_Node_Role & 0xFF;
        break;
      case Cmd_Write_Network_Address:
        stCMD_Object.Data[stCMD_Object.Data_len++] = stRF_params.stRF_nvm_params.RF_Network_Address & 0xFF;
        stCMD_Object.Data[stCMD_Object.Data_len++] = (stRF_params.stRF_nvm_params.RF_Network_Address>>8) & 0xFF;
        stCMD_Object.Data[stCMD_Object.Data_len++] = (stRF_params.stRF_nvm_params.RF_Network_Address>>16) & 0xFF;
        break;
      case Cmd_Write_Network_Channel:
        stCMD_Object.Data[stCMD_Object.Data_len++] = stRF_params.stRF_nvm_params.RF_Network_Channel & 0xFF;
        break;
      case Cmd_Write_Encryption_Key:
        stCMD_Object.Data_len = 16;
        memcpy(stCMD_Object.Data,&stRF_params.stRF_nvm_params.RF_encryption_key[2],stCMD_Object.Data_len);
        break;
      case Cmd_Test_RF_GET_KEY:
        stCMD_Object.Data[stCMD_Object.Data_len++] = stRF_params.RF_Key_String[0];
        break;
      case Cmd_Write_settings_4G:
        stCMD_Object.Data[stCMD_Object.Data_len++] = stRF_params.RF_Node_ID & 0xFF;
        stCMD_Object.Data[stCMD_Object.Data_len++] = (stRF_params.RF_Node_ID>>8) & 0xFF;
        stCMD_Object.Data[stCMD_Object.Data_len++] = (stRF_params.RF_Node_ID>>16) & 0xFF;
        stCMD_Object.Data[stCMD_Object.Data_len++] = (stRF_params.RF_Node_ID>>24) & 0xFF;
        memcpy(&stCMD_Object.Data[stCMD_Object.Data_len], &g_st_NIC_info.nvm_info.APN_name[0], sizeof(NIC_nvm_info) - offsetof(NIC_nvm_info, APN_name)- member_size(NIC_nvm_info, crc));
        stCMD_Object.Data_len += sizeof(NIC_nvm_info) - offsetof(NIC_nvm_info, APN_name) - member_size(NIC_nvm_info, crc);
        
        crc = gen_crc16((uint8_t *)stCMD_Object.Data, 0,stCMD_Object.Data_len, Final);
        stCMD_Object.Data[stCMD_Object.Data_len++] = crc & 0xFF;
        stCMD_Object.Data[stCMD_Object.Data_len++] = (crc>>8) & 0xFF;
        break;
    }
  }
  else
  {
    stCMD_Object.Data = NULL;
    stCMD_Object.Data_len = 0;
  }
  
  stCMD_Object.CMD_Type = ((uint8_t)RF_stat | (direction << 7));
  stCMD_Object.CMD_Ver = 0;
  execute_frame_gen = 1;
}

void RF_comm_frame_gen(uint8_t *data , uint8_t *len)
{
  uint16_t crc = 0;
  uint8_t tempLen = RF_COMM_HEADER_SIZE + RF_COMM_CMD_HEADER_SIZE;
  memcpy(stRF_Comm_Frame.Frame_PRE, (uint8_t*)RF_COMM_PRE, RF_COMM_PRE_SIZE);
  memcpy(stRF_Comm_Frame.Frame_VER, (uint8_t*)RF_COMM_VER, RF_COMM_VER_SIZE);
  stRF_Comm_Frame.CMD_Obj = stCMD_Object;
  stRF_Comm_Frame.Frame_LEN = RF_COMM_CMD_HEADER_SIZE + stRF_Comm_Frame.CMD_Obj.Data_len + RF_COMM_CRC_SIZE;
  
  memcpy(data,(const uint8_t *)&stRF_Comm_Frame,tempLen);
  if(stRF_Comm_Frame.CMD_Obj.Data_len != 0)
  {
    memcpy(&data[tempLen],(const uint8_t *)stRF_Comm_Frame.CMD_Obj.Data,stRF_Comm_Frame.CMD_Obj.Data_len);
  }
  crc = gen_crc16(&data[tempLen - (RF_COMM_CMD_HEADER_SIZE + RF_COMM_CMD_LEN_SIZE)],crc,(stRF_Comm_Frame.Frame_LEN + RF_COMM_CMD_LEN_SIZE) - RF_COMM_CRC_SIZE,Final);
  tempLen += stRF_Comm_Frame.CMD_Obj.Data_len;
  memcpy(&data[tempLen],(const uint8_t *)&crc,RF_COMM_CRC_SIZE);
  *len = stRF_Comm_Frame.Frame_LEN + RF_COMM_HEADER_SIZE;
}

void RF_comm_frame_parser(uint8_t *data , uint8_t len)
{
  RF_Comm_Frame *stRF_Comm_Rx_Frame;
  CMD_Object *stCMD_Rx_Object;
  uint8_t tempBuff[RF_COMM_BUFFER_SIZE], tempBuff2[RF_COMM_BUFFER_SIZE], i, *tempPtr;
  uint32_t temp32_t;
  uint16_t crc = 0;
  
  stRF_Comm_Rx_Frame = (RF_Comm_Frame*)data;
  stCMD_Rx_Object = &stRF_Comm_Rx_Frame->CMD_Obj;
  if((memcmp(&stRF_Comm_Rx_Frame->Frame_PRE[0], "$CRY_RF", 7) == 0) && 
     (memcmp(&stRF_Comm_Rx_Frame->Frame_VER[0], "WP01", 4) == 0))
  {
    tempPtr = &stRF_Comm_Rx_Frame->Frame_LEN;
    crc = gen_crc16(tempPtr,crc,(stRF_Comm_Rx_Frame->Frame_LEN + 1) - RF_COMM_CRC_SIZE,Final);
    tempPtr += (stRF_Comm_Rx_Frame->Frame_LEN + 1) - RF_COMM_CRC_SIZE;

    if(crc != ((uint16_t)(*tempPtr)|(tempPtr[1]<<8)))
    {
      stRF_params.RF_stat = Res_Invalid_Crc;
      return;
    }
	stRF_params.RF_Comm_Timeout = 0;
	stRF_params.RF_Comm_Timeout_cnt = 0;
    stRF_params.RF_Comm_Res_Pending = 0;
    if(stCMD_Rx_Object->Data_len != 0)
    {
      tempPtr = &stCMD_Rx_Object->Data_len;
      for(i = 0;i < stCMD_Rx_Object->Data_len;i++)
      {
        tempPtr++;
        tempBuff[i] = *tempPtr;
      }
    }
    switch(stCMD_Rx_Object->CMD_Type)
    {
      //4G cmd-Res
      case Res_Ping_4G:
        stRF_params.RF_stat = Cmd_Write_settings_4G;
        break;
      case Res_Write_settings_4G:
        stRF_params.RF_stat = Cmd_Read_params_4G;
        break;
      case Res_Read_params_4G:
        stRF_params.RF_stat = Cmd_Read_CSQ_4G;
        break;
      case Res_Read_CSQ_4G:
        stRF_params.RF_RSSI_val = tempBuff[0];
        NIC_error_code = (NicSystemError)tempBuff[1];
        stRF_params.nic_type = NIC_Type_4G;
        break;
      
      //RF cmd-Res
      case Res_Ping:
          if (!IS_SERIAL_NUMBER_UPDATED(0))
          {
              stRF_params.RF_stat = Cmd_Read_Node_Role;
          }
          else
          {
              stRF_params.RF_stat = Cmd_Read_Node_ID;
          }        
        stRF_params.nic_type = NIC_Type_RF;     
        break;
      case Res_Write_Node_ID:
      case Res_Write_Node_Role:
      case Res_Write_Network_Address:
      case Res_Write_Network_Channel:
      case Cmd_Stat_Reset_RF:
        stRF_params.RF_stat = Cmd_Ping;
        break;
//      case Res_Stack_Stop:
//        stRF_params.RF_stat = Cmd_Stack_Start;
//        break;
      case Res_Read_Stack_Stat:
//      case Res_Stack_Start:
        stRF_params.RF_stat = Cmd_Read_RSSI;
        break;
        
      case Res_Read_Node_ID:
        temp32_t = (uint32_t)tempBuff[0] | ((uint32_t)tempBuff[1]<<8) | ((uint32_t)tempBuff[2]<<16);
        if(stRF_params.RF_Node_ID != temp32_t)
        {
          stRF_params.RF_stat = Cmd_Write_Node_ID;
        }
        else
        {
          stRF_params.RF_stat = Cmd_Read_Node_Role;
        }
        break;
      case Res_Read_Node_Role:
        temp32_t = tempBuff[0];
        if((NODE_ROLE1 != (uint8_t)temp32_t) && ((NODE_ROLE2 != (uint8_t)temp32_t)))
        {
          stRF_params.RF_stat = Cmd_Write_Node_Role;
        }
        else
        {
          stRF_params.RF_stat = Cmd_Read_Network_Address;
        }
        break;
      case Res_Read_Network_Address:
        temp32_t = (uint32_t)tempBuff[0] | ((uint32_t)tempBuff[1]<<8) | ((uint32_t)tempBuff[2]<<16);
        if(stRF_params.stRF_nvm_params.RF_Network_Address != temp32_t)
        {
          stRF_params.RF_stat = Cmd_Write_Network_Address;
        }
        else
        {
          stRF_params.RF_stat = Cmd_Read_Network_Channel;
        }
        break;
      case Res_Read_Network_Channel:
        temp32_t = tempBuff[0];
        if(stRF_params.stRF_nvm_params.RF_Network_Channel != (uint8_t)temp32_t)
        {
          stRF_params.RF_stat = Cmd_Write_Network_Channel;
        }
        else
        {
          stRF_params.RF_stat = Cmd_Write_Encryption_Key;
        }
        break;

      case Res_Write_Encryption_Key:
          stRF_params.RF_stat = Cmd_Read_App_Firmware_Ver;
        break;
      case Res_Read_App_Firmware_Ver:
        stRF_params.RF_App_Firmware_Ver = (uint32_t)tempBuff[0] | ((uint32_t)tempBuff[1]<<8) | ((uint32_t)tempBuff[2]<<16) | ((uint32_t)tempBuff[3]<<24);
//        g_dlms_rf_firmware_version = stRF_params.RF_App_Firmware_Ver;
        stRF_params.RF_stat = Cmd_Read_Wire_Firmware_Ver;
        break;
//      case Res_Read_Boot_Firmware_Ver:
//        stRF_params.RF_Boot_Firmware_Ver = (uint32_t)tempBuff[0] | ((uint32_t)tempBuff[1]<<8) | ((uint32_t)tempBuff[2]<<16) | ((uint32_t)tempBuff[3]<<24);
//        stRF_params.RF_stat = Cmd_Read_Wire_Firmware_Ver;
//        break;
      case Res_Read_Wire_Firmware_Ver:
        stRF_params.RF_Wire_Firmware_Ver = (uint32_t)tempBuff[0] | ((uint32_t)tempBuff[1]<<8) | ((uint32_t)tempBuff[2]<<16) | ((uint32_t)tempBuff[3]<<24);
//        g_dlms_rf_stack_version = stRF_params.RF_Wire_Firmware_Ver;
        stRF_params.RF_stat = Cmd_Read_Hardware_Ver;
        break;
      case Res_Read_Hardware_Ver:
        stRF_params.RF_Hardware_Ver = (uint32_t)tempBuff[0] | ((uint32_t)tempBuff[1]<<8) | ((uint32_t)tempBuff[2]<<16) | ((uint32_t)tempBuff[3]<<24);
//        g_dlms_rf_hardware_version = stRF_params.RF_Hardware_Ver;
        stRF_params.RF_stat = Cmd_Read_MAC_Address;
        break;
      case Res_Read_MAC_Address:
        memcpy(stRF_params.RF_MAC_Address, tempBuff, 6);
        tempBuff2[0] = 7;
        tempBuff2[1] = 6;
        memcpy(&tempBuff2[2], tempBuff, 6);
        NIC_MAC_validation_and_store(&g_st_NIC_info, tempBuff2, 0, 8);
        stRF_params.RF_stat = Cmd_Read_Stack_Stat;
        break;
        case Res_Test_RF_GET_KEY:
            memcpy(&stRF_params.RF_Key_String[0], tempBuff, 16);
            break;
      //error handle
      case Res_Invalid_Node_ID:
        stRF_params.RF_stat = Cmd_Write_Node_ID;
        break;
      case Res_Invalid_Node_Role:
        stRF_params.RF_stat = Cmd_Write_Node_Role;
        break;
      case Res_Invalid_Network_Address:
        stRF_params.RF_stat = Cmd_Write_Network_Address;
        break;
      case Res_Invalid_Network_Channel:
        stRF_params.RF_stat = Cmd_Write_Network_Channel;
        break;
      case Res_Read_RSSI:
        stRF_params.RF_RSSI_val = tempBuff[0];
        break;
        //not in use: 
      case Res_Read_Network_Stat:
        break;
      case Res_Test_RF_IO_HIGH:
      case Res_Test_RF_IO_LOW:
        stRF_params.RF_IO_Cmd_Res = tempBuff[0];
        break;
    }
  }
  return;
}

void RF_init_parameters(void)
{
  stRF_params.RF_struct_init = 0;
  stRF_params.RF_Comm_skip = 2;
}

void RF_reset_skip_time(void)
{
  stRF_params.RF_Comm_skip = RF_COMM_MULTIPLIER_100MS * 5;
}

void RF_comm_skip_time(void)
{
  stRF_params.RF_Comm_skip = RSSI_CMD_PERIOD;
}

uint8_t is_RF_comm_ready(void)
{
  return ((stRF_params.RF_stat == Cmd_Read_RSSI) || (stRF_params.RF_stat == Cmd_Read_CSQ_4G) || (stRF_params.RF_stat == RF_Comm_Protocol_Forced_Stopped));
}

uint8_t get_RSSI_signal_bar(void)
{
  if(stRF_params.nic_type == NIC_Type_RF)
  {
  if(stRF_params.RF_RSSI_val < 30)
        {
    return 0;
        }
  else if(stRF_params.RF_RSSI_val < 56)
        {
    return 1;
        }
  else if(stRF_params.RF_RSSI_val < 106)
  {
    return 2;
  }
  else if(stRF_params.RF_RSSI_val < 156)
        {
    return 3;
        }
  else if(stRF_params.RF_RSSI_val < 206)
  {
    return 4;
    }
  else
  {
    return 5;
  }
  }
  else
  {
    if(stRF_params.RF_RSSI_val < 2)
    {
      return 0;
    }
    else if(stRF_params.RF_RSSI_val < 8)
    {
      return 1;
    }
    else if(stRF_params.RF_RSSI_val < 14)
    {
      return 2;
    }
    else if(stRF_params.RF_RSSI_val < 18)
    {
      return 3;
    }
    else if(stRF_params.RF_RSSI_val < 22)
    {
      return 4;
    }
    else
    {
      return 5;
    }
  }
}

int16_t get_RSSI_signal_value(NIC_Type *nic_type)
{
  int16_t temp16;
  *nic_type = stRF_params.nic_type;
  if(stRF_params.nic_type == NIC_Type_RF)
  {
    if ((stRF_params.RF_RSSI_val != 0) && (!is_RF_missing()))
    {
      temp16 = (int16_t)(stRF_params.RF_RSSI_val>>1) - 131;
    }
    else
    {
      temp16 =  -131;
    }
  }
  else
  {
    temp16 = stRF_params.RF_RSSI_val;
  }
  return temp16;
}

uint8_t get_NIC_error_code(NIC_Type *nic_type)
{
  uint8_t temp8;
  *nic_type = stRF_params.nic_type;
  if(stRF_params.nic_type == NIC_Type_RF)
  {
    temp8 = 0;
  }
  else
  {
    temp8 = (uint8_t)NIC_error_code;
  }
  return temp8;
}