#ifndef EEPROM_STORAGE
#define EEPROM_STORAGE

#include <stdint.h>
#include "crc_ccitt_ffff.h"
#include "eeprom.h"
#include "nic_comm.h"
#include "rtc_user.h"
#include "tampers.h"
#include "Activity_Calendar.h"

#define MEMORY_SECTION_1_START_LOC                               (uint32_t)0UL

#define METER_SERIAL_NUMBER_LOC                                  (uint32_t)MEMORY_SECTION_1_START_LOC
#define METER_SERIAL_NUMBER_SIZE                                 (uint32_t)sizeof(uint8_t) * 13
#define METER_SERIAL_NUMBER_FLAG_LOC                             (uint32_t)METER_SERIAL_NUMBER_LOC + METER_SERIAL_NUMBER_SIZE
#define METER_SERIAL_NUMBER_FLAG_SIZE                            (uint32_t)sizeof(uint8_t)
#define RTC_TIME_STATUS_LOC                                      (uint32_t)METER_SERIAL_NUMBER_FLAG_LOC + METER_SERIAL_NUMBER_FLAG_SIZE
#define RTC_TIME_STATUS_SIZE                                     (uint32_t)sizeof(uint8_t)
#define METER_LOCK_CUSTOM_PROTOCOL_ACCESS_LOC                    (uint32_t)RTC_TIME_STATUS_LOC + RTC_TIME_STATUS_SIZE
#define METER_LOCK_CUSTOM_PROTOCOL_ACCESS_SIZE                   (uint32_t)sizeof(uint32_t)
#define BOARD_SERIAL_NUMBER_LOC                                  (uint32_t)METER_LOCK_CUSTOM_PROTOCOL_ACCESS_LOC + METER_LOCK_CUSTOM_PROTOCOL_ACCESS_SIZE
#define BOARD_SERIAL_NUMBER_SIZE                                 (uint32_t)sizeof(uint32_t)

#define MEMORY_SECTION_1_END_LOC                                 (uint32_t)BOARD_SERIAL_NUMBER_LOC + BOARD_SERIAL_NUMBER_SIZE
#define MEMORY_SECTION_2_START_LOC                               (uint32_t)128UL
/***************************Memory overflow check *********************************/
STATIC_ASSERT(MEMORY_SECTION_1_END_LOC < MEMORY_SECTION_2_START_LOC);

#define MEMORY_CHECK_LOC                                         (uint32_t)MEMORY_SECTION_2_START_LOC
#define MEMORY_CHECK_SIZE                                        (uint32_t)sizeof(uint16_t)

#define MEMORY_BLANK_CHECK_DATA_LOC                              (uint32_t)MEMORY_CHECK_LOC + MEMORY_CHECK_SIZE
#define MEMORY_BLANK_CHECK_DATA_SIZE                             (uint32_t)sizeof(uint8_t) * 6
#define MEMORY_READ_FIFO_LIFO_LOC                                (uint32_t)MEMORY_BLANK_CHECK_DATA_LOC + MEMORY_BLANK_CHECK_DATA_SIZE
#define MEMORY_READ_FIFO_LIFO_SIZE                               (uint32_t)sizeof(uint8_t)

#define MEMORY_SECTION_2_END_LOC                                 (uint32_t)MEMORY_READ_FIFO_LIFO_LOC + MEMORY_READ_FIFO_LIFO_SIZE
#define MEMORY_SECTION_3_START_LOC                               (uint32_t)256UL
/***************************Memory overflow check *********************************/
STATIC_ASSERT(MEMORY_SECTION_2_END_LOC < MEMORY_SECTION_3_START_LOC);

#define INVOCATION_COUNTER_MR_LOC                                (uint32_t)MEMORY_SECTION_3_START_LOC
#define INVOCATION_COUNTER_MR_SIZE                               (uint32_t)sizeof(uint32_t)
#define INVOCATION_COUNTER_US_LOC                                (uint32_t)INVOCATION_COUNTER_MR_LOC + (INVOCATION_COUNTER_MR_SIZE * 2)
#define INVOCATION_COUNTER_US_SIZE                               (uint32_t)sizeof(uint32_t)
#define INVOCATION_COUNTER_PH_LOC                                (uint32_t)INVOCATION_COUNTER_US_LOC + (INVOCATION_COUNTER_US_SIZE * 2)
#define INVOCATION_COUNTER_PH_SIZE                               (uint32_t)sizeof(uint32_t)
#define INVOCATION_COUNTER_FW_LOC                                (uint32_t)INVOCATION_COUNTER_PH_LOC + (INVOCATION_COUNTER_PH_SIZE * 2)
#define INVOCATION_COUNTER_FW_SIZE                               (uint32_t)sizeof(uint32_t)
#define INVOCATION_COUNTER_IHD_LOC                               (uint32_t)INVOCATION_COUNTER_FW_LOC + (INVOCATION_COUNTER_FW_SIZE * 2)
#define INVOCATION_COUNTER_IHD_SIZE                              (uint32_t)sizeof(uint32_t)

#define LLS_MR_SECRET_LOC                                        (uint32_t)INVOCATION_COUNTER_IHD_LOC + (INVOCATION_COUNTER_IHD_SIZE * 2)
#define LLS_MR_SECRET_SIZE                                       (uint32_t)sizeof(uint8_t) * 8
#define LLS_MR_SECRET_CRC_LOC                                    (uint32_t)LLS_MR_SECRET_LOC + LLS_MR_SECRET_SIZE
#define LLS_MR_SECRET_CRC_SIZE                                   (uint32_t)sizeof(uint16_t)
#define HLS_US_SECRET_LOC                                        (uint32_t)LLS_MR_SECRET_CRC_LOC + LLS_MR_SECRET_CRC_SIZE
#define HLS_US_SECRET_SIZE                                       (uint32_t)sizeof(uint8_t) * 16
#define HLS_US_SECRET_CRC_LOC                                    (uint32_t)HLS_US_SECRET_LOC + HLS_US_SECRET_SIZE
#define HLS_US_SECRET_CRC_SIZE                                   (uint32_t)sizeof(uint16_t)
#define HLS_FW_SECRET_LOC                                        (uint32_t)HLS_US_SECRET_CRC_LOC + HLS_US_SECRET_CRC_SIZE
#define HLS_FW_SECRET_SIZE                                       (uint32_t)sizeof(uint8_t) * 16
#define HLS_FW_SECRET_CRC_LOC                                    (uint32_t)HLS_FW_SECRET_LOC + HLS_FW_SECRET_SIZE
#define HLS_FW_SECRET_CRC_SIZE                                   (uint32_t)sizeof(uint16_t)
#define METER_DLMS_SECURITY_KEYS_LOC                             (uint32_t)HLS_FW_SECRET_CRC_LOC + HLS_FW_SECRET_CRC_SIZE
#define METER_DLMS_SECURITY_KEYS_SIZE                            (uint32_t)sizeof(uint8_t) * 16 * 4
#define METER_DLMS_SECURITY_KEYS_CRC_LOC                         (uint32_t)METER_DLMS_SECURITY_KEYS_LOC + METER_DLMS_SECURITY_KEYS_SIZE
#define METER_DLMS_SECURITY_KEYS_CRC_SIZE                        (uint32_t)sizeof(uint16_t)

