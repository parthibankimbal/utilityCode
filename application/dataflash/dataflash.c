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
* File Name    : dataflash.c
* Version      : 1.00
* Device(s)    : RL78/I1C
* Tool-Chain   : CCRL
* H/W Platform : Unified Energy Meter Platform
* Description  : DATAFLASH MW Layer APIs
******************************************************************************
* History : DD.MM.YYYY Version Description
******************************************************************************/

/******************************************************************************
Includes   <System Includes> , "Project Includes"
******************************************************************************/
/* Driver */
#include "r_cg_macrodriver.h"   /* MD Macro Driver */
#include "r_cg_crc.h"           /* CG CRC Driver */

/* Code Standard */
#include "typedef.h"            /* GSCE Standard Typedef */

/* Application */
#include "pfdl_types.h"         /* Data Flash Type */
#include "pfdl.h"               /* Data Flash Library */
#include "dataflash.h"          /* Data Flash Memory Application */

DataFlash_user_callback dataflash_wd_toggle_callback = 0;
/******************************************************************************
Typedef definitions
******************************************************************************/
enum tagDFLBlock
{
    DATAFLASH_BLOCK1,
    DATAFLASH_BLOCK2,
};

/******************************************************************************
Macro definitions
******************************************************************************/
/* Actual device hardware information */
#define DATAFLASH_INTERNAL_DEVICE_SIZE          (0x0800)    /* 2048 bytes */
#define DATAFLASH_INTERNAL_BLOCK_SIZE           (0x0400)    /* 1024 bytes */

/* DataFlash header information */
#define DATAFLASH_HEADER_VALID                  (0x01)
#define DATAFLASH_HEADER_INVALID                (0xFF)

#define DATAFLASH_HEADER_SIZE                   (3)
#define DATAFLASH_HEADER_VALID_ADDR             (0)
#define DATAFLASH_HEADER_CRC_ADDR               (1)

#define DATAFLASH_HEADER_VALID_SIZE             (1)
#define DATAFLASH_HEADER_CRC_SIZE               (2)

/* Size of buffer when write to dataflash */
#define DATAFLASH_WRITE_BUFFER_SIZE             (32)
#define DATAFLASH_WRITE_LOOP_TIME               (DATAFLASH_INTERNAL_BLOCK_SIZE / DATAFLASH_WRITE_BUFFER_SIZE)


/******************************************************************************
Imported global variables and functions (from other files)
******************************************************************************/

/******************************************************************************
Exported global variables and functions (to be accessed by other files)
******************************************************************************/

/******************************************************************************
Private global variables and functions
******************************************************************************/
/* CRC checking area information: 
 * This must be set by user application to let dataflash know the critical area
 * which need to be check by CRC to ensure the integrity
*/
extern const uint16_t g_dataflash_crc_block_addr;
extern const uint16_t g_dataflash_crc_block_size;

__near const pfdl_descriptor_t g_dataflash_descriptor =
{
    6,      /* 6 MHz */
    0x00,   /* Full speed */
};

__near pfdl_request_t g_data_flash_req;

#define DATAFLASH_ASSIGN_ERASE_REQ(block) \
{ \
    g_data_flash_req.index_u16 = block; \
    g_data_flash_req.data_pu08 = 0; \
    g_data_flash_req.bytecount_u16 = 0; \
    g_data_flash_req.command_enu = PFDL_CMD_ERASE_BLOCK; \
};

#define DATAFLASH_ASSIGN_WRITE_REQ(addr, buffer, size) \
{ \
    g_data_flash_req.index_u16 = addr; \
    g_data_flash_req.data_pu08 = buffer; \
    g_data_flash_req.bytecount_u16 = size; \
    g_data_flash_req.command_enu = PFDL_CMD_WRITE_BYTES; \
};

#define DATAFLASH_ASSIGN_READ_REQ(addr, buffer, size) \
{ \
    g_data_flash_req.index_u16 = addr; \
    g_data_flash_req.data_pu08 = buffer; \
    g_data_flash_req.bytecount_u16 = size; \
    g_data_flash_req.command_enu = PFDL_CMD_READ_BYTES; \
};

#define DATAFLASH_ASSIGN_VERIFY_REQ(addr, size) \
{ \
    g_data_flash_req.index_u16 = addr; \
    g_data_flash_req.bytecount_u16 = size; \
    g_data_flash_req.command_enu = PFDL_CMD_IVERIFY_BYTES; \
};

