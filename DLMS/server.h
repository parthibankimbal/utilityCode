#ifndef SERVER_H
#define SERVER_H

#include "factory_settings.h"

#undef ASSOCIATION_VER_3
#define SECURITY_ENABLE
#ifdef SECURITY_ENABLE
#define ENCRYPT_PUSH_MSG
#endif

#ifdef FACTORY_DEBUG_TEST_SECURITY_KEY_ENABLE

  #define DEFAULT_LLS_MR { 9, 8 ,'0', '0','0','0','0','0','0','0' }
  #define DEFAULT_HLS_US { 17, 16, 'a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a' }
  #define DEFAULT_HLS_FW { 17, 16, 'a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','b' }

  #define DEFAULT_US_METER_SECURITY_KEYS { {'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A'}, \
                                         {'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A'}, \
                                         {'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A'}, \
                                         {'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B'} }
#endif

extern uint8_t LLS_MR[10];
extern uint8_t HLS_US[18];
extern uint8_t HLS_FW[18];

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "dlms.h"
#include "datalog.h"
#include "em_type.h"
#include "em_measurement.h"
#include "em_operation.h"
#include "inst_read.h"
#include "em_energies.h"
#include "rtc_user.h"
#include "Activity_Calendar.h"
#include "datalog.h"
#include "main.h"
#include "platform.h"
#include "communication.h"
#include "factory_settings.h"


#undef TEMPERATURE_SUPPORT_IN_INSTANT

#define SCALER_CURRENT              0
#define SCALER_VOLTAGE              0
#define SCALER_POWER_FACTOR         0
#define SCALER_FREQUENCY            0
#define SCALER_APPARENT_POWER       3
#define SCALER_ACTIVE_POWER         3
#define SCALER_REACTIVE_POWER       3
#define SCALER_ACTIVE_ENERGY        3
#define SCALER_REACTIVE_ENERGY      3
#define SCALER_APPARENT_ENERGY      3
#define SCALER_TEMPERATURE          0

#define DAILY_PROFILE_ENTRIES_TOTAL 36

typedef enum
{
  e_Postpaid,
  e_Prepaid,
}e_payment_mode_type;

typedef struct
{
  uint8_t ctoSChallenge[16];
  uint8_t stoCChallenge[16];
  uint8_t password[16];
  uint8_t blockCipherKey[16];
  uint8_t dedicatedCipherKey[16];
  uint8_t systemTitle[8];
  uint8_t clientSystemTitle[8];
  uint8_t authenticationKey[16];
  uint8_t CipherKey[16];
  uint8_t received_data[900];
  uint8_t long_buffer[1800];
  uint8_t long_info_buffer[1800];
}dlms_user_data;

#define PDU_SIZE 512

  //clock starts
  extern uint32_t  range_num_entries, range_start_entry;
  extern int16_t clock_time_zone;
  extern uint8_t set_clock_time_zone;
  extern uint8_t set_rtc_data;
  extern int16_t clock_time_zone;
  extern uint8_t set_clock_time_zone;
  extern const uint8_t Meter_category[4];
  extern const uint8_t Current_rating[];
  extern uint16_t Year_of_Manufacture;
  extern const uint8_t Manufacturer_Name[14];
  extern uint32_t Cum_Power_Off_Time;
  extern uint8_t tariff_id[8];
  extern uint8_t passive_tariff_id[8];
  extern uint8_t server_system_title[10];
  extern uint8_t client_system_title[10];
  extern uint8_t client_system_title_MR[10];
  extern uint8_t client_system_title_US[10];
  extern uint8_t client_system_title_PUSH[10];
  extern uint8_t client_system_title_FIRMWARE[10];
  extern uint8_t client_system_title_IHD[10];
  extern const uint8_t Meter_Type;
  extern uint8_t US_Security_Keys[4][16];

  extern uint32_t min_over_threshold_duration_load;
  extern uint32_t min_under_threshold_duration_load;
  extern uint32_t threshold_active_load;
  extern const uint32_t Device_Logical_Address;
  extern uint16_t Device_Physical_Address;
  extern uint16_t Device_Physical_Address_Bak;
  extern uint8_t Set_Device_Physical_Address;
  extern uint8_t update_security_keys;
  extern uint32_t image_activation_time;
  extern uint8_t generate_bill_on_activity_cal;
  extern uint8_t generate_bill_on_firmware_upgrade;
  extern enum IM_TX_STAT Image_Transfer_Status;
  extern uint8_t Image_Transferred_Blocks_Status[];
  extern uint8_t g_image_transfer_status;
  extern uint8_t g_image_activate_counter;
  //clock ends
  
  #pragma pack
  typedef struct
  {
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint16_t crc;
  }st_season_time;
  #pragma unpack
  
  typedef struct
  {
    dlmsServerSettings settings;
  } connection;

  typedef struct
  {
    uint16_t Image_Version;
    uint32_t Image_Size;
  }st_Image_info;

  typedef struct
  {
    uint32_t ATTR_ACCESS_PC : 3;
    uint32_t ATTR_ACCESS_MR : 3;
    uint32_t ATTR_ACCESS_US : 3;
    uint32_t ATTR_ACCESS_PUSH : 3;
    uint32_t ATTR_ACCESS_FIRMWARE : 3;
    uint32_t ATTR_ACCESS_IHD : 3;
    uint32_t ATTR_ACCESS_MS : 3;
  }st_Attribute_Access;

  //	enum
  //	{
#define          ATTR_NO_ACCESS  0ULL
#define          ATTR_READ_ONLY  1ULL
#define          ATTR_WRITE_ONLY 2ULL
#define          ATTR_READ_WRITE  3ULL
#define          ATTR_AUTHENTICATED_READ_ONLY  4ULL
#define          ATTR_AUTHENTICATED_WRITE_ONLY 5ULL
#define          ATTR_AUTHENTICATED_READ_WRITE 6ULL

#define ACCESS_PC___MR___US__        (                                             (0                   )                         )
#define ACCESS_PC___MR___USR_        (                                             (ATTR_READ_ONLY << 6 )                         )
#define ACCESS_PC___MRR__USR_        (                    (ATTR_READ_ONLY << 3 ) | (ATTR_READ_ONLY << 6 )                         )
#define ACCESS_PCR__MRR__USR_        ((ATTR_READ_ONLY ) | (ATTR_READ_ONLY << 3 ) | (ATTR_READ_ONLY << 6 )                         )
#define ACCESS_PC___MRR__USRW        (                    (ATTR_READ_ONLY << 3 ) | (ATTR_READ_WRITE << 6)                         )
#define ACCESS_PCR__MRR__USRW        ((ATTR_READ_ONLY ) | (ATTR_READ_ONLY << 3 ) | (ATTR_READ_WRITE << 6)                         )
#define ACCESS_PCR__MRRW_USRW        ((ATTR_READ_ONLY ) | (ATTR_READ_WRITE << 3) | (ATTR_READ_WRITE<< 6 )                         )
#define ACCESS_PC___MRRW_USRW        (                    (ATTR_READ_WRITE << 3) | (ATTR_READ_WRITE<< 6 )                         )
#define ACCESS_PCR__MR___US__        ((ATTR_READ_ONLY )                                                                           )
#define ACCESS_PC___MRR__US__        (                    (ATTR_READ_ONLY  << 3)                                                  )
#define ACCESS_PC___MR___USRW        (                                             (ATTR_READ_WRITE << 6)                         )
#define ACCESS_PC___MR___US_W        (                                             (ATTR_WRITE_ONLY << 6)                         )

#define ACCESS_PC___MRR__USR__PHR_   (                    (ATTR_READ_ONLY  << 3) | (ATTR_READ_ONLY  << 6) | (ATTR_READ_ONLY  << 9))
#define ACCESS_PCR__MRR__USR__PHR_   ((ATTR_READ_ONLY ) | (ATTR_READ_ONLY  << 3) | (ATTR_READ_ONLY  << 6) | (ATTR_READ_ONLY  << 9))
#define ACCESS_PC___MRR__USRW_PHRW   (                    (ATTR_READ_ONLY  << 3) | (ATTR_READ_WRITE << 6) | (ATTR_READ_WRITE << 9))
#define ACCESS_PC___MR___US_W_PH_W   (                                             (ATTR_WRITE_ONLY << 6) | (ATTR_WRITE_ONLY << 9))
#define ACCESS_PC___MR___US___PHR_   (                                                                      (ATTR_READ_ONLY  << 9))
#define ACCESS_PC___MR___US___PHRW   (                                                                      (ATTR_READ_WRITE << 9))
#define ACCESS_PCR__MRR__USR__PHR__FWR_   ((ATTR_READ_ONLY ) | (ATTR_READ_ONLY  << 3) | (ATTR_READ_ONLY  << 6) | (ATTR_READ_ONLY << 9)| (ATTR_READ_ONLY  << 12))
#define ACCESS_PCR__MRR__USRW_PHR__FWR_   ((ATTR_READ_ONLY ) | (ATTR_READ_ONLY  << 3) | (ATTR_READ_WRITE << 6) | (ATTR_READ_ONLY << 9)| (ATTR_READ_ONLY  << 12))

