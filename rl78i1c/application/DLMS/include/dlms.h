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

#ifndef DLMS_H
#define DLMS_H

#include "errorcodes.h"
#include "bytebuffer.h"
#include "helpers.h"
#include "dlmssettings.h"
#include "apdu.h"
#include "replydata.h"
#include "parameters.h"

#ifdef WIN32
        #define memcpy_far memcpy
#else
        #ifdef __CCRL__
                #define memcpy_far _COM_memcpy_ff
        #endif
#endif

#define BUFF_SIZE       1200
#define DATA_BUFFER_SIZE 2400
//#include "stack_heap_trace.h" 
extern uint8_t Null_LN[];
extern int64_t get_numeric_item(int item);
extern int get_string_item(uint8_t *buf, int len, int item);
extern const struct object_desc_s object_list[];
extern uint8_t LLS_MR[10];
extern uint8_t HLS_US[18];
extern uint8_t HLS_FW[18];
extern const uint32_t Device_Logical_Address;
extern uint16_t Device_Physical_Address;

extern void svr_Key_Update(dlmsSettings* settings);
typedef void(*exchange_data_t)(void* data, int direction);
typedef void(*method_t)(uint8_t* data, uint16_t data_len, uint8_t* response, uint16_t* response_len);

typedef struct
{
  unsigned short Year;
  uint8_t Month;
  uint8_t Day;
  uint8_t Weekday;
  uint8_t Hr;
  uint8_t Min;
} sSA_Range;
  
struct attribute_desc_s
{
  uint8_t id;
  uint64_t access_mode;
  uint8_t type;
  void* addr;
  exchange_data_t callback;
};

struct method_desc_s
{
  uint8_t id;
  uint16_t access_mode;
  method_t callback;
};

struct object_desc_s
{
  uint8_t assoc;
  uint8_t class_id;
  uint8_t version;
  uint8_t instance_id[6];
  uint8_t attr_elements;
  const struct attribute_desc_s* attrs;
  uint8_t method_elements;
  const struct method_desc_s* methods;
};

typedef struct
{
  uint8_t ATTR_ACCESS : 3;
}st_Asso_Attribute_Access;

typedef struct
{
  uint8_t CMD;
  uint8_t CMD_type;
  uint8_t InvokeID_Priority;
  uint16_t IC;
  uint8_t *LN;
  uint8_t index;
  uint8_t result;
  uint8_t *data;
  uint16_t data_len;
  uint8_t block_len;
  uint8_t Assoc;
  uint8_t selection;
  const struct attribute_desc_s *attribute;
  const struct object_desc_s *object;
  uint16_t Num_of_Objects;
  uint16_t Pos_of_Object;
}st_process_command;

typedef struct
	{
          uint8_t *data;
          unsigned long size;
          unsigned long position;
	} ByteBuffer;

typedef struct
{
  uint8_t Response;
  uint8_t Response_type;
  uint8_t InvokeID_Priority;
  uint8_t IsBlock;
  uint8_t block_len;
  ByteBuffer data;
  uint8_t data_len;
  uint8_t result;
  uint8_t IsNextFrame;
  uint8_t IsNextBlock;
  uint16_t FromIndex;
  uint16_t ToIndex;
  uint16_t CurrentIndex;
  uint16_t Object_Availables;
}st_send_data;

enum
{
  ATTR_READ = 0x01,
  ATTR_WRITE = 0x02
};

/* Standardised DLMS PDUs used in COSEM - IEC62056-53 8.1 */
enum
{
  /* DLMS PDUs (no encryption selected) */
  PDU_INITIATE_REQUEST = 1,
  PDU_READ_REQUEST = 5,
  PDU_WRITE_REQUEST = 6,
  PDU_INITIATE_RESPONSE = 8,
  PDU_READ_RESPONSE = 12,
  PDU_WRITE_RESPONSE = 13,
  PDU_CONFIRMED_SERVICE_ERROR = 14,
  PDU_UNCONFIRMED_WRITE_REQUEST = 22,
  PDU_INFORMATION_REPORT_REQUEST = 24,
  /* The ACSE APDUs */
  PDU_AARQ = 96,
  PDU_AARE = 97,
  PDU_RLRQ = 98,
  PDU_RLRE = 99,
  /* APDUs used for data commmunication services, using LN referencing */
  PDU_GET_REQUEST = 192,
  PDU_SET_REQUEST = 193,
  PDU_EVENT_NOTIFICATION_REQUEST = 194,
  PDU_ACTION_REQUEST = 195,
  PDU_GET_RESPONSE = 196,
  PDU_SET_RESPONSE = 197,
  PDU_ACTION_RESPONSE = 199,
  /* Global ciphered PDUs */
  PDU_GLO_GET_REQUEST = 200,
  PDU_GLO_SET_REQUEST = 201,
  PDU_GLO_EVENT_NOTIFICATION_REQUEST = 202,
  PDU_GLO_ACTION_REQUEST = 203,
  PDU_GLO_GET_RESPONSE = 204,
  PDU_GLO_SET_RESPONSE = 205,
  PDU_GLO_ACTION_RESPONSE = 207,
  /* Dedicated ciphered PDUs */
  PDU_DED_GET_REQUEST = 208,
  PDU_DED_SET_REQUEST = 209,
  PDU_DED_EVENT_NOTIFICATION_REQUEST = 210,
  PDU_DED_ACTION_REQUEST = 211,
  PDU_DED_GET_RESPONSE = 212,
  PDU_DED_SET_RESPONSE = 213,
  PDU_DED_ACTION_RESPONSE = 215
};