//energies storage eeprom addresses
#define KWH_ARR                                                   50 // 50 location to store energies in round robin fashion.
#define KWH_8TYPE_ENERGIES                                        8  // types of energies to write like kwh, kvah, kvarh,..
#define KWH_VAL_SIZE                                              (uint32_t)sizeof(uint64_t)
#define KWH_CHKSUM_SIZE                                           (uint32_t)sizeof(uint16_t)
#define KWH_REMAINDER_SIZE                                        (uint32_t)sizeof(uint32_t)
#define KWH_LOC                                                   (uint32_t)METER_DLMS_SECURITY_KEYS_CRC_LOC + METER_DLMS_SECURITY_KEYS_CRC_SIZE // starting locations for the 8 type of energies tootal = 1600
#define KWH_LOG_SIZE                                              (uint32_t)(KWH_ARR * KWH_VAL_SIZE * KWH_8TYPE_ENERGIES)
#define KWH_CHKSUM_LOC                                            (uint32_t)KWH_LOC + KWH_LOG_SIZE
#define KWH_CHKSUM_LOG_SIZE                                       (uint32_t)(KWH_ARR * KWH_CHKSUM_SIZE * KWH_8TYPE_ENERGIES)
#define KWH_REMAINDER_VALUE_LOC                                   (uint32_t)KWH_CHKSUM_LOC + KWH_CHKSUM_LOG_SIZE
#define KWH_REMAINDER_VALUE_LOG_SIZE                              (uint32_t)sizeof(EM_REMAINDER_ENERGY_COUNTER)

//NIC storage eeprom addresses
#define  STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_LOC                    (uint32_t)KWH_REMAINDER_VALUE_LOC + KWH_REMAINDER_VALUE_LOG_SIZE
#define  STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_SIZE                   (uint32_t)sizeof(NIC_nvm_info)

//Tamper data loction
#define TAMPER_DATA_BASE_LOCATION                                 (uint32_t)STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_LOC + STORAGE_EEPROM_DLMS_NIC_NVM_PARAM_SIZE
#define TAMPER_DATA_RESERVE_SIZE                                  (uint32_t)sizeof(uint8_t) * 64
#define ESW_WORD_LOC                                              (uint32_t)TAMPER_DATA_BASE_LOCATION + TAMPER_DATA_RESERVE_SIZE
#define ESW_WORD_SIZE                                             (uint32_t)sizeof(uint8_t) * 16
#define ESW_BAK_WORD_LOC                                          (uint32_t)ESW_WORD_LOC + ESW_WORD_SIZE
#define ESW_BAK_WORD_SIZE                                         (uint32_t)sizeof(uint8_t) * 16
#define ESWF_WORD_LOC                                             (uint32_t)ESW_BAK_WORD_LOC + ESW_BAK_WORD_SIZE
#define ESWF_WORD_SIZE                                            (uint32_t)sizeof(uint8_t) * 16
#define G_TAMPER_STATUS_BITS_LOC                                  (uint32_t)ESWF_WORD_LOC + ESWF_WORD_SIZE
#define G_TAMPER_STATUS_BITS_SIZE                                 (uint32_t)sizeof(St_tamper_bits)
#define TAMPER_OCC_TIME_LOC                                       (uint32_t)G_TAMPER_STATUS_BITS_LOC + G_TAMPER_STATUS_BITS_SIZE
#define TAMPER_OCC_TIME_SIZE                                      (uint32_t)sizeof(uint16_t)
#define TAMPER_RES_TIME_LOC                                       (uint32_t)TAMPER_OCC_TIME_LOC + TAMPER_OCC_TIME_SIZE
#define TAMPER_RES_TIME_SIZE                                      (uint32_t)sizeof(uint16_t)
#define CUOPEN_ENABLE_TIME_LOC                                    (uint32_t)TAMPER_RES_TIME_LOC + TAMPER_RES_TIME_SIZE
#define CUOPEN_ENABLE_TIME_SIZE                                   (uint32_t)sizeof(uint32_t)
#define CUOPEN_FLAG_LOC                                           (uint32_t)CUOPEN_ENABLE_TIME_LOC + CUOPEN_ENABLE_TIME_SIZE
#define CUOPEN_FLAG_SIZE                                          (uint32_t)sizeof(uint8_t)
#define RF_EVENT_FLAG_LOC                                         (uint32_t)CUOPEN_FLAG_LOC + CUOPEN_FLAG_SIZE
#define RF_EVENT_FLAG_SIZE                                        (uint32_t)sizeof(uint8_t)
#define POWER_FAIL_CNT_LOC                                        (uint32_t)RF_EVENT_FLAG_LOC + RF_EVENT_FLAG_SIZE
#define POWER_FAIL_CNT_SIZE                                       (uint32_t)sizeof(uint32_t)
#define CLOCK_TIME_ZONE_LOC                                       (uint32_t)POWER_FAIL_CNT_LOC + POWER_FAIL_CNT_SIZE
#define CLOCK_TIME_ZONE_SIZE                                      (uint32_t)sizeof(int16_t)
#define POWER_OFF_EPOCH_LOC                                       (uint32_t)CLOCK_TIME_ZONE_LOC + CLOCK_TIME_ZONE_SIZE
#define POWER_OFF_EPOCH_SIZE                                      (uint32_t)sizeof(uint32_t)
#define POWER_OFF_EPOCH_BAK_LOC                                   (uint32_t)POWER_OFF_EPOCH_LOC + POWER_OFF_EPOCH_SIZE
#define POWER_OFF_EPOCH_BAK_SIZE                                  (uint32_t)sizeof(uint32_t)
#define POWER_ON_EPOCH_LOC                                        (uint32_t)POWER_OFF_EPOCH_BAK_LOC + POWER_OFF_EPOCH_BAK_SIZE
#define POWER_ON_EPOCH_SIZE                                       (uint32_t)sizeof(uint32_t)
#define POWER_ON_EPOCH_BAK_LOC                                    (uint32_t)POWER_ON_EPOCH_LOC + POWER_ON_EPOCH_SIZE
#define POWER_ON_EPOCH_BAK_SIZE                                   (uint32_t)sizeof(uint32_t)
#define CUM_POWER_ON_TIME_LOC                                     (uint32_t)POWER_ON_EPOCH_BAK_LOC + POWER_ON_EPOCH_BAK_SIZE
#define CUM_POWER_ON_TIME_SIZE                                    (uint32_t)sizeof(uint32_t)
#define CUM_POWER_OFF_TIME_LOC                                    (uint32_t)CUM_POWER_ON_TIME_LOC + CUM_POWER_ON_TIME_SIZE
#define CUM_POWER_OFF_TIME_SIZE                                   (uint32_t)sizeof(uint32_t)
#define I_MAX_MAG_FLAG_LOC                                        (uint32_t)CUM_POWER_OFF_TIME_LOC + CUM_POWER_OFF_TIME_SIZE
#define I_MAX_MAG_FLAG_SIZE                                       (uint32_t)sizeof(uint8_t)
#define RTC_STATUS_LOC                                            (uint32_t)I_MAX_MAG_FLAG_LOC + I_MAX_MAG_FLAG_SIZE
#define RTC_STATUS_SIZE                                           (uint32_t)sizeof(uint8_t)
#define BATTERY_ISSUE_LOC                                         (uint32_t)RTC_STATUS_LOC + RTC_STATUS_SIZE
#define BATTERY_ISSUE_SIZE                                        (uint32_t)sizeof(uint8_t)
#define TOTAL_TAMPERS_COUNT_LOC                                   (uint32_t)BATTERY_ISSUE_LOC + BATTERY_ISSUE_SIZE
#define TOTAL_TAMPERS_COUNT_SIZE                                  (uint32_t)sizeof(uint32_t) 
#define TOTAL_PROGRAMMING_COUNT_LOC                               (uint32_t)TOTAL_TAMPERS_COUNT_LOC + TOTAL_TAMPERS_COUNT_SIZE
#define TOTAL_PROGRAMMING_COUNT_SIZE                              (uint32_t)sizeof(uint32_t)
#define EVENT_PRESISTENT_TIME_LOC                                 (uint32_t)TOTAL_PROGRAMMING_COUNT_LOC + TOTAL_PROGRAMMING_COUNT_SIZE
#define EVENT_PRESISTENT_TIME_SIZE                                (uint32_t)sizeof(uint16_t) * TOTAL_EVENT_TYPES
#define EVENT_RESTORATION_TIME_LOC                                (uint32_t)EVENT_PRESISTENT_TIME_LOC + EVENT_PRESISTENT_TIME_SIZE
#define EVENT_RESTORATION_TIME_SIZE                               (uint32_t)sizeof(uint16_t) * TOTAL_EVENT_TYPES
#define LATEST_EVENT_OCCURED_LOC                                  (uint32_t)EVENT_RESTORATION_TIME_LOC + EVENT_RESTORATION_TIME_SIZE
#define LATEST_EVENT_OCCURED_SIZE                                 (uint32_t)sizeof(st_latest_event)
#define LATEST_EVENT_RESTORED_LOC                                 (uint32_t)LATEST_EVENT_OCCURED_LOC + LATEST_EVENT_OCCURED_SIZE
#define LATEST_EVENT_RESTORED_SIZE                                (uint32_t)sizeof(st_latest_event)
#define METERING_MODE_VAL_LOC                                     (uint32_t)LATEST_EVENT_RESTORED_LOC + LATEST_EVENT_RESTORED_SIZE
#define METERING_MODE_VAL_SIZE                                    (uint32_t)sizeof(uint8_t)
#define PAYMENT_MODE_VAL_LOC                                      (uint32_t)METERING_MODE_VAL_LOC + METERING_MODE_VAL_SIZE
#define PAYMENT_MODE_VAL_SIZE                                     (uint32_t)sizeof(uint8_t)