/******************************************************************************
* Function Name: static uint8_t DATAFLASH_ReadHeaderStatus(uint8_t bank, uint8_t *buf)
* Description  : DATAFLASH Read the header status information
* Arguments    : uint8_t bank: the bank to read the header status byte
*              : uint8_t *buf: buffer to read out (internal function: no checking NULL pointer)
* Return Value : uint8_t, execution status
******************************************************************************/
#ifndef __DEBUG
static 
#endif
uint8_t DATAFLASH_ReadHeaderStatus(uint8_t bank, uint8_t *buf)
{
    uint16_t        addr_u16;
    pfdl_status_t   ret;
    
    /* Open data flash */
    ret = PFDL_Open((pfdl_descriptor_t *)&g_dataflash_descriptor);
    while (ret == PFDL_BUSY)
    {
        ret = PFDL_Handler();
    }
    if (ret != PFDL_OK)
    {
        return DATAFLASH_ERROR; /* data flash error */
    }
    
    /* Address in bank 0 or bank 1 */
    addr_u16 = (bank == 0) ? 0 : DATAFLASH_INTERNAL_BLOCK_SIZE;
    
    /* Read first byte of block */
    DATAFLASH_ASSIGN_READ_REQ(addr_u16, buf, DATAFLASH_HEADER_VALID_SIZE);
    ret = PFDL_Execute(&g_data_flash_req);
    while (ret == PFDL_BUSY)
    {
        ret = PFDL_Handler();
    }
    if (ret != PFDL_OK)
    {
        return DATAFLASH_ERROR; /* data flash error */
    }

    /* Close data flash */
    PFDL_Close();
    
    return DATAFLASH_OK;    /* Read succesful */
}

/******************************************************************************
* Function Name: static uint8_t DATAFLASH_ReadHeaderCRC(uint8_t bank, uint8_t *buf)
* Description  : DATAFLASH Read the header crc information
* Arguments    : uint8_t bank: the bank to read the header status byte
*              : uint8_t *buf: buffer to read out (internal function: no checking NULL pointer)
* Return Value : uint8_t, execution status
******************************************************************************/
#ifndef __DEBUG
static 
#endif
uint8_t DATAFLASH_ReadHeaderCRC(uint8_t bank, uint8_t *buf)
{
    uint16_t        addr_u16;
    pfdl_status_t   ret;
    
    /* Open data flash */
    ret = PFDL_Open((pfdl_descriptor_t *)&g_dataflash_descriptor);
    while (ret == PFDL_BUSY)
    {
        ret = PFDL_Handler();
    }
    if (ret != PFDL_OK)
    {
        return DATAFLASH_ERROR; /* data flash error */
    }
    
    addr_u16 = (bank == 0) ? 0 : DATAFLASH_INTERNAL_BLOCK_SIZE;
    addr_u16 += DATAFLASH_HEADER_CRC_ADDR;
    
    /* Read first byte of block */
    DATAFLASH_ASSIGN_READ_REQ(addr_u16, buf, DATAFLASH_HEADER_CRC_SIZE);
    ret = PFDL_Execute(&g_data_flash_req);
    while (ret == PFDL_BUSY)
    {
        ret = PFDL_Handler();
    }
    if (ret != PFDL_OK)
    {
        return DATAFLASH_ERROR; /* data flash error */
    }

    /* Close data flash */
    PFDL_Close();
    
    return DATAFLASH_OK;    /* Read succesful */
}