enum
{
  Result_Normal = 1,
  Result_Block = 2,
  Result_List = 3
};

typedef enum
{
      OBIS_GROUP_A_ABSTRACT_OBJECTS = 0,
      OBIS_GROUP_A_ELECTRICITY_OBJECTS = 1,
      OBIS_GROUP_A_HEAT_COST_OBJECTS = 4,
      OBIS_GROUP_A_COOLING_OBJECTS = 5,
      OBIS_GROUP_A_HEAT_OBJECTS = 6,
      OBIS_GROUP_A_GAS_OBJECTS = 7,
      OBIS_GROUP_A_COLD_WATER_OBJECTS = 8,
      OBIS_GROUP_A_HOT_WATER_OBJECTS = 9
	/* 10 to 255 are reserved */
} obis_group_a_t;

typedef enum
{
      OBIS_GROUP_B_NO_CHANNEL = 0,
      OBIS_GROUP_B_CHANNEL_1 = 1,
	/* 1 to 64 are for channels 1 to 64 */
	/* 65 to 127 are reserved */
	/* 128 to 254 are manufacturer specific */
	/* 255 is reserved */
} obis_group_b_t;

typedef enum
{
      OBIS_ABSTRACT_GROUP_C_GENERAL_PURPOSE_COSEM_OBJECTS = 0,
      OBIS_ABSTRACT_GROUP_C_COSEM_OBJECTS_OF_IC_CLOCK = 1,
      OBIS_ABSTRACT_GROUP_C_COSEM_OBJECTS_OF_IC_PSTN_MODEM_CONFIG = 2,
      OBIS_ABSTRACT_GROUP_C_COSEM_OBJECTS_OF_IC_SCRIPT_TABLE = 10,
      OBIS_ABSTRACT_GROUP_C_COSEM_OBJECTS_OF_IC_SPECIAL_DAYS_TABLE = 11,
      OBIS_ABSTRACT_GROUP_C_COSEM_OBJECTS_OF_IC_SCHEDULE = 12,
      OBIS_ABSTRACT_GROUP_C_COSEM_OBJECTS_OF_IC_ACTIVITY_CALENDAR = 13,
      OBIS_ABSTRACT_GROUP_C_COSEM_OBJECTS_OF_IC_REGISTER_ACTIVATION = 14,
      OBIS_ABSTRACT_GROUP_C_COSEM_OBJECTS_OF_IC_SINGLE_ACTION_SCHEDULE = 15,
      OBIS_ABSTRACT_GROUP_C_COSEM_OBJECTS_OF_IC_IEC_LOCAL_PORT_SETUP = 20,
      OBIS_ABSTRACT_GROUP_C_STANDARD_READOUT_DEFINITIONS = 21,
      OBIS_ABSTRACT_GROUP_C_COSEM_OBJECTS_OF_IC_IEC_HDLC_SETUP = 22,
      OBIS_ABSTRACT_GROUP_C_COSEM_OBJECTS_OF_IC_IEC_TWISTED_PAIR = 23,
      OBIS_ABSTRACT_GROUP_C_COSEM_OBJECTS_OF_IC_ASSOCIATION_SN_LN = 40,
      OBIS_ABSTRACT_GROUP_C_COSEM_OBJECTS_OF_IC_SAP_ASSIGNMENT = 41,
      OBIS_ABSTRACT_GROUP_C_COSEM_LOGICAL_DEVICE_NAME = 42,
      OBIS_ABSTRACT_GROUP_C_COSEM_OBJECTS_OF_IC_UTILITY_TABLES = 65,
      /* 0 to 89 are context specific identifiers */
      OBIS_ABSTRACT_GROUP_C_COUNTRY_SPECIFIC_IDENTIFIERS = 94,

      OBIS_ABSTRACT_GROUP_C_GENERAL_SERVICE_ENTRIES = 96,
      OBIS_ABSTRACT_GROUP_C_GENERAL_ERROR_MESSAGES = 97,
      OBIS_ABSTRACT_GROUP_C_GENERAL_LIST_OBJECTS = 98,
      OBIS_ABSTRACT_GROUP_C_INACTIVE_OBJECTS = 127,
      /* 128 to 254 are manufacturer specific */
      /* 255 is reserved */
      /* All others are reserved */
} obis_abstract_group_c_t;