//Latch control
#define STORAGE_EEPROM_DLMS_RELAY_OUTPUT_STATE_ADDR               (uint32_t)PAYMENT_MODE_VAL_LOC + PAYMENT_MODE_VAL_SIZE
#define STORAGE_EEPROM_DLMS_RELAY_OUTPUT_STATE_SIZE               (uint32_t)sizeof(uint8_t)
#define STORAGE_EEPROM_DLMS_RELAY_CONTROL_STATE_ADDR              (uint32_t)STORAGE_EEPROM_DLMS_RELAY_OUTPUT_STATE_ADDR + STORAGE_EEPROM_DLMS_RELAY_OUTPUT_STATE_SIZE
#define STORAGE_EEPROM_DLMS_RELAY_CONTROL_STATE_SIZE              (uint32_t)sizeof(uint8_t)
#define STORAGE_EEPROM_DLMS_RELAY_CONTROL_MODE_ADDR               (uint32_t)STORAGE_EEPROM_DLMS_RELAY_CONTROL_STATE_ADDR + STORAGE_EEPROM_DLMS_RELAY_CONTROL_STATE_SIZE
#define STORAGE_EEPROM_DLMS_RELAY_CONTROL_MODE_SIZE               (uint32_t)sizeof(uint8_t)
#define STORAGE_EEPROM_DLMS_RELAY_REMOTE_CONNECTION_STATUS_ADDR   (uint32_t)STORAGE_EEPROM_DLMS_RELAY_CONTROL_MODE_ADDR + STORAGE_EEPROM_DLMS_RELAY_CONTROL_MODE_SIZE
#define STORAGE_EEPROM_DLMS_RELAY_REMOTE_CONNECTION_STATUS_SIZE   (uint32_t)sizeof(uint8_t)

#define STORAGE_EEPROM_DLMS_MINIMUM_OVER_THRESOLD_DURATION_ADDR   (uint32_t)STORAGE_EEPROM_DLMS_RELAY_REMOTE_CONNECTION_STATUS_ADDR + STORAGE_EEPROM_DLMS_RELAY_REMOTE_CONNECTION_STATUS_SIZE
#define STORAGE_EEPROM_DLMS_MINIMUM_OVER_THRESOLD_DURATION_SIZE   (uint32_t)sizeof(uint16_t)
#define STORAGE_EEPROM_DLMS_MINIMUM_UNDER_THRESOLD_DURATION_ADDR  (uint32_t)STORAGE_EEPROM_DLMS_MINIMUM_OVER_THRESOLD_DURATION_ADDR + STORAGE_EEPROM_DLMS_MINIMUM_OVER_THRESOLD_DURATION_SIZE
#define STORAGE_EEPROM_DLMS_MINIMUM_UNDER_THRESOLD_DURATION_SIZE  (uint32_t)sizeof(uint16_t)

#define STORAGE_EEPROM_DLMS_RELAY_NORMAL_DISCONNECT_TIME_ADDR     (uint32_t)STORAGE_EEPROM_DLMS_MINIMUM_UNDER_THRESOLD_DURATION_ADDR + STORAGE_EEPROM_DLMS_MINIMUM_UNDER_THRESOLD_DURATION_SIZE
#define STORAGE_EEPROM_DLMS_RELAY_NORMAL_DISCONNECT_TIME_SIZE     (uint32_t)sizeof(uint16_t)
#define STORAGE_EEPROM_DLMS_RELAY_LOCKOUT_DISCONNECT_TIME_ADDR    (uint32_t)STORAGE_EEPROM_DLMS_RELAY_NORMAL_DISCONNECT_TIME_ADDR + STORAGE_EEPROM_DLMS_RELAY_NORMAL_DISCONNECT_TIME_SIZE
#define STORAGE_EEPROM_DLMS_RELAY_LOCKOUT_DISCONNECT_TIME_SIZE    (uint32_t)sizeof(uint16_t)
#define STORAGE_EEPROM_DLMS_RELAY_LOAD_CHECK_TIME_ADDR            (uint32_t)STORAGE_EEPROM_DLMS_RELAY_LOCKOUT_DISCONNECT_TIME_ADDR + STORAGE_EEPROM_DLMS_RELAY_LOCKOUT_DISCONNECT_TIME_SIZE
#define STORAGE_EEPROM_DLMS_RELAY_LOAD_CHECK_TIME_SIZE            (uint32_t)sizeof(uint16_t)
#define STORAGE_EEPROM_DLMS_RELAY_RECONNECT_COUNT_ADDR            (uint32_t)STORAGE_EEPROM_DLMS_RELAY_LOAD_CHECK_TIME_ADDR + STORAGE_EEPROM_DLMS_RELAY_LOAD_CHECK_TIME_SIZE
#define STORAGE_EEPROM_DLMS_RELAY_RECONNECT_COUNT_SIZE            (uint32_t)sizeof(uint8_t)
#define STORAGE_EEPROM_DLMS_RELAY_WELD_TAMPER_ADDR                (uint32_t)STORAGE_EEPROM_DLMS_RELAY_RECONNECT_COUNT_ADDR + STORAGE_EEPROM_DLMS_RELAY_RECONNECT_COUNT_SIZE
#define STORAGE_EEPROM_DLMS_RELAY_WELD_TAMPER_SIZE                (uint32_t)sizeof(uint8_t)