/******************************************************************************
* Function Name: static uint8_t DATAFLASH_CalculateCRC(uint8_t bank, uint8_t * buffer, uint16_t * p_crc)
* Description  : Calculate the CRC for entire bank (exclude the header)
* Arguments    : uint8_t bank: the bank to read the header status byte
*              : uint8_t *buffer: buffer from calling function to reduce stack use
*              : uint8_t *p_crc: crc output pointer (internal function: no checking NULL pointer)
* Return Value : Execution status
*              :    DATAFLASH_OK            Normal end
*              :    DATAFLASH_ERROR         Error when reading data
******************************************************************************/
#ifndef __DEBUG
static 
#endif 
uint8_t DATAFLASH_CalculateCRC(uint8_t bank, uint8_t * buffer, uint16_t * p_crc)
{
    uint16_t        start_addr;
    uint16_t        size;
    uint16_t        page_size;
    pfdl_status_t   ret;
    
    start_addr = (bank == 0) ? 0 : DATAFLASH_INTERNAL_BLOCK_SIZE;
    start_addr += DATAFLASH_HEADER_CRC_ADDR;
    
    /* Assign the start_address */
    start_addr = bank * DATAFLASH_INTERNAL_BLOCK_SIZE;
    
    /* Offset the header */
    start_addr += DATAFLASH_HEADER_SIZE;
    
    /* Offset to CRC checking address */
    start_addr += g_dataflash_crc_block_addr;
    
    /* Size of crc checking block */
    size = g_dataflash_crc_block_size;
    
    /* Open data flash */
    ret = PFDL_Open((pfdl_descriptor_t *)&g_dataflash_descriptor);
    while (ret == PFDL_BUSY)
    {
        ret = PFDL_Handler();
    }
    if (ret != PFDL_OK)
    {
        return DATAFLASH_ERROR; /* data flash error */
    }
    
    /* Clear CRC data register */
    R_CRC_Clear();
    
    /* Read the content of active bank */
    while(size > 0)
    {
        /* Choose the range for buffer read */
        if (size < DATAFLASH_WRITE_BUFFER_SIZE)
        {
            page_size = size;
        }
        else
        {
            page_size = DATAFLASH_WRITE_BUFFER_SIZE;
        }
        
        /* Read to buffer */
        DATAFLASH_ASSIGN_READ_REQ(start_addr, buffer, page_size);
        ret = PFDL_Execute(&g_data_flash_req);
        while (ret == PFDL_BUSY)
        {
            ret = PFDL_Handler();
        }
        if (ret != PFDL_OK)
        {
            return DATAFLASH_ERROR; /* data flash error */
        }
        
        /* Push to CRC calculator */
        R_CRC_Calculate(buffer, page_size);
        
        start_addr += page_size;
        size -= page_size;
        
        
        if(dataflash_wd_toggle_callback != 0)
        {
            dataflash_wd_toggle_callback();
        }
    }
    
    /* Output the CRC value */
    *p_crc = R_CRC_GetResult();
    
    /* Finish calculating */
    return DATAFLASH_OK;
}

/******************************************************************************
* Function Name: static uint8_t DATAFLASH_InternalRead(uint32_t addr, uint8_t* buf, uint16_t size)
* Description  : Internal Read from data flash
*              : The difference with provided Read API is:
*              : this function do not check the bank, just read as requested
* Arguments    : addr: Local address in data flash
*              : buf : Buffer to store the result (internal function: no checking NULL pointer)
*              : size: Expected size want to read
* Return Value : Execution status
*              :    DATAFLASH_OK            Normal end
*              :    DATAFLASH_ERROR         Parameter error
*              :    DATAFLASH_ERROR_SIZE    Expected size and address are not suitable
******************************************************************************/
#ifndef __DEBUG
static 
#endif
uint8_t DATAFLASH_InternalRead(uint32_t addr, uint8_t* buf, uint16_t size)
{
    pfdl_status_t ret;
    
    /* This function is internal, so type checking could be ignore */
    
    /* Open data flash */
    ret = PFDL_Open((pfdl_descriptor_t *)&g_dataflash_descriptor);
    while (ret == PFDL_BUSY)
    {
        ret = PFDL_Handler();
    }
    if (ret != PFDL_OK)
    {
        return DATAFLASH_ERROR; /* data flash error */
    }
        
    /* Read to buffer */
    DATAFLASH_ASSIGN_READ_REQ((uint16_t)addr, buf, size);
    ret = PFDL_Execute(&g_data_flash_req);
    while (ret == PFDL_BUSY)
    {
        ret = PFDL_Handler();
    }
    if (ret != PFDL_OK)
    {
        return DATAFLASH_ERROR; /* data flash error */
    }

    /* Close data flash */
    PFDL_Close();
    
    return DATAFLASH_OK;    /* Read succesful */
}


/******************************************************************************
* Function Name: void DATAFLASH_Init(void)
* Description  : DATAFLASH Initialization
* Arguments    : None
* Return Value : uint8_t, execution status
******************************************************************************/
uint8_t DATAFLASH_Init(void)
{
    /* Checking the setting */
    
    /* Size must > 0  */
    /* Calculation range must in range of dataflash size */
    if ((g_dataflash_crc_block_size == 0) ||
        ((g_dataflash_crc_block_addr + g_dataflash_crc_block_size) > DATAFLASH_DEVICE_SIZE))
    {
        return DATAFLASH_ERROR;
    }
    
    return DATAFLASH_OK;
}