typedef enum
{
      OBIS_ABSTRACT_SERVICE_ENTRIES_GROUP_D_DEVICE_ID_NUMBERS = 1,
      OBIS_ABSTRACT_SERVICE_ENTRIES_GROUP_D_PARAMETER_CHANGES = 2,
      OBIS_ABSTRACT_SERVICE_ENTRIES_GROUP_D_IO_CONTROL_SIGNALS = 3,
      OBIS_ABSTRACT_SERVICE_ENTRIES_GROUP_D_INTERNAL_CONTROL_SIGNALS = 4,
      OBIS_ABSTRACT_SERVICE_ENTRIES_GROUP_D_INTERNAL_OPERATING_STATUS = 5,
      OBIS_ABSTRACT_SERVICE_ENTRIES_GROUP_D_BATTERY_ENTRIES = 6,
      OBIS_ABSTRACT_SERVICE_ENTRIES_GROUP_D_NUMBER_OF_POWER_FAILURES = 7,
      OBIS_ABSTRACT_SERVICE_ENTRIES_GROUP_D_OPERATING_TIME = 8,
      /* 50 to 96 are manufacturer specific */
      OBIS_ABSTRACT_SERVICE_ENTRIES_GROUP_D_MANUFACTURER_SPECIFIC_START = 50,
      OBIS_ABSTRACT_SERVICE_ENTRIES_GROUP_D_MANUFACTURER_SPECIFIC_END = 96,
      /* All other codes are reserved */
} obis_abstract_service_entries_group_d_t;

typedef enum
{
      OBIS_ABSTRACT_SERVICE_ENTRIES_PARAMETER_CHANGE_GROUP_E_NUMBER_OF_CHANGES = 0,
      OBIS_ABSTRACT_SERVICE_ENTRIES_PARAMETER_CHANGE_GROUP_E_DATE_OF_LAST_CONFIG_CHANGE = 1,
      OBIS_ABSTRACT_SERVICE_ENTRIES_PARAMETER_CHANGE_GROUP_E_DATE_OF_LAST_TIMESWITCH_CHANGE = 2,
      OBIS_ABSTRACT_SERVICE_ENTRIES_PARAMETER_CHANGE_GROUP_E_DATE_OF_LIST_RIPPLE_CHANGE = 3,
      OBIS_ABSTRACT_SERVICE_ENTRIES_PARAMETER_CHANGE_GROUP_E_SECURITY_SWITCH_STATUS = 4,
      OBIS_ABSTRACT_SERVICE_ENTRIES_PARAMETER_CHANGE_GROUP_E_DATE_OF_LAST_CALIBRATION = 5,
      OBIS_ABSTRACT_SERVICE_ENTRIES_PARAMETER_CHANGE_GROUP_E_DATE_OF_NEXT_CONFIG_CHANGE = 6,
      OBIS_ABSTRACT_SERVICE_ENTRIES_PARAMETER_CHANGE_GROUP_E_NUMBER_OF_PROTECTED_CHANGES = 10,
      OBIS_ABSTRACT_SERVICE_ENTRIES_PARAMETER_CHANGE_GROUP_E_DATE_OF_LAST_PROTECTED_CHANGE = 11,
} obis_abstract_service_entries_parameter_change_group_e_t;

typedef enum
{
      OBIS_ABSTRACT_ERROR_MESSAGE_GROUP_D_ERROR_OBJECT = 97,
      /* All other codes are reserved */
} obis_abstract_error_message_group_d_t;