#define STORAGE_EEPROM_DLMS_OVER_CURRENT_VALUE_ADDR               (uint32_t)STORAGE_EEPROM_DLMS_RELAY_WELD_TAMPER_ADDR + STORAGE_EEPROM_DLMS_RELAY_WELD_TAMPER_SIZE
#define STORAGE_EEPROM_DLMS_OVER_CURRENT_VALUE_SIZE               (uint32_t)sizeof(uint32_t)
#define STORAGE_EEPROM_DLMS_OVER_LOAD_VALUE_ADDR                  (uint32_t)STORAGE_EEPROM_DLMS_OVER_CURRENT_VALUE_ADDR + STORAGE_EEPROM_DLMS_OVER_CURRENT_VALUE_SIZE
#define STORAGE_EEPROM_DLMS_OVER_LOAD_VALUE_SIZE                  (uint32_t)sizeof(uint32_t)
#define STORAGE_EEPROM_DLMS_OVER_BYPASS_VALUE_ADDR                (uint32_t)STORAGE_EEPROM_DLMS_OVER_LOAD_VALUE_ADDR + STORAGE_EEPROM_DLMS_OVER_LOAD_VALUE_SIZE
#define STORAGE_EEPROM_DLMS_OVER_BYPASS_VALUE_SIZE                (uint32_t)sizeof(uint32_t)

//Image transfer
#define STORAGE_EEPROM_DLMS_IMAGE_TRANSFER_ACT_PENDING_ADDR       (uint32_t)STORAGE_EEPROM_DLMS_OVER_BYPASS_VALUE_ADDR + STORAGE_EEPROM_DLMS_OVER_BYPASS_VALUE_SIZE
#define STORAGE_EEPROM_DLMS_IMAGE_TRANSFER_ACT_PENDING_SIZE       (uint32_t)sizeof(uint8_t)
#define STORAGE_EEPROM_DLMS_IMAGE_TRANSFER_STATUS_ADDR            (uint32_t)STORAGE_EEPROM_DLMS_IMAGE_TRANSFER_ACT_PENDING_ADDR + STORAGE_EEPROM_DLMS_IMAGE_TRANSFER_ACT_PENDING_SIZE
#define STORAGE_EEPROM_DLMS_IMAGE_TRANSFER_STATUS_SIZE            (uint32_t)sizeof(uint8_t)
#define STORAGE_EEPROM_DLMS_IMAGE_UPDATE_EPOCH_ADDR               (uint32_t)STORAGE_EEPROM_DLMS_IMAGE_TRANSFER_STATUS_ADDR + STORAGE_EEPROM_DLMS_IMAGE_TRANSFER_STATUS_SIZE
#define STORAGE_EEPROM_DLMS_IMAGE_UPDATE_EPOCH_SIZE               (uint32_t)sizeof(uint32_t)
#define STORAGE_EEPROM_DLMS_IMAGE_TRANSFER_BIT_STATUS_ADDR        (uint32_t)STORAGE_EEPROM_DLMS_IMAGE_UPDATE_EPOCH_ADDR + STORAGE_EEPROM_DLMS_IMAGE_UPDATE_EPOCH_SIZE
#define STORAGE_EEPROM_DLMS_IMAGE_TRANSFER_BIT_STATUS_SIZE        (uint32_t)(sizeof(uint8_t) *256)
//void init_tampers_params(St_tamper *tamper);

/*--------------------------------------------------------------------------------------*/
/* Instant push action time */
#define STORAGE_DLMS_INSTANT_PUSH_TIME_ADDR                       (uint32_t)STORAGE_EEPROM_DLMS_IMAGE_TRANSFER_BIT_STATUS_ADDR + STORAGE_EEPROM_DLMS_IMAGE_TRANSFER_BIT_STATUS_SIZE
#define STORAGE_DLMS_INSTANT_PUSH_TIME_SIZE                       (uint32_t)sizeof(st_push_schedular)

/*--------------------------------------------------------------------------------------*/

/* Pre payment */
#define STORAGE_DLMS_LAST_TOKEN_AMOUNT_ADDR                       (uint32_t)STORAGE_DLMS_INSTANT_PUSH_TIME_ADDR + STORAGE_DLMS_INSTANT_PUSH_TIME_SIZE
#define STORAGE_DLMS_LAST_TOKEN_AMOUNT_SIZE                       (uint32_t)sizeof(int32_t)

#define STORAGE_DLMS_LAST_AMOUNT_RECHARGE_TIME_ADDR               (uint32_t)STORAGE_DLMS_LAST_TOKEN_AMOUNT_ADDR + STORAGE_DLMS_LAST_TOKEN_AMOUNT_SIZE
#define STORAGE_DLMS_LAST_AMOUNT_RECHARGE_TIME_SIZE               (uint32_t)(sizeof(uint8_t)*12)

#define STORAGE_DLMS_TOTAL_LAST_AMOUNT_RECHARGE_ADDR              (uint32_t)STORAGE_DLMS_LAST_AMOUNT_RECHARGE_TIME_ADDR + STORAGE_DLMS_LAST_AMOUNT_RECHARGE_TIME_SIZE
#define STORAGE_DLMS_TOTAL_LAST_AMOUNT_RECHARGE_SIZE              (uint32_t)(sizeof(int32_t))

#define STORAGE_DLMS_CURRENT_BALANCE_AMOUNT_ADDR                  (uint32_t)STORAGE_DLMS_LAST_AMOUNT_RECHARGE_TIME_ADDR + STORAGE_DLMS_LAST_AMOUNT_RECHARGE_TIME_SIZE
#define STORAGE_DLMS_CURRENT_BALANCE_AMOUNT_SIZE                  (uint32_t)(sizeof(int32_t))

#define STORAGE_DLMS_CURRENT_BALANCE_TIME_ADDR                    (uint32_t)STORAGE_DLMS_CURRENT_BALANCE_AMOUNT_ADDR + STORAGE_DLMS_CURRENT_BALANCE_AMOUNT_SIZE
#define STORAGE_DLMS_CURRENT_BALANCE_TIME_SIZE                    (uint32_t)(sizeof(uint8_t)*12)
/*--------------------------------------------------------------------------------------*/
/* Activity calendar , 0x0A40 group */
//#define STORAGE_EEPROM_DLMS_ACTIVITY_CALENDAR_GROUP             STORAGE_EEPROM_BASE+0x0640
#define STORAGE_EEPROM_DLMS_ACTIVITY_CALENDAR_GROUP               10000UL //TODO: Rakesh                                                                         /* 0x0BD9 */

/* Cal_name_active , 0x0A40 */
#define STORAGE_EEPROM_DLMS_CALENDAR_NAME_ACTIVE_ADDR             (uint32_t)STORAGE_EEPROM_DLMS_ACTIVITY_CALENDAR_GROUP
#define STORAGE_EEPROM_DLMS_CALENDAR_NAME_ACTIVE_SIZE             (uint32_t)(sizeof(uint8_t)*16)

#define STORAGE_EEPROM_DLMS_PASSIVE_TO_ACTIVE_ADDR                (uint32_t)STORAGE_EEPROM_DLMS_CALENDAR_NAME_ACTIVE_ADDR + STORAGE_EEPROM_DLMS_CALENDAR_NAME_ACTIVE_SIZE
#define STORAGE_EEPROM_DLMS_PASSIVE_TO_ACTIVE_SIZE                (uint32_t)( sizeof(st_tod_passive_to_active_t) * 1 )    /* 18 (Bytes) x 1 */                                                                   /* 17 Bytes */

///Active Table
/* Cal_season_cnt , 0x0A52 */
#define STORAGE_EEPROM_DLMS_ACTIVE_SEASON_CNT_ADDR                (uint32_t)STORAGE_EEPROM_DLMS_PASSIVE_TO_ACTIVE_ADDR + STORAGE_EEPROM_DLMS_PASSIVE_TO_ACTIVE_SIZE
#define STORAGE_EEPROM_DLMS_ACTIVE_SEASON_CNT_SIZE                (uint32_t)( sizeof(uint8_t) * 1 )    /* 1 (Bytes) x 1 */                                                         /* 83 Bytes */

