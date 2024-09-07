/******************************************************************************
* DISCLAIMER

* This software is supplied by Renesas Electronics Corporation and is only
* intended for use with Renesas products. No other uses are authorized.

* This software is owned by Renesas Electronics Corporation and is protected under
* all applicable laws, including copyright laws.

* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES
* REGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY,
* INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
* PARTICULAR PURPOSE AND NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY
* DISCLAIMED.

* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS
* ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE
* FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES
* FOR ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS
* AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

* Renesas reserves the right, without notice, to make changes to this
* software and to discontinue the availability of this software.
* By using this software, you agree to the additional terms and
* conditions found by accessing the following link:
* http://www.renesas.com/disclaimer
******************************************************************************/
/* Copyright (C) 2011 Renesas Electronics Corporation. All rights reserved.  */
/******************************************************************************
* File Name    : eeprom.c
* Version      : 1.00
* Device(s)    : RL78/I1C
* Tool-Chain   : CubeSuite Version 1.5
* H/W Platform : Unified Energy Meter Platform
* Description  : EEPROM MW Layer APIs
******************************************************************************
* History : DD.MM.YYYY Version Description
******************************************************************************/

/******************************************************************************
Includes   <System Includes> , "Project Includes"
******************************************************************************/
/* Application header */
#include "eeprom.h"
#include "eeprom_storage.h"
/******************************************************************************
Typedef definitions
******************************************************************************/

/******************************************************************************
Macro definitions
******************************************************************************/

/******************************************************************************
Imported global variables and functions (from other files)
******************************************************************************/

/******************************************************************************
Exported global variables and functions (to be accessed by other files)
******************************************************************************/

/******************************************************************************
Private global variables and functions
******************************************************************************/
static uint8_t g_is_send_end = 0;
static uint8_t g_is_receive_end = 0;

EPR_user_callback epr_wd_toggle_callback = 0;
EPR_user_fail_callback epr_read_write_fail_callback = 0;
EPR_user_callback epr_init_after_full_erase_callback = 0;
EPR_user_callback clr1_before_full_erase_callback = 0;
EPR_user_callback clr2_before_full_erase_callback = 0;
/******************************************************************************
* Function Name: void EPR_Init(void)
* Description  : EPR Initialization
* Arguments    : WRP_IIC_Init()
* Return Value : None
******************************************************************************/
void kick_watch_dog(void)
{
  if(epr_wd_toggle_callback != 0)
  {
    epr_wd_toggle_callback();
  }
}

void epr_read_write_fail(uint8_t res)
{
  if(epr_read_write_fail_callback != 0)
  {
    epr_read_write_fail_callback(res);
  }
}
void EPR_Init(void)
{
    uint8_t delay = 100;
    /* Init EEPROM device interface mapping */
    WRP_IIC_Init();

    /* Clear all ack flag */
    g_is_send_end = 0;
    g_is_receive_end = 0;
    EPR_SEND_END_CALLBACK = EPR_SendEndCallback;
    EPR_RECEIVE_END_CALLBACK = EPR_ReceiveEndCallback;

    EPR_WRITE_PROTECT_ENABLE_STATEMENT; /* Enable Write Protect */
    while(delay--)
    {
    MCU_Delay(1000);
    }
}

