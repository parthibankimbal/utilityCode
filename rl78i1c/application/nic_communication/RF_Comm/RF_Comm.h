#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "tampers.h"
#include "communication.h"
#include "wrp_app_mcu.h"
#include "nic_comm.h"
#include "crc16.h"

#ifndef RF_COMM_H
#define RF_COMM_H
#define RF_COMM_PRE                                             "$CRY_RF"
#define RF_COMM_VER                                             "WP01"
#define RF_COMM_PRE_SIZE                                        7
#define RF_COMM_VER_SIZE                                        4
#define RF_COMM_LEN_SIZE                                        1
#define RF_COMM_CRC_SIZE                                        2
#define RF_COMM_CMD_SIZE                                        1
#define RF_COMM_CMD_VER_SIZE                                    1
#define RF_COMM_CMD_LEN_SIZE                                    1
#define RF_COMM_HEADER_SIZE                                     (RF_COMM_PRE_SIZE + RF_COMM_VER_SIZE + RF_COMM_LEN_SIZE)
#define RF_COMM_CMD_HEADER_SIZE                                 (RF_COMM_CMD_SIZE + RF_COMM_CMD_VER_SIZE + RF_COMM_CMD_LEN_SIZE)

#define RF_RX_BUFF_MAX_LEN                                      100
#define RF_RX_TIMEOUT_MAX                                       10
#define RF_COMM_MULTIPLIER_100MS                                1
#define RF_COMM_TIMEOUT                                         15
#define RF_COMM_ASSERT_TIME                                     1
#define RF_COMM_CNT_TIMEOUT_RESET                               30
#define RF_COMM_DATA_BUFF                                       RF_data_buffer
#define RF_COMM_DATA_LEN                                        data_len
#define RF_COMM_DATA_SENT_TIMEOUT_FLAG                          Tx_timeout
#define RF_COMM_DATA_SENT_TIMEOUT                               DEFAULT_TICK_TX_TIME_OUT/500
#define RSSI_CMD_PERIOD                                         10500// 0.04468083333333333 per tick

#define ASSERT_RESET                                            RF_RST_ASSERT()
#define DEASSERT_RESET                                          RF_RST_DEASSERT()
#define RF_COMM_ASSERT                                          RF_RTS_ASSERT()
#define RF_COMM_DEASSERT                                        RF_RTS_DEASSERT()
#define NODE_ROLE1                                              0x51
#define NODE_ROLE2                                              0x52

#define RF_COMM_FORCE_STOP_VALUE                               (uint16_t)0xA5AA

typedef enum
{
  Read_data = 0,
  Write_data,
}Direction;

typedef enum
{
  RF_Comm_Removed = 0x01,
  RF_Comm_Plugged = 0x02,
  RF_Comm_Assert_Reset = 0x03,
  RF_Comm_Deassert_Reset = 0x04,
  RF_Comm_Ping = 0x05,
  RF_Comm_Ok = 0x06,
  RF_Comm_Nok = 0x07,
  RF_Comm_Protocol_Forced_Stopped = 0x08,
  
  
  Cmd_Read_settings_4G = 0x10,
  Cmd_Read_params_4G = 0x11,
  Cmd_Read_CSQ_4G = 0x12,
  Cmd_Write_settings_4G = 0x13,
  
  Res_Read_settings_4G = 0x18,
  Res_Read_params_4G = 0x19,
  Res_Read_CSQ_4G = 0x1A,
  Res_Write_settings_4G = 0x1B,
  
  Res_Ping_4G = 0x1E,
  
  Cmd_Ping = 0x1F,
  Cmd_Read_Node_ID = 0x20,
  Cmd_Read_Node_Role = 0x21,
  Cmd_Read_Network_Address = 0x22,
  Cmd_Read_Network_Channel = 0x23,
  Cmd_Write_Node_ID = 0x24,
  Cmd_Write_Node_Role = 0x25,
  Cmd_Write_Network_Address = 0x26,
  Cmd_Write_Network_Channel = 0x27,
  Cmd_Read_RSSI = 0x28,
  Cmd_Read_Network_Stat = 0x29,
  Cmd_Stack_Stop = 0x2A,
  Cmd_Stack_Start = 0x2B,
  Cmd_Read_Stack_Stat = 0x2C,
  Cmd_Read_App_Firmware_Ver = 0x2D,
  Cmd_Read_Boot_Firmware_Ver = 0x2E,
  Cmd_Read_Wire_Firmware_Ver = 0x2F,
  Cmd_Read_Hardware_Ver = 0x30,
  Cmd_Write_Baudrate = 0x31,
  CMD_Read_Gen_UniqueID = 0x32,
  Cmd_Read_MAC_Address = 0x33,
  Cmd_Write_Encryption_Key = 0x34,
  Cmd_Test_RF = 0x35,
  Cmd_Test_RF_IO_HIGH = 0x36,
  Cmd_Test_RF_IO_LOW = 0x37,
  Cmd_Test_RF_GET_KEY = 0x38,
  Cmd_Stat_Reset_RF = 0x39,
  
  Res_Ping = 0x3D,
  Res_Read_Node_ID = 0x3E,
  Res_Read_Node_Role = 0x3F,
  Res_Read_Network_Address = 0x40,
  Res_Read_Network_Channel = 0x41,
  Res_Write_Node_ID = 0x42,
  Res_Write_Node_Role = 0x43,
  Res_Write_Network_Address = 0x44,
  Res_Write_Network_Channel = 0x45,
  Res_Read_RSSI = 0x46,
  Res_Read_Network_Stat = 0x47,
  Res_Stack_Stop = 0x48,
  Res_Stack_Start = 0x49,
  Res_Read_Stack_Stat = 0x4A,
  Res_Read_App_Firmware_Ver = 0x4B,
  Res_Read_Boot_Firmware_Ver = 0x4C,
  Res_Read_Wire_Firmware_Ver = 0x4D,
  Res_Read_Hardware_Ver = 0x4E,
  Res_Write_Baudrate = 0x4F,
  Res_Read_Gen_UniqueID = 0x50,
  Res_Read_MAC_Address = 0x51,
  Res_Write_Encryption_Key = 0x52,
  Res_Test_RF = 0x53,
  Res_Test_RF_IO_HIGH = 0x54,
  Res_Test_RF_IO_LOW = 0x55,
  Res_Test_RF_GET_KEY = 0x56,

  Res_Invalid_Data_Type = 0x5B,
  Res_Invalid_Data_Ver = 0x5C,
  Res_Invalid_Ver = 0x5D,
  Res_Invalid_Header = 0x5E,
  Res_Invalid_Crc = 0x5F,
  Res_Invalid_Frame_Error = 0x60,
  Res_Invalid_Network_Address = 0x61,
  Res_Invalid_Network_Channel = 0x62,
  Res_Invalid_Node_ID = 0x63,
  Res_Invalid_Node_Role = 0x64,
  Res_Error = 0x7F,
}en_RF_stat;