/* Cal_season_active , 0x0AAC */
#define STORAGE_EEPROM_DLMS_ACTIVE_SEASON_BLK_ADDR                (uint32_t)STORAGE_EEPROM_DLMS_ACTIVE_SEASON_CNT_ADDR + STORAGE_EEPROM_DLMS_ACTIVE_SEASON_CNT_SIZE
#define STORAGE_EEPROM_DLMS_ACTIVE_SEASON_BLK_SIZE                (uint32_t)( sizeof(st_tod_season_t) * MAX_SEASON_PROFILES )    /* 106 (Bytes) x 1 */                                                          /* 49 Bytes */

/* Cal_week_active , 0x0B16 */
#define STORAGE_EEPROM_DLMS_ACTIVE_WEEK_CNT_ADDR                  (uint32_t)STORAGE_EEPROM_DLMS_ACTIVE_SEASON_BLK_ADDR + STORAGE_EEPROM_DLMS_ACTIVE_SEASON_BLK_SIZE
#define STORAGE_EEPROM_DLMS_ACTIVE_WEEK_CNT_SIZE                  (uint32_t)( sizeof(uint8_t) * 1 )    /* 1 (Bytes) x 1 */   

#define STORAGE_EEPROM_DLMS_ACTIVE_WEEK_BLK_ADDR                  (uint32_t)STORAGE_EEPROM_DLMS_ACTIVE_WEEK_CNT_ADDR + STORAGE_EEPROM_DLMS_ACTIVE_WEEK_CNT_SIZE
#define STORAGE_EEPROM_DLMS_ACTIVE_WEEK_BLK_SIZE                  (uint32_t)( sizeof(st_tod_week_t) * MAX_WEEK_PROFILES )    /* 130 (Bytes) x 1 */                                                           /* 205 Bytes */

/* Cal_day_active , 0x0B98 */
#define STORAGE_EEPROM_DLMS_ACTIVE_DAY_CNT_ADDR                   (uint32_t)STORAGE_EEPROM_DLMS_ACTIVE_WEEK_BLK_ADDR + STORAGE_EEPROM_DLMS_ACTIVE_WEEK_BLK_SIZE
#define STORAGE_EEPROM_DLMS_ACTIVE_DAY_CNT_SIZE                   (uint32_t)( sizeof(uint8_t) * 1 )    /* 1 (Bytes) x 1 */   

#define STORAGE_EEPROM_DLMS_ACTIVE_DAY_BLK_ADDR                   (uint32_t)STORAGE_EEPROM_DLMS_ACTIVE_DAY_CNT_ADDR + STORAGE_EEPROM_DLMS_ACTIVE_DAY_CNT_SIZE
#define STORAGE_EEPROM_DLMS_ACTIVE_DAY_BLK_SIZE                   (uint32_t)( sizeof(st_tod_day_t) * MAX_DAY_PROFILES )    /* 18 (Bytes) x 1 */                                                                   /* 17 Bytes */

///Passive Table
/* Cal_season_cnt , 0x0A52 */
#define STORAGE_EEPROM_DLMS_PASSIVE_SEASON_CNT_ADDR               (uint32_t)STORAGE_EEPROM_DLMS_ACTIVE_DAY_BLK_ADDR + STORAGE_EEPROM_DLMS_ACTIVE_DAY_BLK_SIZE
#define STORAGE_EEPROM_DLMS_PASSIVE_SEASON_CNT_SIZE               (uint32_t)( sizeof(uint8_t) * 1 )    /* 1 (Bytes) x 1 */                                                         /* 83 Bytes */

/* Cal_season_active , 0x0AAC */
#define STORAGE_EEPROM_DLMS_PASSIVE_SEASON_BLK_ADDR               (uint32_t)STORAGE_EEPROM_DLMS_PASSIVE_SEASON_CNT_ADDR + STORAGE_EEPROM_DLMS_PASSIVE_SEASON_CNT_SIZE
#define STORAGE_EEPROM_DLMS_PASSIVE_SEASON_BLK_SIZE               (uint32_t)( sizeof(st_tod_season_t) * MAX_SEASON_PROFILES )    /* 106 (Bytes) x 1 */                                                          /* 49 Bytes */

/* Cal_week_active , 0x0B16 */
#define STORAGE_EEPROM_DLMS_PASSIVE_WEEK_CNT_ADDR                 (uint32_t)STORAGE_EEPROM_DLMS_PASSIVE_SEASON_BLK_ADDR + STORAGE_EEPROM_DLMS_PASSIVE_SEASON_BLK_SIZE
#define STORAGE_EEPROM_DLMS_PASSIVE_WEEK_CNT_SIZE                 (uint32_t)( sizeof(uint8_t) * 1 )    /* 1 (Bytes) x 1 */  

#define STORAGE_EEPROM_DLMS_PASSIVE_WEEK_BLK_ADDR                 (uint32_t)STORAGE_EEPROM_DLMS_PASSIVE_WEEK_CNT_ADDR + STORAGE_EEPROM_DLMS_PASSIVE_WEEK_CNT_SIZE
#define STORAGE_EEPROM_DLMS_PASSIVE_WEEK_BLK_SIZE                 (uint32_t)( sizeof(st_tod_week_t) * MAX_WEEK_PROFILES )    /* 130 (Bytes) x 1 */                                                           /* 205 Bytes */

/* Cal_day_active , 0x0B98 */
#define STORAGE_EEPROM_DLMS_PASSIVE_DAY_CNT_ADDR                  (uint32_t)STORAGE_EEPROM_DLMS_PASSIVE_WEEK_BLK_ADDR + STORAGE_EEPROM_DLMS_PASSIVE_WEEK_BLK_SIZE
#define STORAGE_EEPROM_DLMS_PASSIVE_DAY_CNT_SIZE                  (uint32_t)( sizeof(uint8_t) * 1 )    /* 1 (Bytes) x 1 */   

#define STORAGE_EEPROM_DLMS_PASSIVE_DAY_BLK_ADDR                  (uint32_t)STORAGE_EEPROM_DLMS_PASSIVE_DAY_CNT_ADDR + STORAGE_EEPROM_DLMS_PASSIVE_DAY_CNT_SIZE
#define STORAGE_EEPROM_DLMS_PASSIVE_DAY_BLK_SIZE                  (uint32_t)( sizeof(st_tod_day_t) * MAX_DAY_PROFILES )    /* 18 (Bytes) x 1 */                                                              /* 17 Bytes */

#define STORAGE_EEPROM_DLMS_RUNNING_ZONE_ID_ADDR                  (uint32_t)STORAGE_EEPROM_DLMS_PASSIVE_DAY_BLK_ADDR + STORAGE_EEPROM_DLMS_PASSIVE_DAY_BLK_SIZE
#define STORAGE_EEPROM_DLMS_RUNNING_ZONE_ID_SIZE                  (uint32_t)( sizeof(uint8_t) * 1 )    /* 1 (Bytes) x 1 */   

#define STORAGE_EEPROM_DLMS_ZONE_CUMULATIVE_KWH_ADDR              (uint32_t)STORAGE_EEPROM_DLMS_RUNNING_ZONE_ID_ADDR + STORAGE_EEPROM_DLMS_RUNNING_ZONE_ID_SIZE
#define STORAGE_EEPROM_DLMS_ZONE_CUMULATIVE_KWH_SIZE              (uint32_t)( sizeof(uint64_t) * 1 )

