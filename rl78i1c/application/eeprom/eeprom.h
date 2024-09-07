/******************************************************************************
  Copyright (C) 2011 Renesas Electronics Corporation, All Rights Reserved.
*******************************************************************************
* File Name    : eeprom.h
* Version      : 1.00
* Description  : EEPROM Application Layer APIs
******************************************************************************
* History : DD.MM.YYYY Version Description
******************************************************************************/

#ifndef _EEPROM_H
#define _EEPROM_H

/******************************************************************************
Includes   <System Includes> , "Project Includes"
******************************************************************************/
/* Code Standard */
#include "typedef.h"                    /* GSCE Standard Typedef */
#include <string.h>

/* Driver */
#include "r_cg_macrodriver.h"           /* MD Macro Driver */
#include "r_cg_userdefine.h"            /* CG User Define */

/* Wrapper */
#include "wrp_user_hardware.h"          /* Wrapper/User IIC */
#include "wrp_app_mcu.h"                /* Wrapper/Core MCU */

//#include "wrp_user_ext.h"

/******************************************************************************
Typedef definitions
******************************************************************************/

/******************************************************************************
Macro definitions
******************************************************************************/
/* Middleware linkage */
#define EPR_SEND_END_CALLBACK           WRP_EPR_SendEndCallback
#define EPR_RECEIVE_END_CALLBACK        WRP_EPR_ReceiveEndCallback
/*
 * Device Information
 * Re-map these below information when change EEPROM device or I2C bus
 */
#define EPR_DEVICE_BUS_ADDRESS              0xA0                    /* EEPROM device bus address */
#define EPR_BUS_SPEED                       1000000                 /* Max I2C Bus speed (Hz) */
#define EPR_DEVICE_START_ADDR               0x0000                  /* EEPROM device start address */
#define EPR_DEVICE_SIZE                     0x20000                 /* EEPROM device size (total bytes) */
#define EPR_DEVICE_PAGESIZE                 0x100                   /* 256 bytes/page */
#define ERP_DEVICE_RESERVE_SPACE            128
#define EPR_DEVICE_WRITE_CYCLE_TIME         (10000)                  /* Spec is 10ms */
#define EPR_WRITE_PROTECT_DISABLE_STATEMENT {;}//{SET_BIT(P6,2,1);}      /* Command or interface to disable write protection, IICA board on EVB not have WP */
#define EPR_WRITE_PROTECT_ENABLE_STATEMENT  {;}//{SET_BIT(P6,2,0);}      /* Command or interface to enable write protection, IICA board on EVB not have WP */
#define EPR_FORMAT_SPECIFIER_STRING_1       "Authenticated_format_EEPROM_1"
#define EPR_FORMAT_SPECIFIER_STRING_2       "Authenticated_format_EEPROM_2"
 /*
  * Delay Specification for EEPROM
  * If the EEPROM is totally different, please consider to change these
  * below information.
  *
  * Normally, these information can be reused.
  */
#define EPR_1BYTE_TIMEOUT                   (10000000.0f / EPR_BUS_SPEED)                                  /* 1 byte timeout (ms) for read/write */
#define EPR_SEND_ADDR_MAX_TIMEOUT           ((uint32_t)(EPR_1BYTE_TIMEOUT * 3))                         /* 2 byte addr + 1 offset */
#define EPR_READ_MAX_TIMEOUT                ((uint32_t)(EPR_1BYTE_TIMEOUT ))       /* Device size + 1 offset */
#define EPR_WRITE_MAX_TIMEOUT               ((uint32_t)(EPR_1BYTE_TIMEOUT ))   /* Page bytes + 1 offset */

  /* EEPROM Return Code */
#define EPR_OK                              0           /* Normal end */
#define EPR_ERROR                           1           /* Error in eeprom */
#define EPR_ERROR_NO_RESPOND                2           /* Device does not respond */
#define EPR_ERROR_SIZE                      3           /* Expected size and address are not suitable */
#define EPR_ERROR_LOW_VOLTAGE               4           /* If there is no dvcc and battery power is low */
#define ERP_VALIDATION_FAILS                5

/******************************************************************************
Variable Externs
******************************************************************************/
/******************************************************************************
Functions Prototypes
******************************************************************************/
/* Control */
typedef void(*EPR_user_callback)(void);
typedef void(*EPR_user_fail_callback)(uint8_t res);

typedef enum
{
  ERP_ERASE_TYPE_1 = 1,
  ERP_ERASE_TYPE_2,
}ERP_ERASE_TYPE;

void    EPR_Init(void);                                         /* EEPROM Init */
uint8_t EPR_Read(uint32_t addr, uint8_t* buf, uint16_t size);   /* EEPROM Read */
uint8_t EPR_Write(uint32_t addr, uint8_t* buf, uint16_t size);  /* EEPROM Write */
uint8_t EPR_Format(uint8_t default_value, uint32_t reserve_address);                                       /* EEPROM Erase */
void EPR_Erase_Reserve(void);
void EPR_Format_Init(uint32_t address, uint16_t size, ERP_ERASE_TYPE type);
void EPR_Format_Check(uint32_t address, uint16_t size);
void write_eeprom(uint32_t address, uint8_t cData);
void write_page_eeprom(uint32_t address, uint8_t *cData, uint16_t len);
void to_eeprom(uint32_t nAddr, uint64_t ldata, uint8_t size);

uint8_t read_eeprom(uint32_t address);
void read_page_eeprom(uint32_t address, uint8_t *data, uint16_t len);
uint64_t from_eeprom(uint32_t nAddr, uint8_t size);

/*
 * Callbacks,
 * Please register these below callbacks to driver callback interfaces
 * before using this module.
 */
void    EPR_SendEndCallback(void);
void    EPR_ReceiveEndCallback(void);

/******************************************************************************
Variable Externs 2
******************************************************************************/
extern EPR_user_callback epr_wd_toggle_callback;
extern EPR_user_fail_callback epr_read_write_fail_callback;
extern EPR_user_callback epr_init_after_full_erase_callback;
extern EPR_user_callback clr1_before_full_erase_callback;
extern EPR_user_callback clr2_before_full_erase_callback;
#endif /* _EEPROM_H */
