#include <stdint.h>
#include "nic_comm.h"
#include "crc_ccitt_ffff.h"
#include "bytebuffer.h"
#include "eeprom_storage.h"

#ifdef WIN32
#include "common.h"
#else
#include "eeprom.h"
#endif

const uint8_t NIC_header_data[] = { 0x00, 0x01, 0x00, 0x01, 0x00, 0x02 };
const uint8_t NIC_header_response_data[] = { 0x00, 0x01, 0x00, 0x02, 0x00, 0x01 };
const uint8_t NIC_4G_Identifier[] = "CELLULAR_NIC";
const uint8_t NIC_RF_Identifier[] = "RF_NIC";
//const uint8_t invoke_id[] = { 0x80, 0x00, 0x00, 0x00, 0x00 };

//push profile OBIS code.
const st_DLMS_OBIS DLMS_OBIS[] = { { DLMS_NIC_Meter_NIC_Info, { 0x00, 0x00, 0x80, 0x01, 0x07, 0xFF }},
                                { DLMS_NIC_Meter_Meter_Reponse, { 0x00, 0x00, 0x80, 0x01, 0x08, 0xFF }},
                                { DLMS_NIC_Meter_RSSI, { 0x00, 0x00, 0x80, 0x01, 0x09, 0xFF }},
                                { DLMS_NIC_Meter_SIM_Info, { 0x00, 0x00, 0x80, 0x01, 0x0A, 0xFF }},
                                { DLMS_NIC_OBIS_NULL,{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }} };