#define STORAGE_EEPROM_DLMS_ZONE_CUMULATIVE_KVAH_ADDR             (uint32_t)STORAGE_EEPROM_DLMS_ZONE_CUMULATIVE_KWH_ADDR + STORAGE_EEPROM_DLMS_ZONE_CUMULATIVE_KWH_SIZE
#define STORAGE_EEPROM_DLMS_ZONE_CUMULATIVE_KVAH_SIZE             (uint32_t)( sizeof(uint64_t) * 1 )

#define STORAGE_EEPROM_BASE                                       (uint32_t)STORAGE_EEPROM_DLMS_ZONE_CUMULATIVE_KVAH_ADDR + STORAGE_EEPROM_DLMS_ZONE_CUMULATIVE_KVAH_SIZE

/*--------------------------------------------------------------------------------------*/
/* Event log , 0x1380 group */

#define ONE_EVENT_FULL_LOG                                        ONE_EVENT_ENERGY_DATA_LOG
#define ONE_EVENT_SHORT_LOG                                       ONE_EVENT_ID_LOG

#define STORAGE_EEPROM_DLMS_EVENT_LOG_GROUP                       (uint32_t)STORAGE_EEPROM_BASE+0x0000UL

/* Event_info_0 , 0x1380 */
#define STORAGE_EEPROM_DLMS_EVENT_INFO_VOLTAGE_ADDR               (uint32_t)STORAGE_EEPROM_DLMS_EVENT_LOG_GROUP
#define STORAGE_EEPROM_DLMS_EVENT_INFO_VOLTAGE_SIZE               (uint32_t)( sizeof(buffer_info_t) * 1 )    /* 10 (Bytes) x 1 */                                                                   /* 10 Bytes */

/* Event_info_1 , 0x138A */
#define STORAGE_EEPROM_DLMS_EVENT_INFO_CURRENT_ADDR               (uint32_t)STORAGE_EEPROM_DLMS_EVENT_INFO_VOLTAGE_ADDR + STORAGE_EEPROM_DLMS_EVENT_INFO_VOLTAGE_SIZE
#define STORAGE_EEPROM_DLMS_EVENT_INFO_CURRENT_SIZE               (uint32_t)( sizeof(buffer_info_t) * 1 )    /* 10 (Bytes) x 1 */                                                                   /* 10 Bytes */

/* Event_info_2 , 0x1394 */
#define STORAGE_EEPROM_DLMS_EVENT_INFO_POWER_ADDR                 (uint32_t)STORAGE_EEPROM_DLMS_EVENT_INFO_CURRENT_ADDR + STORAGE_EEPROM_DLMS_EVENT_INFO_CURRENT_SIZE
#define STORAGE_EEPROM_DLMS_EVENT_INFO_POWER_SIZE                 (uint32_t)( sizeof(buffer_info_t) * 1 )    /* 10 (Bytes) x 1 */                                                                   /* 10 Bytes */

/* Event_info_3 , 0x139E */
#define STORAGE_EEPROM_DLMS_EVENT_INFO_TRANS_ADDR                 (uint32_t)STORAGE_EEPROM_DLMS_EVENT_INFO_POWER_ADDR + STORAGE_EEPROM_DLMS_EVENT_INFO_POWER_SIZE
#define STORAGE_EEPROM_DLMS_EVENT_INFO_TRANS_SIZE                 (uint32_t)( sizeof(buffer_info_t) * 1 )    /* 10 (Bytes) x 1 */                                                                   /* 10 Bytes */

/* Event_info_4 , 0x13A8 */
#define STORAGE_EEPROM_DLMS_EVENT_INFO_OTHER_ADDR                 (uint32_t)STORAGE_EEPROM_DLMS_EVENT_INFO_TRANS_ADDR + STORAGE_EEPROM_DLMS_EVENT_INFO_TRANS_SIZE
#define STORAGE_EEPROM_DLMS_EVENT_INFO_OTHER_SIZE                 (uint32_t)( sizeof(buffer_info_t) * 1 )    /* 10 (Bytes) x 1 */                                                                   /* 10 Bytes */

/* Event_info_5 , 0x13B2 */
#define STORAGE_EEPROM_DLMS_EVENT_INFO_NO_ROLL_ADDR               (uint32_t)STORAGE_EEPROM_DLMS_EVENT_INFO_OTHER_ADDR + STORAGE_EEPROM_DLMS_EVENT_INFO_OTHER_SIZE
#define STORAGE_EEPROM_DLMS_EVENT_INFO_NO_ROLL_SIZE               (uint32_t)( sizeof(buffer_info_t) * 1 )    /* 10 (Bytes) x 1 */                                                                   /* 10 Bytes */

/* Event_info_6 , 0x13BC */
#define STORAGE_EEPROM_DLMS_EVENT_INFO_CON_DISCON_ADDR            (uint32_t)STORAGE_EEPROM_DLMS_EVENT_INFO_NO_ROLL_ADDR + STORAGE_EEPROM_DLMS_EVENT_INFO_NO_ROLL_SIZE
#define STORAGE_EEPROM_DLMS_EVENT_INFO_CON_DISCON_SIZE            (uint32_t)( sizeof(buffer_info_t) * 1 )    /* 10 (Bytes) x 1 */                                                                   /* 10 Bytes */

/* Event0_table , 0x13C6 */
#define STORAGE_EEPROM_DLMS_EVENT_VOLTAGE_TABLE_ADDR              (uint32_t)STORAGE_EEPROM_DLMS_EVENT_INFO_CON_DISCON_ADDR + STORAGE_EEPROM_DLMS_EVENT_INFO_CON_DISCON_SIZE
#define STORAGE_EEPROM_DLMS_EVENT_VOLTAGE_TABLE_SIZE              (uint32_t)( sizeof(ONE_EVENT_FULL_LOG) * EVENT_MAX_ENTRIES_VOLTAGE )    /* 33 (Bytes) x 50 */                  /* 33 * 50 = 1650 Bytes */

/* Event1_table , 0x1B82 */
#define STORAGE_EEPROM_DLMS_EVENT_CURRENT_TABLE_ADDR              (uint32_t)STORAGE_EEPROM_DLMS_EVENT_VOLTAGE_TABLE_ADDR + STORAGE_EEPROM_DLMS_EVENT_VOLTAGE_TABLE_SIZE
#define STORAGE_EEPROM_DLMS_EVENT_CURRENT_TABLE_SIZE              (uint32_t)( sizeof(ONE_EVENT_FULL_LOG) * EVENT_MAX_ENTRIES_CURRENT )    /* 33 (Bytes) x 50 */                  /* 33 * 50 = 1650 Bytes */

/* Event2_table , 0x233E */
#define STORAGE_EEPROM_DLMS_EVENT_POWER_TABLE_ADDR                (uint32_t)STORAGE_EEPROM_DLMS_EVENT_CURRENT_TABLE_ADDR + STORAGE_EEPROM_DLMS_EVENT_CURRENT_TABLE_SIZE
#define STORAGE_EEPROM_DLMS_EVENT_POWER_TABLE_SIZE                (uint32_t)( sizeof(ONE_EVENT_SHORT_LOG) * EVENT_MAX_ENTRIES_POWER )    /* 9 (Bytes) x 50 */                    /* 9 * 50 = 450 Bytes */

/* Event3_table , 0x251E */
#define STORAGE_EEPROM_DLMS_EVENT_TRANS_TABLE_ADDR                (uint32_t)STORAGE_EEPROM_DLMS_EVENT_POWER_TABLE_ADDR + STORAGE_EEPROM_DLMS_EVENT_POWER_TABLE_SIZE
#define STORAGE_EEPROM_DLMS_EVENT_TRANS_TABLE_SIZE                (uint32_t)( sizeof(ONE_EVENT_SHORT_LOG) * EVENT_MAX_ENTRIES_TRANS )    /* 9 (Bytes) x 50 */                    /* 9 * 50 = 450 Bytes */