typedef enum
{
      OBIS_ELECTRICITY_GROUP_C_GENERAL_PURPOSE_OBJECT = 0,
      OBIS_ELECTRICITY_GROUP_C_ACTIVE_POWER_PLUS = 1,
      OBIS_ELECTRICITY_GROUP_C_ACTIVE_POWER_MINUS = 2,
      OBIS_ELECTRICITY_GROUP_C_REACTIVE_POWER_PLUS = 3,
      OBIS_ELECTRICITY_GROUP_C_REACTIVE_POWER_MINUS = 4,
      OBIS_ELECTRICITY_GROUP_C_REACTIVE_QI = 5,
      OBIS_ELECTRICITY_GROUP_C_REACTIVE_QII = 6,
      OBIS_ELECTRICITY_GROUP_C_REACTIVE_QIII = 7,
      OBIS_ELECTRICITY_GROUP_C_REACTIVE_QIV = 8,
      OBIS_ELECTRICITY_GROUP_C_APPARENT_POWER_PLUS = 9,
      OBIS_ELECTRICITY_GROUP_C_APPARENT_POWER_MINUS = 10,
      OBIS_ELECTRICITY_GROUP_C_CURRENT = 11,
      OBIS_ELECTRICITY_GROUP_C_VOLTAGE = 12,
      OBIS_ELECTRICITY_GROUP_C_AVERAGE_POWER_FACTOR = 13,
      OBIS_ELECTRICITY_GROUP_C_SUPPLY_FREQUENCY = 14,
      OBIS_ELECTRICITY_GROUP_C_ACTIVE_QI_QIV_QII_QIII = 15,
      OBIS_ELECTRICITY_GROUP_C_ACTIVE_QI_QIV_MINUS_QII_QIII = 16,
      OBIS_ELECTRICITY_GROUP_C_ACTIVE_QI = 17,
      OBIS_ELECTRICITY_GROUP_C_ACTIVE_QII = 18,
      OBIS_ELECTRICITY_GROUP_C_ACTIVE_QIII = 19,
      OBIS_ELECTRICITY_GROUP_C_ACTIVE_QIV = 20,

      OBIS_ELECTRICITY_GROUP_C_L1_ACTIVE_POWER_PLUS = 21,
      OBIS_ELECTRICITY_GROUP_C_L1_ACTIVE_POWER_MINUS = 22,
      OBIS_ELECTRICITY_GROUP_C_L1_REACTIVE_POWER_PLUS = 23,
      OBIS_ELECTRICITY_GROUP_C_L1_REACTIVE_POWER_MINUS = 24,
      OBIS_ELECTRICITY_GROUP_C_L1_REACTIVE_QI = 25,
      OBIS_ELECTRICITY_GROUP_C_L1_REACTIVE_QII = 26,
      OBIS_ELECTRICITY_GROUP_C_L1_REACTIVE_QIII = 27,
      OBIS_ELECTRICITY_GROUP_C_L1_REACTIVE_QIV = 28,
      OBIS_ELECTRICITY_GROUP_C_L1_APPARENT_POWER_PLUS = 29,
      OBIS_ELECTRICITY_GROUP_C_L1_APPARENT_POWER_MINUS = 30,
      OBIS_ELECTRICITY_GROUP_C_L1_CURRENT = 31,
      OBIS_ELECTRICITY_GROUP_C_L1_VOLTAGE = 32,
      OBIS_ELECTRICITY_GROUP_C_L1_AVERAGE_POWER_FACTOR = 33,
      OBIS_ELECTRICITY_GROUP_C_L1_SUPPLY_FREQUENCY = 34,
      OBIS_ELECTRICITY_GROUP_C_L1_ACTIVE_QI_QIV_QII_QIII = 35,
      OBIS_ELECTRICITY_GROUP_C_L1_ACTIVE_QI_QIV_MINUS_QII_QIII = 36,
      OBIS_ELECTRICITY_GROUP_C_L1_ACTIVE_QI = 37,
      OBIS_ELECTRICITY_GROUP_C_L1_ACTIVE_QII = 38,
      OBIS_ELECTRICITY_GROUP_C_L1_ACTIVE_QIII = 39,
      OBIS_ELECTRICITY_GROUP_C_L1_ACTIVE_QIV = 40,

      OBIS_ELECTRICITY_GROUP_C_L2_ACTIVE_POWER_PLUS = 41,
      OBIS_ELECTRICITY_GROUP_C_L2_ACTIVE_POWER_MINUS = 42,
      OBIS_ELECTRICITY_GROUP_C_L2_REACTIVE_POWER_PLUS = 43,
      OBIS_ELECTRICITY_GROUP_C_L2_REACTIVE_POWER_MINUS = 44,
      OBIS_ELECTRICITY_GROUP_C_L2_REACTIVE_QI = 45,
      OBIS_ELECTRICITY_GROUP_C_L2_REACTIVE_QII = 46,
      OBIS_ELECTRICITY_GROUP_C_L2_REACTIVE_QIII = 47,
      OBIS_ELECTRICITY_GROUP_C_L2_REACTIVE_QIV = 48,
      OBIS_ELECTRICITY_GROUP_C_L2_APPARENT_POWER_PLUS = 49,
      OBIS_ELECTRICITY_GROUP_C_L2_APPARENT_POWER_MINUS = 50,
      OBIS_ELECTRICITY_GROUP_C_L2_CURRENT = 51,
      OBIS_ELECTRICITY_GROUP_C_L2_VOLTAGE = 52,
      OBIS_ELECTRICITY_GROUP_C_L2_AVERAGE_POWER_FACTOR = 53,
      OBIS_ELECTRICITY_GROUP_C_L2_SUPPLY_FREQUENCY = 54,
      OBIS_ELECTRICITY_GROUP_C_L2_ACTIVE_QI_QIV_QII_QIII = 55,
      OBIS_ELECTRICITY_GROUP_C_L2_ACTIVE_QI_QIV_MINUS_QII_QIII = 56,
      OBIS_ELECTRICITY_GROUP_C_L2_ACTIVE_QI = 57,
      OBIS_ELECTRICITY_GROUP_C_L2_ACTIVE_QII = 58,
      OBIS_ELECTRICITY_GROUP_C_L2_ACTIVE_QIII = 59,
      OBIS_ELECTRICITY_GROUP_C_L2_ACTIVE_QIV = 60,

      OBIS_ELECTRICITY_GROUP_C_L3_ACTIVE_POWER_PLUS = 61,
      OBIS_ELECTRICITY_GROUP_C_L3_ACTIVE_POWER_MINUS = 62,
      OBIS_ELECTRICITY_GROUP_C_L3_REACTIVE_POWER_PLUS = 63,
      OBIS_ELECTRICITY_GROUP_C_L3_REACTIVE_POWER_MINUS = 64,
      OBIS_ELECTRICITY_GROUP_C_L3_REACTIVE_QI = 65,
      OBIS_ELECTRICITY_GROUP_C_L3_REACTIVE_QII = 66,
      OBIS_ELECTRICITY_GROUP_C_L3_REACTIVE_QIII = 67,
      OBIS_ELECTRICITY_GROUP_C_L3_REACTIVE_QIV = 68,
      OBIS_ELECTRICITY_GROUP_C_L3_APPARENT_POWER_PLUS = 69,
      OBIS_ELECTRICITY_GROUP_C_L3_APPARENT_POWER_MINUS = 70,
      OBIS_ELECTRICITY_GROUP_C_L3_CURRENT = 71,
      OBIS_ELECTRICITY_GROUP_C_L3_VOLTAGE = 72,
      OBIS_ELECTRICITY_GROUP_C_L3_AVERAGE_POWER_FACTOR = 73,
      OBIS_ELECTRICITY_GROUP_C_L3_SUPPLY_FREQUENCY = 74,
      OBIS_ELECTRICITY_GROUP_C_L3_ACTIVE_QI_QIV_QII_QIII = 75,
      OBIS_ELECTRICITY_GROUP_C_L3_ACTIVE_QI_QIV_MINUS_QII_QIII = 76,
      OBIS_ELECTRICITY_GROUP_C_L3_ACTIVE_QI = 77,
      OBIS_ELECTRICITY_GROUP_C_L3_ACTIVE_QII = 78,
      OBIS_ELECTRICITY_GROUP_C_L3_ACTIVE_QIII = 79,
      OBIS_ELECTRICITY_GROUP_C_L3_ACTIVE_QIV = 80,

      OBIS_ELECTRICITY_GROUP_C_ANGLES = 81,
      OBIS_ELECTRICITY_GROUP_C_UNITLESS_QUANTITY = 82,

      OBIS_ELECTRICITY_GROUP_C_NEUTRAL_CURRENT = 91,
      OBIS_ELECTRICITY_GROUP_C_NEUTRAL_VOLTAGE = 92,

      OBIS_ELECTRICITY_GROUP_C_SERVICE_ENTRIES = 96,
      OBIS_ELECTRICITY_GROUP_C_ERROR_MESSAGES = 97,
      OBIS_ELECTRICITY_GROUP_C_LIST = 98,
      OBIS_ELECTRICITY_GROUP_C_DATA_PROFILE = 99,
      /* 128 to 254 are manufacturer specific */
      /* 255 is reserved */
} obis_electricity_group_c_t;

