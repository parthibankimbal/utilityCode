/******************************************************************************
  Copyright (C) 2011 Renesas Electronics Corporation, All Rights Reserved.
*******************************************************************************
* File Name    : dataflash.h
* Version      : 1.00
* Description  : Data Flash Application Layer APIs
******************************************************************************
* History : DD.MM.YYYY Version Description
******************************************************************************/

#ifndef _DATA_FLASH_H
#define _DATA_FLASH_H

/******************************************************************************
Includes   <System Includes> , "Project Includes"
******************************************************************************/
#include "typedef.h"        /* GSCE Standard Typedef */

/******************************************************************************
Typedef definitions
******************************************************************************/


/******************************************************************************
Macro definitions
******************************************************************************/
/* Dataflash size format:
 * First byte will be reserved for bank status
 * Only half memory will be used, the other will be reserved for backup when writing
 * This is to prevent memory corruption in case of power failure during operation
*/
#define DATAFLASH_DEVICE_START_ADDR         (0x0000)    /* Data flash device start address */
#define DATAFLASH_DEVICE_SIZE               (0x3FD)     /* 1021 bytes */
#define DATAFLASH_DEVICE_PAGE_SIZE          (32)        

/* DATAFLASH Return Code */
#define DATAFLASH_OK                        0           /* Normal end */
#define DATAFLASH_ERROR                     1           /* Error in eeprom */
#define DATAFLASH_ERROR_NO_RESPOND          2           /* Device does not respond */
#define DATAFLASH_ERROR_SIZE                3           /* Error the size of reading data flash */
#define DATAFLASH_NOT_FORMAT                4           /* Dataflash not format */

typedef void(*DataFlash_user_callback)(void); 
/******************************************************************************
Variable Externs
******************************************************************************/

/******************************************************************************
Functions Prototypes
******************************************************************************/
/* Basic operation */
uint8_t DATAFLASH_Init(void);                                                       /* DATAFLASH Init */
uint8_t DATAFLASH_Read(uint32_t addr, uint8_t* buf, uint16_t size);                 /* DATAFLASH Read */
uint8_t DATAFLASH_Write(uint32_t addr, uint8_t* buf, uint16_t size);                /* DATAFLASH Write */

/* Advanced control */
/* DataFlash Self Check of bank header */
uint8_t DATAFLASH_VerifyStatus(void);                                               /* DATAFLASH Self-check function: verify header status content */
uint8_t DATAFLASH_VerifyCRC(uint16_t *p_crc);                                       /* DATAFLASH Self-check function: verify data integerity */

/* Special function: use to erase memory */
uint8_t DATAFLASH_BankErase(uint8_t bank_no);                                       /* DATAFLASH Erase entire bank */
uint8_t DATAFLASH_Format(void);
extern DataFlash_user_callback dataflash_wd_toggle_callback;
#endif /* _DATAFLASH_H */