/******************************************************************************
* Function Name: uint8_t DATAFLASH_VerifyStatus(void)
* Description  : DATAFLASH Self Checking: memory header
* Arguments    : None
* Return Value : uint8_t, execution status
******************************************************************************/
uint8_t DATAFLASH_VerifyStatus(void)
{
    uint8_t status;
    
    /* Flow:
     * Check header on bank 0 first
     * If valid: return OK
     * If invalid:
     *  Check header on bank 1
     *  If valid: return OK
     *  Else: consider not format
    */
    /* Reading header on bank 0 */
    if (DATAFLASH_ReadHeaderStatus(0, &status) != DATAFLASH_OK)
    {
        return DATAFLASH_ERROR;
    }
    
    if (status == DATAFLASH_HEADER_VALID)
    {
        return DATAFLASH_OK;
    }
    else
    {
        /* Reading header on bank 1 */
        if (DATAFLASH_ReadHeaderStatus(1, &status) != DATAFLASH_OK)
        {
            return DATAFLASH_ERROR;
        }
        
        if (status == DATAFLASH_HEADER_VALID)
        {
            return DATAFLASH_OK;
        }
        else
        {
            /* At this point two bank header is both invalid, this means the data corrupted or has no data */
            return DATAFLASH_NOT_FORMAT;
        }
    }
}

/******************************************************************************
* Function Name: uint8_t DATAFLASH_VerifyCRC(uint16_t *p_crc)
* Description  : DATAFLASH Self Checking: memory integrity (by checking CRC)
* Arguments    : uint16_t *p_crc: output CRC value after checking if not NULL
* Return Value : Execution status
*              :    DATAFLASH_OK            Normal end
*              :    DATAFLASH_ERROR         CRC not matched
*              :                            Data corrupt (no valid header)
******************************************************************************/
uint8_t DATAFLASH_VerifyCRC(uint16_t *p_crc)
{
    uint16_t        calculated_crc;
    uint16_t        read_crc;
    uint8_t         block_buffer[DATAFLASH_WRITE_BUFFER_SIZE];
    pfdl_status_t   ret;
    uint8_t         status;
    uint8_t         verify_bank;
        
    /* Check the header to choose the bank to read */
    if (DATAFLASH_ReadHeaderStatus(0, &status) != DATAFLASH_OK)
    {
        return DATAFLASH_ERROR; /* data flash error */
    }
    
    /* Choose the bank to read data */
    if (status == DATAFLASH_HEADER_VALID)
    {
        verify_bank = 0;
    }
    else
    {
        verify_bank = 1;
    }
    
    /* Open data flash */
    ret = PFDL_Open((pfdl_descriptor_t *)&g_dataflash_descriptor);
    while (ret == PFDL_BUSY)
    {
        ret = PFDL_Handler();
    }
    if (ret != PFDL_OK)
    {
        return DATAFLASH_ERROR; /* data flash error */
    }
    
    /* Calculate the CRC */
    if (DATAFLASH_CalculateCRC(verify_bank, &block_buffer[0], &calculated_crc) != DATAFLASH_OK)
    {
        return DATAFLASH_ERROR;
    }
    
    /* Read the CRC value on dataflash header */
    if (DATAFLASH_ReadHeaderCRC(verify_bank, (uint8_t *)&read_crc) != DATAFLASH_OK)
    {
        return DATAFLASH_ERROR;
    }
    
    /* Close data flash */
    PFDL_Close();
    
    /* Compare with CRC content on dataflash */
    if (read_crc != calculated_crc)
    {
        return DATAFLASH_ERROR;
    }
    
    /* Output for user process if not NULL */
    if (p_crc != NULL)
    {
        *p_crc = calculated_crc;
    }
    
    /* Verify complete, CRC OK */    
    return DATAFLASH_OK;
}