typedef enum
{
      OBIS_ELECTRICITY_GROUP_D_BILLING_PERIOD_AVERAGE = 0,
      OBIS_ELECTRICITY_GROUP_D_CUMULATIVE_MINIMUM_1 = 1,
      OBIS_ELECTRICITY_GROUP_D_CUMULATIVE_MAXIMUM_1 = 2,
      OBIS_ELECTRICITY_GROUP_D_MINIMUM_1 = 3,
      OBIS_ELECTRICITY_GROUP_D_CURRENT_AVERAGE_1 = 4,
      OBIS_ELECTRICITY_GROUP_D_LAST_AVERAGE_1 = 5,
      OBIS_ELECTRICITY_GROUP_D_MAXIMUM_1 = 6,
      OBIS_ELECTRICITY_GROUP_D_INSTANTANEOUS_VALUE = 7,
      OBIS_ELECTRICITY_GROUP_D_TIME_INTEGRAL_1 = 8,
      OBIS_ELECTRICITY_GROUP_D_TIME_INTEGRAL_2 = 9,
      OBIS_ELECTRICITY_GROUP_D_TIME_INTEGRAL_3 = 10,

      OBIS_ELECTRICITY_GROUP_D_CUMULATIVE_MINIMUM_2 = 11,
      OBIS_ELECTRICITY_GROUP_D_CUMULATIVE_MAXIMUM_2 = 12,
      OBIS_ELECTRICITY_GROUP_D_MINIMUM_2 = 13,
      OBIS_ELECTRICITY_GROUP_D_CURRENT_AVERAGE_2 = 14,
      OBIS_ELECTRICITY_GROUP_D_LAST_AVERAGE_2 = 15,
      OBIS_ELECTRICITY_GROUP_D_MAXIMUM_2 = 16,

      OBIS_ELECTRICITY_GROUP_D_CUMULATIVE_MINIMUM_3 = 21,
      OBIS_ELECTRICITY_GROUP_D_CUMULATIVE_MAXIMUM_3 = 22,
      OBIS_ELECTRICITY_GROUP_D_MINIMUM_3 = 23,
      OBIS_ELECTRICITY_GROUP_D_CURRENT_AVERAGE_3 = 24,
      OBIS_ELECTRICITY_GROUP_D_LAST_AVERAGE_3 = 25,
      OBIS_ELECTRICITY_GROUP_D_MAXIMUM_3 = 26,

      OBIS_ELECTRICITY_GROUP_D_CURRENT_AVERAGE_5 = 27,
      OBIS_ELECTRICITY_GROUP_D_CURRENT_AVERAGE_6 = 28,
      OBIS_ELECTRICITY_GROUP_D_TIME_INTEGRAL_5 = 29,
      OBIS_ELECTRICITY_GROUP_D_TIME_INTEGRAL_6 = 30,

      OBIS_ELECTRICITY_GROUP_D_UNDER_LIMIT_THRESHOLD = 31,
      OBIS_ELECTRICITY_GROUP_D_UNDER_LIMIT_OCCURRENCE_COUNTER = 32,
      OBIS_ELECTRICITY_GROUP_D_UNDER_LIMIT_DURATION = 33,
      OBIS_ELECTRICITY_GROUP_D_UNDER_LIMIT_MAGNITUDE = 34,
      OBIS_ELECTRICITY_GROUP_D_OVER_LIMIT_THRESHOLD = 35,
      OBIS_ELECTRICITY_GROUP_D_OVER_LIMIT_OCCURRENCE_COUNTER = 36,
      OBIS_ELECTRICITY_GROUP_D_OVER_LIMIT_DURATION = 37,
      OBIS_ELECTRICITY_GROUP_D_OVER_LIMIT_MAGNITUDE = 38,
      OBIS_ELECTRICITY_GROUP_D_MISSING_THRESHOLD = 39,
      OBIS_ELECTRICITY_GROUP_D_MISSING_OCCURRENCE_COUNTER = 40,
      OBIS_ELECTRICITY_GROUP_D_MISSING_DURATION = 41,
      OBIS_ELECTRICITY_GROUP_D_MISSING_MAGNITUDE = 42,

      OBIS_ELECTRICITY_GROUP_D_TEST_AVERAGE = 55,

      OBIS_ELECTRICITY_GROUP_D_TIME_INTEGRAL_4 = 58,
      /* 128 to 254 are manufacturer specific */
      /* All other codes are reserved */
} obis_electricity_group_d_t;