/******************************************************************************
* Function Name: uint8_t EPR_Read(uint32_t addr, uint8_t* buf, uint16_t size)
* Description  : Read from eeprom
* Arguments    : addr: Local address in eeprom
*              : buf : Buffer to store the result
*              : size: Expected size want to read
* Return Value : Execution status
*              :    EPR_OK                  Normal end
*              :    EPR_ERROR_NO_RESPOND    Device does not respond
*              :    EPR_ERROR_SIZE          Expected size and address are not suitable
******************************************************************************/
uint8_t EPR_Read(uint32_t addr, uint8_t* buf, uint16_t size)
{
    uint32_t    timeout;
    uint8_t     device_addr;                /* Device address */
    uint8_t     local_addr[2];              /* EEPROM Local address */

    if ((!IS_MAINS_PRESENT()) && (IS_BAT_POWER_LOW()))
    {
        return EPR_ERROR_LOW_VOLTAGE;
    }
    kick_watch_dog();
    /* Check user buffer */
    if (buf == NULL)
    {
        return EPR_ERROR;   /* parameter error */
    }

    /* check the address */
    if (size == 0 ||
        addr + size > EPR_DEVICE_SIZE)
    {
        return EPR_ERROR_SIZE;
    }

    /* Get the address */
    device_addr = EPR_DEVICE_BUS_ADDRESS | (uint8_t)((addr >> 15) & 0x06);
    local_addr[0] = (uint8_t)((addr >> 8) & 0xFF);
    local_addr[1] = (uint8_t)(addr & 0xFF);

    /* Send the address (HIGH + LOW) */
    g_is_send_end = 0;
    if (WRP_IIC_SendStart(device_addr, local_addr, 2) != WRP_IIC_OK)
    {
        return EPR_ERROR;
    }
    timeout = EPR_SEND_ADDR_MAX_TIMEOUT;
    while (g_is_send_end == 0)
    {
        MCU_Delay(1);   /* 1us delay */

        timeout--;
        if (timeout == 0)
        {
            return EPR_ERROR_NO_RESPOND;
        }
    }

    /* Delay after stop operation: 10us (+1us tolerance of MCU_Delay)
     * (Stop Set-up time + Bus free time) */
    MCU_Delay(11);

    /* Read the buffer */
    g_is_receive_end = 0;
    timeout = EPR_READ_MAX_TIMEOUT * (size + 1);
    if (WRP_IIC_ReceiveStart(device_addr, buf, size) != WRP_IIC_OK)
    {
        return EPR_ERROR;
    }
    while (g_is_receive_end == 0)
    {
        MCU_Delay(1);   /* 1us delay */

        timeout--;
        if (timeout == 0)
        {
            return EPR_ERROR_NO_RESPOND;
        }
    }

    /* Delay after stop operation: 10us (+1us tolerance of MCU_Delay)
     * (Stop Set-up time + Bus free time) */
    MCU_Delay(11);

    return EPR_OK;  /* Read succesful */
}

/******************************************************************************
* Function Name: uint8_t EPR_Write(uint32_t addr, uint8_t* buf, uint16_t size)
* Description  : Write to eeprom
* Arguments    : addr: Local address in eeprom
*              : buf : Buffer to write to eeprom
*              : size: Expected size (in buf) want to write
* Return Value : Execution status
*              :    EPR_OK                  Normal end
*              :    EPR_ERROR_NO_RESPOND    Device does not respond
*              :    EPR_ERROR_SIZE          Expected size and address are not suitable
******************************************************************************/
//extern uint32_t        dI2CWriteCounter;
uint8_t EPR_Write(uint32_t addr, uint8_t* buf, uint16_t size)
{
    uint32_t    timeout;                                /* Timeout counter */
    uint16_t    page_size;                              /* Page size */
    uint8_t     device_addr;                            /* Device address */
    uint8_t     local_buffer[EPR_DEVICE_PAGESIZE + 2];    /* EEPROM Local address + buffer */
    uint16_t    pos;
    //fp_predicate     fp;
    //TODO: rakesh sept 2022
    if ((!IS_MAINS_PRESENT()) && (IS_BAT_POWER_LOW()))
    {
        return EPR_ERROR_LOW_VOLTAGE;
    }
    kick_watch_dog();
    /* Check user buffer */
    if (buf == NULL)
    {
        return EPR_ERROR;   /* parameter error */
    }

    /* Check the address */
    if (size == 0 ||
        addr + size > EPR_DEVICE_SIZE)
    {
        return EPR_ERROR_SIZE;
    }
    //dI2CWriteCounter++;
    EPR_WRITE_PROTECT_DISABLE_STATEMENT;    /* Disable write protect */
    /* Write each page with evaluated address, size */
    while (size > 0)
    {
        /* Get the compensator of 1 page size */
        page_size = EPR_DEVICE_PAGESIZE - (uint16_t)(addr % EPR_DEVICE_PAGESIZE);
        if (size < page_size)
        {
            page_size = size;
        }

        /* Get the address */
        device_addr = EPR_DEVICE_BUS_ADDRESS | (uint8_t)((addr >> 15) & 0x02);
        local_buffer[0] = (uint8_t)((addr >> 8) & 0xFF);
        local_buffer[1] = (uint8_t)(addr & 0xFF);

        /* Copy the page (from buf) to local_buffer
         * max. is EPR_DEVICE_PAGESIZE */
        for (pos = 0; pos < page_size; pos++)
        {
            local_buffer[pos + 2] = buf[pos];
        }

        /* Send the address (HIGH + LOW) + 1 page data */
        g_is_send_end = 0;
        if (WRP_IIC_SendStart(device_addr, local_buffer, page_size + 2) != WRP_IIC_OK)
        {
            return EPR_ERROR;
        }
        timeout = EPR_WRITE_MAX_TIMEOUT * (page_size + 1);
        while (g_is_send_end == 0)
        {
            MCU_Delay(1);   /* 1us delay */

            timeout--;
            if (timeout == 0)
            {
                return EPR_ERROR_NO_RESPOND;
            }
        }

        /* Delay after write cycle + stop operation */
        if (TE0 == 0)
        {
            MCU_Delay((EPR_DEVICE_WRITE_CYCLE_TIME + 1));
        }
        else
        {
            MCU_Delay((EPR_DEVICE_WRITE_CYCLE_TIME + 1));
        }
        /* Point to next page */
        addr += page_size;
        buf += page_size;
        size -= page_size;
    }

    EPR_WRITE_PROTECT_ENABLE_STATEMENT; /* Enable write protect */

    return EPR_OK;  /* Write succesful */
}