/******************************************************************************
* Function Name: uint8_t DATAFLASH_Read(uint32_t addr, uint8_t* buf, uint16_t size)
* Description  : Read from data flash
* Arguments    : addr: Local address in data flash
*              : buf : Buffer to store the result
*              : size: Expected size want to read
* Return Value : Execution status
*              :    DATAFLASH_OK            Normal end
*              :    DATAFLASH_ERROR         Parameter error
*              :    DATAFLASH_ERROR_SIZE    Expected size and address are not suitable
******************************************************************************/
uint8_t DATAFLASH_Read(uint32_t addr, uint8_t* buf, uint16_t size)
{
    pfdl_status_t ret;
    uint8_t status;
    
    /* Check user buffer */
    if (buf == NULL)
    {
        return DATAFLASH_ERROR; /* parameter error */
    }
    
    /* Check the address */
    if (size == 0 ||
        addr + size > DATAFLASH_DEVICE_SIZE)
    {
        return DATAFLASH_ERROR_SIZE;
    }
    
    /* Offset the address, ignore the 1st byte */
    addr += DATAFLASH_HEADER_SIZE;
    
    /* Check the header to choose the bank to read */
    if (DATAFLASH_ReadHeaderStatus(0, &status) != DATAFLASH_OK)
    {
        return DATAFLASH_ERROR; /* data flash error */
    }
    
    /* Choose the bank to read data */
    if (status == DATAFLASH_HEADER_VALID)
    {
        /* Do nothing here, the address will be the same */
    }
    else
    {
        /* Offset the address to get data on the remaining bank */
        addr += DATAFLASH_INTERNAL_BLOCK_SIZE;
    }
    
    /* Open data flash */
    ret = PFDL_Open((pfdl_descriptor_t *)&g_dataflash_descriptor);
    while (ret == PFDL_BUSY)
    {
        ret = PFDL_Handler();
    }
    if (ret != PFDL_OK)
    {
        return DATAFLASH_ERROR; /* data flash error */
    }
        
    /* Read whole block */
    DATAFLASH_ASSIGN_READ_REQ((uint16_t)addr, buf, size);
    ret = PFDL_Execute(&g_data_flash_req);
    while (ret == PFDL_BUSY)
    {
        ret = PFDL_Handler();
    }
    if (ret != PFDL_OK)
    {
        return DATAFLASH_ERROR; /* data flash error */
    }

    /* Close data flash */
    PFDL_Close();
    
    return DATAFLASH_OK;    /* Read succesful */
}