typedef enum
{
      OBIS_ELECTRICITY_GENERAL_PURPOSE_GROUP_D_ELECTRICITY_ID = 0,
      OBIS_ELECTRICITY_GENERAL_PURPOSE_GROUP_D_BILLING_PERIOD_COUNTER = 1,
      OBIS_ELECTRICITY_GENERAL_PURPOSE_GROUP_D_PROGRAM_ENTRY = 2,
      OBIS_ELECTRICITY_GENERAL_PURPOSE_GROUP_D_OUTPUT_PULSE_CONSTANT = 3,
      OBIS_ELECTRICITY_GENERAL_PURPOSE_GROUP_D_RATIO = 4,
      OBIS_ELECTRICITY_GENERAL_PURPOSE_GROUP_D_NOMINAL_VALUE = 6,
      OBIS_ELECTRICITY_GENERAL_PURPOSE_GROUP_D_INPUT_PULSE_CONSTANT = 7,
      OBIS_ELECTRICITY_GENERAL_PURPOSE_GROUP_D_MEASUREMENT_PERIOD_DURATION = 8,
      OBIS_ELECTRICITY_GENERAL_PURPOSE_GROUP_D_TIME_ENTRY = 9,
      OBIS_ELECTRICITY_GENERAL_PURPOSE_GROUP_D_COEFFICIENT = 10,
      OBIS_ELECTRICITY_GENERAL_PURPOSE_GROUP_D_MEASUREMENT_METHOD = 11,
} obis_electricity_general_purpose_group_d_t;

typedef enum
{
      /* These codes follow the IDD prefix for the respective country */
      OBIS_COUNTRY_SPECIFIC_GROUP_D_FINNISH = 0,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_USA = 1,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_CANADIAN = 2,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_RUSSIAN = 7,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_CZECH = 10,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_BULGARIAN = 11,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_CROATIAN = 12,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_IRISH = 13,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_ISRAELI = 14,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_UKRAINE = 15,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_YUGOSLAVIAN = 16,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_SOUTH_AFRICAN = 27,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_GREEK = 30,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_DUTCH = 31,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_BELGIAN = 32,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_FRENCH = 33,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_SPANISH = 34,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_PORTUGUESE = 35,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_HUNGARIAN = 36,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_SLOVENIAN = 38,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_ITALIAN = 39,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_ROMANIAN = 40,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_SWISS = 41,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_SLOVAKIAN = 42,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_AUSTRIAN = 43,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_UNITED_KINGDOM = 44,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_DANISH = 45,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_SWEDISH = 46,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_NORWEGIAN = 47,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_POLISH = 48,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_GERMAN = 49,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_BRAZILIAN = 55,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_AUSTRALIAN = 61,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_INDONESIAN = 62,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_NEW_ZEALAND = 64,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_SINGAPORE = 65,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_JAPANESE = 81,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_CHINESE = 86,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_TURKISH = 90,
      OBIS_COUNTRY_SPECIFIC_GROUP_D_INDIAN = 91,
      /* All other codes are reserved */
} obis_country_specific_group_d_t;