/******************************************************************************
* Function Name: void EPR_SendEndCallback(void)
* Description  : EPR Send End Callback
* Arguments    : None
* Return Value : None
******************************************************************************/
void EPR_SendEndCallback(void)
{
    g_is_send_end = 1;
}

/******************************************************************************
* Function Name: void EPR_ReceiveEndCallback(void)
* Description  : EPR Receive End Callback
* Arguments    : None
* Return Value : None
******************************************************************************/
void EPR_ReceiveEndCallback(void)
{
    g_is_receive_end = 1;
}

/******************************************************************************
* Function Name: void EPR_Format(void)
* Description  : Erases all contents of EPR
* Arguments    : None
* Return Value : Execution status
*              :    EPR_OK                  Normal end
*              :    EPR_ERROR_NO_RESPOND    Device does not respond
*              :    EPR_ERROR_SIZE          Expected size and address are not suitable
******************************************************************************/
uint8_t EPR_Format(uint8_t default_value, uint32_t reserve_address)
{
    uint32_t    data_address = reserve_address;                           /* Address to be written */
    uint8_t     local_buffer[EPR_DEVICE_PAGESIZE];    /* EEPROM Local buffer */
    uint8_t     res;

    memset(&local_buffer, default_value, EPR_DEVICE_PAGESIZE);
    
    if(data_address%EPR_DEVICE_PAGESIZE)
    {
        res = EPR_Write(data_address, &local_buffer[0], (EPR_DEVICE_PAGESIZE-data_address%EPR_DEVICE_PAGESIZE));
        if (EPR_OK != res)
        {
                return res;
        }
        data_address += (EPR_DEVICE_PAGESIZE-data_address%EPR_DEVICE_PAGESIZE);
    }
    while (data_address < STORAGE_EEPROM_DLMS_LAST_ADDR)        //REVIEW: validate
    {
        res = EPR_Write(data_address, &local_buffer[0], EPR_DEVICE_PAGESIZE);
        if (EPR_OK != res)
        {
                return res;
        }
        data_address += EPR_DEVICE_PAGESIZE;
    }
    return EPR_OK;
}