/******************************************************************************
* Function Name: uint8_t DATAFLASH_Write(uint32_t addr, uint8_t* buf, uint16_t size)
* Description  : Write to data flash
* Arguments    : addr: Local address in data flash
*              : buf : Buffer to write to data flash
*              : size: Expected size (in buf) want to write
* Return Value : Execution status
*              :    DATAFLASH_OK            Normal end
*              :    DATAFLASH_ERROR         Parameter error or data flash execution error
*              :    DATAFLASH_ERROR_SIZE    Expected size and address are not suitable
******************************************************************************/
uint8_t DATAFLASH_Write(uint32_t addr, uint8_t* buf, uint16_t size)
{
    uint16_t        calculated_crc;
    uint16_t        write_block_addr;
    uint16_t        read_block_addr;
    uint32_t        index;
    uint8_t         block_buffer[DATAFLASH_WRITE_BUFFER_SIZE];
    pfdl_status_t   ret;
    uint8_t         status;
    uint8_t         i;
    uint8_t         write_bank;
    uint8_t         read_bank;
    
    /* Check user buffer */
    if (buf == NULL)
    {
        return DATAFLASH_ERROR; /* parameter error */
    }
    
    /* check the address */
    if (size == 0 ||
        addr + size > DATAFLASH_DEVICE_SIZE)
    {
        return DATAFLASH_ERROR_SIZE;
    }
    
    /* Offset the address, ignore the 1st byte */
    addr += DATAFLASH_HEADER_SIZE;
    
    /* Check the header to choose the bank to read */
    if (DATAFLASH_ReadHeaderStatus(0, &status) != DATAFLASH_OK)
    {
        return DATAFLASH_ERROR; /* data flash error */
    }
    
    /* Choose the bank to read data */
    if (status == DATAFLASH_HEADER_VALID)
    {
        /* The write address will be on the other bank */
        addr += DATAFLASH_INTERNAL_BLOCK_SIZE;
        write_block_addr = DATAFLASH_INTERNAL_BLOCK_SIZE;
        read_block_addr = 0;
        write_bank = 1;
        read_bank = 0;
    }
    else
    {
        write_block_addr = 0;
        read_block_addr = DATAFLASH_INTERNAL_BLOCK_SIZE;
        write_bank = 0;
        read_bank = 1;
    }
    
    /* Open data flash */
    ret = PFDL_Open((pfdl_descriptor_t *)&g_dataflash_descriptor);
    while (ret == PFDL_BUSY)
    {
        ret = PFDL_Handler();
    }
    if (ret != PFDL_OK)
    {
        return DATAFLASH_ERROR; /* data flash error */
    }
    
    /* 
     * New writing process:
     *  Erase the bank to be written in (in case it not)
     *  Loop the write process below:
     *      Read in block from start address
     *      Modified if need
     *      Write directly to the bank
     *  Calculate the CRC and write to header crc word
     *  Verify the written bank (internal process by pfdl)
     *  Mark bank valid by write to header status byte
     *  Verify the written header status byte (internal process by pfdl)
     *  Erase the other bank to mark it as invalid
     *  Finished
    */
    
    /* Erase bank */
    if (DATAFLASH_BankErase(write_bank) != DATAFLASH_OK)
    {
        return DATAFLASH_ERROR;
    }
    
    /* Write process */
    for (i=0 ; i< DATAFLASH_WRITE_LOOP_TIME; i++)
    {
        /* Read the content from previous content */
        DATAFLASH_InternalRead(read_block_addr, block_buffer, DATAFLASH_WRITE_BUFFER_SIZE);
                
        /* Still has data to modify? */
        if (size > 0)
        {
            /* Need to modified? */
            if (addr >= write_block_addr &&
                addr <= (write_block_addr + DATAFLASH_WRITE_BUFFER_SIZE) )
            {
                for (index=addr; index < write_block_addr + DATAFLASH_WRITE_BUFFER_SIZE; index++)
                {
                    block_buffer[index%DATAFLASH_WRITE_BUFFER_SIZE] = *buf;
                    addr++;
                    buf++;
                    size--;
                    
                    if (size == 0)
                    {
                        break;
                    }
                }
            }
        }
        
        /* Write content to other bank */
        if (i == 0)
        {
            /* In first block, exclude the header, so default reading will be invalid */
            DATAFLASH_ASSIGN_WRITE_REQ( write_block_addr + DATAFLASH_HEADER_SIZE, 
                                        block_buffer + DATAFLASH_HEADER_SIZE, 
                                        DATAFLASH_WRITE_BUFFER_SIZE - DATAFLASH_HEADER_SIZE);
        }
        else
        {
            /* Other, write as usual */
            DATAFLASH_ASSIGN_WRITE_REQ(write_block_addr, block_buffer, DATAFLASH_WRITE_BUFFER_SIZE);
        }
        ret = PFDL_Execute(&g_data_flash_req);
        while (ret == PFDL_BUSY)
        {
            ret = PFDL_Handler();
        }
        if (ret != PFDL_OK)
        {
            return DATAFLASH_ERROR; /* data flash error */
        }
        
        write_block_addr += DATAFLASH_WRITE_BUFFER_SIZE;
        read_block_addr += DATAFLASH_WRITE_BUFFER_SIZE;
        
        if(dataflash_wd_toggle_callback != 0)
        {
            dataflash_wd_toggle_callback();
        }
    }
    
    /* Get the calculated CRC */
    if (DATAFLASH_CalculateCRC(write_bank, &block_buffer[0], &calculated_crc) != DATAFLASH_OK)
    {
        return DATAFLASH_ERROR;
    }
    /* Update the CRC to header */
    DATAFLASH_ASSIGN_WRITE_REQ((write_bank * DATAFLASH_INTERNAL_BLOCK_SIZE) + DATAFLASH_HEADER_CRC_ADDR, 
                                (uint8_t *)&calculated_crc, 
                                DATAFLASH_HEADER_CRC_SIZE);
    ret = PFDL_Execute(&g_data_flash_req);
    while (ret == PFDL_BUSY)
    {
        ret = PFDL_Handler();
    }
    if (ret != PFDL_OK)
    {
        return DATAFLASH_ERROR; /* data flash error */
    }
    
    /* Writing finish, verify bank content electrically */
    DATAFLASH_ASSIGN_VERIFY_REQ((write_bank * DATAFLASH_INTERNAL_BLOCK_SIZE)+1, DATAFLASH_INTERNAL_BLOCK_SIZE - 1);
    ret = PFDL_Execute(&g_data_flash_req);
    while (ret == PFDL_BUSY)
    {
        ret = PFDL_Handler();
    }
    if (ret != PFDL_OK)
    {
        return DATAFLASH_ERROR; /* data flash error */
    }
    
    /* Mark header valid */
    status = DATAFLASH_HEADER_VALID;
    DATAFLASH_ASSIGN_WRITE_REQ((write_bank * DATAFLASH_INTERNAL_BLOCK_SIZE), &status, 1);
    ret = PFDL_Execute(&g_data_flash_req);
    while (ret == PFDL_BUSY)
    {
        ret = PFDL_Handler();
    }
    if (ret != PFDL_OK)
    {
        return DATAFLASH_ERROR; /* data flash error */
    }
    
    /* Verify the written header */
    DATAFLASH_ASSIGN_VERIFY_REQ((write_bank * DATAFLASH_INTERNAL_BLOCK_SIZE), 1);
    ret = PFDL_Execute(&g_data_flash_req);
    while (ret == PFDL_BUSY)
    {
        ret = PFDL_Handler();
    }
    if (ret != PFDL_OK)
    {
        return DATAFLASH_ERROR; /* data flash error */
    }
    
    /* Erase the old block content */
    if (DATAFLASH_BankErase(read_bank) != DATAFLASH_OK)
    {
        return DATAFLASH_ERROR;
    }
    
    /* Close data flash */
    PFDL_Close();
    
    return DATAFLASH_OK;    /* Write succesful */
    
}