typedef enum {
  ERR_ALL_GOOD = 0,
  ERR_METER_NIC_COMM_FAILURE,
  ERR_MODEM_INIT_FAILURE,
  ERR_SIM_NOT_DETECTED,
  ERR_SIM_INVALID,
  ERR_NO_NETWORK_COVERAGE,
  ERR_NETWORK_REG_FAILURE,
  ERR_NETWORK_REG_DENIED,
  ERR_NO_APN_CONFIGURED,
  ERR_NETWORK_CONNECTION_FAILURE,
  ERR_HES_IP_PORT_NOT_CONFIGURED,
  ERR_HES_PORT_NOT_OPEN,
  ERR_KEY_MISMATCH,
} NicSystemError;

typedef uint16_t (*RF_callback_func)(void);

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
  uint8_t CMD_Type; 
  uint8_t CMD_Ver; 
  uint8_t Data_len; 
  uint8_t* Data;
}CMD_Object;

typedef struct
{
  uint8_t Frame_PRE[7];
  uint8_t Frame_VER[4];
  uint8_t Frame_LEN;
  CMD_Object CMD_Obj;
  uint16_t Frame_CRC;
}RF_Comm_Frame;

typedef struct
{
  uint32_t RF_Network_Address;
  uint8_t RF_Network_Channel;
  uint8_t RF_encryption_key[18];
  uint16_t crc;
}RF_nvm_params;


typedef struct
{
  en_RF_stat RF_stat;
  NIC_Type nic_type;
  uint8_t RF_RSSI_val;
  uint32_t RF_Node_ID;
  uint8_t RF_Node_Role;
  RF_nvm_params stRF_nvm_params;
  uint32_t RF_App_Firmware_Ver;
  uint32_t RF_Boot_Firmware_Ver;
  uint32_t RF_Wire_Firmware_Ver;
  uint32_t RF_Hardware_Ver;
  uint8_t RF_MAC_Address[6];
  uint8_t RF_Comm_Is_Assert;
  uint8_t RF_Comm_Res_Pending;
  uint8_t RF_IO_Cmd_Res;
  uint8_t RF_Key_String[18];
  uint16_t RF_Comm_Timeout;
  uint16_t RF_Comm_Timeout_cnt;
  uint8_t RF_struct_init;
  uint32_t RF_Comm_skip;
}RF_params;

#ifdef WIN32
    #pragma pack(pop)
#else
    #ifdef __CCRL__
        #pragma unpack
    #else
        #error please define proper unpacking.
    #endif
#endif

extern RF_params stRF_params;
extern uint32_t Tx_timeout;
extern uint8_t RF_data_buffer[];
extern uint8_t data_len;
extern uint8_t RF_data_send_end;
extern uint8_t network_diagnostic_error;
extern RF_callback_func RF_check_forced_stop;

void RF_Plugged_func(void);
void RF_Assert_Reset_func(void);
void RF_Deassert_Reset_func(void);
void RF_Comm_Ok_func(void);
void RF_Params_get_set_func(uint8_t *data , uint8_t *len, en_RF_stat RF_stat, Direction direction);
void RF_comm_stat_func(uint8_t *data , uint8_t *len);
void RF_comm_frame_gen(uint8_t *data , uint8_t *len);
void RF_comm_frame_parser(uint8_t *data , uint8_t len);
void RF_init_parameters(void);
void RF_Callback(void);
void RF_reset_skip_time(void);
void RF_comm_skip_time(void);
void Send_RF_data(uint8_t *data, uint8_t len, uint8_t test);
uint8_t is_RF_comm_ready(void);
uint8_t get_RSSI_signal_bar(void);
int16_t get_RSSI_signal_value(NIC_Type *nic_type);
uint8_t get_NIC_error_code(NIC_Type *nic_type);
#endif//RF_COMM_H