NIC_info g_st_NIC_info;
NIC_callback NIC_unauthorised_change_callback = 0;
//custom memcpy for DLMS strings
void myMemCpy(gxByteBuffer* arr, uint8_t* src_data, uint8_t len, DLMS_datatpyes stringDataType)
{
  if (stringDataType != TAG_DONTCARE)
  {
    bb_setUInt8(arr, stringDataType); //Device ID
    bb_setUInt8(arr, len);
  }
  bb_set(arr, src_data, len);
}
// find OBIS code
DLMS_NIC_Meter_Objects find_obis(uint8_t* obis, const st_DLMS_OBIS* obis_data)
{
  uint8_t null_obis[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  Error_Meter_Objects error = Error_Meter_Ok;
  const st_DLMS_OBIS* obis_ptr = DLMS_OBIS;
  while (!memcmp((uint8_t*)obis_ptr->obis, obis, 6) == 0)
  {
    if (memcmp(obis_ptr->obis, null_obis, 6) == 0)
    {
      error = Error_Meter_OBIS_Not_Found;
      break;
    }
    obis_ptr++;
  }
  return error == Error_Meter_Ok ? obis_ptr->obis_enum : DLMS_NIC_OBIS_NULL;
}
// respose to NIC
uint16_t NIC_send_response(NIC_info* st_NIC_info, gxByteBuffer* arr)
{
  uint16_t length_index, length, crc = 0;
  if ((st_NIC_info->type == NIC_Type_4G) || (st_NIC_info->type == NIC_Type_RF))
  {
    myMemCpy(arr, (uint8_t*)NIC_header_response_data, sizeof(NIC_header_response_data), TAG_DONTCARE);
    length_index = arr->size;
    bb_setUInt16(arr, 0);
    bb_setUInt8(arr, 0x0F);
    bb_setUInt8(arr, 0x80);
    bb_setUInt32(arr, 1);
    if (st_NIC_info->type == NIC_Type_4G)
    {
      bb_setUInt8(arr, TAG_STRUCTURE);
      bb_setUInt8(arr, 10);

      myMemCpy(arr, (uint8_t*)&DLMS_OBIS[1].obis, 6, TAG_OCTET_STRING);
      myMemCpy(arr, (uint8_t*)&st_NIC_info->Meter_number[1], st_NIC_info->Meter_number[0], TAG_VISIBLE_STRING);
      myMemCpy(arr, (uint8_t*)&st_NIC_info->nvm_info.APN_name[1], st_NIC_info->nvm_info.APN_name[0], TAG_VISIBLE_STRING);
      myMemCpy(arr, (uint8_t*)&st_NIC_info->nvm_info.APN_username[1], st_NIC_info->nvm_info.APN_username[0], TAG_VISIBLE_STRING);
      myMemCpy(arr, (uint8_t*)&st_NIC_info->nvm_info.APN_password[1], st_NIC_info->nvm_info.APN_password[0], TAG_VISIBLE_STRING);
      myMemCpy(arr, (uint8_t*)&st_NIC_info->nvm_info.server_address[1], st_NIC_info->nvm_info.server_address[0], TAG_VISIBLE_STRING);
      bb_setUInt8(arr, TAG_UINT16);bb_setUInt16(arr, st_NIC_info->nvm_info.server_port);
      myMemCpy(arr, (uint8_t*)&st_NIC_info->nvm_info.MQTT_username[1], st_NIC_info->nvm_info.MQTT_username[0], TAG_VISIBLE_STRING);
      myMemCpy(arr, (uint8_t*)&st_NIC_info->nvm_info.MQTT_password[1], st_NIC_info->nvm_info.MQTT_password[0], TAG_VISIBLE_STRING);
    }
    else if (st_NIC_info->type == NIC_Type_RF)
    {
      bb_setUInt8(arr, TAG_STRUCTURE);
      bb_setUInt8(arr, 6);
      myMemCpy(arr, (uint8_t*)&DLMS_OBIS[1].obis, 6, TAG_OCTET_STRING);
      myMemCpy(arr, (uint8_t*)&st_NIC_info->Meter_number[1], st_NIC_info->Meter_number[0], TAG_VISIBLE_STRING);
      bb_setUInt8(arr, TAG_UINT32);bb_setUInt32(arr, st_NIC_info->nvm_info.network_address);
      bb_setUInt8(arr, TAG_UINT8);bb_setUInt8(arr, st_NIC_info->nvm_info.network_channel);
      myMemCpy(arr, (uint8_t*)&st_NIC_info->nvm_info.encryption_key[1], 16, TAG_OCTET_STRING);
    }
    //TODO: to check length if correct
    bb_setUInt8(arr, TAG_UINT16);
    length = arr->size - sizeof(NIC_header_response_data);
    bb_setUInt16ByIndex(arr, length_index, length);
    crc = R_CRC_Calculate_Fresh(arr->data, arr->size);
    bb_setUInt16(arr, crc);
  }
  return arr->size;
}

uint8_t NIC_MAC_validation_and_store(NIC_info* st_NIC_info, uint8_t *data, uint8_t pos, uint8_t length)
{
  uint8_t blank_buffer[10] = { 0,0,0,0,0,0,0,0,0,0 };
  if (length > STORAGE_EEPROM_DLMS_NIC_MAC_ADDRESS_SIZE)
  {
    return 0;
  }
  memcpy(st_NIC_info->Detected_MAC_address, &data[pos], length);
  if (memcmp(st_NIC_info->nvm_info.MAC_address, blank_buffer, length) == 0)
  {
    memcpy(st_NIC_info->nvm_info.MAC_address, &data[pos], length);
    st_NIC_info->nvm_info.crc = R_CRC_Calculate_Fresh((uint8_t*)&st_NIC_info->nvm_info, STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_SIZE - 2);
    write_page_eeprom(STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_LOC, (uint8_t*)&st_NIC_info->nvm_info, STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_SIZE);
  }
  else if (((memcmp(st_NIC_info->nvm_info.MAC_address, &data[pos], length)) != 0) && ((memcmp(st_NIC_info->nvm_info.New_MAC_address, &data[pos], length)) != 0))
  {
    st_NIC_info->status = NIC_Comm_Replaced;
    
    if(NIC_unauthorised_change_callback != 0)
    {
      NIC_unauthorised_change_callback();
    }

    //TODO: Rakesh sept 2022 move it to tampers and access via callback;
    //store_event_data(OTHER_EVENT, 199);
    //Trigger_Byte |= (uint8_t)_BV(NIC_REPLACE_TAMPER_BIT);
    st_NIC_info->unauthorised_nic_replacement = TRUE;
  }
  else if ((memcmp(st_NIC_info->nvm_info.New_MAC_address, &data[pos], length)) == 0)
  {
    memcpy(st_NIC_info->nvm_info.MAC_address, &data[pos], length);
    memset(st_NIC_info->nvm_info.New_MAC_address, 0, STORAGE_EEPROM_DLMS_NIC_MAC_ADDRESS_SIZE);
    st_NIC_info->nvm_info.crc = R_CRC_Calculate_Fresh((uint8_t*)&st_NIC_info->nvm_info, STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_SIZE - 2);
    write_page_eeprom(STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_LOC, (uint8_t*)&st_NIC_info->nvm_info, STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_SIZE);
  }
  return length;
}
//NIC to meter push
uint16_t NIC_parse_push_packet(NIC_info* st_NIC_info, uint8_t* data, uint16_t len, gxByteBuffer* reply)
{
  uint8_t temp8_t;
  DLMS_NIC_Meter_Objects obis_enum;
  uint16_t length = ((uint16_t)data[6] << 8) + data[7];
  uint16_t pos = 25, crc = 0, packet_crc;

  if(len < sizeof(NIC_header_data))
  {
    return 0;
  }
  crc = R_CRC_Calculate_Fresh(data, len - 2);
  packet_crc = data[len - 2];
  packet_crc = (packet_crc << 8) | data[len - 1];
  if ((memcmp(data, NIC_header_data, 6) == 0) && (len == length + 8) && (crc == packet_crc))
  {
    obis_enum = find_obis(&data[18], DLMS_OBIS);
    switch (obis_enum)
    {
    default:
    case DLMS_NIC_OBIS_NULL:
      len = 0;
      break;
    case DLMS_NIC_Meter_NIC_Info:
      length = data[pos];
      length++;
      
      if(NIC_MAC_validation_and_store(st_NIC_info, data, pos, length) == 0)
      {
        return 0;
      }
      pos += length + 1;
      length = data[pos];
      st_NIC_info->type = NIC_Type_None;
      if (memcmp(&data[pos + 1], NIC_4G_Identifier, length) == 0)
      {
        st_NIC_info->type = NIC_Type_4G;
      }
      else if (memcmp(&data[pos + 1], NIC_RF_Identifier, length) == 0)
      {
        st_NIC_info->type = NIC_Type_RF;
      }
      if (st_NIC_info->type == NIC_Type_None)
      {
        return 0;
      }
      pos += length + 2;
      length = data[pos];
      if (length >= MAX_HARDWARE_VERSION_SIZE)
      {
        memcpy(st_NIC_info->hardware_ver, &data[pos], MAX_HARDWARE_VERSION_SIZE);
      }
      else
      {
        memcpy(st_NIC_info->hardware_ver, &data[pos], length + 1);
      }
      pos += length + 2;
      length = data[pos];

      if (length >= MAX_SOFTWARE_VERSION_SIZE)
      {
        memcpy(st_NIC_info->software_ver, &data[pos], MAX_SOFTWARE_VERSION_SIZE);
      }
      else
      {
        memcpy(st_NIC_info->software_ver, &data[pos], length + 1);
      }
      len = NIC_send_response(st_NIC_info, reply);
      /*Send data of length len to the NIC*/
      break;
    case DLMS_NIC_Meter_Meter_Reponse:
      break;
    case DLMS_NIC_Meter_RSSI:
      /* TODO: status */
      length = data[pos];
      if (length >= MAX_NETWORK_PROVIDER_SIZE)
      {
        memcpy(st_NIC_info->IMEI, &data[pos], MAX_NETWORK_PROVIDER_SIZE);
      }
      else
      {
        memcpy(st_NIC_info->IMEI, &data[pos], length + 1);
      }
      pos += length + 2;

      length = data[pos];
      if (length >= MAX_SIM_NUMBER_SIZE)
      {
        memcpy(st_NIC_info->SIM_Number, &data[pos], MAX_SIM_NUMBER_SIZE);
      }
      else
      {
        memcpy(st_NIC_info->SIM_Number, &data[pos], length + 1);
      }
      pos += length + 2;

      temp8_t = data[pos];
      if (temp8_t < 30)
      {
        st_NIC_info->RSSI = 0;
      }
      else if (temp8_t < 56)
      {
        st_NIC_info->RSSI = 1;
      }
      else if (temp8_t < 106)
      {
        st_NIC_info->RSSI = 2;
      }
      else if (temp8_t < 156)
      {
        st_NIC_info->RSSI = 3;
      }
      else if (temp8_t < 206)
      {
        st_NIC_info->RSSI = 4;
      }
      else
      {
        st_NIC_info->RSSI = 5;
      }
      pos += 2;
      st_NIC_info->bit_error = data[pos];

      pos += 2;
      st_NIC_info->status = (en_NIC_stat)data[pos];
      len = 0;
      //TODO: discuss for mismatch
      break;
    case DLMS_NIC_Meter_SIM_Info:// not in use.
      length = data[pos];
      if (length >= MAX_NETWORK_PROVIDER_SIZE)
      {
        memcpy(st_NIC_info->IMEI, &data[pos], MAX_NETWORK_PROVIDER_SIZE);
      }
      else
      {
        memcpy(st_NIC_info->IMEI, &data[pos], length + 1);
      }
      pos = length + pos + 2;
      length = data[pos];
      if (length >= MAX_SIM_NUMBER_SIZE)
      {
        memcpy(st_NIC_info->SIM_Number, &data[pos], MAX_SIM_NUMBER_SIZE);
      }
      else
      {
        memcpy(st_NIC_info->SIM_Number, &data[pos], length + 1);
      }
      len = 0;
      break;
    }
  }
  else
  {
    len = 0;
  }
  return len;
}

void init_NIC_nvm_params(NIC_info* st_NIC_info, uint8_t* meter_serial)
{
  uint16_t crc;
  read_page_eeprom(STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_LOC, (uint8_t*)&st_NIC_info->nvm_info, STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_SIZE);
  crc = R_CRC_Calculate_Fresh((uint8_t*)&st_NIC_info->nvm_info, STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_SIZE - 2);
  if(crc != st_NIC_info->nvm_info.crc)
  {
    st_NIC_info->nvm_info.network_address = DEFAULT_RF_NIC_NETWORK_ADDRESS;
    st_NIC_info->nvm_info.network_channel = DEFAULT_RF_NIC_NETWORK_CHANNEL;
    memset((uint8_t*)st_NIC_info->nvm_info.encryption_key, 0, FIX_RF_NIC_ENCRYPTION_KEY_SIZE);
    st_NIC_info->nvm_info.encryption_key[0] = sizeof(DEFAULT_RF_NIC_ENCRYPTION_KEY) - 1;
    if(st_NIC_info->nvm_info.encryption_key[0] != 0)
    {
      memcpy((uint8_t*)&st_NIC_info->nvm_info.encryption_key[1], (uint8_t*)DEFAULT_RF_NIC_ENCRYPTION_KEY, st_NIC_info->nvm_info.encryption_key[0]);
    }
    
    memset((uint8_t*)st_NIC_info->nvm_info.MAC_address, 0, STORAGE_EEPROM_DLMS_NIC_MAC_ADDRESS_SIZE);
    memset((uint8_t*)st_NIC_info->nvm_info.New_MAC_address, 0, STORAGE_EEPROM_DLMS_NIC_MAC_ADDRESS_SIZE);
    
    memset((uint8_t*)st_NIC_info->nvm_info.APN_name, 0, STORAGE_EEPROM_DLMS_NIC_APN_NAME_SIZE);
    st_NIC_info->nvm_info.APN_name[0] = sizeof(DEFAULT_NIC_APN_NAME) - 1;
    if(st_NIC_info->nvm_info.APN_name[0] != 0)
    {
      memcpy((uint8_t*)&st_NIC_info->nvm_info.APN_name[1], (uint8_t*)DEFAULT_NIC_APN_NAME, st_NIC_info->nvm_info.APN_name[0]);
    }
    
    memset((uint8_t*)st_NIC_info->nvm_info.APN_username, 0, STORAGE_EEPROM_DLMS_NIC_APN_USERNAME_SIZE);
    st_NIC_info->nvm_info.APN_username[0] = sizeof(DEFAULT_NIC_APN_USERNAME) - 1;
    if(st_NIC_info->nvm_info.APN_username[0] != 0)
    {
      memcpy((uint8_t*)&st_NIC_info->nvm_info.APN_username[1], (uint8_t*)DEFAULT_NIC_APN_USERNAME, st_NIC_info->nvm_info.APN_username[0]);
    }
    
    memset((uint8_t*)st_NIC_info->nvm_info.APN_password, 0, STORAGE_EEPROM_DLMS_NIC_APN_PASSWORD_SIZE);
    st_NIC_info->nvm_info.APN_password[0] = sizeof(DEFAULT_NIC_APN_PASSWORD) - 1;
    if(st_NIC_info->nvm_info.APN_password[0] != 0)
    {
      memcpy((uint8_t*)&st_NIC_info->nvm_info.APN_password[1], (uint8_t*)DEFAULT_NIC_APN_PASSWORD, st_NIC_info->nvm_info.APN_password[0]);
    }
    
    memset((uint8_t*)st_NIC_info->nvm_info.server_address, 0, STORAGE_EEPROM_DLMS_NIC_SERVER_ADDRESS_SIZE);
    st_NIC_info->nvm_info.server_address[0] = sizeof(DEFAULT_NIC_SERVER_ADDRESS) - 1;
    if(st_NIC_info->nvm_info.server_address[0] != 0)
    {
      memcpy((uint8_t*)&st_NIC_info->nvm_info.server_address[1], DEFAULT_NIC_SERVER_ADDRESS, st_NIC_info->nvm_info.server_address[0]);
    }
    
    st_NIC_info->nvm_info.server_port = DEFAULT_NIC_SERVER_PORT;
    
    memset((uint8_t*)st_NIC_info->nvm_info.MQTT_username, 0, STORAGE_EEPROM_DLMS_NIC_MQTT_USERNAME_SIZE);
    st_NIC_info->nvm_info.MQTT_username[0] = sizeof(DEFAULT_NIC_MQTT_USERNAME) - 1;
    if(st_NIC_info->nvm_info.MQTT_username[0] != 0)
    {
      memcpy((uint8_t*)&st_NIC_info->nvm_info.MQTT_username[1], (uint8_t*)DEFAULT_NIC_MQTT_USERNAME, st_NIC_info->nvm_info.MQTT_username[0]);
    }
    
    memset((uint8_t*)st_NIC_info->nvm_info.MQTT_password, 0, STORAGE_EEPROM_DLMS_NIC_MQTT_PASSWORD_SIZE);
    st_NIC_info->nvm_info.MQTT_password[0] = sizeof(DEFAULT_NIC_MQTT_PASSWORD) - 1;
    if(st_NIC_info->nvm_info.MQTT_password[0] != 0)
    {
      memcpy((uint8_t*)&st_NIC_info->nvm_info.MQTT_password[1], (uint8_t*)DEFAULT_NIC_MQTT_PASSWORD, st_NIC_info->nvm_info.MQTT_password[0]);
    }
    
    /* calculate crc and store in memory */
    st_NIC_info->nvm_info.crc = R_CRC_Calculate_Fresh((uint8_t*)&st_NIC_info->nvm_info, STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_SIZE - 2);
    write_page_eeprom(STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_LOC, (uint8_t*)&st_NIC_info->nvm_info, STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_SIZE);
  }
  if((meter_serial[0] != 0) && (meter_serial[0] <= 10))
  {
    memcpy(st_NIC_info->Meter_number, &meter_serial[0], meter_serial[0] + 1);
  }
  else
  {
    memset(st_NIC_info->Meter_number, 0, 11);
  }
}

void reset_NIC_info(void)
{
  uint16_t dummy_crc = 0xFFFF;
  write_page_eeprom(STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_LOC + offsetof(NIC_nvm_info, crc), (uint8_t*)&dummy_crc, member_size(NIC_nvm_info, crc));
  init_NIC_nvm_params(&g_st_NIC_info, 0);
}