#define ACCESS_PC___MRR__USR__FWR_   (                    (ATTR_READ_ONLY  << 3) | (ATTR_READ_ONLY  << 6) | (ATTR_READ_ONLY  << 12))
#define ACCESS_PCR__MRR__USR__FWR_   ((ATTR_READ_ONLY ) | (ATTR_READ_ONLY  << 3) | (ATTR_READ_ONLY  << 6) | (ATTR_READ_ONLY  << 12))
#define ACCESS_PC___MR___USR__FWR_   (                                             (ATTR_READ_ONLY  << 6) | (ATTR_READ_ONLY  << 12))
#define ACCESS_PCR__MRR__USRW_FWR_   ((ATTR_READ_ONLY ) | (ATTR_READ_ONLY  << 3) | (ATTR_READ_WRITE << 6) | (ATTR_READ_ONLY  << 12))
#define ACCESS_PC___MRR__USR__FWRW   (                    (ATTR_READ_ONLY  << 3) | (ATTR_READ_ONLY  << 6) | (ATTR_READ_WRITE << 12))
#define ACCESS_PC___MRR__USRW_FWRW   (                    (ATTR_READ_ONLY  << 3) | (ATTR_READ_WRITE << 6) | (ATTR_READ_WRITE << 12))
#define ACCESS_PC___MR___US_W_FW_W   (                                             (ATTR_WRITE_ONLY << 6) | (ATTR_WRITE_ONLY << 12))
#define ACCESS_PC___MR___US___FWR_   (                                                                      (ATTR_READ_ONLY  << 12))
#define ACCESS_PC___MR___US___FWRW   (                                                                      (ATTR_READ_WRITE << 12))

#define ACCESS_PC___MRR__USR__IHR_   (                    (ATTR_READ_ONLY  << 3) | (ATTR_READ_ONLY  << 6) | (ATTR_READ_ONLY  << 15))
#define ACCESS_PC___MRR__USRW_IHRW   (                    (ATTR_READ_ONLY  << 3) | (ATTR_READ_WRITE << 6) | (ATTR_READ_WRITE << 15))
#define ACCESS_PC___MR___US_W_IH_W   (                                             (ATTR_WRITE_ONLY << 6) | (ATTR_WRITE_ONLY << 15))
#define ACCESS_PC___MR___US___IHR_   (                                                                      (ATTR_READ_ONLY  << 15))
#define ACCESS_PC___MR___US___IHRW   (                                                                      (ATTR_READ_WRITE << 15))

#define ASSOC_PC 0x01
#define ASSOC_MR 0x02
#define ASSOC_US 0x04
#define ASSOC_PUSH 0x08
#define ASSOC_FIRMWARE 0x10
#define ASSOC_IHD 0x20

#define ASSOC_PC_MR_US_FW_PH  ASSOC_PC+ASSOC_MR+ASSOC_US+ASSOC_FIRMWARE+ASSOC_PUSH
#define ASSOC_PC_MR_US_FW     ASSOC_PC+ASSOC_MR+ASSOC_US+ASSOC_FIRMWARE
#define ASSOC_PC_MR_US        ASSOC_PC+ASSOC_MR+ASSOC_US
#define ASSOC_MR_US           ASSOC_MR+ASSOC_US

#define ASSOC_PC_MR_US_PH     ASSOC_PC+ASSOC_MR+ASSOC_PUSH+ASSOC_US
#define ASSOC_MR_US_PH        ASSOC_MR+ASSOC_PUSH+ASSOC_US
#define ASSOC_MR_US_FW        ASSOC_MR+ASSOC_FIRMWARE+ASSOC_US
#define ASSOC_MR_US_IHD       ASSOC_MR+ASSOC_IHD+ASSOC_US

#define ASSOC_US_PH           ASSOC_US+ASSOC_PUSH
#define ASSOC_US_FW           ASSOC_US+ASSOC_FIRMWARE
#define ASSOC_US_IHD          ASSOC_US+ASSOC_IHD
#define ASSOC_US_PH_IHD       ASSOC_US+ASSOC_PUSH+ASSOC_IHD

//	};
  enum
  {
    UNICAST_KEY,
    BROADCAST_KEY,
    AUTHENTICATION_KEY,
    KEY_ENCRYPTION_KEY
  };
  enum
  {
    Auto_configuration,
    DHCPv6,
    Manual,
    Neighbour_Discovery
  };
  enum
  {
    MONDAY = 1,
    TUESDAY,
    WEDNESDAY,
    THURSDAY,
    FRIDAY,
    SATURDAY,
    SUNDAY
  };

  enum
  {
    JANUARY = 1,
    FEBRUARY,
    MARCH,
    APRIL,
    MAY,
    JUNE,
    JULY,
    AUGUST,
    SEPTEMBER,
    OCTOBER,
    NOVEMBER,
    DECEMBER
  };

  enum IM_TX_STAT
  {
    NotInitiated = 0,
    TransferInitiated,
    VerificationInitiated,
    VerificationSuccessful,
    VerificationFailed,
    ActivationInitiated,
    ActivationSuccessful,
    ActivationFailed
  };

  enum
  {
    /* DLMS PDUs (no encryption selected) */
    NONE_SECURITY = 0,
    ALL_MSGS_AUTHENTICATED = 1,
    ALL_MSGS_ENCRYPTED = 2,
    ALL_MSGS_AUTHENTICATED_ENCRYPTED = 3
  };
  enum GlobalKeyType
  {
    UnicastEncryption = 0,
    BroadcastEncryption,
    Authentication,
    Kek
  };