typedef enum
{
      OBIS_ELECTRICITY_GROUP_E_TOTAL = 0,
      OBIS_ELECTRICITY_GROUP_E_RATE_1 = 1,
      /* 1 to 63 are for rates 1 to 63 */
      /* 128 to 254 are manufacturer specific */
      /* 255 is reserved */
} obis_electricity_group_e_t;

typedef enum
{
      OBIS_ELECTRICITY_EXTENDED_GROUP_E_TOTAL = 0,
      OBIS_ELECTRICITY_EXTENDED_GROUP_E_1ST_HARMONIC = 1,
      OBIS_ELECTRICITY_EXTENDED_GROUP_E_2ND_HARMONIC = 2,
      OBIS_ELECTRICITY_EXTENDED_GROUP_E_3RD_HARMONIC = 3,
      OBIS_ELECTRICITY_EXTENDED_GROUP_E_4TH_HARMONIC = 4,
      OBIS_ELECTRICITY_EXTENDED_GROUP_E_5TH_HARMONIC = 5,
      OBIS_ELECTRICITY_EXTENDED_GROUP_E_6TH_HARMONIC = 6,
      OBIS_ELECTRICITY_EXTENDED_GROUP_E_7TH_HARMONIC = 7,
      OBIS_ELECTRICITY_EXTENDED_GROUP_E_8TH_HARMONIC = 8,
      OBIS_ELECTRICITY_EXTENDED_GROUP_E_9TH_HARMONIC = 9,
      /* 1 to 127 are for harmonics 1 to 127 */
      /* 128 to 254 are manufacturer specific */
      /* 255 is reserved */
} obis_electricity_extended_group_e_t;

typedef enum
{
      OBIS_BILLING_PERIOD_GROUP_F_FUTURE_PERIOD = 0,
      OBIS_BILLING_PERIOD_GROUP_F_PERIOD_1 = 1,

      OBIS_BILLING_PERIOD_GROUP_F_MOST_RECENT = 101,
      OBIS_BILLING_PERIOD_GROUP_F_2_MOST_RECENT = 102,
      /* 103 to 125 are for 3 most recent to 25 most recent */
      OBIS_BILLING_PERIOD_GROUP_F_UNSPECIFIED_MOST_RECENT = 126,
      OBIS_BILLING_PERIOD_GROUP_F_UNSPECIFIED = 255,
} obis_billing_periods_group_f_t;

typedef enum
{
      Type_MR,
      Type_US,
      Type_PH,
      Type_FW,
      Type_IHD
}eInvocation_Counter_Type;

int dlms_getDataFromBlock(gxByteBuffer* data, int index);

int dlms_getData2(
	dlmsServerSettings* dlmssettings,
    dlmsSettings* settings,
    gxByteBuffer* reply,
    gxReplyData* data);