/* Event4_table , 0x26FE */
#define STORAGE_EEPROM_DLMS_EVENT_OTHER_TABLE_ADDR                (uint32_t)STORAGE_EEPROM_DLMS_EVENT_TRANS_TABLE_ADDR + STORAGE_EEPROM_DLMS_EVENT_TRANS_TABLE_SIZE
#define STORAGE_EEPROM_DLMS_EVENT_OTHER_TABLE_SIZE                (uint32_t)( sizeof(ONE_EVENT_FULL_LOG) * EVENT_MAX_ENTRIES_OTHER )    /* 33 (Bytes) x 50 */                    /* 33 * 50 = 1650 Bytes */

/* Event5_table , 0x2EBA */
#define STORAGE_EEPROM_DLMS_EVENT_NO_ROLL_TABLE_ADDR              (uint32_t)STORAGE_EEPROM_DLMS_EVENT_OTHER_TABLE_ADDR + STORAGE_EEPROM_DLMS_EVENT_OTHER_TABLE_SIZE
#define STORAGE_EEPROM_DLMS_EVENT_NO_ROLL_TABLE_SIZE              (uint32_t)( sizeof(ONE_EVENT_SHORT_LOG) * EVENT_MAX_ENTRIES_NON_ROLLER )    /* 9 (Bytes) x 1 */                /* 9 Bytes */

/* Event6_table , 0x2EC2 */
#define STORAGE_EEPROM_DLMS_EVENT_CON_DISCON_TABLE_ADDR           (uint32_t)STORAGE_EEPROM_DLMS_EVENT_NO_ROLL_TABLE_ADDR + STORAGE_EEPROM_DLMS_EVENT_NO_ROLL_TABLE_SIZE
#define STORAGE_EEPROM_DLMS_EVENT_CON_DISCON_TABLE_SIZE           (uint32_t)( sizeof(ONE_EVENT_SHORT_LOG) * EVENT_MAX_ENTRIES_CONTROL )    /* 9 (Bytes) x 50 */                  /* 9 * 50 = 450 Bytes */

/* Last Tamper occurred, 0x063C */
#define STORAGE_EEPROM_DLMS_LAST_TAMPER_OCCUR_ADDR                (uint32_t)STORAGE_EEPROM_DLMS_EVENT_CON_DISCON_TABLE_ADDR + STORAGE_EEPROM_DLMS_EVENT_CON_DISCON_TABLE_SIZE                                                                           /* 0x2DE0 */
#define STORAGE_EEPROM_DLMS_LAST_TAMPER_OCCUR_SIZE                (uint32_t)( sizeof(ONE_EVENT_SHORT_LOG) * 1)    /* 9 (Bytes) x 1 */                                                               /* 9 Bytes */

/* Last Tamper occurred, 0x063C */
#define STORAGE_EEPROM_DLMS_LAST_TAMPER_RESTORE_ADDR              (uint32_t)STORAGE_EEPROM_DLMS_LAST_TAMPER_OCCUR_ADDR + STORAGE_EEPROM_DLMS_LAST_TAMPER_OCCUR_SIZE
#define STORAGE_EEPROM_DLMS_LAST_TAMPER_RESTORE_SIZE              (uint32_t)( sizeof(ONE_EVENT_SHORT_LOG) * 1)    /* 9 (Bytes) x 1 */                                                               /* 9 Bytes */

/* Event Log group reserved, 0x12B6 */
#define STORAGE_EEPROM_DLMS_EVENT_LOG_GROUP_RES_ADDR              (uint32_t)STORAGE_EEPROM_DLMS_LAST_TAMPER_RESTORE_ADDR + STORAGE_EEPROM_DLMS_LAST_TAMPER_RESTORE_SIZE
#define STORAGE_EEPROM_DLMS_EVENT_LOG_GROUP_RES_SIZE              (uint32_t)( sizeof(uint8_t) * 552 )    /* 1 (Bytes) x 552 */                                                                      /* 552 Bytes */                 /* 6949 Bytes */ 

/*--------------------------------------------------------------------------------------*/

/* Maximum demand */

/*--------------------------------------------------------------------------------------*/

#define STORAGE_EEPROM_DLMS_MD_INTEGRATION_BACKUP_ADDR            (uint32_t)STORAGE_EEPROM_DLMS_EVENT_LOG_GROUP_RES_ADDR + STORAGE_EEPROM_DLMS_EVENT_LOG_GROUP_RES_SIZE
#define STORAGE_EEPROM_DLMS_MD_INTEGRATION_BACKUP_SIZE            (uint32_t)( sizeof(ONE_MD_LOG) * 1 )                               /* 34 x 3360 = 114240 */        /* 114284 Bytes */

#define STORAGE_EEPROM_DLMS_MD_INTEGRATION_PERIOD_ADDR            (uint32_t)STORAGE_EEPROM_DLMS_MD_INTEGRATION_BACKUP_ADDR + STORAGE_EEPROM_DLMS_MD_INTEGRATION_BACKUP_SIZE
#define STORAGE_EEPROM_DLMS_MD_INTEGRATION_PERIOD_SIZE            (uint32_t)( sizeof(uint16_t) * 1 )

/*--------------------------------------------------------------------------------------*/

/* Billing log , 0x0DF0 group */

/*--------------------------------------------------------------------------------------*/
#define STORAGE_EEPROM_DLMS_BILLING_LOG_GROUP                     (uint32_t)STORAGE_EEPROM_DLMS_MD_INTEGRATION_PERIOD_ADDR + STORAGE_EEPROM_DLMS_MD_INTEGRATION_PERIOD_SIZE                                                         /* 0x14F5 */

/* Billing info , 0x0DF0 */
#define STORAGE_EEPROM_DLMS_BILLING_INFO_ADDR                     (uint32_t)STORAGE_EEPROM_DLMS_BILLING_LOG_GROUP
#define STORAGE_EEPROM_DLMS_BILLING_INFO_SIZE                     (uint32_t)( sizeof(buffer_info_t) * 1 )    /* 10 (Bytes) x 1 */                                                                   /* 10 Bytes */

/* Billing table , 0x0DFA */
#define STORAGE_EEPROM_DLMS_BILLING_TABLE_ADDR                    (uint32_t)STORAGE_EEPROM_DLMS_BILLING_INFO_ADDR + STORAGE_EEPROM_DLMS_BILLING_INFO_SIZE
#define STORAGE_EEPROM_DLMS_BILLING_TABLE_SIZE                    (uint32_t)( sizeof(ONE_MONTH_ENERGY_DATA_LOG) * BILLING_MAX_ENTRIES )    /* 101 (Bytes) x 12 */                /* 12 * 101 = 1212  Bytes */

/* Billing table , 0x0DFA */
#define STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR                   (uint32_t)STORAGE_EEPROM_DLMS_BILLING_TABLE_ADDR + STORAGE_EEPROM_DLMS_BILLING_TABLE_SIZE
#define STORAGE_EEPROM_DLMS_BILLING_BACKUP_SIZE                   (uint32_t)( sizeof(ONE_MONTH_ENERGY_DATA_LOG) * 1 )

#define STORAGE_EEPROM_DLMS_CUMULATIVE_BILLING_COUNT_ADDR         (uint32_t)STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + STORAGE_EEPROM_DLMS_BILLING_BACKUP_SIZE
#define STORAGE_EEPROM_DLMS_CUMULATIVE_BILLING_COUNT_SIZE         (uint32_t)( sizeof(uint32_t) * 1 )