#define IMPLICIT_BASE   0x80
#define EXPLICIT_BASE   0xA0

  enum
  {
    AARQ_APDU_TAG_PROTOCOL_VERSION = (IMPLICIT_BASE + 0),
    AARQ_APDU_TAG_APPLICATION_CONTEXT_NAME = (EXPLICIT_BASE + 1),
    AARQ_APDU_TAG_CALLED_AP_TITLE = (EXPLICIT_BASE + 2),
    AARQ_APDU_TAG_CALLED_AE_QUALIFIER = (EXPLICIT_BASE + 3),
    AARQ_APDU_TAG_CALLED_AP_INVOCATION_ID = (EXPLICIT_BASE + 4),
    AARQ_APDU_TAG_CALLED_AE_INVOCATION_ID = (EXPLICIT_BASE + 5),
    AARQ_APDU_TAG_CALLING_AP_TITLE = (EXPLICIT_BASE + 6),
    AARQ_APDU_TAG_CALLING_AE_QUALIFIER = (EXPLICIT_BASE + 7),
    AARQ_APDU_TAG_CALLING_AP_INVOCATION_ID = (EXPLICIT_BASE + 8),
    AARQ_APDU_TAG_CALLING_AE_INVOCATION_ID = (EXPLICIT_BASE + 9),
    AARQ_APDU_TAG_SENDER_ACSE_REQUIREMENTS = (IMPLICIT_BASE + 10),
    AARQ_APDU_TAG_MECHANISM_NAME = (IMPLICIT_BASE + 11),
    AARQ_APDU_TAG_CALLING_AUTHENTICATION_VALUE = (EXPLICIT_BASE + 12),
    AARQ_APDU_TAG_IMPLEMENTATION_INFORMATION = (EXPLICIT_BASE + 29),
    AARQ_APDU_TAG_USER_INFORMATION = (EXPLICIT_BASE + 30)
  };

  enum
  {
    AARE_APDU_TAG_PROTOCOL_VERSION = (IMPLICIT_BASE + 0),
    AARE_APDU_TAG_APPLICATION_CONTEXT_NAME = (EXPLICIT_BASE + 1),
    AARE_APDU_TAG_RESULT = (EXPLICIT_BASE + 2),
    AARE_APDU_TAG_RESULT_SOURCE_DIAGNOSTIC = (EXPLICIT_BASE + 3),
    AARE_APDU_TAG_RESPONDING_AP_TITLE = (EXPLICIT_BASE + 4),
    AARE_APDU_TAG_RESPONDING_AP_QUALIFIER = (EXPLICIT_BASE + 5),
    AARE_APDU_TAG_RESPONDING_AP_INVOCATION_ID = (EXPLICIT_BASE + 6),
    AARE_APDU_TAG_RESPONDING_AE_INVOCATION_ID = (EXPLICIT_BASE + 7),
    AARE_APDU_TAG_RESPONDER_ACSE_REQUIREMENTS = (IMPLICIT_BASE + 8),
    AARE_APDU_TAG_MECHANISM_NAME = (IMPLICIT_BASE + 9),
    AARE_APDU_TAG_RESPONDING_AUTHENTICATION_VALUE = (EXPLICIT_BASE + 10),
    AARE_APDU_TAG_IMPLEMENTATION_INFORMATION = (EXPLICIT_BASE + 29),
    AARE_APDU_TAG_USER_INFORMATION = (EXPLICIT_BASE + 30)
  };

  enum
  {
    TAG_AUTHENTICATION_INFORMATION_COMPONENT = (0x80 + 0),
    TAG_RESPONDER_ACSE_REQUIREMENTS_FIELD_COMPONENT = (0x80 + 8),
    TAG_RESPONDER_MECHANISM_NAME_COMPONENT = (0x80 + 9),
    TAG_ACSE_REQUIREMENTS_FIELD_COMPONENT = (0x80 + 10),
    TAG_MECHANISM_NAME_COMPONENT = (0x80 + 11),
  };

  /*
  ASN.1 BER tags break down according to bits 7 and 6
  Universal               0x00 - 0x3F
  Application specific    0x40 - 0x7F
  Context specific        0x80 - 0xBF
  Private                 0xC0 - 0xFF
  */
  enum
  {
    /* P = primitive, C = constructed */
    TAG_UNIVERSAL_EOC = 0,                  /* P (End-of-Content) */
    TAG_UNIVERSAL_BOOLEAN = 1,              /* P */
    TAG_UNIVERSAL_INTEGER = 2,              /* P */
    TAG_UNIVERSAL_BIT_STRING = 3,           /* P/C */
    TAG_UNIVERSAL_OCTET_STRING = 4,         /* P/C */
    TAG_UNIVERSAL_NULL = 5,                 /* P */
    TAG_UNIVERSAL_OBJECT_IDENTIFIER = 6,    /* P */
    TAG_UNIVERSAL_OBJECT_DESCRIPTOR = 7,    /* P */
    TAG_UNIVERSAL_EXTERNAL = 8,             /* C */
    TAG_UNIVERSAL_REAL = 9,                 /* P */
    TAG_UNIVERSAL_ENUMERATED = 10,          /* P */
    TAG_UNIVERSAL_EMBEDDED_PDV = 11,        /* C */
    TAG_UNIVERSAL_UTF8STRING = 12,          /* P/C */
    TAG_UNIVERSAL_RELATIVE_OID = 13,        /* P */
    TAG_UNIVERSAL_SEQUENCE = 16,            /* C (also TAG_UNIVERSAL_SEQUENCE_OF) */
    TAG_UNIVERSAL_SET = 17,                 /* C (also TAG_UNIVERSAL_SET_OF) */
    TAG_UNIVERSAL_NUMERIC_STRING = 18,      /* P/C */
    TAG_UNIVERSAL_PRINTABLE_STRING = 19,    /* P/C */
    TAG_UNIVERSAL_T61_STRING = 20,          /* P/C */
    TAG_UNIVERSAL_VIDEOTEX_STRING = 21,     /* P/C */
    TAG_UNIVERSAL_IA5_STRING = 22,          /* P/C */
    TAG_UNIVERSAL_UTC_TIME = 23,            /* P/C */
    TAG_UNIVERSAL_GENERALIZED_TIME = 24,    /* P/C */
    TAG_UNIVERSAL_GRAPHIC_STRING = 25,      /* P/C */
    TAG_UNIVERSAL_VISIBLE_STRING = 26,      /* P/C */
    TAG_UNIVERSAL_GENERAL_STRING = 27,      /* P/C */
    TAG_UNIVERSAL_UNIVERSAL_STRING = 28,    /* P/C */
    TAG_UNIVERSAL_CHARACTER_STRING = 29,    /* P/C */
    TAG_UNIVERSAL_BMPSTRING = 30            /* P/C */
  };

  /* Conformance block (60256-53/8.5 and 62056-53/Annex C)
  Reserved0                    byte 9 bit 7
  Reserved1                    byte 9 bit 6
  Reserved2                    byte 9 bit 5
  Read                         byte 9 bit 4
  Write                        byte 9 bit 3
  UnconfirmedWrite             byte 9 bit 2
  Reserved3                    byte 9 bit 1
  Reserved4                    byte 9 bit 0

  Attribute0SupportedWithSet   byte 10 bit 7
  PriorityMgmtSupported        byte 10 bit 6
  Attribute0SupportedWithGet   byte 10 bit 5
  BlockTransferWithGet         byte 10 bit 4
  BlockTransferWithSet         byte 10 bit 3
  BlockTransferWithAction      byte 10 bit 2
  MultipleReferences           byte 10 bit 1
  InformationReport            byte 10 bit 0

  Reserved5                    byte 11 bit 7
  Reserved6                    byte 11 bit 6
  ParameterizedAccess          byte 11 bit 5
  Get                          byte 11 bit 4
  Set                          byte 11 bit 3
  SelectiveAccess              byte 11 bit 2
  EventNotification            byte 11 bit 1
  Action                       byte 11 bit 0
  */

  enum
  {
    ACCESS_RESULT_SUCCESS = 0,
    ACCESS_RESULT_HARDWARE_FAULT = 1,
    ACCESS_RESULT_TEMPORARY_FAILURE = 2,
    ACCESS_RESULT_READ_WRITE_DENIED = 3,
    ACCESS_RESULT_OBJECT_UNDEFINED = 4,
    ACCESS_RESULT_OBJECT_CLASS_INCONSISTENT = 9,
    ACCESS_RESULT_OBJECT_UNAVAILABLE = 11,
    ACCESS_RESULT_TYPE_UNMATCHED = 12,
    ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED = 13,
    ACCESS_RESULT_DATA_BLOCK_UNAVAILABLE = 14,
    ACCESS_RESULT_LONG_GET_ABORTED = 15,
    ACCESS_RESULT_NO_LONG_GET_IN_PROGRESS = 16,
    ACCESS_RESULT_LONG_SET_ABORTED = 17,
    ACCESS_RESULT_NO_LONG_SET_IN_PROGRESS = 18,
    ACCESS_RESULT_OTHER_REASON = 250
  };

  enum
  {
    ACTION_RESULT_SUCCESS = 0,
    ACTION_RESULT_HARDWARE_FAULT = 1,
    ACTION_RESULT_TEMPORARY_FAILURE = 2,
    ACTION_RESULT_READ_WRITE_DENIED = 3,
    ACTION_RESULT_OBJECT_UNDEFINED = 4,
    ACTION_RESULT_OBJECT_CLASS_INCONSISTENT = 9,
    ACTION_RESULT_OBJECT_UNAVAILABLE = 11,
    ACTION_RESULT_TYPE_UNMATCHED = 12,
    ACTION_RESULT_SCOPE_OF_ACCESS_VIOLATED = 13,
    ACTION_RESULT_DATA_BLOCK_UNAVAILABLE = 14,
    ACTION_RESULT_LONG_ACTION_ABORTED = 15,
    ACTION_RESULT_NO_LONG_ACTION_IN_PROGRESS = 16,
    ACTION_RESULT_OTHER_REASON = 250
  };

  enum
  {
    CONFIRMED_SERVICE_ERROR_INITIATEERROR = 1,
    CONFIRMED_SERVICE_ERROR_GETSTATUS = 2,
    CONFIRMED_SERVICE_ERROR_GETNAMELIST = 3,
    CONFIRMED_SERVICE_ERROR_GETVARIABLEATTRIBUTE = 4,
    CONFIRMED_SERVICE_ERROR_READ = 5,
    CONFIRMED_SERVICE_ERROR_WRITE = 6,
    CONFIRMED_SERVICE_ERROR_GETDATASETATTRIBUTE = 7,
    CONFIRMED_SERVICE_ERROR_GETTIATTRIBUTE = 8,
    CONFIRMED_SERVICE_ERROR_CHANGESCOPE = 9,
    CONFIRMED_SERVICE_ERROR_START = 10,
    CONFIRMED_SERVICE_ERROR_STOP = 11,
    CONFIRMED_SERVICE_ERROR_RESUME = 12,
    CONFIRMED_SERVICE_ERROR_MAKEUSABLE = 13,
    CONFIRMED_SERVICE_ERROR_INITIATELOAD = 14,
    CONFIRMED_SERVICE_ERROR_LOADSEGMENT = 15,
    CONFIRMED_SERVICE_ERROR_TERMINATELOAD = 16,
    CONFIRMED_SERVICE_ERROR_INITIATEUPLOAD = 17,
    CONFIRMED_SERVICE_ERROR_UPLOADSEGMENT = 18,
    CONFIRMED_SERVICE_ERROR_TERMINATEUPLOAD = 19
  };

  enum
  {
    SERVICE_ERROR_APPLICATION_REFERENCE = 0,
    SERVICE_ERROR_HARDWARE_RESOURCE = 1,
    SERVICE_ERROR_VDE_STATE_ERROR = 2,
    SERVICE_ERROR_SERVICE = 3,
    SERVICE_ERROR_DEFINITION = 4,
    SERVICE_ERROR_ACCESS = 5,
    SERVICE_ERROR_INITIATE = 6,
    SERVICE_ERROR_LOAD_DATA_SET = 7,
    SERVICE_ERROR_TASK = 9,
    SERVICE_ERROR_OTHER_ERROR = 10
  };

  ////////////////////obis//////////////////
  typedef enum
  {
    CLASS_ID_DATA = 1,
    CLASS_ID_REGISTER = 3,
    CLASS_ID_EXTENDED_REGISTER = 4,
    CLASS_ID_DEMAND_REGISTER = 5,
    CLASS_ID_REGISTER_ACTIVATION = 6,
    CLASS_ID_PROFILE_GENERIC = 7,
    CLASS_ID_CLOCK = 8,
    CLASS_ID_SCRIPT_TABLE = 9,
    CLASS_ID_SCHEDULE = 10,
    CLASS_ID_SPECIAL_DAYS = 11,
    CLASS_ID_ASSOCIATION_SN = 12,
    CLASS_ID_ASSOCIATION_LN = 15,
    CLASS_ID_SAP_ASSIGNMENT = 17,
    CLASS_ID_IMAGE_TRANSFER = 18,
    CLASS_ID_IEC_LOCAL_PORT_SETUP = 19,
    CLASS_ID_ACTIVITY_CALENDAR = 20,
    CLASS_ID_REGISTER_MONITOR = 21,
    CLASS_ID_SINGLE_ACTION_SCHEDULE = 22,
    CLASS_ID_IEC_HDLC_SETUP = 23,
    CLASS_ID_IEC_TWISTED_PAIR_SETUP = 24,
    CLASS_ID_MBUS_PORT_SETUP = 25,
    CLASS_ID_UTILITY_TABLES = 26,
    CLASS_ID_PSTN_MODEM_CONFIG = 27,
    CLASS_ID_PSTN_AUTO_ANSWER = 28,
    CLASS_ID_PSTN_AUTO_DIAL = 29,
    CLASS_ID_PUSH_SETUP = 40,
    CLASS_ID_TCP_UDP_SETUP = 41,
    CLASS_ID_IPV4_SETUP = 42,
    CLASS_ID_ETHERNET_SETUP = 43,
    CLASS_ID_PPP_SETUP = 44,
    CLASS_ID_GPRS_MODEM_SETUP = 45,
    CLASS_ID_SMTP_SETUP = 46,
    CLASS_ID_IPV6_SETUP = 48,
    CLASS_ID_SFSK_PHY_MAC_SETUP = 50,
    CLASS_ID_SFSK_ACTIVE_INITIATOR = 51,
    CLASS_ID_SFSK_MAC_SYNC_TIMEOUTS = 52,
    CLASS_ID_SFSK_MAC_COUNTERS = 53,
    CLASS_ID_SFSK_IEC61334_4_32_LLC_SETUP = 55,
    CLASS_ID_SFSK_REPORTING_SYSTEM_LIST = 56,
    CLASS_ID_IEC8802_2_TYPE_1_SETUP = 57,
    CLASS_ID_IEC8802_2_TYPE_2_SETUP = 58,
    CLASS_ID_IEC8802_2_TYPE_3_SETUP = 59,
    CLASS_ID_REGISTER_TABLE = 61,
    CLASS_ID_STATUS_MAPPING = 63,
    CLASS_ID_SECURITY_SETUP = 64,
    CLASS_ID_DISCONNECT_CONTROL = 70,
    CLASS_ID_LIMITER = 71,
    CLASS_ID_MBUS_CLIENT = 72
  } class_id_t;

  /* IEC62056-62 5.1 */
  typedef enum
  {
    CLASS_DATA_LOGICAL_NAME = 1,
    CLASS_DATA_VALUE = 2
  } data_attributes_t;

  /* IEC62056-62 5.2 */
  typedef enum
  {
    CLASS_REGISTER_LOGICAL_NAME = 1,
    CLASS_REGISTER_VALUE = 2,
    CLASS_REGISTER_SCALER_UNIT = 3
  } register_attributes_t;

  /* IEC62056-62 5.3 */
  typedef enum
  {
    CLASS_EXTENDED_REGISTER_LOGICAL_NAME = 1,
    CLASS_EXTENDED_REGISTER_VALUE = 2,
    CLASS_EXTENDED_REGISTER_SCALER_UNIT = 3,
    CLASS_EXTENDED_REGISTER_STATUS = 4,
    CLASS_EXTENDED_REGISTER_CAPTURE_TIME = 5
  } extended_register_attributes_t;

  /* IEC62056-62 5.4 */
  typedef enum
  {
    CLASS_DEMAND_REGISTER_LOGICAL_NAME = 1,
    CLASS_DEMAND_REGISTER_CURRENT_AVERAGE_VALUE = 2,
    CLASS_DEMAND_REGISTER_LAST_AVERAGE_VALUE = 3,
    CLASS_DEMAND_REGISTER_SCALER_UNIT = 4,
    CLASS_DEMAND_REGISTER_STATUS = 5,
    CLASS_DEMAND_REGISTER_CAPTURE_TIME = 6,
    CLASS_DEMAND_REGISTER_START_TIME_CURRENT = 7,
    CLASS_DEMAND_REGISTER_PERIOD = 8,
    CLASS_DEMAND_REGISTER_NUMBER_OF_PERIODS = 9
  } demand_register_attributes_t;

  /* IEC62056-62 5.5 */
  typedef enum
  {
    CLASS_REGISTER_ACTIVATION_LOGICAL_NAME = 1,
    CLASS_REGISTER_ACTIVATION_REGISTER_ASSIGNMENT = 2,
    CLASS_REGISTER_ACTIVATION_MASK_LIST = 3,
    CLASS_REGISTER_ACTIVATION_ACTIVE_MASK = 4
  } register_activation_attributes_t;

  /* IEC62056-62 5.6 */
  typedef enum
  {
    CLASS_PROFILE_GENERIC_LOGICAL_NAME = 1,
    CLASS_PROFILE_GENERIC_BUFFER = 2,
    CLASS_PROFILE_GENERIC_CAPTURE_OBJECTS = 3,
    CLASS_PROFILE_GENERIC_CAPTURE_PERIOD = 4,
    CLASS_PROFILE_GENERIC_SORT_METHOD = 5,
    CLASS_PROFILE_GENERIC_SORT_OBJECT = 6,
    CLASS_PROFILE_GENERIC_ENTRIES_IN_USE = 7,
    CLASS_PROFILE_GENERIC_PROFILE_ENTRIES = 8
  } profile_generic_attributes_t;

  /* IEC62056-62 5.7 */
  typedef enum
  {
    CLASS_CLOCK_LOGICAL_NAME = 1,
    CLASS_CLOCK_TIME = 2,
    CLASS_CLOCK_TIME_ZONE = 3,
    CLASS_CLOCK_STATUS = 4,
    CLASS_CLOCK_DAYLIGHT_SAVINGS_BEGIN = 5,
    CLASS_CLOCK_DAYLIGHT_SAVINGS_END = 6,
    CLASS_CLOCK_DAYLIGHT_SAVINGS_DEVIATION = 7,
    CLASS_CLOCK_DAYLIGHT_SAVINGS_ENABLED = 8,
    CLASS_CLOCK_BASE = 9
  } clock_attributes_t;

  /* IEC62056-62 5.8 */
  typedef enum
  {
    CLASS_SCRIPT_TABLE_LOGICAL_NAME = 1,
    CLASS_SCRIPT_TABLE_SCRIPTS = 2
  } script_table_attributes_t;

  /* IEC62056-62 5.9 */
  typedef enum
  {
    CLASS_SCHEDULE_LOGICAL_NAME = 1,
    CLASS_SCHEDULE_ENTRIES = 2
  } schedule_attributes_t;

  /* IEC62056-62 5.10 */
  typedef enum
  {
    CLASS_SPECIAL_DAYS_LOGICAL_NAME = 1,
    CLASS_SPECIAL_DAYS_ENTRIES = 2
  } special_days_attributes_t;

  /* IEC62056-62 5.11 */
  typedef enum
  {
    CLASS_ACTIVITY_CALENDAR_LOGICAL_NAME = 1,
    CLASS_ACTIVITY_CALENDAR_CALENDAR_NAME_ACTIVE = 2,
    CLASS_ACTIVITY_CALENDAR_SEASON_PROFILE_ACTIVE = 3,
    CLASS_ACTIVITY_CALENDAR_WEEK_PROFILE_TABLE_ACTIVE = 4,
    CLASS_ACTIVITY_CALENDAR_DAY_PROFILE_TABLE_ACTIVE = 5,
    CLASS_ACTIVITY_CALENDAR_CALENDAR_NAME_PASSIVE = 6,
    CLASS_ACTIVITY_CALENDAR_SEASON_PROFILE_PASSIVE = 7,
    CLASS_ACTIVITY_CALENDAR_WEEK_PROFILE_TABLE_PASSIVE = 8,
    CLASS_ACTIVITY_CALENDAR_DAY_PROFILE_TABLE_PASSIVE = 9,
    CLASS_ACTIVITY_CALENDAR_ACTIVE_PASSIVE_CALENDAR_TIME = 10
  } activity_calendar_attributes_t;

  /* IEC62056-62 5.12 */
  typedef enum
  {
    CLASS_ASSOCIATION_LN_LOGICAL_NAME = 1,
    CLASS_ASSOCIATION_LN_OBJECT_LIST = 2,
    CLASS_ASSOCIATION_LN_ASSOCIATED_PARTNERS_ID = 3,
    CLASS_ASSOCIATION_LN_APPLICATION_CONTEXT_NAME = 4,
    CLASS_ASSOCIATION_LN_XDLMS_CONTEXT_INFO = 5,
    CLASS_ASSOCIATION_LN_AUTHENTICATION_MECHANISM_NAME = 6,
    CLASS_ASSOCIATION_LN_LLS_SECRET = 7,
    CLASS_ASSOCIATION_LN_ASSOCIATION_STATUS = 8,
    CLASS_ASSOCIATION_LN_SECURITY_SETUP_REFERENCE = 9
  } association_ln_attributes_t;

  typedef enum
  {
    CLASS_ASSOCIATION_LN_REPLY_TO_HLS_AUTHENTICATION = 1,
    CLASS_ASSOCIATION_LN_CHANGE_HLS_SECRET = 2,
    CLASS_ASSOCIATION_LN_ADD_OBJECT = 3,
    CLASS_ASSOCIATION_LN_REMOVE_OBJECT = 4
  } association_ln_methods_t;

  /* IEC62056-62 5.13 */
  typedef enum
  {
    CLASS_ASSOCIATION_SN_LOGICAL_NAME = 1,
    CLASS_ASSOCIATION_SN_OBJECT_LIST = 2
  } association_sn_attributes_t;

  /* IEC62056-62 5.14 */
  typedef enum
  {
    CLASS_SAP_ASSIGNMENT_LOGICAL_NAME = 1,
    CLASS_SAP_ASSIGNMENT_LIST = 2
  } sap_assignment_attributes_t;

  /* IEC62056-62 5.15 */
  typedef enum
  {
    CLASS_REGISTER_MONITOR_LOGICAL_NAME = 1,
    CLASS_REGISTER_MONITOR_THRESHOLDS = 2,
    CLASS_REGISTER_MONITOR_MONITORED_VALUE = 3,
    CLASS_REGISTER_MONITOR_ACTIONS = 4
  } register_monitor_attributes_t;

  /* IEC62056-62 5.16 */
  typedef enum
  {
    CLASS_UTILITY_TABLES_LOGICAL_NAME = 1,
    CLASS_UTILITY_TABLES_TABLE_ID = 2,
    CLASS_UTILITY_TABLES_LENGTH = 3,
    CLASS_UTILITY_TABLES_BUFFER = 4
  } utility_tables_attributes_t;

  /* IEC62056-62 5.17 */
  typedef enum
  {
    CLASS_SINGLE_ACTION_SCHEDULE_LOGICAL_NAME = 1,
    CLASS_SINGLE_ACTION_SCHEDULE_EXECUTED_SCRIPT = 2,
    CLASS_SINGLE_ACTION_SCHEDULE_TYPE = 3,
    CLASS_SINGLE_ACTION_SCHEDULE_EXECUTION_TIME = 4
  } single_action_schedule_attributes_t;

  /* IEC62056-62 A.1 */
  typedef enum
  {
    CLASS_IEC_LOCAL_PORT_SETUP_LOGICAL_NAME = 1,
    CLASS_IEC_LOCAL_PORT_SETUP_DEFAULT_MODE = 2,
    CLASS_IEC_LOCAL_PORT_SETUP_DEFAULT_BAUD = 3,
    CLASS_IEC_LOCAL_PORT_SETUP_PROP_BAUD = 4,
    CLASS_IEC_LOCAL_PORT_SETUP_RESPONSE_TIME = 5,
    CLASS_IEC_LOCAL_PORT_SETUP_DEVICE_ADDR = 6,
    CLASS_IEC_LOCAL_PORT_SETUP_PASS_P1 = 7,
    CLASS_IEC_LOCAL_PORT_SETUP_PASS_P2 = 8,
    CLASS_IEC_LOCAL_PORT_SETUP_PASS_P3 = 9
  } iec_local_port_setup_attributes_t;

  /* IEC62056-62 A.2 */
  typedef enum
  {
    CLASS_PSTN_MODEM_CONFIG_LOGICAL_NAME = 1,
    CLASS_PSTN_MODEM_CONFIG_COMM_SPEED = 2,
    CLASS_PSTN_MODEM_CONFIG_INITIALIZATION_STRING = 3,
    CLASS_PSTN_MODEM_CONFIG_MODEM_PROFILE = 4
  } pstn_modem_config_attributes_t;

  /* IEC62056-62 A.3 */
  typedef enum
  {
    CLASS_PSTN_AUTO_ANSWER_LOGICAL_NAME = 1,
    CLASS_PSTN_AUTO_ANSWER_MODE = 2,
    CLASS_PSTN_AUTO_ANSWER_LISTENING_WINDOW = 3,
    CLASS_PSTN_AUTO_ANSWER_STATUS = 4,
    CLASS_PSTN_AUTO_ANSWER_NUMBER_OF_CALLS = 5,
    CLASS_PSTN_AUTO_ANSWER_NUMBER_OF_RINGS = 6
  } pstn_auto_answer_attributes_t;

  /* IEC62056-62 A.4 */
  typedef enum
  {
    CLASS_PSTN_AUTO_DIAL_LOGICAL_NAME = 1,
    CLASS_PSTN_AUTO_DIAL_MODE = 2,
    CLASS_PSTN_AUTO_DIAL_REPITIIONS = 3,
    CLASS_PSTN_AUTO_DIAL_REPITITION_DELAY = 4,
    CLASS_PSTN_AUTO_DIAL_CALLING_WINDOW = 5,
    CLASS_PSTN_AUTO_DIAL_PHONE_LIST = 6
  } pstn_auto_dial_attributes_t;

  /* IEC62056-62 A.5 */
  typedef enum
  {
    CLASS_IEC_HDLC_SETUP_LOGICAL_NAME = 1,
    CLASS_IEC_HDLC_SETUP_COMM_SPEED = 2,
    CLASS_IEC_HDLC_SETUP_WINDOW_SIZE_TRANSMIT = 3,
    CLASS_IEC_HDLC_SETUP_WINDOW_SIZE_RECEIVE = 4,
    CLASS_IEC_HDLC_SETUP_MAX_INFO_FIELD_LENGTH_TRANSMIT = 5,
    CLASS_IEC_HDLC_SETUP_MAX_INFO_FIELD_LENGTH_RECEIVE = 6,
    CLASS_IEC_HDLC_SETUP_INTER_OCTET_TIMEOUT = 7,
    CLASS_IEC_HDLC_SETUP_INACTIVITY_TIMEOUT = 8,
    CLASS_IEC_HDLC_SETUP_DEVICE_ADDRESS = 9
  } iec_hdlc_setup_attributes_t;

  /* IEC62056-62 A.6 */
  typedef enum
  {
    CLASS_IEC_TWISTED_PAIR_SETUP_LOGICAL_NAME = 1,
    CLASS_IEC_TWISTED_PAIR_SETUP_SECONDARY_ADDRESS = 2,
    CLASS_IEC_TWISTED_PAIR_SETUP_PRIMARY_ADDRESS_LIST = 3,
    CLASS_IEC_TWISTED_PAIR_SETUP_TABI_LIST = 4,
    CLASS_IEC_TWISTED_PAIR_SETUP_FATAL_ERROR = 5
  } iec_twisted_pair_setup_attributes_t;

  typedef enum
  {
    CLASS_SECURITY_SETUP_LOGICAL_NAME = 1,
    CLASS_SECURITY_SETUP_SECURITY_POLICY = 2,
    CLASS_SECURITY_SETUP_SECURITY_SUITE = 3,
    CLASS_SECURITY_SETUP_CLIENT_SYSTEM_TITLE = 4,
    CLASS_SECURITY_SETUP_SERVER_SYSTEM_TITLE = 5
  } security_setup_attributes_t;

  typedef enum
  {
    CLASS_SECURITY_SETUP_SECURITY_ACTIVATE = 1,
    CLASS_SECURITY_SETUP_GLOBAL_KEY_TRANSFER = 2
  } security_setup_methods_t;

  typedef enum
  {
    OBIS_UNIT_TIME_YEAR = 1,
    OBIS_UNIT_TIME_MONTH = 2,
    OBIS_UNIT_TIME_WEEK = 3,
    OBIS_UNIT_TIME_DAY = 4,
    OBIS_UNIT_TIME_HOUR = 5,
    OBIS_UNIT_TIME_MINUTE = 6,
    OBIS_UNIT_TIME_SECOND = 7,
    OBIS_UNIT_ANGLE_DEGREE = 8,
    OBIS_UNIT_TEMPERATURE_DEGREE_CENTIGRADE = 9,
    OBIS_UNIT_CURRENCY_LOCAL = 10,
    OBIS_UNIT_LENGTH_METRE = 11,
    OBIS_UNIT_SPEED_METRE_PER_SECOND = 12,
    OBIS_UNIT_VOLUME_CUBIC_METRE = 13,
    OBIS_UNIT_CORRECTED_VOLUME_CUBIC_METRE = 14,
    OBIS_UNIT_VOLUME_FLUX_CUBIC_METRE_PER_HOUR = 15,
    OBIS_UNIT_CORRECTED_VOLUME_FLUX_CUBIC_METRE_PER_HOUR = 16,
    OBIS_UNIT_VOLUME_FLUX_CUBIC_METRE_PER_DAY = 17,
    OBIS_UNIT_CORRECTED_VOLUME_FLUX_CUBIC_METRE_PER_DAY = 18,
    OBIS_UNIT_VOLUME_MILLI_CUBIC_METRE = 19,
    OBIS_UNIT_MASS_KILOGRAM = 20,
    OBIS_UNIT_FORCE_NEWTON = 21,
    OBIS_UNIT_ENERGY_NEWTON_METRE = 22,
    OBIS_UNIT_PRESSURE_PASCAL = 23,
    OBIS_UNIT_PRESSURE_BAR = 24,
    OBIS_UNIT_ENERGY_JOULE = 25,
    OBIS_UNIT_THERMAL_POWER_JOULE_PER_HOUR = 26,
    OBIS_UNIT_ACTIVE_POWER_WATT = 27,
    OBIS_UNIT_APPARENT_POWER_VA = 28,
    OBIS_UNIT_REACTIVE_POWER_VAR = 29,
    OBIS_UNIT_ACTIVE_ENERGY_WATT_HOUR = 30,
    OBIS_UNIT_APPARENT_ENERGY_VA_HOUR = 31,
    OBIS_UNIT_REACTIVE_ENERGY_VAR_HOUR = 32,
    OBIS_UNIT_CURRENT_AMPERE = 33,
    OBIS_UNIT_ELECTRICAL_CHARGE_COULOMB = 34,
    OBIS_UNIT_VOLTAGE_VOLT = 35,
    OBIS_UNIT_ELECTRICAL_FIELD_STRENGTH_VOLT_PER_METRE = 36,
    OBIS_UNIT_CAPACITANCE_FARAD = 37,
    OBIS_UNIT_RESISTANCE_OHM = 38,
    OBIS_UNIT_RESISTIVITY_OHM_METRE = 39,
    OBIS_UNIT_MAGNETIC_FLUX_WEBER = 40,
    OBIS_UNIT_INDUCTION_TESLA = 41,
    OBIS_UNIT_MAGNETIC_FIELD_STRENGTH_AMPERE_PER_METRE = 42,
    OBIS_UNIT_INDUCTANCE_HENRY = 43,
    OBIS_UNIT_FREQUENCY_HERTZ = 44,
    OBIS_UNIT_ACTIVE_ENERGY_METER_CONSTANT = 45,
    OBIS_UNIT_REACTIVE_ENERGY_METER_CONSTANT = 46,
    OBIS_UNIT_APPARENT_ENERGY_METER_CONSTANT = 47,
    OBIS_UNIT_VOLTAGE_SQUARE_HOUR = 48,
    OBIS_UNIT_AMPERE_SQUARE_HOUR = 49,
    OBIS_UNIT_MASS_FLUX_KILOGRAM_PER_SECOND = 50,
    OBIS_UNIT_CONDUCTANCE_SIEMENS = 51,
    OBIS_UNIT_UNITLESS_COUNT = 255
  } unit_t;

  enum
  {
    // Instantaneous Profile Item Tags
    ITEM_TAG_CURRENT_DATETIME = 1,
    #ifdef SINGLE_PHASE_METER
    ITEM_TAG_PHASE_CURRENT,
    ITEM_TAG_NEUTRAL_CURRENT,
    ITEM_TAG_VRMS,
    ITEM_TAG_PF,
    #else
    ITEM_TAG_IR,
    ITEM_TAG_IY,
    ITEM_TAG_IB,
    ITEM_TAG_VR,
    ITEM_TAG_VY,
    ITEM_TAG_VB,
    ITEM_TAG_PFR,
    ITEM_TAG_PFY,
    ITEM_TAG_PFB,
    ITEM_TAG_PF_TOTAL,
    #endif
    ITEM_TAG_FREQUENCY,
    ITEM_TAG_KVA_TOTAL,
    ITEM_TAG_KW_TOTAL,
    ITEM_TAG_KVAR_TOTAL,
    ITEM_TAG_CUM_KWH_TOTAL,
    ITEM_TAG_CUM_KVAR_Q1_TOTAL,
    ITEM_TAG_CUM_KVAR_Q2_TOTAL,
    ITEM_TAG_CUM_KVAH_TOTAL,
    ITEM_TAG_CUM_KWH_EXPO_TOTAL,
    ITEM_TAG_CUM_KVAR_Q3_TOTAL,
    ITEM_TAG_CUM_KVAR_Q4_TOTAL,
    ITEM_TAG_CUM_KVAH_EXPO_TOTAL,
    ITEM_TAG_NUM_POWER_OFFS,
    ITEM_TAG_CUM_POWER_OFF_DURATION,
    ITEM_TAG_CUM_POWER_ON_DURATION,
    ITEM_TAG_DEVICE_TEMPERATURE,
    ITEM_TAG_CUM_TAMPER_COUNT,
    ITEM_TAG_CUM_MD_RESET_COUNT,
    ITEM_TAG_CUM_PROGRAMMING_COUNT,
    ITEM_TAG_CUM_LAST_MD_EVENT_DATETIME,
    ITEM_TAG_KW_MAX_DEMAND,
    ITEM_TAG_CUM_LAST_MD_KW_DATETIME,
    ITEM_TAG_KVA_MAX_DEMAND,
    ITEM_TAG_CUM_LAST_MD_KVA_DATETIME,
    ITEM_TAG_BILL_DATE,
    ITEM_TAG_LOAD_STATUS,
    ITEM_TAG_LOAD_LIMIT_VAL,
    ITEM_TAG_CUMMULATIVE_TAMPER_VAL_TMPR,
    ITEM_TAG_CUMMULATIVE_KWH_EXPO_VAL_TMPR,
    // Load Profile item Tags
    ITEM_TAG_DATETIME_LP,
    #ifdef SINGLE_PHASE_METER
    ITEM_TAG_I_PHASE_LP,
    ITEM_TAG_I_NEUTRAL_LP,
    ITEM_TAG_V_LP,
    ITEM_TAG_PF_LP,
    #else
    ITEM_TAG_IR_LP,
    ITEM_TAG_IY_LP,
    ITEM_TAG_IB_LP,
    ITEM_TAG_VR_LP,
    ITEM_TAG_VY_LP,
    ITEM_TAG_VB_LP,
    ITEM_TAG_PFR_LP,
    ITEM_TAG_PFY_LP,
    ITEM_TAG_PFB_LP,
    ITEM_TAG_PAVG_LP,
    #endif
    ITEM_TAG_CUM_KWH_TOTAL_IMPORT_LP,
    ITEM_TAG_CUM_KWH_TOTAL_EXPORT_LP,
    ITEM_TAG_CUM_KVAR_Q1_TOTAL_LP,
    ITEM_TAG_CUM_KVAR_Q2_TOTAL_LP,
    ITEM_TAG_CUM_KVAR_Q3_TOTAL_LP,
    ITEM_TAG_CUM_KVAR_Q4_TOTAL_LP,
    ITEM_TAG_CUM_KVAH_TOTAL_IMPORT_LP,
    ITEM_TAG_CUM_KVAH_TOTAL_EXPORT_LP,
    ITEM_TAG_THD_IR_LP,
    ITEM_TAG_THD_IY_LP,
    ITEM_TAG_THD_IB_LP,
    ITEM_TAG_THD_VR_LP,
    ITEM_TAG_THD_VY_LP,
    ITEM_TAG_THD_VB_LP,

    //Daily Load Profile
    ITEM_TAG_DATETIME_DL_LP,
    ITEM_TAG_CUM_KWH_TOTAL_DL_LP,
    ITEM_TAG_CUM_KVAH_TOTAL_DL_LP,
    ITEM_TAG_CUM_KWH_EXPO_TOTAL_DL_LP,
    ITEM_TAG_CUM_KVAH_EXPO_TOTAL_DL_LP,
    ITEM_TAG_MD_KW_DL_LP,
    ITEM_TAG_MD_KVA_DL_LP,

    // Billing Profile item TAGS
    ITEM_TAG_BILLING_DATETIME_BI,
    ITEM_TAG_SPF_BI,
    ITEM_TAG_KWH_BI,
    ITEM_TAG_KWH_TZ1_BI,
    ITEM_TAG_KWH_TZ2_BI,
    ITEM_TAG_KWH_TZ3_BI,
    ITEM_TAG_KWH_TZ4_BI,
    ITEM_TAG_KWH_TZ5_BI,
    ITEM_TAG_KWH_TZ6_BI,
    ITEM_TAG_KWH_TZ7_BI,
    ITEM_TAG_KWH_TZ8_BI,
    ITEM_TAG_KVAH_BI,
    ITEM_TAG_KVAH_TZ1_BI,
    ITEM_TAG_KVAH_TZ2_BI,
    ITEM_TAG_KVAH_TZ3_BI,
    ITEM_TAG_KVAH_TZ4_BI,
    ITEM_TAG_KVAH_TZ5_BI,
    ITEM_TAG_KVAH_TZ6_BI,
    ITEM_TAG_KVAH_TZ7_BI,
    ITEM_TAG_KVAH_TZ8_BI,
    ITEM_TAG_MD_KW_BI,
    ITEM_TAG_MD_KW_TZ1_BI,
    ITEM_TAG_MD_KW_TZ2_BI,
    ITEM_TAG_MD_KW_TZ3_BI,
    ITEM_TAG_MD_KW_TZ4_BI,
    ITEM_TAG_MD_KW_TZ5_BI,
    ITEM_TAG_MD_KW_TZ6_BI,
    ITEM_TAG_MD_KW_TZ7_BI,
    ITEM_TAG_MD_KW_TZ8_BI,
    ITEM_TAG_MD_KVA_BI,
    ITEM_TAG_MD_KVA_TZ1_BI,
    ITEM_TAG_MD_KVA_TZ2_BI,
    ITEM_TAG_MD_KVA_TZ3_BI,
    ITEM_TAG_MD_KVA_TZ4_BI,
    ITEM_TAG_MD_KVA_TZ5_BI,
    ITEM_TAG_MD_KVA_TZ6_BI,
    ITEM_TAG_MD_KVA_TZ7_BI,
    ITEM_TAG_MD_KVA_TZ8_BI,
    ITEM_TAG_DATETIME_MD_KW_BI,
    ITEM_TAG_DATETIME_MD_KW_TZ1_BI,
    ITEM_TAG_DATETIME_MD_KW_TZ2_BI,
    ITEM_TAG_DATETIME_MD_KW_TZ3_BI,
    ITEM_TAG_DATETIME_MD_KW_TZ4_BI,
    ITEM_TAG_DATETIME_MD_KW_TZ5_BI,
    ITEM_TAG_DATETIME_MD_KW_TZ6_BI,
    ITEM_TAG_DATETIME_MD_KW_TZ7_BI,
    ITEM_TAG_DATETIME_MD_KW_TZ8_BI,
    ITEM_TAG_DATETIME_MD_KVA_BI,
    ITEM_TAG_DATETIME_MD_KVA_TZ1_BI,
    ITEM_TAG_DATETIME_MD_KVA_TZ2_BI,
    ITEM_TAG_DATETIME_MD_KVA_TZ3_BI,
    ITEM_TAG_DATETIME_MD_KVA_TZ4_BI,
    ITEM_TAG_DATETIME_MD_KVA_TZ5_BI,
    ITEM_TAG_DATETIME_MD_KVA_TZ6_BI,
    ITEM_TAG_DATETIME_MD_KVA_TZ7_BI,
    ITEM_TAG_DATETIME_MD_KVA_TZ8_BI,
    ITEM_TAG_POWER_ON_DURATION_BI,
    ITEM_TAG_KVARH_KWH_EXPO_BI,
    ITEM_TAG_KVARH_KVAH_EXPO_BI,
    ITEM_TAG_KVARH_Q1_BI,
    ITEM_TAG_KVARH_Q2_BI,
    ITEM_TAG_KVARH_Q3_BI,
    ITEM_TAG_KVARH_Q4_BI,
    ITEM_TAG_TAMPER_COUNT_BI,
    //ITEM_TAG_DAY_ID,
    ITEM_TAG_DAY_PROFILE_START_TIME_1,
    ITEM_TAG_DAY_PROFILE_START_TIME_2,
    ITEM_TAG_DAY_PROFILE_START_TIME_3,
    ITEM_TAG_DAY_PROFILE_START_TIME_4,
    ITEM_TAG_DAY_PROFILE_START_TIME_5,
    ITEM_TAG_DAY_PROFILE_START_TIME_6,
    ITEM_TAG_DAY_PROFILE_START_TIME_7,
    ITEM_TAG_DAY_PROFILE_START_TIME_8,
    //
    ITEM_TAG_DAY_PROFILE_PASSIVE_SCRIPT_ID9,
    ITEM_TAG_DAY_PROFILE_PASSIVE_SCRIPT_ID10,
    ITEM_TAG_DAY_PROFILE_PASSIVE_SCRIPT_ID11,
    ITEM_TAG_DAY_PROFILE_PASSIVE_SCRIPT_ID12,
    ITEM_TAG_DAY_PROFILE_PASSIVE_SCRIPT_ID13,
    ITEM_TAG_DAY_PROFILE_PASSIVE_SCRIPT_ID14,
    ITEM_TAG_DAY_PROFILE_PASSIVE_SCRIPT_ID15,
    ITEM_TAG_DAY_PROFILE_PASSIVE_SCRIPT_ID16,
    ITEM_TAG_DAY_PROFILE_SCRIPT_ID9,
    ITEM_TAG_DAY_PROFILE_SCRIPT_ID10,
    ITEM_TAG_DAY_PROFILE_SCRIPT_ID11,
    ITEM_TAG_DAY_PROFILE_SCRIPT_ID12,
    ITEM_TAG_DAY_PROFILE_SCRIPT_ID13,
    ITEM_TAG_DAY_PROFILE_SCRIPT_ID14,
    ITEM_TAG_DAY_PROFILE_SCRIPT_ID15,
    ITEM_TAG_DAY_PROFILE_SCRIPT_ID16,
    //
    ITEM_TAG_DAY_PROFILE_PASSIVE_START_TIME_1,
    ITEM_TAG_DAY_PROFILE_PASSIVE_START_TIME_2,
    ITEM_TAG_DAY_PROFILE_PASSIVE_START_TIME_3,
    ITEM_TAG_DAY_PROFILE_PASSIVE_START_TIME_4,
    ITEM_TAG_DAY_PROFILE_PASSIVE_START_TIME_5,
    ITEM_TAG_DAY_PROFILE_PASSIVE_START_TIME_6,
    ITEM_TAG_DAY_PROFILE_PASSIVE_START_TIME_7,
    ITEM_TAG_DAY_PROFILE_PASSIVE_START_TIME_8,
    ITEM_TAG_DAY_PROFILE_SCRIPT_ID1,
    ITEM_TAG_DAY_PROFILE_SCRIPT_ID2,
    ITEM_TAG_DAY_PROFILE_SCRIPT_ID3,
    ITEM_TAG_DAY_PROFILE_SCRIPT_ID4,
    ITEM_TAG_DAY_PROFILE_SCRIPT_ID5,
    ITEM_TAG_DAY_PROFILE_SCRIPT_ID6,
    ITEM_TAG_DAY_PROFILE_SCRIPT_ID7,
    ITEM_TAG_DAY_PROFILE_SCRIPT_ID8,
    ITEM_TAG_DAY_PROFILE_PASSIVE_SCRIPT_ID1,
    ITEM_TAG_DAY_PROFILE_PASSIVE_SCRIPT_ID2,
    ITEM_TAG_DAY_PROFILE_PASSIVE_SCRIPT_ID3,
    ITEM_TAG_DAY_PROFILE_PASSIVE_SCRIPT_ID4,
    ITEM_TAG_DAY_PROFILE_PASSIVE_SCRIPT_ID5,
    ITEM_TAG_DAY_PROFILE_PASSIVE_SCRIPT_ID6,
    ITEM_TAG_DAY_PROFILE_PASSIVE_SCRIPT_ID7,
    ITEM_TAG_DAY_PROFILE_PASSIVE_SCRIPT_ID8,
    ITEM_TAG_BILLING_SCHEDULE_EXEC_TIME,
    ITEM_TAG_BILLING_SCHEDULE_EXEC_DAY,
    ITEM_TAG_DAY_PROFILE_ACTIVATE_TIME,

    // Tamper Profile item Tags
    ITEM_TAG_DATETIME_TMPR,
    ITEM_TAG_ID_TMPR,
    #ifdef SINGLE_PHASE_METER
    ITEM_TAG_I_MAX_TMPR,
    ITEM_TAG_V_TMPR,
    ITEM_TAG_PF_TMPR,
    #else
    ITEM_TAG_IR_TMPR,
    ITEM_TAG_IY_TMPR,
    ITEM_TAG_IB_TMPR,
    ITEM_TAG_VR_TMPR,
    ITEM_TAG_VY_TMPR,
    ITEM_TAG_VB_TMPR,
    ITEM_TAG_PFR_TMPR,
    ITEM_TAG_PFY_TMPR,
    ITEM_TAG_PFB_TMPR,
    #endif
    ITEM_TAG_CUM_KWH_TOTAL_TMPR,
    ITEM_TAG_CUM_KVAR_LAG_TOTAL_TMPR,
    ITEM_TAG_CUM_KVAR_LEAD_TOTAL_TMPR,
    ITEM_TAG_CUM_KVAH_TOTAL_TMPR,
    ITEM_TAG_KW_TOTAL_TMPR,

    //Name Plate Details
    ITEM_TAG_SLNO,
    ITEM_TAG_DEVICE_ID,
    ITEM_TAG_MANFACT_NAME,
    ITEM_TAG_FW_VERSION,
    ITEM_TAG_METER_TYPE,
    ITEM_TAG_METER_CATEGORY,
    ITEM_TAG_CURRENT_RATING,
    ITEM_TAG_MANFACT_YEAR,
    ITEM_TAG_SAP_LIST,
    ITEM_TAG_CT_RATIO,
    ITEM_TAG_PT_RATIO,
    ITEM_TAG_NIC_SERIAL,
    ITEM_TAG_SIM_NUMBER,
    ITEM_TAG_VOLTAGE_RATE,
    ITEM_TAG_METERT_CONSTANT,
  };
  typedef struct
  {
    unsigned char Bill_Date[12];
    unsigned long Sys_Power_Factor;
    unsigned long long Cumm_Energy_KWh;
    unsigned long Cumm_Energy_KWh_TZ[8];
    unsigned long long Cumm_Energy_KVAh;
    unsigned long Cumm_Energy_KVAh_TZ[8];
    unsigned long MD_KW;
    unsigned char MD_KW_DT[12];
    unsigned long MD_KW_TZ[8];
    unsigned char MD_KW_TZ_DT[8][12];
    unsigned long MD_KVA;
    unsigned char MD_KVA_DT[12];
    unsigned long MD_KVA_TZ[8];
    unsigned char MD_KVA_TZ_DT[8][12];
    unsigned long int Pwr_on_duration;
    unsigned long int tamper_count;
    unsigned long long Cumm_Energy_KWh_expo;
    unsigned long long Cumm_Energy_KVAh_expo;
    unsigned long long Cumm_Energy_KVarh_Q1;
    unsigned long long Cumm_Energy_KVarh_Q2;
    unsigned long long Cumm_Energy_KVarh_Q3;
    unsigned long long Cumm_Energy_KVarh_Q4;
  } sBilling_Profile;

  typedef struct
  {
    unsigned char Tamper_Date[12];
    uint16_t Tamper_ID;
    uint32_t IRrms;
    uint32_t IYrms;
    uint32_t IBrms;
    uint16_t VRrms;
    uint16_t VYrms;
    uint16_t VBrms;
    uint16_t PFR;
    uint16_t PFY;
    uint16_t PFB;
    uint64_t E_Active;
    uint64_t E_Reactive_Lag;
    uint32_t E_Reactive_Lead;
    uint32_t E_Apparent;
    uint32_t tot_kw;
    uint32_t tamper_cnt;
  } sTamper_Profile;

  typedef struct
  {
    unsigned char Load_Date[12];
    uint32_t IRrms;
    uint32_t IYrms;
    uint32_t IBrms;
    uint16_t VRrms;
    uint16_t VYrms;
    uint16_t VBrms;
    uint16_t PFR;
    uint16_t PFY;
    uint16_t PFB;
    uint32_t E_Active;
    uint32_t E_Reactive_Lag;
    uint32_t E_Reactive_Lead;
    uint32_t E_Apparent;
    uint32_t tot_kw;
  } sLoad_Profile;
  //////////////////////////////////////
  int svr_initialize(
    dlmsServerSettings* settings);

  int svr_handleRequest(
    dlmsServerSettings* settings,
    gxByteBuffer* data,
    gxByteBuffer* reply);

  int svr_handleRequest2(
    dlmsServerSettings* settings,
    unsigned char* buff,
    unsigned short size,
    gxByteBuffer* reply);

  /**
  * Check is data sent to this server.
  *
  * @param serverAddress
  *            Server address.
  * @param clientAddress
  *            Client address.
  * @return True, if data is sent to this server.
  */
  unsigned char svr_isTarget(
    unsigned long int serverAddress,
    unsigned long clientAddress);

  /**
  * Get attribute access level.
  */
  //DLMS_ACCESS_MODE svr_getAttributeAccess(
  //    dlmsServerSettings *settings,
  //    gxObject *obj,
  //    unsigned char index);
  //
  ///**
  //* Get method access level.
  //*/
  //DLMS_METHOD_ACCESS_MODE svr_getMethodAccess(
  //    dlmsServerSettings *settings,
  //    gxObject *obj,
  //    unsigned char index);

  /**
  * called when client makes connection to the server.
  */
  int svr_connected(
    dlmsServerSettings* settings);

  /**
      * Client has try to made invalid connection. Password is incorrect.
      *
      * @param connectionInfo
      *            Connection information.
      */
      //int svr_invalidConnection(
      //    dlmsServerSettings *settings);

      /**
      * called when client clses connection to the server.
      */
  int svr_disconnected(
    dlmsServerSettings* settings);

  /**
      * Read selected item(s).
      *
      * @param args
      *            Handled read requests.
      */
      //void svr_read(
      //    dlmsServerSettings* settings,
      //    gxValueEventCollection* args);
      //
      ///**
      //    * Write selected item(s).
      //    *
      //    * @param args
      //    *            Handled write requests.
      //    */
      //void svr_write(
      //    dlmsServerSettings* settings,
      //    gxValueEventCollection* args);
      //
      ///**
      //     * Action is occurred.
      //     *
      //     * @param args
      //     *            Handled action requests.
      //     */
      //void svr_action(
      //    dlmsServerSettings* settings,
      //    gxValueEventCollection* args);

      /**
          * Check whether the authentication and password are correct.
          *
          * @param authentication
          *            Authentication level.
          * @param password
          *            Password.
          * @return Source diagnostic.
          */
  DLMS_SOURCE_DIAGNOSTIC svr_validateAuthentication(
    dlmsServerSettings* settings,
    DLMS_AUTHENTICATION authentication,
    gxByteBuffer* password);

  int svr_generateConfirmedServiceError_withLLC(
    dlmsServerSettings* settings,
    DLMS_CONFIRMED_SERVICE_ERROR service,
    DLMS_SERVICE_ERROR type,
    unsigned char code,
    gxByteBuffer* data);

  void svr_setInitialize(dlmsServerSettings* settings);
  void svr_Key_Update(dlmsSettings* settings);
  /////////////////////////////////////////////////////
  //void Sap_List_func(void *data, int direction);
  //void Access_Season_Profile(void *data, int direction);
  //void Access_Week_Profile_Table(void *data, int direction);
  //void Access_Active_Day_Profile_Table(void *data, int direction);
  //void Access_Passive_Day_Profile_Table(void *data, int direction);
  //void passive_cal_name_func(void *data, int direction);
  //void Activate_Passive_Calendar_Time_func(void *data, int direction);
  //void Access_Billing_Script_Execution_Time(void *data, int direction);
  //void Access_Tariff_Scripts(void *data, int direction);
  //void Capture_Instant_Profile_Data(void *data, int direction);
  //void get_Cum_Power_Off_Count (void *data, int direction);
  //void Capture_Voltage_Tamper_Profile_Objects(void *data, int direction);
  //void Capture_Current_Tamper_Profile_Objects(void *data, int direction);
  //void Capture_Power_Fail_Tamper_Profile_Objects(void *data, int direction);
  //void Capture_Transact_Tamper_Profile_Objects(void *data, int direction);
  //void Capture_Other_Tamper_Profile_Objects(void *data, int direction);
  //void Capture_Non_Roll_Tamper_Profile_Objects(void *data, int direction);
  //void Capture_Control_Tamper_Profile_Objects(void *data, int direction);
  //void Capture_Voltage_Tamper_Profile_Data(void *data, int direction);
  //void Capture_Current_Tamper_Profile_Data(void *data, int direction);
  //void Capture_Power_Fail_Tamper_Profile_Data(void *data, int direction);
  //void Capture_Transact_Tamper_Profile_Data(void *data, int direction);
  //void Capture_Other_Tamper_Profile_Data(void *data, int direction);
  //void Capture_Non_Roll_Tamper_Profile_Data(void *data, int direction);
  //void Capture_Control_Tamper_Profile_Data(void *data, int direction);
  //void Capture_Snap_Tamper_Profile_Data(void *data, int direction);
  //void Capture_No_Snap_Tamper_Profile_Data(void *data, int direction);
  //void Capture_Load_Profile_Data(void *data, int direction);
  //void Capture_Name_Plate_Profile_Data(void *data, int direction);
  /* Security object for meter reader association */
  //void security_activate(uint8_t *data,uint16_t data_len,uint8_t *response,uint16_t *response_len);
  //void global_key_transfer(uint8_t *data,uint16_t data_len,uint8_t *response,uint16_t *response_len);
  //void method_response_null(uint8_t *response, uint16_t *response_len);
  /* Security object for utility settings association */

  /* Clock object */
  //void adjust_to_quarter(uint8_t *data,uint16_t data_len,uint8_t *response,uint16_t *response_len);
  //void adjust_to_measuring_period(uint8_t *data,uint16_t data_len,uint8_t *response,uint16_t *response_len);
  //void adjust_to_minute(uint8_t *data,uint16_t data_len,uint8_t *response,uint16_t *response_len);
  //void adjust_to_preset_time(uint8_t *data,uint16_t data_len,uint8_t *response,uint16_t *response_len);
  //void preset_adjusting_time(uint8_t *data,uint16_t data_len,uint8_t *response,uint16_t *response_len);
  //void shift_time(uint8_t *data,uint16_t data_len,uint8_t *response,uint16_t *response_len);
  //void buildauth1(void *data, int direction);
  //void Load_Control_Stat_Function(void *data, int direction);
  //void clear2_dlms(void *data, int direction);
  //void Over_Current_Val_Function(void *data, int direction);
  //void Over_Load_Val_Function(void *data, int direction);
  //void Conn_Time_Interval_Function(void *data, int direction);
  //void Conn_Lockout_Time_Function(void *data, int direction);
  //void Conn_Time_Repeat_Function(void *data, int direction);
  //void Tamper_Occ_Time_Function(void *data, int direction);
  //void Tamper_Res_Time_Function(void *data, int direction);
  //void Capture_Period_BP_Function(void *data, int direction);
  //void Capture_Period_LP_Function(void *data, int direction);
  //void Capture_Period_Daily_LP_Function(void *data, int direction);
  //void Test_Function(void *data, int direction);
  //void pf_avg_val_func(void *data, int direction);
  //void tot_kva_val_func(void *data, int direction);
  //void tot_kw_val_func(void *data, int direction);
  //void Bill_Available_Func(void *data, int direction);
  //void exchange_date_and_time(void *data, int direction);
  //void clock_time_zone_func(void *data, int direction);
  //void Call_Meter_Calibrate(void *data, int direction);
  //void Image_Transfer_Initiate(uint8_t *data, uint16_t data_len, uint8_t *response, uint16_t *response_len);
  //void Image_Block_Transfer(uint8_t *data, uint16_t data_len, uint8_t *response, uint16_t *response_len);
  //void Image_Verify(uint8_t *data, uint16_t data_len, uint8_t *response, uint16_t *response_len);
  //void Image_Activate(uint8_t *data, uint16_t data_len, uint8_t *response, uint16_t *response_len);
  //void get_Image_To_Activate_Info(void *data, int direction);
  //void get_Image_To_Activate_Info(void *data, int direction);
  DLMS_SOURCE_DIAGNOSTIC svr_validateAuthentication(
    dlmsServerSettings* settings,
    DLMS_AUTHENTICATION authentication,
    gxByteBuffer* password);
  void svr_start(dlmsServerSettings* settings);
  int svr_connected(
    dlmsServerSettings* settings);
  int svr_disconnected(
    dlmsServerSettings* settings);
  unsigned char svr_isTarget(
    unsigned long int serverAddress,
    unsigned long clientAddress);
  /////////////////////////////////////////////////////
  //int getObjectCount(gxByteBuffer* buff);
  /**
       * Find object.
       *
       * @param objectType
       *            Object type.
       * @param sn
       *            Short Name. In Logical name referencing this is not used.
       * @param ln
       *            Logical Name. In Short Name referencing this is not used.
       * @return Found object or NULL if object is not found.
       */
       //int svr_findObject(
       //    dlmsServerSettings* settings,
       //    DLMS_OBJECT_TYPE objectType,
       //    int sn,
       //    unsigned char* ln,
       //    gxValueEventArg *e);
       //
       //void svr_clearObject(
       //    dlmsServerSettings* settings,
       //    gxObject* obj);
  extern dlmsServerSettings lnHdlc_uart_port[2];
  extern dlms_user_data st_dlms_user_data[2];
  extern gxByteBuffer dlms_bb, dlms_reply;
  extern uint8_t Meter_Sr_No[];
  extern uint8_t LOGICAL_DEVICE_NAME[];
  extern sBilling_Profile stBilling_Profile;
  extern sTamper_Profile stTamper_Profile;
  extern void kick_watchdog(void);
  extern const struct object_desc_s object_list[];
  extern uint8_t FiFO_LiFO;
  extern e_payment_mode_type Payment_Mode;
  void svr_Get_Security(dlmsSettings* settings);
  void Invoke_Push_msg(gxByteBuffer *bb, st_RTC_time* RTC_time, uint32_t *framecounter);
  void Fill_Data_Buffer_LE(int64_t val, uint16_t len, uint8_t* data, uint16_t* response_len);
  void check_push_events(uint32_t epoch);
  void get_MD_KVA_date(void);
  void get_MD_KW_date(unsigned int index);
  int DLMS_Lib_call_back(
    dlmsServerSettings* settings,
    gxByteBuffer* data,
    gxByteBuffer* reply);
  int startDlmsServers(void);
  void set_default_meter_DLMS_security_keys(void);
  void load_meter_DLMS_security_keys(void);
  void check_and_push_events(st_RTC_time* RTC_time);
  void get_server_system_title(void);
  void logical_name_func(void* data, int direction);
  uint8_t get_device_id(uint8_t* buff);
#endif //SERVER_H