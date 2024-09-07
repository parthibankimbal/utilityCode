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

#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)
#include <assert.h>
#endif

//#include "../../core/msgs.h"
//#include "../../config.h"
#include <stdio.h>
#include <string.h>
#include <stdio.h> //printf needs this or error is generated.
#if _MSC_VER > 1400
#include <crtdbg.h>
#endif
//#include <stdlib.h> // malloc and free needs this or error is generated.
//#include "stack_heap_trace.h" // new malloc
#include "dlms.h"
#include "server.h"
#include "enums.h"
#include "nic_comm.h"
#include "RF_Comm.h"
#include "eeprom_storage.h"
#include "romflash.h"
#include "crc_ccitt_ffff.h"
#include "push_schedular_user.h"
#include "datalog.h"

extern Msg_Info msg_info;
extern sSA_Range SA_Range[];
//uint16_t range_num_entries = 0, range_start_entry = 0;

//uint32_t  range_num_entries, range_start_entry;
extern uint32_t SA_From_Entry, SA_To_Entry;
extern uint8_t access_selector;

int DLMS_Lib_call_back(
  dlmsServerSettings* settings,
  gxByteBuffer* data,
  gxByteBuffer* reply)
{
  return svr_handleRequest(settings, data, reply);
}
uint8_t Key_Update_BYTE = 0;
static const uint8_t Key_Transfer_IV[8] = { 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6 };
static const uint8_t Key_Len = 16;
const uint32_t Device_Logical_Address = 1;
uint16_t Device_Physical_Address = 0x100;
#define STUFF_DATA 0x80

////////////////////////////////////////// Scalers ////////////////////////////////////


uint8_t LLS_MR[10] = DEFAULT_LLS_MR;
uint8_t HLS_US[18] = DEFAULT_HLS_US;
uint8_t HLS_FW[18] = DEFAULT_HLS_FW;

uint8_t US_Security_Keys[4][16] = DEFAULT_US_METER_SECURITY_KEYS;


dlmsServerSettings lnHdlc_uart_port[2];
dlms_user_data st_dlms_user_data[2];
gxByteBuffer dlms_bb, dlms_reply;

const uint8_t lls_mr[] = DEFAULT_LLS_MR;
const uint8_t hls_us[] = DEFAULT_HLS_US;
const uint8_t hls_fw[] = DEFAULT_HLS_FW;
void set_default_meter_DLMS_security_keys(void)
{
  uint8_t security_keys[][16] = DEFAULT_US_METER_SECURITY_KEYS;
  write_page_eeprom(LLS_MR_SECRET_LOC, (uint8_t*)&lls_mr[2], LLS_MR_SECRET_SIZE);
  write_page_eeprom(HLS_US_SECRET_LOC, (uint8_t*)&hls_us[2], HLS_US_SECRET_SIZE);
  write_page_eeprom(HLS_FW_SECRET_LOC, (uint8_t*)&hls_fw[2], HLS_FW_SECRET_SIZE);
  write_page_eeprom(METER_DLMS_SECURITY_KEYS_LOC, security_keys[0], METER_DLMS_SECURITY_KEYS_SIZE);
}

void load_meter_DLMS_security_keys(void)
{
  read_page_eeprom(LLS_MR_SECRET_LOC, &LLS_MR[2], LLS_MR_SECRET_SIZE);
  read_page_eeprom(HLS_US_SECRET_LOC, &HLS_US[2], HLS_US_SECRET_SIZE);
  read_page_eeprom(HLS_FW_SECRET_LOC, &HLS_FW[2], HLS_FW_SECRET_SIZE);
  read_page_eeprom(METER_DLMS_SECURITY_KEYS_LOC, US_Security_Keys[0], METER_DLMS_SECURITY_KEYS_SIZE);
}

void attach_user_data(dlmsServerSettings* server, dlms_user_data* data)
{
  bb_attach(&server->base.ctoSChallenge, data->ctoSChallenge, 16);
  bb_attach(&server->base.stoCChallenge, data->stoCChallenge, 16);
  bb_attach(&server->base.password, data->password, 16);
  bb_attach(&server->base.cipher.blockCipherKey, data->blockCipherKey, 16);
  bb_attach(&server->base.cipher.dedicatedCipherKey, data->dedicatedCipherKey, 16);
  bb_attach(&server->base.cipher.systemTitle, data->systemTitle, 8);
  bb_attach(&server->base.cipher.clientSystemTitle, data->clientSystemTitle, 8);
  bb_attach(&server->base.cipher.authenticationKey, data->authenticationKey, 16);
  bb_attach(&server->base.cipher.CipherKey, data->CipherKey, 16);
  bb_attach(&server->receivedData, data->received_data, 750); //TODO: Rakesh
  bb_attach(&server->longTransaction, data->long_buffer, 1500);
  bb_reset(&server->longTransaction);
  bb_attach(&server->info.data, data->long_info_buffer, 1500);
  bb_reset(&server->info.data);
  bb_reset(&server->receivedData);
}
/*user setting for server start*/
int startDlmsServers(void)
{
  //Initialize DLMS settings.
  uint8_t i;
  for(i = 0; i < 2; i++)
  {
      svr_init(&lnHdlc_uart_port[i], 1, DLMS_INTERFACE_TYPE_HDLC);
      svr_start(&lnHdlc_uart_port[i]);
      attach_user_data(&lnHdlc_uart_port[i], &st_dlms_user_data[i]);
      lnHdlc_uart_port[i].base.maxServerPDUSize = DLMS_PDU_SIZE;
      lnHdlc_uart_port[i].base.maxPduSize = DLMS_PDU_SIZE;
      lnHdlc_uart_port[i].base.maxInfoTX = DLMS_HDLC_INFO_SIZE;
      lnHdlc_uart_port[i].base.maxInfoRX = DLMS_HDLC_INFO_SIZE;

      lnHdlc_uart_port[i].base.cipher.frameCounter_MR = from_eeprom(INVOCATION_COUNTER_MR_LOC + (i * sizeof(uint32_t)), INVOCATION_COUNTER_MR_SIZE);
      lnHdlc_uart_port[i].base.cipher.frameCounter_MR = (lnHdlc_uart_port[i].base.cipher.frameCounter_MR > 0xFFFFF000)?0xFFFFF000:lnHdlc_uart_port[i].base.cipher.frameCounter_MR;
      
      lnHdlc_uart_port[i].base.cipher.frameCounter_US = from_eeprom(INVOCATION_COUNTER_US_LOC + (i * sizeof(uint32_t)), INVOCATION_COUNTER_US_SIZE);
      lnHdlc_uart_port[i].base.cipher.frameCounter_US = (lnHdlc_uart_port[i].base.cipher.frameCounter_US > 0xFFFFF000)?0xFFFFF000:lnHdlc_uart_port[i].base.cipher.frameCounter_US;
      
      lnHdlc_uart_port[i].base.cipher.frameCounter_PH = from_eeprom(INVOCATION_COUNTER_PH_LOC + (i * sizeof(uint32_t)), INVOCATION_COUNTER_PH_SIZE);
      lnHdlc_uart_port[i].base.cipher.frameCounter_PH = (lnHdlc_uart_port[i].base.cipher.frameCounter_PH > 0xFFFFF000)?0xFFFFF000:lnHdlc_uart_port[i].base.cipher.frameCounter_PH;
      
      lnHdlc_uart_port[i].base.cipher.frameCounter_FW = from_eeprom(INVOCATION_COUNTER_FW_LOC + (i * sizeof(uint32_t)), INVOCATION_COUNTER_FW_SIZE);
      lnHdlc_uart_port[i].base.cipher.frameCounter_FW = (lnHdlc_uart_port[i].base.cipher.frameCounter_FW > 0xFFFFF000)?0xFFFFF000:lnHdlc_uart_port[i].base.cipher.frameCounter_FW;
      
      lnHdlc_uart_port[i].base.cipher.frameCounter_IHD = from_eeprom(INVOCATION_COUNTER_IHD_LOC + (i * sizeof(uint32_t)), INVOCATION_COUNTER_IHD_SIZE);
      lnHdlc_uart_port[i].base.cipher.frameCounter_IHD = (lnHdlc_uart_port[i].base.cipher.frameCounter_IHD > 0xFFFFF000)?0xFFFFF000:lnHdlc_uart_port[i].base.cipher.frameCounter_IHD;
  }
  lnHdlc_uart_port[0].base.interfaceType = DLMS_INTERFACE_TYPE_HDLC;
  lnHdlc_uart_port[1].base.interfaceType = DLMS_INTERFACE_TYPE_WRAPPER;
  return 0;
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

void get_server_system_title(void)
{
  uint8_t i;
  uint8_t serial_number[METER_SERIAL_NUMBER_SIZE] = "AS123454678";
  union{
          uint8_t Ser8_t[4];
          uint32_t Ser32_t;
  }serial;
  
  get_meter_serial_number(serial_number);
  serial.Ser32_t = ASCII_to_numeric(serial_number);
  server_system_title[0] = 9;
  server_system_title[1] = 8;
  memcpy(&server_system_title[2], MANUFACTURER_DLMS_IDENTIFIER, MANUFACTURER_DLMS_IDENTIFIER_SIZE);
  server_system_title[5] = 0;
  
  server_system_title[6] = serial.Ser8_t[3];
  server_system_title[7] = serial.Ser8_t[2];
  server_system_title[8] = serial.Ser8_t[1];
  server_system_title[9] = serial.Ser8_t[0];
}

const uint8_t time_scaler[] =
{
  5,
    2,
      TAG_INT8, INJECT8(0),
      TAG_ENUM, OBIS_UNIT_TIME_MINUTE
};

const uint8_t scalar_unit_I[] =
{
  5,
    2,
      TAG_INT8, INJECT8(SCALER_CURRENT),
      TAG_ENUM, OBIS_UNIT_CURRENT_AMPERE
};

const uint8_t scalar_unit_V[] =
{
  5,
    2,
      TAG_INT8, INJECT8(SCALER_VOLTAGE),
      TAG_ENUM, OBIS_UNIT_VOLTAGE_VOLT
};

const uint8_t scalar_unit_PF[] =
{
  5,
    2,
      TAG_INT8, INJECT8(SCALER_POWER_FACTOR),
      TAG_ENUM, OBIS_UNIT_UNITLESS_COUNT
};

const uint8_t scalar_unit_NONE[] =
{
  5,
    2,
      TAG_INT8, INJECT8(0),
      TAG_ENUM, OBIS_UNIT_UNITLESS_COUNT
};

const uint8_t scalar_unit_Frequency[] =
{
  5,
    2,
      TAG_INT8, INJECT8(SCALER_FREQUENCY),
      TAG_ENUM, OBIS_UNIT_FREQUENCY_HERTZ
};

const uint8_t scalar_unit_KVA[] =
{
  5,
    2,
      TAG_INT8, INJECT8(SCALER_APPARENT_POWER),
      TAG_ENUM, OBIS_UNIT_APPARENT_POWER_VA
};

const uint8_t scalar_unit_KW[] =
{
  5,
    2,
      TAG_INT8, INJECT8(SCALER_ACTIVE_POWER),
      TAG_ENUM, OBIS_UNIT_ACTIVE_POWER_WATT
};

const uint8_t scalar_unit_KVAr[] =
{
  5,
    2,
    TAG_INT8, INJECT8(SCALER_REACTIVE_POWER),
    TAG_ENUM, OBIS_UNIT_REACTIVE_POWER_VAR
};

const uint8_t scalar_unit_KWh[] =
{
  5,
    2,
      TAG_INT8, INJECT8(SCALER_ACTIVE_ENERGY),
      TAG_ENUM, OBIS_UNIT_ACTIVE_ENERGY_WATT_HOUR
};
const uint8_t scalar_unit_KVAh[] =
{
  5,
    2,
      TAG_INT8, INJECT8(SCALER_APPARENT_ENERGY),
      TAG_ENUM, OBIS_UNIT_APPARENT_ENERGY_VA_HOUR
};

const uint8_t scalar_unit_KVArh[] =
{
  5,
    2,
      TAG_INT8, INJECT8(SCALER_REACTIVE_ENERGY),
      TAG_ENUM, OBIS_UNIT_REACTIVE_ENERGY_VAR_HOUR
};

const uint8_t scalar_unit_time_hour[] =
{
  5,
    2,
      TAG_INT8, INJECT8(0),
      TAG_ENUM, OBIS_UNIT_TIME_HOUR
};

const uint8_t scalar_unit_time_min[] =
{
  5,
    2,
      TAG_INT8, INJECT8(0),
      TAG_ENUM, OBIS_UNIT_TIME_MINUTE
};

const uint8_t scalar_unit_temperature_c[] =
{
  5,
    2,
      TAG_INT8, INJECT8(SCALER_TEMPERATURE),
      TAG_ENUM, OBIS_UNIT_TEMPERATURE_DEGREE_CENTIGRADE
};

////////////////////////////////////Globle variable for DLMS///////////////////////////
uint64_t tmp_uint64;
uint32_t tmp_uint32;
uint16_t tmp_uint16;
uint8_t tmp_uint8;
int32_t tmp_int32;
int16_t tmp_int16;
int8_t tmp_int8;
float32_t tmp_float;
///////////////////////////////////////////////////////////////////////////////////////

extern const uint8_t context_info_A0[];
extern const uint8_t context_info_A1[];
extern const uint8_t context_info_A2[];
extern const uint8_t context_info_A3[];
extern const uint8_t context_info_A4[];
extern const uint8_t context_info_A5[];

extern uint8_t Data_Buffer[];
uint32_t  range_num_entries = 0, range_start_entry = 0;

//sBilling_Profile stBilling_Profile;
//sTamper_Profile stTamper_Profile;
//////////////////////////////////////////////////////

uint8_t set_tamper_time;

const uint16_t Internal_CT_Ratio = 1;
const uint16_t Internal_PT_Ratio = 1;
const uint16_t Meter_Constant = FACTORY_DEFAULT_METER_CONSTANT;//TOTAL_ENERGY_PULSES_PER_KW_HOUR;
int get_string_item(uint8_t* buf, int len, int item);

/// Associations

const uint8_t partner_PC[] =
{
  6,
    2,
      TAG_INT8, 0x10,                         //Client SAP
      TAG_UINT16, INJECT16(1)                 //Server SAP
};

const uint8_t partner_MR[] =
{
  6,
    2,
      TAG_INT8, 0x20,                         //Client SAP
      TAG_UINT16, INJECT16(1)                 //Server SAP
};

const uint8_t partner_US[] =
{
  6,
    2,
      TAG_INT8, 0x30,                         //Client SAP
      TAG_UINT16, INJECT16(1)                 //Server SAP
};

const uint8_t partner_Push[] =
{
  6,
    2,
      TAG_INT8, 0x40,                         //Client SAP
      TAG_UINT16, INJECT16(1)                 //Server SAP
};

const uint8_t partner_Firmware[] =
{
  6,
    2,
      TAG_INT8, 0x50,                         //Client SAP
      TAG_UINT16, INJECT16(1)                 //Server SAP
};

const uint8_t partner_IHD[] =
{
  6,
    2,
      TAG_INT8, 0x60,                         //Client SAP
      TAG_UINT16, INJECT16(1)                 //Server SAP
};

static const uint8_t application_context_name_logical_names[] =
{
  8,7, 0x60, 0x85, 0x74, 0x05, 0x08, 0x01, 0x01
};
static const uint8_t application_context_name_logical_names_with_ciphering[] =
{
  8,7, 0x60, 0x85, 0x74, 0x05, 0x08, 0x01, 0x03
};
const uint8_t auth_mech_name_NS[] = { 8,0x07,0x60,0x85,0x74,0x05,0x08,0x02,0x00 };
const uint8_t auth_mech_name_LS[] = { 8,0x07,0x60,0x85,0x74,0x05,0x08,0x02,0x01 };
const uint8_t auth_mech_name_HS[] = { 8,0x07,0x60,0x85,0x74,0x05,0x08,0x02,0x02 };

void secret_func(void* data, int direction)
{
  uint8_t * xdata;

  if (direction == ATTR_WRITE)
  {
    xdata = (uint8_t*)data;

    if (*xdata == LLS_MR_SECRET_SIZE)
    {
      memcpy(&LLS_MR[2],(xdata + 1), LLS_MR_SECRET_SIZE);
      store_event_data(TRANSACT_EVENT, 161, g_RTC_time.epoch);
      write_page_eeprom(LLS_MR_SECRET_LOC,&LLS_MR[2],LLS_MR_SECRET_SIZE);
    }
  }
}
//TODO: Rakesh check and validate this function 23Dec22
void check_connection_status(void* data, int direction, uint8_t clientAddress)
{
  uint8_t i;
  Data_Buffer[0] = 0;
  for(i=0;i<2;i++)
  {
      if (lnHdlc_uart_port[i].base.clientAddress == clientAddress && lnHdlc_uart_port[i].base.connected != 0)
      {
          Data_Buffer[0] = 2;
      }
  }
}
void check_connection_status_PC(void* data, int direction)
{
  if (direction == ATTR_READ)
    check_connection_status(data, direction, 0x10);
}
void check_connection_status_MR(void* data, int direction)
{
  if (direction == ATTR_READ)
    check_connection_status(data, direction, 0x20);
}

void check_connection_status_US(void* data, int direction)
{
  if (direction == ATTR_READ)
    check_connection_status(data, direction, 0x30);
}

void check_connection_status_Push(void* data, int direction)
{
  if (direction == ATTR_READ)
    check_connection_status(data, direction, 0x40);
}

void check_connection_status_Firmware(void* data, int direction)
{
  if (direction == ATTR_READ)
    check_connection_status(data, direction, 0x50);
}

void check_connection_status_IHD(void* data, int direction)
{
  if (direction == ATTR_READ)
    check_connection_status(data, direction, 0x60);
}

static const uint8_t security_setup_reference_PC[] =
{
   1,
    0
};
static const uint8_t security_setup_reference_MR[] =
{
   7,
    6,
      0,0,43,0,2,255
};
static const uint8_t security_setup_reference_US[] =
{
   7,
    6,
      0,0,43,0,3,255
};
static const uint8_t security_setup_reference_Push[] =
{
   7,
    6,
      0,0,43,0,4,255
};
static const uint8_t security_setup_reference_Firmware[] =
{
   7,
    6,
      0,0,43,0,5,255
};
static const uint8_t security_setup_reference_IHD[] =
{
   7,
    6,
      0,0,43,0,6,255
};

#ifdef ASSOCIATION_VER_3
static const uint8_t user_list[] =
{
   1,
    0
};

static const uint8_t current_user[] =
{
   1,
    2,2,
      TAG_UINT8, 0,
      TAG_VISIBLE_STRING,0
};
#endif
void reply_to_hls_auth(uint8_t* data, uint16_t data_len, uint8_t* response, uint16_t* response_len)
{
}

void change_hls_secret_US(uint8_t* data, uint16_t data_len, uint8_t* response, uint16_t* response_len)
{
  if (data_len != HLS_US_SECRET_SIZE+1)
  {
    *response_len = 0xffff;
  }
  else
  {
    memcpy(&HLS_US[2], &data[1], HLS_US_SECRET_SIZE);
    store_event_data(TRANSACT_EVENT, 162, g_RTC_time.epoch);

    write_page_eeprom(HLS_US_SECRET_LOC, &HLS_US[2], HLS_US_SECRET_SIZE);
    response[0] = 0x01;
    response[1] = 0x00;
    response[2] = 0x09;
    memcpy(&response[3], data, data_len);
    *response_len = data_len;
  }
}

void change_hls_secret_Firmware(uint8_t* data, uint16_t data_len, uint8_t* response, uint16_t* response_len)
{
  if (data_len != HLS_FW_SECRET_SIZE+1)
  {
    *response_len = 0xffff;
  }
  else
  {
    memcpy(&HLS_FW[2], &data[1], HLS_FW_SECRET_SIZE);
    store_event_data(TRANSACT_EVENT, 163, g_RTC_time.epoch);

    write_page_eeprom(HLS_FW_SECRET_LOC, &HLS_FW[2], HLS_FW_SECRET_SIZE);

    response[0] = 0x01;
    response[1] = 0x00;
    response[2] = 0x09;
    memcpy(&response[3], data, data_len);
    *response_len = data_len;
  }
}

void add_object(uint8_t* data, uint16_t data_len, uint8_t* response, uint16_t* response_len)
{
}

void remove_object(uint8_t* data, uint16_t data_len, uint8_t* response, uint16_t* response_len)
{
}

void add_user(uint8_t* data, uint16_t data_len, uint8_t* response, uint16_t* response_len)
{
}

void remove_user(uint8_t* data, uint16_t data_len, uint8_t* response, uint16_t* response_len)
{
}

static const struct method_desc_s ln_40_methods[] =
{
    {1, ACCESS_PC___MR___USR__FWR_, reply_to_hls_auth},
    {2, ACCESS_PC___MR___US__, change_hls_secret_US},
    {3, ACCESS_PC___MR___USR_, add_object},
    {4, ACCESS_PC___MR___USR_, remove_object}
#ifdef ASSOCIATION_VER_3
    ,
    {5, ACCESS_PC___MR___USR_, add_user},
    {6, ACCESS_PC___MR___USR_, remove_user}
#endif
};

static const struct method_desc_s ln_40_methods_PC[] =
{
    {1, ACCESS_PC___MR___US__, reply_to_hls_auth},
    {2, ACCESS_PC___MR___US__, change_hls_secret_US},
    {3, ACCESS_PC___MR___US__, add_object},
    {4, ACCESS_PC___MR___US__, remove_object}
#ifdef ASSOCIATION_VER_3
    ,
    {5, ACCESS_PC___MR___US__, add_user},
    {6, ACCESS_PC___MR___US__, remove_user}
#endif
};

static const struct method_desc_s ln_40_methods_MR[] =
{
    {1, ACCESS_PC___MR___US__, reply_to_hls_auth},
    {2, ACCESS_PC___MR___US__, change_hls_secret_US},
    {3, ACCESS_PC___MR___US__, add_object},
    {4, ACCESS_PC___MR___US__, remove_object}
#ifdef ASSOCIATION_VER_3
    ,
    {5, ACCESS_PC___MR___US__, add_user},
    {6, ACCESS_PC___MR___US__, remove_user}
#endif
};

static const struct method_desc_s ln_40_methods_US[] =
{
    {1, ACCESS_PC___MR___USR_, reply_to_hls_auth},
    {2, ACCESS_PC___MR___USR_, change_hls_secret_US},
    {3, ACCESS_PC___MR___US__, add_object},
    {4, ACCESS_PC___MR___US__, remove_object}
#ifdef ASSOCIATION_VER_3
    ,
    {5, ACCESS_PC___MR___USR_, add_user},
    {6, ACCESS_PC___MR___USR_, remove_user}
#endif
};

static const struct method_desc_s ln_40_methods_Push[] =
{
    {1, ACCESS_PC___MR___US__, reply_to_hls_auth},
    {2, ACCESS_PC___MR___US__, change_hls_secret_US},
    {3, ACCESS_PC___MR___US__, add_object},
    {4, ACCESS_PC___MR___US__, remove_object}
#ifdef ASSOCIATION_VER_3
    ,
    {5, ACCESS_PC___MR___US__, add_user},
    {6, ACCESS_PC___MR___US__, remove_user}
#endif
};

static const struct method_desc_s ln_40_methods_Firmware[] =
{
    {1, ACCESS_PC___MR___US___FWR_, reply_to_hls_auth},
    {2, ACCESS_PC___MR___USR_, change_hls_secret_Firmware},
    {3, ACCESS_PC___MR___US__, add_object},
    {4, ACCESS_PC___MR___US__, remove_object}
#ifdef ASSOCIATION_VER_3
    ,
    {5, ACCESS_PC___MR___US__, add_user},
    {6, ACCESS_PC___MR___US__, remove_user}
#endif
};

static const struct method_desc_s ln_40_methods_IHD[] =
{
    {1, ACCESS_PC___MR___US__, reply_to_hls_auth},
    {2, ACCESS_PC___MR___US__, change_hls_secret_US},
    {3, ACCESS_PC___MR___US__, add_object},
    {4, ACCESS_PC___MR___US__, remove_object}
#ifdef ASSOCIATION_VER_3
    ,
    {5, ACCESS_PC___MR___US__, add_user},
    {6, ACCESS_PC___MR___US__, remove_user}
#endif
};

static const struct attribute_desc_s ln_40_attrs_PC[] =
{
    {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL},
    {2, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void*)Data_Buffer, NULL},
    {3, ACCESS_PCR__MRR__USR_, TAG_STRUCTURE,       (void*)partner_PC, NULL},
    {4, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void*)application_context_name_logical_names, NULL},
    {5, ACCESS_PCR__MRR__USR_, TAG_STRUCTURE,       (void*)&context_info_A0, NULL},
    {6, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void*)auth_mech_name_NS, NULL},
    {7, ACCESS_PC___MR___US__, TAG_OCTET_STRING,    (void*)Data_Buffer, secret_func},
    {8, ACCESS_PCR__MRR__USR_, TAG_ENUM,            (void*)Data_Buffer, check_connection_status_PC},
    {9, ACCESS_PCR__MR___US__, TAG_OCTET_STRING,    (void*)security_setup_reference_PC, NULL}
#ifdef ASSOCIATION_VER_3
    ,
    {10, ACCESS_PC___MRR__USR_, TAG_ARRAY,          (void*)user_list, NULL},
    {11, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,      (void*)current_user, NULL}
#endif
};

static const struct attribute_desc_s ln_40_attrs_MR[] =
{
    {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL},
    {2, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void*)Data_Buffer, NULL},
    {3, ACCESS_PCR__MRR__USR_, TAG_STRUCTURE,       (void*)partner_MR, NULL},
    {4, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void*)application_context_name_logical_names_with_ciphering, NULL},
    {5, ACCESS_PCR__MRR__USR_, TAG_STRUCTURE,       (void*)&context_info_A1, NULL},
    {6, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void*)auth_mech_name_LS, NULL},
    {7, ACCESS_PC___MR___US_W, TAG_OCTET_STRING,    (void*)Data_Buffer, secret_func},
    {8, ACCESS_PCR__MRR__USR_, TAG_ENUM,            (void*)Data_Buffer, check_connection_status_MR},
    {9, ACCESS_PC___MRR__US__, TAG_OCTET_STRING,    (void*)security_setup_reference_MR, NULL}
#ifdef ASSOCIATION_VER_3
    ,
    {10, ACCESS_PC___MRR__USR_, TAG_ARRAY,          (void*)user_list, NULL},
    {11, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,      (void*)current_user, NULL}
#endif
};

static const struct attribute_desc_s ln_40_attrs_US[] =
{
    {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL},
    {2, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void*)Data_Buffer, NULL},
    {3, ACCESS_PCR__MRR__USR_, TAG_STRUCTURE,       (void*)partner_US, NULL},
    {4, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void*)application_context_name_logical_names_with_ciphering, NULL},
    {5, ACCESS_PCR__MRR__USR_, TAG_STRUCTURE,       (void*)&context_info_A2, NULL},
    {6, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void*)auth_mech_name_HS, NULL},
    {7, ACCESS_PC___MR___US__, TAG_OCTET_STRING,    (void*)Data_Buffer, secret_func},
    {8, ACCESS_PCR__MRR__USR_, TAG_ENUM,            (void*)Data_Buffer, check_connection_status_US},
    {9, ACCESS_PC___MR___USR_, TAG_OCTET_STRING,    (void*)security_setup_reference_US, NULL}
#ifdef ASSOCIATION_VER_3
    ,
    {10, ACCESS_PC___MRR__USR_, TAG_ARRAY,          (void*)user_list, NULL},
    {11, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,      (void*)current_user, NULL}
#endif
};

static const struct attribute_desc_s ln_40_attrs_Push[] =
{
    {1, ACCESS_PC___MRR__USR__PHR_, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL},
    {2, ACCESS_PC___MRR__USR__PHR_, TAG_OCTET_STRING,    (void*)Data_Buffer, NULL},
    {3, ACCESS_PC___MRR__USR__PHR_, TAG_STRUCTURE,       (void*)partner_Push, NULL},
    {4, ACCESS_PC___MRR__USR__PHR_, TAG_OCTET_STRING,    (void*)application_context_name_logical_names_with_ciphering, NULL},
    {5, ACCESS_PC___MRR__USR__PHR_, TAG_STRUCTURE,       (void*)&context_info_A3, NULL},
    {6, ACCESS_PC___MRR__USR__PHR_, TAG_OCTET_STRING,    (void*)auth_mech_name_NS, NULL},
    {7, ACCESS_PC___MR___US__,      TAG_OCTET_STRING,    (void*)Data_Buffer, secret_func},
    {8, ACCESS_PC___MRR__USR__PHR_, TAG_ENUM,            (void*)Data_Buffer, check_connection_status_Push},
    {9, ACCESS_PC___MR___US___PHR_, TAG_OCTET_STRING,    (void*)security_setup_reference_Push, NULL}
#ifdef ASSOCIATION_VER_3
    ,
    {10, ACCESS_PC___MRR__USR__PHR_, TAG_ARRAY,          (void*)user_list, NULL},
    {11, ACCESS_PC___MRR__USR__PHR_, TAG_STRUCTURE,      (void*)current_user, NULL}
#endif
};

static const struct attribute_desc_s ln_40_attrs_Firmware[] =
{
    {1, ACCESS_PC___MRR__USR__FWR_, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL},
    {2, ACCESS_PC___MRR__USR__FWR_, TAG_OCTET_STRING,    (void*)Data_Buffer, NULL},
    {3, ACCESS_PC___MRR__USR__FWR_, TAG_STRUCTURE,       (void*)partner_Firmware, NULL},
    {4, ACCESS_PC___MRR__USR__FWR_, TAG_OCTET_STRING,    (void*)application_context_name_logical_names_with_ciphering, NULL},
    {5, ACCESS_PC___MRR__USR__FWR_, TAG_STRUCTURE,       (void*)&context_info_A4, NULL},
    {6, ACCESS_PC___MRR__USR__FWR_, TAG_OCTET_STRING,    (void*)auth_mech_name_HS, NULL},
    {7, ACCESS_PC___MR___US__,      TAG_OCTET_STRING,    (void*)Data_Buffer, secret_func},
    {8, ACCESS_PC___MRR__USR__FWR_, TAG_ENUM,            (void*)Data_Buffer, check_connection_status_Firmware},
    {9, ACCESS_PC___MR___US___FWR_, TAG_OCTET_STRING,    (void*)security_setup_reference_Firmware, NULL}
#ifdef ASSOCIATION_VER_3
    ,
    {10, ACCESS_PC___MRR__USR__FWR_, TAG_ARRAY,          (void*)user_list, NULL},
    {11, ACCESS_PC___MRR__USR__FWR_, TAG_STRUCTURE,      (void*)current_user, NULL}
#endif
};

static const struct attribute_desc_s ln_40_attrs_IHD[] =
{
    {1, ACCESS_PC___MR___US___IHR_, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL},
    {2, ACCESS_PC___MR___US___IHR_, TAG_OCTET_STRING,    (void*)Data_Buffer, NULL},
    {3, ACCESS_PC___MR___US___IHR_, TAG_STRUCTURE,       (void*)partner_IHD, NULL},
    {4, ACCESS_PC___MR___US___IHR_, TAG_OCTET_STRING,    (void*)application_context_name_logical_names_with_ciphering, NULL},
    {5, ACCESS_PC___MR___US___IHR_, TAG_STRUCTURE,       (void*)&context_info_A5, NULL},
    {6, ACCESS_PC___MR___US___IHR_, TAG_OCTET_STRING,    (void*)auth_mech_name_NS, NULL},
    {7, ACCESS_PC___MR___US__, TAG_OCTET_STRING,         (void*)Data_Buffer, secret_func},
    {8, ACCESS_PC___MR___US___IHR_, TAG_ENUM,            (void*)Data_Buffer, check_connection_status_IHD},
    {9, ACCESS_PC___MR___US___IHR_, TAG_OCTET_STRING,    (void*)security_setup_reference_IHD, NULL}
#ifdef ASSOCIATION_VER_3
    ,
    {10, ACCESS_PC___MRR__USR__IHR_, TAG_ARRAY,          (void*)user_list, NULL},
    {11, ACCESS_PC___MRR__USR__IHR_, TAG_STRUCTURE,      (void*)current_user, NULL}
#endif
};

static const struct attribute_desc_s ln_40_attrs_PC_Mndtry[] =
{
    {1, ACCESS_PCR__MR___US__, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL},
    {2, ACCESS_PCR__MR___US__, TAG_OCTET_STRING,    (void*)Data_Buffer, NULL},
    {3, ACCESS_PCR__MR___US__, TAG_STRUCTURE,       (void*)partner_PC, NULL},
    {4, ACCESS_PCR__MR___US__, TAG_OCTET_STRING,    (void*)application_context_name_logical_names, NULL},
    {5, ACCESS_PCR__MR___US__, TAG_STRUCTURE,       (void*)&context_info_A0, NULL},
    {6, ACCESS_PCR__MR___US__, TAG_OCTET_STRING,    (void*)auth_mech_name_NS, NULL},
    {7, ACCESS_PC___MR___US__, TAG_OCTET_STRING,    (void*)Data_Buffer, secret_func},
    {8, ACCESS_PCR__MR___US__, TAG_ENUM,            (void*)Data_Buffer, check_connection_status_PC},
    {9, ACCESS_PCR__MR___US__, TAG_OCTET_STRING,    (void*)security_setup_reference_PC, NULL}
#ifdef ASSOCIATION_VER_3
    ,
    {10, ACCESS_PC___MRR__USR_, TAG_ARRAY,          (void*)user_list, NULL},
    {11, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,      (void*)current_user, NULL}
#endif
};

static const struct attribute_desc_s ln_40_attrs_MR_Mndtry[] =
{
    {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL},
    {2, ACCESS_PC___MRR__US__, TAG_OCTET_STRING,    (void*)Data_Buffer, NULL},
    {3, ACCESS_PC___MRR__US__, TAG_STRUCTURE,       (void*)partner_MR, NULL},
    {4, ACCESS_PC___MRR__US__, TAG_OCTET_STRING,    (void*)application_context_name_logical_names_with_ciphering, NULL},
    {5, ACCESS_PC___MRR__US__, TAG_STRUCTURE,       (void*)&context_info_A1, NULL},
    {6, ACCESS_PC___MRR__US__, TAG_OCTET_STRING,    (void*)auth_mech_name_LS, NULL},
    {7, ACCESS_PC___MR___US__, TAG_OCTET_STRING,    (void*)Data_Buffer, secret_func},
    {8, ACCESS_PC___MRR__US__, TAG_ENUM,            (void*)Data_Buffer, check_connection_status_MR},
    {9, ACCESS_PC___MRR__US__, TAG_OCTET_STRING,    (void*)security_setup_reference_MR, NULL}
#ifdef ASSOCIATION_VER_3
    ,
    {10, ACCESS_PC___MRR__USR_, TAG_ARRAY,          (void*)user_list, NULL},
    {11, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,      (void*)current_user, NULL}
#endif
};

static const struct attribute_desc_s ln_40_attrs_US_Mndtry[] =
{
    {1, ACCESS_PC___MR___USR_, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL},
    {2, ACCESS_PC___MR___USR_, TAG_OCTET_STRING,    (void*)Data_Buffer, NULL},
    {3, ACCESS_PC___MR___USR_, TAG_STRUCTURE,       (void*)partner_US, NULL},
    {4, ACCESS_PC___MR___USR_, TAG_OCTET_STRING,    (void*)application_context_name_logical_names_with_ciphering, NULL},
    {5, ACCESS_PC___MR___USR_, TAG_STRUCTURE,       (void*)&context_info_A2, NULL},
    {6, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void*)auth_mech_name_HS, NULL},
    {7, ACCESS_PC___MR___US__, TAG_OCTET_STRING,    (void*)Data_Buffer, secret_func},
    {8, ACCESS_PC___MR___USR_, TAG_ENUM,            (void*)Data_Buffer, check_connection_status_US},
    {9, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void*)security_setup_reference_US, NULL}
#ifdef ASSOCIATION_VER_3
    ,
    {10, ACCESS_PC___MRR__USR_, TAG_ARRAY,          (void*)user_list, NULL},
    {11, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,      (void*)current_user, NULL}
#endif
};

static const struct attribute_desc_s ln_40_attrs_Push_Mndtry[] =
{
    {1, ACCESS_PC___MR___US___PHR_, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL},
    {2, ACCESS_PC___MR___US___PHR_, TAG_OCTET_STRING,    (void*)Data_Buffer, NULL},
    {3, ACCESS_PC___MR___US___PHR_, TAG_STRUCTURE,       (void*)partner_Push, NULL},
    {4, ACCESS_PC___MR___US___PHR_, TAG_OCTET_STRING,    (void*)application_context_name_logical_names_with_ciphering, NULL},
    {5, ACCESS_PC___MR___US___PHR_, TAG_STRUCTURE,       (void*)&context_info_A3, NULL},
    {6, ACCESS_PC___MR___US___PHR_, TAG_OCTET_STRING,    (void*)auth_mech_name_NS, NULL},
    {7, ACCESS_PC___MR___US__, TAG_OCTET_STRING,         (void*)Data_Buffer, secret_func},
    {8, ACCESS_PC___MR___US___PHR_, TAG_ENUM,            (void*)Data_Buffer, check_connection_status_Push},
    {9, ACCESS_PC___MR___US___PHR_, TAG_OCTET_STRING,    (void*)security_setup_reference_Push, NULL}
#ifdef ASSOCIATION_VER_3
    ,
    {10, ACCESS_PC___MRR__USR__PHR_, TAG_ARRAY,          (void*)user_list, NULL},
    {11, ACCESS_PC___MRR__USR__PHR_, TAG_STRUCTURE,      (void*)current_user, NULL}
#endif
};

static const struct attribute_desc_s ln_40_attrs_Firmware_Mndtry[] =
{
    {1, ACCESS_PC___MR___USR__FWR_, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL},
    {2, ACCESS_PC___MR___US___FWR_, TAG_OCTET_STRING,    (void*)Data_Buffer, NULL},
    {3, ACCESS_PC___MR___US___FWR_, TAG_STRUCTURE,       (void*)partner_Firmware, NULL},
    {4, ACCESS_PC___MR___US___FWR_, TAG_OCTET_STRING,    (void*)application_context_name_logical_names_with_ciphering, NULL},
    {5, ACCESS_PC___MR___US___FWR_, TAG_STRUCTURE,       (void*)&context_info_A4, NULL},
    {6, ACCESS_PC___MR___US___FWR_, TAG_OCTET_STRING,    (void*)auth_mech_name_HS, NULL},
    {7, ACCESS_PC___MR___US__, TAG_OCTET_STRING,         (void*)Data_Buffer, secret_func},
    {8, ACCESS_PC___MR___US___FWR_, TAG_ENUM,            (void*)Data_Buffer, check_connection_status_Firmware},
    {9, ACCESS_PC___MR___US___FWR_, TAG_OCTET_STRING,    (void*)security_setup_reference_Firmware, NULL}
#ifdef ASSOCIATION_VER_3
    ,
    {10, ACCESS_PC___MRR__USR__FWR_, TAG_ARRAY,          (void*)user_list, NULL},
    {11, ACCESS_PC___MRR__USR__FWR_, TAG_STRUCTURE,      (void*)current_user, NULL}
#endif
};

static const struct attribute_desc_s ln_40_attrs_IHD_Mndtry[] =
{
    {1, ACCESS_PC___MR___US___IHR_, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL},
    {2, ACCESS_PC___MR___US___IHR_, TAG_OCTET_STRING,    (void*)Data_Buffer, NULL},
    {3, ACCESS_PC___MR___US___IHR_, TAG_STRUCTURE,       (void*)partner_IHD, NULL},
    {4, ACCESS_PC___MR___US___IHR_, TAG_OCTET_STRING,    (void*)application_context_name_logical_names_with_ciphering, NULL},
    {5, ACCESS_PC___MR___US___IHR_, TAG_STRUCTURE,       (void*)&context_info_A5, NULL},
    {6, ACCESS_PC___MR___US___IHR_, TAG_OCTET_STRING,    (void*)auth_mech_name_NS, NULL},
    {7, ACCESS_PC___MR___US__, TAG_OCTET_STRING,         (void*)Data_Buffer, secret_func},
    {8, ACCESS_PC___MR___US___IHR_, TAG_ENUM,            (void*)Data_Buffer, check_connection_status_IHD},
    {9, ACCESS_PC___MR___US___IHR_, TAG_OCTET_STRING,    (void*)security_setup_reference_IHD, NULL}
#ifdef ASSOCIATION_VER_3
    ,
    {10, ACCESS_PC___MRR__USR__IHR_, TAG_ARRAY,          (void*)user_list, NULL},
    {11, ACCESS_PC___MRR__USR__IHR_, TAG_STRUCTURE,      (void*)current_user, NULL}
#endif
};
/*
void buildauth1(void *data, int direction)
{
  uint8_t i, *xdata;

  if (direction == ATTR_WRITE)
  {
  xdata = (uint8_t *)data;

  if (*xdata == 8)
    {
      for (i = 0; i<8; i++)
      auth1[i + 2] = *(xdata + i + 1);
      set_auth_data = 1;
    }
  }

  xdata = (uint8_t*)data;
  *xdata = sizeof(auth1);
  memcpy(xdata + 1, auth1 + 1, sizeof(auth1) - 1);
}

static const uint8_t security_setup_reference[] =
{
  3, '?', '?', '?'
};

static const struct attribute_desc_s ln_40_attrs_NS_Mndtry[] =
{
  { 1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *)object_list[4].instance_id, NULL },
  { 2, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *)Data_Buffer, NULL },
  { 3, ACCESS_PCR__MRR__USR_, TAG_STRUCTURE,       (void *)partner_NS, NULL },
  { 4, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *)application_context_name_logical_names, NULL },
  { 5, ACCESS_PCR__MRR__USR_, TAG_STRUCTURE,       (void *)&context_info_A0, NULL },
  { 6, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *)auth_mech_name_NS, NULL },
  { 7, ACCESS_PC___MR___US_W, TAG_OCTET_STRING,    (void *)Data_Buffer, buildauth1 },
  { 8, ACCESS_PC___MRR__USR_, TAG_ENUM,            (void *)Data_Buffer, check_connection_status_NS },
  { 9, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *)security_setup_reference, NULL }
};
void change_hls_secret(uint8_t *data, uint16_t data_len, uint8_t *response, uint16_t *response_len)
{
  uint16_t i;
  //TODO:RAKESH
  if (data_len != 18)
  {
    *response_len = 0xffff;
  }
  else //auth2 update
  {
  //response_len=0;

    for (i = 0; i<16; i++)
      auth2[i + 2] = *(data + i + 2);
    set_auth_data = 2;

    response = (uint8_t*)data;
    *response_len = data_len;
  }
}

static const struct method_desc_s ln_40_methods[] =
{
  { 1, ACCESS_PC___MR___USRW, reply_to_hls_auth },
  { 2, ACCESS_PC___MR___USRW_MSRW, change_hls_secret },
  { 3, ACCESS_PC___MR___USR_, add_object },
  { 4, ACCESS_PC___MR___USR_, remove_object }
};
*/

//connect disconnect
void load_output_stat_Func(void *data, int direction)
{
  tmp_uint8 = get_relay_output_state();
}

void load_control_stat_Func(void *data, int direction)
{
  tmp_uint8 = get_relay_control_state();
}

void load_control_mode_Func(void *data, int direction)
{
  uint8_t* data_ptr = (uint8_t*)data;
  if(direction == ATTR_WRITE)
  {
    tmp_uint8 = *data_ptr;
    if(tmp_uint8 == 0)
    {
      set_relay_control_mode(tmp_uint8);
      //store_event_data(TRANSACT_EVENT, 160, g_RTC_time.epoch);
    }
    else if(tmp_uint8 == 6)
    {
      set_relay_control_mode(tmp_uint8);
      //store_event_data(TRANSACT_EVENT, 159, g_RTC_time.epoch);
    }
  }
  else
  {
    tmp_uint8 = get_relay_control_mode();
  }
}

static const struct attribute_desc_s Obj_Connect_Disconnect[] =
{
  { 1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,  (void *)object_list[0].instance_id, NULL },
  { 2, ACCESS_PC___MRR__USR_, TAG_BOOLEAN,       (void *)&tmp_uint8, load_output_stat_Func },
  { 3, ACCESS_PC___MRR__USR_, TAG_ENUM,          (void *)&tmp_uint8, load_control_stat_Func },
  { 4, ACCESS_PC___MRR__USRW, TAG_ENUM,          (void *)&tmp_uint8, load_control_mode_Func }
};

void Remote_Disconnect(uint8_t *data, uint16_t data_len, uint8_t *response, uint16_t *response_len)
{
  if(get_relay_control_mode())
  {
    set_relay_remote_state(eTRANSITION_DISCONNECT);
  }
}

void Remote_Reconnect(uint8_t *data, uint16_t data_len, uint8_t *response, uint16_t *response_len)
{
  set_relay_remote_state(eTRANSITION_CONNECT);
}

static const struct method_desc_s Obj_Connect_Disconnect_Methods[] =
{
  { 1, ACCESS_PC___MR___USR_, Remote_Disconnect },
  { 2, ACCESS_PC___MR___USR_, Remote_Reconnect }
};

/*               Image transfer                   */

#define TOTAL_IMAGE_BLOCKS 2040 //bitwise count
const uint32_t Image_Block_Size = 128;
uint8_t g_image_transfer_status = 0;
uint8_t g_image_activate_counter = 0;

void Image_Transfer_update_flag(uint8_t Status)
{
  write_page_eeprom(STORAGE_EEPROM_DLMS_IMAGE_TRANSFER_STATUS_ADDR, &Status, STORAGE_EEPROM_DLMS_IMAGE_TRANSFER_STATUS_SIZE);
}

void Image_Transferred_Blocks_Status_Func(void *data, int direction)
{
  uint8_t* data_ptr = (uint8_t*)data;
  uint16_t temp_len_max;
  if(ATTR_READ == direction)
  {
    uint8_t len = 0;
    temp_len_max = TOTAL_IMAGE_BLOCKS/8 + (TOTAL_IMAGE_BLOCKS%8?1:0) + 3;
    if(temp_len_max > 127)
    {
      data_ptr[len++] = 0x82;
      data_ptr[len++] = (temp_len_max >> 8) & 0xFF;
      data_ptr[len++] = temp_len_max & 0xFF;
    }
    else
    {
      data_ptr[len++] = temp_len_max;
    }
    
    data_ptr[len++] = 0x82;
    data_ptr[len++] = (TOTAL_IMAGE_BLOCKS >> 8) & 0xFF;
    data_ptr[len++] = TOTAL_IMAGE_BLOCKS & 0xFF;
    read_page_eeprom(STORAGE_EEPROM_DLMS_IMAGE_TRANSFER_BIT_STATUS_ADDR, (uint8_t*)&data_ptr[len], TOTAL_IMAGE_BLOCKS/8 + (TOTAL_IMAGE_BLOCKS%8?1:0));
    //len += TOTAL_IMAGE_BLOCKS/8 + (TOTAL_IMAGE_BLOCKS%8?1:0);
  }
}

void Image_First_Not_Tranferred_Block_Num_Func(void *data, int direction)
{
  uint16_t i = 0;
  uint8_t temp_data , j, _break = 0;
  for (i = 0; i < TOTAL_IMAGE_BLOCKS;)
  {
    read_page_eeprom(STORAGE_EEPROM_DLMS_IMAGE_TRANSFER_BIT_STATUS_ADDR + i/8, &temp_data, 1);
    for (j = 0; j < 8; j++, i++)
    {
      if ((temp_data & (1 << (7 - (j % 8)))) == 0)
      {
        _break = 1;
        break;
      }
    }
    if(_break)
    {
      break;
    }
  }
  tmp_uint32 = i;
}
void Image_Transfer_Enabled_Func(void *data, int direction)
{
    tmp_uint8 = 1;
}

void Image_Transfer_Status_Func(void *data, int direction)
{
  read_page_eeprom(STORAGE_EEPROM_DLMS_IMAGE_TRANSFER_STATUS_ADDR, &tmp_uint8, STORAGE_EEPROM_DLMS_IMAGE_TRANSFER_STATUS_SIZE);
}

UN_CONFIG*  uConfigPtr;
void get_Image_To_Activate_Info(void *data, int direction)
{
    uint8_t* Temp_data = (uint8_t*)data;
    uint16_t crc_out = 0;
    uint8_t temp_buffer[256];
    unsigned char len = 0;
    UN_CONFIG __far* uConfigPtr_far = (UN_CONFIG __far*)FIRMWARE_CONFIG_DATA;
    
    read_dataPktToFlash((uint32_t)uConfigPtr_far, temp_buffer, 256);
    R_CRC_Set(0xFFFF);
    crc_out = R_CRC_Calculate(temp_buffer, sizeof(UN_CONFIG) - 2);
    
    if(crc_out == uConfigPtr_far->stPkt.wCRC)
    {
      uConfigPtr = (UN_CONFIG*)&temp_buffer[0];
      Temp_data[len++] = 12 + uConfigPtr->stPkt.wDLMSImageInfoSize + uConfigPtr->stPkt.wDLMSSignatureSize;
      Temp_data[len++] = 1;
      Temp_data[len++] = TAG_STRUCTURE;
      Temp_data[len++] =  3;
      Temp_data[len++] = TAG_UINT32;
      Temp_data[len++] = uConfigPtr->stPkt.dImageSize & 0xFF;
      Temp_data[len++] = (uConfigPtr->stPkt.dImageSize>>8) & 0xFF;
      Temp_data[len++] = (uConfigPtr->stPkt.dImageSize>>16) & 0xFF;
      Temp_data[len++] = (uConfigPtr->stPkt.dImageSize >>24) & 0xFF;
      Temp_data[len++] = TAG_OCTET_STRING;
      Temp_data[len++] =  uConfigPtr->stPkt.wDLMSImageInfoSize;
      memcpy(&Temp_data[len], uConfigPtr->stPkt.bDLMSImageInfo, uConfigPtr->stPkt.wDLMSImageInfoSize);
      len += uConfigPtr->stPkt.wDLMSImageInfoSize;
      Temp_data[len++] = TAG_OCTET_STRING;
      Temp_data[len++] =  uConfigPtr->stPkt.wDLMSSignatureSize;
      memcpy(&Temp_data[len], uConfigPtr->stPkt.bDLMSSignature, uConfigPtr->stPkt.wDLMSSignatureSize);
      //len += uConfigPtr->stPkt.wDLMSSignatureSize;
    }
    else
    {
      Temp_data[len++] = 12;
      Temp_data[len++] = 1;
      Temp_data[len++] = TAG_STRUCTURE;
      Temp_data[len++] = 3;
      Temp_data[len++] = TAG_UINT32;
      Temp_data[len++] = 0;
      Temp_data[len++] = 0;
      Temp_data[len++] = 0;
      Temp_data[len++] = 0;
      Temp_data[len++] = TAG_OCTET_STRING;
      Temp_data[len++] = 0;
      Temp_data[len++] = TAG_OCTET_STRING;
      Temp_data[len++] = 0;
    }
}
static const struct attribute_desc_s Obj_Image_Transfer[] =
{
  { 1, ACCESS_PC___MRR__USR__FWR_, TAG_OCTET_STRING,    (void *)object_list[0].instance_id, NULL },
  { 2, ACCESS_PC___MRR__USR__FWR_, TAG_UINT32,          (void *)&Image_Block_Size, NULL },
  { 3, ACCESS_PC___MRR__USR__FWR_, TAG_BITSTRING,       (void *)Data_Buffer,Image_Transferred_Blocks_Status_Func},
  { 4, ACCESS_PC___MRR__USR__FWR_, TAG_UINT32,          (void *)&tmp_uint32, Image_First_Not_Tranferred_Block_Num_Func },
  { 5, ACCESS_PC___MRR__USR__FWR_, TAG_BOOLEAN,         (void *)&tmp_uint8, Image_Transfer_Enabled_Func },
  { 6, ACCESS_PC___MRR__USR__FWR_, TAG_ENUM,            (void *)&tmp_uint8, Image_Transfer_Status_Func },
  { 7, ACCESS_PC___MRR__USR__FWR_, TAG_ARRAY,           (void *)Data_Buffer, get_Image_To_Activate_Info },
};

void Image_Transfer_Initiate(uint8_t *data, uint16_t data_len, uint8_t *response, uint16_t *response_len)
{
  uint8_t* data_ptr = (uint8_t*)response;
  memset(data_ptr,0,STORAGE_EEPROM_DLMS_IMAGE_TRANSFER_BIT_STATUS_SIZE);
  Image_Transfer_update_flag((uint8_t)TransferInitiated);
  write_page_eeprom(STORAGE_EEPROM_DLMS_IMAGE_TRANSFER_BIT_STATUS_ADDR, data_ptr, STORAGE_EEPROM_DLMS_IMAGE_TRANSFER_BIT_STATUS_SIZE);
}

void Image_Block_Transfer(uint8_t *data, uint16_t data_len, uint8_t *response, uint16_t *response_len)
{
  uint8_t data_type, pos = 1;
  uint16_t data_length;
  uint32_t block_no;

  data_type = data[1];
  if (data_type == 0x06)
  {
    block_no = ((uint32_t)data[2] << 24) | ((uint32_t)data[3] << 16) | ((uint32_t)data[4] << 8) | ((uint32_t)data[5]);
    if(block_no < TOTAL_IMAGE_BLOCKS)
    {
      //data_type = data[6];
      data_length = data[7];
      if(data_length == 0x81)
      {
        data_length = data[8];
        pos = 2;
      }
      else if(data_length == 0x82)
      {
        data_length = data[8];
        data_length <<= 8;
        data_length |= data[9];
        pos = 3;
      }
      
      write_dataPktToFlash(FIRST_USER_FLASH_ADDR + (block_no * Image_Block_Size), &data[7 + pos], data_length);
      pos = 0;
      if(block_no % 8 == 0)
      {
        write_eeprom(STORAGE_EEPROM_DLMS_IMAGE_TRANSFER_BIT_STATUS_ADDR + block_no / 8, pos);
      }
      else
      {
        pos = read_eeprom(STORAGE_EEPROM_DLMS_IMAGE_TRANSFER_BIT_STATUS_ADDR + block_no / 8);
      }
      pos |= (uint8_t)(1 << (7 - (block_no % 8)));
      write_eeprom(STORAGE_EEPROM_DLMS_IMAGE_TRANSFER_BIT_STATUS_ADDR + block_no / 8,pos);
    }
  }
  return;
}

void Image_Verify(uint8_t *data, uint16_t data_len, uint8_t *response, uint16_t *response_len)
{  
  uint32_t dCurrentAddress;
  uint16_t crc_out = 0;
  UN_CONFIG __far* uConfigFlashPtr;
  uint8_t status = 0;
  UN_CONFIG img_data;
  UN_CONFIG firmware_data;
  
  read_dataPktToFlash(IMAGE_CONFIG_DATA, (uint8_t*)&img_data, sizeof(UN_CONFIG));
  read_dataPktToFlash(FIRMWARE_CONFIG_DATA, (uint8_t*)&firmware_data, sizeof(UN_CONFIG));
  
  if(memcmp(img_data.stPkt.bDLMSImageInfo, firmware_data.stPkt.bDLMSImageInfo, img_data.stPkt.wDLMSImageInfoSize) != 0)
  {
    status = 1;
  }
  if(status != 0)
  {
    Image_Transfer_update_flag((uint8_t)VerificationFailed);
    return;
  }
  
  if(read_eeprom(STORAGE_EEPROM_DLMS_IMAGE_TRANSFER_STATUS_ADDR) == (uint8_t)TransferInitiated)
  {
    Image_Transfer_update_flag((uint8_t)VerificationInitiated);
    
    R_CRC_Set(0xFFFF);
    dCurrentAddress = FIRST_USER_FLASH_ADDR;
    uConfigFlashPtr = (UN_CONFIG __far*)FIRMWARE_CONFIG_DATA;
    while (dCurrentAddress < FIRMWARE_CONFIG_DATA)
    {
      kick_watchdog();
      read_dataPktToFlash(dCurrentAddress, response, 256);
      crc_out = R_CRC_Calculate(response, 256);
      dCurrentAddress += 256;
    }
    if (crc_out == uConfigFlashPtr->stPkt.wImageCRC)
    {
      Image_Transfer_update_flag((uint8_t)VerificationSuccessful);
    }
    else
    {
      Image_Transfer_update_flag((uint8_t)VerificationFailed);
    }
  }
}

void Image_Activate(uint8_t *data, uint16_t data_len, uint8_t *response, uint16_t *response_len)
{
  if(read_eeprom(STORAGE_EEPROM_DLMS_IMAGE_TRANSFER_STATUS_ADDR) == (uint8_t)VerificationSuccessful)
  {
    write_image_flag_activation_rom_initiated();
    //generate_bill_on_demand(&g_RTC_time);
    Image_Transfer_update_flag((uint8_t)ActivationInitiated);
    g_image_activate_counter = 10;
    g_image_transfer_status = (uint8_t)ActivationInitiated;
  }
  else
  {
    write_image_flag_activation_rom_fail();
    write_eeprom(STORAGE_EEPROM_DLMS_IMAGE_TRANSFER_STATUS_ADDR, (uint8_t)ActivationFailed);
  }
}

static const struct method_desc_s Obj_Image_Transfer_Methods[] =
{
  { 1, ACCESS_PC___MR___USR__FWR_, Image_Transfer_Initiate },
  { 2, ACCESS_PC___MR___USR__FWR_, Image_Block_Transfer },
  { 3, ACCESS_PC___MR___USR__FWR_, Image_Verify },
  { 4, ACCESS_PC___MR___USR__FWR_, Image_Activate }
};

/*
void Obj_Load_Profile_reset(uint8_t *data, uint16_t data_len, uint8_t *response, uint16_t *response_len)
{
}

//- End of function --------------------------------------------------------

void Obj_Load_Profile_capture(uint8_t *data, uint16_t data_len, uint8_t *response, uint16_t *response_len)
{
}
*/
/*
void capture_bit_string(void *data, int direction)
{
  uint8_t *Temp_Data_Buffer;
  Temp_Data_Buffer = (uint8_t*)data;
  Temp_Data_Buffer[0] = 03;
  Temp_Data_Buffer[1] = 10;
  Temp_Data_Buffer[2] = 0xF0;
  Temp_Data_Buffer[3] = 0xF0;
}
*/
/*
void Capture_Load_Profile_Data(void *data, int direction)
{
  // Load Template for Load Profile
  // interpret_template(data, direction, Load_Profile_Buffer_Template, sizeof(Load_Profile_Buffer_Template));

  msg_info.template = Load_Profile_Buffer_Template;
  msg_info.sz_template = sizeof(Load_Profile_Buffer_Template);

  if (Load_Profile_Entries_In_Use == 0)
  {
    msg_info.num_entries = 0xffff;
  }
  else
  {
  if (access_selector>0)
  {
    if (access_selector == 1)//range
    {
      if ((SA_Range[0].Year >= 0xffff) || (SA_Range[1].Year >= 0xffff))
      msg_info.num_entries = 0xffff;
      else
      {
        //find_entries_by_range();//&SA_Range[0],&SA_Range[1]
        range_num_entries = 2;
        range_start_entry = 1;
        if (range_num_entries == 0)
        {
          msg_info.num_entries = 0xffff;
        }
        else
        {
          msg_info.num_entries = range_num_entries;
          msg_info.start_entry = range_start_entry;
        }
      }
    }
    else //entry
    {
      if (SA_To_Entry == 0)
      {
        msg_info.num_entries = Load_Profile_Entries_In_Use;
        msg_info.start_entry = 1;
      }
      else
      {
        msg_info.num_entries = SA_To_Entry - SA_From_Entry + 1;
        msg_info.start_entry = SA_From_Entry;
      }
    }
  }
  else
  {
    msg_info.num_entries = Load_Profile_Entries_In_Use;
    msg_info.start_entry = 1;
  }
  }
  msg_info.column_szs = (const uint16_t *)Load_Profile_Column_Szs;
}
*/
//const uint8_t Load_Profile_Capture_Objects[] =
//{
//	0x82,INJECT16(14 * 18 + 1),
//	/*TAG_ARRAY,*/14,
//	TAG_STRUCTURE, 4,
//	TAG_UINT16, INJECT16(CLASS_ID_CLOCK),
//	TAG_OCTET_STRING, 6, OBIS_GROUP_A_ABSTRACT_OBJECTS, 0, 1, 0, 0, 255, // Date & Time
//	TAG_INT8, 2,
//	TAG_UINT16, INJECT16(0),
//	TAG_STRUCTURE, 4,
//	TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
//	TAG_OCTET_STRING, 6, 1, 0, 31, 27, 0, 255,
//	TAG_INT8, 2,
//	TAG_UINT16, INJECT16(0),
//	TAG_STRUCTURE, 4,
//	TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
//	TAG_OCTET_STRING, 6, 1, 0, 51, 27, 0, 255,
//	TAG_INT8, 2,
//	TAG_UINT16, INJECT16(0),
//	TAG_STRUCTURE, 4,
//	TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
//	TAG_OCTET_STRING, 6, 1, 0, 71, 27, 0, 255,
//	TAG_INT8, 2,
//	TAG_UINT16, INJECT16(0),
//	TAG_STRUCTURE, 4,
//	TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
//	TAG_OCTET_STRING, 6, 1, 0, 32, 27, 0, 255,
//	TAG_INT8, 2,
//	TAG_UINT16, INJECT16(0),
//	TAG_STRUCTURE, 4,
//	TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
//	TAG_OCTET_STRING, 6, 1, 0, 52, 27, 0, 255,
//	TAG_INT8, 2,
//	TAG_UINT16, INJECT16(0),
//	TAG_STRUCTURE, 4,
//	TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
//	TAG_OCTET_STRING, 6, 1, 0, 72, 27, 0, 255,
//	TAG_INT8, 2,
//	TAG_UINT16, INJECT16(0),
//	TAG_STRUCTURE, 4,
//	TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
//	TAG_OCTET_STRING, 6, 1, 0, 33, 27, 0, 255,
//	TAG_INT8, 2,
//	TAG_UINT16, INJECT16(0),
//	TAG_STRUCTURE, 4,
//	TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
//	TAG_OCTET_STRING, 6, 1, 0, 53, 27, 0, 255,
//	TAG_INT8, 2,
//	TAG_UINT16, INJECT16(0),
//	TAG_STRUCTURE, 4,
//	TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
//	TAG_OCTET_STRING, 6, 1, 0, 73, 27, 0, 255,
//	TAG_INT8, 2,
//	TAG_UINT16, INJECT16(0),
//	TAG_STRUCTURE, 4,
//	TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
//	TAG_OCTET_STRING, 6, 1, 0, 1, 29, 0, 255, // Block Energy KWh
//	TAG_INT8, 2,
//	TAG_UINT16, INJECT16(0),
//	TAG_STRUCTURE, 4,
//	TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
//	TAG_OCTET_STRING, 6, 1, 0, 9, 29, 0, 255, // Block Energy KWh
//	TAG_INT8, 2,
//	TAG_UINT16, INJECT16(0),
//	TAG_STRUCTURE, 4,
//	TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
//	TAG_OCTET_STRING, 6, 1, 0, 5, 29, 0, 255, // Block Energy KVArh - Lag
//	TAG_INT8, 2,
//	TAG_UINT16, INJECT16(0),
//	TAG_STRUCTURE, 4,
//	TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
//	TAG_OCTET_STRING, 6, 1, 0, 8, 29, 0, 255, // Block Energy KVArh - Lead
//	TAG_INT8, 2,
//	TAG_UINT16, INJECT16(0),
//};

//const uint8_t Load_Profile_Capture_Objects[] =
//{
//  (7 * 18 + 1),
//    //TAG_ARRAY,
//    7,
//      TAG_STRUCTURE, 4,
//      TAG_UINT16, INJECT16(CLASS_ID_CLOCK),
//      TAG_OCTET_STRING, 6, OBIS_GROUP_A_ABSTRACT_OBJECTS, 0, 1, 0, 0, 255, // Date & Time
//      TAG_INT8, 2,
//      TAG_UINT16, INJECT16(0),
//      TAG_STRUCTURE, 4,
//      TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
//      TAG_OCTET_STRING, 6, 1, 0, 12, 27, 0, 255,
//      TAG_INT8, 2,
//      TAG_UINT16, INJECT16(0),
//      TAG_STRUCTURE, 4,
//      TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
//      TAG_OCTET_STRING, 6, 1, 0,  1, 29, 0, 255,
//      TAG_INT8, 2,
//      TAG_UINT16, INJECT16(0),
//      TAG_STRUCTURE, 4,
//      TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
//      TAG_OCTET_STRING, 6, 1, 0,  9, 29, 0, 255,
//      TAG_INT8, 2,
//      TAG_UINT16, INJECT16(0),
//      TAG_STRUCTURE, 4,
//      TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
//      TAG_OCTET_STRING, 6, 1, 0, 2, 29, 0, 255,
//      TAG_INT8, 2,
//      TAG_UINT16, INJECT16(0),
//      TAG_STRUCTURE, 4,
//      TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
//      TAG_OCTET_STRING, 6, 1, 0,  10, 29, 0, 255,
//      TAG_INT8, 2,
//      TAG_UINT16, INJECT16(0),
//      TAG_STRUCTURE, 4,
//      TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
//      TAG_OCTET_STRING, 6, 1, 0,  16, 29, 0, 255,
//      TAG_INT8, 2,
//      TAG_UINT16, INJECT16(0),
//};
int get_string_item(uint8_t* buf, int len, int item)
{
  uint8_t time_string[12];
  uint16_t entry_no = 0;
  switch (item)
  {
  //Instant profile
  case ITEM_TAG_CURRENT_DATETIME:
    Epoch_To_DLMS_Time_With_RTC_Status(g_RTC_time.epoch, time_string, &g_RTC_time);
    memcpy(buf, time_string, len);
    return len;
  case ITEM_TAG_CUM_LAST_MD_KW_DATETIME:
    Epoch_To_DLMS_Time(get_mdkw_time(0), buf);
    return len;
  case ITEM_TAG_CUM_LAST_MD_KVA_DATETIME:
    Epoch_To_DLMS_Time(get_mdkva_time(0), buf);
    return len;
  case ITEM_TAG_CUM_LAST_MD_EVENT_DATETIME:
    {
      ONE_MONTH_ENERGY_DATA_LOG bill_log;
      get_billing_profile_data(&bill_log, 1, 1);
      Epoch_To_DLMS_Time(bill_log.u32_epoch, time_string);
      memcpy(buf, time_string, len);
    }
    return len;
    
  //Name plate
  case ITEM_TAG_SLNO:
    memcpy(buf, &Meter_Sr_No[2], Meter_Sr_No[1]);
    return Meter_Sr_No[1];
  case ITEM_TAG_DEVICE_ID:
    len = get_device_id(buf);
    //memcpy(&buf[0], &buf[2], len);
    return  len;
  case ITEM_TAG_MANFACT_NAME:
    memcpy(buf, &Manufacturer_Name[2], Manufacturer_Name[1]);
    return Manufacturer_Name[1];
  case ITEM_TAG_FW_VERSION:
    memcpy(buf, METER_VERSION_NUMBER, strlen(METER_VERSION_NUMBER));
    return strlen(METER_VERSION_NUMBER);
  case ITEM_TAG_METER_CATEGORY:
    memcpy(buf, &Meter_category[2], Meter_category[1]);
    return Meter_category[1];
  case ITEM_TAG_CURRENT_RATING:
    memcpy(buf, &Current_rating[2], Current_rating[1]);
    return Current_rating[1];
  //  case ITEM_TAG_NIC_SERIAL:
  //    for(i=0;i<st_NIC_info.MAC_address[0];i++)
  //      *(buf+i)=st_NIC_info.MAC_address[i+1];
  //    return st_NIC_info.MAC_address[0];
  //  case ITEM_TAG_SIM_NUMBER:
  //    for(i=0;i<SIM_number[1];i++)
  //      *(buf+i)=SIM_number[i+2];
  //    return SIM_number[1];
  //  case ITEM_TAG_VOLTAGE_RATE:
  //    for(i=0;i<Voltage_rating[1];i++)
  //      *(buf+i)=Voltage_rating[i+2];
  //    return Voltage_rating[1];
  //  //
  // Tamper Profile Data
  case ITEM_TAG_DATETIME_TMPR:
    if(FiFO_LiFO==0)
    {
      entry_no = msg_info.start_entry + msg_info.num_entries - msg_info.entries_remaining - 1;
    }
    else
    {
      entry_no = msg_info.total_entries - (msg_info.start_entry + msg_info.num_entries - msg_info.entries_remaining);
    }
    get_tamper_profile(&u_s_profile.event, scan_event_type, entry_no);
    Epoch_To_DLMS_Time(u_s_profile.event.u32_epoch, buf);
    return len;
  // Daily Load Profile Data
  case ITEM_TAG_DATETIME_DL_LP:
    if(FiFO_LiFO==0)
    {
      entry_no = msg_info.start_entry + msg_info.num_entries - msg_info.entries_remaining - 1;
    }
    else
    {
      entry_no = msg_info.total_entries - (msg_info.start_entry + msg_info.num_entries - msg_info.entries_remaining);
    }
    get_daily_load_profile(&u_s_profile.daily_load, entry_no);
    Epoch_To_DLMS_Time(u_s_profile.daily_load.u32_epoch, buf);
    return len;
    
  // Load Profile Data
  case ITEM_TAG_DATETIME_LP:
    if(FiFO_LiFO==0)
    {
      entry_no = msg_info.start_entry + msg_info.num_entries - msg_info.entries_remaining - 1;
    }
    else
    {
      entry_no = msg_info.total_entries - (msg_info.start_entry + msg_info.num_entries - msg_info.entries_remaining);
    }
    get_load_profile(&u_s_profile.block_load, entry_no, e_entry_none);
    Epoch_To_DLMS_Time(u_s_profile.block_load.u32_epoch, buf);
    return len;
  
  // Billing Data
  case ITEM_TAG_BILLING_DATETIME_BI:
    if(FiFO_LiFO==0)
    {
      entry_no = msg_info.start_entry + msg_info.num_entries - msg_info.entries_remaining - 1;
      entry_no++;
      if(entry_no == msg_info.total_entries)
      {
        entry_no = 0;
      }
    }
    else
    {
      entry_no = msg_info.total_entries - (msg_info.start_entry + msg_info.num_entries - msg_info.entries_remaining);
    }
    get_billing_profile_data(&u_s_profile.billing, entry_no, 0);
    //if(((FiFO_LiFO==0) && (entry_no == (msg_info.total_entries - 1))) || ((FiFO_LiFO!=0) && (entry_no == 0)))
    
    if(entry_no == 0)
    {
      u_s_profile.billing.u32_epoch = g_RTC_time.epoch;
      u_s_profile.billing.u32_tamper_count = get_tamper_counts();
      u_s_profile.billing.u32_Pwr_on_duration = (get_cumu_power_on_time() - u_s_profile.billing.u32_Pwr_on_duration);
    }
    Epoch_To_DLMS_Time(u_s_profile.billing.u32_epoch, buf);
    return len;
  
  //  // Billing profile MD dateTime
  case ITEM_TAG_DATETIME_MD_KW_BI:       Epoch_To_DLMS_Time(u_s_profile.billing.u32_MD_KW_Epoch,buf); return len;
  case ITEM_TAG_DATETIME_MD_KW_TZ1_BI:   Epoch_To_DLMS_Time(u_s_profile.billing.u32_MD_KW_TZ_Epoch[0],buf); return len;
  case ITEM_TAG_DATETIME_MD_KW_TZ2_BI:   Epoch_To_DLMS_Time(u_s_profile.billing.u32_MD_KW_TZ_Epoch[1],buf); return len;
  case ITEM_TAG_DATETIME_MD_KW_TZ3_BI:   Epoch_To_DLMS_Time(u_s_profile.billing.u32_MD_KW_TZ_Epoch[2],buf); return len;
  case ITEM_TAG_DATETIME_MD_KW_TZ4_BI:   Epoch_To_DLMS_Time(u_s_profile.billing.u32_MD_KW_TZ_Epoch[3],buf); return len;
  case ITEM_TAG_DATETIME_MD_KW_TZ5_BI:   Epoch_To_DLMS_Time(u_s_profile.billing.u32_MD_KW_TZ_Epoch[4],buf); return len;
  case ITEM_TAG_DATETIME_MD_KW_TZ6_BI:   Epoch_To_DLMS_Time(u_s_profile.billing.u32_MD_KW_TZ_Epoch[5],buf); return len;
  case ITEM_TAG_DATETIME_MD_KW_TZ7_BI:   Epoch_To_DLMS_Time(u_s_profile.billing.u32_MD_KW_TZ_Epoch[6],buf); return len;
  case ITEM_TAG_DATETIME_MD_KW_TZ8_BI:   Epoch_To_DLMS_Time(u_s_profile.billing.u32_MD_KW_TZ_Epoch[7],buf); return len;
  case ITEM_TAG_DATETIME_MD_KVA_BI:      Epoch_To_DLMS_Time(u_s_profile.billing.u32_MD_KVA_Epoch,buf); return len;
  case ITEM_TAG_DATETIME_MD_KVA_TZ1_BI:  Epoch_To_DLMS_Time(u_s_profile.billing.u32_MD_KVA_TZ_Epoch[0],buf); return len;
  case ITEM_TAG_DATETIME_MD_KVA_TZ2_BI:  Epoch_To_DLMS_Time(u_s_profile.billing.u32_MD_KVA_TZ_Epoch[1],buf); return len;
  case ITEM_TAG_DATETIME_MD_KVA_TZ3_BI:  Epoch_To_DLMS_Time(u_s_profile.billing.u32_MD_KVA_TZ_Epoch[2],buf); return len;
  case ITEM_TAG_DATETIME_MD_KVA_TZ4_BI:  Epoch_To_DLMS_Time(u_s_profile.billing.u32_MD_KVA_TZ_Epoch[3],buf); return len;
  case ITEM_TAG_DATETIME_MD_KVA_TZ5_BI:  Epoch_To_DLMS_Time(u_s_profile.billing.u32_MD_KVA_TZ_Epoch[4],buf); return len;
  case ITEM_TAG_DATETIME_MD_KVA_TZ6_BI:  Epoch_To_DLMS_Time(u_s_profile.billing.u32_MD_KVA_TZ_Epoch[5],buf); return len;
  case ITEM_TAG_DATETIME_MD_KVA_TZ7_BI:  Epoch_To_DLMS_Time(u_s_profile.billing.u32_MD_KVA_TZ_Epoch[6],buf); return len;
  case ITEM_TAG_DATETIME_MD_KVA_TZ8_BI:  Epoch_To_DLMS_Time(u_s_profile.billing.u32_MD_KVA_TZ_Epoch[7],buf); return len;
  default:  len = 0;
  }
  return 0;
}

int64_t get_numeric_item(int item)
{
  union
  {
    int64_t val;
    float32_t   f_val;
  }un;

  un.f_val = 0;
  switch (item)
  {
    //Instant profile
    #ifdef SINGLE_PHASE_METER
    case ITEM_TAG_PHASE_CURRENT:          un.f_val = get_signed_ph_current();break;
    case ITEM_TAG_NEUTRAL_CURRENT:        un.f_val = get_signed_neu_current();break;
    case ITEM_TAG_VRMS:                   un.f_val = get_ph_voltage();break;
    case ITEM_TAG_PF:                     un.f_val = get_signed_pf();break;
    #endif
    
    #ifdef  WHOLE_CURRENT_METER
    case ITEM_TAG_IR:                     un.val = (is_rev(0) > 0) ? get_signed_ph_current(0) : -get_signed_ph_current(0);break;
    case ITEM_TAG_IY:                     un.val = (is_rev(1) > 0) ? get_signed_ph_current(1) : -get_signed_ph_current(1);break;
    case ITEM_TAG_IB:                     un.val = (is_rev(2) > 0) ? get_signed_ph_current(2) : -get_signed_ph_current(2);break;
    case ITEM_TAG_VR:                     un.val = get_ph_voltage(0);   break;
    case ITEM_TAG_VY:                     un.val = get_ph_voltage(1);   break;
    case ITEM_TAG_VB:                     un.val = get_ph_voltage(2);   break;
    case ITEM_TAG_PFR:                    un.val = get_signed_pf(0);   break;
    case ITEM_TAG_PFY:                    un.val = get_signed_pf(1);   break;
    case ITEM_TAG_PFB:                    un.val = get_signed_pf(2);   break;
    case ITEM_TAG_PF_TOTAL:               un.val = abs(get_signed_pf(4));   break;
    #endif
    case ITEM_TAG_FREQUENCY:              un.f_val = get_freq();   break;
    case ITEM_TAG_KVA_TOTAL:              un.f_val = ((float32_t)get_tot_kva())/1000;   break;
    case ITEM_TAG_KW_TOTAL:               un.f_val = ((float32_t)get_signed_tot_kw())/1000;    break;
    case ITEM_TAG_KVAR_TOTAL:             un.f_val = ((float32_t)get_signed_tot_kvar())/1000;  break;
    
    case ITEM_TAG_CUM_KWH_TOTAL:          un.f_val = ((float32_t)get_energy((uint8_t)Active_Imp))/100000;    break;
    case ITEM_TAG_CUM_KWH_EXPO_TOTAL:     un.f_val = ((float32_t)get_energy((uint8_t)Active_Exp))/100000;    break;
    case ITEM_TAG_CUM_KVAR_Q1_TOTAL:      un.f_val = ((float32_t)get_energy((uint8_t)Reactive_Ind_Imp))/100000;    break;
    case ITEM_TAG_CUM_KVAR_Q2_TOTAL:      un.f_val = ((float32_t)get_energy((uint8_t)Reactive_Cap_Exp))/100000;    break;
    case ITEM_TAG_CUM_KVAR_Q3_TOTAL:      un.f_val = ((float32_t)get_energy((uint8_t)Reactive_Ind_Exp))/100000;    break;
    case ITEM_TAG_CUM_KVAR_Q4_TOTAL:      un.f_val = ((float32_t)get_energy((uint8_t)Reactive_Cap_Imp))/100000;    break;
    case ITEM_TAG_CUM_KVAH_TOTAL:         un.f_val = ((float32_t)get_energy((uint8_t)Apparent_Imp))/100000;    break;
    case ITEM_TAG_CUM_KVAH_EXPO_TOTAL:    un.f_val = ((float32_t)get_energy((uint8_t)Apparent_Exp))/100000;    break;
    
    case ITEM_TAG_NUM_POWER_OFFS:         un.val = from_eeprom(POWER_FAIL_CNT_LOC,POWER_FAIL_CNT_SIZE);    break;
    case ITEM_TAG_CUM_POWER_OFF_DURATION: un.val = get_cumu_power_off_time()/60;    break;
    
    case ITEM_TAG_CUM_POWER_ON_DURATION:  un.val = get_cumu_power_on_time()/60;   break;
    case ITEM_TAG_DEVICE_TEMPERATURE:     un.f_val = 2700;     break;//TODO: Rakesh (Tsensor not is current hw)actual value
    
    case ITEM_TAG_CUM_TAMPER_COUNT:       un.val = get_tamper_counts();   break;
    case ITEM_TAG_CUM_MD_RESET_COUNT:     un.val = get_cumulative_bill_count();     break;
    case ITEM_TAG_CUM_PROGRAMMING_COUNT:  un.val = from_eeprom(TOTAL_PROGRAMMING_COUNT_LOC, TOTAL_PROGRAMMING_COUNT_SIZE);  break;
    case ITEM_TAG_KW_MAX_DEMAND:          un.f_val = ((float32_t)get_mdkw_value(0))/100000;      break;
    case ITEM_TAG_KVA_MAX_DEMAND:         un.f_val = ((float32_t)get_mdkva_value(0))/100000;        break;
    
    case ITEM_TAG_LOAD_STATUS:            un.val = get_relay_output_state();   break;
    case ITEM_TAG_LOAD_LIMIT_VAL:         un.f_val = get_over_load_value();   break;
    
    //Name plate
    case ITEM_TAG_METER_TYPE:             un.val = Meter_Type; break;
    case ITEM_TAG_MANFACT_YEAR:           un.val = Year_of_Manufacture; break;
    case ITEM_TAG_CT_RATIO:               un.val = Internal_CT_Ratio;   break;
    case ITEM_TAG_PT_RATIO:               un.val = Internal_PT_Ratio;   break;
    case ITEM_TAG_METERT_CONSTANT:        un.val = Meter_Constant;   break;
    
    //Get Daily Load Profile Values
    case ITEM_TAG_CUM_KWH_TOTAL_DL_LP:         un.f_val = ((float32_t)u_s_profile.daily_load.u64_energy_val[0])/100000; break;
    case ITEM_TAG_CUM_KVAH_TOTAL_DL_LP:        un.f_val = ((float32_t)u_s_profile.daily_load.u64_energy_val[1])/100000; break;
    case ITEM_TAG_CUM_KWH_EXPO_TOTAL_DL_LP:    un.f_val = ((float32_t)u_s_profile.daily_load.u64_energy_val[2])/100000; break;
    case ITEM_TAG_CUM_KVAH_EXPO_TOTAL_DL_LP:   un.f_val = ((float32_t)u_s_profile.daily_load.u64_energy_val[3])/100000; break;
    case ITEM_TAG_MD_KW_DL_LP:                 un.f_val = ((float32_t)u_s_profile.daily_load.u32_mdKw)/100000; break;
    case ITEM_TAG_MD_KVA_DL_LP:                un.f_val = ((float32_t)u_s_profile.daily_load.u32_mdKva)/100000; break;
    
    //Get Load Profile Values
#ifdef SINGLE_PHASE_METER
    case ITEM_TAG_I_PHASE_LP:                  un.f_val = ((float32_t)u_s_profile.block_load.u16_current_phase)/100; break;
    case ITEM_TAG_I_NEUTRAL_LP:                un.f_val = ((float32_t)u_s_profile.block_load.u16_current_neutral)/100; break;
    case ITEM_TAG_V_LP:                        un.f_val = ((float32_t)u_s_profile.block_load.u16_volt)/100; break;
    case ITEM_TAG_CUM_KWH_TOTAL_IMPORT_LP:     un.f_val = ((float32_t)u_s_profile.block_load.u32_block_energy_val[LP_Active_Imp])/100000; break;
    case ITEM_TAG_CUM_KVAH_TOTAL_IMPORT_LP:    un.f_val = ((float32_t)u_s_profile.block_load.u32_block_energy_val[LP_Apparent_Imp])/100000; break;
    case ITEM_TAG_CUM_KWH_TOTAL_EXPORT_LP:     un.f_val = ((float32_t)u_s_profile.block_load.u32_block_energy_val[LP_Active_Exp])/100000; break;
    case ITEM_TAG_CUM_KVAH_TOTAL_EXPORT_LP:    un.f_val = ((float32_t)u_s_profile.block_load.u32_block_energy_val[LP_Apparent_Exp])/100000; break;
//    case ITEM_TAG_PF_LP: val=u_s_profile.block_load.u8_pf; break;
#else
    case ITEM_TAG_IR_LP:  un.val=u_s_profile.block_load.u32_current[0]; break;
    case ITEM_TAG_IY_LP:  un.val=u_s_profile.block_load.u32_current[1]; break;
    case ITEM_TAG_IB_LP:  un.val=u_s_profile.block_load.u32_current[2]; break;
    case ITEM_TAG_VR_LP:  un.val=u_s_profile.block_load.u16_volt[0]; break;
    case ITEM_TAG_VY_LP:  un.val=u_s_profile.block_load.u16_volt[1]; break;
    case ITEM_TAG_VB_LP:  un.val=u_s_profile.block_load.u16_volt[2]; break;
    case ITEM_TAG_PFR_LP: un.val=u_s_profile.block_load.u8_pf[0]; break;
    case ITEM_TAG_PFY_LP: un.val=u_s_profile.block_load.u8_pf[1]; break;
    case ITEM_TAG_PFB_LP: un.val=u_s_profile.block_load.u8_pf[2]; break;
    case ITEM_TAG_PAVG_LP:un.val=u_s_profile.block_load.u8_avg_pf; break;
    case ITEM_TAG_CUM_KWH_TOTAL_IMPORT_LP:  un.val=u_s_profile.block_load.u32_block_energy_val[Active_Imp]; break;
    case ITEM_TAG_CUM_KVAH_TOTAL_IMPORT_LP: un.val=u_s_profile.block_load.u32_block_energy_val[Apparent_Imp]; break;
    case ITEM_TAG_CUM_KWH_TOTAL_EXPORT_LP:  un.val=u_s_profile.block_load.u32_block_energy_val[Active_Exp]; break;
    case ITEM_TAG_CUM_KVAH_TOTAL_EXPORT_LP: un.val=u_s_profile.block_load.u32_block_energy_val[Apparent_Exp]; break;
    case ITEM_TAG_CUM_KVAR_Q1_TOTAL_LP:     un.val=u_s_profile.block_load.u32_block_energy_val[Reactive_Ind_Imp]; break;
    case ITEM_TAG_CUM_KVAR_Q2_TOTAL_LP:     un.val=u_s_profile.block_load.u32_block_energy_val[Reactive_Cap_Exp]; break;
    case ITEM_TAG_CUM_KVAR_Q3_TOTAL_LP:     un.val=u_s_profile.block_load.u32_block_energy_val[Reactive_Ind_Exp]; break;
    case ITEM_TAG_CUM_KVAR_Q4_TOTAL_LP:     un.val=u_s_profile.block_load.u32_block_energy_val[Reactive_Cap_Imp]; break;
#endif
    //    case ITEM_TAG_THD_IR_LP:  val=st_load_profile.thd_current[0]; break;
    //    case ITEM_TAG_THD_IY_LP:  val=st_load_profile.thd_current[1]; break;
    //    case ITEM_TAG_THD_IB_LP:  val=st_load_profile.thd_current[2]; break;
    //    case ITEM_TAG_THD_VR_LP:  val=st_load_profile.thd_volt[0]; break;
    //    case ITEM_TAG_THD_VY_LP:  val=st_load_profile.thd_volt[1]; break;
    //    case ITEM_TAG_THD_VB_LP:  val=st_load_profile.thd_volt[2]; break;
    //    //case ITEM_TAG_KW_TOTAL_LP: val=stLoad_Profile.tot_kw; break;
    //    //
    
    // Get Billing Profile Values
    case ITEM_TAG_TAMPER_COUNT_BI:      un.val = u_s_profile.billing.u32_tamper_count;                    break;
    case ITEM_TAG_SPF_BI:               un.f_val = ((float)u_s_profile.billing.u8_Sys_Power_Factor)/100;                 break;
    case ITEM_TAG_KWH_BI:               un.f_val = ((float)u_s_profile.billing.u64_Cumm_Energy_KWh)/100000;                 break;
    case ITEM_TAG_KWH_TZ1_BI:           un.f_val = ((float)u_s_profile.billing.u64_Cumm_Energy_KWh_TZ[0])/100000;           break;
    case ITEM_TAG_KWH_TZ2_BI:           un.f_val = ((float)u_s_profile.billing.u64_Cumm_Energy_KWh_TZ[1])/100000;           break;
    case ITEM_TAG_KWH_TZ3_BI:           un.f_val = ((float)u_s_profile.billing.u64_Cumm_Energy_KWh_TZ[2])/100000;           break;
    case ITEM_TAG_KWH_TZ4_BI:           un.f_val = ((float)u_s_profile.billing.u64_Cumm_Energy_KWh_TZ[3])/100000;           break;
    case ITEM_TAG_KWH_TZ5_BI:           un.f_val = ((float)u_s_profile.billing.u64_Cumm_Energy_KWh_TZ[4])/100000;           break;
    case ITEM_TAG_KWH_TZ6_BI:           un.f_val = ((float)u_s_profile.billing.u64_Cumm_Energy_KWh_TZ[5])/100000;           break;
    case ITEM_TAG_KWH_TZ7_BI:           un.f_val = ((float)u_s_profile.billing.u64_Cumm_Energy_KWh_TZ[6])/100000;           break;
    case ITEM_TAG_KWH_TZ8_BI:           un.f_val = ((float)u_s_profile.billing.u64_Cumm_Energy_KWh_TZ[7])/100000;           break;
    case ITEM_TAG_KVAH_BI:              un.f_val = ((float)u_s_profile.billing.u64_Cumm_Energy_KVAh)/100000;                break;
    case ITEM_TAG_KVAH_TZ1_BI:          un.f_val = ((float)u_s_profile.billing.u64_Cumm_Energy_KVAh_TZ[0])/100000;          break;
    case ITEM_TAG_KVAH_TZ2_BI:          un.f_val = ((float)u_s_profile.billing.u64_Cumm_Energy_KVAh_TZ[1])/100000;          break;
    case ITEM_TAG_KVAH_TZ3_BI:          un.f_val = ((float)u_s_profile.billing.u64_Cumm_Energy_KVAh_TZ[2])/100000;          break;
    case ITEM_TAG_KVAH_TZ4_BI:          un.f_val = ((float)u_s_profile.billing.u64_Cumm_Energy_KVAh_TZ[3])/100000;          break;
    case ITEM_TAG_KVAH_TZ5_BI:          un.f_val = ((float)u_s_profile.billing.u64_Cumm_Energy_KVAh_TZ[4])/100000;          break;
    case ITEM_TAG_KVAH_TZ6_BI:          un.f_val = ((float)u_s_profile.billing.u64_Cumm_Energy_KVAh_TZ[5])/100000;          break;
    case ITEM_TAG_KVAH_TZ7_BI:          un.f_val = ((float)u_s_profile.billing.u64_Cumm_Energy_KVAh_TZ[6])/100000;          break;
    case ITEM_TAG_KVAH_TZ8_BI:          un.f_val = ((float)u_s_profile.billing.u64_Cumm_Energy_KVAh_TZ[7])/100000;          break;
    case ITEM_TAG_MD_KW_BI:             un.f_val = ((float)u_s_profile.billing.u32_MD_KW)/100000;                           break;
    case ITEM_TAG_MD_KW_TZ1_BI:         un.f_val = ((float)u_s_profile.billing.u32_MD_KW_TZ[0])/100000;                     break;
    case ITEM_TAG_MD_KW_TZ2_BI:         un.f_val = ((float)u_s_profile.billing.u32_MD_KW_TZ[1])/100000;                     break;
    case ITEM_TAG_MD_KW_TZ3_BI:         un.f_val = ((float)u_s_profile.billing.u32_MD_KW_TZ[2])/100000;                     break;
    case ITEM_TAG_MD_KW_TZ4_BI:         un.f_val = ((float)u_s_profile.billing.u32_MD_KW_TZ[3])/100000;                     break;
    case ITEM_TAG_MD_KW_TZ5_BI:         un.f_val = ((float)u_s_profile.billing.u32_MD_KW_TZ[4])/100000;                     break;
    case ITEM_TAG_MD_KW_TZ6_BI:         un.f_val = ((float)u_s_profile.billing.u32_MD_KW_TZ[5])/100000;                     break;
    case ITEM_TAG_MD_KW_TZ7_BI:         un.f_val = ((float)u_s_profile.billing.u32_MD_KW_TZ[6])/100000;                     break;
    case ITEM_TAG_MD_KW_TZ8_BI:         un.f_val = ((float)u_s_profile.billing.u32_MD_KW_TZ[7])/100000;                     break;
    case ITEM_TAG_MD_KVA_BI:            un.f_val = ((float)u_s_profile.billing.u32_MD_KVA)/100000;                          break;
    case ITEM_TAG_MD_KVA_TZ1_BI:        un.f_val = ((float)u_s_profile.billing.u32_MD_KVA_TZ[0])/100000;                    break;
    case ITEM_TAG_MD_KVA_TZ2_BI:        un.f_val = ((float)u_s_profile.billing.u32_MD_KVA_TZ[1])/100000;                    break;
    case ITEM_TAG_MD_KVA_TZ3_BI:        un.f_val = ((float)u_s_profile.billing.u32_MD_KVA_TZ[2])/100000;                    break;
    case ITEM_TAG_MD_KVA_TZ4_BI:        un.f_val = ((float)u_s_profile.billing.u32_MD_KVA_TZ[3])/100000;                    break;
    case ITEM_TAG_MD_KVA_TZ5_BI:        un.f_val = ((float)u_s_profile.billing.u32_MD_KVA_TZ[4])/100000;                    break;
    case ITEM_TAG_MD_KVA_TZ6_BI:        un.f_val = ((float)u_s_profile.billing.u32_MD_KVA_TZ[5])/100000;                    break;
    case ITEM_TAG_MD_KVA_TZ7_BI:        un.f_val = ((float)u_s_profile.billing.u32_MD_KVA_TZ[6])/100000;                    break;
    case ITEM_TAG_MD_KVA_TZ8_BI:        un.f_val = ((float)u_s_profile.billing.u32_MD_KVA_TZ[7])/100000;                    break;
    case ITEM_TAG_POWER_ON_DURATION_BI: un.val = u_s_profile.billing.u32_Pwr_on_duration/60;                break;
    case ITEM_TAG_KVARH_KWH_EXPO_BI:    un.f_val = ((float)u_s_profile.billing.u64_Cumm_Energy_KWh_expo)/100000;            break;
    case ITEM_TAG_KVARH_KVAH_EXPO_BI:   un.f_val = ((float)u_s_profile.billing.u64_Cumm_Energy_KVAh_expo)/100000;           break;
    case ITEM_TAG_KVARH_Q1_BI:          un.f_val = ((float)u_s_profile.billing.u64_Cumm_Energy_KVarh_Q1)/100000;            break;
    case ITEM_TAG_KVARH_Q2_BI:          un.f_val = ((float)u_s_profile.billing.u64_Cumm_Energy_KVarh_Q2)/100000;            break;
    case ITEM_TAG_KVARH_Q3_BI:          un.f_val = ((float)u_s_profile.billing.u64_Cumm_Energy_KVarh_Q3)/100000;            break;
    case ITEM_TAG_KVARH_Q4_BI:          un.f_val = ((float)u_s_profile.billing.u64_Cumm_Energy_KVarh_Q4)/100000;            break;
    
    // Get Tamper Profile Values
    case ITEM_TAG_ID_TMPR:                  un.val = u_s_profile.event.u16_Tamper_ID;                                                                         break;
    #ifdef SINGLE_PHASE_METER
    
    case ITEM_TAG_I_MAX_TMPR:               un.f_val = (u_s_profile.event.u32_current_phase > u_s_profile.event.u32_current_neutral) ? ((float)u_s_profile.event.u32_current_phase)/100 : ((float)u_s_profile.event.u32_current_neutral)/100;                                                                   break;
    case ITEM_TAG_V_TMPR:                   un.f_val = ((float)u_s_profile.event.u16_volt)/100;                                                                              break;
    case ITEM_TAG_PF_TMPR:                  un.f_val = ((float)u_s_profile.event.u8_pf)/100;                                                                                           break;
    #else
    case ITEM_TAG_IR_TMPR:                  un.val = u_s_profile.event.u32_current[0];                                                                        break;
    case ITEM_TAG_IY_TMPR:                  un.val = u_s_profile.event.u32_current[1];                                                                        break;
    case ITEM_TAG_IB_TMPR:                  un.val = u_s_profile.event.u32_current[2];                                                                        break;
    case ITEM_TAG_VR_TMPR:                  un.val = u_s_profile.event.u16_volt[0];                                                                           break;
    case ITEM_TAG_VY_TMPR:                  un.val = u_s_profile.event.u16_volt[1];                                                                           break;
    case ITEM_TAG_VB_TMPR:                  un.val = u_s_profile.event.u16_volt[2];                                                                           break;
    case ITEM_TAG_PFR_TMPR:                 un.val = ((int8_t)u_s_profile.event.u8_pf[0] < 0)?-(int8_t)u_s_profile.event.u8_pf[0]:u_s_profile.event.u8_pf[0]; break;
    case ITEM_TAG_PFY_TMPR:                 un.val = ((int8_t)u_s_profile.event.u8_pf[1] < 0)?-(int8_t)u_s_profile.event.u8_pf[1]:u_s_profile.event.u8_pf[1]; break;
    case ITEM_TAG_PFB_TMPR:                 un.val = ((int8_t)u_s_profile.event.u8_pf[2] < 0)?-(int8_t)u_s_profile.event.u8_pf[2]:u_s_profile.event.u8_pf[2]; break;
    #endif
    case ITEM_TAG_CUM_KWH_TOTAL_TMPR:       un.f_val = ((float)u_s_profile.event.u64_energy_val[0])/100000;                                                                     break;
    case ITEM_TAG_CUM_KVAR_LAG_TOTAL_TMPR:  un.f_val = ((float)u_s_profile.event.u64_energy_val[1])/100000;                                                                     break;
    case ITEM_TAG_CUM_KVAR_LEAD_TOTAL_TMPR: un.f_val = ((float)u_s_profile.event.u64_energy_val[2])/100000;                                                                     break;
    case ITEM_TAG_CUM_KVAH_TOTAL_TMPR:      un.f_val = ((float)u_s_profile.event.u64_energy_val[3])/100000;                                                                     break;
    case ITEM_TAG_KW_TOTAL_TMPR:            un.val = u_s_profile.event.u32_tot_kw;                                                                            break;
    case ITEM_TAG_CUMMULATIVE_TAMPER_VAL_TMPR:   un.val = u_s_profile.event.u32_tamper_count;                                                                 break;
    case ITEM_TAG_CUMMULATIVE_KWH_EXPO_VAL_TMPR: un.f_val = ((float)u_s_profile.event.u64_kwh_expo)/100000;                                                                     break;
    
    default:  un.val = 0;
    //
  }

  return un.val;
}

void svr_Key_Update(dlmsSettings* settings)
{
  bb_reset(&settings->cipher.systemTitle);
  bb_reset(&settings->cipher.authenticationKey);
  bb_reset(&settings->cipher.blockCipherKey);

  settings->authentication = DLMS_AUTHENTICATION_NONE;
  settings->cipher.security = DLMS_SECURITY_NONE;
  bb_set(&settings->cipher.systemTitle, &server_system_title[2], server_system_title[1]);

  if (settings->clientAddress == 0x20)//MR
  {//TODO: Rakesh (later sept 2022)factory_settings
    settings->authentication = DLMS_AUTHENTICATION_LOW;
#ifdef SECURITY_ENABLE
    settings->cipher.security = DLMS_SECURITY_ENCRYPTION;
#else
    settings->cipher.security = DLMS_SECURITY_NONE;
#endif
    settings->cipher.frameCounter = &settings->cipher.frameCounter_MR;
  }
  else if (settings->clientAddress == 0x30)//US
  {
    settings->authentication = DLMS_AUTHENTICATION_HIGH;
#ifdef SECURITY_ENABLE
    settings->cipher.security = DLMS_SECURITY_AUTHENTICATION_ENCRYPTION;
#else
    settings->cipher.security = DLMS_SECURITY_NONE;
#endif
    settings->cipher.frameCounter = &settings->cipher.frameCounter_US;
  }
  else if (settings->clientAddress == 0x40)//PUSH
  {
    settings->authentication = DLMS_AUTHENTICATION_NONE;
#ifdef SECURITY_ENABLE
    settings->cipher.security = DLMS_SECURITY_ENCRYPTION;
#else
    settings->cipher.security = DLMS_SECURITY_NONE;
#endif
    settings->cipher.frameCounter = &settings->cipher.frameCounter_PH;
  }
  else if (settings->clientAddress == 0x50)//FIRMWARE
  {
    settings->authentication = DLMS_AUTHENTICATION_HIGH;
#ifdef SECURITY_ENABLE
    settings->cipher.security = DLMS_SECURITY_AUTHENTICATION_ENCRYPTION;
#else
    settings->cipher.security = DLMS_SECURITY_NONE;
#endif
    settings->cipher.frameCounter = &settings->cipher.frameCounter_FW;
  }
  else if (settings->clientAddress == 0x60)//IHD
  {
    settings->authentication = DLMS_AUTHENTICATION_NONE;
#ifdef SECURITY_ENABLE
    settings->cipher.security = DLMS_SECURITY_ENCRYPTION;
#else
    settings->cipher.security = DLMS_SECURITY_NONE;
#endif
    settings->cipher.frameCounter = &settings->cipher.frameCounter_IHD;
  }
  bb_set(&settings->cipher.blockCipherKey, US_Security_Keys[UNICAST_KEY], 16);
  bb_set(&settings->cipher.authenticationKey, US_Security_Keys[AUTHENTICATION_KEY], 16);
}

///////////////////////////////////////////////////////////////////////////////////////
void addFloat32(
    float value, 
    uint8_t* data,
    uint16_t *src_data_len)
{
    uint16_t len = *src_data_len;
    typedef union
    {
        float value;
        uint8_t b[sizeof(float)];
    } HELPER;

    HELPER tmp;
    tmp.value = value;
    data[len++] = TAG_FLOAT32;
    data[len++] = tmp.b[3];
    data[len++] = tmp.b[2];
    data[len++] = tmp.b[1];
    data[len++] = tmp.b[0];

    *src_data_len = len;
}

void Fill_Data_Buffer_LE(int64_t val, uint16_t len,uint8_t *data,uint16_t *response_len)
{
  uint8_t i;
  *response_len +=len;
  for(i = 0; i < len; i++)
  {
    data[*response_len - 1] = (val & 0xFF);
    *response_len -= 1;
    val >>= 8;
  }
  *response_len +=len;
}
void check_push_events(uint32_t epoch)
{
  check_push_time(&st_Instant_push_time, epoch);
  if(st_Instant_push_time.is_push_action && (st_Instant_push_time.last_epoch_push_time < epoch))
  {
    st_Instant_push_time.last_epoch_push_time = epoch;
    st_Instant_push_time.is_push_action = 0;
    st_tamper.Trigger_Byte |= _BV(INSTANT_PUSH_BIT);
  }
}
/////////////////////////////////// Objects defination start //////////////////////////
//clock
uint8_t clock_status = 0;
static const int8_t clock_dst_deviation = 0;
static const int8_t clock_dst_enabled = 0;
static const uint8_t clock_base = 1;
uint8_t dummy_time[14] = { 13,12,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0 };
uint8_t set_rtc_data;

//methods
void adjust_to_quarter(uint8_t* data, uint16_t data_len, uint8_t* response, uint16_t* response_len)
{
}
/*- End of function --------------------------------------------------------*/

void adjust_to_measuring_period(uint8_t* data, uint16_t data_len, uint8_t* response, uint16_t* response_len)
{
}
/*- End of function --------------------------------------------------------*/

void adjust_to_minute(uint8_t* data, uint16_t data_len, uint8_t* response, uint16_t* response_len)
{
}
/*- End of function --------------------------------------------------------*/

void adjust_to_preset_time(uint8_t* data, uint16_t data_len, uint8_t* response, uint16_t* response_len)
{
}
/*- End of function --------------------------------------------------------*/

void preset_adjusting_time(uint8_t* data, uint16_t data_len, uint8_t* response, uint16_t* response_len)
{
}
/*- End of function --------------------------------------------------------*/

void shift_time(uint8_t* data, uint16_t data_len, uint8_t* response, uint16_t* response_len)
{
}
/*- End of function --------------------------------------------------------*/
static const struct method_desc_s clock_methods[] =
{
  {1, ACCESS_PC___MR___US__, adjust_to_quarter},
  {2, ACCESS_PC___MR___US__, adjust_to_measuring_period},
  {3, ACCESS_PC___MR___US__, adjust_to_minute},
  {4, ACCESS_PC___MR___US__, adjust_to_preset_time},
  {5, ACCESS_PC___MR___US__, preset_adjusting_time},
  {6, ACCESS_PC___MR___US__, shift_time},
};

//attributes
void exchange_date_and_time(void* data, int direction)
{
  uint8_t* data_ptr = (uint8_t*)data;
  st_RTC_time RTC_time;
  
  if(direction == ATTR_WRITE)
  {
    RTC_time.dateTime = DLMS_Time_To_StTime(&data_ptr[1]);
    set_RTC_dateTime(&RTC_time.dateTime);
    get_RTC_dateTime_with_epoch(&RTC_time);
    store_event_data(TRANSACT_EVENT, 151, RTC_time.epoch);
    reset_timeSyncRTC();
  }
  else
  {
    Epoch_To_DLMS_Time_With_RTC_Status(g_RTC_time.epoch, &Data_Buffer[2], &g_RTC_time);
    Data_Buffer[0] = 0x0D;
    Data_Buffer[1] = 0x0C;
  }
}

void clock_time_zone_func(void* data, int direction)
{
  uint8_t* data_ptr = (uint8_t*)data;

  if (direction == ATTR_WRITE)
  {
    tmp_int16 = (*(uint8_t*)data_ptr << 8) + *((uint8_t*)data_ptr + 1);
    to_eeprom(CLOCK_TIME_ZONE_LOC, tmp_int16,CLOCK_TIME_ZONE_SIZE);
  }
  else
  {
    tmp_int16 = from_eeprom(CLOCK_TIME_ZONE_LOC,CLOCK_TIME_ZONE_SIZE);
  }
}

void clock_status_func(void* data, int direction)
{
  tmp_uint8 = (g_RTC_time.battery_status * 16) + g_RTC_time.rtc_status;
}

static const struct attribute_desc_s clock_attrs[] =
{
  {1, ACCESS_PCR__MRR__USR__PHR__FWR_, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL},
  {2, ACCESS_PCR__MRR__USRW_PHR__FWR_, TAG_OCTET_STRING,    (void*)Data_Buffer, exchange_date_and_time},
  {3, ACCESS_PCR__MRR__USR__PHR__FWR_, TAG_INT16,           (void*)&tmp_int16, clock_time_zone_func},
  {4, ACCESS_PCR__MRR__USR__PHR__FWR_, TAG_UINT8,           (void*)&tmp_uint8, clock_status_func},
  {5, ACCESS_PCR__MRR__USR__PHR__FWR_, TAG_OCTET_STRING,    (void*)dummy_time, NULL},
  {6, ACCESS_PCR__MRR__USR__PHR__FWR_, TAG_OCTET_STRING,    (void*)dummy_time, NULL},
  {7, ACCESS_PCR__MRR__USR__PHR__FWR_, TAG_INT8,            (void*)&clock_dst_deviation, NULL},
  {8, ACCESS_PCR__MRR__USR__PHR__FWR_, TAG_BOOLEAN,         (void*)&clock_dst_enabled, NULL},
  {9, ACCESS_PCR__MRR__USR__PHR__FWR_, TAG_ENUM,            (void*)&clock_base, NULL},
};
//clock end
//Manufacturer Name
const uint8_t Manufacturer_Name[14] = { 13,12,'S','I','N','H','A','L',' ','U','D','Y','O','G' };
static const struct attribute_desc_s Obj_Manufacturer_Name[] =
{
  { 1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL },
  { 2, ACCESS_PCR__MRR__USR_, TAG_VISIBLE_STRING,    (void*)Manufacturer_Name, NULL },
};
//
// Meter Number
uint8_t Meter_Sr_No[METER_SERIAL_NUMBER_SIZE];
static const struct attribute_desc_s Obj_Meter_Sr_No[] =
{
  {1, ACCESS_PCR__MRR__USR__PHR__FWR_, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL},
  {2, ACCESS_PCR__MRR__USR__PHR__FWR_, TAG_VISIBLE_STRING,  (void*)Meter_Sr_No, NULL},
};
//
//Logical Name
void logical_name_func(void* data, int direction)
{
  uint8_t* data_ptr = (uint8_t*)data;
  uint8_t i;
  if (direction == ATTR_READ)
  {
    data_ptr[0] = 17;
    data_ptr[1] = 16;
    memcpy(&data_ptr[2], "CRY", 3);
    for(i = 0; i < 16-(3 + Meter_Sr_No[1]); i++)
    {
      memcpy(&data_ptr[5+i], "0", 1);
    }
    memcpy(&data_ptr[5 + i], &Meter_Sr_No[2], Meter_Sr_No[1]);
  }
}

static const struct attribute_desc_s obj_logical_name[] =
{
  {1, ACCESS_PCR__MRR__USR__PHR__FWR_, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL},
  {2, ACCESS_PCR__MRR__USR__PHR__FWR_, TAG_VISIBLE_STRING,  (void*)Data_Buffer, logical_name_func},
};

//Device ID
uint8_t get_device_id(uint8_t* buff)
{
    memcpy(buff, "CRY", 3);
    memcpy(&buff[3], &Meter_Sr_No[2], Meter_Sr_No[1]);
    return 3 + Meter_Sr_No[1];
}

void Device_id_func(void* data, int direction)
{
    uint8_t* data_ptr = (uint8_t*)data;
    uint8_t i;
    if (direction == ATTR_READ)
    {
        data_ptr[1] = get_device_id(&data_ptr[2]);
        data_ptr[0] = data_ptr[1] + 1;
    }
}
static const struct attribute_desc_s obj_device_id[] =
{
  {1, ACCESS_PCR__MRR__USR__PHR__FWR_, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL},
  {2, ACCESS_PCR__MRR__USR__PHR__FWR_, TAG_VISIBLE_STRING,  (void*)Data_Buffer, Device_id_func},
};

//
//Firmware version
void meter_ver_no_func(void* data, int direction)
{
  uint8_t* data_ptr = (uint8_t*)data;
  
  data_ptr[0] = strlen(METER_VERSION_NUMBER) + 1;
  data_ptr[1] = strlen(METER_VERSION_NUMBER);
  memcpy(&data_ptr[2], METER_VERSION_NUMBER, strlen(METER_VERSION_NUMBER));
}
static const struct attribute_desc_s Obj_FW_Version_No[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,      (void*)object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_VISIBLE_STRING,    (void*)Data_Buffer, meter_ver_no_func},
};
//Intenal Firmware version
void meter_internal_ver_no_func(void* data, int direction)
{
  uint8_t* data_ptr = (uint8_t*)data;
  
  data_ptr[0] = strlen(FACTORY_MANUFACTURER_INTERNAL_VERSION_ID) + 1;
  data_ptr[1] = strlen(FACTORY_MANUFACTURER_INTERNAL_VERSION_ID);
  memcpy(&data_ptr[2], FACTORY_MANUFACTURER_INTERNAL_VERSION_ID, strlen(FACTORY_MANUFACTURER_INTERNAL_VERSION_ID));
}
static const struct attribute_desc_s Obj_Internal_FW_Version_No[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,      (void*)object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_VISIBLE_STRING,    (void*)Data_Buffer, meter_internal_ver_no_func},
};
//
// Meter type
const uint8_t Meter_Type = METER_TYPE;
static const struct attribute_desc_s Obj_Meter_Type[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_UINT8,           (void*)&Meter_Type, NULL},
};
//
// CT ratio
static const struct attribute_desc_s Obj_Internal_CT_Ratio[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_UINT8,           (void*)&Internal_CT_Ratio, NULL},
};
//
// PT ratio
static const struct attribute_desc_s Obj_Internal_PT_Ratio[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_UINT8,          (void*)&Internal_PT_Ratio, NULL},
};
//
// Manufacturing Year
uint16_t Year_of_Manufacture = 2023;
static const struct attribute_desc_s Obj_Year_of_Manufacture[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_UINT16,          (void*)&Year_of_Manufacture, NULL},
};
//
////NIC_Serial
//uint8_t NIC_Serial[10] = {9,8, '0', '0', '0', '0', '0', '0', '0', '0'};
//static const struct attribute_desc_s Obj_NIC_Serial[] =
//{
//  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
//  {2, ACCESS_PC___MRR__USR_, TAG_VISIBLE_STRING,  (void *) NIC_Serial, NULL},
//};

//static const struct attribute_desc_s Obj_SIM_number[] =
//{
//  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL},
//  {2, ACCESS_PC___MRR__USR_, TAG_VISIBLE_STRING,  (void*)g_st_NIC_info.SIM_Number, NULL},
//};

//TODO: rakesh sept 2022 if possible iterate from factory settings.
#ifdef SINGLE_PHASE_METER
const uint8_t Voltage_rating[] = { 7, 6, '2', '4', '0', '.', '0', 'V' };
#endif
#ifdef WHOLE_CURRENT_METER
const uint8_t Voltage_rating[] = { 7, 6, '2', '4', '0', '.', '0', 'V' };
#endif
#ifdef HTCT_METER
const uint8_t Voltage_rating[] = { 6, 5, '6', '3', '.', '5', 'V' };
#else
  #ifdef LTCT_METER
  const uint8_t Voltage_rating[] = { 7, 6, '2', '4', '0', '.', '0', 'V' };
  #endif
#endif
static const struct attribute_desc_s Obj_Voltage_rating[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_VISIBLE_STRING,  (void*)Voltage_rating, NULL},
};

static const struct attribute_desc_s Obj_Meter_Constant[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_UINT16,          (void*)&Meter_Constant, NULL},
};
//
// Meter catogory
#ifdef LTCT_METER
  const uint8_t Meter_category[4] = { 3,2, 'D','3' };
#endif
#ifdef WHOLE_CURRENT_METER
  const uint8_t Meter_category[4] = { 3,2, 'D','2' };
#endif
#ifdef SINGLE_PHASE_METER
  const uint8_t Meter_category[4] = { 3,2, 'D','1' };
#endif
static const struct attribute_desc_s Obj_Meter_category[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_VISIBLE_STRING,  (void*)Meter_category, NULL},
};
//
//current rating
#ifdef LTCT_METER
const uint8_t Current_rating[10] = { 9,8,'(','5','-','1','0',')',' ','A' };
#endif
#ifdef WHOLE_CURRENT_METER
const uint8_t Current_rating[11] = { 10,9,'(','1','0','-','6','0',')',' ','A' };
#endif
#ifdef SINGLE_PHASE_METER
  #ifdef METER_RATING_5_30
    const uint8_t Current_rating[10] = { 9,8,'(','5','-','3','0',')',' ','A' };
  #elif METER_RATING_10_60
    const uint8_t Current_rating[11] = { 10,9,'(','1','0','-','6','0',')',' ','A' };
  #endif
#endif
static const struct attribute_desc_s Obj_Current_rating[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_VISIBLE_STRING,  (void*)Current_rating, NULL},
};

//load disconnect interval
void disConn_Time_Interval_Function(void *data, int direction)
{
  if(direction==ATTR_WRITE)
  {
    tmp_uint32 = (((unsigned char*)data)[0]);
    tmp_uint32 <<=8;
    tmp_uint32 |= (((unsigned char*)data)[1]);
    tmp_uint32 <<=8;
    tmp_uint32 |= (((unsigned char*)data)[2]);
    tmp_uint32 <<=8;
    tmp_uint32 |= (((unsigned char*)data)[3]);

    set_relay_normal_disconnect_time(tmp_uint32);
  }
  else
  {
    tmp_uint32 = (uint32_t)get_relay_normal_disconnect_time();
  }
}

static const struct attribute_desc_s Obj_Ctrl_disConn_Time_Interval[] =
{
    {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
    {2, ACCESS_PC___MRR__USRW, TAG_UINT32,          (void *) &tmp_uint32, disConn_Time_Interval_Function}
};

//load connect retry time
void Conn_Time_Interval_Function(void *data, int direction)
{
  if(direction==ATTR_WRITE)
  {
    tmp_uint32 = (((unsigned char*)data)[0]);
    tmp_uint32 <<=8;
    tmp_uint32 |= (((unsigned char*)data)[1]);
    tmp_uint32 <<=8;
    tmp_uint32 |= (((unsigned char*)data)[2]);
    tmp_uint32 <<=8;
    tmp_uint32 |= (((unsigned char*)data)[3]);
    
    set_relay_load_check_time((uint16_t)tmp_uint32);
  }
  else
  {
    tmp_uint32 = (uint32_t)get_relay_load_check_time();
  }
}

static const struct attribute_desc_s Obj_Ctrl_Conn_Time_Interval[] =
{
    {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
    {2, ACCESS_PC___MRR__USRW, TAG_UINT32,          (void *) &tmp_uint32, Conn_Time_Interval_Function}
};

//Lock load connect time interval
void Conn_Lockout_Time_Function(void *data, int direction)
{
  if(direction==ATTR_WRITE)
  {
    tmp_uint32 = (((unsigned char*)data)[0]);
    tmp_uint32 <<=8;
    tmp_uint32 |= (((unsigned char*)data)[1]);
    tmp_uint32 <<=8;
    tmp_uint32 |= (((unsigned char*)data)[2]);
    tmp_uint32 <<=8;
    tmp_uint32 |= (((unsigned char*)data)[3]);
    
    set_relay_lockout_disconnect_time((uint16_t)tmp_uint32);
  }
  else
  {
    tmp_uint32 = (uint32_t)get_relay_lockout_disconnect_time();
  }
}

static const struct attribute_desc_s Obj_Ctrl_Conn_Lockout_Time[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USRW, TAG_UINT32,          (void *) &tmp_uint32, Conn_Lockout_Time_Function}
};

//No of time to repeat before lockout
void Conn_Time_Repeat_Function(void *data, int direction)
{
  if(direction==ATTR_WRITE)
  {
    tmp_uint8 = (((unsigned char*)data)[0]);
    
    set_relay_reconnect_count(tmp_uint8);
  }
  else
  {
    tmp_uint8 = get_relay_reconnect_count();
  }
}

static const struct attribute_desc_s Obj_Ctrl_Conn_Time_Repeat[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USRW, TAG_UINT8,           (void *) &tmp_uint8, Conn_Time_Repeat_Function}
};

//Time within which tamper to occur
void Tamper_Occ_Time_Function(void *data, int direction)
{
  uint32_t tmp_data;

  if(direction==ATTR_WRITE)
  {
    tmp_data = (((unsigned char*)data)[0]);
    tmp_data <<=8;
    tmp_data |= (((unsigned char*)data)[1]);
    tmp_data <<=8;
    tmp_data |= (((unsigned char*)data)[2]);
    tmp_data <<=8;
    tmp_data |= (((unsigned char*)data)[3]);

    if((tmp_data>=15)&&(tmp_data<=1800))
    {
      to_eeprom(TAMPER_OCC_TIME_LOC,tmp_data, TAMPER_OCC_TIME_SIZE);
      set_tamper_occurance_time((uint16_t)tmp_data);
    }
  }
  else
  {
    tmp_uint32 = (uint32_t)from_eeprom(TAMPER_OCC_TIME_LOC, 2);
  }
}

static const struct attribute_desc_s Obj_Tamper_Occ_Time[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USRW, TAG_UINT32,          (void *) &tmp_uint32, Tamper_Occ_Time_Function}
};
//
//Time within which tamper to restore
void Tamper_Res_Time_Function(void *data, int direction)
{
  uint32_t tmp_data;

  if(direction==ATTR_WRITE)
  {
    tmp_data = (((unsigned char*)data)[0]);
    tmp_data <<=8;
    tmp_data |= (((unsigned char*)data)[1]);
    tmp_data <<=8;
    tmp_data |= (((unsigned char*)data)[2]);
    tmp_data <<=8;
    tmp_data |= (((unsigned char*)data)[3]);

    if((tmp_data>=15)&&(tmp_data<=1800))
    {
      to_eeprom(TAMPER_RES_TIME_LOC,tmp_data, TAMPER_RES_TIME_SIZE);
      set_tamper_restoration_time((uint16_t)tmp_data);
    }
  }
  else
  {
    tmp_uint32 = (uint32_t)from_eeprom(TAMPER_RES_TIME_LOC, 2);
  }
}

static const struct attribute_desc_s Obj_Tamper_Res_Time[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USRW, TAG_UINT32,          (void *) &tmp_uint32, Tamper_Res_Time_Function}
};

////Memory Flags
//static const struct attribute_desc_s Obj_Energy_Write_issue[] =
//{
//  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
//  {2, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &write_valid_issue, NULL}
//};
//
//static const struct attribute_desc_s Obj_Energy_Read_issue[] =
//{
//  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
//  {2, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &read_valid_issue, NULL}
//};
//
void NIC_Firmware_Version_Function(void *data, int direction)
{
  if(direction == ATTR_READ)
  {
    tmp_uint32 = stRF_params.RF_App_Firmware_Ver;
  }
}
static const struct attribute_desc_s Obj_NIC_Firmware_Version[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &tmp_uint32, NIC_Firmware_Version_Function}
};

void NIC_Stack_Version_Function(void *data, int direction)
{
  uint8_t *data_ptr = (uint8_t*)data;
  if(direction == ATTR_READ)
  {
    tmp_uint32 = 0;
  }
}
static const struct attribute_desc_s Obj_NIC_Stack_Version[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &tmp_uint32, NIC_Stack_Version_Function}
};

void NIC_Hardware_Version_Function(void *data, int direction)
{
  if(direction == ATTR_READ)
  {
    tmp_uint32 = stRF_params.RF_Hardware_Ver;
  }
}
static const struct attribute_desc_s Obj_NIC_Hardware_Version[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &tmp_uint32, NIC_Hardware_Version_Function}
};

void RF_Network_Address_Function(void *data, int direction)
{
  uint32_t tmp_data;

  if(direction==ATTR_WRITE)
  {
    tmp_data = (((unsigned char*)data)[0]);
    tmp_data <<=8;
    tmp_data |= (((unsigned char*)data)[1]);
    tmp_data <<=8;
    tmp_data |= (((unsigned char*)data)[2]);
    tmp_data <<=8;
    tmp_data |= (((unsigned char*)data)[3]);

    if((tmp_data!=g_st_NIC_info.nvm_info.network_address) && (tmp_data < 0xFFFDAA))
    {
      g_st_NIC_info.nvm_info.network_address = tmp_data;
      g_st_NIC_info.nvm_info.crc = R_CRC_Calculate_Fresh((uint8_t*)&g_st_NIC_info.nvm_info, STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_SIZE - 2);
      write_page_eeprom(STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_LOC, (uint8_t*)&g_st_NIC_info.nvm_info, STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_SIZE);
      g_st_NIC_info.NIC_struct_init = 0;
      RF_init_parameters();
    }
  }
  else if(direction==ATTR_READ)
  {
    tmp_uint32 = g_st_NIC_info.nvm_info.network_address;
  }
}
//
static const struct attribute_desc_s Obj_NIC_Network_Address[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USRW, TAG_UINT32,          (void *) &tmp_uint32, RF_Network_Address_Function}
};

void RF_Channel_Number_Function(void *data, int direction)
{
  uint8_t tmp_data;

  if(direction==ATTR_WRITE)
  {
    tmp_data = (((unsigned char*)data)[0]);

    if((tmp_data!=g_st_NIC_info.nvm_info.network_channel) && (tmp_data > 0) && (tmp_data < 9))
    {
      g_st_NIC_info.nvm_info.network_channel = tmp_data;
      g_st_NIC_info.nvm_info.crc = R_CRC_Calculate_Fresh((uint8_t*)&g_st_NIC_info.nvm_info, STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_SIZE - 2);
      write_page_eeprom(STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_LOC, (uint8_t*)&g_st_NIC_info.nvm_info, STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_SIZE);
      g_st_NIC_info.NIC_struct_init = 0;
      RF_init_parameters();
    }
  }
  else if(direction==ATTR_READ)
  {
    tmp_uint8 = g_st_NIC_info.nvm_info.network_channel;
  }
}

static const struct attribute_desc_s Obj_NIC_Channel_Number[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USRW, TAG_UINT8,          (void *) &tmp_uint8, RF_Channel_Number_Function}
};

void RF_Encryption_Key_Function(void *data, int direction)
{
  uint8_t len;
  uint8_t *data_ptr = (uint8_t*)data;
  if(direction == ATTR_WRITE)
  {
    len = (((unsigned char*)data)[0]);

    if(len==16)
    {
      memcpy((uint8_t*)g_st_NIC_info.nvm_info.encryption_key, data_ptr, len + 1);
      
      g_st_NIC_info.nvm_info.crc = R_CRC_Calculate_Fresh((uint8_t*)&g_st_NIC_info.nvm_info, STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_SIZE - 2);
      write_page_eeprom(STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_LOC, (uint8_t*)&g_st_NIC_info.nvm_info, STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_SIZE);
      g_st_NIC_info.NIC_struct_init = 0;
      RF_init_parameters();
    }
  }
  else if(direction == ATTR_READ)
  {
    if(g_st_NIC_info.nvm_info.encryption_key[0] != 0)
    {
      data_ptr[0] = g_st_NIC_info.nvm_info.encryption_key[0] + 1;
      memcpy(&data_ptr[1],g_st_NIC_info.nvm_info.encryption_key,data_ptr[0]);
    }
    else
    {
      data_ptr[0] = 0;
    }
  }
}

static const struct attribute_desc_s Obj_NIC_Encryption_Key[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USRW, TAG_OCTET_STRING,    (void *) Data_Buffer, RF_Encryption_Key_Function}
};

//NIC MAC Address
void NIC_MAC_address_Function(void *data, int direction)
{
  uint8_t *data_ptr = (uint8_t*)data;
  if(direction ==ATTR_READ)
  {
    if(g_st_NIC_info.nvm_info.MAC_address[0] != 0)
    {
      data_ptr[0] = g_st_NIC_info.Detected_MAC_address[1] + 1;
      memcpy(&data_ptr[1],&g_st_NIC_info.Detected_MAC_address[1],data_ptr[0]);
    }
    else
    {
      data_ptr[0] = 0;
    }
  }
}
static const struct attribute_desc_s Obj_NIC_MAC_Address[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_VISIBLE_STRING,  (void *) Data_Buffer, NIC_MAC_address_Function}
};

//SIM number
void NIC_SIM_Number_Function(void *data, int direction)
{
  uint8_t *data_ptr = (uint8_t*)data;
  if(direction == ATTR_READ)
  {
    if(g_st_NIC_info.SIM_Number[0] != 0)
    {
      data_ptr[0] = g_st_NIC_info.SIM_Number[0] + 1;
      memcpy(&data_ptr[1],g_st_NIC_info.SIM_Number,data_ptr[0]);
    }
    else
    {
      data_ptr[0] = 0;
    }
  }
}
static const struct attribute_desc_s Obj_NIC_SIM_Number[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,  (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,  (void *) Data_Buffer, NIC_SIM_Number_Function}
};

//NIC New MAC address
void NIC_New_MAC_address_Function(void *data, int direction)
{
  uint8_t *data_ptr = (uint8_t*)data;
  if(direction ==ATTR_WRITE)
  {
    if(data_ptr[0] < STORAGE_EEPROM_DLMS_NIC_MAC_ADDRESS_SIZE)
    {
      if(data_ptr[0] != 0)
      {
        memcpy(g_st_NIC_info.nvm_info.New_MAC_address,data_ptr,data_ptr[0] + 1);
      }
      else
      {
        memset(g_st_NIC_info.nvm_info.New_MAC_address, 0, STORAGE_EEPROM_DLMS_NIC_MAC_ADDRESS_SIZE);
      }
      g_st_NIC_info.nvm_info.crc = R_CRC_Calculate_Fresh((uint8_t*)&g_st_NIC_info.nvm_info, STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_SIZE - 2);
      write_page_eeprom(STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_LOC, (uint8_t*)&g_st_NIC_info.nvm_info, STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_SIZE);
    }
  }
  else
  {
    if((g_st_NIC_info.nvm_info.New_MAC_address[0] != 0) && (g_st_NIC_info.nvm_info.New_MAC_address[0] < STORAGE_EEPROM_DLMS_NIC_MAC_ADDRESS_SIZE))
    {
      data_ptr[0] = g_st_NIC_info.nvm_info.New_MAC_address[0] + 1;
      memcpy(&data_ptr[1],g_st_NIC_info.nvm_info.New_MAC_address,data_ptr[0]);
    }
    else
    {
      data_ptr[0] = 0;
    }
  }
}
static const struct attribute_desc_s Obj_NIC_New_MAC_Address[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,  (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USRW, TAG_OCTET_STRING,  (void *) Data_Buffer, NIC_New_MAC_address_Function}
};

//NIC APN name
void NIC_APN_Name_Function(void *data, int direction)
{
  uint8_t *data_ptr = (uint8_t*)data;
  if(direction ==ATTR_WRITE)
  {
    if(data_ptr[0] < STORAGE_EEPROM_DLMS_NIC_APN_NAME_SIZE)
    {
      if(data_ptr[0] != 0)
      {
        memcpy(g_st_NIC_info.nvm_info.APN_name,data_ptr,data_ptr[0] + 1);
      }
      else
      {
        g_st_NIC_info.nvm_info.APN_name[0] = 0;
      }
      g_st_NIC_info.nvm_info.crc = R_CRC_Calculate_Fresh((uint8_t*)&g_st_NIC_info.nvm_info, STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_SIZE - 2);
      write_page_eeprom(STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_LOC, (uint8_t*)&g_st_NIC_info.nvm_info, STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_SIZE);
    }
    g_st_NIC_info.NIC_struct_init = 0;
  }
  else
  {
    if(g_st_NIC_info.nvm_info.APN_name[0] != 0)
    {
      data_ptr[0] = g_st_NIC_info.nvm_info.APN_name[0] + 1;
      memcpy(&data_ptr[1],g_st_NIC_info.nvm_info.APN_name,data_ptr[0]);
    }
    else
    {
      data_ptr[0] = 0;
    }
  }
}
static const struct attribute_desc_s Obj_NIC_APN_Name[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USRW, TAG_VISIBLE_STRING,  (void *) Data_Buffer, NIC_APN_Name_Function}
};

//NIC APN username
void NIC_APN_Username_Function(void *data, int direction)
{
  uint8_t *data_ptr = (uint8_t*)data;
  if(direction ==ATTR_WRITE)
  {
    if(data_ptr[0] < STORAGE_EEPROM_DLMS_NIC_APN_USERNAME_SIZE)
    {
      if(data_ptr[0] != 0)
      {
        memcpy(g_st_NIC_info.nvm_info.APN_username,data_ptr,data_ptr[0] + 1);
      }
      else
      {
        g_st_NIC_info.nvm_info.APN_username[0] = 0;
      }
      g_st_NIC_info.nvm_info.crc = R_CRC_Calculate_Fresh((uint8_t*)&g_st_NIC_info.nvm_info, STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_SIZE - 2);
      write_page_eeprom(STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_LOC, (uint8_t*)&g_st_NIC_info.nvm_info, STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_SIZE);
    }
    g_st_NIC_info.NIC_struct_init = 0;
  }
  else
  {
    if(g_st_NIC_info.nvm_info.APN_username[0] != 0)
    {
      data_ptr[0] = g_st_NIC_info.nvm_info.APN_username[0] + 1;
      memcpy(&data_ptr[1],g_st_NIC_info.nvm_info.APN_username,data_ptr[0]);
    }
    else
    {
      data_ptr[0] = 0;
    }
  }
}
static const struct attribute_desc_s Obj_NIC_APN_Username[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USRW, TAG_VISIBLE_STRING,  (void *) Data_Buffer, NIC_APN_Username_Function}
};

//NIC APN password
void NIC_APN_Password_Function(void *data, int direction)
{
  uint8_t *data_ptr = (uint8_t*)data;
  if(direction ==ATTR_WRITE)
  {
    if(data_ptr[0] < STORAGE_EEPROM_DLMS_NIC_APN_PASSWORD_SIZE)
    {
      if(data_ptr[0] != 0)
      {
        memcpy(g_st_NIC_info.nvm_info.APN_password,data_ptr,data_ptr[0] + 1);
      }
      else
      {
        g_st_NIC_info.nvm_info.APN_password[0] = 0;
      }
      g_st_NIC_info.nvm_info.crc = R_CRC_Calculate_Fresh((uint8_t*)&g_st_NIC_info.nvm_info, STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_SIZE - 2);
      write_page_eeprom(STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_LOC, (uint8_t*)&g_st_NIC_info.nvm_info, STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_SIZE);
    }
    g_st_NIC_info.NIC_struct_init = 0;
  }
  else
  {
    if(g_st_NIC_info.nvm_info.APN_password[0] != 0)
    {
      data_ptr[0] = g_st_NIC_info.nvm_info.APN_password[0] + 1;
      memcpy(&data_ptr[1],g_st_NIC_info.nvm_info.APN_password,data_ptr[0]);
    }
    else
    {
      data_ptr[0] = 0;
    }
  }
}
static const struct attribute_desc_s Obj_NIC_APN_Password[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USRW, TAG_VISIBLE_STRING,  (void *) Data_Buffer, NIC_APN_Password_Function}
};

//NIC Server address
void NIC_Server_Address_Function(void *data, int direction)
{
  uint8_t *data_ptr = (uint8_t*)data;
  if(direction ==ATTR_WRITE)
  {
    if(data_ptr[0] < STORAGE_EEPROM_DLMS_NIC_SERVER_ADDRESS_SIZE)
    {
      if(data_ptr[0] != 0)
      {
        memcpy(g_st_NIC_info.nvm_info.server_address,data_ptr,data_ptr[0] + 1);
      }
      else
      {
        g_st_NIC_info.nvm_info.server_address[0] = 0;
      }
      g_st_NIC_info.nvm_info.crc = R_CRC_Calculate_Fresh((uint8_t*)&g_st_NIC_info.nvm_info, STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_SIZE - 2);
      write_page_eeprom(STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_LOC, (uint8_t*)&g_st_NIC_info.nvm_info, STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_SIZE);
    }
    g_st_NIC_info.NIC_struct_init = 0;
  }
  else
  {
    if(g_st_NIC_info.nvm_info.server_address[0] != 0)
    {
      data_ptr[0] = g_st_NIC_info.nvm_info.server_address[0] + 1;
      memcpy(&data_ptr[1],g_st_NIC_info.nvm_info.server_address,data_ptr[0]);
    }
    else
    {
      data_ptr[0] = 0;
    }
  }
}
static const struct attribute_desc_s Obj_NIC_Server_Address[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USRW, TAG_VISIBLE_STRING,  (void *) Data_Buffer, NIC_Server_Address_Function}
};

//NIC Server port
void NIC_Server_Port_Function(void *data, int direction)
{
  uint16_t tmp_data;
  uint8_t *data_ptr = (uint8_t*)data;
  if(direction==ATTR_WRITE)
  {
    tmp_data = data_ptr[0];
    tmp_data <<=8;
    tmp_data |= data_ptr[1];

    if((tmp_data!=g_st_NIC_info.nvm_info.server_port) && (tmp_data > 1000) && (tmp_data < 60000))
    {
      g_st_NIC_info.nvm_info.server_port = tmp_data;
      g_st_NIC_info.nvm_info.crc = R_CRC_Calculate_Fresh((uint8_t*)&g_st_NIC_info.nvm_info, STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_SIZE - 2);
      write_page_eeprom(STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_LOC, (uint8_t*)&g_st_NIC_info.nvm_info, STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_SIZE);
    }
    g_st_NIC_info.NIC_struct_init = 0;
  }
}
static const struct attribute_desc_s Obj_NIC_Server_Port[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USRW, TAG_UINT16,          (void *) &g_st_NIC_info.nvm_info.server_port, NIC_Server_Port_Function}
};

//NIC MQTT username
void NIC_MQTT_Username_Function(void *data, int direction)
{
  uint8_t *data_ptr = (uint8_t*)data;
  if(direction ==ATTR_WRITE)
  {
    if(data_ptr[0] < STORAGE_EEPROM_DLMS_NIC_MQTT_USERNAME_SIZE)
    {
      if(data_ptr[0] != 0)
      {
        memcpy(g_st_NIC_info.nvm_info.MQTT_username,data_ptr,data_ptr[0] + 1);
      }
      else
      {
        g_st_NIC_info.nvm_info.MQTT_username[0] = 0;
      }
      g_st_NIC_info.nvm_info.crc = R_CRC_Calculate_Fresh((uint8_t*)&g_st_NIC_info.nvm_info, STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_SIZE - 2);
      write_page_eeprom(STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_LOC, (uint8_t*)&g_st_NIC_info.nvm_info, STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_SIZE);
    }
    g_st_NIC_info.NIC_struct_init = 0;
  }
  else
  {
    if(g_st_NIC_info.nvm_info.MQTT_username[0] != 0)
    {
      data_ptr[0] = g_st_NIC_info.nvm_info.MQTT_username[0] + 1;
      memcpy(&data_ptr[1],g_st_NIC_info.nvm_info.MQTT_username,data_ptr[0]);
    }
    else
    {
      data_ptr[0] = 0;
    }
  }
}
static const struct attribute_desc_s Obj_NIC_MQTT_Username[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USRW, TAG_VISIBLE_STRING,  (void *) Data_Buffer, NIC_MQTT_Username_Function}
};

//NIC MQTT password
void NIC_MQTT_Password_Function(void *data, int direction)
{
  uint8_t *data_ptr = (uint8_t*)data;
  if(direction ==ATTR_WRITE)
  {
    if(data_ptr[0] < STORAGE_EEPROM_DLMS_NIC_MQTT_PASSWORD_SIZE)
    {
      if(data_ptr[0] != 0)
      {
        memcpy(g_st_NIC_info.nvm_info.MQTT_password,data_ptr,data_ptr[0] + 1);
      }
      else
      {
        g_st_NIC_info.nvm_info.MQTT_password[0] = 0;
      }
      g_st_NIC_info.nvm_info.crc = R_CRC_Calculate_Fresh((uint8_t*)&g_st_NIC_info.nvm_info, STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_SIZE - 2);
      write_page_eeprom(STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_LOC, (uint8_t*)&g_st_NIC_info.nvm_info, STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_SIZE);
    }
    g_st_NIC_info.NIC_struct_init = 0;
  }
  else
  {
    if(g_st_NIC_info.nvm_info.MQTT_password[0] != 0)
    {
      data_ptr[0] = g_st_NIC_info.nvm_info.MQTT_password[0] + 1;
      memcpy(&data_ptr[1],g_st_NIC_info.nvm_info.MQTT_password,data_ptr[0]);
    }
    else
    {
      data_ptr[0] = 0;
    }
  }
}
static const struct attribute_desc_s Obj_NIC_MQTT_Password[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USRW, TAG_VISIBLE_STRING,  (void *) Data_Buffer, NIC_MQTT_Password_Function}
};

//NIC MQTT password
void NIC_Force_Stop_Function(void* data, int direction)
{
    uint8_t* data_ptr = (uint8_t*)data;
    if (direction == ATTR_WRITE)
    {
        tmp_uint8 = *data_ptr;
        if (tmp_uint8)
        {
            to_eeprom(METER_DLMS_RF_COMM_FORCE_STOP_ADDR, RF_COMM_FORCE_STOP_VALUE, METER_DLMS_RF_COMM_FORCE_STOP_SIZE);
        }
        else
        {
            to_eeprom(METER_DLMS_RF_COMM_FORCE_STOP_ADDR, 0, METER_DLMS_RF_COMM_FORCE_STOP_SIZE);
        }
    }
    else
    {
        if (from_eeprom(METER_DLMS_RF_COMM_FORCE_STOP_ADDR, METER_DLMS_RF_COMM_FORCE_STOP_SIZE) == RF_COMM_FORCE_STOP_VALUE)
        {
            tmp_uint8 = 1;
        }
        else
        {
            tmp_uint8 = 0;
        }
    }
}

static const struct attribute_desc_s Obj_NIC_Force_Stop[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USRW, TAG_INT8,            (void*)&tmp_uint8, NIC_Force_Stop_Function}
};

//Demand integration period
void Demand_integration_Period_Function(void *data, int direction)
{
  uint16_t tmp_data;

  if(direction==ATTR_WRITE)
  {
    tmp_data = (((unsigned char*)data)[0]);
    tmp_data <<=8;
    tmp_data |= (((unsigned char*)data)[1]);
//    tmp_data <<=8;
//    tmp_data |= (((unsigned char*)data)[2]);
//    tmp_data <<=8;
//    tmp_data |= (((unsigned char*)data)[3]);

    if((tmp_data==900) || (tmp_data==1800) /*|| (tmp_data==3600)*/)
    {
      set_md_integration_time(g_RTC_time.epoch, (uint16_t)tmp_data);
      store_event_data(TRANSACT_EVENT, 152, g_RTC_time.epoch);
    }
  }
  else
  {
    tmp_uint16 = (uint16_t)get_md_integration_time();
  }
}

static const struct attribute_desc_s Obj_Demand_Integration_Period[] =
{
    {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
    {2, ACCESS_PC___MRR__USRW, TAG_UINT16,          (void *) &tmp_uint16, Demand_integration_Period_Function} //Demand_Integration_Period
};

//load profile capture period
void Capture_Period_LP_Function(void *data, int direction)
{
  uint32_t tmp_data;

  if(direction==ATTR_WRITE)
  {
    tmp_data = (((unsigned char*)data)[0]);
    tmp_data <<=8;
    tmp_data |= (((unsigned char*)data)[1]);
    tmp_data <<=8;
    tmp_data |= (((unsigned char*)data)[2]);
    tmp_data <<=8;
    tmp_data |= (((unsigned char*)data)[3]);

    if((tmp_data==900) || (tmp_data==1800) || (tmp_data==3600))
    {
      set_block_load_capture_period(g_RTC_time.epoch, (uint16_t)tmp_data);
      store_event_data(TRANSACT_EVENT, 153, g_RTC_time.epoch);
    }
  }
  else
  {
      tmp_uint32 = (uint16_t)get_block_load_capture_period();
  }
}
static const struct attribute_desc_s Obj_Profile_Capture_Period[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USRW, TAG_UINT32,          (void *) &tmp_uint32, Capture_Period_LP_Function}
};
//
//Daily load profile capture period
const uint32_t Daily_Load_Profile_Capture_Period = 86400;//24 hours
static const struct attribute_desc_s Obj_Daily_LP_Profile_Capture_Period[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Daily_Load_Profile_Capture_Period, NULL}
};
//
// meter power off duration
void Cum_Power_Off_Count_Func(void *data, int direction)
{
    tmp_uint32 = from_eeprom(POWER_FAIL_CNT_LOC,POWER_FAIL_CNT_SIZE);
}
static const struct attribute_desc_s Obj_Cum_Pow_Off_Count[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &tmp_uint32, Cum_Power_Off_Count_Func}
};
//
// tamper counts
void tamper_count_Function(void *data, int direction)
{
  if(direction==ATTR_READ)
  {
      tmp_uint16 = (uint16_t)get_tamper_counts();
  }
}
static const struct attribute_desc_s Obj_Cum_Tamper_Count[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_UINT16,          (void *) &tmp_uint16, tamper_count_Function}
};
//
// cumulative bill reset count
void md_bill_count_Function(void *data, int direction)
{
  if(direction==ATTR_READ)
  {
      tmp_uint32 = (uint32_t)get_cumulative_bill_count();
  }
}
static const struct attribute_desc_s Obj_Cum_MD_Reset_Count[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &tmp_uint32, md_bill_count_Function}
};
//
// available bill counts
void Bill_Available_Function(void *data, int direction)
{
  tmp_uint8 = get_avaiable_bill_count();
}

static const struct attribute_desc_s Obj_Billing_Profile_Entries_In_Use[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_UINT8,          (void *)  &tmp_uint8, Bill_Available_Function}
};
//
// cumulative programing counts
void get_programming_counts_Function(void *data, int direction)
{
  tmp_uint16 = (uint16_t)get_programming_counts();
}
static const struct attribute_desc_s Obj_Cum_Prog_Count[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_UINT16,          (void *) &tmp_uint16, get_programming_counts_Function}
};

// last voltage related event stored
void get_event_id_last_stored_volt_Function(void *data, int direction)
{
  tmp_uint16 = get_latest_event_id(PROFILE_TYPE_VOLTAGE_RELATED);
}
static const struct attribute_desc_s Obj_event_ID_volt[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_UINT16,          (void *) &tmp_uint16, get_event_id_last_stored_volt_Function}
};

// last current related event stored
void get_event_id_last_stored_current_Function(void *data, int direction)
{
  tmp_uint16 = get_latest_event_id(PROFILE_TYPE_CURRENT_RELATED);
}
static const struct attribute_desc_s Obj_event_ID_current[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_UINT16,          (void *) &tmp_uint16, get_event_id_last_stored_current_Function}
};
//
// last power related event stored
void get_event_id_last_stored_power_Function(void *data, int direction)
{
  tmp_uint16 = get_latest_event_id(PROFILE_TYPE_POWER_RELATED);
}
static const struct attribute_desc_s Obj_event_ID_power[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_UINT16,          (void *) &tmp_uint16, get_event_id_last_stored_power_Function}
};
//
// last transcation related event stored
void get_event_id_last_stored_trans_Function(void *data, int direction)
{
  tmp_uint16 = get_latest_event_id(PROFILE_TYPE_TRANSACTION_RELATED);
}
static const struct attribute_desc_s Obj_event_ID_transcation[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_UINT16,          (void *) &tmp_uint16, get_event_id_last_stored_trans_Function}
};
//
// last other related event stored
void get_event_id_last_stored_other_Function(void *data, int direction)
{
  tmp_uint16 = get_latest_event_id(PROFILE_TYPE_OTHERS);
}
static const struct attribute_desc_s Obj_event_ID_other[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_UINT16,          (void *) &tmp_uint16, get_event_id_last_stored_other_Function}
};
//
// last non rollover related event stored
void get_event_id_last_stored_non_roll_Function(void *data, int direction)
{
  tmp_uint16 = get_latest_event_id(PROFILE_TYPE_NON_ROLLOVER_EVENTS);
}
static const struct attribute_desc_s Obj_event_ID_nonnroll[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_UINT16,          (void *) &tmp_uint16, get_event_id_last_stored_non_roll_Function}
};
//
// last connect disconnect related event stored
void get_event_id_last_stored_con_discon_Function(void *data, int direction)
{
  tmp_uint16 = get_latest_event_id(PROFILE_TYPE_CONTROL_EVENTS);
}
static const struct attribute_desc_s Obj_event_ID_condiscon[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_UINT16,          (void *) &tmp_uint16, get_event_id_last_stored_con_discon_Function}
};

void get_invocation_counter_MR_Function(void *data, int direction)
{
  tmp_uint32 = get_invocation_counter(Type_MR);
}
static const struct attribute_desc_s Obj_invocation_counter_MR[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL},
  {2, ACCESS_PCR__MRR__USR_, TAG_UINT32,          (void*)&tmp_uint32, get_invocation_counter_MR_Function}
};

void get_invocation_counter_US_Function(void *data, int direction)
{
  tmp_uint32 = get_invocation_counter(Type_US);
}
static const struct attribute_desc_s Obj_invocation_counter_US[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL},
  {2, ACCESS_PCR__MRR__USR_, TAG_UINT32,          (void*)&tmp_uint32, get_invocation_counter_US_Function}
};

void get_invocation_counter_PH_Function(void *data, int direction)
{
  tmp_uint32 = get_invocation_counter(Type_PH);
}
static const struct attribute_desc_s Obj_invocation_counter_PH[] =
{
  {1, ACCESS_PCR__MRR__USR__PHR_, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL},
  {2, ACCESS_PCR__MRR__USR__PHR_, TAG_UINT32,          (void*)&tmp_uint32, get_invocation_counter_PH_Function}
};

void get_invocation_counter_FW_Function(void *data, int direction)
{
  tmp_uint32 = get_invocation_counter(Type_FW);
}
static const struct attribute_desc_s Obj_invocation_counter_FW[] =
{
  {1, ACCESS_PCR__MRR__USR__FWR_, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL},
  {2, ACCESS_PCR__MRR__USR__FWR_, TAG_UINT32,          (void*)&tmp_uint32, get_invocation_counter_FW_Function}
};

void get_invocation_counter_IHD_Function(void *data, int direction)
{
  tmp_uint32 = get_invocation_counter(Type_IHD);
}
static const struct attribute_desc_s Obj_invocation_counter_IHD[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL},
  {2, ACCESS_PCR__MRR__USR_, TAG_UINT32,          (void*)&tmp_uint32, get_invocation_counter_IHD_Function}
};
////
//HDLC setup
typedef enum
{
    BAUD_300 = 0,
    BAUD_600 = 1,
    BAUD_1200 = 2,
    BAUD_2400 = 3,
    BAUD_4800 = 4,
    BAUD_9600 = 5,
    BAUD_19200 = 6,
    BAUD_38400 = 7,
    BAUD_57600 = 8,
    BAUD_115200 = 9
} Baud_Rates;

const uint8_t Comm_Speed = BAUD_9600; //BAUD_9600;
const uint8_t Window_Size_Transmit = 1;
const uint8_t Window_Size_Receive = 1;
const uint16_t Max_Info_Field_Len_TX = DLMS_HDLC_INFO_SIZE;
const uint16_t Max_Info_Field_Len_RX = DLMS_HDLC_INFO_SIZE;
const uint16_t Inter_Octet_Timeout = 1000;//3000;
const uint16_t Inactivity_Time_Out = 20;//1000;
uint16_t Device_Physical_Address_Bak;
uint8_t Set_Device_Physical_Address = 0;
void Device_Address_Func(void *data, int direction)
{
  uint16_t tmp_data;

  if(direction==ATTR_WRITE)
  {
    tmp_data = (((unsigned char*)data)[0]);
    tmp_data <<=8;
    tmp_data |= (((unsigned char*)data)[1]);
    if((tmp_data>=0x0010)&&(tmp_data<=0x3FFD))
    {
      Device_Physical_Address_Bak = tmp_data;
      Set_Device_Physical_Address = 1;
    }
  }
}
static const struct attribute_desc_s Obj_HDLC_Setup[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_ENUM,            (void *) &Comm_Speed, NULL},
  {3, ACCESS_PC___MRR__USR_, TAG_UINT8,           (void *) &Window_Size_Transmit, NULL},
  {4, ACCESS_PC___MRR__USR_, TAG_UINT8,           (void *) &Window_Size_Receive, NULL},
  {5, ACCESS_PC___MRR__USR_, TAG_UINT16,          (void *) &Max_Info_Field_Len_TX, NULL},
  {6, ACCESS_PC___MRR__USR_, TAG_UINT16,          (void *) &Max_Info_Field_Len_RX, NULL},
  {7, ACCESS_PC___MRR__USR_, TAG_UINT16,          (void *) &Inter_Octet_Timeout, NULL},
  {8, ACCESS_PC___MRR__USR_, TAG_UINT16,          (void *) &Inactivity_Time_Out, NULL},
  {9, ACCESS_PC___MRR__USR_, TAG_UINT16,          (void *) &Device_Physical_Address, Device_Address_Func},
  };
//
//last Billing date captured
void bill_captured_date_function(void *data, int direction)
{
    ONE_MONTH_ENERGY_DATA_LOG bill_log;
    uint8_t* date_ptr = (uint8_t*)data;
    date_ptr[0] = 13;
    date_ptr[1] = 12;
    get_billing_profile_data(&bill_log, 1, 1);
    Epoch_To_DLMS_Time(bill_log.u32_epoch, &date_ptr[2]);
}
static const struct attribute_desc_s Obj_Last_MD_Rst_DT[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *) Data_Buffer  , bill_captured_date_function},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_NONE, NULL}
};
//
//
////Meter power of duration
void Power_off_duration_func (void *data, int direction)
{
  tmp_uint32 = get_cumu_power_off_time()/60;
}
static const struct attribute_desc_s Obj_Power_Fail_Duration[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_UINT32 ,         (void *) &tmp_uint32  , Power_off_duration_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &time_scaler, NULL}
};
//
#ifdef SINGLE_PHASE_METER
// instant currnet in phase
void phase_irms_func(void *data, int direction)
{
  tmp_float=get_signed_ph_current();
}
static const struct attribute_desc_s Obj_Phase_Current[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, phase_irms_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_I, NULL}
};
//
// instant currnet in R phase
void neutral_irms_func(void *data, int direction)
{
  tmp_float=get_signed_neu_current();
}
static const struct attribute_desc_s Obj_Neutral_Current[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, neutral_irms_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_I, NULL}
};
// instant currnet in R phase
void active_irms_func(void *data, int direction)
{
  tmp_float=get_signed_act_current();
}
static const struct attribute_desc_s Obj_Active_Current[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, active_irms_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_I, NULL}
};
// instant currnet in R phase
void Over_Current_Threshold_Func(void *data, int direction)
{
  union
  {
     uint32_t tmp_uint32;
     float32_t tmp_float;
     
  }un;
  if(direction==ATTR_WRITE)
  {
    un.tmp_uint32 = (((unsigned char*)data)[0]);
    un.tmp_uint32 <<=8;
    un.tmp_uint32 |= (((unsigned char*)data)[1]);
    un.tmp_uint32 <<=8;
    un.tmp_uint32 |= (((unsigned char*)data)[2]);
    un.tmp_uint32 <<=8;
    un.tmp_uint32 |= (((unsigned char*)data)[3]);
    
    set_over_current_value(un.tmp_float);
  }
  else
  {
    tmp_float = get_over_current_value();
  }
}
static const struct attribute_desc_s Obj_Over_Current_Threshold[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USRW, TAG_FLOAT32,         (void *) &tmp_float, Over_Current_Threshold_Func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_I, NULL}
};
// instant voltage
void vrms_reg_func(void *data, int direction)
{
  tmp_float=get_ph_voltage();
}
static const struct attribute_desc_s Obj_VRMS[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, vrms_reg_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_V, NULL}
};
//
// instant PF
void pf_val_func(void *data, int direction)
{
  tmp_float=get_signed_pf();
}
static const struct attribute_desc_s Obj_PF[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, pf_val_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_PF, NULL}
};
//
#else
// instant currnet in R phase
void irms_reg_r_func(void *data, int direction)
{
  tmp_int32=(is_rev(0) > 0) ? get_signed_ph_current(0) : -get_signed_ph_current(0);
}
static const struct attribute_desc_s Obj_I_R[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_INT32,           (void *) &tmp_int32, irms_reg_r_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_I, NULL}
};
//
// instant currnet in Y phase
void irms_reg_y_func(void *data, int direction)
{
  tmp_int32=(is_rev(1) > 0) ? get_signed_ph_current(1) : -get_signed_ph_current(1);
}
static const struct attribute_desc_s Obj_I_Y[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_INT32,           (void *) &tmp_int32, irms_reg_y_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_I, NULL}
};
//
// instant currnet in B phase
void irms_reg_b_func(void *data, int direction)
{
  tmp_int32=(is_rev(2) > 0) ? get_signed_ph_current(2) : -get_signed_ph_current(2);
}
static const struct attribute_desc_s Obj_I_B[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_INT32,           (void *) &tmp_int32, irms_reg_b_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_I, NULL}
};
//
// instant voltage in R phase
void vrms_reg_r_func(void *data, int direction)
{
  tmp_uint16=get_ph_voltage(0);
}
static const struct attribute_desc_s Obj_V_RN[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_UINT16,          (void *) &tmp_uint16, vrms_reg_r_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_V, NULL}
};
//
// instant voltage in Y phase
void vrms_reg_y_func(void *data, int direction)
{
  tmp_uint16=get_ph_voltage(1);
}
static const struct attribute_desc_s Obj_V_YN[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_UINT16,          (void *) &tmp_uint16, vrms_reg_y_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_V, NULL}
};
//
// instant voltage in B phase
void vrms_reg_b_func(void *data, int direction)
{
  tmp_uint16=get_ph_voltage(2);
}
static const struct attribute_desc_s Obj_V_BN[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_UINT16,          (void *) &tmp_uint16, vrms_reg_b_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_V, NULL}
};
//
// PF R phase
void pf_r_val_func(void *data, int direction)
{
  tmp_int8=get_signed_pf(0);
}
static const struct attribute_desc_s Obj_PF_R[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_INT8,            (void *) &tmp_int8, pf_r_val_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_PF, NULL}
};
//
// PF Y phase
void pf_y_val_func(void *data, int direction)
{
  tmp_int8=get_signed_pf(1);
}
static const struct attribute_desc_s Obj_PF_Y[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_INT8,            (void *) &tmp_int8, pf_y_val_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_PF, NULL}
};
//
// PF B phase
void pf_b_val_func(void *data, int direction)
{
  tmp_int8=get_signed_pf(2);
}
static const struct attribute_desc_s Obj_PF_B[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_INT8,            (void *) &tmp_int8, pf_b_val_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_PF, NULL}
};
//
// Avg PF
void pf_avg_val_func(void *data, int direction)
{
  tmp_int8=abs(get_signed_pf(4));
}
static const struct attribute_desc_s Obj_PF_Avg[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_INT8,            (void *) &tmp_int8, pf_avg_val_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_PF, NULL}
};
//
#endif
// Avg PF Bill
void pf_avg_bill_val_func(void *data, int direction)
{
    ONE_MONTH_ENERGY_DATA_LOG bill_log;
    get_billing_profile_data(&bill_log, 0, 0);
    tmp_float = (float)bill_log.u8_Sys_Power_Factor/100.0;
}
static const struct attribute_desc_s Obj_PF_Avg_bill[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, pf_avg_bill_val_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_PF, NULL}
};

void Frequency_val_func(void *data, int direction)
{
  tmp_float = get_freq();
}
static const struct attribute_desc_s Obj_Frequency[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, Frequency_val_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_Frequency, NULL}
};
//
// instant kva
void tot_kva_val_func(void *data, int direction)
{
  tmp_float=get_tot_kva()/1000;
}
static const struct attribute_desc_s Obj_KVA_Total[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, tot_kva_val_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KVA, NULL}
};
//
// instant kw
void tot_kw_val_func(void *data, int direction)
{
  tmp_float=get_signed_tot_kw()/1000;
}
static const struct attribute_desc_s Obj_KW_Total[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, tot_kw_val_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KW, NULL}
};
//
//instant kvar
void tot_kvar_val_func(void *data, int direction)
{
  tmp_float=get_signed_tot_kvar()/1000;
}
static const struct attribute_desc_s Obj_KVAr_Total[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, tot_kvar_val_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KVAr, NULL}
};
//
// cummulative kwh
void cum_kwh_val_func(void *data, int direction)
{
  tmp_float=((float32_t)get_energy((uint8_t)Active_Imp))/100000;
}
static const struct attribute_desc_s Obj_cum_KWh[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, cum_kwh_val_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KWh, NULL}
};
//
// cummulative kvah
void cum_kvah_val_func(void *data, int direction)
{
  tmp_float=((float32_t)get_energy((uint8_t)Apparent_Imp))/100000;
}
static const struct attribute_desc_s Obj_cum_KVAh[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, cum_kvah_val_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KVAh, NULL}
};
//
// cumulative kvarh lag Q1
void cum_kvarh_q1_val_func(void *data, int direction)
{
  tmp_float=((float32_t)get_energy((uint8_t)Reactive_Ind_Imp))/100000;
}
static const struct attribute_desc_s Obj_cum_KVArh_Q1[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, cum_kvarh_q1_val_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KVArh, NULL}
};
//
// cumulative kvarh lead Q2
void cum_kvarh_q2_val_func(void *data, int direction)
{
  tmp_float=((float32_t)get_energy((uint8_t)Reactive_Cap_Exp))/100000;
}
static const struct attribute_desc_s Obj_cum_KVArh_Q2[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, cum_kvarh_q2_val_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KVArh, NULL}
};
//

// cummulative kwh export
void cum_kwh_export_val_func(void *data, int direction)
{
  tmp_float=((float32_t)get_energy((uint8_t)Active_Exp))/100000;
}
static const struct attribute_desc_s Obj_cum_KWh_Expo[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, cum_kwh_export_val_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KWh, NULL}
};
//
// cummulative kvah export
void cum_kvah_export_val_func(void *data, int direction)
{
  tmp_float=((float32_t)get_energy((uint8_t)Apparent_Exp))/100000;
}
static const struct attribute_desc_s Obj_cum_KVAh_Expo[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, cum_kvah_export_val_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KVAh, NULL}
};
//
// cumulative kvarh lead Q3
void cum_kvarh_q3_val_func(void *data, int direction)
{
  tmp_float=((float32_t)get_energy((uint8_t)Reactive_Ind_Exp))/100000;
}
static const struct attribute_desc_s Obj_cum_KVArh_Q3[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, cum_kvarh_q3_val_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KVArh, NULL}
};
//
// cumulative kvarh lag Q4
void cum_kvarh_q4_val_func(void *data, int direction)
{
  tmp_float=((float32_t)get_energy((uint8_t)Reactive_Cap_Imp))/100000;
}
static const struct attribute_desc_s Obj_cum_KVArh_Q4[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, cum_kvarh_q4_val_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KVArh, NULL}
};

#ifdef SINGLE_PHASE_METER
// LP voltage
void LP_volt_func(void *data, int direction)
{
  ONE_BLOCK_ENERGY_DATA_LOG profile_log;
  get_load_profile(&profile_log, 0, e_latest_entry);
  tmp_float = ((float32_t)profile_log.u16_volt)/100;
}
static const struct attribute_desc_s Obj_V_LP[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, LP_volt_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_V, NULL}
};

// LP currnet in phase
void LP_ph_current_func(void *data, int direction)
{
  ONE_BLOCK_ENERGY_DATA_LOG profile_log;
  get_load_profile(&profile_log, 0, e_latest_entry);
  tmp_float = ((float32_t)profile_log.u16_current_phase)/100;
}
static const struct attribute_desc_s Obj_I_Phase_LP[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, LP_ph_current_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_I, NULL}
};

// LP currnet in neutral
void LP_neu_current_func(void *data, int direction)
{
  ONE_BLOCK_ENERGY_DATA_LOG profile_log;
  get_load_profile(&profile_log, 0, e_latest_entry);
  tmp_float = ((float32_t)profile_log.u16_current_neutral)/100;
}
static const struct attribute_desc_s Obj_I_Neutral_LP[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, LP_neu_current_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_I, NULL}
};

//// LP PF
//void LP_pf_val_func(void *data, int direction)
//{
//  ONE_BLOCK_ENERGY_DATA_LOG profile_log;
//  get_load_profile(&profile_log, 0, e_latest_entry);
//  tmp_uint8 = profile_log.u8_pf;
//}
//static const struct attribute_desc_s Obj_PF_LP[] =
//{
//  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
//  {2, ACCESS_PC___MRR__USR_, TAG_INT8,            (void *) &tmp_uint8, LP_pf_val_func},
//  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_PF, NULL}
//};

#else
// LP voltage in R phase
void LP_R_ph_volt_func(void *data, int direction)
{
  ONE_BLOCK_ENERGY_DATA_LOG profile_log;
  get_load_profile(&profile_log, 0, e_latest_entry);
  tmp_uint16 = profile_log.u16_volt[0];
}
static const struct attribute_desc_s Obj_V_RN_LP[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_UINT16,          (void *) &tmp_uint16, LP_R_ph_volt_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_V, NULL}
};

// LP voltage in Y phase
void LP_Y_ph_volt_func(void *data, int direction)
{
  ONE_BLOCK_ENERGY_DATA_LOG profile_log;
  get_load_profile(&profile_log, 0, e_latest_entry);
  tmp_uint16 = profile_log.u16_volt[1];
}
static const struct attribute_desc_s Obj_V_YN_LP[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_UINT16,          (void *) &tmp_uint16, LP_Y_ph_volt_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_V, NULL}
};

// LP voltage in B phase
void LP_B_ph_volt_func(void *data, int direction)
{
  ONE_BLOCK_ENERGY_DATA_LOG profile_log;
  get_load_profile(&profile_log, 0, e_latest_entry);
  tmp_uint16 = profile_log.u16_volt[2];
}
static const struct attribute_desc_s Obj_V_BN_LP[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_UINT16,          (void *) &tmp_uint16, LP_B_ph_volt_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_V, NULL}
};

// LP currnet in R phase
void LP_R_ph_current_func(void *data, int direction)
{
  ONE_BLOCK_ENERGY_DATA_LOG profile_log;
  get_load_profile(&profile_log, 0, e_latest_entry);
  tmp_uint32 = profile_log.u32_current[0];
}
static const struct attribute_desc_s Obj_I_R_LP[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &tmp_uint32, LP_R_ph_current_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_I, NULL}
};

// LP currnet in Y phase
void LP_Y_ph_current_func(void *data, int direction)
{
  ONE_BLOCK_ENERGY_DATA_LOG profile_log;
  get_load_profile(&profile_log, 0, e_latest_entry);
  tmp_uint32 = profile_log.u32_current[1];
}
static const struct attribute_desc_s Obj_I_Y_LP[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &tmp_uint32, LP_Y_ph_current_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_I, NULL}
};
//
// LP currnet in B phase
void LP_B_ph_current_func(void *data, int direction)
{
  ONE_BLOCK_ENERGY_DATA_LOG profile_log;
  get_load_profile(&profile_log, 0, e_latest_entry);
  tmp_uint32 = profile_log.u32_current[2];
}
static const struct attribute_desc_s Obj_I_B_LP[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &tmp_uint32, LP_B_ph_current_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_I, NULL}
};

// LP PF in R phase
void LP_pf_r_val_func(void *data, int direction)
{
  ONE_BLOCK_ENERGY_DATA_LOG profile_log;
  get_load_profile(&profile_log, 0, e_latest_entry);
  tmp_uint8 = profile_log.u8_pf[0];
}
static const struct attribute_desc_s Obj_PF_R_LP[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_INT8,            (void *) &tmp_uint8, LP_pf_r_val_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_PF, NULL}
};

// LP PF in Y phase
void LP_pf_y_val_func(void *data, int direction)
{
  ONE_BLOCK_ENERGY_DATA_LOG profile_log;
  get_load_profile(&profile_log, 0, e_latest_entry);
  tmp_uint8 = profile_log.u8_pf[1];
}
static const struct attribute_desc_s Obj_PF_Y_LP[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_INT8,            (void *) &tmp_uint8, LP_pf_y_val_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_PF, NULL}
};

// LP PF in B phase
void LP_pf_b_val_func(void *data, int direction)
{
  ONE_BLOCK_ENERGY_DATA_LOG profile_log;
  get_load_profile(&profile_log, 0, e_latest_entry);
  tmp_uint8 = profile_log.u8_pf[2];
}
static const struct attribute_desc_s Obj_PF_B_LP[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_INT8,            (void *) &tmp_uint8, LP_pf_b_val_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_PF, NULL}
};

//// LP Avg PF
//void LP_pf_avg_val_func(void *data, int direction)
//{
//  get_lp_data(1);
//  tmp_int8 = st_load_profile.avg_pf;
//}
//static const struct attribute_desc_s Obj_PF_Avg_LP[] =
//{
//  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
//  {2, ACCESS_PC___MRR__USR_, TAG_INT8,            (void *) &tmp_int8, LP_pf_avg_val_func},
//  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_PF, NULL}
//};
//
#endif
// LP kwh
void lp_kwh_val_func(void *data, int direction)
{
  ONE_BLOCK_ENERGY_DATA_LOG profile_log;
  get_load_profile(&profile_log, 0, e_latest_entry);
#ifdef SINGLE_PHASE_METER
  tmp_float = ((float32_t)profile_log.u32_block_energy_val[LP_Active_Imp])/100000;
#else
  tmp_float = ((float32_t)profile_log.u32_block_energy_val[Active_Imp])/100000;
#endif
}
static const struct attribute_desc_s Obj_cum_KWh_LP[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, lp_kwh_val_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KWh, NULL}
};

// LP kvah
void lp_kvah_val_func(void *data, int direction)
{
  ONE_BLOCK_ENERGY_DATA_LOG profile_log;
  get_load_profile(&profile_log, 0, e_latest_entry);
#ifdef SINGLE_PHASE_METER
  tmp_float = ((float32_t)profile_log.u32_block_energy_val[LP_Apparent_Imp])/100000;
#else
  tmp_float = ((float32_t)profile_log.u32_block_energy_val[Apparent_Imp])/100000;
#endif
}
static const struct attribute_desc_s Obj_cum_KVAh_LP[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, lp_kvah_val_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KVAh, NULL}
};
//

//LP kw Expo
void lp_kwh_expo_val_func(void *data, int direction)
{
  ONE_BLOCK_ENERGY_DATA_LOG profile_log;
  get_load_profile(&profile_log, 0, e_latest_entry);
#ifdef SINGLE_PHASE_METER
  tmp_float = ((float32_t)profile_log.u32_block_energy_val[LP_Active_Exp])/100000;
#else
  tmp_float = ((float32_t)profile_log.u32_block_energy_val[Active_Exp])/100000;
#endif
}
static const struct attribute_desc_s Obj_cum_KWh_Expo_LP[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, lp_kwh_expo_val_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KWh, NULL}
};

//LP kva expo
void lp_kvah_expo_val_func(void *data, int direction)
{
  ONE_BLOCK_ENERGY_DATA_LOG profile_log;
  get_load_profile(&profile_log, 0, e_latest_entry);
#ifdef SINGLE_PHASE_METER
  tmp_float = ((float32_t)profile_log.u32_block_energy_val[LP_Apparent_Exp])/100000;
#else
  tmp_float = ((float32_t)profile_log.u32_block_energy_val[Apparent_Exp])/100000;
#endif
}
static const struct attribute_desc_s Obj_cum_KVAh_Expo_LP[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, lp_kvah_expo_val_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KVAh, NULL}
};

#ifdef LTCT_METER
//LP kvarh Q1
void lp_kvarh_q1_val_func(void *data, int direction)
{
  ONE_BLOCK_ENERGY_DATA_LOG profile_log;
  get_load_profile(&profile_log, 0, e_latest_entry);
#ifdef SINGLE_PHASE_METER
  tmp_uint32 = profile_log.u32_block_energy_val[LP_Reactive_Ind_Imp];
#else
  tmp_uint32 = profile_log.u32_block_energy_val[Reactive_Ind_Imp];
#endif
}
static const struct attribute_desc_s Obj_cum_KVArh_Q1_LP[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_INT32,           (void *) &tmp_uint32, lp_kvarh_q1_val_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KVArh, NULL}
};

//LP kvarh Q2
void lp_kvarh_q2_val_func(void *data, int direction)
{
  ONE_BLOCK_ENERGY_DATA_LOG profile_log;
  get_load_profile(&profile_log, 0, e_latest_entry);
#ifdef SINGLE_PHASE_METER
  tmp_uint32 = profile_log.u32_block_energy_val[LP_Reactive_Cap_Exp];
#else
  tmp_uint32 = profile_log.u32_block_energy_val[Reactive_Cap_Exp];
#endif
}
static const struct attribute_desc_s Obj_cum_KVArh_Q2_LP[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_INT32,           (void *) &tmp_uint32, lp_kvarh_q2_val_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KVArh, NULL}
};

//LP kvarh Q3
void lp_kvarh_q3_val_func(void *data, int direction)
{
  ONE_BLOCK_ENERGY_DATA_LOG profile_log;
  get_load_profile(&profile_log, 0, e_latest_entry);
#ifdef SINGLE_PHASE_METER
  tmp_uint32 = profile_log.u32_block_energy_val[LP_Reactive_Ind_Exp];
#else
  tmp_uint32 = profile_log.u32_block_energy_val[Reactive_Ind_Exp];
#endif
}
static const struct attribute_desc_s Obj_cum_KVArh_Q3_LP[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_INT32,           (void *) &tmp_uint32, lp_kvarh_q3_val_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KVArh, NULL}
};

//LP kvarh Q4
void lp_kvarh_q4_val_func(void *data, int direction)
{
  ONE_BLOCK_ENERGY_DATA_LOG profile_log;
  get_load_profile(&profile_log, 0, e_latest_entry);
#ifdef SINGLE_PHASE_METER
  tmp_uint32 = profile_log.u32_block_energy_val[LP_Reactive_Cap_Imp];
#else
  tmp_uint32 = profile_log.u32_block_energy_val[Reactive_Cap_Imp];
#endif
}
static const struct attribute_desc_s Obj_cum_KVArh_Q4_LP[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_INT32,           (void *) &tmp_uint32, lp_kvarh_q4_val_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KVArh, NULL}
};
#endif
//TZ1 kwh
void TZ1_cum_KWh_func(void *data, int direction)
{
  tmp_float = ((float32_t)get_kwh_time_zone(1)/100000);
}
static const struct attribute_desc_s Obj_cum_KWh_TZ1[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, TZ1_cum_KWh_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KWh, NULL}
};
//TZ2 kwh
void TZ2_cum_KWh_func(void *data, int direction)
{
  tmp_float = ((float32_t)get_kwh_time_zone(2)/100000);
}
static const struct attribute_desc_s Obj_cum_KWh_TZ2[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, TZ2_cum_KWh_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KWh, NULL}
};
//TZ3 kwh
void TZ3_cum_KWh_func(void *data, int direction)
{
  tmp_float = ((float32_t)get_kwh_time_zone(3)/100000);
}
static const struct attribute_desc_s Obj_cum_KWh_TZ3[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, TZ3_cum_KWh_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KWh, NULL}
};
//TZ4 kwh
void TZ4_cum_KWh_func(void *data, int direction)
{
  tmp_float = ((float32_t)get_kwh_time_zone(4)/100000);
}
static const struct attribute_desc_s Obj_cum_KWh_TZ4[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, TZ4_cum_KWh_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KWh, NULL}
};
//TZ5 kwh
void TZ5_cum_KWh_func(void *data, int direction)
{
  tmp_float = ((float32_t)get_kwh_time_zone(5)/100000);
}
static const struct attribute_desc_s Obj_cum_KWh_TZ5[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, TZ5_cum_KWh_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KWh, NULL}
};
//TZ6 kwh
void TZ6_cum_KWh_func(void *data, int direction)
{
  tmp_float = ((float32_t)get_kwh_time_zone(6)/100000);
}
static const struct attribute_desc_s Obj_cum_KWh_TZ6[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, TZ6_cum_KWh_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KWh, NULL}
};
//TZ7 kwh
void TZ7_cum_KWh_func(void *data, int direction)
{
  tmp_float = ((float32_t)get_kwh_time_zone(7)/100000);
}
static const struct attribute_desc_s Obj_cum_KWh_TZ7[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, TZ7_cum_KWh_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KWh, NULL}
};
//TZ8 kwh
void TZ8_cum_KWh_func(void *data, int direction)
{
  tmp_float = ((float32_t)get_kwh_time_zone(8)/100000);
}
static const struct attribute_desc_s Obj_cum_KWh_TZ8[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, TZ8_cum_KWh_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KWh, NULL}
};

// TZ1 kvah
void TZ1_cum_KVAh_func(void *data, int direction)
{
  tmp_float = ((float32_t)get_kvah_time_zone(1)/100000);
}
static const struct attribute_desc_s Obj_cum_KVAh_TZ1[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, TZ1_cum_KVAh_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KVAh, NULL}
};
// TZ2 kvah
void TZ2_cum_KVAh_func(void *data, int direction)
{
  tmp_float = ((float32_t)get_kvah_time_zone(2)/100000);
}
static const struct attribute_desc_s Obj_cum_KVAh_TZ2[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, TZ2_cum_KVAh_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KVAh, NULL}
};
// TZ3 kvah
void TZ3_cum_KVAh_func(void *data, int direction)
{
  tmp_float = ((float32_t)get_kvah_time_zone(3)/100000);
}
static const struct attribute_desc_s Obj_cum_KVAh_TZ3[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, TZ3_cum_KVAh_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KVAh, NULL}
};
// TZ4 kvah
void TZ4_cum_KVAh_func(void *data, int direction)
{
  tmp_float = ((float32_t)get_kvah_time_zone(4)/100000);
}
static const struct attribute_desc_s Obj_cum_KVAh_TZ4[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, TZ4_cum_KVAh_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KVAh, NULL}
};
// TZ5 kvah
void TZ5_cum_KVAh_func(void *data, int direction)
{
  tmp_float = ((float32_t)get_kvah_time_zone(5)/100000);
}
static const struct attribute_desc_s Obj_cum_KVAh_TZ5[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, TZ5_cum_KVAh_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KVAh, NULL}
};
// TZ6 kvah
void TZ6_cum_KVAh_func(void *data, int direction)
{
  tmp_float = ((float32_t)get_kvah_time_zone(6)/100000);
}
static const struct attribute_desc_s Obj_cum_KVAh_TZ6[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, TZ6_cum_KVAh_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KVAh, NULL}
};
// TZ7 kvah
void TZ7_cum_KVAh_func(void *data, int direction)
{
  tmp_float = ((float32_t)get_kvah_time_zone(7)/100000);
}
static const struct attribute_desc_s Obj_cum_KVAh_TZ7[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, TZ7_cum_KVAh_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KVAh, NULL}
};
// TZ8 kvah
void TZ8_cum_KVAh_func(void *data, int direction)
{
  tmp_float = ((float32_t)get_kvah_time_zone(8)/100000);
}
static const struct attribute_desc_s Obj_cum_KVAh_TZ8[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, TZ8_cum_KVAh_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KVAh, NULL}
};

void bill_power_on_time_func(void *data, int direction)
{
  tmp_uint16 = get_running_bill_power_on_time()/60;
}
static const struct attribute_desc_s Obj_bill_power_on_time[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_INT16,           (void *) &tmp_uint16, bill_power_on_time_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_time_min, NULL}
};

void cumulative_power_on_time_func(void *data, int direction)
{
  tmp_uint32 = get_cumu_power_on_time()/60;
}
static const struct attribute_desc_s Obj_cumulative_power_on_time[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_INT32,           (void *) &tmp_uint32, cumulative_power_on_time_func},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_time_min, NULL}
};

uint32_t device_temperature = 2700;//TODO: Rakesh sept 2022 not in use
static const struct attribute_desc_s Obj_device_temperature[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_INT32,           (void *) &device_temperature, NULL},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_temperature_c, NULL}
};

// Maximum demand KW
void zero_func(void *data, int direction)
{
  Data_Buffer[0] = 0;
}

void get_mdkw_vaule_n_date_function(void *data, int direction)
{
  uint8_t* data_ptr = (uint8_t*)data;
  tmp_float = ((float32_t)get_mdkw_value(0))/100000;
  Epoch_To_DLMS_Time(get_mdkw_time(0), &data_ptr[2]);
  data_ptr[0] = 13;
  data_ptr[1] = 12;
  return;
}
static const struct attribute_desc_s Obj_KW_MAX[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, get_mdkw_vaule_n_date_function},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KW, NULL},
  {4, ACCESS_PC___MRR__USR_, TAG_NULLDATA,        (void *) Data_Buffer, zero_func},
  {5, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *) Data_Buffer, get_mdkw_vaule_n_date_function},
};

// Maximum demand KW TZ1
void get_mdkw_date_TZ1(void *data, int direction)
{
  uint8_t* data_ptr = (uint8_t*)data;
  tmp_float = ((float32_t)get_mdkw_value(1))/100000;
  Epoch_To_DLMS_Time(get_mdkw_time(1), &data_ptr[2]);
  data_ptr[0] = 13;
  data_ptr[1] = 12;
  return;
}
static const struct attribute_desc_s Obj_KW_MAX_TZ1[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, get_mdkw_date_TZ1},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KW, NULL},
  {4, ACCESS_PC___MRR__USR_, TAG_NULLDATA,        (void *) Data_Buffer, zero_func},
  {5, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *) Data_Buffer, get_mdkw_date_TZ1},
};
// Maximum demand KW TZ2
void get_mdkw_date_TZ2(void *data, int direction)
{
  uint8_t* data_ptr = (uint8_t*)data;
  tmp_float = ((float32_t)get_mdkw_value(2))/100000;
  Epoch_To_DLMS_Time(get_mdkw_time(2), &data_ptr[2]);
  data_ptr[0] = 13;
  data_ptr[1] = 12;
  return;
}
static const struct attribute_desc_s Obj_KW_MAX_TZ2[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, get_mdkw_date_TZ2},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KW, NULL},
  {4, ACCESS_PC___MRR__USR_, TAG_NULLDATA,        (void *) Data_Buffer, zero_func},
  {5, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *) Data_Buffer, get_mdkw_date_TZ2},
};
// Maximum demand KW TZ3
void get_mdkw_date_TZ3(void *data, int direction)
{
  uint8_t* data_ptr = (uint8_t*)data;
  tmp_float = ((float32_t)get_mdkw_value(3))/100000;
  Epoch_To_DLMS_Time(get_mdkw_time(3), &data_ptr[2]);
  data_ptr[0] = 13;
  data_ptr[1] = 12;
  return;
}
static const struct attribute_desc_s Obj_KW_MAX_TZ3[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, get_mdkw_date_TZ3},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KW, NULL},
  {4, ACCESS_PC___MRR__USR_, TAG_NULLDATA,        (void *) Data_Buffer, zero_func},
  {5, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *) Data_Buffer, get_mdkw_date_TZ3},
};
// Maximum demand KW TZ4
void get_mdkw_date_TZ4(void *data, int direction)
{
  uint8_t* data_ptr = (uint8_t*)data;
  tmp_float = ((float32_t)get_mdkw_value(4))/100000;
  Epoch_To_DLMS_Time(get_mdkw_time(4), &data_ptr[2]);
  data_ptr[0] = 13;
  data_ptr[1] = 12;
  return;
}
static const struct attribute_desc_s Obj_KW_MAX_TZ4[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, get_mdkw_date_TZ4},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KW, NULL},
  {4, ACCESS_PC___MRR__USR_, TAG_NULLDATA,        (void *) Data_Buffer, zero_func},
  {5, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *) Data_Buffer, get_mdkw_date_TZ4},
};
// Maximum demand KW TZ5
void get_mdkw_date_TZ5(void *data, int direction)
{
  uint8_t* data_ptr = (uint8_t*)data;
  tmp_float = ((float32_t)get_mdkw_value(5))/100000;
  Epoch_To_DLMS_Time(get_mdkw_time(5), &data_ptr[2]);
  data_ptr[0] = 13;
  data_ptr[1] = 12;
  return;
}
static const struct attribute_desc_s Obj_KW_MAX_TZ5[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, get_mdkw_date_TZ5},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KW, NULL},
  {4, ACCESS_PC___MRR__USR_, TAG_NULLDATA,        (void *) Data_Buffer, zero_func},
  {5, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *) Data_Buffer, get_mdkw_date_TZ5},
};
// Maximum demand KW TZ6
void get_mdkw_date_TZ6(void *data, int direction)
{
  uint8_t* data_ptr = (uint8_t*)data;
  tmp_float = ((float32_t)get_mdkw_value(6))/100000;
  Epoch_To_DLMS_Time(get_mdkw_time(6), &data_ptr[2]);
  data_ptr[0] = 13;
  data_ptr[1] = 12;
  return;
}
static const struct attribute_desc_s Obj_KW_MAX_TZ6[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, get_mdkw_date_TZ6},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KW, NULL},
  {4, ACCESS_PC___MRR__USR_, TAG_NULLDATA,        (void *) Data_Buffer, zero_func},
  {5, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *) Data_Buffer, get_mdkw_date_TZ6},
};
// Maximum demand KW TZ7
void get_mdkw_date_TZ7(void *data, int direction)
{
  uint8_t* data_ptr = (uint8_t*)data;
  tmp_float = ((float32_t)get_mdkw_value(7))/100000;
  Epoch_To_DLMS_Time(get_mdkw_time(7), &data_ptr[2]);
  data_ptr[0] = 13;
  data_ptr[1] = 12;
  return;
}
static const struct attribute_desc_s Obj_KW_MAX_TZ7[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, get_mdkw_date_TZ7},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KW, NULL},
  {4, ACCESS_PC___MRR__USR_, TAG_NULLDATA,        (void *) Data_Buffer, zero_func},
  {5, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *) Data_Buffer, get_mdkw_date_TZ7},
};
// Maximum demand KW TZ8
void get_mdkw_date_TZ8(void *data, int direction)
{
  uint8_t* data_ptr = (uint8_t*)data;
  tmp_float = ((float32_t)get_mdkw_value(8))/100000;
  Epoch_To_DLMS_Time(get_mdkw_time(8), &data_ptr[2]);
  data_ptr[0] = 13;
  data_ptr[1] = 12;
  return;
}
static const struct attribute_desc_s Obj_KW_MAX_TZ8[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, get_mdkw_date_TZ8},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KW, NULL},
  {4, ACCESS_PC___MRR__USR_, TAG_NULLDATA,        (void *) Data_Buffer, zero_func},
  {5, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *) Data_Buffer, get_mdkw_date_TZ8},
};

// Maximum demand KVA
void get_mdkva_date(void *data, int direction)
{
  uint8_t* data_ptr = (uint8_t*)data;
  tmp_float = ((float32_t)get_mdkva_value(0))/100000;
  Epoch_To_DLMS_Time(get_mdkva_time(0), &data_ptr[2]);
  data_ptr[0] = 13;
  data_ptr[1] = 12;
  return;
}

static const struct attribute_desc_s Obj_KVA_MAX[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, get_mdkva_date},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KVA, NULL},
  {4, ACCESS_PC___MRR__USR_, TAG_NULLDATA,        (void *) Data_Buffer, zero_func},
  {5, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *) Data_Buffer, get_mdkva_date},
};
// Maximum demand KVA TZ1
void get_mdkva_date_TZ1(void *data, int direction)
{
  uint8_t* data_ptr = (uint8_t*)data;
  tmp_float = ((float32_t)get_mdkva_value(1))/100000;
  Epoch_To_DLMS_Time(get_mdkva_time(1), &data_ptr[2]);
  data_ptr[0] = 13;
  data_ptr[1] = 12;
  return;
}
static const struct attribute_desc_s Obj_KVA_MAX_TZ1[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, get_mdkva_date_TZ1},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KVA, NULL},
  {4, ACCESS_PC___MRR__USR_, TAG_NULLDATA,        (void *) Data_Buffer, zero_func},
  {5, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *) Data_Buffer, get_mdkva_date_TZ1},
};
// Maximum demand KVA TZ2
void get_mdkva_date_TZ2(void *data, int direction)
{
  uint8_t* data_ptr = (uint8_t*)data;
  tmp_float = ((float32_t)get_mdkva_value(2))/100000;
  Epoch_To_DLMS_Time(get_mdkva_time(2), &data_ptr[2]);
  data_ptr[0] = 13;
  data_ptr[1] = 12;
  return;
}
static const struct attribute_desc_s Obj_KVA_MAX_TZ2[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, get_mdkva_date_TZ2},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KVA, NULL},
  {4, ACCESS_PC___MRR__USR_, TAG_NULLDATA,        (void *) Data_Buffer, zero_func},
  {5, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *) Data_Buffer, get_mdkva_date_TZ2},
};
// Maximum demand KVA TZ3
void get_mdkva_date_TZ3(void *data, int direction)
{
  uint8_t* data_ptr = (uint8_t*)data;
  tmp_float = ((float32_t)get_mdkva_value(3))/100000;
  Epoch_To_DLMS_Time(get_mdkva_time(3), &data_ptr[2]);
  data_ptr[0] = 13;
  data_ptr[1] = 12;
  return;
}
static const struct attribute_desc_s Obj_KVA_MAX_TZ3[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, get_mdkva_date_TZ3},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KVA, NULL},
  {4, ACCESS_PC___MRR__USR_, TAG_NULLDATA,        (void *) Data_Buffer, zero_func},
  {5, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *) Data_Buffer, get_mdkva_date_TZ3},
};
// Maximum demand KVA TZ4
void get_mdkva_date_TZ4(void *data, int direction)
{
  uint8_t* data_ptr = (uint8_t*)data;
  tmp_float = ((float32_t)get_mdkva_value(4))/100000;
  Epoch_To_DLMS_Time(get_mdkva_time(4), &data_ptr[2]);
  data_ptr[0] = 13;
  data_ptr[1] = 12;
  return;
}
static const struct attribute_desc_s Obj_KVA_MAX_TZ4[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_uint32, get_mdkva_date_TZ4},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KVA, NULL},
  {4, ACCESS_PC___MRR__USR_, TAG_NULLDATA,        (void *) Data_Buffer, zero_func},
  {5, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *) Data_Buffer, get_mdkva_date_TZ4},
};
// Maximum demand KVA TZ5
void get_mdkva_date_TZ5(void *data, int direction)
{
  uint8_t* data_ptr = (uint8_t*)data;
  tmp_uint32 = ((float32_t)get_mdkva_value(5))/100000;
  Epoch_To_DLMS_Time(get_mdkva_time(5), &data_ptr[2]);
  data_ptr[0] = 13;
  data_ptr[1] = 12;
  return;
}
static const struct attribute_desc_s Obj_KVA_MAX_TZ5[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, get_mdkva_date_TZ5},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KVA, NULL},
  {4, ACCESS_PC___MRR__USR_, TAG_NULLDATA,        (void *) Data_Buffer, zero_func},
  {5, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *) Data_Buffer, get_mdkva_date_TZ5},
};
// Maximum demand KVA TZ6
void get_mdkva_date_TZ6(void *data, int direction)
{
  uint8_t* data_ptr = (uint8_t*)data;
  tmp_float = ((float32_t)get_mdkva_value(6))/100000;
  Epoch_To_DLMS_Time(get_mdkva_time(6), &data_ptr[2]);
  data_ptr[0] = 13;
  data_ptr[1] = 12;
  return;
}
static const struct attribute_desc_s Obj_KVA_MAX_TZ6[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, get_mdkva_date_TZ6},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KVA, NULL},
  {4, ACCESS_PC___MRR__USR_, TAG_NULLDATA,        (void *) Data_Buffer, zero_func},
  {5, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *) Data_Buffer, get_mdkva_date_TZ6},
};
// Maximum demand KVA TZ7
void get_mdkva_date_TZ7(void *data, int direction)
{
  uint8_t* data_ptr = (uint8_t*)data;
  tmp_float = ((float32_t)get_mdkva_value(7))/100000;
  Epoch_To_DLMS_Time(get_mdkva_time(7), &data_ptr[2]);
  data_ptr[0] = 13;
  data_ptr[1] = 12;
  return;
}
static const struct attribute_desc_s Obj_KVA_MAX_TZ7[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, get_mdkva_date_TZ7},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KVA, NULL},
  {4, ACCESS_PC___MRR__USR_, TAG_NULLDATA,        (void *) Data_Buffer, zero_func},
  {5, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *) Data_Buffer, get_mdkva_date_TZ7},
};
// Maximum demand KVA TZ8
void get_mdkva_date_TZ8(void *data, int direction)
{
  uint8_t* data_ptr = (uint8_t*)data;
  tmp_float = ((float32_t)get_mdkva_value(8))/100000;
  Epoch_To_DLMS_Time(get_mdkva_time(8), &data_ptr[2]);
  data_ptr[0] = 13;
  data_ptr[1] = 12;
  return;
}
static const struct attribute_desc_s Obj_KVA_MAX_TZ8[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_FLOAT32,         (void *) &tmp_float, get_mdkva_date_TZ8},
  {3, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) &scalar_unit_KVA, NULL},
  {4, ACCESS_PC___MRR__USR_, TAG_NULLDATA,        (void *) Data_Buffer, zero_func},
  {5, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *) Data_Buffer, get_mdkva_date_TZ8},
};

//Activity calendar
void Calendar_Name_Active_func(void* data, int direction)
{
  uint8_t *data_ptr = (uint8_t*)data;
  
  if(direction==ATTR_READ)
  {
    data_ptr[0] = get_calendar_name_active(&data_ptr[2]) + 1;
    data_ptr[1] = data_ptr[0] - 1;
  }
}

void Calendar_Name_Passive_func(void* data, int direction)
{
  uint8_t *data_ptr = (uint8_t*)data;
  
  if(direction==ATTR_WRITE)
  {
    set_calendar_name_passive(&data_ptr[1], data_ptr[0]);
  }
  else
  {
    data_ptr[0] = get_calendar_name_passive(&data_ptr[2]) + 1;
    data_ptr[1] = data_ptr[0] - 1;
  }
}

void Season_Profile_Active_func(void *data, int direction)
{
  uint8_t* data_ptr = (uint8_t*)data;
  
  if(direction==ATTR_READ)
  {
    get_season_profile_active(data_ptr);
  }
}

void Season_Profile_Passive_func(void *data, int direction)
{
  uint8_t* data_ptr = (uint8_t*)data;
  
  if(direction==ATTR_WRITE)
  {
    if(data_ptr[0] <= MAX_SEASON_PROFILES)
    {
      set_season_profile_passive(&data_ptr[1], data_ptr[0]);
    }
  }
  else
  {
    get_season_profile_passive(data_ptr);
  }
}

void Week_Profile_Table_Active_func(void *data, int direction)
{
  uint8_t* data_ptr = (uint8_t*)data;
  
  if(direction==ATTR_READ)
  {
    get_week_profile_active(data_ptr);
  }
}
    
void Week_Profile_Table_Passive_func(void *data, int direction)
{
  uint8_t* data_ptr = (uint8_t*)data;
  if(direction==ATTR_WRITE)
  {
    if(data_ptr[0] <= MAX_WEEK_PROFILES)
    {
      set_week_profile_passive(&data_ptr[1], data_ptr[0]);
    }
  }
  else
  {
    get_week_profile_passive(data_ptr);
  }
}

void Access_Active_Day_Profile_Table(void *data, int direction)
{
  uint8_t* data_ptr = (uint8_t *)data;
  if(direction == ATTR_READ)
  {
    get_day_profile_active(data_ptr);
  }
}

void Access_Passive_Day_Profile_Table(void *data, int direction)
{
  uint8_t* data_ptr = (uint8_t *)data;

  if(direction == ATTR_WRITE)
  {
    if(data_ptr[0] <= MAX_DAY_PROFILES)
    {
      set_day_profile_passive(&data_ptr[1], data_ptr[0]);
    }
  }
  else
  {
    get_day_profile_passive(data_ptr);
  }
}

void Activate_Passive_Calendar_Time_func(void *data, int direction)
{
  uint8_t* data_ptr = (uint8_t*)data;

  if(direction==ATTR_WRITE)
  {
    if(data_ptr[0] == 0x0c)
    {
      set_calendar_activation_time(DLMS_Time_To_Epoch(&data_ptr[1]));
    }
  }
  else
  {
      data_ptr[0] = 13;
      data_ptr[1] = 12;
      Epoch_To_DLMS_Time(get_calendar_activation_time(), &data_ptr[2]);
  }
}

static const struct attribute_desc_s Obj_Activity_Calendar[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *) Data_Buffer, Calendar_Name_Active_func},
  {3, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *) Data_Buffer, Season_Profile_Active_func},
  {4, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *) Data_Buffer, Week_Profile_Table_Active_func},
  {5, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *) Data_Buffer, Access_Active_Day_Profile_Table},
  {6, ACCESS_PC___MRR__USRW, TAG_OCTET_STRING,    (void *) Data_Buffer, Calendar_Name_Passive_func},
  {7, ACCESS_PC___MRR__USRW, TAG_ARRAY,           (void *) Data_Buffer, Season_Profile_Passive_func},
  {8, ACCESS_PC___MRR__USRW, TAG_ARRAY,           (void *) Data_Buffer, Week_Profile_Table_Passive_func},
  {9, ACCESS_PC___MRR__USRW, TAG_ARRAY,           (void *) Data_Buffer, Access_Passive_Day_Profile_Table},
  {10,ACCESS_PC___MRR__USRW, TAG_OCTET_STRING,    (void *) Data_Buffer, Activate_Passive_Calendar_Time_func}
};

void Obj_Activity_Calendar_Activate(uint8_t *data,uint16_t data_len,uint8_t *response,uint16_t *response_len)
{
    uint8_t temp_buffer[64];
    activate_passive_calendar(temp_buffer);
}

static const struct method_desc_s Obj_Activity_Calendar_Methods[] =
{
  {1, ACCESS_PC___MR___USR_, Obj_Activity_Calendar_Activate}
};

//// billing action schedule
const uint8_t Executed_Billing_Script[] =
{
  11 + 1,
    2,
      TAG_OCTET_STRING, 6, 0, 0, 10, 0, 1, 255,
      TAG_UINT16, INJECT16(1),
};
const uint8_t Billing_Script_Type = 1;
void Update_Billing_Script_Execution_Time_func(void *data, int direction)
{
  unsigned int i;
  uint8_t *data_ptr = (uint8_t*)data;
  uint8_t time_buffer[4];

  if(direction ==ATTR_WRITE)
  {
    time_buffer[0] = data_ptr[14];
    memcpy(&time_buffer[1], &data_ptr[5], 3);
    set_bill_date(&g_RTC_time, time_buffer);
    store_event_data(TRANSACT_EVENT, 154, g_RTC_time.epoch);
  }
  else if(direction == ATTR_READ)
  {
    i = 0;
    get_bill_date(time_buffer);
    Data_Buffer[i++] = 0;
    Data_Buffer[i++] = 1;
    Data_Buffer[i++] = TAG_STRUCTURE;
    Data_Buffer[i++] = 2;
    Data_Buffer[i++] = TAG_OCTET_STRING;
    Data_Buffer[i++] = 4;
    memcpy(&Data_Buffer[i],&time_buffer[1],3);
    i += 3;
    Data_Buffer[i++] = 0;
    Data_Buffer[i++] = TAG_OCTET_STRING;
    Data_Buffer[i++] = 5;
    memset(&Data_Buffer[i],0xFF,5);
    Data_Buffer[i+3] = time_buffer[0];
    i += 5;
    Data_Buffer[0] = i-1;
  }
}
static const struct attribute_desc_s Obj_Billing_Schedule[] =
{
    {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
    {2, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) Executed_Billing_Script, NULL},
    {3, ACCESS_PC___MRR__USR_, TAG_ENUM,            (void *) &Billing_Script_Type, NULL},
    {4, ACCESS_PC___MRR__USRW, TAG_ARRAY,           (void *) Data_Buffer,Update_Billing_Script_Execution_Time_func}
};
//
// Billing script table
const uint8_t Billing_Script[] =
{
  (1 * 26) + 1,
    1,
      TAG_STRUCTURE, 2,
        TAG_UINT16, INJECT16(1),
        TAG_ARRAY, 1,
          TAG_STRUCTURE, 5,
            TAG_ENUM, 2,
            TAG_UINT16, INJECT16(CLASS_ID_PROFILE_GENERIC),
            TAG_OCTET_STRING, 6, 1, 0, 98, 1, 0, 255,
            TAG_INT8, 1,
            TAG_INT8, 0
};
static const struct attribute_desc_s Obj_Billing_Script_Table[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *) Billing_Script, NULL}
};


// Billing script table methods
void Obj_Billing_script_table_execute(uint8_t *data,uint16_t data_len,uint8_t *response,uint16_t *response_len)
{
  uint8_t *temp_data = (uint8_t*)data;
  uint16_t temp_16;
  temp_16 = *temp_data;
  temp_data++;
  temp_16 = (temp_16<<8)|(*temp_data);
  if(temp_16 == 1)
  {
    generate_bill_on_demand(&g_RTC_time);
    store_event_data(TRANSACT_EVENT, 166, g_RTC_time.epoch);
  }
}

static const struct method_desc_s Obj_Billing_script_table_Methods[] =
{
  {1, ACCESS_PC___MR___USR_, Obj_Billing_script_table_execute}
};
//
//
//// sap list
//static const uint8_t sap_list[] =
//{
//  24,
//    1,
//      TAG_STRUCTURE, 2,
//        TAG_UINT16, INJECT16(1),
//        TAG_OCTET_STRING, 16
//};
//void Sap_List_func(void *data, int direction)
//{
//  unsigned char i;
//  for(i=0;i<9;i++)
//    Data_Buffer[i]=sap_list[i];
//  for(i=0;i<16;i++)
//    Data_Buffer[i+9]=LOGICAL_DEVICE_NAME[i+2];
//
//}
//static const struct attribute_desc_s sap_41_attrs[] =
//{
//  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
//  {2, ACCESS_PCR__MRR__USR_, TAG_ARRAY,           (void *) Data_Buffer, Sap_List_func},
//};
////
/////////////////////////////////////////Profiles///////////////////////////////////////////////
const uint8_t Common_Sort_Object[] =
{
  16 + 1,
    4,
      TAG_UINT16, INJECT16(CLASS_ID_CLOCK),
      TAG_OCTET_STRING, 6, 0, 0, 1, 0, 0, 255, // Date & Time
      TAG_INT8, 3,
      TAG_UINT16, INJECT16(0)
};
//
//Instant profile
//uint32_t Cum_Power_Off_Time = 0;
const uint32_t Instant_Profile_Entries_In_Use = 1;
const uint32_t Instant_Profile_Entries = 1;
const uint32_t Instant_Profile_Capture_Period = 0;
const uint8_t Instant_Profile_Sort_Method = 1;
const uint16_t Instant_Profile_Column_Szs[]={16,21,26,31,34,37,40,43,46,49,52,55,60,65,70,75,80,85,90,95,100,105,110,115,120,125,130,135,149,154,168,173,187,189,194};
const uint8_t Instant_Profile_Buffer_Template[] =
{
//    TAG_ARRAY, 1,
#ifdef SINGLE_PHASE_METER
    STUFF_DATA | TAG_STRUCTURE, 22,
        STUFF_DATA | TAG_OCTET_STRING, 12, ITEM_TAG_CURRENT_DATETIME, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_VRMS),                       /* V */
        STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_PHASE_CURRENT),              /* Iph */
        STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_NEUTRAL_CURRENT),            /* Ineu */
        STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_PF),                         /* PF */
        STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_FREQUENCY),                  /* Frequency */
        STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_KVA_TOTAL),                  /* kVA */
        STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_KW_TOTAL),                   /* kW */
        STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_CUM_KWH_TOTAL),              /* Cumulative kWh */
        STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_CUM_KVAH_TOTAL),             /* Cumulative kVAh */
        STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_KW_MAX_DEMAND),              /* Max demand kW */
        STUFF_DATA | TAG_OCTET_STRING, 12, ITEM_TAG_CUM_LAST_MD_KW_DATETIME, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,/* Max demand kW time */
        STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_KVA_MAX_DEMAND),             /* Max demand kVA */
        STUFF_DATA | TAG_OCTET_STRING, 12, ITEM_TAG_CUM_LAST_MD_KVA_DATETIME, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,/* Max demand kVA time */
        STUFF_DATA | TAG_UINT32, INJECT32(ITEM_TAG_CUM_POWER_ON_DURATION),    /* Cumulative power on duration */
        STUFF_DATA | TAG_UINT16,  INJECT16(ITEM_TAG_CUM_TAMPER_COUNT),             /* Cumulative tamper count */
        STUFF_DATA | TAG_UINT32, INJECT32(ITEM_TAG_CUM_MD_RESET_COUNT),       /* Cumulative MD resets count */
        STUFF_DATA | TAG_UINT16,  INJECT16(ITEM_TAG_CUM_PROGRAMMING_COUNT),        /* Cumulative programming count */
        STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_CUM_KWH_EXPO_TOTAL),         /* Cumulative kWh */
        STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_CUM_KVAH_EXPO_TOTAL),        /* Cumulative kVAh */
        STUFF_DATA | TAG_BOOLEAN, ITEM_TAG_LOAD_STATUS,                       /* Load status */
        STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_LOAD_LIMIT_VAL),             /* Cumulative kWh */ 
#else

#ifdef WHOLE_CURRENT_METER
        STUFF_DATA | TAG_STRUCTURE, 35,
#endif
#ifdef LTCT_METER
        STUFF_DATA | TAG_STRUCTURE, 33,
#endif
        STUFF_DATA | TAG_OCTET_STRING, 12, ITEM_TAG_CURRENT_DATETIME, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        STUFF_DATA | TAG_INT32, INJECT32(ITEM_TAG_IR),                        /* Ir */
        STUFF_DATA | TAG_INT32, INJECT32(ITEM_TAG_IY),                        /* Iy */
        STUFF_DATA | TAG_INT32, INJECT32(ITEM_TAG_IB),                        /* Ib */
        STUFF_DATA | TAG_UINT16, INJECT16(ITEM_TAG_VR),                       /* Vr */
        STUFF_DATA | TAG_UINT16, INJECT16(ITEM_TAG_VY),                       /* Vy */
        STUFF_DATA | TAG_UINT16, INJECT16(ITEM_TAG_VB),                       /* Vb */
        STUFF_DATA | TAG_INT8,   (ITEM_TAG_PFR),                              /* PFr */
        STUFF_DATA | TAG_INT8,   (ITEM_TAG_PFY),                              /* PFy */
        STUFF_DATA | TAG_INT8,   (ITEM_TAG_PFB),                              /* PFb */
        STUFF_DATA | TAG_INT8,   (ITEM_TAG_PF_TOTAL),                         /* PF-total */
        STUFF_DATA | TAG_UINT16, INJECT16(ITEM_TAG_FREQUENCY),                /* Frequency */
        STUFF_DATA | TAG_UINT32, INJECT32(ITEM_TAG_KVA_TOTAL),                /* kVA */
        STUFF_DATA | TAG_INT32, INJECT32(ITEM_TAG_KW_TOTAL),                  /* kW */
        STUFF_DATA | TAG_INT32, INJECT32(ITEM_TAG_KVAR_TOTAL),                /* kVAr */
        STUFF_DATA | TAG_UINT32, INJECT32(ITEM_TAG_NUM_POWER_OFFS),           /* Number of power failures */
        STUFF_DATA | TAG_UINT32, INJECT32(ITEM_TAG_CUM_POWER_OFF_DURATION),   /* Cumulative power off duration */
        STUFF_DATA | TAG_UINT32, INJECT32(ITEM_TAG_CUM_TAMPER_COUNT),         /* Cumulative tamper count */
        STUFF_DATA | TAG_UINT32, INJECT32(ITEM_TAG_CUM_MD_RESET_COUNT),       /* Cumulative MD resets count */
        STUFF_DATA | TAG_UINT32, INJECT32(ITEM_TAG_CUM_PROGRAMMING_COUNT),    /* Cumulative programming count */
        STUFF_DATA | TAG_OCTET_STRING, 12, ITEM_TAG_CUM_LAST_MD_EVENT_DATETIME, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        STUFF_DATA | TAG_UINT64, INJECT64(ITEM_TAG_CUM_KWH_TOTAL),            /* Cumulative kWh */
        STUFF_DATA | TAG_UINT64, INJECT64(ITEM_TAG_CUM_KWH_EXPO_TOTAL),       /* Cumulative kWh */
#ifdef LTCT_METER
        STUFF_DATA | TAG_UINT64, INJECT64(ITEM_TAG_CUM_KVAR_Q1_TOTAL),        /* Cumulative kvarh lag */
        STUFF_DATA | TAG_UINT64, INJECT64(ITEM_TAG_CUM_KVAR_Q2_TOTAL),        /* Cumulative kvarh lead */
        STUFF_DATA | TAG_UINT64, INJECT64(ITEM_TAG_CUM_KVAR_Q3_TOTAL),        /* Cumulative kvarh lag */
        STUFF_DATA | TAG_UINT64, INJECT64(ITEM_TAG_CUM_KVAR_Q4_TOTAL),        /* Cumulative kvarh lead */
#endif
        STUFF_DATA | TAG_UINT64, INJECT64(ITEM_TAG_CUM_KVAH_TOTAL),           /* Cumulative kVAh */
        STUFF_DATA | TAG_UINT64, INJECT64(ITEM_TAG_CUM_KVAH_EXPO_TOTAL),      /* Cumulative kVAh */
        //STUFF_DATA | TAG_UINT32, INJECT32(ITEM_TAG_CUM_POWER_ON_DURATION),  /* Cumulative power on hours */
        //STUFF_DATA | TAG_UINT16, INJECT16(ITEM_TAG_DEVICE_TEMPERATURE),     /* Device temperature */
        STUFF_DATA | TAG_UINT32, INJECT32(ITEM_TAG_KW_MAX_DEMAND),            /* Max demand kW */
        STUFF_DATA | TAG_OCTET_STRING, 12, ITEM_TAG_CUM_LAST_MD_KW_DATETIME, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,/* Max demand kW time */
        STUFF_DATA | TAG_UINT32, INJECT32(ITEM_TAG_KVA_MAX_DEMAND),           /* Max demand kVA */
        STUFF_DATA | TAG_OCTET_STRING, 12, ITEM_TAG_CUM_LAST_MD_KVA_DATETIME, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,/* Max demand kVA time */
#ifdef WHOLE_CURRENT_METER
        STUFF_DATA | TAG_BOOLEAN, ITEM_TAG_LOAD_STATUS,                       /* Load status */
        STUFF_DATA | TAG_UINT32, INJECT32(ITEM_TAG_LOAD_LIMIT_VAL),           /* Cumulative kWh */
        STUFF_DATA | TAG_UINT64, INJECT64(ITEM_TAG_CUM_KVAR_Q1_TOTAL),        /* Cumulative kvarh lag */
        STUFF_DATA | TAG_UINT64, INJECT64(ITEM_TAG_CUM_KVAR_Q2_TOTAL),        /* Cumulative kvarh lead */
        STUFF_DATA | TAG_UINT64, INJECT64(ITEM_TAG_CUM_KVAR_Q3_TOTAL),        /* Cumulative kvarh lag */
        STUFF_DATA | TAG_UINT64, INJECT64(ITEM_TAG_CUM_KVAR_Q4_TOTAL),        /* Cumulative kvarh lead */
#endif
#endif // SINGLE_PHASE_METER
};
void Capture_Instant_Profile_Data(void *data, int direction)
{
  if (direction != ATTR_READ)
    return;
  msg_info.template=Instant_Profile_Buffer_Template;
  msg_info.sz_template=sizeof(Instant_Profile_Buffer_Template);
  msg_info.num_entries=1;
  msg_info.start_entry=1;
  msg_info.column_szs=Instant_Profile_Column_Szs;
}
const __far uint8_t Instant_Profile_Capture_Objects[] =
{
   0x82,
#ifdef SINGLE_PHASE_METER
   INJECT16((22*18 + 1)),
        /*TAG_ARRAY,*/ 22,
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_CLOCK),       // Date & Time
                TAG_OCTET_STRING, 6, 0, 0, 1, 0, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Voltage R phase
                TAG_OCTET_STRING, 6, 1, 0, 12, 7, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Current phase
                TAG_OCTET_STRING, 6, 1, 0, 11, 7, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Current neutral
                TAG_OCTET_STRING, 6, 1, 0, 91, 7, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // PF
                TAG_OCTET_STRING, 6, 1, 0, 13, 7, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // freq
                TAG_OCTET_STRING, 6, 1, 0, 14, 7, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Apparent power, kVA
                TAG_OCTET_STRING, 6, 1, 0, 9, 7, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Active power, kW
                TAG_OCTET_STRING, 6, 1, 0, 1, 7, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative energy, kWh (Import)
                TAG_OCTET_STRING, 6, 1, 0, 1, 8, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative energy, kVAh (Import)
                TAG_OCTET_STRING, 6, 1, 0, 9, 8, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_EXTENDED_REGISTER), //MD kW
                TAG_OCTET_STRING, 6, 1, 0, 1, 6, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_EXTENDED_REGISTER), //MD kW time
                TAG_OCTET_STRING, 6, 1, 0, 1, 6, 0, 255,
                TAG_INT8, 5,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_EXTENDED_REGISTER),  //MD kVA
                TAG_OCTET_STRING, 6, 1, 0, 9, 6, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_EXTENDED_REGISTER),  //MD kVA time
                TAG_OCTET_STRING, 6, 1, 0, 9, 6, 0, 255,
                TAG_INT8, 5,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative power on duration
                TAG_OCTET_STRING, 6, 0, 0, 94, 91, 14, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_DATA),        // Cumulative tamper count
                TAG_OCTET_STRING, 6, 0, 0, 94, 91, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_DATA),        // Cumulative MD Reset Count
                TAG_OCTET_STRING, 6, 0, 0, 0, 1, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_DATA),        // Cumulative programming count
                TAG_OCTET_STRING, 6, 0, 0, 96, 2, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative energy, kWh (Export)
                TAG_OCTET_STRING, 6, 1, 0, 2, 8, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative energy, kVAh (Export)
                TAG_OCTET_STRING, 6, 1, 0, 10, 8, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_DISCONNECT_CONTROL), // load status
                TAG_OCTET_STRING, 6, 0, 0, 96, 3, 10, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_LIMITER), // load limit
                TAG_OCTET_STRING, 6, 0, 0, 17, 0, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
        
#else

#ifdef WHOLE_CURRENT_METER
   INJECT16((35*18 + 1)),
        /*TAG_ARRAY,*/ 35,
#endif
#ifdef LTCT_METER
   INJECT16((33*18 + 1)),
        /*TAG_ARRAY,*/ 33,
#endif
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_CLOCK),       // Date & Time
                TAG_OCTET_STRING, 6, 0, 0, 1, 0, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Current R phase
                TAG_OCTET_STRING, 6, 1, 0, 31, 7, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Current Y phase
                TAG_OCTET_STRING, 6, 1, 0, 51, 7, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Current B phase
                TAG_OCTET_STRING, 6, 1, 0, 71, 7, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Voltage R phase
                TAG_OCTET_STRING, 6, 1, 0, 32, 7, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Voltage Y phase
                TAG_OCTET_STRING, 6, 1, 0, 52, 7, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Voltage B phase
                TAG_OCTET_STRING, 6, 1, 0, 72, 7, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // PF R phase
                TAG_OCTET_STRING, 6, 1, 0, 33, 7, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // PF Y phase
                TAG_OCTET_STRING, 6, 1, 0, 53, 7, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // PF B phase
                TAG_OCTET_STRING, 6, 1, 0, 73, 7, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Avg PF
                TAG_OCTET_STRING, 6, 1, 0, 13, 7, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // freq
                TAG_OCTET_STRING, 6, 1, 0, 14, 7, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Apparent power, kVA
                TAG_OCTET_STRING, 6, 1, 0, 9, 7, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Active power, kW
                TAG_OCTET_STRING, 6, 1, 0, 1, 7, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    //Reactive power, kVAr
                TAG_OCTET_STRING, 6, 1, 0, 3, 7, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_DATA),        // Number of power failures
                TAG_OCTET_STRING, 6, 0, 0, 96, 7, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative power off duration
                TAG_OCTET_STRING, 6, 0, 0, 94, 91, 8, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_DATA),        // Cumulative tamper count
                TAG_OCTET_STRING, 6, 0, 0, 94, 91, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_DATA),        // Cumulative MD Reset Count
                TAG_OCTET_STRING, 6, 0, 0, 0, 1, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_DATA),        // Cumulative programming count
                TAG_OCTET_STRING, 6, 0, 0, 96, 2, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Billing date
                TAG_OCTET_STRING, 6, 0, 0, 0, 1, 2, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative energy, kWh (Import)
                TAG_OCTET_STRING, 6, 1, 0, 1, 8, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative energy, kWh (Export)
                TAG_OCTET_STRING, 6, 1, 0, 2, 8, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
#ifdef LTCT_METER
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative energy kvarh-Q1
                TAG_OCTET_STRING, 6, 1, 0, 5, 8, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative energy kvarh-Q2
                TAG_OCTET_STRING, 6, 1, 0, 6, 8, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative energy kvarh-Q3
                TAG_OCTET_STRING, 6, 1, 0, 7, 8, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative energy kvarh-Q4
                TAG_OCTET_STRING, 6, 1, 0, 8, 8, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
#endif
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative energy, kVAh (Import)
                TAG_OCTET_STRING, 6, 1, 0, 9, 8, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative energy, kVAh (Export)
                TAG_OCTET_STRING, 6, 1, 0, 10, 8, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
//            TAG_STRUCTURE, 4,
//                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // cumulative power on time
//                TAG_OCTET_STRING, 6, 0,0,94,91,14,255,
//                TAG_INT8, 2,
//                TAG_UINT16, INJECT16(0),
//            TAG_STRUCTURE, 4,
//                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // device temperature
//                TAG_OCTET_STRING, 6, 0,0,96,9,128,255,
//                TAG_INT8, 2,
//                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_EXTENDED_REGISTER), //MD kW
                TAG_OCTET_STRING, 6, 1, 0, 1, 6, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_EXTENDED_REGISTER), //MD kW time
                TAG_OCTET_STRING, 6, 1, 0, 1, 6, 0, 255,
                TAG_INT8, 5,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_EXTENDED_REGISTER),  //MD kVA
                TAG_OCTET_STRING, 6, 1, 0, 9, 6, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_EXTENDED_REGISTER),  //MD kVA time
                TAG_OCTET_STRING, 6, 1, 0, 9, 6, 0, 255,
                TAG_INT8, 5,
                TAG_UINT16, INJECT16(0),
#ifdef WHOLE_CURRENT_METER
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_DISCONNECT_CONTROL), // load status
                TAG_OCTET_STRING, 6, 0, 0, 96, 3, 10, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_LIMITER), // load limit
                TAG_OCTET_STRING, 6, 0, 0, 17, 0, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative energy kvarh-Q1
                TAG_OCTET_STRING, 6, 1, 0, 5, 8, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative energy kvarh-Q2
                TAG_OCTET_STRING, 6, 1, 0, 6, 8, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative energy kvarh-Q3
                TAG_OCTET_STRING, 6, 1, 0, 7, 8, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative energy kvarh-Q4
                TAG_OCTET_STRING, 6, 1, 0, 8, 8, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
#endif
#endif //SINGLE_PHASE_METER else
};

void Instant_Profile_Capture_Objects_copy(void* data, int direction)
{
  _COM_memcpy_ff(Data_Buffer, Instant_Profile_Capture_Objects, sizeof(Instant_Profile_Capture_Objects));
}
static const struct attribute_desc_s Obj_Instant_Profile[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *) Data_Buffer, Capture_Instant_Profile_Data},
  {3, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *) Data_Buffer, Instant_Profile_Capture_Objects_copy},
  {4, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Instant_Profile_Capture_Period, NULL},
  {5, ACCESS_PC___MRR__USR_, TAG_ENUM,            (void *) &Instant_Profile_Sort_Method, NULL},
  {6, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) Common_Sort_Object, NULL},
  {7, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Instant_Profile_Entries_In_Use, NULL},
  {8, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Instant_Profile_Entries, NULL}
};

//Name profile scaler
const uint32_t Instant_Scaler_Profile_Entries_In_Use = 1;
const uint32_t Instant_Scaler_Profile_Entries = 1;
const uint32_t Instant_Scaler_Profile_Capture_Period = 0;
const uint8_t Instant_Scaler_Profile_Sort_Method = 1;

const __far uint8_t Instant_Scaler_Profile_Buffer[] =
{
#ifdef SINGLE_PHASE_METER
    INJECT16(0x8100 | (14*6 + 2+1)),
        /*TAG_ARRAY,*/ 1,
          TAG_STRUCTURE, 14,
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_VOLTAGE),
                TAG_ENUM, OBIS_UNIT_VOLTAGE_VOLT,       // Unit of V
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_CURRENT),
                TAG_ENUM, OBIS_UNIT_CURRENT_AMPERE,     // Unit of Iph
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_CURRENT),
                TAG_ENUM, OBIS_UNIT_CURRENT_AMPERE,     // Unit of Ineu
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_POWER_FACTOR),
                TAG_ENUM, OBIS_UNIT_UNITLESS_COUNT,     // Unit of PF
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_FREQUENCY),
                TAG_ENUM, OBIS_UNIT_FREQUENCY_HERTZ,    // Unit of Frequency
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_APPARENT_POWER),
                TAG_ENUM, OBIS_UNIT_APPARENT_POWER_VA,  // Unit of KVA
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_ACTIVE_POWER),
                TAG_ENUM, OBIS_UNIT_ACTIVE_POWER_WATT,  // Unit of KW
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_ACTIVE_ENERGY),
                TAG_ENUM, OBIS_UNIT_ACTIVE_ENERGY_WATT_HOUR, // Unit of kWh
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_APPARENT_ENERGY),
                TAG_ENUM, OBIS_UNIT_APPARENT_ENERGY_VA_HOUR, // Unit of kVAh
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_ACTIVE_POWER),
                TAG_ENUM, OBIS_UNIT_ACTIVE_POWER_WATT, // Unit of KW
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_APPARENT_POWER),
                TAG_ENUM, OBIS_UNIT_APPARENT_POWER_VA, // Unit of KVA
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(0),
                TAG_ENUM, OBIS_UNIT_TIME_MINUTE, // Unit of Cumulative Power on Duration
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_ACTIVE_ENERGY),
                TAG_ENUM, OBIS_UNIT_ACTIVE_ENERGY_WATT_HOUR, // Unit of kWh
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_APPARENT_ENERGY),
                TAG_ENUM, OBIS_UNIT_APPARENT_ENERGY_VA_HOUR, // Unit of kVAh
#else
    INJECT16(0x8100 | (26*6 + 2+1)),
        /*TAG_ARRAY,*/ 1,
          TAG_STRUCTURE, 26,
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_CURRENT),
                TAG_ENUM, OBIS_UNIT_CURRENT_AMPERE,     // Unit of Ir
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_CURRENT),
                TAG_ENUM, OBIS_UNIT_CURRENT_AMPERE,     // Unit of Iy
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_CURRENT),
                TAG_ENUM, OBIS_UNIT_CURRENT_AMPERE,     // Unit of Ib
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_VOLTAGE),
                TAG_ENUM, OBIS_UNIT_VOLTAGE_VOLT,       // Unit of Vr
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_VOLTAGE),
                TAG_ENUM, OBIS_UNIT_VOLTAGE_VOLT,       // Unit of Vy
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_VOLTAGE),
                TAG_ENUM, OBIS_UNIT_VOLTAGE_VOLT,       // Unit of Vb
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_POWER_FACTOR),
                TAG_ENUM, OBIS_UNIT_UNITLESS_COUNT,     // Unit of PF_R
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_POWER_FACTOR),
                TAG_ENUM, OBIS_UNIT_UNITLESS_COUNT,     // Unit of PF_Y
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_POWER_FACTOR),
                TAG_ENUM, OBIS_UNIT_UNITLESS_COUNT,     // Unit of PF_B
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_POWER_FACTOR),
                TAG_ENUM, OBIS_UNIT_UNITLESS_COUNT,     // Unit of PF_Total
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_FREQUENCY),
                TAG_ENUM, OBIS_UNIT_FREQUENCY_HERTZ,    // Unit of Frequency Total
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_APPARENT_POWER),
                TAG_ENUM, OBIS_UNIT_APPARENT_POWER_VA,  // Unit of KVA
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_ACTIVE_POWER),
                TAG_ENUM, OBIS_UNIT_ACTIVE_POWER_WATT,  // Unit of KW
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_REACTIVE_POWER),
                TAG_ENUM, OBIS_UNIT_REACTIVE_POWER_VAR, // Unit of KVAr
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(0),
                TAG_ENUM, OBIS_UNIT_TIME_MINUTE, // Unit of Cumulative Power off Duration
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(0),
                TAG_ENUM, OBIS_UNIT_UNITLESS_COUNT, //bill day // Unit of Last MD Reset Date and Time
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_ACTIVE_ENERGY),
                TAG_ENUM, OBIS_UNIT_ACTIVE_ENERGY_WATT_HOUR, // Unit of kWh
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_ACTIVE_ENERGY),
                TAG_ENUM, OBIS_UNIT_ACTIVE_ENERGY_WATT_HOUR, // Unit of kWh
#ifdef LTCT_METER
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_REACTIVE_ENERGY),
                TAG_ENUM, OBIS_UNIT_REACTIVE_ENERGY_VAR_HOUR, // Unit of kVArh Lag
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_REACTIVE_ENERGY),
                TAG_ENUM, OBIS_UNIT_REACTIVE_ENERGY_VAR_HOUR, // Unit of kVArh Lead
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_REACTIVE_ENERGY),
                TAG_ENUM, OBIS_UNIT_REACTIVE_ENERGY_VAR_HOUR, // Unit of kVArh Lag
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_REACTIVE_ENERGY),
                TAG_ENUM, OBIS_UNIT_REACTIVE_ENERGY_VAR_HOUR, // Unit of kVArh Lead
#endif
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_APPARENT_ENERGY),
                TAG_ENUM, OBIS_UNIT_APPARENT_ENERGY_VA_HOUR, // Unit of kVAh
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_APPARENT_ENERGY),
                TAG_ENUM, OBIS_UNIT_APPARENT_ENERGY_VA_HOUR, // Unit of kVAh
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_ACTIVE_POWER),
                TAG_ENUM, OBIS_UNIT_ACTIVE_POWER_WATT, // Unit of KW
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_APPARENT_POWER),
                TAG_ENUM, OBIS_UNIT_APPARENT_POWER_VA, // Unit of KVA
#ifdef WHOLE_CURRENT_METER
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_REACTIVE_ENERGY),
                TAG_ENUM, OBIS_UNIT_REACTIVE_ENERGY_VAR_HOUR, // Unit of kVArh Lag
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_REACTIVE_ENERGY),
                TAG_ENUM, OBIS_UNIT_REACTIVE_ENERGY_VAR_HOUR, // Unit of kVArh Lead
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_REACTIVE_ENERGY),
                TAG_ENUM, OBIS_UNIT_REACTIVE_ENERGY_VAR_HOUR, // Unit of kVArh Lag
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_REACTIVE_ENERGY),
                TAG_ENUM, OBIS_UNIT_REACTIVE_ENERGY_VAR_HOUR, // Unit of kVArh Lead
#endif
#endif //SINGLE_PHASE_METER else
};
const __far uint8_t Instant_scaler_capture_Objects[]=
{
   0x82,
#ifdef SINGLE_PHASE_METER
   INJECT16((14*18 + 1)),
        /*TAG_ARRAY,*/ 14,
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Voltage R phase
                TAG_OCTET_STRING, 6, 1, 0, 12, 7, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Current phase
                TAG_OCTET_STRING, 6, 1, 0, 11, 7, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Current neutral
                TAG_OCTET_STRING, 6, 1, 0, 91, 7, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // PF
                TAG_OCTET_STRING, 6, 1, 0, 13, 7, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // freq
                TAG_OCTET_STRING, 6, 1, 0, 14, 7, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Apparent power, kVA
                TAG_OCTET_STRING, 6, 1, 0, 9, 7, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Active power, kW
                TAG_OCTET_STRING, 6, 1, 0, 1, 7, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative energy, kWh (Import)
                TAG_OCTET_STRING, 6, 1, 0, 1, 8, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative energy, kVAh (Import)
                TAG_OCTET_STRING, 6, 1, 0, 9, 8, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_EXTENDED_REGISTER), //MD kW
                TAG_OCTET_STRING, 6, 1, 0, 1, 6, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_EXTENDED_REGISTER),  //MD kVA
                TAG_OCTET_STRING, 6, 1, 0, 9, 6, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative power on duration
                TAG_OCTET_STRING, 6, 0, 0, 94, 91, 14, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative energy, kWh (Export)
                TAG_OCTET_STRING, 6, 1, 0, 2, 8, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative energy, kVAh (Export)
                TAG_OCTET_STRING, 6, 1, 0, 10, 8, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
#else
   INJECT16((26*18 + 1)),
        /*TAG_ARRAY,*/ 26,
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Current R phase
                TAG_OCTET_STRING, 6, 1, 0, 31, 7, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Current Y phase
                TAG_OCTET_STRING, 6, 1, 0, 51, 7, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Current B phase
                TAG_OCTET_STRING, 6, 1, 0, 71, 7, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Voltage R phase
                TAG_OCTET_STRING, 6, 1, 0, 32, 7, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Voltage Y phase
                TAG_OCTET_STRING, 6, 1, 0, 52, 7, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Voltage B phase
                TAG_OCTET_STRING, 6, 1, 0, 72, 7, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // PF R phase
                TAG_OCTET_STRING, 6, 1, 0, 33, 7, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // PF Y phase
                TAG_OCTET_STRING, 6, 1, 0, 53, 7, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // PF B phase
                TAG_OCTET_STRING, 6, 1, 0, 73, 7, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Avg PF
                TAG_OCTET_STRING, 6, 1, 0, 13, 7, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // freq
                TAG_OCTET_STRING, 6, 1, 0, 14, 7, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Apparent power, kVA
                TAG_OCTET_STRING, 6, 1, 0, 9, 7, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Active power, kW
                TAG_OCTET_STRING, 6, 1, 0, 1, 7, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    //Reactive power, kVAr
                TAG_OCTET_STRING, 6, 1, 0, 3, 7, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative power off duration
                TAG_OCTET_STRING, 6, 0, 0, 94, 91, 8, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Billing date
                TAG_OCTET_STRING, 6, 0, 0, 0, 1, 2, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative energy, kWh (Import)
                TAG_OCTET_STRING, 6, 1, 0, 1, 8, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative energy, kWh (Export)
                TAG_OCTET_STRING, 6, 1, 0, 2, 8, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
#ifdef LTCT_METER
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative energy kvarh-Q1
                TAG_OCTET_STRING, 6, 1, 0, 5, 8, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative energy kvarh-Q2
                TAG_OCTET_STRING, 6, 1, 0, 6, 8, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative energy kvarh-Q3
                TAG_OCTET_STRING, 6, 1, 0, 7, 8, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative energy kvarh-Q4
                TAG_OCTET_STRING, 6, 1, 0, 8, 8, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
#endif
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative energy, kVAh (Import)
                TAG_OCTET_STRING, 6, 1, 0, 9, 8, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative energy, kVAh (Export)
                TAG_OCTET_STRING, 6, 1, 0, 10, 8, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_EXTENDED_REGISTER), //MD kW
                TAG_OCTET_STRING, 6, 1, 0, 1, 6, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_EXTENDED_REGISTER),  //MD kVA
                TAG_OCTET_STRING, 6, 1, 0, 9, 6, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
#ifdef WHOLE_CURRENT_METER
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative energy kvarh-Q1
                TAG_OCTET_STRING, 6, 1, 0, 5, 8, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative energy kvarh-Q2
                TAG_OCTET_STRING, 6, 1, 0, 6, 8, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative energy kvarh-Q3
                TAG_OCTET_STRING, 6, 1, 0, 7, 8, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),    // Cumulative energy kvarh-Q4
                TAG_OCTET_STRING, 6, 1, 0, 8, 8, 0, 255,
                TAG_INT8, 3,
                TAG_UINT16, INJECT16(0),
#endif
#endif //SINGLE_PHASE_METER else
};

void Instant_Scaler_Profile_Buffer_Fucntion(void* data, int direction)
{
  _COM_memcpy_ff(Data_Buffer, Instant_Scaler_Profile_Buffer, sizeof(Instant_Scaler_Profile_Buffer));
}
void Instant_scaler_capture_Objects_Fucntion(void* data, int direction)
{
  _COM_memcpy_ff(Data_Buffer, Instant_scaler_capture_Objects, sizeof(Instant_scaler_capture_Objects));
}
static const struct attribute_desc_s Obj_Instant_Scaler_Profile[] =
{
    {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
    {2, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *) Data_Buffer, Instant_Scaler_Profile_Buffer_Fucntion},
    {3, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *) Data_Buffer, Instant_scaler_capture_Objects_Fucntion},
    {4, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Instant_Scaler_Profile_Capture_Period, NULL},
    {5, ACCESS_PC___MRR__USR_, TAG_ENUM,            (void *) &Instant_Scaler_Profile_Sort_Method, NULL},
    {6, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) Common_Sort_Object, NULL},
    {7, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Instant_Scaler_Profile_Entries_In_Use, NULL},
    {8, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Instant_Scaler_Profile_Entries, NULL}
};

//Load profile
const uint32_t Load_Profile_Entries = BLOCKLOAD_MAX_ENTRIES;//LOAD_PROFILE_ENTRIES_TOTAL;
const uint8_t Load_Profile_Sort_Method = 1;
const uint8_t Load_Profile_Capture_Objects[] =
{
  0x82,
#ifdef SINGLE_PHASE_METER
    INJECT16(8*18 + 1),
        /*TAG_ARRAY,*/ 8,
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_CLOCK),
                TAG_OCTET_STRING, 6, 0, 0, 1, 0, 0, 255, // Date & Time
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER), // voltage
                TAG_OCTET_STRING, 6, 1, 0, 12, 27, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
//            TAG_STRUCTURE, 4,
//                TAG_UINT16, INJECT16(CLASS_ID_REGISTER), // PF
//                TAG_OCTET_STRING, 6, 1, 0, 13, 27, 0, 255,
//                TAG_INT8, 2,
//                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER), // Block Energy KWh Import
                TAG_OCTET_STRING, 6, 1, 0, 1, 29, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER), // Block Energy KVAh Import
                TAG_OCTET_STRING, 6, 1, 0, 9, 29, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER), // Block Energy KWh Export
                TAG_OCTET_STRING, 6, 1, 0, 2, 29, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER), // Block Energy KVAh Export
                TAG_OCTET_STRING, 6, 1, 0, 10, 29, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER), // current phase
                TAG_OCTET_STRING, 6, 1, 0, 11, 27, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER), // current neutral
                TAG_OCTET_STRING, 6, 1, 0, 91, 27, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
#else

#ifdef WHOLE_CURRENT_METER
    INJECT16(14*18 + 1),
        /*TAG_ARRAY,*/ 14,
#endif
#ifdef LTCT_METER
    INJECT16(18*18 + 1),
        /*TAG_ARRAY,*/ 18,
#endif
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_CLOCK),
                TAG_OCTET_STRING, 6, 0, 0, 1, 0, 0, 255, // Date & Time
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER), // current R phase
                TAG_OCTET_STRING, 6, 1, 0, 31, 27, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER), // current Y phase
                TAG_OCTET_STRING, 6, 1, 0, 51, 27, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER), // current B phase
                TAG_OCTET_STRING, 6, 1, 0, 71, 27, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER), // voltage R phase
                TAG_OCTET_STRING, 6, 1, 0, 32, 27, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER), // voltage Y phase
                TAG_OCTET_STRING, 6, 1, 0, 52, 27, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER), // voltage B phase
                TAG_OCTET_STRING, 6, 1, 0, 72, 27, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER), // PF R phase
                TAG_OCTET_STRING, 6, 1, 0, 33, 27, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER), // PF Y phase
                TAG_OCTET_STRING, 6, 1, 0, 53, 27, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER), // PF B phase
                TAG_OCTET_STRING, 6, 1, 0, 73, 27, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER), // Block Energy KWh Import
                TAG_OCTET_STRING, 6, 1, 0, 1, 29, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER), // Block Energy KVAh Import
                TAG_OCTET_STRING, 6, 1, 0, 9, 29, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
#ifdef LTCT_METER
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER), // Block Energy KVArh Q1
                TAG_OCTET_STRING, 6, 1, 0, 5, 29, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER), // Block Energy KVArh Q2
                TAG_OCTET_STRING, 6, 1, 0, 6, 29, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER), // Block Energy KVArh Q3
                TAG_OCTET_STRING, 6, 1, 0, 7, 29, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER), // Block Energy KVArh Q4
                TAG_OCTET_STRING, 6, 1, 0, 8, 29, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
#endif
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER), // Block Energy KWh Export
                TAG_OCTET_STRING, 6, 1, 0, 2, 29, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
            TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER), // Block Energy KVAh Export
                TAG_OCTET_STRING, 6, 1, 0, 10, 29, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
#endif //SINGLE_PHASE_METER else
};
const uint16_t Load_Profile_Column_Szs[] = {16,21,26,31,34,37,40,42,44,46,51,56,61,66};
const uint8_t Load_Profile_Buffer_Template[] =
{
#ifdef SINGLE_PHASE_METER
    STUFF_DATA | TAG_STRUCTURE, 8,
        STUFF_DATA | TAG_OCTET_STRING, 12, ITEM_TAG_DATETIME_LP, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_V_LP),                        /* Vr */
        //STUFF_DATA | TAG_INT8,   ITEM_TAG_PF_LP,                                 /* PFr */
        STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_CUM_KWH_TOTAL_IMPORT_LP),     /* Block Active Energy impo*/
        STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_CUM_KVAH_TOTAL_IMPORT_LP),    /* Block Apparent Energy impo*/
        STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_CUM_KWH_TOTAL_EXPORT_LP),     /* Block Active Energy expo*/
        STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_CUM_KVAH_TOTAL_EXPORT_LP),    /* Block Apparent Energy expo*/
        STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_I_PHASE_LP),                  /* Iph */
        STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_I_NEUTRAL_LP),                /* Ineu */
#else
#ifdef WHOLE_CURRENT_METER
    STUFF_DATA | TAG_STRUCTURE, 14,
#endif
#ifdef LTCT_METER
    STUFF_DATA | TAG_STRUCTURE, 18,
#endif
        STUFF_DATA | TAG_OCTET_STRING, 12, ITEM_TAG_DATETIME_LP, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        STUFF_DATA | TAG_UINT32, INJECT32(ITEM_TAG_IR_LP),                       /* Ir */
        STUFF_DATA | TAG_UINT32, INJECT32(ITEM_TAG_IY_LP),                       /* Iy */
        STUFF_DATA | TAG_UINT32, INJECT32(ITEM_TAG_IB_LP),                       /* Ib */
        STUFF_DATA | TAG_UINT16, INJECT16(ITEM_TAG_VR_LP),                       /* Vr */
        STUFF_DATA | TAG_UINT16, INJECT16(ITEM_TAG_VY_LP),                       /* Vy */
        STUFF_DATA | TAG_UINT16, INJECT16(ITEM_TAG_VB_LP),                       /* Vb */
        STUFF_DATA | TAG_INT8,   ITEM_TAG_PFR_LP,                                /* PFr */
        STUFF_DATA | TAG_INT8,   ITEM_TAG_PFY_LP,                                /* PFy */
        STUFF_DATA | TAG_INT8,   ITEM_TAG_PFB_LP,                                /* PFb */
        STUFF_DATA | TAG_UINT32, INJECT32(ITEM_TAG_CUM_KWH_TOTAL_IMPORT_LP),     /* Block Active Energy impo*/
        STUFF_DATA | TAG_UINT32, INJECT32(ITEM_TAG_CUM_KVAH_TOTAL_IMPORT_LP),    /* Block Apparent Energy impo*/
#ifdef LTCT_METER
        STUFF_DATA | TAG_UINT32, INJECT32(ITEM_TAG_CUM_KVAR_Q1_TOTAL_LP),     /* Block Active Energy impo*/
        STUFF_DATA | TAG_UINT32, INJECT32(ITEM_TAG_CUM_KVAR_Q2_TOTAL_LP),    /* Block Apparent Energy impo*/
        STUFF_DATA | TAG_UINT32, INJECT32(ITEM_TAG_CUM_KVAR_Q3_TOTAL_LP),     /* Block Active Energy impo*/
        STUFF_DATA | TAG_UINT32, INJECT32(ITEM_TAG_CUM_KVAR_Q4_TOTAL_LP),    /* Block Apparent Energy impo*/
#endif
        STUFF_DATA | TAG_UINT32, INJECT32(ITEM_TAG_CUM_KWH_TOTAL_EXPORT_LP),     /* Block Active Energy expo*/
        STUFF_DATA | TAG_UINT32, INJECT32(ITEM_TAG_CUM_KVAH_TOTAL_EXPORT_LP),    /* Block Apparent Energy expo*/
#endif //SINGLE_PHASE_METER else
};
void Capture_Load_Profile_Data(void *data, int direction)
{
  range_entries entries;
  msg_info.total_entries = get_profile_entry_in_use(PROFILE_TYPE_BLOCKLOAD);
  msg_info.template=Load_Profile_Buffer_Template;
  msg_info.sz_template=sizeof(Load_Profile_Buffer_Template);
  
  if(msg_info.total_entries==0)
  {
    msg_info.num_entries=0xffff;
  }
  else
  {
    if(access_selector>0)
    {
      if(access_selector==1)//range
      {
        if((SA_Range[0].Year>=0xffff)||(SA_Range[1].Year>=0xffff))
        {
          msg_info.num_entries=0xffff;
        }
        else
        {
          entries = find_entries_by_range(Time_To_Epoch(SA_Range[0].Year%100, SA_Range[0].Month, SA_Range[0].Day, SA_Range[0].Hr, SA_Range[0].Min, 0),
                                                Time_To_Epoch(SA_Range[1].Year%100, SA_Range[1].Month, SA_Range[1].Day, SA_Range[1].Hr, SA_Range[1].Min, 0),
                                                PROFILE_TYPE_BLOCKLOAD);
          
          msg_info.num_entries=entries.num;
          if(FiFO_LiFO==0)
          {
            msg_info.start_entry=entries.from + 1;
          }
          else
          {
            msg_info.start_entry= msg_info.total_entries - (entries.from + entries.num) + 1;
          }
        }
      }
      else //entry
      {
        if(SA_To_Entry==0)
        {
          msg_info.num_entries=msg_info.total_entries;
          msg_info.start_entry=1;
        }
        else
        {
          if(SA_To_Entry > msg_info.total_entries)
          {
            SA_To_Entry = msg_info.total_entries;
          }
          if(SA_From_Entry > msg_info.total_entries)
          {
            SA_From_Entry = msg_info.total_entries;
          }

          msg_info.num_entries=SA_To_Entry-SA_From_Entry+1;
          msg_info.start_entry=SA_From_Entry;
        }
      }
    }
    else
    {
      msg_info.num_entries=msg_info.total_entries;
      msg_info.start_entry=1;
    }
  }

  msg_info.column_szs=Load_Profile_Column_Szs;
  if(msg_info.num_entries==0)
  {
    msg_info.num_entries=0xFFFF;
  }
//#ifdef MAINS_ON
//  else
//  {
//    if(mains_stat == 0)
//    {
//      msg_info.num_entries = 0xFFFF;
//    }
//  }
//#endif
}
void Load_Profile_Entries_In_Use_Function(void *data, int direction)
{
    tmp_uint32 = get_profile_entry_in_use(PROFILE_TYPE_BLOCKLOAD);
}
static const struct attribute_desc_s Obj_Load_Profile[] =
{
    {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
    {0xC0+2, ACCESS_PC___MRR__USR_, TAG_ARRAY,      (void *) Data_Buffer, Capture_Load_Profile_Data},
    {3, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *) Load_Profile_Capture_Objects, NULL},
    {4, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &tmp_uint32, Capture_Period_LP_Function},
    {5, ACCESS_PC___MRR__USR_, TAG_ENUM,            (void *) &Load_Profile_Sort_Method, NULL},
    {6, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) Common_Sort_Object, NULL},
    {7, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &tmp_uint32, Load_Profile_Entries_In_Use_Function},
    {8, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Load_Profile_Entries, NULL},
};
//
// Load profile scaler
const uint32_t Load_Scaler_Profile_Entries_In_Use = 1;
const uint32_t Load_Scaler_Profile_Entries = 1;
const uint32_t Load_Scaler_Profile_Capture_Period = 0;
const uint8_t Load_Scaler_Profile_Sort_Method = 1;
void Block_load_scaler_capture_Objects(void *data, int direction)
{
  uint16_t size;
#ifdef SINGLE_PHASE_METER
  uint8_t i = 7;
#endif
#ifdef WHOLE_CURRENT_METER
  uint8_t i = 13;
#endif
#ifdef LTCT_METER
  uint8_t i = 17;
#endif

  Data_Buffer[0]=0x82;
  Data_Buffer[1]=(uint8_t)((i*18 + 1)>>8);
  Data_Buffer[2]=(uint8_t)((i*18 + 1)&0xFF);
  Data_Buffer[3]=i;
  size=i*18;
  memcpy(&Data_Buffer[4],&Load_Profile_Capture_Objects[22],size);
  size = i;
  for(i=1;i<=size;i++)
  {
    Data_Buffer[i*18]=3;
  }
}
const uint8_t Load_Scaler_Profile_Buffer[] =
{
#ifdef SINGLE_PHASE_METER
  INJECT16(0x8100 | (7*6 + 2 + 1)),
    1,
      TAG_STRUCTURE, 7,
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_VOLTAGE),
          TAG_ENUM, OBIS_UNIT_VOLTAGE_VOLT,                 // Unit of V
//        TAG_STRUCTURE, 2,
//          TAG_INT8, INJECT8(SCALER_POWER_FACTOR),
//          TAG_ENUM, OBIS_UNIT_UNITLESS_COUNT,               // Unit of PF
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_ACTIVE_ENERGY),
          TAG_ENUM, OBIS_UNIT_ACTIVE_ENERGY_WATT_HOUR,      // Unit of Active Energy impo
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_APPARENT_ENERGY),
          TAG_ENUM, OBIS_UNIT_APPARENT_ENERGY_VA_HOUR,      // Unit of Appparent Energy impo
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_ACTIVE_ENERGY),
          TAG_ENUM, OBIS_UNIT_ACTIVE_ENERGY_WATT_HOUR,      // Unit of Active Energy expo
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_APPARENT_ENERGY),
          TAG_ENUM, OBIS_UNIT_APPARENT_ENERGY_VA_HOUR,      // Unit of Appparent Energy expo
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_CURRENT),
          TAG_ENUM, OBIS_UNIT_CURRENT_AMPERE,               // Unit of Iph
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_CURRENT),
          TAG_ENUM, OBIS_UNIT_CURRENT_AMPERE,               // Unit of Ineu
#else
#ifdef WHOLE_CURRENT_METER
  INJECT16(0x8100 | (13*6 + 2 + 1)),
    1,
      TAG_STRUCTURE, 13,
#endif
#ifdef LTCT_METER
  INJECT16(0x8100 | (17*6 + 2 + 1)),
    1,
      TAG_STRUCTURE, 17,
#endif
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_CURRENT),
          TAG_ENUM, OBIS_UNIT_CURRENT_AMPERE,               // Unit of Ir
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_CURRENT),
          TAG_ENUM, OBIS_UNIT_CURRENT_AMPERE,               // Unit of Iy
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_CURRENT),
          TAG_ENUM, OBIS_UNIT_CURRENT_AMPERE,               // Unit of Ib
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_VOLTAGE),
          TAG_ENUM, OBIS_UNIT_VOLTAGE_VOLT,                 // Unit of Vr
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_VOLTAGE),
          TAG_ENUM, OBIS_UNIT_VOLTAGE_VOLT,                 // Unit of Vy
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_VOLTAGE),
          TAG_ENUM, OBIS_UNIT_VOLTAGE_VOLT,                 // Unit of Vb
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_POWER_FACTOR),
          TAG_ENUM, OBIS_UNIT_UNITLESS_COUNT,               // Unit of PFr
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_POWER_FACTOR),
          TAG_ENUM, OBIS_UNIT_UNITLESS_COUNT,               // Unit of PFy
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_POWER_FACTOR),
          TAG_ENUM, OBIS_UNIT_UNITLESS_COUNT,               // Unit of PFb
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_ACTIVE_ENERGY),
          TAG_ENUM, OBIS_UNIT_ACTIVE_ENERGY_WATT_HOUR,      // Unit of Active Energy impo
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_APPARENT_ENERGY),
          TAG_ENUM, OBIS_UNIT_APPARENT_ENERGY_VA_HOUR,      // Unit of Appparent Energy impo
#ifdef LTCT_METER
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_REACTIVE_ENERGY),
          TAG_ENUM, OBIS_UNIT_REACTIVE_ENERGY_VAR_HOUR,      // Unit of Reactive Energy Q1
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_REACTIVE_ENERGY),
          TAG_ENUM, OBIS_UNIT_REACTIVE_ENERGY_VAR_HOUR,      // Unit of Reactive Energy Q2
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_REACTIVE_ENERGY),
          TAG_ENUM, OBIS_UNIT_REACTIVE_ENERGY_VAR_HOUR,      // Unit of Reactive Energy Q3
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_REACTIVE_ENERGY),
          TAG_ENUM, OBIS_UNIT_REACTIVE_ENERGY_VAR_HOUR,      // Unit of Reactive Energy Q4
#endif
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_ACTIVE_ENERGY),
          TAG_ENUM, OBIS_UNIT_ACTIVE_ENERGY_WATT_HOUR,      // Unit of Active Energy expo
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_APPARENT_ENERGY),
          TAG_ENUM, OBIS_UNIT_APPARENT_ENERGY_VA_HOUR,      // Unit of Appparent Energy expo
#endif //SINGLE_PHASE_METER else
};
static const struct attribute_desc_s Obj_Load_Scaler_Profile[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *) Load_Scaler_Profile_Buffer, NULL},
  {3, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *) Data_Buffer, Block_load_scaler_capture_Objects},
  {4, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Load_Scaler_Profile_Capture_Period, NULL},
  {5, ACCESS_PC___MRR__USR_, TAG_ENUM,            (void *) &Load_Scaler_Profile_Sort_Method, NULL},
  {6, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) Common_Sort_Object, NULL},
  {7, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Load_Scaler_Profile_Entries_In_Use, NULL},
  {8, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Load_Scaler_Profile_Entries, NULL},
};

// Daily load profile
const uint32_t Daily_Load_Profile_Entries = DAILYLOAD_MAX_ENTRIES;//1 entry per day, total 36 days
const uint8_t Daily_Load_Profile_Sort_Method = 1;
const uint8_t Daily_Load_Profile_Capture_Objects[] =
{
  (5*18 + 1),
    5,
      TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_CLOCK),
        TAG_OCTET_STRING, 6, 0, 0, 1, 0, 0, 255,
        TAG_INT8, 2,
        TAG_UINT16, INJECT16(0),
      TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
        TAG_OCTET_STRING, 6, 1, 0,  1, 8, 0, 255,
        TAG_INT8, 2,
        TAG_UINT16, INJECT16(0),
      TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
        TAG_OCTET_STRING, 6, 1, 0,  9, 8, 0, 255,
        TAG_INT8, 2,
        TAG_UINT16, INJECT16(0),
      TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
        TAG_OCTET_STRING, 6, 1, 0,  2, 8, 0, 255,
        TAG_INT8, 2,
        TAG_UINT16, INJECT16(0),
      TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
        TAG_OCTET_STRING, 6, 1, 0,  10, 8, 0, 255,
        TAG_INT8, 2,
        TAG_UINT16, INJECT16(0),
};
const uint16_t Daily_Load_Profile_Column_Szs[] = {16,21,26,31,36};
const uint8_t Daily_Load_Profile_Buffer_Template[] =
{
  STUFF_DATA | TAG_STRUCTURE, 5,
    STUFF_DATA | TAG_OCTET_STRING, 12, ITEM_TAG_DATETIME_DL_LP, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_CUM_KWH_TOTAL_DL_LP),            /* cumulative Energy */
    STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_CUM_KVAH_TOTAL_DL_LP),           /* cumulative Energy */
    STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_CUM_KWH_EXPO_TOTAL_DL_LP),            /* cumulative Energy */
    STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_CUM_KVAH_EXPO_TOTAL_DL_LP),           /* cumulative Energy */
};
void Capture_Daily_Load_Profile_Data(void *data, int direction)
{
   range_entries entries;
   msg_info.total_entries = get_profile_entry_in_use(PROFILE_TYPE_DAILYLOAD);
   msg_info.template=Daily_Load_Profile_Buffer_Template;
   msg_info.sz_template=sizeof(Daily_Load_Profile_Buffer_Template);

   if(msg_info.total_entries==0)
   {
      msg_info.num_entries=0xffff;
   }
   else
   {
     if(access_selector>0)
     {
        if(access_selector==1)//range
        {
          if((SA_Range[0].Year>=0xffff)||(SA_Range[1].Year>=0xffff))
          {
            msg_info.num_entries=0xffff;
          }
          else
          {
            entries = find_entries_by_range(Time_To_Epoch(SA_Range[0].Year%100, SA_Range[0].Month, SA_Range[0].Day, SA_Range[0].Hr, SA_Range[0].Min, 0),
                                                  Time_To_Epoch(SA_Range[1].Year%100, SA_Range[1].Month, SA_Range[1].Day, SA_Range[1].Hr, SA_Range[1].Min, 0),
                                                PROFILE_TYPE_DAILYLOAD);
          
            msg_info.num_entries=entries.num;
            if(FiFO_LiFO==0)
            {
              msg_info.start_entry=entries.from + 1;
            }
            else
            {
              msg_info.start_entry= msg_info.total_entries - (entries.from + entries.num) + 1;
            }
          }
        }
        else //entry
        {
           if(SA_To_Entry==0)
           {
             msg_info.num_entries=msg_info.total_entries;
             msg_info.start_entry=1;
           }
           else
           {
             if(SA_To_Entry > msg_info.total_entries)
             {
               SA_To_Entry = msg_info.total_entries;
             }
             if(SA_From_Entry > msg_info.total_entries)
             {
               SA_From_Entry = msg_info.total_entries;
             }
           
             msg_info.num_entries=SA_To_Entry-SA_From_Entry+1;
             msg_info.start_entry=SA_From_Entry;
           }
        }
     }
     else
     {
       msg_info.num_entries=msg_info.total_entries;
       msg_info.start_entry=1;
     }
   }
   msg_info.column_szs=Daily_Load_Profile_Column_Szs;
   if(msg_info.num_entries==0)
   {
     msg_info.num_entries=0xFFFF;
   }
//#ifdef MAINS_ON
//   else
//   {
//     if(mains_stat == 0)
//     {
//       msg_info.num_entries = 0xFFFF;
//     }
//   }
//#endif
}
void Daily_Load_Profile_Entries_In_Use_Function(void *data, int direction)
{
    tmp_uint32 = get_profile_entry_in_use(PROFILE_TYPE_DAILYLOAD);
}
static const struct attribute_desc_s Obj_Daily_Load_Profile[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {0xC0+2, ACCESS_PC___MRR__USR_, TAG_ARRAY,      (void *) Data_Buffer, Capture_Daily_Load_Profile_Data},
  {3, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *) Daily_Load_Profile_Capture_Objects, NULL},
  {4, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Daily_Load_Profile_Capture_Period, NULL},
  {5, ACCESS_PC___MRR__USR_, TAG_ENUM,            (void *) &Daily_Load_Profile_Sort_Method, NULL},
  {6, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) Common_Sort_Object, NULL},
  {7, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &tmp_uint32, Daily_Load_Profile_Entries_In_Use_Function},
  {8, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Daily_Load_Profile_Entries, NULL},
};
//
//Daily load profile scaler
const uint32_t Daily_Load_Scaler_Profile_Entries_In_Use = 1;
const uint32_t Daily_Load_Scaler_Profile_Entries = 1;
const uint32_t Daily_Load_Scaler_Profile_Capture_Period = 0;
const uint8_t Daily_Load_Scaler_Profile_Sort_Method = 1;
const uint8_t Daily_Load_Scaler_Profile_Capture_Objects[] =
{
  (4*18 + 1),
  /*TAG_ARRAY,*/ 4,
    TAG_STRUCTURE, 4,
      TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
      TAG_OCTET_STRING, 6, 1, 0, 1, 8, 0, 255,
      TAG_INT8, 3,
      TAG_UINT16, INJECT16(0),
    TAG_STRUCTURE, 4,
      TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
      TAG_OCTET_STRING, 6, 1, 0, 9, 8, 0, 255,
      TAG_INT8, 3,
      TAG_UINT16, INJECT16(0),
    TAG_STRUCTURE, 4,
      TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
      TAG_OCTET_STRING, 6, 1, 0, 2, 8, 0, 255,
      TAG_INT8, 3,
      TAG_UINT16, INJECT16(0),
    TAG_STRUCTURE, 4,
      TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
      TAG_OCTET_STRING, 6, 1, 0, 10, 8, 0, 255,
      TAG_INT8, 3,
      TAG_UINT16, INJECT16(0),
};
const uint8_t Daily_Load_Scaler_Profile_Buffer[] =
{
  6*4 + 2 +1,
    1,
      TAG_STRUCTURE, 4,
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_ACTIVE_ENERGY),
          TAG_ENUM, OBIS_UNIT_ACTIVE_ENERGY_WATT_HOUR,    // Unit of Active Energy
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_APPARENT_ENERGY),
          TAG_ENUM, OBIS_UNIT_APPARENT_ENERGY_VA_HOUR,    // Unit of Apparent Energy
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_ACTIVE_ENERGY),
          TAG_ENUM, OBIS_UNIT_ACTIVE_ENERGY_WATT_HOUR,    // Unit of Active Energy
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_APPARENT_ENERGY),
          TAG_ENUM, OBIS_UNIT_APPARENT_ENERGY_VA_HOUR,    // Unit of Apparent Energy
};
static const struct attribute_desc_s Obj_Daily_Load_Scaler_Profile[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *) Daily_Load_Scaler_Profile_Buffer, NULL},
  {3, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *) Daily_Load_Scaler_Profile_Capture_Objects, NULL},
  {4, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Daily_Load_Scaler_Profile_Capture_Period, NULL},
  {5, ACCESS_PC___MRR__USR_, TAG_ENUM,            (void *) &Daily_Load_Scaler_Profile_Sort_Method, NULL},
  {6, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) Common_Sort_Object, NULL},
  {7, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Daily_Load_Scaler_Profile_Entries_In_Use, NULL},
  {8, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Daily_Load_Scaler_Profile_Entries, NULL},
};
//
//// Billing profile
uint8_t generate_bill_on_firmware_upgrade = 0;
uint8_t generate_bill_on_activity_cal = 0;

uint32_t Billing_Profile_Entries = 13;
const uint32_t Billing_Profile_Capture_Period = 0;
const uint8_t Billing_Profile_Sort_Method = 1;
const uint16_t Billing_Profile_Column_Szs[]={16,21,30,35,40,45,50,55,60,65,70,79,88,97,102,107,112,117,122,127,132,137,142,156,161,175,180,194,199,213,218,232,237,251,256,270,275,289,294,308,313,327,332,346,351,365,370,384,389,403,408,422,427,441,446,460,465,479,484};
const uint8_t Billing_Profile_Buffer_Template[]=
{
  STUFF_DATA | TAG_STRUCTURE, 23,
   STUFF_DATA | TAG_OCTET_STRING, 12,  ITEM_TAG_BILLING_DATETIME_BI, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,//16
   STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_SPF_BI), //System Power Factor for billing period //21
   STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_KWH_BI), // Cumulative Energy - kWh 64 bit //30
   STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_KWH_TZ1_BI), // Cumulative Energy - kWh - TZ1 //35
   STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_KWH_TZ2_BI), // Cumulative Energy - kWh - TZ2 //40
   STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_KWH_TZ3_BI), // Cumulative Energy - kWh - TZ3 //45
   STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_KWH_TZ4_BI), // Cumulative Energy - kWh - TZ4 //50
   STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_KWH_TZ5_BI), // Cumulative Energy - kWh - TZ5 //55
   STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_KWH_TZ6_BI), // Cumulative Energy - kWh - TZ6 //60
   //STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_KWH_TZ7_BI), // Cumulative Energy - kWh - TZ7 //65
   //STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_KWH_TZ8_BI), // Cumulative Energy - kWh - TZ8 //70
   STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_KVAH_BI), // Cumulative Energy - kVAh //97
   STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_KVAH_TZ1_BI), // Cumulative Energy - kVAH - TZ1 //102
   STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_KVAH_TZ2_BI), // Cumulative Energy - kVAH - TZ2 //107
   STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_KVAH_TZ3_BI), // Cumulative Energy - kVAH - TZ3 //112
   STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_KVAH_TZ4_BI), // Cumulative Energy - kVAH - TZ4 //117
   STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_KVAH_TZ5_BI), // Cumulative Energy - kVAH - TZ5 //122
   STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_KVAH_TZ6_BI), // Cumulative Energy - kVAH - TZ6 //127
   //STUFF_DATA | TAG_FLOAT32, INJECT_F32((ITEM_TAG_KVAH_TZ7_BI), // Cumulative Energy - kVAH - TZ7 //132
   //STUFF_DATA | TAG_FLOAT32, INJECT_F32((ITEM_TAG_KVAH_TZ8_BI), // Cumulative Energy - kVAH - TZ8 //137
   STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_MD_KW_BI), STUFF_DATA | TAG_OCTET_STRING, 12,  ITEM_TAG_DATETIME_MD_KW_BI, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //142//156
   //STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_MD_KW_TZ1_BI), STUFF_DATA | TAG_OCTET_STRING, 12,  ITEM_TAG_DATETIME_MD_KW_TZ1_BI, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //161//175
   //STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_MD_KW_TZ2_BI), STUFF_DATA | TAG_OCTET_STRING, 12,  ITEM_TAG_DATETIME_MD_KW_TZ2_BI, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //180//194
   //STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_MD_KW_TZ3_BI), STUFF_DATA | TAG_OCTET_STRING, 12,  ITEM_TAG_DATETIME_MD_KW_TZ3_BI, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //199//213
   //STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_MD_KW_TZ4_BI), STUFF_DATA | TAG_OCTET_STRING, 12,  ITEM_TAG_DATETIME_MD_KW_TZ4_BI, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //218//232
   //STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_MD_KW_TZ5_BI), STUFF_DATA | TAG_OCTET_STRING, 12,  ITEM_TAG_DATETIME_MD_KW_TZ5_BI, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //237//251
   //STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_MD_KW_TZ6_BI), STUFF_DATA | TAG_OCTET_STRING, 12,  ITEM_TAG_DATETIME_MD_KW_TZ6_BI, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //256//270
   //STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_MD_KW_TZ7_BI), STUFF_DATA | TAG_OCTET_STRING, 12,  ITEM_TAG_DATETIME_MD_KW_TZ7_BI, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //275//289
   //STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_MD_KW_TZ8_BI), STUFF_DATA | TAG_OCTET_STRING, 12,  ITEM_TAG_DATETIME_MD_KW_TZ8_BI, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //294//308
   STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_MD_KVA_BI), STUFF_DATA | TAG_OCTET_STRING, 12,  ITEM_TAG_DATETIME_MD_KVA_BI, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //313//327
   //STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_MD_KVA_TZ1_BI), STUFF_DATA | TAG_OCTET_STRING, 12,  ITEM_TAG_DATETIME_MD_KVA_TZ1_BI, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //332//346
   //STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_MD_KVA_TZ2_BI), STUFF_DATA | TAG_OCTET_STRING, 12,  ITEM_TAG_DATETIME_MD_KVA_TZ2_BI, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //351//365
   //STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_MD_KVA_TZ3_BI), STUFF_DATA | TAG_OCTET_STRING, 12,  ITEM_TAG_DATETIME_MD_KVA_TZ3_BI, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //370//384
   //STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_MD_KVA_TZ4_BI), STUFF_DATA | TAG_OCTET_STRING, 12,  ITEM_TAG_DATETIME_MD_KVA_TZ4_BI, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //389//403
   //STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_MD_KVA_TZ5_BI), STUFF_DATA | TAG_OCTET_STRING, 12,  ITEM_TAG_DATETIME_MD_KVA_TZ5_BI, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //408//422
   //STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_MD_KVA_TZ6_BI), STUFF_DATA | TAG_OCTET_STRING, 12,  ITEM_TAG_DATETIME_MD_KVA_TZ6_BI, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //427//441
   //STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_MD_KVA_TZ7_BI), STUFF_DATA | TAG_OCTET_STRING, 12,  ITEM_TAG_DATETIME_MD_KVA_TZ7_BI, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //446//460
   //STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_MD_KVA_TZ8_BI), STUFF_DATA | TAG_OCTET_STRING, 12,  ITEM_TAG_DATETIME_MD_KVA_TZ8_BI, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //465//479
   STUFF_DATA | TAG_UINT16, INJECT16(ITEM_TAG_POWER_ON_DURATION_BI), // Cumulative Energy - kVAH - TZ8 //484
   STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_KVARH_KWH_EXPO_BI), // Cumulative Energy - kvarh - Lag //79
   STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_KVARH_KVAH_EXPO_BI), // Cumulative Energy - kvarh - Lead //88
   //STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_KVARH_Q1_BI), // Cumulative Energy - kvarh - Lag //79
   //STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_KVARH_Q2_BI), // Cumulative Energy - kvarh - Lead //88
   //STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_KVARH_Q3_BI), // Cumulative Energy - kvarh - Lag //79
   //STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_KVARH_Q4_BI), // Cumulative Energy - kvarh - Lead //88
   //STUFF_DATA | TAG_UINT32, INJECT32(ITEM_TAG_TAMPER_COUNT_BI), // Tamper count

};

const uint8_t Billing_Capture_Objects[] =
{
  TAG_STRUCTURE, 4,//0
          TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
          TAG_OCTET_STRING, 6, 0, 0, 0, 1, 2, 255, // Bill Date & Time
          TAG_INT8, 2,
          TAG_UINT16, INJECT16(0),
  TAG_STRUCTURE, 4,//18
          TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
          TAG_OCTET_STRING, 6, 1, 0, 13, 0, 0, 255, // pf
          TAG_INT8, 2,
          TAG_UINT16, INJECT16(0),
  TAG_STRUCTURE, 4,//36
          TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
          TAG_OCTET_STRING, 6, 1, 0, 1, 8, 0, 255, // kwh
          TAG_INT8, 2,
          TAG_UINT16, INJECT16(0),
  TAG_STRUCTURE, 4,//54
          TAG_UINT16, INJECT16(CLASS_ID_EXTENDED_REGISTER),
          TAG_OCTET_STRING, 6, 1, 0, 1, 6, 0, 255, // MD-KW
          TAG_INT8, 2,
          TAG_UINT16, INJECT16(0),
  TAG_STRUCTURE, 4,//72
          TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
          TAG_OCTET_STRING, 6, 0, 0, 94, 91, 13, 255, // Power On Time
          TAG_INT8, 2,
          TAG_UINT16, INJECT16(0),
  TAG_STRUCTURE, 4,//72
          TAG_UINT16, INJECT16(CLASS_ID_DATA),
          TAG_OCTET_STRING, 6, 0, 0, 94, 91, 0, 255, // Tamper
          TAG_INT8, 2,
          TAG_UINT16, INJECT16(0),
};

void Capture_Billing_Profile_Data(void *data, int direction)
{
  range_entries entries;
  msg_info.total_entries = get_profile_entry_in_use(PROFILE_TYPE_BILLING) + 1;
  msg_info.template=Billing_Profile_Buffer_Template;
  msg_info.sz_template=sizeof(Billing_Profile_Buffer_Template);

  if(msg_info.total_entries==0)
  {
     msg_info.num_entries=0xffff;
  }
  else
  {
    if(access_selector>0)
    {
       if(access_selector==1)//range
       {
         if((SA_Range[0].Year>=0xffff)||(SA_Range[1].Year>=0xffff))
         {
           msg_info.num_entries=0xffff;
         }
         else
         {
           entries = find_entries_by_range(Time_To_Epoch(SA_Range[0].Year%100, SA_Range[0].Month, SA_Range[0].Day, SA_Range[0].Hr, SA_Range[0].Min, 0),
                                                 Time_To_Epoch(SA_Range[1].Year%100, SA_Range[1].Month, SA_Range[1].Day, SA_Range[1].Hr, SA_Range[1].Min, 0),
                                                 PROFILE_TYPE_BILLING);
         
           msg_info.num_entries=entries.num;
           if(FiFO_LiFO==0)
           {
             msg_info.start_entry=entries.from + 1;
           }
           else
           {
             msg_info.start_entry= msg_info.total_entries - (entries.from + entries.num) + 1;
           }
         }
       }
       else //entry
       {
          if(SA_To_Entry==0)
          {
            msg_info.num_entries=msg_info.total_entries;
            msg_info.start_entry=1;
          }
          else
          {
            if(SA_To_Entry > msg_info.total_entries)
            {
              SA_To_Entry = msg_info.total_entries;
            }
            if(SA_From_Entry > msg_info.total_entries)
            {
              SA_From_Entry = msg_info.total_entries;
            }
          
            msg_info.num_entries=SA_To_Entry-SA_From_Entry+1;
            msg_info.start_entry=SA_From_Entry;
          }
       }
    }
    else
    {
      msg_info.num_entries=msg_info.total_entries;
      msg_info.start_entry=1;
    }
  }
  msg_info.column_szs=Billing_Profile_Column_Szs;
  if(msg_info.num_entries==0)
  {
     msg_info.num_entries=0xFFFF;
  }
//#ifdef MAINS_ON
//  if(mains_stat == 0)
//  {
//   msg_info.num_entries = 1;
//   msg_info.start_entry = 1;
//  }
//#endif
}

void Billing_Capture_func(unsigned char scaler)
{
    uint8_t i;
#ifdef SINGLE_PHASE_METER
    if (scaler == 0)
    {
        Data_Buffer[0] = 0x82;
        Data_Buffer[1] = 0x01;
        Data_Buffer[2] = 0x9F;
        Data_Buffer[3] = 23;
    }
    else
    {
        Data_Buffer[0] = 0x82;
        Data_Buffer[1] = 0x01;
        Data_Buffer[2] = 0x7B;
        Data_Buffer[3] = 21;
    }
    memcpy(&Data_Buffer[4], &Billing_Capture_Objects[0], 36);
    for (i = 0; i < 7; i++)
    {
        memcpy(&Data_Buffer[40 + (i * 18)], &Billing_Capture_Objects[36], 18);
        Data_Buffer[51 + (i * 18)] = i;
    }
    for (i = 0; i < 7; i++)
    {
        memcpy(&Data_Buffer[166 + (i * 18)], &Billing_Capture_Objects[36], 18);
        Data_Buffer[175 + (i * 18)] = 9;
        Data_Buffer[177 + (i * 18)] = i;
    }
    
    //MD-KW
    for (i = 0; i < 1; i++)
    {
        unsigned char j = 0;
        if (scaler == 0)
        {
            for (j = 0; j < 2; j++)
            {
                memcpy(&Data_Buffer[292 + j * 18 + (i * (36))], &Billing_Capture_Objects[54], 18);
            }
            Data_Buffer[303 + (i * 36)] = i;
            Data_Buffer[321 + (i * 36)] = i;
            Data_Buffer[324 + (i * 36)] = 5;
        }
        else
        {
            memcpy(&Data_Buffer[292 + (i * (18))], &Billing_Capture_Objects[54], 18);
            Data_Buffer[303 + (i * 18)] = i;
        }
    }

    //MD-KVA
    for (i = 0; i < 1; i++)
    {
        unsigned char j = 0;
        if (scaler == 0)
        {
            for (j = 0; j < 2; j++)
            {
                memcpy(&Data_Buffer[328 + j * 18 + (i * (36))], &Billing_Capture_Objects[54], 18);
            }
            Data_Buffer[337 + (i * 36)] = 9;
            Data_Buffer[355 + (i * 36)] = 9;
            Data_Buffer[339 + (i * 36)] = i;
            Data_Buffer[357 + (i * 36)] = i;
            Data_Buffer[360 + (i * 36)] = 5;
        }
        else
        {
            memcpy(&Data_Buffer[310 + (i * (18))], &Billing_Capture_Objects[54], 18);
            Data_Buffer[319 + (i * 18)] = 9;
            Data_Buffer[321 + (i * 18)] = i;
        }
    }
    memcpy(&Data_Buffer[(328 + (uint16_t)(scaler==0)*36)], &Billing_Capture_Objects[72], 18);
    
    for (i = 0; i < 2; i++)
    {
        memcpy(&Data_Buffer[346 + ((uint16_t)(scaler==0) * 36) + (i * 18)], &Billing_Capture_Objects[36], 18);
    }
    Data_Buffer[355 + ((uint16_t)(scaler==0) * 36)] = 2;
    Data_Buffer[373 + ((uint16_t)(scaler==0) * 36)] = 10;
    
    if (scaler != 0)
    {
        for (i = 0; i < 21; i++)
        {
            Data_Buffer[18+(i * 18)] = 3;
        }
    }
                
#else
    if (scaler == 0)
    {
        Data_Buffer[0] = 0x82;
        Data_Buffer[1] = 0x04;
        Data_Buffer[2] = 0x81;
        Data_Buffer[3] = 64;
    }
    else
    {
        Data_Buffer[0] = 0x82;
        Data_Buffer[1] = 0x03;
        Data_Buffer[2] = 0x2B;
        Data_Buffer[3] = 45;
    }

    memcpy(&Data_Buffer[4], &Billing_Capture_Objects[0], 36);
    for (i = 0; i < 9; i++)
    {
        memcpy(&Data_Buffer[40 + (i * 18)], &Billing_Capture_Objects[36], 18);
        Data_Buffer[51 + (i * 18)] = i;
    }
    for (i = 0; i < 9; i++)
    {
        memcpy(&Data_Buffer[202 + (i * 18)], &Billing_Capture_Objects[36], 18);
        Data_Buffer[211 + (i * 18)] = 9;
        Data_Buffer[213 + (i * 18)] = i;
    }
    //MD-KW
    for (i = 0; i < 9; i++)
    {
        if (scaler == 0)
        {
            for (j = 0; j < 2; j++)
            {
                memcpy(&Data_Buffer[364 + j * 18 + (i * (36))], &Billing_Capture_Objects[54], 18);
            }
            Data_Buffer[375 + (i * 36)] = i;
            Data_Buffer[393 + (i * 36)] = i;
            Data_Buffer[396 + (i * 36)] = 5;
        }
        else
        {
            memcpy(&Data_Buffer[364 + (i * (18))], &Billing_Capture_Objects[54], 18);
            Data_Buffer[375 + (i * 18)] = i;
        }
    }

    //MD-KVA
    for (i = 0; i < 9; i++)
    {
        if (scaler == 0)
        {
            for (j = 0; j < 2; j++)
            {
                memcpy(&Data_Buffer[688 + j * 18 + (i * (36))], &Billing_Capture_Objects[54], 18);
            }
            Data_Buffer[697 + (i * 36)] = 9;
            Data_Buffer[715 + (i * 36)] = 9;
            Data_Buffer[699 + (i * 36)] = i;
            Data_Buffer[717 + (i * 36)] = i;
            Data_Buffer[720 + (i * 36)] = 5;
        }
        else
        {
            memcpy(&Data_Buffer[526 + (i * (18))], &Billing_Capture_Objects[54], 18);
            Data_Buffer[535 + (i * 18)] = 9;
            Data_Buffer[537 + (i * 18)] = i;
        }
    }

    memcpy(&Data_Buffer[(688+(uint16_t)(scaler==0)*324)], &Billing_Capture_Objects[72], 18);
    for (i = 0; i < 6; i++)
    {
        memcpy(&Data_Buffer[706 + ((uint16_t)(scaler==0) * 324) + (i * 18)], &Billing_Capture_Objects[36], 18);
    }
    Data_Buffer[715 + ((uint16_t)(scaler==0) * 324)] = 2;
    Data_Buffer[733 + ((uint16_t)(scaler==0) * 324)] = 10;
    Data_Buffer[751 + ((uint16_t)(scaler==0) * 324)] = 5;
    Data_Buffer[769 + ((uint16_t)(scaler==0) * 324)] = 6;
    Data_Buffer[787 + ((uint16_t)(scaler==0) * 324)] = 7;
    Data_Buffer[805 + ((uint16_t)(scaler==0) * 324)] = 8;

    if (scaler == 0)
    {
      memcpy(&Data_Buffer[1138], &Billing_Capture_Objects[90], 18);
    }
    if (scaler != 0)
    {
        for (i = 0; i < 45; i++)
        {
            Data_Buffer[18 + (i * 18)] = 3;
        }
    }
    
    #endif //SINGLE_PHASE_METER else
}
void Billing_Capture_Objects_func(void *data, int direction)
{
  Billing_Capture_func(0);
}
void Billing_scaler_capture_Objects(void *data, int direction)
{
  Billing_Capture_func(1);
}
void Billing_Profile_Entries_In_Use_Function(void *data, int direction)
{
  tmp_uint32 = get_profile_entry_in_use(PROFILE_TYPE_BILLING) + 1;
}

const uint32_t Billing_Profile_Entries_Total = BILLING_MAX_ENTRIES + 1;

static const struct attribute_desc_s Obj_Billing_Profile[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {0x80+2, ACCESS_PC___MRR__USR_, TAG_ARRAY,      (void *) Data_Buffer, Capture_Billing_Profile_Data},
  {3, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *) Data_Buffer,Billing_Capture_Objects_func},
  {4, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Billing_Profile_Capture_Period, NULL},
  {5, ACCESS_PC___MRR__USR_, TAG_ENUM,            (void *) &Billing_Profile_Sort_Method, NULL},
  {6, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) Common_Sort_Object, NULL},
  {7, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &tmp_uint32, Billing_Profile_Entries_In_Use_Function},
  {8, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Billing_Profile_Entries_Total, NULL},
};
//
static const struct method_desc_s Obj_Dummy_Resiter_And_ExtendedRegister_Methods[] =
{
    {1, ACCESS_PC___MR___US__, NULL},
};
////Billing profile methods
//void Obj_Billing_Profile_reset(uint8_t *data,uint16_t data_len,uint8_t *response,uint16_t *response_len)
//{
//  NOP();
//}
//void Obj_Billing_Profile_capture(uint8_t *data,uint16_t data_len,uint8_t *response,uint16_t *response_len)
//{
//  generate_bill_on_demand(&g_RTC_time);
//  store_event_data(TRANSACT_EVENT, 166, g_RTC_time.epoch);
//}

static const struct method_desc_s Obj_Dummy_Profile_Methods[] =
{
    {1, ACCESS_PC___MR___US__, NULL /*Obj_Billing_Profile_reset*/},
    {2, ACCESS_PC___MR___US__, NULL /*Obj_Billing_Profile_capture*/}
};
//

// Billing scaler profile
const uint32_t Billing_Scaler_Profile_Entries_In_Use = 1;
const uint32_t Billing_Scaler_Profile_Entries = 1;
const uint32_t Billing_Scaler_Profile_Capture_Period = 0;
const uint8_t Billing_Scaler_Profile_Sort_Method = 1;

const uint8_t Billing_Scaler_Profile_Buffer[] =
{

  0x81,
    INJECT8 (21*6 + 2 +1),
        1,
            TAG_STRUCTURE, 21,
            TAG_STRUCTURE, 2,
                TAG_INT8, 0,
                TAG_ENUM, OBIS_UNIT_UNITLESS_COUNT,             // Unit billing Date and Time
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_POWER_FACTOR),
                TAG_ENUM, OBIS_UNIT_UNITLESS_COUNT,             // Unit of PF
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_ACTIVE_ENERGY),
                TAG_ENUM, OBIS_UNIT_ACTIVE_ENERGY_WATT_HOUR,    // Unit of Active Energy
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_ACTIVE_ENERGY),
                TAG_ENUM, OBIS_UNIT_ACTIVE_ENERGY_WATT_HOUR,    // Unit of Active Energy TZ1
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_ACTIVE_ENERGY),
                TAG_ENUM, OBIS_UNIT_ACTIVE_ENERGY_WATT_HOUR,    // Unit of Active Energy TZ2
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_ACTIVE_ENERGY),
                TAG_ENUM, OBIS_UNIT_ACTIVE_ENERGY_WATT_HOUR,    // Unit of Active Energy TZ3
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_ACTIVE_ENERGY),
                TAG_ENUM, OBIS_UNIT_ACTIVE_ENERGY_WATT_HOUR,    // Unit of Active Energy TZ4
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_ACTIVE_ENERGY),
                TAG_ENUM, OBIS_UNIT_ACTIVE_ENERGY_WATT_HOUR,    // Unit of Active Energy TZ5
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_ACTIVE_ENERGY),
                TAG_ENUM, OBIS_UNIT_ACTIVE_ENERGY_WATT_HOUR,    // Unit of Active Energy TZ6
//            TAG_STRUCTURE, 2,
//                TAG_INT8, INJECT8(SCALER_ACTIVE_ENERGY),
//                TAG_ENUM, OBIS_UNIT_ACTIVE_ENERGY_WATT_HOUR,    // Unit of Active Energy TZ7
//            TAG_STRUCTURE, 2,
//                TAG_INT8, INJECT8(SCALER_ACTIVE_ENERGY),
//                TAG_ENUM, OBIS_UNIT_ACTIVE_ENERGY_WATT_HOUR,    // Unit of Active Energy TZ8
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_APPARENT_ENERGY),
                TAG_ENUM, OBIS_UNIT_APPARENT_ENERGY_VA_HOUR,    // Unit of Apparent Energy
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_APPARENT_ENERGY),
                TAG_ENUM, OBIS_UNIT_APPARENT_ENERGY_VA_HOUR,    // Unit of Apparent Energy TZ1
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_APPARENT_ENERGY),
                TAG_ENUM, OBIS_UNIT_APPARENT_ENERGY_VA_HOUR,    // Unit of Apparent Energy TZ2
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_APPARENT_ENERGY),
                TAG_ENUM, OBIS_UNIT_APPARENT_ENERGY_VA_HOUR,    // Unit of Apparent Energy TZ3
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_APPARENT_ENERGY),
                TAG_ENUM, OBIS_UNIT_APPARENT_ENERGY_VA_HOUR,    // Unit of Apparent Energy TZ4
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_APPARENT_ENERGY),
                TAG_ENUM, OBIS_UNIT_APPARENT_ENERGY_VA_HOUR,    // Unit of Apparent Energy TZ5
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_APPARENT_ENERGY),
                TAG_ENUM, OBIS_UNIT_APPARENT_ENERGY_VA_HOUR,    // Unit of Apparent Energy TZ6
//            TAG_STRUCTURE, 2,
//                TAG_INT8, INJECT8(SCALER_APPARENT_ENERGY),
//                TAG_ENUM, OBIS_UNIT_APPARENT_ENERGY_VA_HOUR,    // Unit of Apparent Energy TZ7
//            TAG_STRUCTURE, 2,
//                TAG_INT8, INJECT8(SCALER_APPARENT_ENERGY),
//                TAG_ENUM, OBIS_UNIT_APPARENT_ENERGY_VA_HOUR,    // Unit of Apparent Energy TZ8
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_ACTIVE_POWER),
                TAG_ENUM, OBIS_UNIT_ACTIVE_POWER_WATT,          // Unit of MD kW
//            TAG_STRUCTURE, 2,
//                TAG_INT8, INJECT8(SCALER_ACTIVE_POWER),
//                TAG_ENUM, OBIS_UNIT_ACTIVE_POWER_WATT,          // Unit of MD kW TZ1
//            TAG_STRUCTURE, 2,
//                TAG_INT8, INJECT8(SCALER_ACTIVE_POWER),
//                TAG_ENUM, OBIS_UNIT_ACTIVE_POWER_WATT,          // Unit of MD kW TZ2
//            TAG_STRUCTURE, 2,
//                TAG_INT8, INJECT8(SCALER_ACTIVE_POWER),
//                TAG_ENUM, OBIS_UNIT_ACTIVE_POWER_WATT,          // Unit of MD kW TZ3
//            TAG_STRUCTURE, 2,
//                TAG_INT8, INJECT8(SCALER_ACTIVE_POWER),
//                TAG_ENUM, OBIS_UNIT_ACTIVE_POWER_WATT,          // Unit of MD kW TZ4
//            TAG_STRUCTURE, 2,
//                TAG_INT8, INJECT8(SCALER_ACTIVE_POWER),
//                TAG_ENUM, OBIS_UNIT_ACTIVE_POWER_WATT,          // Unit of MD kW TZ5
//            TAG_STRUCTURE, 2,
//                TAG_INT8, INJECT8(SCALER_ACTIVE_POWER),
//                TAG_ENUM, OBIS_UNIT_ACTIVE_POWER_WATT,          // Unit of MD kW TZ6
//            TAG_STRUCTURE, 2,
//                TAG_INT8, INJECT8(SCALER_ACTIVE_POWER),
//                TAG_ENUM, OBIS_UNIT_ACTIVE_POWER_WATT,          // Unit of MD kW TZ7
//            TAG_STRUCTURE, 2,
//                TAG_INT8, INJECT8(SCALER_ACTIVE_POWER),
//                TAG_ENUM, OBIS_UNIT_ACTIVE_POWER_WATT,          // Unit of MD kW TZ8
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_APPARENT_POWER),
                TAG_ENUM, OBIS_UNIT_APPARENT_POWER_VA,          // Unit of MD kVA
//            TAG_STRUCTURE, 2,
//                TAG_INT8, INJECT8(SCALER_APPARENT_POWER),
//                TAG_ENUM, OBIS_UNIT_APPARENT_POWER_VA,          // Unit of MD kVA TZ1
//            TAG_STRUCTURE, 2,
//                TAG_INT8, INJECT8(SCALER_APPARENT_POWER),
//                TAG_ENUM, OBIS_UNIT_APPARENT_POWER_VA,          // Unit of MD kVA TZ2
//            TAG_STRUCTURE, 2,
//                TAG_INT8, INJECT8(SCALER_APPARENT_POWER),
//                TAG_ENUM, OBIS_UNIT_APPARENT_POWER_VA,          // Unit of MD kVA TZ3
//            TAG_STRUCTURE, 2,
//                TAG_INT8, INJECT8(SCALER_APPARENT_POWER),
//                TAG_ENUM, OBIS_UNIT_APPARENT_POWER_VA,          // Unit of MD kVA TZ4
//            TAG_STRUCTURE, 2,
//                TAG_INT8, INJECT8(SCALER_APPARENT_POWER),
//                TAG_ENUM, OBIS_UNIT_APPARENT_POWER_VA,          // Unit of MD kVA TZ5
//            TAG_STRUCTURE, 2,
//                TAG_INT8, INJECT8(SCALER_APPARENT_POWER),
//                TAG_ENUM, OBIS_UNIT_APPARENT_POWER_VA,          // Unit of MD kVA TZ6
//            TAG_STRUCTURE, 2,
//                TAG_INT8, INJECT8(SCALER_APPARENT_POWER),
//                TAG_ENUM, OBIS_UNIT_APPARENT_POWER_VA,          // Unit of MD kVA TZ7
//            TAG_STRUCTURE, 2,
//                TAG_INT8, INJECT8(SCALER_APPARENT_POWER),
//                TAG_ENUM, OBIS_UNIT_APPARENT_POWER_VA,          // Unit of MD kVA TZ8
            TAG_STRUCTURE, 2,
                TAG_INT8, 0,
                TAG_ENUM, OBIS_UNIT_TIME_MINUTE,                // cumulative power on duration
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_ACTIVE_ENERGY),
                TAG_ENUM, OBIS_UNIT_ACTIVE_ENERGY_WATT_HOUR,    // Unit of Active Energy expo
            TAG_STRUCTURE, 2,
                TAG_INT8, INJECT8(SCALER_APPARENT_ENERGY),
                TAG_ENUM, OBIS_UNIT_APPARENT_ENERGY_VA_HOUR,    // Unit of Apparent Energy expo
//            TAG_STRUCTURE, 2,
//                TAG_INT8, INJECT8(SCALER_REACTIVE_ENERGY),
//                TAG_ENUM, OBIS_UNIT_REACTIVE_ENERGY_VAR_HOUR,    // Unit of Reactive Energy Q1
//            TAG_STRUCTURE, 2,
//                TAG_INT8, INJECT8(SCALER_REACTIVE_ENERGY),
//                TAG_ENUM, OBIS_UNIT_REACTIVE_ENERGY_VAR_HOUR,    // Unit of Reactive Energy Q2
//            TAG_STRUCTURE, 2,
//                TAG_INT8, INJECT8(SCALER_REACTIVE_ENERGY),
//                TAG_ENUM, OBIS_UNIT_REACTIVE_ENERGY_VAR_HOUR,    // Unit of Reactive Energy Q3
//            TAG_STRUCTURE, 2,
//                TAG_INT8, INJECT8(SCALER_REACTIVE_ENERGY),
//                TAG_ENUM, OBIS_UNIT_REACTIVE_ENERGY_VAR_HOUR,    // Unit of Reactive Energy Q4
};
static const struct attribute_desc_s Obj_Billing_Scaler_Profile[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *) Billing_Scaler_Profile_Buffer, NULL},
  {3, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *) Data_Buffer, Billing_scaler_capture_Objects},
  {4, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Billing_Scaler_Profile_Capture_Period, NULL},
  {5, ACCESS_PC___MRR__USR_, TAG_ENUM,            (void *) &Billing_Scaler_Profile_Sort_Method, NULL},
  {6, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) Common_Sort_Object, NULL},
  {7, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Billing_Scaler_Profile_Entries_In_Use, NULL},
  {8, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Billing_Scaler_Profile_Entries, NULL},
};
//
//// Tamper profile
//uint32_t Tamper_Profile_Entries_In_Use[TOT_EVENT_TYPE];
//uint32_t Tamper_Profile_Entries[TOT_EVENT_TYPE]={60,60,60,60,60,1,60};
const uint32_t Tamper_Profile_Capture_Period = 0;
const uint8_t  Tamper_Profile_Sort_Method = 1;

const uint8_t No_Snap_Tamper_Profile_Capture_Objects[] =
{
    (2*18 + 1),//INJECT16(0x8100 | (2*18 + 1))
        /*TAG_ARRAY,*/ 2,
    TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_CLOCK),
                TAG_OCTET_STRING, 6, 0, 0, 1, 0, 0, 255, // Date & Time
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),

	TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_DATA),
                TAG_OCTET_STRING, 6, 0, 0, 96, 11, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
};
const  uint8_t Snap_Tamper_Profile_Capture_Objects[] =
{
#ifdef SINGLE_PHASE_METER
    INJECT16(0x8100 | (8*18 + 1)),
        /*TAG_ARRAY,*/ 8,
#else
    INJECT16(0x8100 | (14*18 + 1)),
        /*TAG_ARRAY,*/ 14,
#endif
        TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_CLOCK),
                TAG_OCTET_STRING, 6, 0, 0, 1, 0, 0, 255, // Date & Time
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),

	TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_DATA),
                TAG_OCTET_STRING, 6, 0, 0, 96, 11, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
#ifdef SINGLE_PHASE_METER
        TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
                TAG_OCTET_STRING, 6, 1, 0, 94, 91, 14, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),

	TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
                TAG_OCTET_STRING, 6, 1, 0, 12, 7, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
                
	TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
                TAG_OCTET_STRING, 6, 1, 0, 13, 7, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
#else
	TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
                TAG_OCTET_STRING, 6, 1, 0, 31, 7, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
	TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
                TAG_OCTET_STRING, 6, 1, 0, 51, 7, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
	TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
                TAG_OCTET_STRING, 6, 1, 0, 71, 7, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
	TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
                TAG_OCTET_STRING, 6, 1, 0, 32, 7, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
	TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
                TAG_OCTET_STRING, 6, 1, 0, 52, 7, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
	TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
                TAG_OCTET_STRING, 6, 1, 0, 72, 7, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
	TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
                TAG_OCTET_STRING, 6, 1, 0, 33, 7, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
	TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
                TAG_OCTET_STRING, 6, 1, 0, 53, 7, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
	TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
                TAG_OCTET_STRING, 6, 1, 0, 73, 7, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
#endif
	TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
                TAG_OCTET_STRING, 6, 1, 0, 1, 8, 0, 255, // cumulative Energy KWh
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
	TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
                TAG_OCTET_STRING, 6, 1, 0, 2, 8, 0, 255, // cumulative Energy KWh
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
	TAG_STRUCTURE, 4,
                TAG_UINT16, INJECT16(CLASS_ID_DATA),
                TAG_OCTET_STRING, 6, 0, 0, 94, 91, 0, 255,
                TAG_INT8, 2,
                TAG_UINT16, INJECT16(0),
};

const uint16_t No_Snap_Tamper_Profile_Column_Szs[] = {16,19};
const uint16_t Snap_Tamper_Profile_Column_Szs[] = {16,19,24,29,34,37,40,43,46,49,52,57,62,67};
const uint8_t Snap_Tamper_Profile_Buffer_Template[] =
{
#ifdef SINGLE_PHASE_METER
    STUFF_DATA | TAG_STRUCTURE, 8,
#else
    STUFF_DATA | TAG_STRUCTURE, 14,
#endif
        STUFF_DATA | TAG_OCTET_STRING, 12, ITEM_TAG_DATETIME_TMPR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,//16
        STUFF_DATA | TAG_UINT16, INJECT16(ITEM_TAG_ID_TMPR),                       /* ID */
#ifdef SINGLE_PHASE_METER
        STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_I_MAX_TMPR),                 /* Imax */
        STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_V_TMPR),                     /* V */
        STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_PF_TMPR),                    /* PF */
#else
        STUFF_DATA | TAG_UINT32, INJECT32(ITEM_TAG_IR_TMPR),                       /* Ir */
        STUFF_DATA | TAG_UINT32, INJECT32(ITEM_TAG_IY_TMPR),                       /* Iy */
        STUFF_DATA | TAG_UINT32, INJECT32(ITEM_TAG_IB_TMPR),                       /* Ib */
        STUFF_DATA | TAG_UINT16, INJECT16(ITEM_TAG_VR_TMPR),                       /* Vr */
        STUFF_DATA | TAG_UINT16, INJECT16(ITEM_TAG_VY_TMPR),                       /* Vy */
        STUFF_DATA | TAG_UINT16, INJECT16(ITEM_TAG_VB_TMPR),                       /* Vb */
        STUFF_DATA | TAG_UINT8,  (ITEM_TAG_PFR_TMPR),                              /* PFr */
        STUFF_DATA | TAG_UINT8,  (ITEM_TAG_PFY_TMPR),                              /* PFy */
        STUFF_DATA | TAG_UINT8,  (ITEM_TAG_PFB_TMPR),                              /* PFb */
#endif
        STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_CUM_KWH_TOTAL_TMPR),            /* Active Energy */
        STUFF_DATA | TAG_FLOAT32, INJECT_F32(ITEM_TAG_CUMMULATIVE_KWH_EXPO_VAL_TMPR), /* Active Energy expo*/
        STUFF_DATA | TAG_UINT32, INJECT32(ITEM_TAG_CUMMULATIVE_TAMPER_VAL_TMPR),   /* tamper cnt */
};
const uint8_t No_Snap_Tamper_Profile_Buffer_Template[] =
{
  STUFF_DATA | TAG_STRUCTURE, 2,
    STUFF_DATA | TAG_OCTET_STRING, 12, ITEM_TAG_DATETIME_TMPR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //16
    STUFF_DATA | TAG_UINT16, INJECT16(ITEM_TAG_ID_TMPR)                        /* ID */ //19
};
void Capture_No_Snap_Tamper_Profile_Data(void *data, int direction)
{
    range_entries entries;
    msg_info.total_entries = get_profile_entry_in_use(scan_event_type);
    msg_info.template=No_Snap_Tamper_Profile_Buffer_Template;
    msg_info.sz_template=sizeof(No_Snap_Tamper_Profile_Buffer_Template);

   if(msg_info.total_entries==0)
   {
      msg_info.num_entries=0xffff;
   }
   else
   {
     if(access_selector>0)
     {
        if(access_selector==1)//range
        {
          if((SA_Range[0].Year>=0xffff)||(SA_Range[1].Year>=0xffff))
          {
            msg_info.num_entries=0xffff;
          }
          else
          {
            entries = find_entries_by_range(Time_To_Epoch(SA_Range[0].Year%100, SA_Range[0].Month, SA_Range[0].Day, SA_Range[0].Hr, SA_Range[0].Min, 0),
                                                  Time_To_Epoch(SA_Range[1].Year%100, SA_Range[1].Month, SA_Range[1].Day, SA_Range[1].Hr, SA_Range[1].Min, 0),
                                                  scan_event_type);
          
            msg_info.num_entries=entries.num;
            if(FiFO_LiFO==0)
            {
              msg_info.start_entry=entries.from + 1;
            }
            else
            {
              msg_info.start_entry= msg_info.total_entries - (entries.from + entries.num) + 1;
            }
          }
        }
        else //entry
        {
           if(SA_To_Entry==0)
           {
             msg_info.num_entries=msg_info.total_entries;
             msg_info.start_entry=1;
           }
           else
           {
             if(SA_To_Entry > msg_info.total_entries)
             {
               SA_To_Entry = msg_info.total_entries;
             }
             if(SA_From_Entry > msg_info.total_entries)
             {
               SA_From_Entry = msg_info.total_entries;
             }
           
             msg_info.num_entries=SA_To_Entry-SA_From_Entry+1;
             msg_info.start_entry=SA_From_Entry;
           }
        }
     }
     else
     {
       msg_info.num_entries=msg_info.total_entries;
       msg_info.start_entry=1;
     }
   }
   msg_info.column_szs=No_Snap_Tamper_Profile_Column_Szs;
   if(msg_info.num_entries==0)
   {
     msg_info.num_entries=0xFFFF;
   }
}
void Capture_Snap_Tamper_Profile_Data(void *data, int direction)
{
   range_entries entries;
   msg_info.total_entries = get_profile_entry_in_use(scan_event_type);
   msg_info.template=Snap_Tamper_Profile_Buffer_Template;
   msg_info.sz_template=sizeof(Snap_Tamper_Profile_Buffer_Template);

   if(msg_info.total_entries==0)
   {
      msg_info.num_entries=0xffff;
   }
   else
   {
     if(access_selector>0)
     {
        if(access_selector==1)//range
        {
          if((SA_Range[0].Year>=0xffff)||(SA_Range[1].Year>=0xffff))
          {
            msg_info.num_entries=0xffff;
          }
          else
          {
            entries = find_entries_by_range(Time_To_Epoch(SA_Range[0].Year%100, SA_Range[0].Month, SA_Range[0].Day, SA_Range[0].Hr, SA_Range[0].Min, 0),
                                                  Time_To_Epoch(SA_Range[1].Year%100, SA_Range[1].Month, SA_Range[1].Day, SA_Range[1].Hr, SA_Range[1].Min, 0),
                                                  scan_event_type);
          
            msg_info.num_entries=entries.num;
            if(FiFO_LiFO==0)
            {
              msg_info.start_entry=entries.from + 1;
            }
            else
            {
              msg_info.start_entry= msg_info.total_entries - (entries.from + entries.num) + 1;
            }
          }
        }
        else //entry
        {
           if(SA_To_Entry==0)
           {
             msg_info.num_entries=msg_info.total_entries;
             msg_info.start_entry=1;
           }
           else
           {
             if(SA_To_Entry > msg_info.total_entries)
             {
               SA_To_Entry = msg_info.total_entries;
             }
             if(SA_From_Entry > msg_info.total_entries)
             {
               SA_From_Entry = msg_info.total_entries;
             }
           
             msg_info.num_entries=SA_To_Entry-SA_From_Entry+1;
             msg_info.start_entry=SA_From_Entry;
           }
        }
     }
     else
     {
       msg_info.num_entries=msg_info.total_entries;
       msg_info.start_entry=1;
     }
   }

   msg_info.column_szs=Snap_Tamper_Profile_Column_Szs;
   if(msg_info.num_entries==0)
   {
     msg_info.num_entries=0xFFFF;
   }
}
//
// Voltage Tamper profile
void Capture_Voltage_Tamper_Profile_Data(void *data, int direction)
{
  scan_event_type=PROFILE_TYPE_VOLTAGE_RELATED;
  Capture_Snap_Tamper_Profile_Data(data, direction);
}

void Capture_Voltage_Tamper_Profile_Objects(void *data, int direction)
{
  uint16_t size;

  size=sizeof(Snap_Tamper_Profile_Capture_Objects);
  memcpy(Data_Buffer,Snap_Tamper_Profile_Capture_Objects,size);
  Data_Buffer[32]=0;
}
void Voltage_Event_Profile_Entries_In_Use_Function(void *data, int direction)
{
  tmp_uint32 = get_profile_entry_in_use(PROFILE_TYPE_VOLTAGE_RELATED);
}
const uint32_t Voltage_Tamper_Profile_Total_Entries = EVENT_MAX_ENTRIES_VOLTAGE ;
static const struct attribute_desc_s Obj_Voltage_Tamper_Profile[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {0x80+2, ACCESS_PC___MRR__USR_, TAG_ARRAY,      (void *) Data_Buffer, Capture_Voltage_Tamper_Profile_Data},
  {3, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *) Data_Buffer, Capture_Voltage_Tamper_Profile_Objects}, //(void *) Snap_Tamper_Profile_Capture_Objects, NULL},
  {4, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Tamper_Profile_Capture_Period, NULL},
  {5, ACCESS_PC___MRR__USR_, TAG_ENUM,            (void *) &Tamper_Profile_Sort_Method, NULL},
  {6, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) Common_Sort_Object, NULL},
  {7, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &tmp_uint32, Voltage_Event_Profile_Entries_In_Use_Function},
  {8, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Voltage_Tamper_Profile_Total_Entries, NULL},
};

// Current Tamper profile
void Capture_Current_Tamper_Profile_Data(void *data, int direction)
{
  scan_event_type=PROFILE_TYPE_CURRENT_RELATED;
  Capture_Snap_Tamper_Profile_Data(data, direction);
}
void Capture_Current_Tamper_Profile_Objects(void *data, int direction)
{
  uint16_t size;


  size=sizeof(Snap_Tamper_Profile_Capture_Objects);
  memcpy(Data_Buffer,Snap_Tamper_Profile_Capture_Objects,size);
  Data_Buffer[32]=1;
}
void Current_Event_Profile_Entries_In_Use_Function(void *data, int direction)
{
  tmp_uint32 = get_profile_entry_in_use(PROFILE_TYPE_CURRENT_RELATED);
}
const uint32_t Current_Tamper_Profile_Total_Entries = EVENT_MAX_ENTRIES_CURRENT ;
static const struct attribute_desc_s Obj_Current_Tamper_Profile[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {0x80+2, ACCESS_PC___MRR__USR_, TAG_ARRAY,      (void *) Data_Buffer, Capture_Current_Tamper_Profile_Data},
  {3, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *) Data_Buffer, Capture_Current_Tamper_Profile_Objects}, //(void *) Snap_Tamper_Profile_Capture_Objects, NULL},
  {4, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Tamper_Profile_Capture_Period, NULL},
  {5, ACCESS_PC___MRR__USR_, TAG_ENUM,            (void *) &Tamper_Profile_Sort_Method, NULL},
  {6, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) Common_Sort_Object, NULL},
  {7, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &tmp_uint32, Current_Event_Profile_Entries_In_Use_Function},
  {8, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Current_Tamper_Profile_Total_Entries, NULL},
};

// Power Tamper profile
void Capture_Power_Fail_Tamper_Profile_Data(void *data, int direction)
{
  scan_event_type=PROFILE_TYPE_POWER_RELATED;
  Capture_No_Snap_Tamper_Profile_Data(data, direction);
}
void Capture_Power_Fail_Tamper_Profile_Objects(void *data, int direction)
{
  uint16_t size;

  size=sizeof(No_Snap_Tamper_Profile_Capture_Objects);
  memcpy(Data_Buffer,No_Snap_Tamper_Profile_Capture_Objects,size);
  Data_Buffer[31]=2;
}
void Power_Event_Profile_Entries_In_Use_Function(void *data, int direction)
{
  tmp_uint32 = get_profile_entry_in_use(PROFILE_TYPE_POWER_RELATED);
}
const uint32_t Power_Tamper_Profile_Total_Entries = EVENT_MAX_ENTRIES_POWER ;
static const struct attribute_desc_s Obj_Power_Fail_Tamper_Profile[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {0x80+2, ACCESS_PC___MRR__USR_, TAG_ARRAY,      (void *) Data_Buffer, Capture_Power_Fail_Tamper_Profile_Data},
  {3, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *) Data_Buffer, Capture_Power_Fail_Tamper_Profile_Objects}, //(void *) No_Snap_Tamper_Profile_Capture_Objects, NULL},
  {4, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Tamper_Profile_Capture_Period, NULL},
  {5, ACCESS_PC___MRR__USR_, TAG_ENUM,            (void *) &Tamper_Profile_Sort_Method, NULL},
  {6, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) Common_Sort_Object, NULL},
  {7, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &tmp_uint32, Power_Event_Profile_Entries_In_Use_Function},
  {8, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Power_Tamper_Profile_Total_Entries, NULL},
};
//
// Transaction Tamper profile
void Capture_Transact_Tamper_Profile_Data(void *data, int direction)
{
  scan_event_type=PROFILE_TYPE_TRANSACTION_RELATED;
  Capture_No_Snap_Tamper_Profile_Data(data, direction);
}
void Capture_Transact_Tamper_Profile_Objects(void *data, int direction)
{
  uint16_t size;

  size=sizeof(No_Snap_Tamper_Profile_Capture_Objects);
  memcpy(Data_Buffer,No_Snap_Tamper_Profile_Capture_Objects,size);
  Data_Buffer[31]=3;
}
void Transaction_Event_Profile_Entries_In_Use_Function(void *data, int direction)
{
  tmp_uint32 = get_profile_entry_in_use(PROFILE_TYPE_TRANSACTION_RELATED);
}
const uint32_t Transaction_Tamper_Profile_Total_Entries = EVENT_MAX_ENTRIES_TRANS ;
static const struct attribute_desc_s Obj_Transact_Tamper_Profile[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {0x80+2, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *) Data_Buffer, Capture_Transact_Tamper_Profile_Data},
  {3, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *) Data_Buffer, Capture_Transact_Tamper_Profile_Objects}, //(void *) No_Snap_Tamper_Profile_Capture_Objects, NULL},
  {4, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Tamper_Profile_Capture_Period, NULL},
  {5, ACCESS_PC___MRR__USR_, TAG_ENUM,            (void *) &Tamper_Profile_Sort_Method, NULL},
  {6, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) Common_Sort_Object, NULL},
  {7, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &tmp_uint32, Transaction_Event_Profile_Entries_In_Use_Function},
  {8, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Transaction_Tamper_Profile_Total_Entries, NULL},
};

// other Tamper profile
void Capture_Other_Tamper_Profile_Data(void *data, int direction)
{
  scan_event_type=PROFILE_TYPE_OTHERS;
  Capture_Snap_Tamper_Profile_Data(data, direction);
}
void Capture_Other_Tamper_Profile_Objects(void *data, int direction)
{
  uint16_t size;

  size=sizeof(Snap_Tamper_Profile_Capture_Objects);
  memcpy(Data_Buffer,Snap_Tamper_Profile_Capture_Objects,size);
  Data_Buffer[32]=4;
}
void Other_Event_Profile_Entries_In_Use_Function(void *data, int direction)
{
  tmp_uint32 = get_profile_entry_in_use(PROFILE_TYPE_OTHERS);
}
const uint32_t Other_Tamper_Profile_Total_Entries = EVENT_MAX_ENTRIES_OTHER ;
static const struct attribute_desc_s Obj_Other_Tamper_Profile[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {0x80+2, ACCESS_PC___MRR__USR_, TAG_ARRAY,      (void *) Data_Buffer, Capture_Other_Tamper_Profile_Data},
  {3, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *) Data_Buffer, Capture_Other_Tamper_Profile_Objects}, //(void *) Snap_Tamper_Profile_Capture_Objects, NULL},
  {4, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Tamper_Profile_Capture_Period, NULL},
  {5, ACCESS_PC___MRR__USR_, TAG_ENUM,            (void *) &Tamper_Profile_Sort_Method, NULL},
  {6, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) Common_Sort_Object, NULL},
  {7, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &tmp_uint32, Other_Event_Profile_Entries_In_Use_Function},
  {8, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Other_Tamper_Profile_Total_Entries, NULL},
};

//Non roll over profile
void Capture_Non_Roll_Tamper_Profile_Data(void *data, int direction)
{
  scan_event_type=PROFILE_TYPE_NON_ROLLOVER_EVENTS;
  Capture_No_Snap_Tamper_Profile_Data(data, direction);
}
void Capture_Non_Roll_Tamper_Profile_Objects(void *data, int direction)
{
  uint16_t size;

  size=sizeof(No_Snap_Tamper_Profile_Capture_Objects);
  memcpy(Data_Buffer,No_Snap_Tamper_Profile_Capture_Objects,size);
  Data_Buffer[31]=5;
}
void No_Roll_Event_Profile_Entries_In_Use_Function(void *data, int direction)
{
  tmp_uint32 = get_profile_entry_in_use(PROFILE_TYPE_NON_ROLLOVER_EVENTS);
}
const uint32_t No_Roll_Tamper_Profile_Total_Entries = EVENT_MAX_ENTRIES_NON_ROLLER ;
static const struct attribute_desc_s Obj_Non_Roll_Tamper_Profile[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {0x80+2, ACCESS_PC___MRR__USR_, TAG_ARRAY,      (void *) Data_Buffer, Capture_Non_Roll_Tamper_Profile_Data},
  {3, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *) Data_Buffer, Capture_Non_Roll_Tamper_Profile_Objects}, //(void *) No_Snap_Tamper_Profile_Capture_Objects, NULL},
  {4, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Tamper_Profile_Capture_Period, NULL},
  {5, ACCESS_PC___MRR__USR_, TAG_ENUM,            (void *) &Tamper_Profile_Sort_Method, NULL},
  {6, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) Common_Sort_Object, NULL},
  {7, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &tmp_uint32, No_Roll_Event_Profile_Entries_In_Use_Function},
  {8, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &No_Roll_Tamper_Profile_Total_Entries, NULL},
};
//
// Connect disconnet object
void Capture_Control_Tamper_Profile_Data(void *data, int direction)
{
  scan_event_type=PROFILE_TYPE_CONTROL_EVENTS;
  Capture_No_Snap_Tamper_Profile_Data(data, direction);
}
void Capture_Control_Tamper_Profile_Objects(void *data, int direction)
{
  uint16_t size;

  size=sizeof(No_Snap_Tamper_Profile_Capture_Objects);
  memcpy(Data_Buffer,No_Snap_Tamper_Profile_Capture_Objects,size);
  Data_Buffer[31]=6;
}
void Con_Discon_Event_Profile_Entries_In_Use_Function(void *data, int direction)
{
  tmp_uint32 = get_profile_entry_in_use(PROFILE_TYPE_CONTROL_EVENTS);
}
const uint32_t Con_Discon_Tamper_Profile_Total_Entries = EVENT_MAX_ENTRIES_CONTROL;
static const struct attribute_desc_s Obj_Control_Tamper_Profile[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {0x80+2, ACCESS_PC___MRR__USR_, TAG_ARRAY,      (void *) Data_Buffer, Capture_Control_Tamper_Profile_Data},
  {3, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *) Data_Buffer, Capture_Control_Tamper_Profile_Objects}, //(void *) No_Snap_Tamper_Profile_Capture_Objects, NULL},
  {4, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Tamper_Profile_Capture_Period, NULL},
  {5, ACCESS_PC___MRR__USR_, TAG_ENUM,            (void *) &Tamper_Profile_Sort_Method, NULL},
  {6, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) Common_Sort_Object, NULL},
  {7, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &tmp_uint32, Con_Discon_Event_Profile_Entries_In_Use_Function},
  {8, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Con_Discon_Tamper_Profile_Total_Entries, NULL},
};
//
// Tamper profile scaler
const uint32_t Tamper_Scaler_Profile_Entries_In_Use = 1;
const uint32_t Tamper_Scaler_Profile_Entries = 1;
const uint32_t Tamper_Scaler_Profile_Capture_Period = 0;
const uint8_t Tamper_Scaler_Profile_Sort_Method = 1;

void Tamper_scaler_capture_Objects(void *data, int direction)
{
  uint16_t size;
  uint8_t i;
  #ifdef SINGLE_PHASE_METER
   #define TOTAL_OBJ 5
  #else
   #define TOTAL_OBJ 11
  #endif
  
  Data_Buffer[0]=0x81;
  Data_Buffer[1]=(TOTAL_OBJ*18) + 1;
  Data_Buffer[2]=TOTAL_OBJ;
  size=TOTAL_OBJ*18;
  
  memcpy(&Data_Buffer[3],&Snap_Tamper_Profile_Capture_Objects[39],size);
  for(i=0;i<TOTAL_OBJ;i++)
  {
    Data_Buffer[17+(i*18)]=3;
  }
}

const uint8_t Tamper_Scaler_Profile_Buffer[] =
{
#ifdef SINGLE_PHASE_METER
  5*6 + 2 +1,
    1,
      TAG_STRUCTURE, 5,
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_CURRENT),
          TAG_ENUM, OBIS_UNIT_CURRENT_AMPERE,             // Unit of Imax
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_VOLTAGE),
          TAG_ENUM, OBIS_UNIT_VOLTAGE_VOLT,               // Unit of Volt
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_POWER_FACTOR),
          TAG_ENUM, OBIS_UNIT_UNITLESS_COUNT,             // Unit of PF
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_ACTIVE_ENERGY),
          TAG_ENUM, OBIS_UNIT_ACTIVE_ENERGY_WATT_HOUR,    // Unit of Active Energy
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_ACTIVE_ENERGY),
          TAG_ENUM, OBIS_UNIT_ACTIVE_ENERGY_WATT_HOUR,    // Unit of Active Energy expo
#else
  11*6 + 2 +1,
    1,
      TAG_STRUCTURE, 11,
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_CURRENT),
          TAG_ENUM, OBIS_UNIT_CURRENT_AMPERE,             // Unit of Ir
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_CURRENT),
          TAG_ENUM, OBIS_UNIT_CURRENT_AMPERE,             // Unit of Iy
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_CURRENT),
          TAG_ENUM, OBIS_UNIT_CURRENT_AMPERE,             // Unit of Ib
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_VOLTAGE),
          TAG_ENUM, OBIS_UNIT_VOLTAGE_VOLT,               // Unit of Vr
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_VOLTAGE),
          TAG_ENUM, OBIS_UNIT_VOLTAGE_VOLT,               // Unit of Vy
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_VOLTAGE),
          TAG_ENUM, OBIS_UNIT_VOLTAGE_VOLT,               // Unit of Vb
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_POWER_FACTOR),
          TAG_ENUM, OBIS_UNIT_UNITLESS_COUNT,               // Unit of PFr
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_POWER_FACTOR),
          TAG_ENUM, OBIS_UNIT_UNITLESS_COUNT,               // Unit of PFy
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_POWER_FACTOR),
          TAG_ENUM, OBIS_UNIT_UNITLESS_COUNT,               // Unit of PFb
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_ACTIVE_ENERGY),
          TAG_ENUM, OBIS_UNIT_ACTIVE_ENERGY_WATT_HOUR,    // Unit of Active Energy
        TAG_STRUCTURE, 2,
          TAG_INT8, INJECT8(SCALER_ACTIVE_ENERGY),
          TAG_ENUM, OBIS_UNIT_ACTIVE_ENERGY_WATT_HOUR,    // Unit of Active Energy expo
          
#endif //SINGLE_PHASE_METER else
};
static const struct attribute_desc_s Obj_Tamper_Scaler_Profile[] =
{
    {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
    {2, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *) Tamper_Scaler_Profile_Buffer, NULL},
    {3, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *) Data_Buffer, Tamper_scaler_capture_Objects},
    {4, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Tamper_Scaler_Profile_Capture_Period, NULL},
    {5, ACCESS_PC___MRR__USR_, TAG_ENUM,            (void *) &Tamper_Scaler_Profile_Sort_Method, NULL},
    {6, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void *) Common_Sort_Object, NULL},
    {7, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Tamper_Scaler_Profile_Entries_In_Use, NULL},
    {8, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &Tamper_Scaler_Profile_Entries, NULL},
};
//
// Name Plate
const uint32_t Name_Plate_Profile_Entries_In_Use = 1;
const uint32_t Name_Plate_Profile_Entries = 1;
const uint32_t Name_Plate_Profile_Capture_Period = 0;
const uint8_t Name_Plate_Profile_Sort_Method = 1;
const uint8_t Name_Plate_Profile_Capture_Objects[] =
{
  0x82,
    INJECT16(8 * 18 + 1),//    INJECT16(0x8100 | (9*18 + 1)),
    /*TAG_ARRAY,*/ 8,
      TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_DATA),
        TAG_OCTET_STRING, 6, 0, 0, 96, 1, 0, 255, // Serial Number
        TAG_INT8, 2,
        TAG_UINT16, INJECT16(0),
      TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_DATA),
        TAG_OCTET_STRING, 6, 0, 0, 96, 1, 1, 255, //Manufacturer Name
        TAG_INT8, 2,
        TAG_UINT16, INJECT16(0),
      TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_DATA),
        TAG_OCTET_STRING, 6, 0, 0, 96, 1, 2, 255, // Device ID
        TAG_INT8, 2,
        TAG_UINT16, INJECT16(0),
      TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_DATA),
        TAG_OCTET_STRING, 6, 1, 0, 0, 2, 0, 255, //FW Version
        TAG_INT8, 2,
        TAG_UINT16, INJECT16(0),
      TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_DATA),
        TAG_OCTET_STRING, 6, 0, 0, 94, 91, 9, 255, //Meter Type
        TAG_INT8, 2,
        TAG_UINT16, INJECT16(0),
      TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_DATA),
        TAG_OCTET_STRING, 6, 0, 0, 94, 91, 11, 255, //Category
        TAG_INT8, 2,
        TAG_UINT16, INJECT16(0),
      TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_DATA),
        TAG_OCTET_STRING, 6, 0, 0, 94, 91, 12, 255, //Current Rating
        TAG_INT8, 2,
        TAG_UINT16, INJECT16(0),
      TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_DATA),
        TAG_OCTET_STRING, 6, 0, 0, 96, 1, 4, 255, //Year of Manufacture
        TAG_INT8, 2,
        TAG_UINT16, INJECT16(0),
//      TAG_STRUCTURE, 4,
//        TAG_UINT16, INJECT16(CLASS_ID_DATA),
//        TAG_OCTET_STRING, 6, 1, 0, 0, 4, 2, 255, //CT
//        TAG_INT8, 2,
//        TAG_UINT16, INJECT16(0),
//      TAG_STRUCTURE, 4,
//        TAG_UINT16, INJECT16(CLASS_ID_DATA),
//        TAG_OCTET_STRING, 6, 1, 0, 0, 4, 3, 255, //PT
//        TAG_INT8, 2,
//        TAG_UINT16, INJECT16(0),
//      TAG_STRUCTURE, 4,
//        TAG_UINT16, INJECT16(CLASS_ID_DATA),
//        TAG_OCTET_STRING, 6, 0, 0, 96, 1, 5, 255, //NICsr
//        TAG_INT8, 2,
//        TAG_UINT16, INJECT16(0),
//      TAG_STRUCTURE, 4,
//        TAG_UINT16, INJECT16(CLASS_ID_DATA),
//        TAG_OCTET_STRING, 6, 0, 0, 96, 1, 6, 255, //SIMnu
//        TAG_INT8, 2,
//        TAG_UINT16, INJECT16(0),
//      TAG_STRUCTURE, 4,
//        TAG_UINT16, INJECT16(CLASS_ID_DATA),
//        TAG_OCTET_STRING, 6, 0, 0, 94, 91, 15, 255, //V_Rate
//        TAG_INT8, 2,
//        TAG_UINT16, INJECT16(0),
//      TAG_STRUCTURE, 4,
//        TAG_UINT16, INJECT16(CLASS_ID_DATA),
//        TAG_OCTET_STRING, 6, 1, 0, 0, 3, 0, 255, //MeterC
//        TAG_INT8, 2,
//        TAG_UINT16, INJECT16(0)
};
const uint16_t Name_Plate_Profile_Column_Szs[] = { 14,32,46,58,60,64,75,78,80,82,89 };
const uint8_t Name_Plate_Profile_Buffer_Template[] =
{
  STUFF_DATA | TAG_STRUCTURE, 8,
#ifdef SINGLE_PHASE_METER
   STUFF_DATA | TAG_VISIBLE_STRING, 9, ITEM_TAG_SLNO, 0, 0, 0, 0, 0, 0, 0, 0,
#else
   STUFF_DATA | TAG_VISIBLE_STRING, 10, ITEM_TAG_SLNO, 0, 0, 0, 0, 0, 0, 0, 0, 0,
#endif
   STUFF_DATA | TAG_VISIBLE_STRING, 12, ITEM_TAG_MANFACT_NAME, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
#ifdef SINGLE_PHASE_METER
   STUFF_DATA | TAG_VISIBLE_STRING, 12, ITEM_TAG_DEVICE_ID, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
#else
   STUFF_DATA | TAG_VISIBLE_STRING, 13, ITEM_TAG_DEVICE_ID, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
#endif
   STUFF_DATA | TAG_VISIBLE_STRING, 9, ITEM_TAG_FW_VERSION, 0, 0, 0, 0, 0, 0, 0, 0,
   STUFF_DATA | TAG_UINT8, ITEM_TAG_METER_TYPE,
   STUFF_DATA | TAG_VISIBLE_STRING, 2, ITEM_TAG_METER_CATEGORY, 0,
#ifdef SINGLE_PHASE_METER
   #ifdef METER_RATING_5_30
     STUFF_DATA | TAG_VISIBLE_STRING, 8, ITEM_TAG_CURRENT_RATING, 0, 0, 0, 0, 0, 0, 0,
   #elif METER_RATING_10_60
     STUFF_DATA | TAG_VISIBLE_STRING, 9, ITEM_TAG_CURRENT_RATING, 0, 0, 0, 0, 0, 0, 0, 0,
   #endif
#endif
#ifdef LTCT_METER
   STUFF_DATA | TAG_VISIBLE_STRING, 8, ITEM_TAG_CURRENT_RATING, 0, 0, 0, 0, 0, 0, 0,
#endif
#ifdef WHOLE_CURRENT_METER
   STUFF_DATA | TAG_VISIBLE_STRING, 9, ITEM_TAG_CURRENT_RATING, 0, 0, 0, 0, 0, 0, 0, 0,
#endif
   STUFF_DATA | TAG_UINT16, INJECT16(ITEM_TAG_MANFACT_YEAR),
//   STUFF_DATA | TAG_UINT8, ITEM_TAG_CT_RATIO,//ct ratio
//   STUFF_DATA | TAG_UINT8, ITEM_TAG_PT_RATIO,//pt ratio
//   STUFF_DATA | TAG_VISIBLE_STRING, 8, ITEM_TAG_NIC_SERIAL, 0, 0, 0, 0, 0, 0, 0,
//   STUFF_DATA | TAG_VISIBLE_STRING, 20, ITEM_TAG_SIM_NUMBER, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//   STUFF_DATA | TAG_VISIBLE_STRING, 4, ITEM_TAG_VOLTAGE_RATE, 0, 0, 0,
//   STUFF_DATA | TAG_UINT16, INJECT16(ITEM_TAG_METERT_CONSTANT),
};
void Capture_Name_Plate_Profile_Data(void* data, int direction)
{
  msg_info.template = Name_Plate_Profile_Buffer_Template;
  msg_info.sz_template = sizeof(Name_Plate_Profile_Buffer_Template);
  msg_info.start_entry = 1;
  msg_info.num_entries = 1;
  msg_info.column_szs = Name_Plate_Profile_Column_Szs;
}
static const struct attribute_desc_s Obj_Name_Plate_Profile[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void*)Data_Buffer, Capture_Name_Plate_Profile_Data},
  {3, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void*)Name_Plate_Profile_Capture_Objects,NULL},
  {4, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void*)&Name_Plate_Profile_Capture_Period, NULL},
  {5, ACCESS_PC___MRR__USR_, TAG_ENUM,            (void*)&Name_Plate_Profile_Sort_Method, NULL},
  {6, ACCESS_PC___MRR__USR_, TAG_STRUCTURE,       (void*)Common_Sort_Object, NULL},
  {7, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void*)&Name_Plate_Profile_Entries_In_Use, NULL},
  {8, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void*)&Name_Plate_Profile_Entries, NULL},
};

////load limiter
const uint8_t Monitored_Load_Object[] =
{
  14,
    3,
      TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
      TAG_OCTET_STRING, 6,1,0,1,7,0,255,
      TAG_INT8, 2
};

const float32_t threshold_emergency_load = 0;

void threshold_active_load_func(void *data, int direction)
{
  union
  {
     uint32_t tmp_uint32;
     float32_t tmp_float;
     
  }un;
  if (direction == ATTR_WRITE)
  {
    un.tmp_uint32 = (((unsigned char*)data)[0]);
    un.tmp_uint32 <<=8;
    un.tmp_uint32 |= (((unsigned char*)data)[1]);
    un.tmp_uint32 <<=8;
    un.tmp_uint32 |= (((unsigned char*)data)[2]);
    un.tmp_uint32 <<=8;
    un.tmp_uint32 |= (((unsigned char*)data)[3]);

    set_over_load_value(un.tmp_float);
    store_event_data(TRANSACT_EVENT, 158, g_RTC_time.epoch);
  }
  else
  {
    tmp_float = get_over_load_value();
  }
}

void min_over_threshold_duration_load_func(void *data, int direction)
{ 
  if (direction == ATTR_WRITE)
  {
    tmp_uint32 = (((unsigned char*)data)[0]);
    tmp_uint32 <<=8;
    tmp_uint32 |= (((unsigned char*)data)[1]);
    tmp_uint32 <<=8;
    tmp_uint32 |= (((unsigned char*)data)[2]);
    tmp_uint32 <<=8;
    tmp_uint32 |= (((unsigned char*)data)[3]);

    set_minimum_over_duration(tmp_uint32);
    set_relay_load_check_time(tmp_uint32);
    //store_event_data(TRANSACT_EVENT, 158, g_RTC_time.epoch);
  }
  else
  {
    tmp_uint32 = get_minimum_over_duration();
  }
}

void min_under_threshold_duration_load_func(void *data, int direction)
{
  if (direction == ATTR_WRITE)
  {
    tmp_uint32 = (((unsigned char*)data)[0]);
    tmp_uint32 <<=8;
    tmp_uint32 |= (((unsigned char*)data)[1]);
    tmp_uint32 <<=8;
    tmp_uint32 |= (((unsigned char*)data)[2]);
    tmp_uint32 <<=8;
    tmp_uint32 |= (((unsigned char*)data)[3]);
    
    set_minimum_under_duration(tmp_uint32);
    //set_relay_normal_disconnect_time(tmp_uint32);
    //store_event_data(TRANSACT_EVENT, 158, g_RTC_time.epoch);
  }
  else
  {
    tmp_uint32 = get_minimum_under_duration();
  }
}

const uint8_t Emergency_Profile_Load_Object[] =
{
  23,
    3,
      TAG_UINT16, INJECT16(1),
      TAG_OCTET_STRING, 12,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x80,0x00,0x00,
      TAG_UINT32, INJECT32(1)
};

const uint8_t Emergency_Profile_ID_Load_Object[] =
{
  4,
    1,
     TAG_UINT16,00,00
};

const uint8_t Emergency_Profile_Active_Load_Object = 0;
const uint8_t limiter_load_profile_Obj_actions[] =
{
  13*2 + 1,
    2,
      2,2,
        TAG_OCTET_STRING, 6, 0,   0,   10,   0,128, 255,
        TAG_UINT16, INJECT16(1),
      2,2,
        TAG_OCTET_STRING, 6, 0,   0,   10,   0,128, 255,
        TAG_UINT16, INJECT16(2),
};
static const struct attribute_desc_s limiter_load_profile_Obj[] =
{
  { 1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,     (void *)object_list[0].instance_id, NULL },
  { 2, ACCESS_PCR__MRR__USR_, TAG_STRUCTURE,        (void *)Monitored_Load_Object, NULL },
  { 3, ACCESS_PCR__MRR__USR_, TAG_FLOAT32,          (void *)&tmp_float, threshold_active_load_func },
  { 4, ACCESS_PCR__MRR__USRW, TAG_FLOAT32,          (void *)&tmp_float, threshold_active_load_func },
  { 5, ACCESS_PCR__MRR__USR_, TAG_FLOAT32,          (void *)&threshold_emergency_load, NULL },
  { 6, ACCESS_PCR__MRR__USRW, TAG_UINT32,           (void *)&tmp_uint32, min_over_threshold_duration_load_func },
  { 7, ACCESS_PCR__MRR__USRW, TAG_UINT32,           (void *)&tmp_uint32, min_under_threshold_duration_load_func },
  { 8, ACCESS_PCR__MRR__USR_, TAG_STRUCTURE,        (void *)Emergency_Profile_Load_Object, NULL },
  { 9, ACCESS_PCR__MRR__USR_, TAG_ARRAY,            (void *)Emergency_Profile_ID_Load_Object, NULL },
  { 10, ACCESS_PCR__MRR__USR_,TAG_BOOLEAN,          (void *)&Emergency_Profile_Active_Load_Object, NULL },
  { 11, ACCESS_PCR__MRR__USR_,TAG_STRUCTURE,        (void *)limiter_load_profile_Obj_actions, NULL },
};

//////current limiter
////
////const uint8_t Monitored_Current_Object[] =
////{
////  14,
////    3,
////      TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
////      TAG_OCTET_STRING, 6,1,0,31,7,0,255,
////      TAG_INT8, 2
////};
////uint32_t threshold_active_Current = 0;
////uint32_t threshold_normal_Current = 0;
////uint32_t threshold_emergency_Current = 0;
////
////void threshold_active_Current_func(void *data, int direction)
////{
////  if (direction == ATTR_WRITE)
////  {
////    threshold_active_Current = *(uint32_t *)data;
////    threshold_active_Current = ((threshold_active_Current >> 24) & 0xFF) | ((threshold_active_Current >> 8) & 0xFF00) | ((threshold_active_Current << 8) & 0xFF0000) | ((threshold_active_Current << 24) & 0xFF000000);
////    if((threshold_active_Current >= OVER_CURRENT_MIN_VAL) && (threshold_active_Current <= OVER_CURRENT_MAX_VAL))
////    {
////      Over_Current_Val = threshold_active_Current;
////      to_eeprom(OVER_CURRENT_VAL_LOC,Over_Current_Val,2);
////    }
////    threshold_active_Current = Over_Current_Val;
////  }
////}
////uint32_t min_over_threshold_duration_Current = 0;
////uint32_t min_under_threshold_duration_Current = 0;
////
////void min_over_threshold_duration_Current_func(void *data, int direction)
////{
////  if (direction == ATTR_WRITE)
////  {
////    min_over_threshold_duration_Current = *(uint32_t *)data;
////    min_over_threshold_duration_Current = ((min_over_threshold_duration_Current >> 24) & 0xFF) | ((min_over_threshold_duration_Current >> 8) & 0xFF00) | ((min_over_threshold_duration_Current << 8) & 0xFF0000) | ((min_over_threshold_duration_Current << 24) & 0xFF000000);
////    if((min_over_threshold_duration_Current >= CONN_DISON_TIME_INTERVAL_CURRENT_MIN_VAL) && (min_over_threshold_duration_Current <= CONN_DISON_TIME_INTERVAL_CURRENT_MAX_VAL))
////    {
////      Conn_Discon_Time_Interval_Current = min_over_threshold_duration_Current;
////      to_eeprom(CONN_TIME_INTERVAL_CURRENT_LOC,Conn_Discon_Time_Interval_Current,2);
////    }
////    min_over_threshold_duration_Current = Conn_Discon_Time_Interval_Current;
////  }
////}
////
////void min_under_threshold_duration_Current_func(void *data, int direction)
////{
////  if (direction == ATTR_WRITE)
////  {
////    min_under_threshold_duration_Current = *(uint32_t *)data;
////    min_under_threshold_duration_Current = ((min_under_threshold_duration_Current >> 24) & 0xFF) | ((min_under_threshold_duration_Current >> 8) & 0xFF00) | ((min_under_threshold_duration_Current << 8) & 0xFF0000) | ((min_under_threshold_duration_Current << 24) & 0xFF000000);
////    if((min_under_threshold_duration_Current >= CONN_TIME_INTERVAL_CURRENT_MIN_VAL) && (min_under_threshold_duration_Current <= CONN_TIME_INTERVAL_CURRENT_MAX_VAL))
////    {
////      Conn_Time_Interval_Current = min_under_threshold_duration_Current;
////      to_eeprom(CONN_DISCON_TIME_INTERVAL_CURRENT_LOC,Conn_Time_Interval_Current,2);
////    }
////    min_under_threshold_duration_Current = Conn_Time_Interval_Current;
////  }
////}
////
////const uint8_t Emergency_Profile_Current_Object[] =
////{
////  23,
////    3,
////      TAG_UINT16, INJECT16(1),
////      TAG_OCTET_STRING, 12,0x07,0xE1,01,01,255,00,00,00,0,0x80,0,0,
////      TAG_UINT32, INJECT32(0)
////};
////
/////*
////const uint8_t Emergency_Profile_Current_Object[] =
////{
////  1,
////    0,
////};
////*/
////const uint8_t Emergency_Profile_ID_Current_Object[] =
////{
////  4,
////    1,
////     TAG_UINT16,00,00
////};
////
/////*
////const uint8_t Executed_Billing_Script[] =
////{
////  11 + 1,
////    2,
////      TAG_OCTET_STRING, 6, 0, 0, 10, 0, 1, 255,
////      TAG_UINT16, INJECT16(1),
////};
////*/
////uint8_t Emergency_Profile_Active_Current_Object = 0;
////const uint8_t limiter_Current_profile_Obj_actions[] =
////{
////  13*2 + 1,
////    2,
////      2,2,
////        TAG_OCTET_STRING, 6, 0, 0, 96, 3, 10, 255,
////        TAG_UINT16, INJECT16(1),
////      2,2,
////        TAG_OCTET_STRING, 6, 0, 0, 96, 3, 10, 255,
////        TAG_UINT16, INJECT16(2),
////};
////static const struct attribute_desc_s limiter_Current_profile_Obj[] =
////{
////  { 1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,     (void *)object_list[0].instance_id, NULL },
////  { 2, ACCESS_PCR__MRR__USR_, TAG_STRUCTURE,        (void *)Monitored_Current_Object, NULL },
////  { 3, ACCESS_PCR__MRR__USRW, TAG_UINT32,           (void *)&threshold_active_Current, threshold_active_Current_func },
////  { 4, ACCESS_PCR__MRR__USR_, TAG_UINT32,           (void *)&threshold_normal_Current, NULL },
////  { 5, ACCESS_PCR__MRR__USR_, TAG_UINT32,           (void *)&threshold_emergency_Current, NULL },
////  { 6, ACCESS_PCR__MRR__USR_, TAG_UINT32,           (void *)&min_over_threshold_duration_Current, min_over_threshold_duration_Current_func },
////  { 7, ACCESS_PCR__MRR__USR_, TAG_UINT32,           (void *)&min_under_threshold_duration_Current, min_under_threshold_duration_Current_func },
////  { 8, ACCESS_PCR__MRR__USR_, TAG_STRUCTURE,        (void *)Emergency_Profile_Current_Object, NULL },
////  { 9, ACCESS_PCR__MRR__USR_, TAG_ARRAY,            (void *)Emergency_Profile_ID_Current_Object, NULL },
////  { 10, ACCESS_PCR__MRR__USR_, TAG_BOOLEAN,         (void *)&Emergency_Profile_Active_Current_Object, NULL },
////  { 11, ACCESS_PCR__MRR__USR_, TAG_STRUCTURE,       (void *)limiter_Current_profile_Obj_actions, NULL },
////};
////
//////
//
//SMO1
void Obj_SMO1_function(void *data, int direction)
{
  uint8_t *Tempdata = data,len,pos = 1;
  if (direction == ATTR_WRITE)
  {
    len = Tempdata[0];
    if(len == 0x81)
    {
      len = Tempdata[1];
      pos = 2;
    }

    if(len <= 128)
    {
        write_page_eeprom(STORAGE_EEPROM_DLMS_SMO1_MESSAGE_ADDR,(unsigned char*)&Tempdata[pos],len);
        if(len != 128)
        {
          write_eeprom(STORAGE_EEPROM_DLMS_SMO1_MESSAGE_ADDR+len,0);
        }
        //Trigger_Byte |= _BV(SMO1_BIT);
    }
  }
  else if (direction == ATTR_READ)
  {
    read_page_eeprom(STORAGE_EEPROM_DLMS_SMO1_MESSAGE_ADDR,&Tempdata[4],128);
    len = strlen((const char*)&Tempdata[4]);
    if(len > 128)
    {
      len = 128;
    }
    if(len < 0x80)
    {
      memcpy(&Tempdata[2],&Tempdata[4],len);
      Tempdata[0] = len+1;
      Tempdata[1] = len;
    }
    else
    {
      Tempdata[0] = 0x81;
      Tempdata[1] = len+2;
      Tempdata[2] = 0x81;
      Tempdata[3] = len;
    }
  }
}
static const struct attribute_desc_s Obj_SMO1[] =
{
  { 1, ACCESS_PC___MRR__USR__IHR_, TAG_OCTET_STRING,    (void *)object_list[0].instance_id, NULL },
  { 2, ACCESS_PC___MRR__USRW_IHRW, TAG_VISIBLE_STRING,    (void *)Data_Buffer, Obj_SMO1_function },
};
//
//// SMO2
//void Obj_SMO2_function(void *data, int direction)
//{
//  uint8_t *Tempdata = data,len,pos = 1,Temp_len;
//  if (direction == ATTR_WRITE)
//  {
//    len = Tempdata[0];
//    if(len == 0x81)
//    {
//      len = Tempdata[1];
//      pos = 2;
//    }
//
//    read_page_eeprom(SMO2_LOC,Data_Buffer,128);
//    Temp_len = strlen((const char*)Data_Buffer);
//    if(len <= 128)
//    {
//      if((memcmp(Data_Buffer,&Tempdata[pos],len) != 0) || (len != Temp_len))
//      {
//        write_page_eeprom(SMO2_LOC,(unsigned char*)&Tempdata[pos],len);
//        if(len != 128)
//          write_eeprom(SMO2_LOC+len,0);
//        Trigger_Byte |= _BV(SMO2_BIT);
//      }
//    }
//  }
//  else if (direction == ATTR_READ)
//  {
//    read_page_eeprom(SMO2_LOC,&Tempdata[4],128);
//    len = strlen((const char*)&Tempdata[4]);
//    if(len > 128)
//    {
//      len = 128;
//    }
//    if(len < 0x80)
//    {
//      memcpy(&Tempdata[2],&Tempdata[4],len);
//      Tempdata[0] = len+1;
//      Tempdata[1] = len;
//    }
//    else
//    {
//      Tempdata[0] = 0x81;
//      Tempdata[1] = len+2;
//      Tempdata[2] = 0x81;
//      Tempdata[3] = len;
//    }
//  }
//}
//static const struct attribute_desc_s Obj_SMO2[] =
//{
//  { 1, ACCESS_PC___MRR__USR__IHR_, TAG_OCTET_STRING,    (void *)object_list[0].instance_id, NULL },
//  { 2, ACCESS_PC___MRR__USRW_IHRW, TAG_VISIBLE_STRING,  (void *)Data_Buffer, Obj_SMO2_function },
//};
//
////TODO:Rakesh
//
//const uint16_t LCD_LOC = 13;
//static const struct attribute_desc_s Obj_LCD_TEST[] =
//{
//  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
//  {2, ACCESS_PC___MRR__USR_, TAG_UINT16,          (void *) &LCD_LOC, NULL}
//};
////
//bypass current val
void bypass_Current_Val_Function(void *data, int direction)
{
  if(direction==ATTR_WRITE)
  {
    tmp_uint32 = (((unsigned char*)data)[0]);
    tmp_uint32 <<=8;
    tmp_uint32 |= (((unsigned char*)data)[1]);
    tmp_uint32 <<=8;
    tmp_uint32 |= (((unsigned char*)data)[2]);
    tmp_uint32 <<=8;
    tmp_uint32 |= (((unsigned char*)data)[3]);
    
    set_over_bypass_value(tmp_uint32);
  }
  else
  {
    tmp_uint32 = get_over_bypass_value();
  }
}

static const struct attribute_desc_s Obj_Ctrl_bypass_Current_Val[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USRW, TAG_UINT32,          (void *) &tmp_uint32, bypass_Current_Val_Function}
};
//
//
//Push related objects
// Push script table
const uint8_t Obj_Push_Script[] =
{
  0x81,
    (5 * 26) + 1,
      5,
      TAG_STRUCTURE, 2,
        TAG_UINT16, INJECT16(1),
        TAG_ARRAY, 1,
          TAG_STRUCTURE, 5,
            TAG_ENUM, 2,
            TAG_UINT16, INJECT16(CLASS_ID_PUSH_SETUP),
            TAG_OCTET_STRING, 6, 0, 0, 25, 9, 0, 255,
            TAG_INT8, 1,
            TAG_INT8, 0,

      TAG_STRUCTURE, 2,
        TAG_UINT16, INJECT16(2),
        TAG_ARRAY, 1,
          TAG_STRUCTURE, 5,
            TAG_ENUM, 2,
            TAG_UINT16, INJECT16(CLASS_ID_PUSH_SETUP),
            TAG_OCTET_STRING, 6, 0, 1, 25, 9, 0, 255,
            TAG_INT8, 1,
            TAG_INT8, 0,

      TAG_STRUCTURE, 2,
        TAG_UINT16, INJECT16(3),
        TAG_ARRAY, 1,
          TAG_STRUCTURE, 5,
            TAG_ENUM, 2,
            TAG_UINT16, INJECT16(CLASS_ID_PUSH_SETUP),
            TAG_OCTET_STRING, 6, 0, 2, 25, 9, 0, 255,
            TAG_INT8, 1,
            TAG_INT8, 0,

      TAG_STRUCTURE, 2,
        TAG_UINT16, INJECT16(4),
        TAG_ARRAY, 1,
          TAG_STRUCTURE, 5,
            TAG_ENUM, 2,
            TAG_UINT16, INJECT16(CLASS_ID_PUSH_SETUP),
            TAG_OCTET_STRING, 6, 0, 3, 25, 9, 0, 255,
            TAG_INT8, 1,
            TAG_INT8, 0,

      TAG_STRUCTURE, 2,
        TAG_UINT16, INJECT16(5),
        TAG_ARRAY, 1,
          TAG_STRUCTURE, 5,
            TAG_ENUM, 2,
            TAG_UINT16, INJECT16(CLASS_ID_PUSH_SETUP),
            TAG_OCTET_STRING, 6, 0, 4, 25, 9, 0, 255,
            TAG_INT8, 1,
            TAG_INT8, 0
};

static const struct attribute_desc_s Obj_Push_Script_Table[] =
{
  { 1, ACCESS_PC___MRR__USR__PHR_, TAG_OCTET_STRING,    (void *)object_list[0].instance_id, NULL },
  { 2, ACCESS_PC___MRR__USR__PHR_, TAG_ARRAY,           (void *)Obj_Push_Script, NULL }
};
//
//// Register monitor 1
//const uint8_t HES_IHD_Actions[] =
//{
//  28 + 1,
//    1,
//      2,2,
//        2,2,
//          TAG_OCTET_STRING, 6, 0,0,10,0,108, 255,
//          TAG_UINT16, INJECT16(2),
//         2,2,
//          TAG_OCTET_STRING, 6, 0,0,10,0,108, 255,
//          TAG_UINT16, INJECT16(2)
//};
//
//void HES_IHD_Thresholds_Func(void *data, int direction)
//{
//  if (direction == ATTR_READ)
//  {
//    uint8_t *Tempdata = data, len;
//    read_page_eeprom(SMO1_LOC,&Tempdata[6],128);
//    len = strlen((const char*)&Tempdata[6]);
//    if(len > 128)
//    {
//      len = 128;
//    }
//    if(len < 0x80)
//    {
//      memcpy(&Tempdata[4],&Tempdata[6],len);
//      Tempdata[0] = len + 3;
//      Tempdata[1] = 1;
//      Tempdata[2] = TAG_VISIBLE_STRING;
//      Tempdata[3] = len;
//    }
//    else
//    {
//      Tempdata[0] = 0x81;
//      Tempdata[1] = len + 4;
//      Tempdata[2] = 1;
//      Tempdata[3] = TAG_VISIBLE_STRING;
//      Tempdata[4] = 0x81;
//      Tempdata[5] = len;
//    }
//  }
//};
//
//const uint8_t HES_IHD_Monitored_value[] =
//{
//  14,
//    3,
//      TAG_UINT16, INJECT16(CLASS_ID_DATA),
//      TAG_OCTET_STRING, 6,0,0,96,13,1,255,
//      TAG_INT8, 2
//};
//
//static const struct attribute_desc_s Obj_HES_IHD_Push_Monitor[] =
//{
//  { 1, ACCESS_PC___MRR__USR__PHR_, TAG_OCTET_STRING,     (void *)object_list[0].instance_id, NULL },
//  { 2, ACCESS_PC___MRR__USR__PHR_, TAG_ARRAY,            (void *)Data_Buffer, HES_IHD_Thresholds_Func },
//  { 3, ACCESS_PC___MRR__USR__PHR_, TAG_STRUCTURE,        (void *)HES_IHD_Monitored_value, NULL },
//  { 4, ACCESS_PC___MRR__USRW_PHRW, TAG_ARRAY,            (void *)HES_IHD_Actions,NULL }
//};
////
//// Register monitor 2
//const uint8_t IHD_HES_Actions[] =
//{
//  28 + 1,
//    1,
//      2,2,
//        2,2,
//          TAG_OCTET_STRING, 6, 0,0,10,0,108, 255,
//          TAG_UINT16, INJECT16(3),
//         2,2,
//          TAG_OCTET_STRING, 6, 0,0,10,0,108, 255,
//          TAG_UINT16, INJECT16(3)
//};
//
//void IHD_HES_Thresholds_Func(void *data, int direction)
//{
//  if (direction == ATTR_READ)
//  {
//    uint8_t *Tempdata = data, len;
//    read_page_eeprom(SMO2_LOC,&Tempdata[6],128);
//    len = strlen((const char*)&Tempdata[6]);
//    if(len > 128)
//    {
//      len = 128;
//    }
//    if(len < 0x80)
//    {
//      memcpy(&Tempdata[4],&Tempdata[6],len);
//      Tempdata[0] = len + 3;
//      Tempdata[1] = 1;
//      Tempdata[2] = TAG_VISIBLE_STRING;
//      Tempdata[3] = len;
//    }
//    else
//    {
//      Tempdata[0] = 0x81;
//      Tempdata[1] = len + 4;
//      Tempdata[2] = 1;
//      Tempdata[3] = TAG_VISIBLE_STRING;
//      Tempdata[4] = 0x81;
//      Tempdata[5] = len;
//    }
//  }
//};
//
//const uint8_t IHD_HES_Monitored_value[] =
//{
//  14,
//    3,
//      TAG_UINT16, INJECT16(CLASS_ID_DATA),
//      TAG_OCTET_STRING, 6,0,0,96,13,2,255,
//      TAG_INT8, 2
//};
//
//static const struct attribute_desc_s Obj_IHD_HES_Push_Monitor[] =
//{
//  { 1, ACCESS_PC___MRR__USR__PHR_, TAG_OCTET_STRING,     (void *)object_list[0].instance_id, NULL },
//  { 2, ACCESS_PC___MRR__USR__PHR_, TAG_ARRAY,            (void *)Data_Buffer, IHD_HES_Thresholds_Func },
//  { 3, ACCESS_PC___MRR__USR__PHR_, TAG_STRUCTURE,        (void *)IHD_HES_Monitored_value, NULL },
//  { 4, ACCESS_PC___MRR__USRW_PHRW, TAG_ARRAY,            (void *)IHD_HES_Actions,NULL }
//};
////
// Action schedule 1
const uint8_t Meter_HES_Push_Script_Type = 5;
const uint8_t Executed_Meter_HES_Push_Script[] =
{
  11 + 1,
    2,
      TAG_OCTET_STRING, 6, 0,0,10,0,108, 255,
      TAG_UINT16, INJECT16(1),
};

void Meter_HES_Push_Script_Execution_Time_func(void *data, int direction)
{
  uint16_t length = 0;
  uint8_t i,*Data = (uint8_t*)data;
  uint8_t template_buffer [] =  { TAG_STRUCTURE, 2,
                                  TAG_OCTET_STRING, 4, 0, 0, 0, 0, //HH,MM,SS,MS
                                  TAG_OCTET_STRING, 5, 255, 255, 255, 255, 255, //YYYY,MON,DAY,WEEKDAY
                                };
  if(direction == ATTR_WRITE)
  {
    st_Instant_push_time.total_actions = Data[0];
    if(st_Instant_push_time.total_actions > INSTANT_MAX_PUSH_SCHEDULE)
    {
      st_Instant_push_time.total_actions = INSTANT_MAX_PUSH_SCHEDULE;
    }
    for(i = 0; i < st_Instant_push_time.total_actions; i++)
    {
      if((Data[1+i*15] == TAG_STRUCTURE) && (Data[2+i*15] == TAG_STRUCTURE))
      {
        st_Instant_push_time.push_time[i].hour = Data[5+i*15];
        st_Instant_push_time.push_time[i].minute = Data[6+i*15];
        st_Instant_push_time.push_time[i].second = Data[7+i*15];
      }
    }
    st_Instant_push_time.last_epoch_push_time = 0;
    st_Instant_push_time.is_push_action = 0;
    set_instant_push_profile_time(&st_Instant_push_time);
  }
  else
  {
    Data[length++] = (st_Instant_push_time.total_actions * 15) + 1;
    Data[length++] = st_Instant_push_time.total_actions;
    for(i = 0; i <  st_Instant_push_time.total_actions; i++)
    {
      memcpy(&Data[length], template_buffer, 15);
      Data[length + 4] = st_Instant_push_time.push_time[i].hour;
      Data[length + 5] = st_Instant_push_time.push_time[i].minute;
      Data[length + 6] = st_Instant_push_time.push_time[i].second;
      length += 15;
    }
  }
}

static const struct attribute_desc_s Obj_Meter_HES_Push_Schedule[] =
{
  { 1, ACCESS_PC___MRR__USR__PHR_, TAG_OCTET_STRING,    (void *)object_list[0].instance_id, NULL },
  { 2, ACCESS_PC___MRR__USR__PHR_, TAG_STRUCTURE,       (void *)Executed_Meter_HES_Push_Script, NULL },
  { 3, ACCESS_PC___MRR__USR__PHR_, TAG_ENUM,            (void *)&Meter_HES_Push_Script_Type, NULL },
  { 4, ACCESS_PC___MRR__USRW_PHRW, TAG_ARRAY,           (void *)Data_Buffer,Meter_HES_Push_Script_Execution_Time_func }
};
//
////// Action schedule 2
////const uint8_t Meter_IHD_Push_Script_Type = 5;
////const uint8_t Executed_Meter_IHD_Push_Script[] =
////{
////  11 + 1,
////    2,
////      TAG_OCTET_STRING, 6, 0,0,10,0,108, 255,
////      TAG_UINT16, INJECT16(4)
////};
////
////const uint8_t Meter_IHD_Push_Script_Execution_Time[] =
////{
////  0x81,
////  12*15+1,
////    12,
////      TAG_STRUCTURE, 2,
////        TAG_OCTET_STRING, 4, 0, 0, 0, 0, //HH,MM,SS,MS
////        TAG_OCTET_STRING, 5, 255, 255, 255, 255, 255, //YYYY,MON,DAY,WEEKDAY
////      TAG_STRUCTURE, 2,
////        TAG_OCTET_STRING, 4, 2, 0, 0, 0, //HH,MM,SS,MS
////        TAG_OCTET_STRING, 5, 255, 255, 255, 255, 255, //YYYY,MON,DAY,WEEKDAY
////      TAG_STRUCTURE, 2,
////        TAG_OCTET_STRING, 4, 4, 0, 0, 0, //HH,MM,SS,MS
////        TAG_OCTET_STRING, 5, 255, 255, 255, 255, 255, //YYYY,MON,DAY,WEEKDAY
////      TAG_STRUCTURE, 2,
////        TAG_OCTET_STRING, 4, 6, 0, 0, 0, //HH,MM,SS,MS
////        TAG_OCTET_STRING, 5, 255, 255, 255, 255, 255, //YYYY,MON,DAY,WEEKDAY
////      TAG_STRUCTURE, 2,
////        TAG_OCTET_STRING, 4, 8, 0, 0, 0, //HH,MM,SS,MS
////        TAG_OCTET_STRING, 5, 255, 255, 255, 255, 255, //YYYY,MON,DAY,WEEKDAY
////      TAG_STRUCTURE, 2,
////        TAG_OCTET_STRING, 4, 10, 0, 0, 0, //HH,MM,SS,MS
////        TAG_OCTET_STRING, 5, 255, 255, 255, 255, 255, //YYYY,MON,DAY,WEEKDAY
////      TAG_STRUCTURE, 2,
////        TAG_OCTET_STRING, 4, 12, 0, 0, 0, //HH,MM,SS,MS
////        TAG_OCTET_STRING, 5, 255, 255, 255, 255, 255, //YYYY,MON,DAY,WEEKDAY
////      TAG_STRUCTURE, 2,
////        TAG_OCTET_STRING, 4, 14, 0, 0, 0, //HH,MM,SS,MS
////        TAG_OCTET_STRING, 5, 255, 255, 255, 255, 255, //YYYY,MON,DAY,WEEKDAY
////      TAG_STRUCTURE, 2,
////        TAG_OCTET_STRING, 4, 16, 0, 0, 0, //HH,MM,SS,MS
////        TAG_OCTET_STRING, 5, 255, 255, 255, 255, 255, //YYYY,MON,DAY,WEEKDAY
////      TAG_STRUCTURE, 2,
////        TAG_OCTET_STRING, 4, 18, 0, 0, 0, //HH,MM,SS,MS
////        TAG_OCTET_STRING, 5, 255, 255, 255, 255, 255, //YYYY,MON,DAY,WEEKDAY
////      TAG_STRUCTURE, 2,
////        TAG_OCTET_STRING, 4, 20, 0, 0, 0, //HH,MM,SS,MS
////        TAG_OCTET_STRING, 5, 255, 255, 255, 255, 255, //YYYY,MON,DAY,WEEKDAY
////      TAG_STRUCTURE, 2,
////        TAG_OCTET_STRING, 4, 22, 0, 0, 0, //HH,MM,SS,MS
////        TAG_OCTET_STRING, 5, 255, 255, 255, 255, 255
////};
////static const struct attribute_desc_s Obj_Meter_IHD_Push_Schedule[] =
////{
////  { 1, ACCESS_PC___MRR__USR__PHR_, TAG_OCTET_STRING,    (void *)object_list[0].instance_id, NULL },
////  { 2, ACCESS_PC___MRR__USR__PHR_, TAG_STRUCTURE,       (void *)Executed_Meter_IHD_Push_Script, NULL },
////  { 3, ACCESS_PC___MRR__USR__PHR_, TAG_ENUM,            (void *)&Meter_IHD_Push_Script_Type, NULL },
////  { 4, ACCESS_PC___MRR__USRW_PHRW, TAG_ARRAY,           (void *)Meter_IHD_Push_Script_Execution_Time,NULL }//TODO:RAKESH
////};
//////
// PUSH profile 1
uint8_t Obj_Meter_HES_Push_send_destination_and_method[] =
{
  7+1,
    3,
      TAG_ENUM, 5,  // HDLC
      TAG_OCTET_STRING, 1, 64, // HDLC
      TAG_ENUM, 0 // A-XDR xDLMS
};
uint8_t Obj_Meter_HES_Push_Communication_window[] =
{
  30+1,
    1,
      TAG_STRUCTURE, 2,
        TAG_OCTET_STRING, 12,255,255,255,255,255,255,255,255,255,0x80,0,0, // start time
        TAG_OCTET_STRING, 12,255,255,255,255,255,255,255,255,255,0x80,0,0  // end time
};
const uint16_t Meter_HES_Push_randomisation_start_interval = 0;
const uint8_t Meter_HES_Push_number_of_retries = 0;
const uint16_t Meter_HES_Push_repetition_delay = 0;
//__near
void Obj_Meter_HES_Push_method_capture(uint8_t *data, uint16_t data_len, uint8_t *response, uint16_t *response_len)
{
  uint8_t LN[6] = {0,0,25,9,0,255};
  uint16_t len = 0;
  uint8_t *Tempdata = response;
  uint8_t temp_string[18], temp_string_len;
  int64_t val;
    
  Tempdata[len++] = TAG_STRUCTURE;
  Tempdata[len++] = 24;

  temp_string_len = get_device_id(temp_string);

  Tempdata[len++] = TAG_OCTET_STRING;  // Device ID
  Tempdata[len++] = temp_string_len;

  memcpy(&Tempdata[len], temp_string, temp_string_len);
  len += temp_string_len;

  Tempdata[len++] = TAG_OCTET_STRING; // Push Setup ID
  Tempdata[len++] = 6;
  memcpy(&Tempdata[len],LN,6);
  len += 6;

  Tempdata[len++] = TAG_OCTET_STRING; // RTC
  Tempdata[len++] = 12;
  Epoch_To_DLMS_Time_With_RTC_Status(g_RTC_time.epoch, temp_string, &g_RTC_time);
  memcpy(&Tempdata[len], temp_string, 12);
  len += 12;

  addFloat32((float32_t)get_ph_voltage(), Data_Buffer,&len); //ITEM_TAG_V

  addFloat32((float32_t)get_signed_ph_current(), Data_Buffer,&len); //ITEM_TAG_Iph

  addFloat32((float32_t)get_signed_neu_current(), Data_Buffer,&len); //ITEM_TAG_Ineu

  addFloat32((float32_t)get_signed_pf(), Data_Buffer,&len); //ITEM_TAG_PF_TOTAL

  addFloat32((float32_t)get_freq(), Data_Buffer,&len); //ITEM_TAG_FREQUENCY

  addFloat32((float32_t)get_tot_kva()/1000, Data_Buffer,&len); //ITEM_TAG_KVA_TOTAL

  addFloat32((float32_t)get_signed_tot_kw()/1000, Data_Buffer,&len); //ITEM_TAG_KW_TOTAL

  addFloat32(((float32_t)get_energy(Active_Imp)/100000), Data_Buffer,&len); //ITEM_TAG_CUM_KWH_TOTAL

  addFloat32(((float32_t)get_energy(Apparent_Imp)/100000), Data_Buffer,&len); //ITEM_TAG_CUM_KVAH_TOTAL

  addFloat32(((float32_t)get_mdkw_value(0)/100000), Data_Buffer,&len); //ITEM_TAG_KW_MAX_DEMAND//

  Tempdata[len++] = TAG_OCTET_STRING; // ITEM_TAG_CUM_LAST_MD_KW_DATETIME//
  Tempdata[len++] = 12;
  Epoch_To_DLMS_Time(get_mdkw_time(0), temp_string);
  memcpy(&Tempdata[len],temp_string,12);
  len += 12;

  addFloat32(((float32_t)get_mdkva_value(0)/100000), Data_Buffer,&len); //ITEM_TAG_KVA_MAX_DEMAND//

  Tempdata[len++] = TAG_OCTET_STRING; // ITEM_TAG_CUM_LAST_MD_KVA_DATETIME//
  Tempdata[len++] = 12;
  Epoch_To_DLMS_Time(get_mdkva_time(0), temp_string);
  memcpy(&Tempdata[len],temp_string,12);
  len += 12;

  Tempdata[len++] = TAG_UINT32;
  Fill_Data_Buffer_LE(get_cumu_power_on_time()/60, 4,Data_Buffer,&len); //ITEM_TAG_CUM_POWER_ON_DURATION

  Tempdata[len++] = TAG_UINT16;
  Fill_Data_Buffer_LE(get_tamper_counts(), 2,Data_Buffer,&len); //ITEM_TAG_CUM_TAMPER_COUNT//

  Tempdata[len++] = TAG_UINT32;
  Fill_Data_Buffer_LE(get_cumulative_bill_count(), 4,Data_Buffer,&len); //ITEM_TAG_CUM_MD_RESET_COUNT//

  Tempdata[len++] = TAG_UINT16;
  Fill_Data_Buffer_LE(from_eeprom(TOTAL_PROGRAMMING_COUNT_LOC, TOTAL_PROGRAMMING_COUNT_SIZE), 2,Data_Buffer,&len); //ITEM_TAG_CUM_PROGRAMMING_COUNT//

  addFloat32(((float32_t)get_energy(Active_Exp)/100000), Data_Buffer,&len); //ITEM_TAG_CUM_KWH_EXPO_TOTAL

  addFloat32(((float32_t)get_energy(Apparent_Exp)/100000), Data_Buffer,&len); //ITEM_TAG_CUM_KVAH_EXPO_TOTAL

  Tempdata[len++] = TAG_BOOLEAN;
  Tempdata[len++] = get_relay_output_state(); //ITEM_TAG_LOAD_STATUS//

  addFloat32(((float32_t)get_over_load_value()), Data_Buffer,&len); //ITEM_TAG_LOAD_LIMIT_VAL//

//  Tempdata[len++] = TAG_UINT32;
//  Fill_Data_Buffer_LE(Cum_Power_Off_Count, 4,Data_Buffer,&len); //ITEM_TAG_NUM_POWER_OFFS
//
//  Tempdata[len++] = TAG_UINT32;
//  Fill_Data_Buffer_LE(power_off, 4,Data_Buffer,&len); //ITEM_TAG_CUM_POWER_ON_DURATION
//
//  Tempdata[len++] = TAG_UINT32;
//  Fill_Data_Buffer_LE(all_tamper_cnt, 4,Data_Buffer,&len); //ITEM_TAG_CUM_TAMPER_COUNT//
//
//  Tempdata[len++] = TAG_UINT32;
//  Fill_Data_Buffer_LE(md_reset_cnt, 4,Data_Buffer,&len); //ITEM_TAG_CUM_MD_RESET_COUNT//
//
//  Tempdata[len++] = TAG_UINT32;
//  Fill_Data_Buffer_LE(Cum_Prog_Count, 4,Data_Buffer,&len); //ITEM_TAG_CUM_PROGRAMMING_COUNT//
//
//  Tempdata[len++] = TAG_OCTET_STRING; // ITEM_TAG_CUM_LAST_MD_EVENT_DATETIME//
//  Tempdata[len++] = 12;
//  memcpy(&Tempdata[len],&Next_MD_Rst_DT[2],12);
//  len += 12;
//
//  Tempdata[len++] = TAG_UINT32;
//  Fill_Data_Buffer_LE(((float32_t)get_mdkw_value(0))/100000, 4,Data_Buffer,&len); //ITEM_TAG_KW_MAX_DEMAND//
//
//  Tempdata[len++] = TAG_OCTET_STRING; // ITEM_TAG_CUM_LAST_MD_KW_DATETIME//
//  Tempdata[len++] = 12;
//  get_MD_KW_date(1);
//  memcpy(&Tempdata[len],&stBilling_Profile.MD_KW_DT,12);
//  len += 12;
//
//  Tempdata[len++] = TAG_UINT32;
//  Fill_Data_Buffer_LE(((float32_t)get_mdkva_value(0))/100000, 4,Data_Buffer,&len); //ITEM_TAG_KVA_MAX_DEMAND//
//
//  Tempdata[len++] = TAG_OCTET_STRING; // ITEM_TAG_CUM_LAST_MD_KVA_DATETIME//
//  Tempdata[len++] = 12;
//  get_MD_KVA_date();
//  memcpy(&Tempdata[len],&stBilling_Profile.MD_KVA_DT,12);
//  len += 12;
//


  *response_len = len;
}

static const struct method_desc_s Obj_Meter_HES_Push_method[] =
{
  { 1, ACCESS_PC___MR___US__, Obj_Meter_HES_Push_method_capture},
};

const __far uint8_t Meter_HES_Push_Profile_Obj[] =
{
  0x82,
  INJECT16((24 * 18 + 1)),
  24,
    TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_DATA),
        TAG_OCTET_STRING, 6, 0, 0, 96, 1, 2, 255,    // Device ID
        TAG_INT8, 2,
        TAG_UINT16, INJECT16(0),
    TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_PUSH_SETUP),
        TAG_OCTET_STRING, 6, 0, 0, 25, 9, 0, 255,          //Push setup 1
        TAG_INT8, 1,
        TAG_UINT16, INJECT16(0),
    TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_CLOCK),
        TAG_OCTET_STRING, 6, 0, 0, 1, 0, 0, 255,    // Date & Time
        TAG_INT8, 2,
        TAG_UINT16, INJECT16(0),
    TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
        TAG_OCTET_STRING, 6, 1, 0, 12, 7, 0, 255,          //phase volt
        TAG_INT8, 2,
        TAG_UINT16, INJECT16(0),
    TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
        TAG_OCTET_STRING, 6, 1, 0, 11, 7, 0, 255,          //phase current
        TAG_INT8, 2,
        TAG_UINT16, INJECT16(0),
    TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
        TAG_OCTET_STRING, 6, 1, 0, 91, 7, 0, 255,          //neutral current
        TAG_INT8, 2,
        TAG_UINT16, INJECT16(0),
    TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
        TAG_OCTET_STRING, 6, 1, 0, 13, 7, 0, 255,          //frequency
        TAG_INT8, 2,
        TAG_UINT16, INJECT16(0),
    TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
        TAG_OCTET_STRING, 6, 1, 0, 14, 7, 0, 255,          //pf
        TAG_INT8, 2,
        TAG_UINT16, INJECT16(0),
    TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
        TAG_OCTET_STRING, 6, 1, 0, 9, 7, 0, 255,           //kvar
        TAG_INT8, 2,
        TAG_UINT16, INJECT16(0),
    TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
        TAG_OCTET_STRING, 6, 1, 0, 1, 7, 0, 255,           //kw
        TAG_INT8, 2,
        TAG_UINT16, INJECT16(0),
    TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
        TAG_OCTET_STRING, 6, 1, 0, 1, 8, 0, 255,           //c-kwh
        TAG_INT8, 2,
        TAG_UINT16, INJECT16(0),
    TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
        TAG_OCTET_STRING, 6, 1, 0, 9, 8, 0, 255,           //c-kvah
        TAG_INT8, 2,
        TAG_UINT16, INJECT16(0),
    TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_EXTENDED_REGISTER),
        TAG_OCTET_STRING, 6, 1, 0, 1, 6, 0, 255,           //MD-kw-val
        TAG_INT8, 2,
        TAG_UINT16, INJECT16(0),
    TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_EXTENDED_REGISTER),
        TAG_OCTET_STRING, 6, 1, 0, 1, 6, 0, 255,           //MD-kw-time
        TAG_INT8, 5,
        TAG_UINT16, INJECT16(0),
    TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_EXTENDED_REGISTER),
        TAG_OCTET_STRING, 6, 1, 0, 9, 6, 0, 255,           //MD-kva-value
        TAG_INT8, 2,
        TAG_UINT16, INJECT16(0),
    TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_EXTENDED_REGISTER),
        TAG_OCTET_STRING, 6, 1, 0, 9, 6, 0, 255,           //MD-kva-time
        TAG_INT8, 5,
        TAG_UINT16, INJECT16(0),
    TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
        TAG_OCTET_STRING, 6, 1, 0, 94, 91, 14, 255,        //active current
        TAG_INT8, 2,
        TAG_UINT16, INJECT16(0),
    TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_DATA),
        TAG_OCTET_STRING, 6, 0, 0, 94, 91, 0, 255,         //total tamper count
        TAG_INT8, 2,
        TAG_UINT16, INJECT16(0),
    TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_DATA),
        TAG_OCTET_STRING, 6, 0, 0, 0, 1, 0, 255,           //total bill count
        TAG_INT8, 2,
        TAG_UINT16, INJECT16(0),
    TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_DATA),
        TAG_OCTET_STRING, 6, 0, 0, 96, 2, 0, 255,          // Cumulative programming count
        TAG_INT8, 2,
        TAG_UINT16, INJECT16(0),
    TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_REGISTER),           // Cumulative energy, kWh (Export)
        TAG_OCTET_STRING, 6, 1, 0, 2, 8, 0, 255,
        TAG_INT8, 2,
        TAG_UINT16, INJECT16(0),
    TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_REGISTER),           // Cumulative energy, kVAh (Export)
        TAG_OCTET_STRING, 6, 1, 0, 10, 8, 0, 255,
        TAG_INT8, 2,
        TAG_UINT16, INJECT16(0),
    TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_DISCONNECT_CONTROL), // load status
        TAG_OCTET_STRING, 6, 0, 0, 96, 3, 10, 255,
        TAG_INT8, 2,
        TAG_UINT16, INJECT16(0),
    TAG_STRUCTURE, 4,
        TAG_UINT16, INJECT16(CLASS_ID_LIMITER),            // load limit
        TAG_OCTET_STRING, 6, 0, 0, 17, 0, 0, 255,
        TAG_INT8, 3,
        TAG_UINT16, INJECT16(0)
};

void Meter_HES_Push_Profile_Obj_Fucntion(void* data, int direction)
{
  _COM_memcpy_ff(Data_Buffer, Meter_HES_Push_Profile_Obj, sizeof(Meter_HES_Push_Profile_Obj));
}

static const struct attribute_desc_s Obj_Meter_HES_Push_attribute[] =
{
  { 1, ACCESS_PC___MRR__USR__PHR_, TAG_OCTET_STRING,    (void *)object_list[0].instance_id, NULL },
  { 2, ACCESS_PC___MRR__USR__PHR_, TAG_ARRAY,           (void *)Data_Buffer, Meter_HES_Push_Profile_Obj_Fucntion },//Elements to check
  { 3, ACCESS_PC___MRR__USR__PHR_, TAG_STRUCTURE,       (void *)Obj_Meter_HES_Push_send_destination_and_method, NULL },//IP/address to where data to sent
  { 4, ACCESS_PC___MRR__USR__PHR_, TAG_ARRAY,           (void *)Obj_Meter_HES_Push_Communication_window, NULL },//function//data_buff
  { 5, ACCESS_PC___MRR__USR__PHR_, TAG_UINT16,          (void *)&Meter_HES_Push_randomisation_start_interval, NULL },//function to update variable
  { 6, ACCESS_PC___MRR__USR__PHR_, TAG_UINT8,           (void *)&Meter_HES_Push_number_of_retries, NULL },//function to update variable
  { 7, ACCESS_PC___MRR__USR__PHR_, TAG_UINT16,          (void *)&Meter_HES_Push_repetition_delay, NULL },//function to update variable
};
//
//// PUSH profile 2
////uint8_t Obj_HES_IHD_Push_send_destination_and_method[] =
////{
////  7 + 1,
////    3,
////      TAG_ENUM, 5,  // HDLC
////      TAG_OCTET_STRING, 1, 64, // HDLC
////      TAG_ENUM, 0 // A-XDR xDLMS
////};
////uint8_t Obj_HES_IHD_Push_Communication_window[] =
////{
////  30 + 1,
////    1,
////      TAG_STRUCTURE, 2,
////        TAG_OCTET_STRING, 12,255,255,255,255,255,255,255,255,255,0x80,0,0, // start time
////        TAG_OCTET_STRING, 12,255,255,255,255,255,255,255,255,255,0x80,0,0  // end time
////};
////uint16_t HES_IHD_Push_randomisation_start_interval = 300;
////uint8_t HES_IHD_Push_number_of_retries = 0;
////uint16_t HES_IHD_Push_repetition_delay = 0;
////
////void Obj_HES_IHD_Push_method_capture(uint8_t *data, uint16_t data_len, uint8_t *response, uint16_t *response_len)
////{
////  uint8_t LN[6] = {0,1,25,9,0,255};
////  uint8_t len = 0, templen = 0;
////  uint8_t *Tempdata = response;
////  Tempdata[len++] = TAG_STRUCTURE;
////  Tempdata[len++] = 3;
////  Tempdata[len++] = TAG_OCTET_STRING; //RTC
////  Tempdata[len++] = 12;
////  memcpy(&Tempdata[len],&time_string[2],12);
////  len += 12;
////  Tempdata[len++] = TAG_OCTET_STRING; //Push ID
////  Tempdata[len++] = 6;
////  memcpy(&Tempdata[len],LN,6);
////  len += 6;
////  Tempdata[len++] = TAG_VISIBLE_STRING; // SMO1
////
////  read_page_eeprom(SMO1_LOC,&Tempdata[len+2],128);
////  templen = strlen((const char*)&Tempdata[len+2]);
////  if(templen  < 0x80)
////  {
////    Tempdata[len++] = templen;
////    memcpy(&Tempdata[len],&Tempdata[len+1],templen);
////  }
////  else
////  {
////    Tempdata[len++] = 0x81;
////    Tempdata[len++] = templen;
////  }
////  *response_len = templen + len;
////}
////
////static const struct method_desc_s Obj_HES_IHD_Push_method[] =
////{
////  { 1, ACCESS_PC___MR___US__, Obj_HES_IHD_Push_method_capture }
////};
////
////const uint8_t HES_IHD_Push_Profile_Obj[] =
////{
////  (3 * 18 + 1),
////    3,
////      TAG_STRUCTURE, 4,
////        TAG_UINT16, INJECT16(CLASS_ID_CLOCK), // RTC
////        TAG_OCTET_STRING, 6, 0, 0, 1, 0, 0, 255,
////        TAG_INT8, 2,
////        TAG_UINT16, INJECT16(0),
////      TAG_STRUCTURE, 4,
////        TAG_UINT16, INJECT16(CLASS_ID_PUSH_SETUP), //Push setup 2
////        TAG_OCTET_STRING, 6, 0, 1, 25, 9, 0, 255,
////        TAG_INT8, 1,
////        TAG_UINT16, INJECT16(0),
////      TAG_STRUCTURE, 4,
////        TAG_UINT16, INJECT16(CLASS_ID_DATA),
////        TAG_OCTET_STRING, 6, 0, 0, 96, 13, 1, 255,  // SMO 1
////        TAG_INT8, 2,
////        TAG_UINT16, INJECT16(0)
////};
////static const struct attribute_desc_s Obj_HES_IHD_Push_attribute[] =
////{
////  { 1, ACCESS_PC___MRR__USR__PHR_, TAG_OCTET_STRING,    (void *)object_list[0].instance_id, NULL },
////  { 2, ACCESS_PC___MRR__USR__PHR_, TAG_ARRAY,           (void *)HES_IHD_Push_Profile_Obj, NULL },//Elements to check
////  { 3, ACCESS_PC___MRR__USR__PHR_, TAG_STRUCTURE,       (void *)Obj_HES_IHD_Push_send_destination_and_method, NULL },//IP/address to where data to sent
////  { 4, ACCESS_PC___MRR__USR__PHR_, TAG_ARRAY,           (void *)Obj_HES_IHD_Push_Communication_window, NULL },//function//data_buff
////  { 5, ACCESS_PC___MRR__USR__PHR_, TAG_UINT16,          (void *)&HES_IHD_Push_randomisation_start_interval, NULL },//function to update variable
////  { 6, ACCESS_PC___MRR__USR__PHR_, TAG_UINT8,           (void *)&HES_IHD_Push_number_of_retries, NULL },//function to update variable
////  { 7, ACCESS_PC___MRR__USR__PHR_, TAG_UINT16,          (void *)&HES_IHD_Push_repetition_delay, NULL },//function to update variable
////};
////
//// PUSH profile 3
////uint8_t Obj_IHD_HES_Push_send_destination_and_method[] =
////{
////	7 + 1,
////	3,
////	TAG_ENUM, 5,  // HDLC
////	TAG_OCTET_STRING, 1, 64, // HDLC
////	TAG_ENUM, 0 // A-XDR xDLMS
////};
////uint8_t Obj_IHD_HES_Push_Communication_window[] =
////{
////	30 + 1,
////	1,
////	TAG_STRUCTURE, 2,
////		TAG_OCTET_STRING, 12,255,255,255,255,255,255,255,255,255,0x80,0,0, // start time
////		TAG_OCTET_STRING, 12,255,255,255,255,255,255,255,255,255,0x80,0,0  // end time
////};
////uint16_t IHD_HES_Push_randomisation_start_interval = 300;
////uint8_t IHD_HES_Push_number_of_retries = 0;
////uint16_t IHD_HES_Push_repetition_delay = 0;
////
////void Obj_IHD_HES_Push_method_capture(uint8_t *data, uint16_t data_len, uint8_t *response, uint16_t *response_len)
////{
////  uint8_t LN[6] = {0,2,25,9,0,255};
////  uint8_t len = 0, templen = 0;
////  uint8_t *Tempdata = response;
////  Tempdata[len++] = TAG_STRUCTURE;
////  Tempdata[len++] = 4;
////  Tempdata[len++] = TAG_OCTET_STRING;// Device ID
////  Tempdata[len++] = 16;
////  memcpy(&Tempdata[len],&LOGICAL_DEVICE_NAME[2],16);
////  len += 16;
////  Tempdata[len++] = TAG_OCTET_STRING;//Push setup 3
////  Tempdata[len++] = 6;
////  memcpy(&Tempdata[len],LN,6);
////  len += 6;
////  Tempdata[len++] = TAG_OCTET_STRING; //Real Time Clock
////  Tempdata[len++] = 12;
////  memcpy(&Tempdata[len],&time_string[2],12);
////  len += 12;
////  Tempdata[len++] = TAG_VISIBLE_STRING;// SMO 2
////  read_page_eeprom(SMO2_LOC,&Tempdata[len+2],128);
////  templen = strlen((const char*)&Tempdata[len+2]);
////  if(templen < 0x80)
////  {
////    Tempdata[len++] = templen;
////    memcpy(&Tempdata[len],&Tempdata[len+1],templen);
////  }
////  else
////  {
////    Tempdata[len++] = 0x81;
////    Tempdata[len++] = templen;
////  }
////  *response_len = templen + len;
////}
////static const struct method_desc_s Obj_IHD_HES_Push_method[] =
////{
////  { 1, ACCESS_PC___MR___US__, Obj_IHD_HES_Push_method_capture }
////};
////const uint8_t IHD_HES_Push_Profile_Obj[] =
////{
////  (4 * 18 + 1),
////    4,
////      TAG_STRUCTURE, 4,
////        TAG_UINT16, INJECT16(CLASS_ID_DATA),
////        TAG_OCTET_STRING, 6, 0, 0, 96, 1, 2, 255,    // Device ID
////        TAG_INT8, 2,
////        TAG_UINT16, INJECT16(0),
////      TAG_STRUCTURE, 4,
////        TAG_UINT16, INJECT16(CLASS_ID_PUSH_SETUP), //Push setup 1
////        TAG_OCTET_STRING, 6, 0, 2, 25, 9, 0, 255,
////        TAG_INT8, 1,
////        TAG_UINT16, INJECT16(0),
////      TAG_STRUCTURE, 4,
////        TAG_UINT16, INJECT16(CLASS_ID_CLOCK), // RTC
////        TAG_OCTET_STRING, 6, 0, 0, 1, 0, 0, 255,
////        TAG_INT8, 2,
////        TAG_UINT16, INJECT16(0),
////      TAG_STRUCTURE, 4,
////        TAG_UINT16, INJECT16(CLASS_ID_DATA),
////        TAG_OCTET_STRING, 6, 0, 0, 96, 13, 2, 255,  // SMO 2
////        TAG_INT8, 2,
////        TAG_UINT16, INJECT16(0)
////};
////
////static const struct attribute_desc_s Obj_IHD_HES_Push_attribute[] =
////{
////  { 1, ACCESS_PC___MRR__USR__PHR_, TAG_OCTET_STRING,    (void *)object_list[0].instance_id, NULL },
////  { 2, ACCESS_PC___MRR__USR__PHR_, TAG_ARRAY,           (void *)IHD_HES_Push_Profile_Obj, NULL },//Elements to check
////  { 3, ACCESS_PC___MRR__USR__PHR_, TAG_STRUCTURE,       (void *)Obj_IHD_HES_Push_send_destination_and_method, NULL },//IP/address to where data to sent
////  { 4, ACCESS_PC___MRR__USR__PHR_, TAG_ARRAY,           (void *)Obj_IHD_HES_Push_Communication_window, NULL },//function//data_buff
////  { 5, ACCESS_PC___MRR__USR__PHR_, TAG_UINT16,          (void *)&IHD_HES_Push_randomisation_start_interval, NULL },//function to update variable
////  { 6, ACCESS_PC___MRR__USR__PHR_, TAG_UINT8,           (void *)&IHD_HES_Push_number_of_retries, NULL },//function to update variable
////  { 7, ACCESS_PC___MRR__USR__PHR_, TAG_UINT16,          (void *)&IHD_HES_Push_repetition_delay, NULL },//function to update variable
////};
//////
////// PUSH profile 4
////uint8_t Obj_Meter_IHD_Push_send_destination_and_method[] =
////{
////  7 + 1,
////    3,
////      TAG_ENUM, 5,  // HDLC
////      TAG_OCTET_STRING, 1, 64, // HDLC
////      TAG_ENUM, 0 // A-XDR xDLMS
////};
////uint8_t Obj_Meter_IHD_Push_Communication_window[] =
////{
////  30 + 1,
////    1,
////      TAG_STRUCTURE, 2,
////        TAG_OCTET_STRING, 12,255,255,255,255,255,255,255,255,255,0x80,0,0, // start time
////        TAG_OCTET_STRING, 12,255,255,255,255,255,255,255,255,255,0x80,0,0  // end time
////};
////uint16_t Meter_IHD_Push_randomisation_start_interval = 300;
////uint8_t Meter_IHD_Push_number_of_retries = 0;
////uint16_t Meter_IHD_Push_repetition_delay = 0;
////
////void Obj_Meter_IHD_Push_method_capture(uint8_t *data, uint16_t data_len, uint8_t *response, uint16_t *response_len)
////{
////  uint8_t LN[6] = {0,3,25,9,0,255};
////  uint16_t len = 0;
////  uint8_t *Tempdata = response;
////  Tempdata[len++] = TAG_STRUCTURE;
////  Tempdata[len++] = 13;
////
////  Tempdata[len++] = TAG_OCTET_STRING; // RTC
////  Tempdata[len++] = 12;
////  memcpy(&Tempdata[len],&time_string[2],12);
////  len += 12;
////
////  Tempdata[len++] = TAG_OCTET_STRING; // Push Setup ID
////  Tempdata[len++] = 6;
////  memcpy(&Tempdata[len],LN,6);
////  len += 6;
////
////  Tempdata[len++] = TAG_UINT32;
////  Fill_Data_Buffer_LE(load_val[0] - last_load_val[0], 4,Data_Buffer,&len); //ITEM_MONTHLY_KWH
////
////  Tempdata[len++] = TAG_UINT32;
////  Fill_Data_Buffer_LE(irms_reg3[0], 4,Data_Buffer,&len); //ITEM_TAG_IR
////
////  Tempdata[len++] = TAG_UINT32;
////  Fill_Data_Buffer_LE(irms_reg3[1], 4,Data_Buffer,&len); //ITEM_TAG_IY
////
////  Tempdata[len++] = TAG_UINT32;
////  Fill_Data_Buffer_LE(irms_reg3[2], 4,Data_Buffer,&len); //ITEM_TAG_IB
////
////  Tempdata[len++] = TAG_UINT16;
////  Fill_Data_Buffer_LE(vrms_reg3_high_res[0], 2,Data_Buffer,&len); //ITEM_TAG_VR
////
////  Tempdata[len++] = TAG_UINT16;
////  Fill_Data_Buffer_LE(vrms_reg3_high_res[0], 2,Data_Buffer,&len); //ITEM_TAG_VY
////
////  Tempdata[len++] = TAG_UINT16;
////  Fill_Data_Buffer_LE(vrms_reg3_high_res[0], 2,Data_Buffer,&len); //ITEM_TAG_VB
////
////  Tempdata[len++] = TAG_INT16;
////  Fill_Data_Buffer_LE(tot_pf_reg3/10, 2,Data_Buffer,&len); //ITEM_TAG_AvPF
////
////  Tempdata[len++] = TAG_UINT16;
////  Fill_Data_Buffer_LE(MySystemParams.frequency, 2,Data_Buffer,&len); //ITEM_TAG_FREQUENCY
////
////  *response_len = len;
////}
////static const struct method_desc_s Obj_Meter_IHD_Push_method[] =
////{
////  { 1, ACCESS_PC___MR___US__, Obj_Meter_IHD_Push_method_capture },
////};
////const uint8_t Meter_IHD_Push_Profile_Obj[] =
////{
////  0x81,
////    (11 * 18 + 1),
////      11,
////        TAG_STRUCTURE, 4,
////          TAG_UINT16, INJECT16(CLASS_ID_CLOCK), // RTC
////          TAG_OCTET_STRING, 6, 0, 0, 1, 0, 0, 255,
////          TAG_INT8, 2,
////          TAG_UINT16, INJECT16(0),
////        TAG_STRUCTURE, 4,
////          TAG_UINT16, INJECT16(CLASS_ID_PUSH_SETUP), //Push setup 1
////          TAG_OCTET_STRING, 6, 0, 3, 25, 9, 0, 255,
////          TAG_INT8, 1,
////          TAG_UINT16, INJECT16(0),
////        TAG_STRUCTURE, 4,
////          TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
////          TAG_OCTET_STRING, 6, 1, 0, 1, 9, 0, 255, // cumulative kWh of current month
////          TAG_INT8, 2,
////          TAG_UINT16, INJECT16(0),
////        TAG_STRUCTURE, 4,
////          TAG_UINT16, INJECT16(CLASS_ID_REGISTER), // R Ph Current
////          TAG_OCTET_STRING, 6, 1, 0, 31, 7, 0, 255,
////          TAG_INT8, 2,
////          TAG_UINT16, INJECT16(0),
////        TAG_STRUCTURE, 4,
////          TAG_UINT16, INJECT16(CLASS_ID_REGISTER), // Y Ph Current
////          TAG_OCTET_STRING, 6, 1, 0, 51, 7, 0, 255,
////          TAG_INT8, 2,
////          TAG_UINT16, INJECT16(0),
////        TAG_STRUCTURE, 4,
////          TAG_UINT16, INJECT16(CLASS_ID_REGISTER), // B Ph Current
////          TAG_OCTET_STRING, 6, 1, 0, 71, 7, 0, 255,
////          TAG_INT8, 2,
////          TAG_UINT16, INJECT16(0),
////        TAG_STRUCTURE, 4,
////          TAG_UINT16, INJECT16(CLASS_ID_REGISTER), // R Ph Voltage
////          TAG_OCTET_STRING, 6, 1, 0, 32, 7, 0, 255,
////          TAG_INT8, 2,
////          TAG_UINT16, INJECT16(0),
////        TAG_STRUCTURE, 4,
////          TAG_UINT16, INJECT16(CLASS_ID_REGISTER), // Y Ph Voltage
////          TAG_OCTET_STRING, 6, 1, 0, 52, 7, 0, 255,
////          TAG_INT8, 2,
////          TAG_UINT16, INJECT16(0),
////        TAG_STRUCTURE, 4,
////          TAG_UINT16, INJECT16(CLASS_ID_REGISTER), // B Ph Voltage
////          TAG_OCTET_STRING, 6, 1, 0, 72, 7, 0, 255,
////          TAG_INT8, 2,
////          TAG_UINT16, INJECT16(0),
////        TAG_STRUCTURE, 4,
////          TAG_UINT16, INJECT16(CLASS_ID_REGISTER), // Avg PF
////          TAG_OCTET_STRING, 6, 1, 0, 13, 7, 0, 255,
////          TAG_INT8, 2,
////          TAG_UINT16, INJECT16(0),
////        TAG_STRUCTURE, 4,
////          TAG_UINT16, INJECT16(CLASS_ID_REGISTER), // Sys Freq
////          TAG_OCTET_STRING, 6, 1, 0, 14, 7, 0, 255,
////          TAG_INT8, 2,
////          TAG_UINT16, INJECT16(0)
////};
////
////static const struct attribute_desc_s Obj_Meter_IHD_Push_attribute[] =
////{
////  { 1, ACCESS_PC___MRR__USR__PHR_, TAG_OCTET_STRING,    (void *)object_list[0].instance_id, NULL },
////  { 2, ACCESS_PC___MRR__USR__PHR_, TAG_ARRAY,           (void *)Meter_IHD_Push_Profile_Obj, NULL },//Elements to check
////  { 3, ACCESS_PC___MRR__USR__PHR_, TAG_STRUCTURE,       (void *)Obj_Meter_IHD_Push_send_destination_and_method, NULL },//IP/address to where data to sent
////  { 4, ACCESS_PC___MRR__USR__PHR_, TAG_ARRAY,           (void *)Obj_Meter_IHD_Push_Communication_window, NULL },//function//data_buff
////  { 5, ACCESS_PC___MRR__USR__PHR_, TAG_UINT16,          (void *)&Meter_IHD_Push_randomisation_start_interval, NULL },//function to update variable
////  { 6, ACCESS_PC___MRR__USR__PHR_, TAG_UINT8,           (void *)&Meter_IHD_Push_number_of_retries, NULL },//function to update variable
////  { 7, ACCESS_PC___MRR__USR__PHR_, TAG_UINT16,          (void *)&Meter_IHD_Push_repetition_delay, NULL },//function to update variable
////};
//////
// PUSH profile 5
uint8_t Obj_Events_Push_send_destination_and_method[] =
{
  7 + 1,
    3,
      TAG_ENUM, 5,  // HDLC
      TAG_OCTET_STRING, 1, 64, // HDLC
      TAG_ENUM, 0 // A-XDR xDLMS
};
uint8_t Obj_Events_Push_Communication_window[] =
{
  30 + 1,
    1,
      TAG_STRUCTURE, 2,
        TAG_OCTET_STRING, 12,255,255,255,255,255,255,255,255,255,0x80,0,0, // start time
        TAG_OCTET_STRING, 12,255,255,255,255,255,255,255,255,255,0x80,0,0  // end time
};
uint16_t Events_Push_randomisation_start_interval = 300;
uint8_t Events_Push_number_of_retries = 0;
uint16_t Events_Push_repetition_delay = 0;

//Device ID(Logical name) 0.0.96.1.2.255 1/2
//Push setup ID 0.4.25.9.0.255 40/1
//Real Time Clock - Date and Time 0.0.1.0.0.255 8/2
//Event Status word 1 (ESW 1) 0.0.94.91.18.255 1/2
//__near
void Obj_Events_Push_method_capture(uint8_t *data, uint16_t data_len, uint8_t *response, uint16_t *response_len)
{
  uint8_t LN[6] = {0,4,25,9,0,255};
  uint8_t i, len = 0;
  uint8_t temp_string[18], temp_string_len;
  uint8_t *Tempdata = response;
  
  Tempdata[len++] = TAG_STRUCTURE;
  Tempdata[len++] = 4;

  temp_string_len = get_device_id(temp_string);

  Tempdata[len++] = TAG_OCTET_STRING; //Device ID
  Tempdata[len++] = temp_string_len;

  memcpy(&Tempdata[len], temp_string, temp_string_len);
  len += temp_string_len;

  Tempdata[len++] = TAG_OCTET_STRING; //Push setup ID
  Tempdata[len++] = 6;
  memcpy(&Tempdata[len],LN,6);
  len += 6;
  Tempdata[len++] = TAG_OCTET_STRING; //Real Time Clock
  Tempdata[len++] = 12;
  
  Epoch_To_DLMS_Time_With_RTC_Status(g_RTC_time.epoch, temp_string, &g_RTC_time);
  memcpy(&Tempdata[len],temp_string,12);
  len += 12;
  Tempdata[len++] = TAG_BITSTRING; //ESW
  Tempdata[len++] = 0x81;
  Tempdata[len++] = 128;
  for (i = 0; i < 16; i++)
  {
   Tempdata[(len + i)] = 0;
  }
  read_page_eeprom(ESW_WORD_LOC, temp_string, ESW_WORD_SIZE);
  for (i = 0; i < 128; i++)
  {
    Tempdata[((i) / 8) + len] |= ((((temp_string[i / 8] >> i % 8)) & 0x01) << (7 - (i % 8)));
  }
  len += 16;
  *response_len = len;
}
static const struct method_desc_s Obj_Events_Push_method[] =
{
  { 1, ACCESS_PC___MR___US__, Obj_Events_Push_method_capture },
};
const __far uint8_t Events_Push_Profile_Obj[] =
{
    4 * 18 + 1,
      4,
        TAG_STRUCTURE, 4,
          TAG_UINT16, INJECT16(CLASS_ID_DATA),
          TAG_OCTET_STRING, 6, 0, 0, 96, 1, 2, 255, // Device ID
          TAG_INT8, 2,
          TAG_UINT16, INJECT16(0),
        TAG_STRUCTURE, 4,
          TAG_UINT16, INJECT16(CLASS_ID_PUSH_SETUP), //Push setup 1
          TAG_OCTET_STRING, 6, 0, 4, 25, 9, 0, 255,
          TAG_INT8, 1,
          TAG_UINT16, INJECT16(0),
        TAG_STRUCTURE, 4,
          TAG_UINT16, INJECT16(CLASS_ID_CLOCK), // RTC
          TAG_OCTET_STRING, 6, 0, 0, 1, 0, 0, 255,
          TAG_INT8, 2,
          TAG_UINT16, INJECT16(0),
        TAG_STRUCTURE, 4,
          TAG_UINT16, INJECT16(CLASS_ID_DATA), // ESW
          TAG_OCTET_STRING, 6, 0, 0, 94, 91, 18, 255,
          TAG_INT8, 2,
          TAG_UINT16, INJECT16(0)
};

void Events_Push_Profile_Obj_Fucntion(void* data, int direction)
{
  _COM_memcpy_ff(Data_Buffer, Events_Push_Profile_Obj, sizeof(Events_Push_Profile_Obj));
}

static const struct attribute_desc_s Obj_Events_Push_attribute[] =
{
  { 1, ACCESS_PC___MRR__USR__PHR_, TAG_OCTET_STRING,    (void *)object_list[0].instance_id, NULL },
  { 2, ACCESS_PC___MRR__USR__PHR_, TAG_ARRAY,           (void *)Data_Buffer, Events_Push_Profile_Obj_Fucntion },//Elements to check
  { 3, ACCESS_PC___MRR__USR__PHR_, TAG_STRUCTURE,       (void *)Obj_Events_Push_send_destination_and_method, NULL },//IP/address to where data to sent
  { 4, ACCESS_PC___MRR__USR__PHR_, TAG_ARRAY,           (void *)Obj_Events_Push_Communication_window, NULL },//function//data_buff
  { 5, ACCESS_PC___MRR__USR__PHR_, TAG_UINT16,          (void *)&Events_Push_randomisation_start_interval, NULL },//function to update variable
  { 6, ACCESS_PC___MRR__USR__PHR_, TAG_UINT8,           (void *)&Events_Push_number_of_retries, NULL },//function to update variable
  { 7, ACCESS_PC___MRR__USR__PHR_, TAG_UINT16,          (void *)&Events_Push_repetition_delay, NULL },//function to update variable
};
//Extra push profiles
void Obj_load_profile_Push_method_capture(uint8_t *data, uint16_t data_len, uint8_t *response, uint16_t *response_len)
{
  uint8_t temp_string[18], temp_string_len;
  uint8_t LN[6] = {0,5,25,9,0,255};
  uint16_t len = 0;
  uint8_t *Tempdata = response;

  
  Tempdata[len++] = TAG_STRUCTURE;
#ifdef LTCT_METER
  Tempdata[len++] = 20;
#endif
#ifdef WHOLE_CURRENT_METER
  Tempdata[len++] = 16;
#endif
#ifdef SINGLE_PHASE_METER
  Tempdata[len++] = 10;
#endif

  temp_string_len = get_device_id(temp_string);

  Tempdata[len++] = TAG_OCTET_STRING;// Device ID
  Tempdata[len++] = temp_string_len;

  memcpy(&Tempdata[len], temp_string, temp_string_len);
  len += temp_string_len;

  Tempdata[len++] = TAG_OCTET_STRING; // Push Setup ID
  Tempdata[len++] = 6;
  memcpy(&Tempdata[len],LN,6);
  len += 6;

  get_load_profile(&u_s_profile.block_load, 1, e_last_entry);
  
  Tempdata[len++] = TAG_OCTET_STRING; // RTC
  Tempdata[len++] = 12;
  Epoch_To_DLMS_Time(u_s_profile.block_load.u32_epoch, temp_string);
  memcpy(&Tempdata[len],temp_string,12);
  len += 12;
  
#ifdef SINGLE_PHASE_METER

  addFloat32(((float32_t)u_s_profile.block_load.u16_volt)/100, Data_Buffer,&len); //ITEM_TAG_Volt

  addFloat32(((float32_t)u_s_profile.block_load.u32_block_energy_val[LP_Active_Imp])/100000, Data_Buffer,&len); //ITEM_TAG_KWH_TOTAL

  addFloat32(((float32_t)u_s_profile.block_load.u32_block_energy_val[LP_Apparent_Imp])/100000, Data_Buffer,&len); //ITEM_TAG_KVAH_TOTAL

  addFloat32(((float32_t)u_s_profile.block_load.u32_block_energy_val[LP_Active_Exp])/100000, Data_Buffer,&len); //ITEM_TAG_KWH_EXPO_TOTAL

  addFloat32(((float32_t)u_s_profile.block_load.u32_block_energy_val[LP_Apparent_Exp])/100000, Data_Buffer,&len); //ITEM_TAG_KVAH_EXPO_TOTAL

  addFloat32(((float32_t)u_s_profile.block_load.u16_current_phase)/100, Data_Buffer,&len); //ITEM_TAG_Iph

  addFloat32(((float32_t)u_s_profile.block_load.u16_current_neutral)/100, Data_Buffer,&len); //ITEM_TAG_Ineu

#else

  Tempdata[len++] = TAG_UINT32;
  Fill_Data_Buffer_LE(u_s_profile.block_load.u32_current[0], 4,Data_Buffer,&len); //ITEM_TAG_IR

  Tempdata[len++] = TAG_UINT32;
  Fill_Data_Buffer_LE(u_s_profile.block_load.u32_current[1], 4,Data_Buffer,&len); //ITEM_TAG_IY

  Tempdata[len++] = TAG_UINT32;
  Fill_Data_Buffer_LE(u_s_profile.block_load.u32_current[2], 4,Data_Buffer,&len); //ITEM_TAG_IB

  Tempdata[len++] = TAG_UINT16;
  Fill_Data_Buffer_LE(u_s_profile.block_load.u16_volt[0], 2,Data_Buffer,&len); //ITEM_TAG_VR

  Tempdata[len++] = TAG_UINT16;
  Fill_Data_Buffer_LE(u_s_profile.block_load.u16_volt[1], 2,Data_Buffer,&len); //ITEM_TAG_VY

  Tempdata[len++] = TAG_UINT16;
  Fill_Data_Buffer_LE(u_s_profile.block_load.u16_volt[2], 2,Data_Buffer,&len); //ITEM_TAG_VB

  Tempdata[len++] = TAG_UINT8;
  Fill_Data_Buffer_LE(u_s_profile.block_load.u8_pf[0], 1,Data_Buffer,&len); //ITEM_TAG_PFR

  Tempdata[len++] = TAG_UINT8;
  Fill_Data_Buffer_LE(u_s_profile.block_load.u8_pf[1], 1,Data_Buffer,&len); //ITEM_TAG_PFY

  Tempdata[len++] = TAG_UINT8;
  Fill_Data_Buffer_LE(u_s_profile.block_load.u8_pf[2], 1,Data_Buffer,&len); //ITEM_TAG_PFB

  Tempdata[len++] = TAG_UINT32;
  Fill_Data_Buffer_LE(u_s_profile.block_load.u32_block_energy_val[Active_Imp], 4,Data_Buffer,&len); //ITEM_TAG_KWH_TOTAL

  Tempdata[len++] = TAG_UINT32;
  Fill_Data_Buffer_LE(u_s_profile.block_load.u32_block_energy_val[Apparent_Imp], 4,Data_Buffer,&len); //ITEM_TAG_KVAH_TOTAL

#ifdef LTCT_METER  
  Tempdata[len++] = TAG_UINT32;
  Fill_Data_Buffer_LE(u_s_profile.block_load.u32_block_energy_val[Reactive_Ind_Imp], 4,Data_Buffer,&len); //ITEM_TAG_KVARH_Q1_TOTAL

  Tempdata[len++] = TAG_UINT32;
  Fill_Data_Buffer_LE(u_s_profile.block_load.u32_block_energy_val[Reactive_Cap_Exp], 4,Data_Buffer,&len); //ITEM_TAG_KVARH_Q2_TOTAL

  Tempdata[len++] = TAG_UINT32;
  Fill_Data_Buffer_LE(u_s_profile.block_load.u32_block_energy_val[Reactive_Ind_Exp], 4,Data_Buffer,&len); //ITEM_TAG_KVARH_Q3_TOTAL

  Tempdata[len++] = TAG_UINT32;
  Fill_Data_Buffer_LE(u_s_profile.block_load.u32_block_energy_val[Reactive_Cap_Imp], 4,Data_Buffer,&len); //ITEM_TAG_KVARH_Q4_TOTAL
#endif

  Tempdata[len++] = TAG_UINT32;
  Fill_Data_Buffer_LE(u_s_profile.block_load.u32_block_energy_val[Active_Exp], 4,Data_Buffer,&len); //ITEM_TAG_KWH_EXPO_TOTAL

  Tempdata[len++] = TAG_UINT32;
  Fill_Data_Buffer_LE(u_s_profile.block_load.u32_block_energy_val[Apparent_Exp], 4,Data_Buffer,&len); //ITEM_TAG_KVAH_EXPO_TOTAL

#endif //SINGLE_PHASE_METER else
  *response_len = len;
}
void Obj_daily_profile_Push_method_capture(uint8_t *data, uint16_t data_len, uint8_t *response, uint16_t *response_len)
{
  uint8_t temp_string[18], temp_string_len;
  uint8_t LN[6] = {0,6,25,9,0,255};
  uint16_t len = 0;
  
  uint8_t *Tempdata = response;

  get_daily_load_profile(&u_s_profile.daily_load , get_profile_entry_in_use(PROFILE_TYPE_DAILYLOAD) - 1);
  
  Tempdata[len++] = TAG_STRUCTURE;
  Tempdata[len++] = 7;

  temp_string_len = get_device_id(temp_string);

  Tempdata[len++] = TAG_OCTET_STRING;// Device ID
  Tempdata[len++] = temp_string_len;

  memcpy(&Tempdata[len], temp_string, temp_string_len);
  len += temp_string_len;

  Tempdata[len++] = TAG_OCTET_STRING; // Push Setup ID
  Tempdata[len++] = 6;
  memcpy(&Tempdata[len],LN,6);
  len += 6;

  Tempdata[len++] = TAG_OCTET_STRING; // RTC
  Tempdata[len++] = 12;
  Epoch_To_DLMS_Time(u_s_profile.daily_load.u32_epoch, temp_string);
  memcpy(&Tempdata[len],temp_string,12);
  len += 12;

  addFloat32(((float)u_s_profile.daily_load.u64_energy_val[0])/100000, Data_Buffer,&len); //KWH IMPO

  addFloat32(((float)u_s_profile.daily_load.u64_energy_val[1])/100000, Data_Buffer,&len); //KVAH IMPO

  addFloat32(((float)u_s_profile.daily_load.u64_energy_val[2])/100000, Data_Buffer,&len); //KWH EXPO

  addFloat32(((float)u_s_profile.daily_load.u64_energy_val[3])/100000, Data_Buffer,&len); //KVAH EXPO

//  Fill_Data_Buffer_LE(get_mdkw_value(0), 4,Data_Buffer,&len); //ITEM_TAG_KW_MAX_DEMAND//

//  Tempdata[len++] = TAG_OCTET_STRING; // ITEM_TAG_CUM_LAST_MD_KW_DATETIME//
//  Tempdata[len++] = 12;
//  Epoch_To_DLMS_Time(get_mdkw_time(0), temp_string);
//  memcpy(&Tempdata[len],temp_string,12);
//  len += 12;

//  Fill_Data_Buffer_LE(get_mdkva_value(0), 4,Data_Buffer,&len); //ITEM_TAG_KVA_MAX_DEMAND//

//  Tempdata[len++] = TAG_OCTET_STRING; // ITEM_TAG_CUM_LAST_MD_KVA_DATETIME//
//  Tempdata[len++] = 12;
//  Epoch_To_DLMS_Time(get_mdkva_time(0), temp_string);
//  memcpy(&Tempdata[len],temp_string,12);
//  len += 12;

//  Tempdata[len++] = TAG_UINT32;
//  Fill_Data_Buffer_LE(get_over_load_value(), 4,Data_Buffer,&len); //ITEM_TAG_LOAD_LIMIT_VAL//

//  Tempdata[len++] = TAG_UINT32;
//  Fill_Data_Buffer_LE((get_cumu_power_off_time()/60), 4,Data_Buffer,&len); //ITEM_TAG_CUM_POWER_OFF_DURATION

//  Tempdata[len++] = TAG_UINT32;
//  Fill_Data_Buffer_LE(from_eeprom(TOTAL_PROGRAMMING_COUNT_LOC, TOTAL_PROGRAMMING_COUNT_SIZE), 4,Data_Buffer,&len); //ITEM_TAG_NUM_POWER_OFFS

  *response_len = len;
}

void Obj_billing_profile_Push_method_capture(uint8_t *data, uint16_t data_len, uint8_t *response, uint16_t *response_len)
{
  uint8_t temp_string[18], temp_string_len;
  uint8_t LN[6] = {0,7,25,9,0,255};
  uint16_t len = 0;
  uint8_t *Tempdata = response;
  
  get_billing_profile_data(&u_s_profile.billing, 1, 1);
  Tempdata[len++] = TAG_STRUCTURE;
  Tempdata[len++] = 25;

  temp_string_len = get_device_id(temp_string);

  Tempdata[len++] = TAG_OCTET_STRING;// Device ID
  Tempdata[len++] = temp_string_len;

  memcpy(&Tempdata[len], temp_string, temp_string_len);
  len += temp_string_len;

  Tempdata[len++] = TAG_OCTET_STRING; // Push Setup ID
  Tempdata[len++] = 6;
  memcpy(&Tempdata[len],LN,6);
  len += 6;

  Tempdata[len++] = TAG_OCTET_STRING; // RTC
  Tempdata[len++] = 12;
  Epoch_To_DLMS_Time(u_s_profile.billing.u32_epoch, temp_string);
  memcpy(&Tempdata[len],temp_string,12);
  len += 12;

  addFloat32(((float)u_s_profile.billing.u8_Sys_Power_Factor)/100, Data_Buffer,&len); //ITEM_TAG_SPF_BI

  addFloat32(((float)u_s_profile.billing.u64_Cumm_Energy_KWh)/100000, Data_Buffer,&len); //ITEM_TAG_KWH_BI

  addFloat32(((float)u_s_profile.billing.u64_Cumm_Energy_KWh_TZ[0])/100000, Data_Buffer,&len); //ITEM_TAG_KWH_TZ1_BI

  addFloat32(((float)u_s_profile.billing.u64_Cumm_Energy_KWh_TZ[1])/100000, Data_Buffer,&len); //ITEM_TAG_KWH_TZ2_BI

  addFloat32(((float)u_s_profile.billing.u64_Cumm_Energy_KWh_TZ[2])/100000, Data_Buffer,&len); //ITEM_TAG_KWH_TZ3_BI

  addFloat32(((float)u_s_profile.billing.u64_Cumm_Energy_KWh_TZ[3])/100000, Data_Buffer,&len); //ITEM_TAG_KWH_TZ4_BI

  addFloat32(((float)u_s_profile.billing.u64_Cumm_Energy_KWh_TZ[4])/100000, Data_Buffer,&len); //ITEM_TAG_KWH_TZ5_BI

  addFloat32(((float)u_s_profile.billing.u64_Cumm_Energy_KWh_TZ[5])/100000, Data_Buffer,&len); //ITEM_TAG_KWH_TZ6_BI

  addFloat32(((float)u_s_profile.billing.u64_Cumm_Energy_KVAh)/100000, Data_Buffer,&len); //ITEM_TAG_KVAH_BI

  addFloat32(((float)u_s_profile.billing.u64_Cumm_Energy_KVAh_TZ[0])/100000, Data_Buffer,&len); //ITEM_TAG_KVAH_TZ1_BI

  addFloat32(((float)u_s_profile.billing.u64_Cumm_Energy_KVAh_TZ[1])/100000, Data_Buffer,&len); //ITEM_TAG_KVAH_TZ2_BI

  addFloat32(((float)u_s_profile.billing.u64_Cumm_Energy_KVAh_TZ[2])/100000, Data_Buffer,&len); //ITEM_TAG_KVAH_TZ3_BI

  addFloat32(((float)u_s_profile.billing.u64_Cumm_Energy_KVAh_TZ[3])/100000, Data_Buffer,&len); //ITEM_TAG_KVAH_TZ4_BI

  addFloat32(((float)u_s_profile.billing.u64_Cumm_Energy_KVAh_TZ[4])/100000, Data_Buffer,&len); //ITEM_TAG_KVAH_TZ5_BI

  addFloat32(((float)u_s_profile.billing.u64_Cumm_Energy_KVAh_TZ[5])/100000, Data_Buffer,&len); //ITEM_TAG_KVAH_TZ6_BI

  addFloat32(((float)u_s_profile.billing.u32_MD_KW)/100000, Data_Buffer,&len); //ITEM_TAG_MDKW_BI

  Tempdata[len++] = TAG_OCTET_STRING; // ITEM_TAG_CUM_LAST_MD_KW_DATETIME//
  Tempdata[len++] = 12;
  Epoch_To_DLMS_Time(u_s_profile.billing.u32_MD_KW_Epoch, temp_string);
  memcpy(&Tempdata[len],temp_string,12);
  len += 12;

  addFloat32(((float)u_s_profile.billing.u32_MD_KVA)/100000, Data_Buffer,&len); //ITEM_TAG_MDKVA_BI

  Tempdata[len++] = TAG_OCTET_STRING; // ITEM_TAG_CUM_LAST_MD_KVA_DATETIME//
  Tempdata[len++] = 12;
  Epoch_To_DLMS_Time(u_s_profile.billing.u32_MD_KVA_Epoch,temp_string);
  memcpy(&Tempdata[len],temp_string,12);
  len += 12;

  Tempdata[len++] = TAG_UINT32;
  Fill_Data_Buffer_LE(u_s_profile.billing.u32_Pwr_on_duration/60, 4, Data_Buffer,&len); //ITEM_TAG_PWRON_BI

  addFloat32(((float)u_s_profile.billing.u64_Cumm_Energy_KWh_expo)/100000, Data_Buffer,&len); //ITEM_TAG_KWH_EXPO_BI

  addFloat32(((float)u_s_profile.billing.u64_Cumm_Energy_KVAh_expo)/100000, Data_Buffer,&len); //ITEM_TAG_KVAH_EXPO_BI

  *response_len = len;
}

void Obj_event_profile_Push_method_capture(tag_dlms_profile_type_t eventType, uint8_t *data, uint16_t data_len, uint8_t *response, uint16_t *response_len)
{
  uint8_t temp_string[18], temp_string_len;
  uint8_t LN[6] = {0,0,25,9,0,255};
  uint8_t is_data = 1;
  uint16_t len = 0;
  uint8_t *Tempdata = response;
  uint8_t entry;
  LN[1] = eventType + 8;
  
  entry = get_profile_entry_in_use(eventType);
  if(entry == 0)
  {
    *response_len = 0;
    return;
  }
  get_tamper_profile(&u_s_profile.event, eventType, entry - 1);
  
  if((eventType == NOROLL_EVENT) ||
     (eventType == PFAIL_EVENT)||
     (eventType == TRANSACT_EVENT)||
     (eventType == CTRL_EVENT))
  {
    is_data = 0;
  }
  Tempdata[len++] = TAG_STRUCTURE;
  if(is_data)
  {
    Tempdata[len++] = 10;
  }
  else
  {
    Tempdata[len++] = 4;
  }

  temp_string_len = get_device_id(temp_string);

  Tempdata[len++] = TAG_OCTET_STRING;// Device ID
  Tempdata[len++] = temp_string_len;

  memcpy(&Tempdata[len], temp_string, temp_string_len);
  len += temp_string_len;

  Tempdata[len++] = TAG_OCTET_STRING; // Push Setup ID
  Tempdata[len++] = 6;
  memcpy(&Tempdata[len],LN,6);
  len += 6;

  Tempdata[len++] = TAG_OCTET_STRING; // RTC
  Tempdata[len++] = 12;
  Epoch_To_DLMS_Time(u_s_profile.event.u32_epoch, temp_string);
  memcpy(&Tempdata[len],temp_string,12);
  len += 12;

  Tempdata[len++] = TAG_UINT16;
  Fill_Data_Buffer_LE(u_s_profile.event.u16_Tamper_ID, 2,Data_Buffer,&len); //ID
  if(!is_data)
  {
    *response_len = len;
    return;
  }
  
  addFloat32(((float)u_s_profile.event.u32_current_phase)/100, Data_Buffer,&len); //ITEM_TAG_IR

  addFloat32(((float)u_s_profile.event.u16_volt)/100, Data_Buffer,&len); //ITEM_TAG_VR//

  addFloat32(((float)u_s_profile.event.u8_pf)/100, Data_Buffer,&len); //ITEM_TAG_PFR//

  addFloat32(((float)u_s_profile.event.u64_energy_val[0])/100000, Data_Buffer,&len); //KWH IMPO//

  addFloat32(((float)u_s_profile.event.u64_kwh_expo)/100000, Data_Buffer,&len); //KWH EXPO//

  Tempdata[len++] = TAG_UINT32;
  Fill_Data_Buffer_LE(u_s_profile.event.u32_tamper_count, 4,Data_Buffer,&len); //TAMPER COUNT //

  *response_len = len;
}

//void Obj_NIC_HES_Push_method_capture(uint8_t *data, uint16_t data_len, uint8_t *response, uint16_t *response_len)
//{
//  uint8_t LN[6] = {0,0,25,128,0,255};
//  uint16_t len = 0;
//  uint8_t *Tempdata = response;
//
//  Tempdata[len++] = TAG_STRUCTURE;
//  Tempdata[len++] = 4;
//
//  Tempdata[len++] = TAG_OCTET_STRING;// Device ID
//  Tempdata[len++] = 16;
//  memcpy(&Tempdata[len],&LOGICAL_DEVICE_NAME[2],16);
//  len += 16;
//
//  Tempdata[len++] = TAG_OCTET_STRING; // Push Setup ID
//  Tempdata[len++] = 6;
//  memcpy(&Tempdata[len],LN,6);
//  len += 6;
//
//  Tempdata[len++] = TAG_OCTET_STRING; // RTC
//  Tempdata[len++] = 12;
//  memcpy(&Tempdata[len],&time_string[2],12);
//  len += 12;
//
//  Tempdata[len++] = TAG_OCTET_STRING; // Detected NIC address
//  Tempdata[len++] = st_NIC_info.Detected_MAC_address[0];
//  memcpy(&Tempdata[len], &st_NIC_info.Detected_MAC_address[1], st_NIC_info.Detected_MAC_address[0]);
//  len += st_NIC_info.Detected_MAC_address[0];
//  *response_len = len;
//}


void Invoke_Push_msg(gxByteBuffer *bb, st_RTC_time* RTC_time, uint32_t *framecounter)
{
  gxLNParameters p;
  gxByteBuffer reply;
  gxByteBuffer reply_data;
  dlmsSettings client;
  struct tm Time;
  uint16_t response_len = 0, PushSetup_ID = 0;
  uint8_t i;
  uint8_t cipher_key[16];
  uint8_t ded_key[16];
  #ifdef SECURITY_ENABLE
  
  #endif
  cl_init(&client, 1, 0x40/*Target*/, 1/*Source*/, DLMS_AUTHENTICATION_NONE, NULL, DLMS_INTERFACE_TYPE_WRAPPER/*DLMS_INTERFACE_TYPE_HDLC*/);//Management to manegment device.
  client.server = 1;
  
  if((st_tamper.Trigger_Byte & _BV(EVENTS_BIT)) != 0) /* high priority */
  {
    PushSetup_ID = _BV(EVENTS_BIT);
  }
  else
  {
  for(i = 0; i < 16; i++)
  {
    if((st_tamper.Trigger_Byte & _BV(i)) != 0)
    {
      PushSetup_ID = _BV(i);
      break;
    }
  }
  }
  switch(PushSetup_ID)
  {
  case _BV(EVENTS_BIT):
    Obj_Events_Push_method_capture(NULL,0,Data_Buffer,&response_len);
    st_tamper.Trigger_Byte &= ~_BV(EVENTS_BIT);
    break;
  case _BV(INSTANT_PUSH_BIT):
    Obj_Meter_HES_Push_method_capture(NULL,0,Data_Buffer,&response_len);
    st_tamper.Trigger_Byte &= ~_BV(INSTANT_PUSH_BIT);
    break;
//  case _BV(SMO1_BIT):
//    Obj_HES_IHD_Push_method_capture(NULL,0,Data_Buffer,&response_len);
//    st_tamper.Trigger_Byte &= ~_BV(SMO1_BIT);
//    break;
//  case _BV(SMO2_BIT):
//    Obj_IHD_HES_Push_method_capture(NULL,0,Data_Buffer,&response_len);
//    st_tamper.Trigger_Byte &= ~_BV(SMO2_BIT);
//    break;
//  case _BV(IHD_MSG_BIT):
//    Obj_Meter_IHD_Push_method_capture(NULL,0,Data_Buffer,&response_len);
//    st_tamper.Trigger_Byte &= ~_BV(IHD_MSG_BIT);
//    break;
    
  case _BV(LOAD_PROFILE_BIT):
    Obj_load_profile_Push_method_capture(NULL,0,Data_Buffer,&response_len);
    st_tamper.Trigger_Byte &= ~_BV(LOAD_PROFILE_BIT);
    break;
  case _BV(DAILY_PROFILE_BIT):
    Obj_daily_profile_Push_method_capture(NULL,0,Data_Buffer,&response_len);
    st_tamper.Trigger_Byte &= ~_BV(DAILY_PROFILE_BIT);
    break;
  case _BV(BILLING_PROFILE_BIT):
    Obj_billing_profile_Push_method_capture(NULL,0,Data_Buffer,&response_len);
    st_tamper.Trigger_Byte &= ~_BV(BILLING_PROFILE_BIT);
    break;
  case _BV(VOLTAGE_TAMPER_BIT):
    Obj_event_profile_Push_method_capture(PROFILE_TYPE_VOLTAGE_RELATED,NULL,0,Data_Buffer,&response_len);
    st_tamper.Trigger_Byte &= ~_BV(VOLTAGE_TAMPER_BIT);
    break;
  case _BV(CURRENT_TAMPER_BIT):
    Obj_event_profile_Push_method_capture(PROFILE_TYPE_CURRENT_RELATED,NULL,0,Data_Buffer,&response_len);
    st_tamper.Trigger_Byte &= ~_BV(CURRENT_TAMPER_BIT);
    break;
  case _BV(POWER_TAMPER_BIT):
    Obj_event_profile_Push_method_capture(PROFILE_TYPE_POWER_RELATED,NULL,0,Data_Buffer,&response_len);
    st_tamper.Trigger_Byte &= ~_BV(POWER_TAMPER_BIT);
    break;
  case _BV(TRANSACTION_TAMPER_BIT):
    Obj_event_profile_Push_method_capture(PROFILE_TYPE_TRANSACTION_RELATED,NULL,0,Data_Buffer,&response_len);
    st_tamper.Trigger_Byte &= ~_BV(TRANSACTION_TAMPER_BIT);
    break;
  case _BV(OTHER_TAMPER_BIT):
    Obj_event_profile_Push_method_capture(PROFILE_TYPE_OTHERS,NULL,0,Data_Buffer,&response_len);
    st_tamper.Trigger_Byte &= ~_BV(OTHER_TAMPER_BIT);
    break;
  case _BV(NON_ROLLOVER_TAMPER_BIT):
    Obj_event_profile_Push_method_capture(PROFILE_TYPE_NON_ROLLOVER_EVENTS,NULL,0,Data_Buffer,&response_len);
    st_tamper.Trigger_Byte &= ~_BV(NON_ROLLOVER_TAMPER_BIT);
    break;
  case _BV(CONNECT_DISCONNECT_TAMPER_BIT):
    Obj_event_profile_Push_method_capture(PROFILE_TYPE_CONTROL_EVENTS,NULL,0,Data_Buffer,&response_len);
    st_tamper.Trigger_Byte &= ~_BV(CONNECT_DISCONNECT_TAMPER_BIT);
    break;
  }
  if(response_len == 0)
  {  
    cl_clear(&client);
    return;
  }
  Time.tm_year = RTC_time->dateTime.year;
  Time.tm_mon = RTC_time->dateTime.month;
  Time.tm_mday = RTC_time->dateTime.day;
  Time.tm_hour = RTC_time->dateTime.hour;
  Time.tm_min = RTC_time->dateTime.minute;
  Time.tm_sec = RTC_time->dateTime.second;
  Time.tm_deviation = from_eeprom(CLOCK_TIME_ZONE_LOC,CLOCK_TIME_ZONE_SIZE);
  client.SendData = 1;
  client.receiverFrame = 0x10;
  bb_set(bb, Data_Buffer, response_len);
  
#ifdef ENCRYPT_PUSH_MSG
  *framecounter += 1;
  client.cipher.frameCounter = framecounter;
  bb_clear(&client.cipher.systemTitle);
  bb_clear(&client.cipher.authenticationKey);
  bb_clear(&client.cipher.blockCipherKey);
  bb_clear(&client.cipher.dedicatedCipherKey);
  bb_clear(&client.cipher.CipherKey);
  bb_attach(&client.cipher.systemTitle, &server_system_title[2], server_system_title[1]);
  client.cipher.security = DLMS_SECURITY_ENCRYPTION;
  bb_attach(&client.cipher.blockCipherKey, &US_Security_Keys[UNICAST_KEY][0], 16);
  bb_attach(&client.cipher.authenticationKey, &US_Security_Keys[AUTHENTICATION_KEY][0], 16);
  bb_attach(&client.cipher.dedicatedCipherKey, ded_key, sizeof(ded_key));
  bb_attach(&client.cipher.CipherKey, cipher_key, sizeof(cipher_key));
  client.cipher.generalciphering = 1;
#endif
        
  params_initLN(&p, &client, DLMS_COMMAND_DATA_NOTIFICATION, 0, NULL, bb, 0xff);
  if(Time.tm_year != 0)
  {
    p.time = &Time;
  }
  p.lastBlock = 1;
  bb_attach(&reply, Data_Buffer, DATA_BUFFER_SIZE);
  bb_reset(&reply);
  bb_attach(&reply_data, Data_Buffer, DATA_BUFFER_SIZE);
  bb_reset(&reply_data);
  dlms_getLNPdu(&p, &reply);
  if(client.interfaceType == DLMS_INTERFACE_TYPE_WRAPPER)
  {
    dlms_getWrapperFrame(p.settings, &reply, &reply_data, 0);
  }
  else if(client.interfaceType == DLMS_INTERFACE_TYPE_HDLC)
  {
    dlms_getHdlcFrame(p.settings, 0, &reply, &reply_data);
  }
  
  bb_reset(bb);
  bb_set(bb, reply_data.data, reply_data.size);
  client.cipher.generalciphering = 0;
  bb_clear(&reply);
  cl_clear(&client);
  return;
}

void check_and_push_events(st_RTC_time* RTC_time)
{
  gxByteBuffer reply;
  bb_attach(&reply, tx_msg[RF_COMM_PORT].buf.uint8, COMM_BUFFER_SIZE);
  bb_reset(&reply);
  check_push_events(RTC_time->epoch);
  
//#ifndef FACTORY_DEBUG_RF_PUSH_FORCED
//
//  if(is_RF_missing() || (tx_msg[RF_COMM_PORT].Tx_timeout < DEFAULT_TICK_TX_TIME_OUT) || (tx_msg[RF_COMM_PORT].next_msg_send_wait != 0) 
//#ifndef RF_NIC_DISABLE
//      || (!is_RF_comm_ready())
//#endif      //!RF_NIC_DISABLE
//      )
//  {
//    return;
//  }
//#endif

#ifndef FACTORY_DEBUG_RF_PUSH_FORCED
  if (!RF_NET_STAT() || is_RF_missing() || (tx_msg[RF_COMM_PORT].Tx_timeout < DEFAULT_TICK_TX_TIME_OUT) || (tx_msg[RF_COMM_PORT].next_msg_send_wait != 0) || (!is_RF_comm_ready()))
  {
      return;
  }
#endif

  if(st_tamper.Trigger_Byte != 0)
  {
    LCD_DisplaySpSign(S_RX);
    Invoke_Push_msg(&reply, RTC_time, &lnHdlc_uart_port[RF_COMM_PORT].base.cipher.frameCounter_PH);
    if(0 != reply.size)
    {
      RF_comm_skip_time();
      send_message(RF_COMM_PORT, reply.data, reply.size);
    }
    bb_clear(&reply);
  }
  
  if(from_eeprom(STORAGE_EEPROM_DLMS_IMAGE_TRANSFER_ACT_PENDING_ADDR, STORAGE_EEPROM_DLMS_IMAGE_TRANSFER_ACT_PENDING_SIZE) == 1)
  {
    if(from_eeprom(STORAGE_EEPROM_DLMS_IMAGE_UPDATE_EPOCH_ADDR, STORAGE_EEPROM_DLMS_IMAGE_UPDATE_EPOCH_SIZE))
    {
      to_eeprom(STORAGE_EEPROM_DLMS_IMAGE_TRANSFER_ACT_PENDING_ADDR, 0, STORAGE_EEPROM_DLMS_IMAGE_TRANSFER_ACT_PENDING_SIZE);
      Image_Activate(0, 0, 0, 0);
    }
  }
}

//// Image activation action schedule
const uint8_t Image_activation_Script_Type = 1;
const uint8_t Executed_Image_activation_Script[] =
{
  11 + 1,
    2,
      TAG_OCTET_STRING, 6, 0,0,10,0,107, 255,
      TAG_UINT16, INJECT16(1),
};

void Image_activation_Script_Execution_Time_Func(void *data, int direction)
{
  uint8_t *Tempdata = (uint8_t*)data;
  stTime_struct Time;
  uint32_t epoch_time;
  if (direction == ATTR_READ)
  {
    read_page_eeprom(STORAGE_EEPROM_DLMS_IMAGE_UPDATE_EPOCH_ADDR, (uint8_t*)&epoch_time, STORAGE_EEPROM_DLMS_IMAGE_UPDATE_EPOCH_SIZE);
    UnixToDatteTime(epoch_time, &Time);
    Tempdata[0] = 16;
    Tempdata[1] = 1;
    Tempdata[2] = TAG_STRUCTURE;
    Tempdata[3] = 2;
    Tempdata[4] = TAG_OCTET_STRING;
    Tempdata[5] = 4;
    Tempdata[6] = Time.hour;
    Tempdata[7] = Time.minute;
    Tempdata[8] = Time.second;
    Tempdata[9] = 255;
    Tempdata[10] = TAG_OCTET_STRING;
    Tempdata[11] = 5;
    Tempdata[12] = ((Time.year&0xFF00)>>8);
    Tempdata[13] = (Time.year & 0xFF);
    Tempdata[14] = Time.month;
    Tempdata[15] = Time.day;
    Tempdata[16] = Time.weekday;
  }
  if (direction == ATTR_WRITE)
  {
    Time.hour = Tempdata[5];
    Time.minute = Tempdata[6];
    Time.second = Tempdata[7];
    Time.year = Tempdata[11];
    Time.year = (Time.year << 8) | Tempdata[12];
    Time.month = Tempdata[13];
    Time.day = Tempdata[14];
    epoch_time = Time_Convert_TO2TS(&Time);
#ifdef LTCT_METER
    store_event_data(TRANSACT_EVENT, 169, g_RTC_time.epoch);
#endif
    write_page_eeprom(STORAGE_EEPROM_DLMS_IMAGE_UPDATE_EPOCH_ADDR, (uint8_t*)&epoch_time, STORAGE_EEPROM_DLMS_IMAGE_UPDATE_EPOCH_SIZE);
    to_eeprom(STORAGE_EEPROM_DLMS_IMAGE_TRANSFER_ACT_PENDING_ADDR, 1, STORAGE_EEPROM_DLMS_IMAGE_TRANSFER_ACT_PENDING_SIZE);
#ifdef WHOLE_CURRENT_METER
    to_eeprom(TOTAL_PROGRAMMING_COUNT_LOC, from_eeprom(TOTAL_PROGRAMMING_COUNT_LOC, TOTAL_PROGRAMMING_COUNT_SIZE) + 1, TOTAL_PROGRAMMING_COUNT_SIZE);
#endif
  }
}
static const struct attribute_desc_s Obj_Image_activation_Schedule[] =
{
  { 1, ACCESS_PC___MRR__USR__FWR_, TAG_OCTET_STRING,    (void *)object_list[0].instance_id, NULL },
  { 2, ACCESS_PC___MRR__USR__FWR_, TAG_STRUCTURE,       (void *)Executed_Image_activation_Script, NULL },
  { 3, ACCESS_PC___MRR__USR__FWR_, TAG_ENUM,            (void *)&Image_activation_Script_Type, NULL },
  { 4, ACCESS_PC___MRR__USR__FWRW, TAG_ARRAY,           (void *)Data_Buffer,Image_activation_Script_Execution_Time_Func }
};
// Image activation script table
const uint8_t Image_activation_Script[] =
{
  (1 * 26) + 1,
    1,
      TAG_STRUCTURE, 2,
        TAG_UINT16, INJECT16(1),
        TAG_ARRAY, 1,
          TAG_STRUCTURE, 5,
            TAG_ENUM, 2,
            TAG_UINT16, INJECT16(CLASS_ID_IMAGE_TRANSFER),
            TAG_OCTET_STRING, 6, 0, 0, 44, 0, 0, 255,
            TAG_INT8, 4,
            TAG_INT8, 0
};

static const struct attribute_desc_s Obj_Image_activation_Script_Table[] =
{
  { 1, ACCESS_PC___MRR__USR__FWR_, TAG_OCTET_STRING,    (void *)object_list[0].instance_id, NULL },
  { 2, ACCESS_PC___MRR__USR__FWR_, TAG_ARRAY,           (void *)Image_activation_Script, NULL}
};
//


const uint8_t Tariffication_Script[] =
{
  0x81,
  (6 * 26) + 1,
    6,
      TAG_STRUCTURE, 2,
        TAG_UINT16, INJECT16(0),
        TAG_ARRAY, 1,
          TAG_STRUCTURE, 5,
            TAG_ENUM, 2,
            TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
            TAG_OCTET_STRING, 6, 0, 0, 0, 0, 0, 0,
            TAG_INT8, 4,
            TAG_INT8, 0,
      TAG_STRUCTURE, 2,
        TAG_UINT16, INJECT16(1),
        TAG_ARRAY, 1,
          TAG_STRUCTURE, 5,
            TAG_ENUM, 2,
            TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
            TAG_OCTET_STRING, 6, 0, 0, 0, 0, 0, 0,
            TAG_INT8, 4,
            TAG_INT8, 0,
      TAG_STRUCTURE, 2,
        TAG_UINT16, INJECT16(2),
        TAG_ARRAY, 1,
          TAG_STRUCTURE, 5,
            TAG_ENUM, 2,
            TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
            TAG_OCTET_STRING, 6, 0, 0, 0, 0, 0, 0,
            TAG_INT8, 4,
            TAG_INT8, 0,
      TAG_STRUCTURE, 2,
        TAG_UINT16, INJECT16(3),
        TAG_ARRAY, 1,
          TAG_STRUCTURE, 5,
            TAG_ENUM, 2,
            TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
            TAG_OCTET_STRING, 6, 0, 0, 0, 0, 0, 0,
            TAG_INT8, 4,
            TAG_INT8, 0,
      TAG_STRUCTURE, 2,
        TAG_UINT16, INJECT16(4),
        TAG_ARRAY, 1,
          TAG_STRUCTURE, 5,
            TAG_ENUM, 2,
            TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
            TAG_OCTET_STRING, 6, 0, 0, 0, 0, 0, 0,
            TAG_INT8, 4,
            TAG_INT8, 0,
      TAG_STRUCTURE, 2,
        TAG_UINT16, INJECT16(5),
        TAG_ARRAY, 1,
          TAG_STRUCTURE, 5,
            TAG_ENUM, 2,
            TAG_UINT16, INJECT16(CLASS_ID_REGISTER),
            TAG_OCTET_STRING, 6, 0, 0, 0, 0, 0, 0,
            TAG_INT8, 4,
            TAG_INT8, 0,
};

static const struct attribute_desc_s Obj_Tariffication_Script_Table[] =
{
  { 1, ACCESS_PC___MRR__USR__FWR_, TAG_OCTET_STRING,    (void *)object_list[0].instance_id, NULL },
  { 2, ACCESS_PC___MRR__USR__FWR_, TAG_ARRAY,           (void *)Tariffication_Script, NULL}
};
//
// Disconnect connect script table
const uint8_t Obj_Limiter_Action_Script[] =
{
  (2 * 26) + 1,
    2,
      TAG_STRUCTURE, 2,
        TAG_UINT16, INJECT16(1),
        TAG_ARRAY, 1,
          TAG_STRUCTURE, 5,
            TAG_ENUM, 2,
            TAG_UINT16, INJECT16(CLASS_ID_DISCONNECT_CONTROL),
            TAG_OCTET_STRING, 6, 0,   0,  96,   3,  10, 255,
            TAG_INT8, 1,
            TAG_INT8, 0,
      TAG_STRUCTURE, 2,
        TAG_UINT16, INJECT16(2),
        TAG_ARRAY, 1,
          TAG_STRUCTURE, 5,
            TAG_ENUM, 2,
            TAG_UINT16, INJECT16(CLASS_ID_DISCONNECT_CONTROL),
            TAG_OCTET_STRING, 6, 0,   0,  96,   3,  10, 255,
            TAG_INT8, 2,
            TAG_INT8, 0,
};

static const struct attribute_desc_s Obj_Limiter_Action_Script_Table[] =
{
  { 1, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *)object_list[0].instance_id, NULL },
  { 2, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *)Obj_Limiter_Action_Script, NULL}
};

// ESW
void Bit_Array_ESW_Func(void *data, int direction)
{
  uint8_t ESW_Word_128[16];
  
  if(ATTR_READ == direction)
  {
    uint8_t i, *Tempdata = (uint8_t*)data;
    
    read_page_eeprom(ESW_WORD_LOC, ESW_Word_128, ESW_WORD_SIZE);
    memset(Tempdata, 0, 19);

    for (i = 0; i < 128; i++)
    {
      Tempdata[((i) / 8) + 3] |= ((((ESW_Word_128[i / 8] >> i % 8)) & 0x01) << (7 - (i % 8)));
    }
    Tempdata[0] = 18;
    Tempdata[1] = 0x81;
    Tempdata[2] = 128;
  }
}
static const struct attribute_desc_s Obj_ESW[] =
{
  { 1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,       (void *)object_list[0].instance_id, NULL },
  { 2, ACCESS_PC___MRR__USR_, TAG_BITSTRING,          (void *)Data_Buffer, Bit_Array_ESW_Func }
};

void Bit_Array_ESWF_Func(void *data, int direction)
{
  uint8_t i, *Tempdata = (uint8_t*)data,*data_type = (uint8_t*)data;
  uint8_t ESWF_Word_128[16];
  
  read_page_eeprom(ESWF_WORD_LOC, ESWF_Word_128, ESWF_WORD_SIZE);
  if(ATTR_READ == direction)
  {
    memset(Tempdata, 0, 19);
    for (i = 0; i < 128; i++)
    {
      Tempdata[((i) / 8) + 3] |= ((((ESWF_Word_128[i / 8] >> i % 8)) & 0x01) << (7 - (i % 8)));
    }
    Tempdata[0] = 18;
    Tempdata[1] = 0x81;
    Tempdata[2] = 128;
  }
  else if(ATTR_WRITE == direction)
  {
    data_type--;
    if(data_type[0] == 4)
    {
      //if(memcmp(ESWF_Word_128, &Tempdata[2], 11) != 0)
      //{
        memset(ESWF_Word_128, 0, 16);
        for (i = 0; i < 128; i++)
        {
          ESWF_Word_128[i / 8] |= ((((Tempdata[((i) / 8) + 2] >> i % 8))& 0x01) << (7 - (i % 8)));
        }
        write_page_eeprom(ESWF_WORD_LOC, ESWF_Word_128, ESWF_WORD_SIZE);
        store_event_data(TRANSACT_EVENT, 165, g_RTC_time.epoch);
      //}
    }
  }
}
static const struct attribute_desc_s Obj_ESWF[] =
{
  { 1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,       (void *)object_list[0].instance_id, NULL },
  { 2, ACCESS_PC___MRR__USRW, TAG_BITSTRING,          (void *)Data_Buffer, Bit_Array_ESWF_Func }
};

// Metering mode
void Metering_Mode_Func(void *data,int direction)
{
  uint8_t * ptr = (uint8_t*)data;
  
  if(direction == ATTR_WRITE)
  {
    if((get_metering_mode() != *ptr) && (*ptr < 2))
    {
      tmp_uint8 = *ptr;
      set_metering_mode(tmp_uint8);
#if defined WHOLE_CURRENT_METER || defined SINGLE_PHASE_METER
        to_eeprom(TOTAL_TAMPERS_COUNT_LOC, from_eeprom(TOTAL_TAMPERS_COUNT_LOC, TOTAL_TAMPERS_COUNT_SIZE) + 1, TOTAL_TAMPERS_COUNT_SIZE);
        to_eeprom(TOTAL_PROGRAMMING_COUNT_LOC, from_eeprom(TOTAL_PROGRAMMING_COUNT_LOC, TOTAL_PROGRAMMING_COUNT_SIZE) + 1, TOTAL_PROGRAMMING_COUNT_SIZE);
#endif
#ifdef LTCT_METER
      store_event_data(TRANSACT_EVENT, 167, g_RTC_time.epoch);
#endif
      if(tmp_uint8 == 0)
      {
#ifdef LTCT_METER
      store_event_data(TRANSACT_EVENT, 177, g_RTC_time.epoch);
#endif
#if defined WHOLE_CURRENT_METER || defined SINGLE_PHASE_METER
        store_event_data(OTHER_EVENT, 213, g_RTC_time.epoch);
#endif
      }
      else
      {
#ifdef LTCT_METER
      store_event_data(TRANSACT_EVENT, 178, g_RTC_time.epoch);
#endif
#if defined WHOLE_CURRENT_METER || defined SINGLE_PHASE_METER
        store_event_data(OTHER_EVENT, 214, g_RTC_time.epoch);
#endif
      }
    }
  }
  else
  {
    tmp_uint8 = get_metering_mode();
  }
}
static const struct attribute_desc_s Obj_Metering_Mode[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USRW, TAG_UINT8,           (void *) &tmp_uint8, Metering_Mode_Func},
};
//
#if defined WHOLE_CURRENT_METER || defined SINGLE_PHASE_METER
// Payment mode
e_payment_mode_type Payment_Mode = FACTORY_DEFAULT_PAYMENT_MODE;
void Payment_Mode_Func(void *data,int direction)
{
  uint8_t * ptr = data;
  if(direction == ATTR_READ)
  {
    Payment_Mode = (e_payment_mode_type)read_eeprom(PAYMENT_MODE_VAL_LOC);
  }
  else if(direction == ATTR_WRITE)
  {
    if((Payment_Mode != (e_payment_mode_type)*ptr) && (*ptr < 2))
    {
      Payment_Mode = (e_payment_mode_type)*ptr;
      write_eeprom(PAYMENT_MODE_VAL_LOC, (uint8_t)Payment_Mode);
      //store_event_data(TRANSACT_EVENT, 167, g_RTC_time.epoch);
      to_eeprom(TOTAL_TAMPERS_COUNT_LOC, from_eeprom(TOTAL_TAMPERS_COUNT_LOC, TOTAL_TAMPERS_COUNT_SIZE) + 1, TOTAL_TAMPERS_COUNT_SIZE);
      to_eeprom(TOTAL_PROGRAMMING_COUNT_LOC, from_eeprom(TOTAL_PROGRAMMING_COUNT_LOC, TOTAL_PROGRAMMING_COUNT_SIZE) + 1, TOTAL_PROGRAMMING_COUNT_SIZE);
      if(Payment_Mode == e_Postpaid)
      {
        store_event_data(OTHER_EVENT, 211, g_RTC_time.epoch);
      }
      else
      {
        store_event_data(OTHER_EVENT, 212, g_RTC_time.epoch);
      }
    }
  }
}
static const struct attribute_desc_s Obj_Payment_Mode[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USRW, TAG_UINT8,           (void *) &Payment_Mode, Payment_Mode_Func},
};
//

// Last_Token_Recharge_Amount
int32_t Last_Token_Recharge_Amount = 0;
void Last_Token_Recharge_Amount_Func(void *data,int direction)
{
  uint32_t tmp_data;
  if(direction==ATTR_WRITE)
  {
    tmp_data = (((unsigned char*)data)[0]);
    tmp_data <<=8;
    tmp_data |= (((unsigned char*)data)[1]);
    tmp_data <<=8;
    tmp_data |= (((unsigned char*)data)[2]);
    tmp_data <<=8;
    tmp_data |= (((unsigned char*)data)[3]);
    
    if(tmp_data!=Last_Token_Recharge_Amount)
    {
      Last_Token_Recharge_Amount = tmp_data;
      to_eeprom(STORAGE_DLMS_LAST_TOKEN_AMOUNT_ADDR, Last_Token_Recharge_Amount, STORAGE_DLMS_LAST_TOKEN_AMOUNT_SIZE);
      to_eeprom(TOTAL_PROGRAMMING_COUNT_LOC, from_eeprom(TOTAL_PROGRAMMING_COUNT_LOC, TOTAL_PROGRAMMING_COUNT_SIZE) + 1, TOTAL_PROGRAMMING_COUNT_SIZE);
    }
  }
  else
  {
    Last_Token_Recharge_Amount = from_eeprom(STORAGE_DLMS_LAST_TOKEN_AMOUNT_ADDR, STORAGE_DLMS_LAST_TOKEN_AMOUNT_SIZE);
  }
}
static const struct attribute_desc_s Obj_Last_Token_Recharge_Amount[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USRW, TAG_INT32,           (void *) &Last_Token_Recharge_Amount, Last_Token_Recharge_Amount_Func},
};
//

// Obj_Last_Token_Recharge_Time
uint8_t Last_Token_Recharge_Time[14] = {0};
void Last_Token_Recharge_Time_Func(void *data,int direction)
{
  uint8_t* data_ptr = (uint8_t*)data;
  if(direction==ATTR_WRITE)
  {
    if(data_ptr[0] == 12)
    {
      memcpy(&Last_Token_Recharge_Time[2], &data_ptr[1], STORAGE_DLMS_LAST_AMOUNT_RECHARGE_TIME_SIZE);
      write_page_eeprom(STORAGE_DLMS_LAST_AMOUNT_RECHARGE_TIME_ADDR,&Last_Token_Recharge_Time[2],STORAGE_DLMS_LAST_AMOUNT_RECHARGE_TIME_SIZE);
      to_eeprom(TOTAL_PROGRAMMING_COUNT_LOC, from_eeprom(TOTAL_PROGRAMMING_COUNT_LOC, TOTAL_PROGRAMMING_COUNT_SIZE) + 1, TOTAL_PROGRAMMING_COUNT_SIZE);
    }
  }
  else
  {
    read_page_eeprom(STORAGE_DLMS_LAST_AMOUNT_RECHARGE_TIME_ADDR,&Last_Token_Recharge_Time[2],STORAGE_DLMS_LAST_AMOUNT_RECHARGE_TIME_SIZE);
    Last_Token_Recharge_Time[0] = 13;
    Last_Token_Recharge_Time[1] = 12;
  }
}

static const struct attribute_desc_s Obj_Last_Token_Recharge_Time[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USRW, TAG_OCTET_STRING,    (void *) Last_Token_Recharge_Time, Last_Token_Recharge_Time_Func},
};
//

// Obj_Total_Amount_Last_Recharge
int32_t Total_Amount_Last_Recharge = 0;
void Total_Amount_Last_Recharge_Func(void *data,int direction)
{
  uint32_t tmp_data;
  if(direction==ATTR_WRITE)
  {
    tmp_data = (((unsigned char*)data)[0]);
    tmp_data <<=8;
    tmp_data |= (((unsigned char*)data)[1]);
    tmp_data <<=8;
    tmp_data |= (((unsigned char*)data)[2]);
    tmp_data <<=8;
    tmp_data |= (((unsigned char*)data)[3]);
    
    if(tmp_data!=Total_Amount_Last_Recharge)
    {
      Total_Amount_Last_Recharge = tmp_data;
      to_eeprom(STORAGE_DLMS_TOTAL_LAST_AMOUNT_RECHARGE_ADDR, Total_Amount_Last_Recharge, STORAGE_DLMS_TOTAL_LAST_AMOUNT_RECHARGE_SIZE);
      to_eeprom(TOTAL_PROGRAMMING_COUNT_LOC, from_eeprom(TOTAL_PROGRAMMING_COUNT_LOC, TOTAL_PROGRAMMING_COUNT_SIZE) + 1, TOTAL_PROGRAMMING_COUNT_SIZE);
    }
  }
  else
  {
    Total_Amount_Last_Recharge = from_eeprom(STORAGE_DLMS_TOTAL_LAST_AMOUNT_RECHARGE_ADDR, STORAGE_DLMS_TOTAL_LAST_AMOUNT_RECHARGE_SIZE);
  }
}
static const struct attribute_desc_s Obj_Total_Amount_Last_Recharge[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USRW, TAG_INT32,           (void *) &Total_Amount_Last_Recharge, Total_Amount_Last_Recharge_Func},
};
//

// Obj_Current_Balance_Amount
int32_t Current_Balance_Amount = 0;
void Current_Balance_Amount_Func(void *data,int direction)
{
  uint32_t tmp_data;
  if(direction==ATTR_WRITE)
  {
    tmp_data = (((unsigned char*)data)[0]);
    tmp_data <<=8;
    tmp_data |= (((unsigned char*)data)[1]);
    tmp_data <<=8;
    tmp_data |= (((unsigned char*)data)[2]);
    tmp_data <<=8;
    tmp_data |= (((unsigned char*)data)[3]);
    
    if(tmp_data!=Current_Balance_Amount)
    {
      Current_Balance_Amount = tmp_data;
      to_eeprom(STORAGE_DLMS_CURRENT_BALANCE_AMOUNT_ADDR, Current_Balance_Amount, STORAGE_DLMS_CURRENT_BALANCE_AMOUNT_SIZE);
      to_eeprom(TOTAL_PROGRAMMING_COUNT_LOC, from_eeprom(TOTAL_PROGRAMMING_COUNT_LOC, TOTAL_PROGRAMMING_COUNT_SIZE) + 1, TOTAL_PROGRAMMING_COUNT_SIZE);
    }
  }
  else
  {
    Current_Balance_Amount = from_eeprom(STORAGE_DLMS_CURRENT_BALANCE_AMOUNT_ADDR, STORAGE_DLMS_CURRENT_BALANCE_AMOUNT_SIZE);
  }
}
static const struct attribute_desc_s Obj_Current_Balance_Amount[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USRW, TAG_INT32,           (void *) &Current_Balance_Amount, Current_Balance_Amount_Func},
};
//
// Obj_Current_Balance_Time
uint8_t Current_Balance_Time[14] = {0};

void Current_Balance_Time_Func(void *data,int direction)
{
  uint8_t* data_ptr = (uint8_t*)data;
  if(direction==ATTR_WRITE)
  {
    if(data_ptr[0] == 12)
    {
      memcpy(&Current_Balance_Time[2], &data_ptr[1], STORAGE_DLMS_CURRENT_BALANCE_TIME_SIZE);
      write_page_eeprom(STORAGE_DLMS_CURRENT_BALANCE_TIME_ADDR,&Current_Balance_Time[2],STORAGE_DLMS_CURRENT_BALANCE_TIME_SIZE);
      to_eeprom(TOTAL_PROGRAMMING_COUNT_LOC, from_eeprom(TOTAL_PROGRAMMING_COUNT_LOC, TOTAL_PROGRAMMING_COUNT_SIZE) + 1, TOTAL_PROGRAMMING_COUNT_SIZE);
    }
  }
  else
  {
    read_page_eeprom(STORAGE_DLMS_CURRENT_BALANCE_TIME_ADDR,&Current_Balance_Time[2],STORAGE_DLMS_CURRENT_BALANCE_TIME_SIZE);
    Current_Balance_Time[0] = 13;
    Current_Balance_Time[1] = 12;
  }
}

//void Current_Balance_Time_Func(void *data,int direction)
//{
//  uint8_t* data_ptr = (uint8_t*)data;
//  if(direction==ATTR_WRITE)
//  {
//    //if(data_ptr[0] == 12)
//    {
//      memcpy(&Current_Balance_Time[0], &data_ptr[0], 12);
//      write_page_eeprom(STORAGE_DLMS_CURRENT_BALANCE_TIME_ADDR,&Current_Balance_Time[0],STORAGE_DLMS_CURRENT_BALANCE_TIME_SIZE);
//      to_eeprom(TOTAL_PROGRAMMING_COUNT_LOC, from_eeprom(TOTAL_PROGRAMMING_COUNT_LOC, TOTAL_PROGRAMMING_COUNT_SIZE) + 1, TOTAL_PROGRAMMING_COUNT_SIZE);
//    }
//  }
//  else
//  {
//    read_page_eeprom(STORAGE_DLMS_CURRENT_BALANCE_TIME_ADDR,&Current_Balance_Time[0],STORAGE_DLMS_CURRENT_BALANCE_TIME_SIZE);
//    //Current_Balance_Time[0] = 13;
//    //Current_Balance_Time[1] = 12;
//  }
//}
static const struct attribute_desc_s Obj_Current_Balance_Time[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USRW, TAG_OCTET_STRING,    (void *) Current_Balance_Time, Current_Balance_Time_Func},
};
//
#endif
//////////////////////////////////////Security setup///////////////////////////////////
uint8_t server_system_title[10] = { 9,8,'C','R','Y',0,0,0,0,0 };
uint8_t client_system_title[10] = { 9,8,0,0,0,0,0,0,0,0 };
/* Security object for meter reader association */
uint8_t security_policy_encrypted = ALL_MSGS_ENCRYPTED;
uint8_t security_policy_authenticated_encrypted = ALL_MSGS_AUTHENTICATED_ENCRYPTED;
uint8_t security_suite = 0;
uint8_t update_security_keys = 0;
//void security_activate(uint8_t *data, uint16_t data_len, uint8_t *response, uint16_t *response_len);
//void global_key_transfer_MR(uint8_t *data, uint16_t data_len, uint8_t *response, uint16_t *response_len);
void global_key_transfer_US(uint8_t *data, uint16_t data_len, uint8_t *response, uint16_t *response_len);
void client_system_title_func(void *data,int direction,uint8_t clientAddress)
{
  uint8_t i;
  client_system_title[0] = 1;
  memset(&client_system_title[1],0,9);
  for(i=0;i<2;i++)
   {
     if(lnHdlc_uart_port[i].base.clientAddress == clientAddress && lnHdlc_uart_port[i].base.connected != 0)
     {
       if(lnHdlc_uart_port[i].base.cipher.clientSystemTitle.data != NULL)
       {
         client_system_title[0] = 9;
         client_system_title[1] = 8;
         memcpy(&client_system_title[2],lnHdlc_uart_port[i].base.cipher.clientSystemTitle.data,8);
       }
     }
   }
}
void client_system_title_func_PC(void *data,int direction)
{
  if(direction == ATTR_READ)
    client_system_title_func(data,direction,0x10);
}
void client_system_title_func_MR(void *data,int direction)
{
  if(direction == ATTR_READ)
    client_system_title_func(data,direction,0x20);
}

void client_system_title_func_US(void *data,int direction)
{
  if(direction == ATTR_READ)
    client_system_title_func(data,direction,0x30);
}

void client_system_title_func_Push(void *data,int direction)
{
  if(direction == ATTR_READ)
    client_system_title_func(data,direction,0x40);
}

void client_system_title_func_Firmware(void *data,int direction)
{
  if(direction == ATTR_READ)
    client_system_title_func(data,direction,0x50);
}

void client_system_title_func_IHD(void *data,int direction)
{
  if(direction == ATTR_READ)
    client_system_title_func(data,direction,0x60);
}

void security_activate(uint8_t *data, uint16_t data_len, uint8_t *response, uint16_t *response_len)
{

}

void global_key_transfer_MR(uint8_t *data, uint16_t data_len, uint8_t *response, uint16_t *response_len)
{
  uint8_t *chrptr = data, keyType, len;
  uint8_t Key_Data[24];
  uint8_t deCipheredKey[16];
  
  if(2 == *chrptr++)
  {
    if(TAG_ENUM == *chrptr++)
    {
      keyType = *chrptr++;
      if(TAG_OCTET_STRING == *chrptr++)
      {
        len = *chrptr++;
        memcpy(Key_Data,chrptr,len);
        //aes_key_unwrap(Key_Data, US_Security_Keys[KEY_ENCRYPTION_KEY], Key_Len);
        if (aes_key_unwrap(US_Security_Keys[KEY_ENCRYPTION_KEY], Key_Data, deCipheredKey, 16) == 0)
        {
          switch(keyType)
          {
          case UnicastEncryption :
            memcpy(US_Security_Keys[UNICAST_KEY], deCipheredKey, Key_Len);
            break;
          case BroadcastEncryption :
            memcpy(US_Security_Keys[BROADCAST_KEY], deCipheredKey, Key_Len);
            break;
          case Authentication :
            memcpy(US_Security_Keys[AUTHENTICATION_KEY], deCipheredKey, Key_Len);
            break;
          case Kek :
            memcpy(US_Security_Keys[KEY_ENCRYPTION_KEY], deCipheredKey, Key_Len);
            break;
          }
          store_event_data(TRANSACT_EVENT, 164, g_RTC_time.epoch);
          update_security_keys = 1;
        }
      }
    }
  }
}

static const struct attribute_desc_s Obj_Security_Setup_MR[] =
{
  { 1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *)object_list[0].instance_id, NULL },
  { 2, ACCESS_PC___MRR__USR_, TAG_ENUM,            (void *)&security_policy_encrypted, NULL },
  { 3, ACCESS_PC___MRR__USR_, TAG_ENUM,            (void *)&security_suite,NULL },
  { 4, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *)client_system_title, client_system_title_func_MR },
  { 5, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *)server_system_title, NULL },
};

static const struct method_desc_s Obj_Security_Setup_Methods_MR[] =
{
  { 1, ACCESS_PC___MR___USR_, security_activate },
  { 2, ACCESS_PC___MR___USR_, global_key_transfer_US },
};

/*Utility settings*/

void global_key_transfer_US(uint8_t *data, uint16_t data_len, uint8_t *response, uint16_t *response_len)
{
  uint8_t *chrptr = data, keyType, len;
  uint8_t Key_Data[24];
  uint8_t deCipheredKey[16];
  
  chrptr++;
  if(2 == *chrptr++)
  {
    chrptr++;
    if(TAG_ENUM == *chrptr++)
    {
      keyType = *chrptr++;
      if(TAG_OCTET_STRING == *chrptr++)
      {
        len = *chrptr++;
        memcpy(Key_Data,chrptr,len);
        //aes_key_unwrap(Key_Data, US_Security_Keys[KEY_ENCRYPTION_KEY], Key_Len);
        if (aes_key_unwrap(US_Security_Keys[KEY_ENCRYPTION_KEY], Key_Data, deCipheredKey, 16) == 0)
        {
          switch(keyType)
          {
          case UnicastEncryption :
            memcpy(US_Security_Keys[UNICAST_KEY], deCipheredKey, Key_Len);
            break;
          case BroadcastEncryption :
            memcpy(US_Security_Keys[BROADCAST_KEY], deCipheredKey, Key_Len);
            break;
          case Authentication :
            memcpy(US_Security_Keys[AUTHENTICATION_KEY], deCipheredKey, Key_Len);
            break;
          case Kek :
            memcpy(US_Security_Keys[KEY_ENCRYPTION_KEY], deCipheredKey, Key_Len);
            break;
          }
          store_event_data(TRANSACT_EVENT, 164, g_RTC_time.epoch);
          update_security_keys = 1;
        }
      }
    }
  }
}
static const struct attribute_desc_s Obj_Security_Setup_US[] =
{
  { 1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *)object_list[0].instance_id, NULL },
  { 2, ACCESS_PC___MRR__USR_, TAG_ENUM,            (void *)&security_policy_authenticated_encrypted, NULL },
  { 3, ACCESS_PC___MRR__USR_, TAG_ENUM,            (void *)&security_suite,NULL },
  { 4, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *)client_system_title, client_system_title_func_US },
  { 5, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *)server_system_title, NULL },
};

static const struct method_desc_s Obj_Security_Setup_Methods_US[] =
{
  { 1, ACCESS_PC___MR___USR_, security_activate },
  { 2, ACCESS_PC___MR___USR_, global_key_transfer_US },
};
/*Push*/

void global_key_transfer_PUSH(uint8_t *data, uint16_t data_len, uint8_t *response, uint16_t *response_len)
{
  uint8_t *chrptr = data, keyType, len;
  uint8_t Key_Data[24];
  uint8_t deCipheredKey[16];
  
  if(2 == *chrptr++)
  {
    if(TAG_ENUM == *chrptr++)
    {
      keyType = *chrptr++;
      if(TAG_OCTET_STRING == *chrptr++)
      {
        len = *chrptr++;
        memcpy(Key_Data,chrptr,len);
        //aes_key_unwrap(Key_Data, US_Security_Keys[KEY_ENCRYPTION_KEY], Key_Len);
        if (aes_key_unwrap(US_Security_Keys[KEY_ENCRYPTION_KEY], Key_Data, deCipheredKey, 16) == 0)
        {
          switch(keyType)
          {
          case UnicastEncryption :
            memcpy(US_Security_Keys[UNICAST_KEY], deCipheredKey, Key_Len);
            break;
          case BroadcastEncryption :
            memcpy(US_Security_Keys[BROADCAST_KEY], deCipheredKey, Key_Len);
            break;
          case Authentication :
            memcpy(US_Security_Keys[AUTHENTICATION_KEY], deCipheredKey, Key_Len);
            break;
          case Kek :
            memcpy(US_Security_Keys[KEY_ENCRYPTION_KEY], deCipheredKey, Key_Len);
            break;
          }
          store_event_data(TRANSACT_EVENT, 164, g_RTC_time.epoch);
          update_security_keys = 1;
        }
      }
    }
  }
}
static const struct attribute_desc_s Obj_Security_Setup_PUSH[] =
{
  { 1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *)object_list[0].instance_id, NULL },
  { 2, ACCESS_PC___MRR__USR_, TAG_ENUM,            (void *)&security_policy_encrypted, NULL },
  { 3, ACCESS_PC___MRR__USR_, TAG_ENUM,            (void *)&security_suite,NULL },
  { 4, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *)client_system_title, client_system_title_func_Push },
  { 5, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *)server_system_title, NULL },
};

static const struct method_desc_s Obj_Security_Setup_Methods_PUSH[] =
{
  { 1, ACCESS_PC___MR___USR_, security_activate },
  { 2, ACCESS_PC___MR___USR_, global_key_transfer_US },
};
/*Firmware*/

void global_key_transfer_FIRMWARE(uint8_t *data, uint16_t data_len, uint8_t *response, uint16_t *response_len)
{
  uint8_t *chrptr = data, keyType, len;
  uint8_t Key_Data[24];
  uint8_t deCipheredKey[16];
  
  if(2 == *chrptr++)
  {
    if(TAG_ENUM == *chrptr++)
    {
      keyType = *chrptr++;
      if(TAG_OCTET_STRING == *chrptr++)
      {
        len = *chrptr++;
        memcpy(Key_Data,chrptr,len);
        //aes_key_unwrap(Key_Data, US_Security_Keys[KEY_ENCRYPTION_KEY], Key_Len);
        if (aes_key_unwrap(US_Security_Keys[KEY_ENCRYPTION_KEY], Key_Data, deCipheredKey, 16) == 0)
        {
          switch(keyType)
          {
          case UnicastEncryption :
            memcpy(US_Security_Keys[UNICAST_KEY], deCipheredKey, Key_Len);
            break;
          case BroadcastEncryption :
            memcpy(US_Security_Keys[BROADCAST_KEY], deCipheredKey, Key_Len);
            break;
          case Authentication :
            memcpy(US_Security_Keys[AUTHENTICATION_KEY], deCipheredKey, Key_Len);
            break;
          case Kek :
            memcpy(US_Security_Keys[KEY_ENCRYPTION_KEY], deCipheredKey, Key_Len);
            break;
          }
          store_event_data(TRANSACT_EVENT, 164, g_RTC_time.epoch);
          update_security_keys = 1;
        }
      }
    }
  }
}
static const struct attribute_desc_s Obj_Security_Setup_FIRMWARE[] =
{
  { 1, ACCESS_PC___MRR__USR__FWR_, TAG_OCTET_STRING,    (void *)object_list[0].instance_id, NULL },
  { 2, ACCESS_PC___MRR__USR__FWR_, TAG_ENUM,            (void *)&security_policy_authenticated_encrypted, NULL },
  { 3, ACCESS_PC___MRR__USR__FWR_, TAG_ENUM,            (void *)&security_suite,NULL },
  { 4, ACCESS_PC___MRR__USR__FWR_, TAG_OCTET_STRING,    (void *)client_system_title, client_system_title_func_Firmware },
  { 5, ACCESS_PC___MRR__USR__FWR_, TAG_OCTET_STRING,    (void *)server_system_title, NULL },
};

static const struct method_desc_s Obj_Security_Setup_Methods_FIRMWARE[] =
{
  { 1, ACCESS_PC___MR___USR_, security_activate },
  { 2, ACCESS_PC___MR___USR_, global_key_transfer_US },
};
/*IHD*/

void global_key_transfer_IHD(uint8_t *data, uint16_t data_len, uint8_t *response, uint16_t *response_len)
{
  uint8_t *chrptr = data, keyType, len;
  uint8_t Key_Data[24];
  uint8_t deCipheredKey[16];
  
  if(2 == *chrptr++)
  {
    if(TAG_ENUM == *chrptr++)
    {
      keyType = *chrptr++;
      if(TAG_OCTET_STRING == *chrptr++)
      {
        len = *chrptr++;
        memcpy(Key_Data,chrptr,len);
        //aes_key_unwrap(Key_Data, US_Security_Keys[KEY_ENCRYPTION_KEY], Key_Len);
        if (aes_key_unwrap(US_Security_Keys[KEY_ENCRYPTION_KEY], Key_Data, deCipheredKey, 16) == 0)
        {
          switch(keyType)
          {
          case UnicastEncryption :
            memcpy(US_Security_Keys[UNICAST_KEY], deCipheredKey, Key_Len);
            break;
          case BroadcastEncryption :
            memcpy(US_Security_Keys[BROADCAST_KEY], deCipheredKey, Key_Len);
            break;
          case Authentication :
            memcpy(US_Security_Keys[AUTHENTICATION_KEY], deCipheredKey, Key_Len);
            break;
          case Kek :
            memcpy(US_Security_Keys[KEY_ENCRYPTION_KEY], deCipheredKey, Key_Len);
            break;
          }
          store_event_data(TRANSACT_EVENT, 164, g_RTC_time.epoch);
          update_security_keys = 1;
        }
      }
    }
  }
}
static const struct attribute_desc_s Obj_Security_Setup_IHD[] =
{
  { 1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *)object_list[0].instance_id, NULL },
  { 2, ACCESS_PC___MRR__USR_, TAG_ENUM,            (void *)&security_policy_encrypted, NULL },
  { 3, ACCESS_PC___MRR__USR_, TAG_ENUM,            (void *)&security_suite,NULL },
  { 4, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *)client_system_title, client_system_title_func_IHD },
  { 5, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *)server_system_title, NULL },
};

static const struct method_desc_s Obj_Security_Setup_Methods_IHD[] =
{
  { 1, ACCESS_PC___MR___US__, security_activate },
  { 2, ACCESS_PC___MR___US___IHR_, global_key_transfer_US },
};
//////////////////////////////////////////end//////////////////////////////////////////
//TCP/UDP setup
uint16_t Tcp_Udp_Port = 4059;
void Tcp_Udp_Port_Func(void *data, int direction)
{
  uint8_t *Temp_data = (uint8_t*)data;
  if(ATTR_WRITE == direction)
  {
    Tcp_Udp_Port = *Temp_data;
    Temp_data++;
    Tcp_Udp_Port = (Tcp_Udp_Port << 8) | *Temp_data;
  }
}
const uint8_t IP_reference[] =
{
  7,
    6,
      0,0,25,1,0,255
};
static const uint16_t TCP_UDP_mss = 576;
static const uint8_t TCP_UDP_nb_of_sim_conn = 1;
static const uint16_t TCP_UDP_nactivity_time_out = 20;
static const struct attribute_desc_s Obj_TCP_UDP_SETUP[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_UINT16,          (void *) &Tcp_Udp_Port, Tcp_Udp_Port_Func},
  {3, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *) IP_reference, NULL},
  {4, ACCESS_PC___MRR__USR_, TAG_UINT16,          (void *) &TCP_UDP_mss, NULL},
  {5, ACCESS_PC___MRR__USR_, TAG_UINT8,           (void *) &TCP_UDP_nb_of_sim_conn, NULL},
  {6, ACCESS_PC___MRR__USR_, TAG_UINT16,          (void *) &TCP_UDP_nactivity_time_out, NULL}
};

////////////
//IPv4 setup
const uint8_t IPv4_DL_reference[] =
{
  7,
    6,
      0,0,25,3,0,255
};

const uint32_t IPv4_addresses = 0xC0A80001;
const uint8_t multicast_IPv4_address[] =
{
  1,
  0,
};
const uint8_t IPv4_options[] =
{
  1,
  0,
};
const uint32_t IPv4_subnetmask = 0;
const uint32_t IPv4_gateway_ip = 0;
const uint8_t IPv4_use_DHCP_flag = 0;
const uint32_t IPv4_primary_DNS_address = 0;
const uint32_t IPv4_secondary_DNS_address = 0;

static const struct attribute_desc_s Obj_IPv4_Setup[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *) IPv4_DL_reference, NULL},
  {3, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &IPv4_addresses, NULL},
  {4, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *) multicast_IPv4_address, NULL},
  {5, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *) IPv4_options, NULL},
  {6, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &IPv4_subnetmask, NULL},
  {7, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &IPv4_gateway_ip, NULL},
  {8, ACCESS_PC___MRR__USR_, TAG_BOOLEAN,         (void *) &IPv4_use_DHCP_flag, NULL},
  {9, ACCESS_PC___MRR__USR_, TAG_UINT32,          (void *) &IPv4_primary_DNS_address, NULL},
  {10, ACCESS_PC___MRR__USR_, TAG_UINT32,         (void *) &IPv4_secondary_DNS_address, NULL},
};

void add_mc_IPv4_address(uint8_t *data,uint16_t data_len,uint8_t *response,uint16_t *response_len)
{
}

void delete_mc_IPv4_address(uint8_t *data,uint16_t data_len,uint8_t *response,uint16_t *response_len)
{
}

void get_nbof_mc_IPv4_addresses(uint8_t *data,uint16_t data_len,uint8_t *response,uint16_t *response_len)
{
}

static const struct method_desc_s IPv4_Setup_methods[] =
{
  {1, ACCESS_PC___MR___US__, add_mc_IPv4_address},
  {2, ACCESS_PC___MR___US__, delete_mc_IPv4_address},
  {3, ACCESS_PC___MR___US__, get_nbof_mc_IPv4_addresses}
};
//
////////////

//IPv6 setup
const uint8_t DL_reference[] =
{
  7,
    6,
      0,0,25,0,0,255
};

const uint8_t IPv6_address_config_mode = Auto_configuration;
const uint8_t IPv6_unicast_IPv6_addresses[] =
{
  19,
  1,
    9,
      16,
        0x20,0x01,0,0,0,0,0,0,0x12,0x34,0x56,0x78,0xab,0xcd,0xef,0x43
};
const uint8_t IPv6_multicast_IPv6_addresses[] =
{
  19,
  1,
    9,
      16,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};
const uint8_t IPv6_gateway_IPv6_addresses[] =
{
  19,
  1,
    9,
      16,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};
const uint8_t IPv6_primary_DNS_address[] =
{
  1,
    0
};
const uint8_t IPv6_secondary_DNS_address[] =
{
  1,
    0
};
static const uint8_t IPv6_traffic_class = 2;
static const uint8_t IPv6_neighbor_discovery_setup[] =
{
  13,
    1,
      2,3,
         TAG_UINT8,
         3,
         TAG_UINT16,
         INJECT16(10000),
         TAG_UINT32,
         INJECT32(520000)
};
static const struct attribute_desc_s Obj_IPv6_Setup[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *) DL_reference, NULL},
  {3, ACCESS_PC___MRR__USR_, TAG_ENUM,            (void *) &IPv6_address_config_mode, NULL},
  {4, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *) IPv6_unicast_IPv6_addresses, NULL},
  {5, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *) IPv6_multicast_IPv6_addresses, NULL},
  {6, ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *) IPv6_gateway_IPv6_addresses, NULL},
  {7, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *) IPv6_primary_DNS_address, NULL},
  {8, ACCESS_PC___MRR__USR_, TAG_OCTET_STRING,    (void *) IPv6_secondary_DNS_address, NULL},
  {9, ACCESS_PC___MRR__USR_, TAG_UINT8,           (void *) &IPv6_traffic_class, NULL},
  {10,ACCESS_PC___MRR__USR_, TAG_ARRAY,           (void *) IPv6_neighbor_discovery_setup, NULL}
};

void add_IPv6_address(uint8_t *data,uint16_t data_len,uint8_t *response,uint16_t *response_len)
{
}

void remove_IPv6_address(uint8_t *data,uint16_t data_len,uint8_t *response,uint16_t *response_len)
{
}

static const struct method_desc_s IPv6_Setup_methods[] =
{
  {1, ACCESS_PC___MR___US__, add_IPv6_address},
  {2, ACCESS_PC___MR___US__, remove_IPv6_address}
};
//
const uint8_t fix_flag = 0;
//// Stack and Heap trace objects
static const struct attribute_desc_s Obj_Stack_Overflow[] =
{
  { 1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *)object_list[0].instance_id, NULL },
  { 2, ACCESS_PCR__MRR__USR_, TAG_UINT16,          (void *)&fix_flag/*&st_stack_trace.overflow_sp*/, NULL },
};


//RTC Sync
void RTC_Sync_Function(void* data, int direction)
{
    if (direction == ATTR_WRITE)
    {
        tmp_int16 = (((unsigned char*)data)[0]);
        tmp_int16 <<= 8;
        tmp_int16 |= (((unsigned char*)data)[1]);
        //Specify range
        tmp_int16 = tmp_int16 / 10;
        tmp_int16 = tmp_int16 * 10;
        if (abs(tmp_int16) <= 300)
        { //EEPROM(write);
            stRTCTimeSync.i16CorrectionTime = tmp_int16;
            to_eeprom(RTC_TIME_SYNC_VAL_LOC, stRTCTimeSync.i16CorrectionTime, RTC_TIME_SYNC_VAL_SIZE);
            stRTCTimeSync.u8UpdateFlag = 0;
            stRTCTimeSync.u32NextUpdateTime = 0;
            to_eeprom(RTC_TIME_SYNC_FLAG_LOC, stRTCTimeSync.u8UpdateFlag, RTC_TIME_SYNC_FLAG_SIZE);
        }
    }
    else
    {
        read_page_eeprom(RTC_TIME_SYNC_VAL_LOC, (uint8_t*)&tmp_int16, RTC_TIME_SYNC_VAL_SIZE);
    }
}

static const struct attribute_desc_s Obj_RTC_Sync[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void*)object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USRW, TAG_INT16,            (void*)&tmp_int16, RTC_Sync_Function}
};
//static const struct attribute_desc_s Obj_Stack_Size[] =
//{
//  { 1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *)object_list[0].instance_id, NULL },
//  { 2, ACCESS_PCR__MRR__USR_, TAG_UINT16,          (void *)&st_stack_trace.size_sp, NULL },
//};
//void Stack_Use_func(void *data, int direction)
//{
//  tmp_uint16 = st_stack_trace.begin_sp - st_stack_trace.peak_sp;
//}
//static const struct attribute_desc_s Obj_Stack_Use[] =
//{
//  { 1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *)object_list[0].instance_id, NULL },
//  { 2, ACCESS_PCR__MRR__USR_, TAG_UINT16,          (void *)&tmp_uint16, Stack_Use_func},
//};
static const struct attribute_desc_s Obj_Heap_Overflow[] =
{
  { 1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *)object_list[0].instance_id, NULL },
  { 2, ACCESS_PCR__MRR__USR_, TAG_UINT16,          (void *)&fix_flag/*&st_heap_trace.overflow_heap*/, NULL },
};
//static const struct attribute_desc_s Obj_Heap_Size[] =
//{
//  { 1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *)object_list[0].instance_id, NULL },
//  { 2, ACCESS_PCR__MRR__USR_, TAG_UINT16,          (void *)&st_heap_trace.max_heap_size, NULL },
//};
//static const struct attribute_desc_s Obj_Heap_Allocated[] =
//{
//  { 1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *)object_list[0].instance_id, NULL },
//  { 2, ACCESS_PCR__MRR__USR_, TAG_UINT16,          (void *)&st_heap_trace.total_heap_allocated, NULL },
//};
//static const struct attribute_desc_s Obj_Heap_Max_Allocated[] =
//{
//  { 1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *)object_list[0].instance_id, NULL },
//  { 2, ACCESS_PCR__MRR__USR_, TAG_UINT16,          (void *)&st_heap_trace.max_heap_allocated, NULL },
//};
//static const struct attribute_desc_s Obj_Heap_Elements_Allocated[] =
//{
//  { 1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *)object_list[0].instance_id, NULL },
//  { 2, ACCESS_PCR__MRR__USR_, TAG_UINT16,          (void *)&st_heap_trace.total_active_heap_elements, NULL },
//};
//static const struct attribute_desc_s Obj_Heap_Max_Elements_Allocated[] =
//{
//  { 1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *)object_list[0].instance_id, NULL },
//  { 2, ACCESS_PCR__MRR__USR_, TAG_UINT16,          (void *)&st_heap_trace.max_active_heap_elements, NULL },
//};
//

// FiFO_LiFO mode
uint8_t FiFO_LiFO = 0;
void FiFO_LiFO_Func(void *data,int direction)
{
  uint8_t * ptr = (uint8_t*)data;
  if(direction == ATTR_READ)
  {
    FiFO_LiFO=read_eeprom(MEMORY_READ_FIFO_LIFO_LOC);
  }
  else if(direction == ATTR_WRITE)
  {
    if((FiFO_LiFO != *ptr) && (*ptr < 2))
    {
      FiFO_LiFO = *ptr;
      write_eeprom(MEMORY_READ_FIFO_LIFO_LOC,FiFO_LiFO);
    }
  }

}
static const struct attribute_desc_s Obj_FiFO_LiFO[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USRW, TAG_UINT8,           (void *) &FiFO_LiFO, FiFO_LiFO_Func},
};


uint8_t FiFO_LiFO2 = 0;
void StructSetTest(void *data,int direction)
{
  const uint8_t test[] =
  {
    6,
      2,
        TAG_INT8, 0x10,                         //Client SAP
        TAG_UINT16, INJECT16(1)                 //Server SAP
  };
  if(direction == ATTR_READ)
  {
    memcpy(Data_Buffer, test, sizeof(test));
  }
  else if(direction == ATTR_WRITE)
  {
    ;
  }

}
static const struct attribute_desc_s Obj_StructSetTest[] =
{
  {1, ACCESS_PCR__MRR__USR_, TAG_OCTET_STRING,    (void *) object_list[0].instance_id, NULL},
  {2, ACCESS_PC___MRR__USRW, TAG_STRUCTURE,       (void *) Data_Buffer, StructSetTest},
};
/////////////////////////////////// Objects defination ends //////////////////////////

/////////////////////////////////// Objects list begin ////////////////////////////////

#ifdef ASSOCIATION_VER_3
#define Association_attrs_no  11
#define Association_methods_no  6
#define ASSOCIATION_VER 3
#else
#define Association_attrs_no 9
#define Association_methods_no 4
#define ASSOCIATION_VER 1
#endif
const struct object_desc_s object_list[] =
{
  {ASSOC_PC_MR_US_FW_PH,    CLASS_ID_DATA,                0, {  0,   0,  42,   0,   0, 255}, 2,  obj_logical_name, 0, NULL},
  {ASSOC_PC_MR_US_FW_PH,    CLASS_ID_DATA,                0, {  0,   0,  96,   1,   0, 255}, 2,  Obj_Meter_Sr_No, 0, NULL},
  {ASSOC_PC_MR_US_FW_PH,    CLASS_ID_DATA,                0, {  0,   0,  96,   1,   2, 255}, 2,  obj_device_id, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0,  96,   1,   1, 255}, 2,  Obj_Manufacturer_Name, 0, NULL },
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  1,   0,   0,   2,   0, 255}, 2,  Obj_FW_Version_No, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  1,   0, 128,   2,   0, 255}, 2,  Obj_Internal_FW_Version_No, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0,  94,  91,   9, 255}, 2,  Obj_Meter_Type, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0,  94,  91,  11, 255}, 2,  Obj_Meter_category, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0,  94,  91,  12, 255}, 2,  Obj_Current_rating, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0,  96,   1,   4, 255}, 2,  Obj_Year_of_Manufacture, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  1,   0,   0,   8,   0, 255}, 2,  Obj_Demand_Integration_Period, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  1,   0,   0,   8,   4, 255}, 2,  Obj_Profile_Capture_Period, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  1,   0,   0,   8,   5, 255}, 2,  Obj_Daily_LP_Profile_Capture_Period, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0,  94,  91,   0, 255}, 2,  Obj_Cum_Tamper_Count, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0,   0,   1,   0, 255}, 2,  Obj_Cum_MD_Reset_Count, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0,  96,   2,   0, 255}, 2,  Obj_Cum_Prog_Count, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0,  96,  11,   0, 255}, 2,  Obj_event_ID_volt, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0,  96,  11,   1, 255}, 2,  Obj_event_ID_current, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0,  96,  11,   2, 255}, 2,  Obj_event_ID_power, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0,  96,  11,   3, 255}, 2,  Obj_event_ID_transcation, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0,  96,  11,   4, 255}, 2,  Obj_event_ID_other, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0,  96,  11,   5, 255}, 2,  Obj_event_ID_nonnroll, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0,  96,  11,   6, 255}, 2,  Obj_event_ID_condiscon, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0,   0,   1,   1, 255}, 2,  Obj_Billing_Profile_Entries_In_Use, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0,  94,  91,  18, 255}, 2,  Obj_ESW, 0, NULL },
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0,  94,  91,  26, 255}, 2,  Obj_ESWF, 0, NULL },
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0,  94,  96,  19, 255}, 2,  Obj_Metering_Mode, 0, NULL},
  //Special message object
  {ASSOC_MR_US_IHD,   CLASS_ID_DATA,                      0, {  0,   0,  96,  13,   1, 255}, 2,  Obj_SMO1, 0, NULL },

  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0,  94,  96,  20, 255}, 2,  Obj_Payment_Mode, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0,  94,  96,  21, 255}, 2,  Obj_Last_Token_Recharge_Amount, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0,  94,  96,  22, 255}, 2,  Obj_Last_Token_Recharge_Time, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0,  94,  96,  23, 255}, 2,  Obj_Total_Amount_Last_Recharge, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0,  94,  96,  24, 255}, 2,  Obj_Current_Balance_Amount, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0,  94,  96,  25, 255}, 2,  Obj_Current_Balance_Time, 0, NULL},
  
//#ifdef WHOLE_CURRENT_METER
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0, 128, 128,   0,   1, 255}, 2,  Obj_Ctrl_disConn_Time_Interval, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0, 128, 128,   0,   2, 255}, 2,  Obj_Ctrl_Conn_Time_Interval, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0, 128, 128,   0,   3, 255}, 2,  Obj_Ctrl_Conn_Lockout_Time, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0, 128, 128,   0,   4, 255}, 2,  Obj_Ctrl_Conn_Time_Repeat, 0, NULL},
//#endif
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0, 128, 128,   0,   5, 255}, 2,  Obj_Tamper_Occ_Time, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0, 128, 128,   0,   6, 255}, 2,  Obj_Tamper_Res_Time, 0, NULL},

  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0, 128,   1,   0, 255}, 2,  Obj_NIC_Firmware_Version, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0, 128,   1,   1, 255}, 2,  Obj_NIC_Stack_Version, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0, 128,   1,   2, 255}, 2,  Obj_NIC_Hardware_Version, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0, 128,   1,   3, 255}, 2,  Obj_NIC_Network_Address, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0, 128,   1,   4, 255}, 2,  Obj_NIC_Channel_Number, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0, 128,   1,   5, 255}, 2,  Obj_NIC_Encryption_Key, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0, 128,   1,   7, 255}, 2,  Obj_NIC_MAC_Address, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0,  96,   1,   6, 255}, 2,  Obj_NIC_SIM_Number, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0, 128,   1,   8, 255}, 2,  Obj_NIC_New_MAC_Address, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0, 128,   1,   9, 255}, 2,  Obj_NIC_APN_Name, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0, 128,   1,  10, 255}, 2,  Obj_NIC_APN_Username, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0, 128,   1,  11, 255}, 2,  Obj_NIC_APN_Password, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0, 128,   1,  12, 255}, 2,  Obj_NIC_Server_Address, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0, 128,   1,  13, 255}, 2,  Obj_NIC_Server_Port, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0, 128,   1,  14, 255}, 2,  Obj_NIC_MQTT_Username, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0, 128,   1,  15, 255}, 2,  Obj_NIC_MQTT_Password, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0, 128,   1,  16, 255}, 2,  Obj_NIC_Force_Stop, 0, NULL},

  //stack heap
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0, 128,   3,   0, 255}, 2,  Obj_Heap_Overflow, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0, 128,   2,   0, 255}, 2,  Obj_Stack_Overflow, 0, NULL},
  {ASSOC_MR_US,       CLASS_ID_DATA,                      0, {  0,   0, 128,   4,   0, 255}, 2,  Obj_RTC_Sync, 0, NULL},
  //invocation counter
  {ASSOC_PC_MR_US,    CLASS_ID_DATA,                      0, {  0,   0,  43,   1,   2, 255}, 2,  Obj_invocation_counter_MR, 0, NULL},
  {ASSOC_PC_MR_US,    CLASS_ID_DATA,                      0, {  0,   0,  43,   1,   3, 255}, 2,  Obj_invocation_counter_US, 0, NULL},
  {ASSOC_PC_MR_US_PH, CLASS_ID_DATA,                      0, {  0,   0,  43,   1,   4, 255}, 2,  Obj_invocation_counter_PH, 0, NULL},
  {ASSOC_PC_MR_US_FW, CLASS_ID_DATA,                      0, {  0,   0,  43,   1,   5, 255}, 2,  Obj_invocation_counter_FW, 0, NULL},
  
  //Registers
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  0,   0,  94,   91,  13,255}, 3,  Obj_bill_power_on_time, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  0,   0,  94,   91,  14,255}, 3,  Obj_cumulative_power_on_time, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  0,   0,  0,    1,   2, 255}, 3,  Obj_Last_MD_Rst_DT, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  1,   0,  1,    7,   0, 255}, 3,  Obj_KW_Total, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  1,   0,  12,   7,   0, 255}, 3,  Obj_VRMS, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  1,   0,  11,   7,   0, 255}, 3,  Obj_Phase_Current, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  1,   0,  91,   7,   0, 255}, 3,  Obj_Neutral_Current, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  1,   0,  13,   7,   0, 255}, 3,  Obj_PF, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  1,   0,  14,   7,   0, 255}, 3,  Obj_Frequency, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  1,   0,  9,    7,   0, 255}, 3,  Obj_KVA_Total, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  1,   0,  1,    8,   0, 255}, 3,  Obj_cum_KWh, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  1,   0,  9,    8,   0, 255}, 3,  Obj_cum_KVAh, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  1,   0,  2,    8,   0, 255}, 3,  Obj_cum_KWh_Expo, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  1,   0,  10,   8,   0, 255}, 3,  Obj_cum_KVAh_Expo, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  0,   0,  94,   91,  8, 255}, 3,  Obj_Power_Fail_Duration, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  
  //load profile registers
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  1,   0,  12,   27,   0, 255}, 3,  Obj_V_LP, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  1,   0,  11,   27,   0, 255}, 3,  Obj_I_Phase_LP, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  1,   0,  91,   27,   0, 255}, 3,  Obj_I_Neutral_LP, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  1,   0,   1,   29,   0, 255}, 3,  Obj_cum_KWh_LP, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  1,   0,   9,   29,   0, 255}, 3,  Obj_cum_KVAh_LP, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  1,   0,   2,   29,   0, 255}, 3,  Obj_cum_KWh_Expo_LP, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  1,   0,  10,   29,   0, 255}, 3,  Obj_cum_KVAh_Expo_LP, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},

  //billing profile registers
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  1,   0,  13,   0,   0, 255}, 3,  Obj_PF_Avg_bill, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  1,   0,  1,    8,   1, 255}, 3,  Obj_cum_KWh_TZ1, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  1,   0,  1,    8,   2, 255}, 3,  Obj_cum_KWh_TZ2, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  1,   0,  1,    8,   3, 255}, 3,  Obj_cum_KWh_TZ3, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  1,   0,  1,    8,   4, 255}, 3,  Obj_cum_KWh_TZ4, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  1,   0,  1,    8,   5, 255}, 3,  Obj_cum_KWh_TZ5, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  1,   0,  1,    8,   6, 255}, 3,  Obj_cum_KWh_TZ6, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  1,   0,  9,    8,   1, 255}, 3,  Obj_cum_KVAh_TZ1, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  1,   0,  9,    8,   2, 255}, 3,  Obj_cum_KVAh_TZ2, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  1,   0,  9,    8,   3, 255}, 3,  Obj_cum_KVAh_TZ3, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  1,   0,  9,    8,   4, 255}, 3,  Obj_cum_KVAh_TZ4, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  1,   0,  9,    8,   5, 255}, 3,  Obj_cum_KVAh_TZ5, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  1,   0,  9,    8,   6, 255}, 3,  Obj_cum_KVAh_TZ6, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  1,   0,  94,  91,  14, 255}, 3,  Obj_Active_Current, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_REGISTER,                  0, {  1,   0,  94,  91, 128, 255}, 3,  Obj_Over_Current_Threshold, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  
  //extended registers
  {ASSOC_MR_US,       CLASS_ID_EXTENDED_REGISTER,         0, {  1,   0,  1,    6,   0, 255}, 5,  Obj_KW_MAX, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  {ASSOC_MR_US,       CLASS_ID_EXTENDED_REGISTER,         0, {  1,   0,  9,    6,   0, 255}, 5,  Obj_KVA_MAX, 1, Obj_Dummy_Resiter_And_ExtendedRegister_Methods},
  //name plate
  {ASSOC_MR_US,       CLASS_ID_PROFILE_GENERIC,           1, {  0,   0,  94,  91,  10, 255}, 8,  Obj_Name_Plate_Profile, 2, Obj_Dummy_Profile_Methods},
  //isntant
  {ASSOC_MR_US,       CLASS_ID_PROFILE_GENERIC,           1, {  1,   0,  94,  91,   0, 255}, 8, Obj_Instant_Profile, 2, Obj_Dummy_Profile_Methods},
  {ASSOC_MR_US,       CLASS_ID_PROFILE_GENERIC,           1, {  1,   0,  94,  91,   3, 255}, 8, Obj_Instant_Scaler_Profile, 2, Obj_Dummy_Profile_Methods},
  //load profile
  {ASSOC_MR_US,       CLASS_ID_PROFILE_GENERIC,           1, {  1,   0,  99,   1,   0, 255}, 8, Obj_Load_Profile, 2, Obj_Dummy_Profile_Methods},
  {ASSOC_MR_US,       CLASS_ID_PROFILE_GENERIC,           1, {  1,   0,  94,  91,   4, 255}, 8, Obj_Load_Scaler_Profile, 2, Obj_Dummy_Profile_Methods},
  //daily load profile
  {ASSOC_MR_US,       CLASS_ID_PROFILE_GENERIC,           1, {  1,   0,  99,   2,   0, 255}, 8, Obj_Daily_Load_Profile, 2, Obj_Dummy_Profile_Methods},
  {ASSOC_MR_US,       CLASS_ID_PROFILE_GENERIC,           1, {  1,   0,  94,  91,   5, 255}, 8, Obj_Daily_Load_Scaler_Profile,2, Obj_Dummy_Profile_Methods},
  //billing profile
  {ASSOC_MR_US,       CLASS_ID_PROFILE_GENERIC,           1, {  1,   0,  98,   1,   0, 255}, 8, Obj_Billing_Profile, 2, Obj_Dummy_Profile_Methods},
  {ASSOC_MR_US,       CLASS_ID_PROFILE_GENERIC,           1, {  1,   0,  94,  91,   6, 255}, 8, Obj_Billing_Scaler_Profile, 2, Obj_Dummy_Profile_Methods},
  //tamper profiles
  {ASSOC_MR_US,       CLASS_ID_PROFILE_GENERIC,           1, {  1,   0,  94,   91,  7, 255}, 8,  Obj_Tamper_Scaler_Profile, 2, Obj_Dummy_Profile_Methods},
  {ASSOC_MR_US,       CLASS_ID_PROFILE_GENERIC,           1, {  0,   0,  99,   98,  0, 255}, 8,  Obj_Voltage_Tamper_Profile, 2, Obj_Dummy_Profile_Methods},
  {ASSOC_MR_US,       CLASS_ID_PROFILE_GENERIC,           1, {  0,   0,  99,   98,  1, 255}, 8,  Obj_Current_Tamper_Profile, 2, Obj_Dummy_Profile_Methods},
  {ASSOC_MR_US,       CLASS_ID_PROFILE_GENERIC,           1, {  0,   0,  99,   98,  2, 255}, 8,  Obj_Power_Fail_Tamper_Profile, 2, Obj_Dummy_Profile_Methods},
  {ASSOC_MR_US,       CLASS_ID_PROFILE_GENERIC,           1, {  0,   0,  99,   98,  3, 255}, 8,  Obj_Transact_Tamper_Profile, 2, Obj_Dummy_Profile_Methods},
  {ASSOC_MR_US,       CLASS_ID_PROFILE_GENERIC,           1, {  0,   0,  99,   98,  4, 255}, 8,  Obj_Other_Tamper_Profile, 2, Obj_Dummy_Profile_Methods},
  {ASSOC_MR_US,       CLASS_ID_PROFILE_GENERIC,           1, {  0,   0,  99,   98,  5, 255}, 8,  Obj_Non_Roll_Tamper_Profile, 2, Obj_Dummy_Profile_Methods},
  {ASSOC_MR_US,       CLASS_ID_PROFILE_GENERIC,           1, {  0,   0,  99,   98,  6, 255}, 8,  Obj_Control_Tamper_Profile, 2, Obj_Dummy_Profile_Methods},
  
  //rtc clock
  {ASSOC_PC_MR_US_FW_PH, CLASS_ID_CLOCK,                  0,               {  0,   0,   1,   0,   0, 255}, 9,  clock_attrs, 6, clock_methods},
  //scripts
  {ASSOC_MR_US,       CLASS_ID_SCRIPT_TABLE,              0, {  0,   0,  10,   0,   1, 255}, 2,  Obj_Billing_Script_Table, 1, Obj_Billing_script_table_Methods},
  {ASSOC_MR_US,       CLASS_ID_SCRIPT_TABLE,              0, {  0,   0,  10,   0, 100, 255}, 2, Obj_Tariffication_Script_Table, 0, NULL},
  {ASSOC_MR_US_FW,    CLASS_ID_SCRIPT_TABLE,              0, {  0,   0,  10,   0, 107, 255}, 2, Obj_Image_activation_Script_Table, 0, NULL},
  {ASSOC_MR_US_PH,    CLASS_ID_SCRIPT_TABLE,              0, {  0,   0,  10,   0, 108, 255}, 2, Obj_Push_Script_Table, 0, NULL},
  //{ASSOC_MR_US,       CLASS_ID_SCRIPT_TABLE,              0, {  0,   0,   10,   0,128, 255}, 2, Obj_Limiter_Action_Script_Table, 0, NULL},
  
  //Associations
  {ASSOC_PC,          CLASS_ID_ASSOCIATION_LN,            ASSOCIATION_VER, {  0,   0,  40,   0,   0, 255}, Association_attrs_no,  ln_40_attrs_PC_Mndtry, Association_methods_no, ln_40_methods},
  {ASSOC_MR,          CLASS_ID_ASSOCIATION_LN,            ASSOCIATION_VER, {  0,   0,  40,   0,   0, 255}, Association_attrs_no,  ln_40_attrs_MR_Mndtry, Association_methods_no, ln_40_methods},
  {ASSOC_US,          CLASS_ID_ASSOCIATION_LN,            ASSOCIATION_VER, {  0,   0,  40,   0,   0, 255}, Association_attrs_no,  ln_40_attrs_US_Mndtry, Association_methods_no, ln_40_methods},
  {ASSOC_PUSH,        CLASS_ID_ASSOCIATION_LN,            ASSOCIATION_VER, {  0,   0,  40,   0,   0, 255}, Association_attrs_no,  ln_40_attrs_Push_Mndtry, Association_methods_no, ln_40_methods},
  {ASSOC_FIRMWARE,    CLASS_ID_ASSOCIATION_LN,            ASSOCIATION_VER, {  0,   0,  40,   0,   0, 255}, Association_attrs_no,  ln_40_attrs_Firmware_Mndtry, Association_methods_no, ln_40_methods},
  //{ASSOC_IHD,         CLASS_ID_ASSOCIATION_LN,            ASSOCIATION_VER, {  0,   0,  40,   0,   0, 255}, Association_attrs_no,  ln_40_attrs_IHD_Mndtry, Association_methods_no, ln_40_methods},
  
  {ASSOC_PC_MR_US,    CLASS_ID_ASSOCIATION_LN,            ASSOCIATION_VER, {  0,   0,  40,   0,   1, 255}, Association_attrs_no,  ln_40_attrs_PC, Association_methods_no, ln_40_methods_PC},
  {ASSOC_MR_US,       CLASS_ID_ASSOCIATION_LN,            ASSOCIATION_VER, {  0,   0,  40,   0,   2, 255}, Association_attrs_no,  ln_40_attrs_MR, Association_methods_no, ln_40_methods_MR},
  {ASSOC_MR_US,       CLASS_ID_ASSOCIATION_LN,            ASSOCIATION_VER, {  0,   0,  40,   0,   3, 255}, Association_attrs_no,  ln_40_attrs_US, Association_methods_no, ln_40_methods_US},
  {ASSOC_MR_US_PH,    CLASS_ID_ASSOCIATION_LN,            ASSOCIATION_VER, {  0,   0,  40,   0,   4, 255}, Association_attrs_no,  ln_40_attrs_Push, Association_methods_no, ln_40_methods_Push},
  {ASSOC_MR_US_FW,    CLASS_ID_ASSOCIATION_LN,            ASSOCIATION_VER, {  0,   0,  40,   0,   5, 255}, Association_attrs_no,  ln_40_attrs_Firmware, Association_methods_no, ln_40_methods_Firmware},
  //{ASSOC_US_IHD,      CLASS_ID_ASSOCIATION_LN,            ASSOCIATION_VER, {  0,   0,  40,   0,   6, 255}, Association_attrs_no,  ln_40_attrs_IHD, Association_methods_no, ln_40_methods_IHD},
  
  //image transfer
  {ASSOC_MR_US_FW,    CLASS_ID_IMAGE_TRANSFER,                          0, {  0,   0,   44,   0,  0, 255}, 7, Obj_Image_Transfer, 4, Obj_Image_Transfer_Methods},
  
  //activity calendar
  {ASSOC_MR_US,       CLASS_ID_ACTIVITY_CALENDAR,                       0, {  0,   0,  13,   0,   0, 255}, 10, Obj_Activity_Calendar, 1, Obj_Activity_Calendar_Methods},
  
  //single action schedule
  {ASSOC_MR_US,       CLASS_ID_SINGLE_ACTION_SCHEDULE,                  0, {  0,   0,  15,   0,   0, 255}, 4,  Obj_Billing_Schedule, 0, NULL},
  {ASSOC_MR_US_PH,    CLASS_ID_SINGLE_ACTION_SCHEDULE,                  0, {  0,   0,  15,   0,   4, 255}, 4, Obj_Meter_HES_Push_Schedule, 0, NULL },
  {ASSOC_MR_US_FW,    CLASS_ID_SINGLE_ACTION_SCHEDULE,                  0, {  0,   0,  15,   0,   2, 255}, 4, Obj_Image_activation_Schedule, 0, NULL },
  //{ASSOC_MR_US_PH,    CLASS_ID_SINGLE_ACTION_SCHEDULE,                  0, {  0,   0,  15,   1,   4, 255}, 4, Obj_Meter_IHD_Push_Schedule, 0, NULL },
  
  //hdlcsetup
  {ASSOC_MR_US,       CLASS_ID_IEC_HDLC_SETUP,                          1, {  0,   0,  22,   0,   0, 255}, 9,  Obj_HDLC_Setup, 0, NULL},
  
  //push setup
  {ASSOC_US_PH,       CLASS_ID_PUSH_SETUP,                0, {  0,   0,  25,   9,   0, 255}, 7, Obj_Meter_HES_Push_attribute, 1, Obj_Meter_HES_Push_method },
  //  {ASSOC_US_PH_IHD,        CLASS_ID_PUSH_SETUP,                0, {  0,   1,  25,   9,   0, 255}, 7, Obj_HES_IHD_Push_attribute, 1, Obj_HES_IHD_Push_method },
  //  {ASSOC_US_PH_IHD,        CLASS_ID_PUSH_SETUP,                0, {  0,   2,  25,   9,   0, 255}, 7, Obj_IHD_HES_Push_attribute, 1, Obj_IHD_HES_Push_method },
  //  {ASSOC_US_PH_IHD,        CLASS_ID_PUSH_SETUP,                0, {  0,   3,  25,   9,   0, 255}, 7, Obj_Meter_IHD_Push_attribute, 1, Obj_Meter_IHD_Push_method },
  {ASSOC_US_PH,       CLASS_ID_PUSH_SETUP,                0, {  0,   4,  25,   9,   0, 255}, 7, Obj_Events_Push_attribute, 1, Obj_Events_Push_method },
  
  //TCP/UDP setup
  {ASSOC_MR_US,       CLASS_ID_TCP_UDP_SETUP,             0, {  0,   0,   25,  0,   0, 255}, 6, Obj_TCP_UDP_SETUP, 0, NULL},
  
  //IPv4 setup
  {ASSOC_MR_US,       CLASS_ID_IPV4_SETUP,                0, {  0,   0,  25,  1,    0, 255}, 10, Obj_IPv4_Setup, 3, IPv4_Setup_methods},
  
  //IPv6 setup
  {ASSOC_MR_US,       CLASS_ID_IPV6_SETUP,                0, {  0,   0,  25,  7,    0, 255}, 10, Obj_IPv6_Setup, 2, IPv6_Setup_methods},
  
  //Security setup
  {ASSOC_MR_US,       CLASS_ID_SECURITY_SETUP,            0, {  0,   0,   43,   0,  2, 255}, 5, Obj_Security_Setup_MR, 2, Obj_Security_Setup_Methods_MR },
  {ASSOC_MR_US,       CLASS_ID_SECURITY_SETUP,            0, {  0,   0,   43,   0,  3, 255}, 5, Obj_Security_Setup_US, 2, Obj_Security_Setup_Methods_US },
  {ASSOC_MR_US_PH,    CLASS_ID_SECURITY_SETUP,            0, {  0,   0,   43,   0,  4, 255}, 5, Obj_Security_Setup_PUSH, 2, Obj_Security_Setup_Methods_PUSH },
  {ASSOC_MR_US_FW,    CLASS_ID_SECURITY_SETUP,            0, {  0,   0,   43,   0,  5, 255}, 5, Obj_Security_Setup_FIRMWARE, 2, Obj_Security_Setup_Methods_FIRMWARE },
  //{ASSOC_MR_US,       CLASS_ID_SECURITY_SETUP,            0, {  0,   0,   43,   0,  6, 255}, 5, Obj_Security_Setup_IHD, 2, Obj_Security_Setup_Methods_IHD },
  
  //disconnect control
  {ASSOC_MR_US,       CLASS_ID_DISCONNECT_CONTROL,        0, {  0,   0,   96,   3, 10, 255}, 4, Obj_Connect_Disconnect, 2, Obj_Connect_Disconnect_Methods },
  
  //limiter
  {ASSOC_MR_US,       CLASS_ID_LIMITER,                   0, {  0,   0,   17,   0,  0, 255}, 11,limiter_load_profile_Obj, 0, NULL },
  { 0,                0,                                  0, {  0,   0,   0,   0,   0,   0}, 0,  0,                      0,    0}
};
//////////////////////////////////// Objects list ends ////////////////////////////////