#define STORAGE_EEPROM_DLMS_BILLING_ACTION_DATETIME_ADDR          (uint32_t)STORAGE_EEPROM_DLMS_CUMULATIVE_BILLING_COUNT_ADDR + STORAGE_EEPROM_DLMS_CUMULATIVE_BILLING_COUNT_SIZE
#define STORAGE_EEPROM_DLMS_BILLING_ACTION_DATETIME_SIZE          (uint32_t)( sizeof(BILLING_DATE_TIME_ACTION) * 1 )

/* Billing Log group reserved, 0x12B6 */
#define STORAGE_EEPROM_DLMS_BILLING_LOG_GROUP_RES_ADDR            (uint32_t)STORAGE_EEPROM_DLMS_BILLING_ACTION_DATETIME_ADDR + STORAGE_EEPROM_DLMS_BILLING_ACTION_DATETIME_SIZE
#define STORAGE_EEPROM_DLMS_BILLING_LOG_GROUP_RES_SIZE            (uint32_t)( sizeof(uint8_t) * 166 )    /* 1 (Bytes) x 202 */                                                                      /* 166 Bytes */                 /* 1388 Bytes */

/*--------------------------------------------------------------------------------------*/

/* Daily load survey log , 0x3100 group */

/*--------------------------------------------------------------------------------------*/
#define STORAGE_EEPROM_DLMS_DAILY_LOAD_SURVEY_LOG_GROUP           (uint32_t)STORAGE_EEPROM_DLMS_BILLING_LOG_GROUP_RES_ADDR + STORAGE_EEPROM_DLMS_BILLING_LOG_GROUP_RES_SIZE                                                             /* 0x301A */

/* Daily info , 0x3100 */
#define STORAGE_EEPROM_DLMS_DAILY_INFO_ADDR                       (uint32_t)STORAGE_EEPROM_DLMS_DAILY_LOAD_SURVEY_LOG_GROUP
#define STORAGE_EEPROM_DLMS_DAILY_INFO_SIZE                       (uint32_t)( sizeof(buffer_info_t) * 1 )    /* 10 (Bytes) x 1 */                                                                   /* 10 Bytes */

/* Daily backup , 0x310A */
#define STORAGE_EEPROM_DLMS_DAILY_BACKUP_ADDR                     (uint32_t)STORAGE_EEPROM_DLMS_DAILY_INFO_ADDR + STORAGE_EEPROM_DLMS_DAILY_INFO_SIZE
#define STORAGE_EEPROM_DLMS_DAILY_BACKUP_SIZE                     (uint32_t)( sizeof(ONE_DAILY_ENERGY_DATA_LOG) * 1 )    /* 20 (Bytes) x 1 */                                                       /* 20 Bytes */

/* Daily table , 0x311C */      //Harjeet
#define STORAGE_EEPROM_DLMS_DAILY_TABLE_ADDR                      (uint32_t)STORAGE_EEPROM_DLMS_DAILY_BACKUP_ADDR + STORAGE_EEPROM_DLMS_DAILY_BACKUP_SIZE
#define STORAGE_EEPROM_DLMS_DAILY_TABLE_SIZE                      (uint32_t)( sizeof(ONE_DAILY_ENERGY_DATA_LOG) * DAILYLOAD_MAX_ENTRIES )    /* 20 (Bytes) x 60 */               /* 70 * 20 = 1400 Bytes */

/* Daily Load Log group reserved, 0x35CE */
#define STORAGE_EEPROM_DLMS_DAILY_LOAD_LOG_GROUP_RES_ADDR         (uint32_t)STORAGE_EEPROM_DLMS_DAILY_TABLE_ADDR + STORAGE_EEPROM_DLMS_DAILY_TABLE_SIZE
#define STORAGE_EEPROM_DLMS_DAILY_LOAD_EVENT_LOG_GROUP_RES_SIZE   (uint32_t)( sizeof(uint8_t) * 362 )    /* 1 (Bytes) x 562 */                                                                    /* 362 Bytes */                 /* 1792 Bytes */

/*--------------------------------------------------------------------------------------*/
/* Block load survey log , 0x3800 group */
#define STORAGE_EEPROM_DLMS_BLOCK_LOAD_SURVEY_LOG_GROUP           (uint32_t)STORAGE_EEPROM_DLMS_DAILY_LOAD_LOG_GROUP_RES_ADDR + STORAGE_EEPROM_DLMS_DAILY_LOAD_EVENT_LOG_GROUP_RES_SIZE                                             /* 0x371A */

/* Blk load survey buffer , 0x3880 */
#define STORAGE_EEPROM_DLMS_BLK_LOAD_SURVEY_INFO_ADDR             (uint32_t)STORAGE_EEPROM_DLMS_BLOCK_LOAD_SURVEY_LOG_GROUP
#define STORAGE_EEPROM_DLMS_BLK_LOAD_SURVEY_INFO_SIZE             (uint32_t)( sizeof(buffer_info_t) * 1 )    /* 10 (Bytes) x 1 */                                                                   /* 10 Bytes */

/* Blk load backup , 0x380A */
#define STORAGE_EEPROM_DLMS_BLK_LOAD_BACKUP_ADDR                  (uint32_t)STORAGE_EEPROM_DLMS_BLK_LOAD_SURVEY_INFO_ADDR + STORAGE_EEPROM_DLMS_BLK_LOAD_SURVEY_INFO_SIZE
#define STORAGE_EEPROM_DLMS_BLK_LOAD_BACKUP_SIZE                  (uint32_t)( sizeof(ONE_BLOCK_BACKUP_ENERGY_DATA_LOG)) * 1     /* 34 (Bytes) x 1 */                                             /* 34 Bytes */

/* Blk load table , 0x3307 */
#define STORAGE_EEPROM_DLMS_BLK_LOAD_TABLE_ADDR                   (uint32_t)STORAGE_EEPROM_DLMS_BLK_LOAD_BACKUP_ADDR + STORAGE_EEPROM_DLMS_BLK_LOAD_BACKUP_SIZE
#define STORAGE_EEPROM_DLMS_BLK_LOAD_TABLE_SIZE                   (uint32_t)( sizeof(ONE_BLOCK_ENERGY_DATA_LOG)) * BLOCKLOAD_MAX_ENTRIES                               /* 34 x 3360 = 114240 */        /* 114284 Bytes */

#define STORAGE_EEPROM_DLMS_BLK_LOAD_AVG_VALUES_ADDR              (uint32_t)STORAGE_EEPROM_DLMS_BLK_LOAD_TABLE_ADDR + STORAGE_EEPROM_DLMS_BLK_LOAD_TABLE_SIZE
#define STORAGE_EEPROM_DLMS_BLK_LOAD_AVG_VALUES_SIZE              (uint32_t)sizeof(BLOCK_ENERGY_AVG_DATA_LOG)
/* Block load capture period */
#define STORAGE_EEPROM_DLMS_BLK_LOAD_CAP_PERIOD_ADDR              (uint32_t)STORAGE_EEPROM_DLMS_BLK_LOAD_AVG_VALUES_ADDR + STORAGE_EEPROM_DLMS_BLK_LOAD_AVG_VALUES_SIZE
#define STORAGE_EEPROM_DLMS_BLK_LOAD_CAP_PERIOD_SIZE              (uint32_t)sizeof(uint16_t)          

#define STORAGE_EEPROM_DLMS_LAST_ADDR                             (EPR_DEVICE_SIZE - EPR_DEVICE_PAGESIZE)

#endif