void EPR_Erase_Reserve(void)
{
  uint8_t buf[ERP_DEVICE_RESERVE_SPACE];
  memset(buf, 0 , ERP_DEVICE_RESERVE_SPACE);
  EPR_Write(0, buf, ERP_DEVICE_RESERVE_SPACE);
  //EPR_Write(STORAGE_EEPROM_DLMS_LAST_ADDR, buf, ERP_DEVICE_RESERVE_SPACE);
  //EPR_Write(STORAGE_EEPROM_DLMS_LAST_ADDR + ERP_DEVICE_RESERVE_SPACE, buf, ERP_DEVICE_RESERVE_SPACE);
}

void EPR_Format_Init(uint32_t address, uint16_t size, ERP_ERASE_TYPE type)
{
  if(type == ERP_ERASE_TYPE_1)
  {
    EPR_Write(address, (uint8_t*)EPR_FORMAT_SPECIFIER_STRING_1, size);
  }
  else if(type == ERP_ERASE_TYPE_2)
  {
    EPR_Write(address, (uint8_t*)EPR_FORMAT_SPECIFIER_STRING_2, size);
  }
}

void EPR_Format_Check(uint32_t address, uint16_t size)
{
  uint8_t clear_type = 0;
  uint8_t buffer[sizeof(EPR_FORMAT_SPECIFIER_STRING_1)];
  
  EPR_Read(address, buffer, size);
  
  if(memcmp(EPR_FORMAT_SPECIFIER_STRING_1, buffer, size) == 0)
  {
    if(clr1_before_full_erase_callback != 0)
    {
        clr1_before_full_erase_callback();
    }
    clear_type = 1;
      }
  else if(memcmp(EPR_FORMAT_SPECIFIER_STRING_2, buffer, size) == 0)
  {
    if(clr2_before_full_erase_callback != 0)
    {
        clr2_before_full_erase_callback();
    }
    clear_type = 2;
  }
  if(clear_type != 0)
  {
    if (EPR_OK == EPR_Format(0, ERP_DEVICE_RESERVE_SPACE))
    {
      if(epr_init_after_full_erase_callback != 0)
      {
          epr_init_after_full_erase_callback();
      }
      if(clear_type == 1)
      {
        EPR_Erase_Reserve();
      }
      memset(buffer, 0, size);
      EPR_Write(address, buffer, size);
      while(1);
    }
    else
    {
      epr_read_write_fail(0);
    }
  }
}
//Old functions compatibility
void write_eeprom(uint32_t address, uint8_t cData)
{
        uint8_t res = EPR_OK;
        
        res = EPR_Write(address, &cData, 1);
        if(res != EPR_OK)
        {
            epr_read_write_fail(res);
        }
}

void write_page_eeprom(uint32_t address, uint8_t *cData, uint16_t len)
{
        uint8_t res = EPR_OK;
        
        res = EPR_Write(address, cData, len);
        if(res != EPR_OK)
        {
            epr_read_write_fail(res);
        }
}

uint8_t read_eeprom(uint32_t address)
{
        uint8_t cData;
        uint8_t res = EPR_OK;
        
        res = EPR_Read(address, &cData, 1);
        if(res != EPR_OK)
        {
            epr_read_write_fail(res);
        }
        return cData;
}

void read_page_eeprom(uint32_t address, uint8_t *cData, uint16_t len)
{
        uint8_t res = EPR_OK;
        
        res = EPR_Read(address, cData, len);
        if(res != EPR_OK)
        {
            epr_read_write_fail(res);
        }
}

void to_eeprom(uint32_t nAddr, uint64_t ldata, uint8_t size)
{
        uint8_t res = EPR_OK;
        
        res = EPR_Write(nAddr, (uint8_t*)&ldata, size);
        if(res != EPR_OK)
        {
            epr_read_write_fail(res);
        }
}

uint64_t from_eeprom(uint32_t nAddr, uint8_t size)
{
        uint64_t ldata = 0;
        uint8_t res = EPR_OK;
        
        res = EPR_Read(nAddr, (uint8_t*)&ldata, size);
        if(res != EPR_OK)
        {
            epr_read_write_fail(res);
        }
        return ldata;
}