static const unsigned char LLC_SEND_BYTES[3] = { 0xE6, 0xE6, 0x00 };
static const unsigned char LLC_REPLY_BYTES[3] = { 0xE6, 0xE7, 0x00 };

    /**
    * Get PDU as HDLC frame.
    */
    int dlms_getHdlcFrame(
        dlmsSettings* settings,
        int frame,
        gxByteBuffer* data,
        gxByteBuffer* reply);


    /**
         * Split DLMS PDU to wrapper frames.
         *
         * @param settings
         *            DLMS settings.
         * @param data
         *            Wrapped data.
         * @return Wrapper frames.
    */
    int dlms_getWrapperFrame(
        dlmsSettings* settings,
        gxByteBuffer* data,
        gxByteBuffer* reply,int frame);

    /**
    * Get next logical name PDU.
    *
    * @param p
    *            LN parameters.
    * @param reply
    *            Generated message.
    */
    int dlms_getLNPdu(
        gxLNParameters* p,
        gxByteBuffer* reply);

    //Return DLMS_ERROR_CODE_FALSE if LLC bytes are not included.
    int dlms_checkLLCBytes(
        dlmsSettings* settings,
        gxByteBuffer* data);

    int dlms_getHdlcData(
		dlmsServerSettings* dlmssettings,
        unsigned char server,
        dlmsSettings* settings,
        gxByteBuffer* reply,
        gxReplyData* data,
        unsigned char* frame);

    int dlms_getAddressBytes(
      unsigned long value,
      gxByteBuffer* bytes);
      
    int dlms_checkHdlcAddress(
      dlmsServerSettings* dlmssettings,
      unsigned char server,
      dlmsSettings* settings,
      gxByteBuffer* reply,
      int index);
    
    unsigned long check_Wrapper_DeviceAddress(void);
    unsigned char dlms_getInvokeIDPriority(dlmsSettings* settings);
    long dlms_getLongInvokeIDPriority(dlmsSettings* settings);
    unsigned char getGeneralBlockTransfer(dlmsSettings* settings);
    int dlms_getPdu(
        dlmsSettings* settings,
        gxReplyData* data);

    int dlms_getData2(
		dlmsServerSettings* dlmssettings,
        dlmsSettings* settings,
        gxByteBuffer* reply,
        gxReplyData* data);

    /**
    * Add LLC bytes to generated message.
    *
    * @param settings
    *            DLMS settings.
    * @param data
    *            Data where bytes are added.
    */
    void dlms_addLLCBytes(
        dlmsSettings* settings,
        gxByteBuffer* data);

    int dlms_generateChallenge(
        gxByteBuffer* challenge);

    /**
        * Chipher text.
        *
        * @param auth
        *            Authentication level.
        * @param data
        *            Text to chipher.
        * @param secret
        *            Secret.
        * @return Chiphered text.
        */
    int dlms_secure(
        dlmsSettings* settings,
        unsigned long ic,
        gxByteBuffer* data,
        gxByteBuffer* secret,
        gxByteBuffer* reply);
	void svr_reset(
		dlmsServerSettings* settings);
	void svr_reset2(
		dlmsServerSettings* settings);
	void svr_reset3(
		dlmsServerSettings* settings);
        
    int get_next_GBT_block(
        dlmsSettings* settings,
        gxByteBuffer* reply);
    
    int svr_handleGetRequestNormal(
	dlmsServerSettings* settings,
	gxByteBuffer* data);
	
    int svr_getRequestNextDataBlock(
            dlmsServerSettings* settings,
            gxByteBuffer *data, unsigned char *status);
            
    int svr_handleGetRequest(
            dlmsServerSettings* settings,
            gxByteBuffer* data);
            
    int svr_handleSetRequestNormal(
            dlmsServerSettings* settings,
            gxByteBuffer* data,
            DLMS_GET_COMMAND_TYPE type,
            gxLNParameters *p);
            
    int svr_handleSetRequestWithDataBlock(
            dlmsServerSettings* settings,
            gxByteBuffer *data,
            gxLNParameters *p);
            
    int svr_handleSetRequest(
            dlmsServerSettings* settings,
            gxByteBuffer* data);
            
    int svr_handleMethodrequestNormal(
            dlmsServerSettings* settings,
            gxByteBuffer* data);
            
    int svr_handleMethodrequest(
            dlmsServerSettings* settings,
            gxByteBuffer* data);
            
    int svr_initialize(
            dlmsServerSettings* settings);
            
    void svr_setInitialize(dlmsServerSettings* settings);

    void svr_Get_Security(dlmsSettings* settings);

    int svr_HandleAarqRequest(
            dlmsServerSettings* settings,
            gxByteBuffer* data);
            
    void getUA(dlmsServerSettings* settings);

    int svr_handleSnrmRequest(
            dlmsServerSettings* settings);
            
    int svr_generateDisconnectRequest(dlmsServerSettings* settings);

    int svr_generateConfirmedServiceError(
            DLMS_CONFIRMED_SERVICE_ERROR service,
            DLMS_SERVICE_ERROR type,
            unsigned char code,
            gxByteBuffer * data);
            
    int svr_generateConfirmedServiceError_withLLC(
            dlmsServerSettings* settings,
            DLMS_CONFIRMED_SERVICE_ERROR service,
            DLMS_SERVICE_ERROR type,
            unsigned char code,
            gxByteBuffer * data);
            
    int svr_handleunsupportedservices(
            dlmsServerSettings* settings,
            gxByteBuffer* data);
            
    int svr_handleCommand(
            dlmsServerSettings* settings,
            DLMS_COMMAND cmd,
            gxByteBuffer* data,
            gxByteBuffer* reply);
            
    int svr_handleRequest(
            dlmsServerSettings* settings,
            gxByteBuffer* data,
            gxByteBuffer* reply);
            
    int svr_handleRequest2(
            dlmsServerSettings* settings,
            unsigned char* buff,
            unsigned short size,
            gxByteBuffer* reply);
            
    DLMS_SOURCE_DIAGNOSTIC svr_validateAuthentication(
        dlmsServerSettings* settings,
        DLMS_AUTHENTICATION authentication,
        gxByteBuffer* password);
            
    void svr_start(dlmsServerSettings *settings);

    int svr_connected(
            dlmsServerSettings *settings);
            
    int svr_disconnected(
            dlmsServerSettings *settings);

    unsigned char svr_isTarget(
        unsigned long int serverAddress,
        unsigned long clientAddress);
    void stuff_data_buffer(uint8_t *data);
    
    unsigned short get_asso_buff(dlmsServerSettings* settings, uint8_t Association, uint8_t *data, unsigned short len);
    
    void method_response_null(uint8_t *response, uint16_t *response_len);
    uint32_t get_invocation_counter(eInvocation_Counter_Type association_type);
#endif //DLMS_H
