#ifndef NIC_COMM_H
#define NIC_COMM_H
#include <stdint.h>
#include <string.h>

#ifdef WIN32
    #include <stdint.h>
#else 
    #ifdef __CCRL__
        #include <typedef.h>
    #else
        #include <stdint.h>
    #endif
#endif
#include "bytebuffer.h"
#include "enums.h"
#include "factory_settings.h"


//NIC config
#define FIX_RF_NIC_ENCRYPTION_KEY_SIZE                             17
#define MAX_HARDWARE_VERSION_SIZE                                  16
#define MAX_SOFTWARE_VERSION_SIZE                                  16
#define MAX_NETWORK_PROVIDER_SIZE                                  20
#define MAX_SIM_NUMBER_SIZE                                        24
#define STORAGE_EEPROM_DLMS_NIC_MAC_ADDRESS_SIZE                   10
#define STORAGE_EEPROM_DLMS_NIC_APN_NAME_SIZE                      32
#define STORAGE_EEPROM_DLMS_NIC_APN_USERNAME_SIZE                  32
#define STORAGE_EEPROM_DLMS_NIC_APN_PASSWORD_SIZE                  32
#define STORAGE_EEPROM_DLMS_NIC_SERVER_ADDRESS_SIZE                32
#define STORAGE_EEPROM_DLMS_NIC_SERVER_PORT_SIZE                   2
#define STORAGE_EEPROM_DLMS_NIC_MQTT_USERNAME_SIZE                 32
#define STORAGE_EEPROM_DLMS_NIC_MQTT_PASSWORD_SIZE                 32

#ifdef FACTORY_DEBUG_TEST_SECURITY_KEY_ENABLE
  #define DEFAULT_RF_NIC_ENCRYPTION_KEY                            "RFENCRYPT_DEMO12"
  #define DEFAULT_RF_NIC_NETWORK_ADDRESS                           0x223344UL
  #define DEFAULT_RF_NIC_NETWORK_CHANNEL                           1
  #define DEFAULT_NIC_APN_NAME                                     "www"
  #define DEFAULT_NIC_APN_USERNAME                                 ""
  #define DEFAULT_NIC_APN_PASSWORD                                 ""
  #define DEFAULT_NIC_SERVER_ADDRESS                               "demo-broker.crystalpower.in"
  #define DEFAULT_NIC_SERVER_PORT                                  (uint16_t)8883
  #define DEFAULT_NIC_MQTT_USERNAME                                "mqttmasteruser"
  #define DEFAULT_NIC_MQTT_PASSWORD                                "Orvw69ZwBoc9JqEckgnFK8fZz4f72DY"
#endif

typedef void(*NIC_callback)(void);

typedef enum
{
  NIC_Comm_Removed = 0x01,
  NIC_Comm_Plugged = 0x02,
  NIC_Comm_Replaced = 0x03,
}en_NIC_stat;

typedef enum
{
    DLMS_NIC_OBIS_NULL,
    DLMS_NIC_Meter_NIC_Info = 1,
    DLMS_NIC_Meter_Meter_Reponse,
    DLMS_NIC_Meter_RSSI,
    DLMS_NIC_Meter_SIM_Info
}DLMS_NIC_Meter_Objects;

typedef struct
{
    DLMS_NIC_Meter_Objects obis_enum;
    uint8_t obis[6];
}st_DLMS_OBIS;

typedef enum
{
    Error_Meter_Ok,
    Error_Meter_OBIS_Not_Found
}Error_Meter_Objects;

typedef enum {
    NIC_Type_None,
    NIC_Type_RF,
    NIC_Type_4G,
}NIC_Type;

/*user requested to provide thses params value from user init
in the NIC_nvm_info type g_st_NIC_info.nvm_info structure.*/
#ifdef WIN32
    #pragma pack(push,1)
#else 
    #ifdef __CCRL__
        #pragma pack
    #else
        #error please define proper packing.
    #endif
#endif
typedef struct {
  uint8_t MAC_address[10];
  uint8_t New_MAC_address[10];
  uint8_t network_channel;
  uint32_t network_address;
  uint8_t encryption_key[17];
  uint8_t APN_name[32];
  uint8_t APN_username[32];
  uint8_t APN_password[32];
  uint8_t server_address[32];
  uint16_t server_port;
  uint8_t MQTT_username[32];
  uint8_t MQTT_password[32];
  uint16_t crc;
}NIC_nvm_info;

typedef struct {
  NIC_nvm_info nvm_info;
  uint8_t Meter_number[11];
  uint8_t Detected_MAC_address[10];
  uint8_t NIC_struct_init;
  NIC_Type type;
  uint8_t hardware_ver[16];
  uint8_t software_ver[16];
  uint8_t IMEI[20];
  uint8_t SIM_Number[24];
  uint16_t RSSI;
  uint8_t unauthorised_nic_replacement;
  uint8_t bit_error;
  en_NIC_stat status;
}NIC_info;
#ifdef WIN32
    #pragma pack(pop)
#else
    #ifdef __CCRL__
        #pragma unpack
    #else
        #error please define proper unpacking.
    #endif
#endif

extern const uint8_t NIC_header_data[];
extern NIC_info g_st_NIC_info;
extern NIC_callback NIC_unauthorised_change_callback;

void init_NIC_nvm_params(NIC_info* st_NIC_info, uint8_t* meter_serial);
uint16_t NIC_parse_push_packet(NIC_info* st_NIC_info, uint8_t* data, uint16_t len, gxByteBuffer* reply);
uint16_t NIC_send_response(NIC_info* st_NIC_info, gxByteBuffer* arr);
uint8_t NIC_MAC_validation_and_store(NIC_info* st_NIC_info, uint8_t *data, uint8_t pos, uint8_t length);
void reset_NIC_info(void);
#endif//NIC_COMM_H