/******************************************************************************
* Function Name: uint8_t DATAFLASH_BankErase(uint8_t bank_no)
* Description  : Write to data flash
* Arguments    : uint8_t bank_no: choose bank to erase
* Return Value : Execution status
*              :    DATAFLASH_OK            Normal end
*              :    DATAFLASH_ERROR         Parameter error or data flash execution error
*              :    DATAFLASH_ERROR_SIZE    Expected size and address are not suitable
******************************************************************************/
uint8_t DATAFLASH_BankErase(uint8_t bank_no)
{
    pfdl_status_t   ret;
    
    /* Check block index, is out of range? */
    if (bank_no >=  (DATAFLASH_INTERNAL_DEVICE_SIZE /DATAFLASH_INTERNAL_BLOCK_SIZE))
    {
        return DATAFLASH_ERROR_SIZE;
    }
    
    /* Open data flash */
    ret = PFDL_Open((pfdl_descriptor_t *)&g_dataflash_descriptor);
    while (ret == PFDL_BUSY)
    {
        ret = PFDL_Handler();
    }
    
    if (ret != PFDL_OK)
    {
        return DATAFLASH_ERROR; /* data flash error */
    }
    
    /* Erase that whole block */
    DATAFLASH_ASSIGN_ERASE_REQ(bank_no);
    ret = PFDL_Execute(&g_data_flash_req);
    while (ret == PFDL_BUSY)
    {
        ret = PFDL_Handler();
    }
    if (ret != PFDL_OK)
    {
        return DATAFLASH_ERROR; /* data flash error */
    }
    if(dataflash_wd_toggle_callback != 0)
    {
        dataflash_wd_toggle_callback();
    }
    /* Close data flash */
    PFDL_Close();
    
    return DATAFLASH_OK;    /* Write succesful */
}
    
    
/******************************************************************************
* Function Name: uint8_t DATAFLASH_Format(void)
* Description  : Erase Dataflash contents
* Arguments    : None
* Return Value : Execution status
*              :    DATAFLASH_OK            Normal end
*              :    DATAFLASH_ERROR         Parameter error or data flash execution error
*              :    DATAFLASH_ERROR_SIZE    Expected size and address are not suitable
******************************************************************************/
uint8_t DATAFLASH_Format(void)
{
    pfdl_status_t   ret;
    uint8_t u8DFTemp, u8Bank_Nos = DATAFLASH_INTERNAL_DEVICE_SIZE /DATAFLASH_INTERNAL_BLOCK_SIZE;

    for ( u8DFTemp = 0; u8DFTemp < u8Bank_Nos; u8DFTemp++)
    {
            ret = (pfdl_status_t)DATAFLASH_BankErase(u8DFTemp);
            if (DATAFLASH_OK != ret)
            {
                    return ret;
            }
    }
    
    return DATAFLASH_OK;    /* Write succesful */
}
