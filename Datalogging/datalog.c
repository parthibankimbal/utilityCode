#include "datalog.h"
#include "eeprom_storage.h"

#define NON_ROLL_OVER 0x8000//TODO: Rakesh sept 2022 case for latch weld
pre_datalog_user_callback log_energies_callback = 0;
address_return_type address_return;
BLOCK_ENERGY_AVG_DATA_LOG avg_blk_load_values;
u_st_profiles u_s_profile;
tag_dlms_profile_type_t scan_event_type;
ONE_MONTH_ENERGY_DATA_LOG	g_Last_bill_log;
#ifdef UTILITY_JNK
uint64_t                        g_bill_kwh_h2;
uint64_t                        g_bill_kwh_h3;
uint32_t                        g_bill_mdkw_h2;
uint32_t                        g_bill_mdkw_h3;
uint32_t                        g_bill_mdkw_epoch_h2;
uint32_t                        g_bill_mdkw_epoch_h3;
#endif

void average_lp_values(void);

#define FAR_PTR_ __far
/* Storage memory map */
const FAR_PTR_ DLMS_STORAGE g_memory_map_event_table[][3] =
{
    /* Voltage related events */
    {
        { STORAGE_EEPROM_DLMS_EVENT_INFO_VOLTAGE_ADDR,            STORAGE_EEPROM_DLMS_EVENT_INFO_VOLTAGE_SIZE       },
        { STORAGE_EEPROM_DLMS_EVENT_VOLTAGE_TABLE_ADDR,           STORAGE_EEPROM_DLMS_EVENT_VOLTAGE_TABLE_SIZE      },
        { (NON_ROLL_OVER * 0 | EVENT_MAX_ENTRIES_VOLTAGE),        sizeof(ONE_EVENT_ENERGY_DATA_LOG)                 },
    },
    /* Current related events */
    {
        { STORAGE_EEPROM_DLMS_EVENT_INFO_CURRENT_ADDR,            STORAGE_EEPROM_DLMS_EVENT_INFO_CURRENT_SIZE       },
        { STORAGE_EEPROM_DLMS_EVENT_CURRENT_TABLE_ADDR,           STORAGE_EEPROM_DLMS_EVENT_CURRENT_TABLE_SIZE      },
        { (NON_ROLL_OVER * 0 | EVENT_MAX_ENTRIES_CURRENT),        sizeof(ONE_EVENT_ENERGY_DATA_LOG)                 },
    },
    /* Power related events */
    {
        { STORAGE_EEPROM_DLMS_EVENT_INFO_POWER_ADDR,              STORAGE_EEPROM_DLMS_EVENT_INFO_POWER_SIZE         },
        { STORAGE_EEPROM_DLMS_EVENT_POWER_TABLE_ADDR,             STORAGE_EEPROM_DLMS_EVENT_POWER_TABLE_SIZE        },
        { (NON_ROLL_OVER * 0 | EVENT_MAX_ENTRIES_POWER),          sizeof(ONE_EVENT_ID_LOG)                          },
    },
    /* Transaction related events */
    {
        { STORAGE_EEPROM_DLMS_EVENT_INFO_TRANS_ADDR,              STORAGE_EEPROM_DLMS_EVENT_INFO_TRANS_SIZE         },
        { STORAGE_EEPROM_DLMS_EVENT_TRANS_TABLE_ADDR,             STORAGE_EEPROM_DLMS_EVENT_TRANS_TABLE_SIZE        },
        { (NON_ROLL_OVER * 0 | EVENT_MAX_ENTRIES_TRANS),          sizeof(ONE_EVENT_ID_LOG)                          },
    },
    /* Other events */
    {
        { STORAGE_EEPROM_DLMS_EVENT_INFO_OTHER_ADDR,              STORAGE_EEPROM_DLMS_EVENT_INFO_OTHER_SIZE         },
        { STORAGE_EEPROM_DLMS_EVENT_OTHER_TABLE_ADDR,             STORAGE_EEPROM_DLMS_EVENT_OTHER_TABLE_SIZE        },
        { (NON_ROLL_OVER * 0 | EVENT_MAX_ENTRIES_OTHER),          sizeof(ONE_EVENT_ENERGY_DATA_LOG)                 },
    },
    /* Non-Roller events */
    {
        { STORAGE_EEPROM_DLMS_EVENT_INFO_NO_ROLL_ADDR,            STORAGE_EEPROM_DLMS_EVENT_INFO_NO_ROLL_SIZE       },
        { STORAGE_EEPROM_DLMS_EVENT_NO_ROLL_TABLE_ADDR,           STORAGE_EEPROM_DLMS_EVENT_NO_ROLL_TABLE_SIZE      },
        { (NON_ROLL_OVER * 1 | EVENT_MAX_ENTRIES_NON_ROLLER),     sizeof(ONE_EVENT_ID_LOG)                          },
    },
    /* Control events */
    {
        { STORAGE_EEPROM_DLMS_EVENT_INFO_CON_DISCON_ADDR,         STORAGE_EEPROM_DLMS_EVENT_INFO_CON_DISCON_SIZE    },
        { STORAGE_EEPROM_DLMS_EVENT_CON_DISCON_TABLE_ADDR,        STORAGE_EEPROM_DLMS_EVENT_CON_DISCON_TABLE_SIZE   },
        { (NON_ROLL_OVER * 0 | EVENT_MAX_ENTRIES_CONTROL),        sizeof(ONE_EVENT_ID_LOG)                          },
    },
    /* Storage info billing table address */
    {
        { STORAGE_EEPROM_DLMS_BILLING_INFO_ADDR,                  STORAGE_EEPROM_DLMS_BILLING_INFO_SIZE             },
        { STORAGE_EEPROM_DLMS_BILLING_TABLE_ADDR,                 STORAGE_EEPROM_DLMS_BILLING_TABLE_SIZE            },
        { (NON_ROLL_OVER * 0 | BILLING_MAX_ENTRIES),              sizeof(ONE_MONTH_ENERGY_DATA_LOG)                 },
    },
    /* Memory map for daily load table */
    {
        { STORAGE_EEPROM_DLMS_DAILY_INFO_ADDR,                    STORAGE_EEPROM_DLMS_DAILY_INFO_SIZE               },
        { STORAGE_EEPROM_DLMS_DAILY_TABLE_ADDR,                   STORAGE_EEPROM_DLMS_DAILY_TABLE_SIZE              },
        { (NON_ROLL_OVER * 0 | DAILYLOAD_MAX_ENTRIES),            sizeof(ONE_DAILY_ENERGY_DATA_LOG)                 },
    },
    /* Storage info block load */
    {
        { STORAGE_EEPROM_DLMS_BLK_LOAD_SURVEY_INFO_ADDR,          STORAGE_EEPROM_DLMS_BLK_LOAD_SURVEY_INFO_SIZE     },
        { STORAGE_EEPROM_DLMS_BLK_LOAD_TABLE_ADDR,                STORAGE_EEPROM_DLMS_BLK_LOAD_TABLE_SIZE           },
        { (NON_ROLL_OVER * 0 | BLOCKLOAD_MAX_ENTRIES),            sizeof(ONE_BLOCK_ENERGY_DATA_LOG)                 },
    }
};


float fl_abs(float value)
{
  return value * ((uint8_t)(value > 0) - (uint8_t)(value < 0));
}
/******************************************************************************
* Function Name : r_dlms_buffer_getlog
* Interface     : void r_dlms_buffer_getlog(
*               :     void
*               : );
* Description   : Collect the data from EEPROM and return the struct to DLMS OBIS
* Arguments     :
* Function Calls:
* Return Value  :
******************************************************************************/
uint8_t r_dlms_buffer_getlog(
    uint8_t* p_log,
    uint16_t log_size,
    uint16_t entry_index,
    DLMS_STORAGE FAR_PTR_* p_storage_table_maps
)
{
    uint32_t eeprom_addr;

    buffer_info_t buf_info = {0};

    uint32_t offset;

    /**************************************************************************************
     *
     * STORAGE MEMORY MAP FOR TABLE
     *
     *          [buffer_info_t info]
     *          [uint16_t write_pos         ]
     *          [uint16_t read_pos          ]
     *          [uint16_t max_entries       ]
     *          [uint16_t entries_in_used   ]
     *          [uint8_t  is_non_roller     ]
     *
     *          [Table]
     *          [0][Entry 0]
     *          [1][Entry 1]
     *          [2][Entry 2] <- Current_Entry | Entries_in_used
     *
     *          [N][Entry N] <- Max_Entries
     *
     ***************************************************************************************/

     /* Check buffer information */
    {
        eeprom_addr = p_storage_table_maps[0].address;
#ifdef RETURN_ERROR
        if (
#endif
         read_page_eeprom(eeprom_addr, (uint8_t*)&buf_info, sizeof(buffer_info_t))
#ifndef RETURN_ERROR
           ;
#endif
#ifdef RETURN_ERROR
          != EPR_OK)
        {
            return RLT_ERR; /* Write error */
        }
#endif
        if ((buf_info.entries_in_used < buf_info.max_entries) && (buf_info.read_pos != 0))
        {
            buf_info.read_pos = 0;
#ifdef RETURN_ERROR
            if (
#endif
             write_page_eeprom(eeprom_addr, (uint8_t*)&buf_info, sizeof(buffer_info_t))
#ifndef RETURN_ERROR
               ;
#endif
#ifdef RETURN_ERROR
              != EPR_OK)
            {
                return RLT_ERR; /* Write error */
            }
#endif
        }
        if ((buf_info.read_pos != buf_info.write_pos) && (buf_info.entries_in_used == buf_info.max_entries))
        {
            buf_info.read_pos = buf_info.write_pos;
#ifdef RETURN_ERROR
            if (
#endif
             write_page_eeprom(eeprom_addr, (uint8_t*)&buf_info, sizeof(buffer_info_t))
#ifndef RETURN_ERROR
               ;
#endif
#ifdef RETURN_ERROR
              != EPR_OK)
            {
                return RLT_ERR; /* Write error */
            }
#endif
        }
    }

    /**************************************************************************************
     *
     *  Circular buffer
     *
     *                        read_pos          write_pos                     max
     *              +---------+-----+-----+-----+-----+---------+---------+---------+
     *      +-----> | <empty> |  o  |  o  |  o  |  o  | <empty> | <empty> | <empty> | ----+
     *      |       +---------+-----+-----+-----+-----+---------+---------+---------+     |
     *      |                    0     1     2     3                                      |
     *      +-----------------------------------------------------------------------------+
     *
     *          o mark the data that was logged
     *          0, 1, 2, 3 is entry index (from DLMS OBIS)
     *          4 is entries in used
     *
     ***************************************************************************************/

     /* Calculate memory address */
    if(buf_info.entries_in_used == 0)
    {
        memset(p_log, 0, log_size);
        return RLT_ERR;
    }
    offset = (buf_info.read_pos + entry_index) % buf_info.max_entries;

    /* Get log data */
    {
        eeprom_addr = p_storage_table_maps[1].address;      /* Point to ahead of array */
        eeprom_addr += (uint32_t)offset * log_size;                    /* Add offset */
        if (address_return != 0)
        {
          address_return(eeprom_addr);
        }
#ifdef RETURN_ERROR
        if (
#endif
         read_page_eeprom(eeprom_addr, (uint8_t*)p_log, log_size)
#ifndef RETURN_ERROR
           ;
#endif
#ifdef RETURN_ERROR
          != EPR_OK)
        {
            return RLT_ERR; /* Write error */
        }
#endif
    }

    return RLT_SUCCESS; // OK
}

/******************************************************************************
* Function Name : r_dlms_buffer_addlog
* Interface     : void r_dlms_buffer_addlog(
*               :    uint8_t         * p_log,
*               :    int16_t           log_size,
*               :    DLMS_STORAGE    * p_storage_table_map
*               : );
* Description   : This function will add data of p_log into EEPROM buffer.
*				: After adding, the entries_in_used and write_pos will increase
*				: Rollover happend when entries_in_used > max_entries
* Arguments     :
*               : p_log    				: Buffer that contain data of entry
*               : log_size 				: Size of p_log data
*               : p_storage_table_map 	: Address of EEPROM
*               : 			It shall include Address of
*               : 					[buffer_info]
*               : 					[Data table]
* Function Calls:
* Return Value  : RLT_SUCCESS (0) 	: Result success
*   			: Other 	  (!=0) : The logging has (an) error(s).
******************************************************************************/
uint8_t r_dlms_buffer_addlog(
    uint8_t* p_log,
    uint16_t log_size,
    DLMS_STORAGE FAR_PTR_* p_storage_table_map
)
{
    uint32_t eeprom_addr;

    buffer_info_t buf_info = {0};

    /**************************************************************************************
     *
     * STORAGE MEMORY MAP FOR TABLE
     *
     *      [buffer_info_t info]
     *              [uint16_t write_pos     	]
     *              [uint16_t read_pos   		]
     *              [uint16_t max_entries      	]
     *              [uint16_t entries_in_used 	]
     *              [uint8_t  is_non_roller     ]
     *
     *      [Table]
     *          [0][Entry 0]
     *          [1][Entry 1]
     *          [2][Entry 2] <- Current_Entry | Entries_in_used
     *
     *          [N][Entry N] <- Max_Entries
     *
     ***************************************************************************************/

    if ((p_log == NULL) ||
        (p_storage_table_map == NULL))
    {
        return RLT_ERR_NULL;
    }

    /* Check buffer information */
    {
        eeprom_addr = p_storage_table_map[0].address;
#ifdef RETURN_ERROR
        if (
#endif
          read_page_eeprom(eeprom_addr,
            (uint8_t*)&buf_info,
            sizeof(buffer_info_t))
#ifndef RETURN_ERROR
          ;
#endif
#ifdef RETURN_ERROR
          != EPR_OK)
        {
            return RLT_ERR; /* Write error */
        }
#endif
        buf_info.max_entries = p_storage_table_map[2].address & 0x7FFF;
        if ((buf_info.entries_in_used < buf_info.max_entries) && (buf_info.read_pos != 0))
        {
            buf_info.read_pos = 0;
#ifdef RETURN_ERROR
            if (
#endif
              write_page_eeprom(eeprom_addr, (uint8_t*)&buf_info, sizeof(buffer_info_t))
#ifndef RETURN_ERROR
                ;
#endif
#ifdef RETURN_ERROR
              != EPR_OK)
            {
                return RLT_ERR; /* Write error */
            }
#endif
        }
        if ((buf_info.read_pos != buf_info.write_pos) && (buf_info.entries_in_used == buf_info.max_entries))
        {
            buf_info.read_pos = buf_info.write_pos;
#ifdef RETURN_ERROR
            if (
#endif
             write_page_eeprom(eeprom_addr, (uint8_t*)&buf_info, sizeof(buffer_info_t))
#ifndef RETURN_ERROR
               ;
#endif
#ifdef RETURN_ERROR
              != EPR_OK)
            {
                return RLT_ERR; /* Write error */
            }
#endif
        }
    }

    /**************************************************************************************
     *
     *   Circular FIFO buffer
     *
     *                        read_pos          write_pos                     max
     *              +---------+-----+-----+-----+-----+---------+---------+---------+
     *      +-----> | <empty> |  o  |  o  |  o  |  o  | <empty> | <empty> | <empty> | ----+
     *      |       +---------+-----+-----+-----+-----+---------+---------+---------+     |
     *      |                    0     1     2     3                                      |
     *      +-----------------------------------------------------------------------------+
     *
     *          o mark logged data
     *          0, 1, 2, 3 is entry index (from DLMS OBIS)
     *          4 is entries in used
     *
     ***************************************************************************************/

     /* For non-roll over buffer */
    {
        if (buf_info.is_non_roller == 1)
        {
//            if (buf_info.write_pos >= buf_info.max_entries)
//            {
//                /* Buffer is full, It's just 1 entry */
//                return RLT_ERR; // OK
//            }
            if (buf_info.entries_in_used >= buf_info.max_entries)
            {
                return RLT_SUCCESS;
            }
        }
    }

    /* Log data */
    {
        eeprom_addr = p_storage_table_map[1].address;      /* Point to ahead of array */
        eeprom_addr += (uint32_t)buf_info.write_pos * log_size;       /* Add offset */
        if (address_return != 0)//anwar
        {
          address_return(eeprom_addr);
        }

#ifdef RETURN_ERROR
        if (
#endif
            write_page_eeprom(eeprom_addr,
            (uint8_t*)p_log,
            log_size)
#ifndef RETURN_ERROR
              ;
#endif
#ifdef RETURN_ERROR
          != EPR_OK)
        {
            return RLT_ERR; /* Write error */
        }
#endif
    }

    /* Rotate down the current entry position */
    buf_info.write_pos++;
    if (buf_info.write_pos >= buf_info.max_entries)
    {
        buf_info.write_pos = 0;
    }

    /* Count up entries in used */
    if (buf_info.entries_in_used < buf_info.max_entries)
    {
        buf_info.entries_in_used++;
    }
    else
    {
        /* Count up number of entries are used */
        buf_info.entries_in_used = buf_info.max_entries;
        buf_info.read_pos++;
        if (buf_info.read_pos >= buf_info.max_entries)
        {
            buf_info.read_pos = 0;
        }
    }

    /* Backup storage buffer info */
    eeprom_addr = p_storage_table_map[0].address;
#ifdef RETURN_ERROR
    if (
#endif
     write_page_eeprom(eeprom_addr,
        (uint8_t*)&buf_info,
        sizeof(buffer_info_t))
#ifndef RETURN_ERROR
      ;
#endif
#ifdef RETURN_ERROR
      != EPR_OK)
    {
        return RLT_ERR; /* Write error */
    }
#endif

    return RLT_SUCCESS; // OK
}



/******************************************************************************
*Function Name : R_DLMS_DataLog_GetEntry
* Interface : void R_DLMS_DataLog_GetEntry(
    *:     uint16_t datalog_id, void* pbuff, uint16_t entry_index
    * : );
*Description   :
*Arguments :
    *Function Calls :
*Return Value :
******************************************************************************/
void R_DLMS_DataLog_GetEntry(tag_dlms_profile_type_t datalogProfile_id, uint8_t* pbuff, uint16_t entry_index)
{
    DLMS_STORAGE FAR_PTR_* p_storage = (DLMS_STORAGE FAR_PTR_*)g_memory_map_event_table[datalogProfile_id];
    uint16_t entry_size = (uint16_t)p_storage[2].length;

    
    r_dlms_buffer_getlog(
        (uint8_t*)pbuff,
        entry_size,
        entry_index,
        p_storage
    );
}

/******************************************************************************
* Function Name : R_DLMS_DataLog_SetEntry
* Interface     : void R_DLMS_DataLog_SetEntry(
*               :     tag_dlms_profile_type_t datalogProfile_id, void * pbuff,
*               : );
* Description   :
* Arguments     :
* Function Calls:
* Return Value  :
******************************************************************************/
void R_DLMS_DataLog_SetEntry(tag_dlms_profile_type_t datalogProfile_id, uint8_t* pbuff)
{
    DLMS_STORAGE FAR_PTR_* p_storage = (DLMS_STORAGE FAR_PTR_*)g_memory_map_event_table[datalogProfile_id];
    uint16_t entry_size = (uint16_t)p_storage[2].length;
    
    r_dlms_buffer_addlog(
        (uint8_t*)pbuff,
        entry_size,
        p_storage
    );
    return;
}

range_entries find_entries_by_range(uint32_t epoch_from, uint32_t epoch_to, tag_dlms_profile_type_t datalogProfile_id)
{
  unsigned long int tmp_long;
  int16_t f_index, l_index, m_index;
  uint16_t entries_in_use = get_profile_entry_in_use(datalogProfile_id);
  range_entries entries;

  if (epoch_to < epoch_from)
  {
    tmp_long = epoch_to;
    epoch_to = epoch_from;
    epoch_from = tmp_long;
  }

  f_index = 0;
  l_index = entries_in_use;
  m_index = (f_index + l_index) / 2;

  while (f_index <= l_index)
  {
    R_DLMS_DataLog_GetEntry(datalogProfile_id, (uint8_t*)&u_s_profile.billing, m_index);
    tmp_long = u_s_profile.billing.u32_epoch;

    if (tmp_long > epoch_from)
    {
      l_index = m_index - 1;
    }
    else if (tmp_long < epoch_from)
    {
      f_index = m_index + 1;
    }
    else
    {
      f_index = m_index;
      break;
    }

    m_index = (f_index + l_index) / 2;
  }

  entries.from = f_index;
  entries.num = 0;

  f_index = 0;
  l_index = entries_in_use;
  m_index = (f_index + l_index) / 2;

  while (f_index <= l_index)
  {
    R_DLMS_DataLog_GetEntry(datalogProfile_id, (uint8_t*)&u_s_profile.billing, m_index);
    tmp_long = u_s_profile.billing.u32_epoch;

    if ((tmp_long > epoch_to) || (tmp_long == 0))
    {
      l_index = m_index - 1;
    }
    else if (tmp_long < epoch_to)
    {
      f_index = m_index + 1;
    }
    else
    {
      l_index = m_index;
      break;
    }

    m_index = (f_index + l_index) / 2;
  }

  if ((entries.from > entries_in_use) /*|| (l_index < 1)*/)
  {
    entries.from = 0;
    entries.num = 0;
  }
  else
  {
    if (l_index > entries.from)
    {
      entries.num = (l_index - entries.from) + 1;
    }
    else
    {
      entries.num = 1;
    }
    if(entries.num > entries_in_use)
    {
      entries.num = entries_in_use;
    }
  }
  return entries;
}


uint8_t cal_zone = 0;
void check_and_log_data(st_RTC_time* RTC_time)
{
  //static uint8_t cal_zone = 0;
  //if(cal_zone != 0)
  {
    log_md_data(RTC_time, cal_zone);
  }
  cal_zone = activity_calendar_polling_process(RTC_time->epoch);//TODO: Rakesh force activation to check
  log_load_profile(RTC_time);
  log_daily_load_profile(RTC_time);
  log_billing_profile(RTC_time, 0);
}

void average_lp_values(void)//per ssecond call.
{
    float temp_flt;
    uint8_t loop;
    temp_flt = (avg_blk_load_values.fl_volt * avg_blk_load_values.u16_volt_counter++) + get_ph_voltage();
    avg_blk_load_values.fl_volt = temp_flt / avg_blk_load_values.u16_volt_counter;
    
    temp_flt = (avg_blk_load_values.fl_current_phase * avg_blk_load_values.u16_current_counter_phase++) + get_signed_ph_current();
    avg_blk_load_values.fl_current_phase = temp_flt / avg_blk_load_values.u16_current_counter_phase;
    
    temp_flt = (avg_blk_load_values.fl_current_neutral * avg_blk_load_values.u16_current_counter_neutral++) + get_signed_neu_current();
    avg_blk_load_values.fl_current_neutral = temp_flt / avg_blk_load_values.u16_current_counter_neutral;
    
    temp_flt = ((avg_blk_load_values.fl_pf * avg_blk_load_values.u16_pf_counter++) + fl_abs(get_signed_pf()));
    avg_blk_load_values.fl_pf = temp_flt / avg_blk_load_values.u16_current_counter_phase;
}

void store_average_lp_values(void)
{
  write_page_eeprom(STORAGE_EEPROM_DLMS_BLK_LOAD_AVG_VALUES_ADDR, (uint8_t*)&avg_blk_load_values, STORAGE_EEPROM_DLMS_BLK_LOAD_AVG_VALUES_SIZE);
}

void read_average_lp_values(void)
{
  read_page_eeprom(STORAGE_EEPROM_DLMS_BLK_LOAD_AVG_VALUES_ADDR, (uint8_t*)&avg_blk_load_values, STORAGE_EEPROM_DLMS_BLK_LOAD_AVG_VALUES_SIZE);
}

void log_load_profile(st_RTC_time* RTC_time)
{
  uint16_t capture_period;
  uint8_t loop;
  ONE_BLOCK_BACKUP_ENERGY_DATA_LOG load_bkp_profile;
  ONE_BLOCK_ENERGY_DATA_LOG load_profile;
  stTime_struct temp_time;
  
  /* Block load profile */
  capture_period = (uint16_t)from_eeprom(STORAGE_EEPROM_DLMS_BLK_LOAD_CAP_PERIOD_ADDR, STORAGE_EEPROM_DLMS_BLK_LOAD_CAP_PERIOD_SIZE);
  load_bkp_profile.u32_epoch = from_eeprom(STORAGE_EEPROM_DLMS_BLK_LOAD_BACKUP_ADDR + offsetof(ONE_BLOCK_ENERGY_DATA_LOG, u32_epoch), member_size(ONE_BLOCK_ENERGY_DATA_LOG, u32_epoch));
  
  if(load_bkp_profile.u32_epoch == 0)
  {
    load_bkp_profile.u32_epoch = ((RTC_time->epoch/86400)*86400)/capture_period;
  }
  
  if(load_bkp_profile.u32_epoch != RTC_time->epoch/capture_period)
  {
    st_tamper.Trigger_Byte |= _BV(LOAD_PROFILE_BIT);
    
    read_page_eeprom(STORAGE_EEPROM_DLMS_BLK_LOAD_BACKUP_ADDR, (uint8_t*)&load_bkp_profile, STORAGE_EEPROM_DLMS_BLK_LOAD_BACKUP_SIZE);
    load_bkp_profile.u32_epoch++;
    load_profile.u32_epoch = load_bkp_profile.u32_epoch*capture_period;
    
    if(get_profile_entry_in_use(PROFILE_TYPE_BLOCKLOAD) != 0)
    {//REVIEW: Rakesh, common location.
         if(log_energies_callback != 0)
         {
           log_energies_callback();
         }

#ifdef SINGLE_PHASE_METER
         if (get_energy(Active_Imp) >= load_bkp_profile.u64_block_energy_val[LP_Active_Imp])
         {
           load_profile.u32_block_energy_val[LP_Active_Imp] = get_energy(Active_Imp) - load_bkp_profile.u64_block_energy_val[LP_Active_Imp];
         }
         else
         {
           load_profile.u32_block_energy_val[LP_Active_Imp] = 0;
         }
         if (get_energy(Active_Exp) >= load_bkp_profile.u64_block_energy_val[LP_Active_Exp])
         {
           load_profile.u32_block_energy_val[LP_Active_Exp] = get_energy(Active_Exp) - load_bkp_profile.u64_block_energy_val[LP_Active_Exp];
         }
         else
         {
           load_profile.u32_block_energy_val[LP_Active_Exp];
         }
         if (get_energy(Apparent_Imp) >= load_bkp_profile.u64_block_energy_val[LP_Apparent_Imp])
         {
           load_profile.u32_block_energy_val[LP_Apparent_Imp] = get_energy(Apparent_Imp) - load_bkp_profile.u64_block_energy_val[LP_Apparent_Imp];
         }
         else
         {
           load_profile.u32_block_energy_val[LP_Apparent_Imp] = 0;
         }
         if (get_energy(Apparent_Imp) >= load_bkp_profile.u64_block_energy_val[LP_Apparent_Imp])
         {
           load_profile.u32_block_energy_val[LP_Apparent_Exp] = get_energy(Apparent_Exp) - load_bkp_profile.u64_block_energy_val[LP_Apparent_Exp];
         }
         else
         {
           load_profile.u32_block_energy_val[LP_Apparent_Exp] = 0;
         }

         load_profile.u16_volt = (uint16_t)(avg_blk_load_values.fl_volt*100);
         load_profile.u16_current_phase = (uint16_t)(avg_blk_load_values.fl_current_phase*100);
         load_profile.u16_current_neutral = (uint16_t)(avg_blk_load_values.fl_current_neutral*100);
//         load_profile.u8_pf = (uint8_t)avg_blk_load_values.fl_pf;

         /* backup latest energies */
         load_bkp_profile.u64_block_energy_val[LP_Active_Imp] = get_energy(Active_Imp);
         load_bkp_profile.u64_block_energy_val[LP_Active_Exp] = get_energy(Active_Exp);
         load_bkp_profile.u64_block_energy_val[LP_Apparent_Imp] = get_energy(Apparent_Imp);
         load_bkp_profile.u64_block_energy_val[LP_Apparent_Exp] = get_energy(Apparent_Exp);
#else
         for(loop = 0; loop < 8; loop++)
         {
             if(get_energy(loop) >= load_bkp_profile.u64_block_energy_val[loop])
             {
               load_profile.u32_block_energy_val[loop] = (uint32_t)(get_energy(loop) - load_bkp_profile.u64_block_energy_val[loop]);
             }
             else
             {
               load_profile.u32_block_energy_val[loop] = (uint32_t)0;  
             }
         }
         
         for(loop = 0; loop < 3; loop++)
         {//seperate function for avg.
             load_profile.u16_volt[loop] = (uint16_t)avg_blk_load_values.fl_volt[loop];
             load_profile.u32_current[loop] = (uint32_t)avg_blk_load_values.fl_current[loop];
             load_profile.u8_pf[loop] = (uint8_t)avg_blk_load_values.fl_pf[loop];
         }
         /* backup latest energies */
         for(loop = 0; loop < 8; loop++)
         {
             load_bkp_profile.u64_block_energy_val[loop] = get_energy(loop);
         }
#endif
    }
    else
    {
        memset((uint8_t*)&load_profile + sizeof(load_profile.u32_epoch), 0, sizeof(ONE_BLOCK_ENERGY_DATA_LOG) - sizeof(load_profile.u32_epoch));
    }
    R_DLMS_DataLog_SetEntry(PROFILE_TYPE_BLOCKLOAD,(uint8_t*)&load_profile);
    
    memset((uint8_t*)&load_profile + sizeof(load_profile.u32_epoch), 0, sizeof(ONE_BLOCK_ENERGY_DATA_LOG) - sizeof(load_profile.u32_epoch));
    /* fill last day blank entries if any */
    while((load_profile.u32_epoch/capture_period != RTC_time->epoch/capture_period) && (((load_bkp_profile.u32_epoch*capture_period) % 86400)/capture_period != 0))
    {
        load_bkp_profile.u32_epoch++;
        load_profile.u32_epoch = load_bkp_profile.u32_epoch*capture_period;
        R_DLMS_DataLog_SetEntry(PROFILE_TYPE_BLOCKLOAD,(uint8_t*)&load_profile);
        if(((load_bkp_profile.u32_epoch*capture_period) % 86400)/capture_period == 0)
        {
            break;
        }
    }
    /* check for current day */
    UnixToDatteTime(load_bkp_profile.u32_epoch*capture_period, &temp_time);
    if(load_bkp_profile.u32_epoch != RTC_time->epoch/capture_period)
    {
        temp_time.day = RTC_time->dateTime.day;
        temp_time.month = RTC_time->dateTime.month;
        temp_time.year = RTC_time->dateTime.year;
        temp_time.hour = 0;
        temp_time.minute = 0;
        temp_time.second = 0;
    }
    load_bkp_profile.u32_epoch = Time_Convert_TO2TS(&temp_time)/capture_period;
    /* fill current day blank entries if any */
    while((load_profile.u32_epoch/capture_period < RTC_time->epoch/capture_period)
         || (load_bkp_profile.u32_epoch < RTC_time->epoch/capture_period))
    {
        load_bkp_profile.u32_epoch++;
        load_profile.u32_epoch = load_bkp_profile.u32_epoch*capture_period;
        R_DLMS_DataLog_SetEntry(PROFILE_TYPE_BLOCKLOAD,(uint8_t*)&load_profile);
        if(((load_bkp_profile.u32_epoch*capture_period) % 86400)/capture_period == 0)
        {
            break;
        }
    }
    
    write_page_eeprom(STORAGE_EEPROM_DLMS_BLK_LOAD_BACKUP_ADDR, (uint8_t*)&load_bkp_profile, STORAGE_EEPROM_DLMS_BLK_LOAD_BACKUP_SIZE);
    memset((uint8_t*)&avg_blk_load_values, 0, sizeof(BLOCK_ENERGY_AVG_DATA_LOG));
  }
}
void log_daily_load_profile(st_RTC_time* RTC_time)
{
  ONE_DAILY_ENERGY_DATA_LOG daily_load_profile;
  stTime_struct temp_time;

  /* Dailay load profile */
  daily_load_profile.u32_epoch = from_eeprom(STORAGE_EEPROM_DLMS_DAILY_BACKUP_ADDR + offsetof(ONE_DAILY_ENERGY_DATA_LOG, u32_epoch), member_size(ONE_DAILY_ENERGY_DATA_LOG, u32_epoch));
  UnixToDatteTime(daily_load_profile.u32_epoch, &temp_time);
  if((temp_time.day != RTC_time->dateTime.day) || (temp_time.month != RTC_time->dateTime.month) || (temp_time.year != RTC_time->dateTime.year))
  {
      st_tamper.Trigger_Byte |= _BV(DAILY_PROFILE_BIT);
      if(log_energies_callback != 0)
      {
        log_energies_callback();
      }
      daily_load_profile.u32_epoch += 86400;
      daily_load_profile.u64_energy_val[0] = get_energy(Active_Imp);
      daily_load_profile.u64_energy_val[1] = get_energy(Apparent_Imp);
      daily_load_profile.u64_energy_val[2] = get_energy(Active_Exp);
      daily_load_profile.u64_energy_val[3] = get_energy(Apparent_Exp);
      R_DLMS_DataLog_SetEntry(PROFILE_TYPE_DAILYLOAD,(uint8_t*)&daily_load_profile);
      
      temp_time.day = RTC_time->dateTime.day;
      temp_time.month = RTC_time->dateTime.month;
      temp_time.year = RTC_time->dateTime.year;
      temp_time.hour = 0;
      temp_time.minute = 0;
      temp_time.second = 0;
      
      daily_load_profile.u32_epoch = Time_Convert_TO2TS(&temp_time);
      write_page_eeprom(STORAGE_EEPROM_DLMS_DAILY_BACKUP_ADDR, (uint8_t*)&daily_load_profile, STORAGE_EEPROM_DLMS_DAILY_BACKUP_SIZE);
  }
}

void log_billing_profile(st_RTC_time* RTC_time, uint8_t forced)
{
  ONE_MONTH_ENERGY_DATA_LOG billing_profile;
  stTime_struct temp_time;
  uint8_t month_change;
  uint8_t loop;
  ONE_MD_LOG md_data;
  
  billing_profile.u32_epoch = from_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_epoch), member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_epoch));
  
  UnixToDatteTime(billing_profile.u32_epoch, &temp_time);
  month_change = temp_time.month != RTC_time->dateTime.month;
  month_change |= temp_time.year != RTC_time->dateTime.year;
  
  temp_time.year = RTC_time->dateTime.year;
  temp_time.month = RTC_time->dateTime.month;
  
  temp_time.day = from_eeprom(STORAGE_EEPROM_DLMS_BILLING_ACTION_DATETIME_ADDR + offsetof(BILLING_DATE_TIME_ACTION, u8_day), member_size(BILLING_DATE_TIME_ACTION, u8_day));
  temp_time.hour = from_eeprom(STORAGE_EEPROM_DLMS_BILLING_ACTION_DATETIME_ADDR + offsetof(BILLING_DATE_TIME_ACTION, u8_hour), member_size(BILLING_DATE_TIME_ACTION, u8_hour));
  temp_time.minute = from_eeprom(STORAGE_EEPROM_DLMS_BILLING_ACTION_DATETIME_ADDR + offsetof(BILLING_DATE_TIME_ACTION, u8_minute), member_size(BILLING_DATE_TIME_ACTION, u8_minute));
  temp_time.second = from_eeprom(STORAGE_EEPROM_DLMS_BILLING_ACTION_DATETIME_ADDR + offsetof(BILLING_DATE_TIME_ACTION, u8_second), member_size(BILLING_DATE_TIME_ACTION, u8_second));
  
  if((month_change && (Time_Convert_TO2TS(&temp_time) <= RTC_time->epoch))
  || (forced))
  {
    st_tamper.Trigger_Byte |= _BV(BILLING_PROFILE_BIT);
    if(log_energies_callback != 0)
    {
      log_energies_callback();
    }
    get_running_billing_data(&billing_profile);
    to_eeprom(STORAGE_EEPROM_DLMS_ZONE_CUMULATIVE_KWH_ADDR, get_energy(Active_Imp), STORAGE_EEPROM_DLMS_ZONE_CUMULATIVE_KWH_SIZE);
    to_eeprom(STORAGE_EEPROM_DLMS_ZONE_CUMULATIVE_KVAH_ADDR, get_energy(Apparent_Imp), STORAGE_EEPROM_DLMS_ZONE_CUMULATIVE_KVAH_SIZE);
    billing_profile.u32_epoch = RTC_time->epoch;
    if(forced == 0)
    {
      billing_profile.u32_epoch -= (RTC_time->epoch%60);
    }
    if(get_cumu_power_on_time() > billing_profile.u32_Pwr_on_duration)
    {
      billing_profile.u32_Pwr_on_duration = get_cumu_power_on_time() - billing_profile.u32_Pwr_on_duration;
    }
    else
    {
      billing_profile.u32_Pwr_on_duration = 0;
    }
    billing_profile.u32_tamper_count = get_tamper_counts();
    
    R_DLMS_DataLog_SetEntry(PROFILE_TYPE_BILLING,(uint8_t*)&billing_profile);
    
    billing_profile.u32_Pwr_on_duration = get_cumu_power_on_time();
    billing_profile.u32_tamper_count = get_tamper_counts();
    billing_profile.u32_MD_KW = 0;
    billing_profile.u32_MD_KVA = 0;
    billing_profile.u32_MD_KW_Epoch = billing_profile.u32_epoch;
    billing_profile.u32_MD_KVA_Epoch = billing_profile.u32_epoch;
    for(loop = 0; loop < 8; loop++)
    {
      billing_profile.u32_MD_KW_TZ[loop] = 0;
      billing_profile.u32_MD_KVA_TZ[loop] = 0;
      billing_profile.u32_MD_KW_TZ_Epoch[loop] = billing_profile.u32_epoch;
      billing_profile.u32_MD_KVA_TZ_Epoch[loop] = billing_profile.u32_epoch;
    }
    read_page_eeprom(STORAGE_EEPROM_DLMS_MD_DATA_BACKUP_ADDR, (uint8_t*)&md_data, STORAGE_EEPROM_DLMS_MD_DATA_BACKUP_SIZE);
    md_data.u32_MD_KW_Export = 0;
    md_data.u32_MD_KW_Export_epoch = billing_profile.u32_epoch;
    md_data.u32_MD_KVA_Export = 0;
    md_data.u32_MD_KVA_Export_epoch = billing_profile.u32_epoch;
    write_page_eeprom(STORAGE_EEPROM_DLMS_MD_DATA_BACKUP_ADDR, (uint8_t*)&md_data, STORAGE_EEPROM_DLMS_MD_DATA_BACKUP_SIZE);
    billing_profile.u32_epoch = RTC_time->epoch;
    write_page_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR, (uint8_t*)&billing_profile, sizeof(ONE_MONTH_ENERGY_DATA_LOG));
    add_cumulative_bill_count();
    log_bill_bit(forced);
    load_last_bill();
  }
}

void store_energies_in_zones(uint8_t zone)
{
    uint64_t energy;
    uint8_t zone_index;
    
    if((zone < 1) || (zone > 8))
    {
        return;
    }
    zone_index = zone - 1;
    if(log_energies_callback != 0)
    {
      log_energies_callback();
    }
    if(get_energy(Active_Imp) > from_eeprom(STORAGE_EEPROM_DLMS_ZONE_CUMULATIVE_KWH_ADDR, STORAGE_EEPROM_DLMS_ZONE_CUMULATIVE_KWH_SIZE))
    {
      energy = from_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u64_Cumm_Energy_KWh_TZ[zone_index]), member_size(ONE_MONTH_ENERGY_DATA_LOG, u64_Cumm_Energy_KWh_TZ[zone_index]));
      energy += get_energy(Active_Imp) - from_eeprom(STORAGE_EEPROM_DLMS_ZONE_CUMULATIVE_KWH_ADDR, STORAGE_EEPROM_DLMS_ZONE_CUMULATIVE_KWH_SIZE);
      to_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u64_Cumm_Energy_KWh_TZ[zone_index]), energy, member_size(ONE_MONTH_ENERGY_DATA_LOG, u64_Cumm_Energy_KWh_TZ[zone_index]));
    
      to_eeprom(STORAGE_EEPROM_DLMS_ZONE_CUMULATIVE_KWH_ADDR, get_energy(Active_Imp), STORAGE_EEPROM_DLMS_ZONE_CUMULATIVE_KWH_SIZE);
    }
    if(get_energy(Apparent_Imp) > from_eeprom(STORAGE_EEPROM_DLMS_ZONE_CUMULATIVE_KVAH_ADDR, STORAGE_EEPROM_DLMS_ZONE_CUMULATIVE_KVAH_SIZE))
    {
      energy = from_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u64_Cumm_Energy_KVAh_TZ[zone_index]), member_size(ONE_MONTH_ENERGY_DATA_LOG, u64_Cumm_Energy_KVAh_TZ[zone_index]));
      energy += get_energy(Apparent_Imp) - from_eeprom(STORAGE_EEPROM_DLMS_ZONE_CUMULATIVE_KVAH_ADDR, STORAGE_EEPROM_DLMS_ZONE_CUMULATIVE_KVAH_SIZE);
      to_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u64_Cumm_Energy_KVAh_TZ[zone_index]), energy, member_size(ONE_MONTH_ENERGY_DATA_LOG, u64_Cumm_Energy_KVAh_TZ[zone_index]));
    
      to_eeprom(STORAGE_EEPROM_DLMS_ZONE_CUMULATIVE_KVAH_ADDR, get_energy(Apparent_Imp), STORAGE_EEPROM_DLMS_ZONE_CUMULATIVE_KVAH_SIZE);
    }
}


/*Function below not required*/
/*void update_md_in_zones(uint8_t zone)
{///todo: issue in this method. need to populate MD accordingly to the running zone.
    uint32_t md, epoch;
    uint32_t md_bill;
    uint8_t zone_index;
    if((zone < 1) || (zone > 8))
    {
        return;
    }
    zone_index = zone - 1;
    md_bill = from_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KW_TZ[zone_index]), member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KW_TZ[zone_index]));
    md = get_mdkw_value(0);
    epoch = get_mdkw_time(0);
    if(md_bill < md)
    {
        to_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KW_TZ[zone_index]), md, member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KW_TZ[zone_index]));
        to_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KW_TZ_Epoch[zone_index]), epoch, member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KW_TZ_Epoch[zone_index]));
    }
    
    md = get_mdkva_value(0);
    epoch = get_mdkva_time(0);
    md_bill = from_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KVA_TZ[zone_index]), member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KVA_TZ[zone_index]));
    if(md_bill < md)
    {
        to_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KVA_TZ[zone_index]), md, member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KVA_TZ[zone_index]));
        to_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KVA_TZ_Epoch[zone_index]), epoch, member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KVA_TZ_Epoch[zone_index]));
    }
}
*/

void log_md_data(st_RTC_time* RTC_time, uint8_t zone)
{
  ONE_MD_LOG md_data;
  uint32_t u32_temp, integration_period;
  uint32_t md;
  uint32_t mdkw_snap = 0;
  uint64_t temp_energy;
  stTime_struct temp_time;
  uint8_t zone_index;
  /* md data */
  integration_period = (uint16_t)from_eeprom(STORAGE_EEPROM_DLMS_MD_INTEGRATION_PERIOD_ADDR, STORAGE_EEPROM_DLMS_MD_INTEGRATION_PERIOD_SIZE);
  read_page_eeprom(STORAGE_EEPROM_DLMS_MD_DATA_BACKUP_ADDR, (uint8_t*)&md_data, STORAGE_EEPROM_DLMS_MD_DATA_BACKUP_SIZE);
  UnixToDatteTime(md_data.u32_epoch, &temp_time);
  if(md_data.u32_epoch != RTC_time->epoch/integration_period)
  {
    stRTCTimeSync.u8UpdateFlag = 1;
    stRTCTimeSync.u32NextUpdateTime = (RTC_time->epoch + 300) / 60;     //epoch minutes
    if(log_energies_callback != 0)
    {
      log_energies_callback();
    }
    md_data.u32_epoch++;
    md = from_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KW), member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KW));
    temp_energy = get_energy(Active_Imp);
    u32_temp = ((temp_energy - md_data.u64_energy_kwh)*3600)/integration_period;
    if((u32_temp > md) && (temp_energy > md_data.u64_energy_kwh))
    {
      mdkw_snap = u32_temp;
      to_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KW), u32_temp, member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KW));
      to_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KW_Epoch),md_data.u32_epoch*integration_period, member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KW_Epoch));
    }
    //TOU Zone
    if((zone > 0) && (zone <= 8))
    {
      zone_index = zone - 1;
      md = from_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KW_TZ[zone_index]), member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KW_TZ[zone_index]));
      if(u32_temp > md)
      {
        to_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KW_TZ[zone_index]), u32_temp, member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KW_TZ[zone_index]));
        to_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KW_TZ_Epoch[zone_index]), md_data.u32_epoch*integration_period, member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KW_TZ_Epoch[zone_index]));
      }
    }
    
    md = from_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KVA), member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KVA));
    temp_energy = get_energy(Apparent_Imp);
    u32_temp = ((temp_energy - md_data.u64_energy_kvah)*3600)/integration_period;
    if(u32_temp < mdkw_snap)
    {
      u32_temp = mdkw_snap;
    }
    if((u32_temp > md) && (temp_energy > md_data.u64_energy_kvah))
    {
      to_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KVA), u32_temp, member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KVA));
      to_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KVA_Epoch),md_data.u32_epoch*integration_period, member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KVA_Epoch));
    }
    //TOU Zone
    if((zone > 0) && (zone <= 8))
    {
      zone_index = zone - 1;
      md = from_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KVA_TZ[zone_index]), member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KVA_TZ[zone_index]));
      if(u32_temp > md)
      {
        to_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KVA_TZ[zone_index]), u32_temp, member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KVA_TZ[zone_index]));
        to_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KVA_TZ_Epoch[zone_index]), md_data.u32_epoch*integration_period, member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KVA_TZ_Epoch[zone_index]));
      }
    }
    
    //Export MD
    md = from_eeprom(STORAGE_EEPROM_DLMS_MD_DATA_BACKUP_ADDR + offsetof(ONE_MD_LOG, u32_MD_KW_Export), member_size(ONE_MD_LOG, u32_MD_KW_Export));
    temp_energy = get_energy(Active_Exp);
    u32_temp = ((temp_energy - md_data.u64_energy_kwh_export)*3600)/integration_period;
    if((u32_temp > md) && (temp_energy > md_data.u64_energy_kwh_export))
    {
      md_data.u32_MD_KW_Export = u32_temp;
      md_data.u32_MD_KW_Export_epoch = md_data.u32_epoch*integration_period;
    }
    
    md = from_eeprom(STORAGE_EEPROM_DLMS_MD_DATA_BACKUP_ADDR + offsetof(ONE_MD_LOG, u32_MD_KVA_Export), member_size(ONE_MD_LOG, u32_MD_KVA_Export));
    temp_energy = get_energy(Apparent_Exp);
    u32_temp = ((temp_energy - md_data.u64_energy_kvah_export)*3600)/integration_period;
    if((u32_temp > md) && (temp_energy > md_data.u64_energy_kvah_export))
    {
      
      md_data.u32_MD_KVA_Export = u32_temp;
      md_data.u32_MD_KVA_Export_epoch = md_data.u32_epoch*integration_period;
    }
    
    // MD backup data
    md_data.u32_epoch = RTC_time->epoch/integration_period;
    md_data.u64_energy_kwh = get_energy(Active_Imp);
    md_data.u64_energy_kvah = get_energy(Apparent_Imp);
    md_data.u64_energy_kwh_export = get_energy(Active_Exp);
    md_data.u64_energy_kvah_export = get_energy(Apparent_Exp);
    write_page_eeprom(STORAGE_EEPROM_DLMS_MD_DATA_BACKUP_ADDR, (uint8_t*)&md_data, STORAGE_EEPROM_DLMS_MD_DATA_BACKUP_SIZE);
  }
}

void set_block_load_capture_period(uint32_t RTC_epoch, uint16_t value)
{
    uint32_t time_slot = RTC_epoch/value;
    
    to_eeprom(STORAGE_EEPROM_DLMS_BLK_LOAD_CAP_PERIOD_ADDR, value, STORAGE_EEPROM_DLMS_BLK_LOAD_CAP_PERIOD_SIZE);
    write_page_eeprom(STORAGE_EEPROM_DLMS_BLK_LOAD_BACKUP_ADDR + offsetof(ONE_BLOCK_BACKUP_ENERGY_DATA_LOG, u32_epoch), (uint8_t*)&time_slot, member_size(ONE_BLOCK_BACKUP_ENERGY_DATA_LOG, u32_epoch));
}

uint16_t get_block_load_capture_period(void)
{
    return (uint16_t)from_eeprom(STORAGE_EEPROM_DLMS_BLK_LOAD_CAP_PERIOD_ADDR, STORAGE_EEPROM_DLMS_BLK_LOAD_CAP_PERIOD_SIZE);
}

void set_md_integration_time(uint32_t RTC_epoch, uint16_t value)
{
    uint32_t time_slot = RTC_epoch/value;
    
    to_eeprom(STORAGE_EEPROM_DLMS_MD_INTEGRATION_PERIOD_ADDR, value, STORAGE_EEPROM_DLMS_MD_INTEGRATION_PERIOD_SIZE);
    write_page_eeprom(STORAGE_EEPROM_DLMS_MD_DATA_BACKUP_ADDR + offsetof(ONE_MD_LOG, u32_epoch), (uint8_t*)&time_slot, member_size(ONE_MD_LOG, u32_epoch));
}

uint16_t get_md_integration_time(void)
{
    return (uint16_t)from_eeprom(STORAGE_EEPROM_DLMS_MD_INTEGRATION_PERIOD_ADDR, STORAGE_EEPROM_DLMS_MD_INTEGRATION_PERIOD_SIZE);
}

void add_cumulative_bill_count(void)
{
    uint32_t temp_32 = from_eeprom(STORAGE_EEPROM_DLMS_CUMULATIVE_BILLING_COUNT_ADDR, STORAGE_EEPROM_DLMS_CUMULATIVE_BILLING_COUNT_SIZE);
    to_eeprom(STORAGE_EEPROM_DLMS_CUMULATIVE_BILLING_COUNT_ADDR, (temp_32 + 1), STORAGE_EEPROM_DLMS_CUMULATIVE_BILLING_COUNT_SIZE);
}

uint16_t get_cumulative_bill_count(void)
{
    return from_eeprom(STORAGE_EEPROM_DLMS_CUMULATIVE_BILLING_COUNT_ADDR, STORAGE_EEPROM_DLMS_CUMULATIVE_BILLING_COUNT_SIZE);
}

uint8_t get_avaiable_bill_count(void)
{
    
    uint32_t eeprom_addr;
    buffer_info_t buf_info = {0};
    DLMS_STORAGE FAR_PTR_* p_storage = (DLMS_STORAGE FAR_PTR_*)g_memory_map_event_table[PROFILE_TYPE_BILLING];
    /* Check buffer information */
    eeprom_addr = p_storage[0].address;
#ifdef RETURN_ERROR
    if (
#endif
        read_page_eeprom(eeprom_addr, (uint8_t*)&buf_info, sizeof(buffer_info_t))
#ifndef RETURN_ERROR
        ;
#endif
#ifdef RETURN_ERROR
        != EPR_OK)
        {
            return RLT_ERR; /* Write error */
        }
#endif
        return (uint8_t)buf_info.entries_in_used;
}

uint16_t get_latest_event_id(tag_dlms_profile_type_t datalogProfile_id)
{
    DLMS_STORAGE FAR_PTR_* p_storage = (DLMS_STORAGE FAR_PTR_*)g_memory_map_event_table[datalogProfile_id];
    ONE_EVENT_ENERGY_DATA_LOG profile_log;
    buffer_info_t buf_info = {0};
    
    read_page_eeprom(p_storage[0].address, (uint8_t*)&buf_info, sizeof(buffer_info_t));
    if(buf_info.entries_in_used == 0)
    {
        return 0;
    }
    R_DLMS_DataLog_GetEntry(datalogProfile_id, (uint8_t*)&profile_log, buf_info.entries_in_used - 1);
    
    return profile_log.u16_Tamper_ID;
}

uint32_t get_running_bill_power_on_time(void)
{
  return get_cumu_power_on_time() - from_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_Pwr_on_duration), member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_Pwr_on_duration));
}

void get_load_profile(ONE_BLOCK_ENERGY_DATA_LOG* profile_log,
                    uint16_t index,
                    e_entry_type entry_type)
{
    DLMS_STORAGE FAR_PTR_* p_storage = (DLMS_STORAGE FAR_PTR_*)g_memory_map_event_table[PROFILE_TYPE_BLOCKLOAD];
    buffer_info_t buf_info = {0};
    uint8_t loop;
    uint16_t read_index;
    
    read_page_eeprom(p_storage[0].address, (uint8_t*)&buf_info, sizeof(buffer_info_t));
    if(entry_type == e_last_entry)
    {
        read_index = buf_info.entries_in_used - 1;
    }
    else
    {
        read_index = index;
    }
    if(buf_info.entries_in_used == 0)
    {
        memset(profile_log, 0, sizeof(ONE_BLOCK_ENERGY_DATA_LOG));
    }
    
    if(entry_type == e_latest_entry)
    {
        ONE_BLOCK_BACKUP_ENERGY_DATA_LOG backup_load_profile;
        read_page_eeprom(STORAGE_EEPROM_DLMS_BLK_LOAD_BACKUP_ADDR, (uint8_t*)&backup_load_profile, STORAGE_EEPROM_DLMS_BLK_LOAD_BACKUP_SIZE);
#ifdef SINGLE_PHASE_METER
        profile_log->u32_block_energy_val[LP_Active_Imp] = get_energy(Active_Imp) - backup_load_profile.u64_block_energy_val[LP_Active_Imp];
        profile_log->u32_block_energy_val[LP_Active_Exp] = get_energy(Active_Exp) - backup_load_profile.u64_block_energy_val[LP_Active_Exp];
        profile_log->u32_block_energy_val[LP_Apparent_Imp] = get_energy(Apparent_Imp) - backup_load_profile.u64_block_energy_val[LP_Apparent_Imp];
        profile_log->u32_block_energy_val[LP_Apparent_Exp] = get_energy(Apparent_Exp) - backup_load_profile.u64_block_energy_val[LP_Apparent_Exp];
        
        profile_log->u16_volt = (uint16_t)(avg_blk_load_values.fl_volt*100);
        profile_log->u16_current_phase = (uint16_t)(avg_blk_load_values.fl_current_phase*100);
        profile_log->u16_current_neutral = (uint16_t)(avg_blk_load_values.fl_current_neutral*100);
#else
        for(loop = 0; loop < 8; loop++)
        {
            profile_log->u32_block_energy_val[loop] = get_energy(loop) - backup_load_profile.u64_block_energy_val[loop];
        }
        
        for(loop = 0; loop < 3; loop++)
        {
            profile_log->u16_volt[loop] = (uint16_t)avg_blk_load_values.fl_volt[loop];
            profile_log->u32_current[loop] = (uint32_t)avg_blk_load_values.fl_current[loop];
            profile_log->u8_pf[loop] = (uint8_t)avg_blk_load_values.fl_pf[loop];
        }
#endif
    }
    else
    {
        R_DLMS_DataLog_GetEntry(PROFILE_TYPE_BLOCKLOAD, (uint8_t *)profile_log, read_index);
    }
}

void get_daily_load_profile(ONE_DAILY_ENERGY_DATA_LOG* profile_log, uint16_t index)
{
    R_DLMS_DataLog_GetEntry(PROFILE_TYPE_DAILYLOAD, (uint8_t *)profile_log, index);
}

void get_running_billing_data(ONE_MONTH_ENERGY_DATA_LOG* profile_log)
{
  uint64_t temp_64, temp_kvah_64;
  uint8_t running_zone;
  
  
  read_page_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR, (uint8_t*)profile_log, STORAGE_EEPROM_DLMS_BILLING_BACKUP_SIZE);
  running_zone = read_eeprom(STORAGE_EEPROM_DLMS_RUNNING_ZONE_ID_ADDR) - 1;
  
  temp_64 = get_energy(Active_Imp) - /*use profile_log*/
                                     from_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u64_Cumm_Energy_KWh), member_size(ONE_MONTH_ENERGY_DATA_LOG, u64_Cumm_Energy_KWh)); 
  temp_64 *= 100;
  temp_kvah_64 = get_energy(Apparent_Imp) - 
                                     from_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u64_Cumm_Energy_KVAh), member_size(ONE_MONTH_ENERGY_DATA_LOG, u64_Cumm_Energy_KVAh));
  if(temp_kvah_64 > 0)
  {
    temp_64 /= temp_kvah_64;
  }
  if(temp_64 > 100)
  {
    temp_64 = 100;
  }
  profile_log->u8_Sys_Power_Factor = (uint8_t)temp_64;
  
  profile_log->u64_Cumm_Energy_KWh = get_energy(Active_Imp);
  profile_log->u64_Cumm_Energy_KVAh = get_energy(Apparent_Imp);
  profile_log->u64_Cumm_Energy_KWh_expo = get_energy(Active_Exp);
  profile_log->u64_Cumm_Energy_KVAh_expo = get_energy(Apparent_Exp);
  profile_log->u64_Cumm_Energy_KVarh_Q1 = get_energy(Reactive_Ind_Imp);
  profile_log->u64_Cumm_Energy_KVarh_Q2 = get_energy(Reactive_Cap_Exp);
  profile_log->u64_Cumm_Energy_KVarh_Q3 = get_energy(Reactive_Ind_Exp);
  profile_log->u64_Cumm_Energy_KVarh_Q4 = get_energy(Reactive_Cap_Imp);
  
  temp_64 = get_energy(Active_Imp) - from_eeprom(STORAGE_EEPROM_DLMS_ZONE_CUMULATIVE_KWH_ADDR, STORAGE_EEPROM_DLMS_ZONE_CUMULATIVE_KWH_SIZE);
  profile_log->u64_Cumm_Energy_KWh_TZ[running_zone] += temp_64;
  
  temp_64 = get_energy(Apparent_Imp) - from_eeprom(STORAGE_EEPROM_DLMS_ZONE_CUMULATIVE_KVAH_ADDR, STORAGE_EEPROM_DLMS_ZONE_CUMULATIVE_KVAH_SIZE);
  profile_log->u64_Cumm_Energy_KVAh_TZ[running_zone] += temp_64;
}

void get_billing_profile_data(ONE_MONTH_ENERGY_DATA_LOG* profile_log, uint16_t index, uint8_t latest_entry)
{
    DLMS_STORAGE FAR_PTR_* p_storage = (DLMS_STORAGE FAR_PTR_*)g_memory_map_event_table[PROFILE_TYPE_BILLING];
    buffer_info_t buf_info = {0};
    
    read_page_eeprom(p_storage[0].address, (uint8_t*)&buf_info, sizeof(buffer_info_t));
    if(latest_entry == 1)
    {
        if(buf_info.entries_in_used == 0)
        {
            memset(profile_log, 0, sizeof(ONE_MONTH_ENERGY_DATA_LOG));
            return;
        }
        index = buf_info.entries_in_used;
    }
    if(index == 0)
    {
        get_running_billing_data(profile_log);
    }
    else
    {
        index -= 1;
        if(buf_info.entries_in_used == 0)
        {
            memset(profile_log, 0, sizeof(ONE_MONTH_ENERGY_DATA_LOG));
        }
        else
        {
            R_DLMS_DataLog_GetEntry(PROFILE_TYPE_BILLING, (uint8_t *)profile_log, index);
        }
    }
}

void load_last_bill(void)
{
    buffer_info_t buf_info = {0};
    uint16_t index;
#ifdef UTILITY_JNK
    ONE_MONTH_ENERGY_DATA_LOG	bill_log;
#endif
    DLMS_STORAGE FAR_PTR_* p_storage = (DLMS_STORAGE FAR_PTR_*)g_memory_map_event_table[PROFILE_TYPE_BILLING];
    
    read_page_eeprom(p_storage[0].address, (uint8_t*)&buf_info, sizeof(buffer_info_t));
    index = buf_info.entries_in_used - 1;
    
    if(buf_info.entries_in_used < 1)
    {
        memset((uint8_t *)&g_Last_bill_log, 0, sizeof(ONE_MONTH_ENERGY_DATA_LOG));
    }
    else
    {
        R_DLMS_DataLog_GetEntry(PROFILE_TYPE_BILLING, (uint8_t *)&g_Last_bill_log, index);
#ifdef UTILITY_JNK
        if (index != 0)
        {
            index--;
            R_DLMS_DataLog_GetEntry(PROFILE_TYPE_BILLING, (uint8_t*)&bill_log, index);
            g_bill_kwh_h2 = bill_log.u64_Cumm_Energy_KWh;
            g_bill_mdkw_h2 = bill_log.u32_MD_KW;
            g_bill_mdkw_epoch_h2 = bill_log.u32_MD_KW_Epoch;
        }
        if (index != 0)
        {
            index--;
            R_DLMS_DataLog_GetEntry(PROFILE_TYPE_BILLING, (uint8_t*)&bill_log, index);
            g_bill_kwh_h3 = bill_log.u64_Cumm_Energy_KWh;
            g_bill_mdkw_h3 = bill_log.u32_MD_KW;
            g_bill_mdkw_epoch_h3 = bill_log.u32_MD_KW_Epoch;
        }
#endif
    }    
}

uint32_t get_mdkw_expo_value(void)
{
  return from_eeprom(STORAGE_EEPROM_DLMS_MD_DATA_BACKUP_ADDR + offsetof(ONE_MD_LOG, u32_MD_KW_Export), member_size(ONE_MD_LOG, u32_MD_KW_Export));
}

uint32_t get_mdkva_expo_value(void)
{
  return from_eeprom(STORAGE_EEPROM_DLMS_MD_DATA_BACKUP_ADDR + offsetof(ONE_MD_LOG, u32_MD_KVA_Export), member_size(ONE_MD_LOG, u32_MD_KVA_Export));
}

uint32_t get_mdkw_expo_time(void)
{
  return from_eeprom(STORAGE_EEPROM_DLMS_MD_DATA_BACKUP_ADDR + offsetof(ONE_MD_LOG, u32_MD_KW_Export_epoch), member_size(ONE_MD_LOG, u32_MD_KW_Export_epoch));
}

uint32_t get_mdkva_expo_time(void)
{
  return from_eeprom(STORAGE_EEPROM_DLMS_MD_DATA_BACKUP_ADDR + offsetof(ONE_MD_LOG, u32_MD_KVA_Export_epoch), member_size(ONE_MD_LOG, u32_MD_KVA_Export_epoch));
}

uint32_t get_mdkw_value(uint8_t zone)
{
    if(zone > 8)
    {
        return 0;
    }
    if(zone == 0)
    {
      return from_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KW), member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KW));
    }
    else
    {
      return from_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KW_TZ[zone - 1]), member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KW_TZ[zone - 1]));
    }
}

uint32_t get_mdkva_value(uint8_t zone)
{
    if(zone > 8)
    {
        return 0;
    }
    if(zone == 0)
    {
      return from_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KVA), member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KVA));
    }
    else
    {
      return from_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KVA_TZ[zone - 1]), member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KVA_TZ[zone - 1]));
    }
}

uint64_t get_kwh_time_zone(uint8_t zone)
{
    uint32_t energy;
    uint8_t active_zone;
    
    active_zone = read_eeprom(STORAGE_EEPROM_DLMS_RUNNING_ZONE_ID_ADDR);
    if(zone > 8)
    {
        return 0;
    }
    if(zone == 0)
    {
        return get_energy(Active_Imp);
    }
    energy = from_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u64_Cumm_Energy_KWh_TZ[zone - 1]), member_size(ONE_MONTH_ENERGY_DATA_LOG, u64_Cumm_Energy_KWh_TZ[zone - 1]));
    if(zone == active_zone)
    {
      return energy + (get_energy(Active_Imp) - from_eeprom(STORAGE_EEPROM_DLMS_ZONE_CUMULATIVE_KWH_ADDR, STORAGE_EEPROM_DLMS_ZONE_CUMULATIVE_KWH_SIZE));
    }
    else
    {
      return energy;
    }
    
}

uint64_t get_kvah_time_zone(uint8_t zone)
{
    uint32_t energy;
    uint8_t active_zone;
    
    active_zone = read_eeprom(STORAGE_EEPROM_DLMS_RUNNING_ZONE_ID_ADDR);
    if(zone > 8)
    {
        return 0;
    }
    if(zone == 0)
    {
        return get_energy(Apparent_Imp);
    }
    energy = from_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u64_Cumm_Energy_KVAh_TZ[zone - 1]), member_size(ONE_MONTH_ENERGY_DATA_LOG, u64_Cumm_Energy_KVAh_TZ[zone - 1]));
    if(zone == active_zone)
    {
      return energy + (get_energy(Apparent_Imp) - from_eeprom(STORAGE_EEPROM_DLMS_ZONE_CUMULATIVE_KVAH_ADDR, STORAGE_EEPROM_DLMS_ZONE_CUMULATIVE_KVAH_SIZE));
    }
    else
    {
      return energy;
    }
}

uint32_t get_mdkw_time(uint8_t zone)
{
    if(zone > 8)
    {
        return 0;
    }
    if(zone == 0)
    {
      return from_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KW_Epoch), member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KW_Epoch));
    }
    else
    {
      return from_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KW_TZ_Epoch[zone - 1]), member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KW_TZ_Epoch[zone - 1]));
    }
}

uint32_t get_mdkva_time(uint8_t zone)
{
    if(zone > 8)
    {
        return 0;
    }
    if(zone == 0)
    {
      return from_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KVA_Epoch), member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KVA_Epoch));
    }
    else
    {
      return from_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KVA_TZ_Epoch[zone - 1]), member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KVA_TZ_Epoch[zone - 1]));
    }
}

void get_tamper_profile(ONE_EVENT_ENERGY_DATA_LOG *profile_log, tag_dlms_profile_type_t datalogProfile_id, uint32_t index)
{
    R_DLMS_DataLog_GetEntry(datalogProfile_id, (uint8_t *)profile_log, index);
}

uint16_t get_profile_entry_in_use(tag_dlms_profile_type_t datalogProfile_id)
{
    DLMS_STORAGE FAR_PTR_* p_storage = (DLMS_STORAGE FAR_PTR_*)g_memory_map_event_table[datalogProfile_id];
    buffer_info_t buf_info = {0};
    
    read_page_eeprom(p_storage[0].address, (uint8_t*)&buf_info, sizeof(buffer_info_t));
    return buf_info.entries_in_used;
}

void set_default_datalog_on_eeprom_erase(st_RTC_time* RTC_time)
{
    set_data_logging_times(RTC_time, 1);
    /* default blk load profile capture period */
    to_eeprom(STORAGE_EEPROM_DLMS_BLK_LOAD_CAP_PERIOD_ADDR, DEFAULT_BLOCK_LOAD_CAPTURE_PERIOD, STORAGE_EEPROM_DLMS_BLK_LOAD_CAP_PERIOD_SIZE);
    
    /* default MD integration epoch and epoch */
    to_eeprom(STORAGE_EEPROM_DLMS_MD_INTEGRATION_PERIOD_ADDR, DEFAULT_MD_INTEGRATION_PERIOD, STORAGE_EEPROM_DLMS_MD_INTEGRATION_PERIOD_SIZE);
    
    /* billimg date */
    to_eeprom(STORAGE_EEPROM_DLMS_BILLING_ACTION_DATETIME_ADDR + offsetof(BILLING_DATE_TIME_ACTION, u8_day), DEFAULT_BILLING_DATE, member_size(BILLING_DATE_TIME_ACTION, u8_day));
    to_eeprom(STORAGE_EEPROM_DLMS_BILLING_ACTION_DATETIME_ADDR + offsetof(BILLING_DATE_TIME_ACTION, u8_hour), DEFAULT_BILLING_HOUR, member_size(BILLING_DATE_TIME_ACTION, u8_hour));
    to_eeprom(STORAGE_EEPROM_DLMS_BILLING_ACTION_DATETIME_ADDR + offsetof(BILLING_DATE_TIME_ACTION, u8_minute), DEFAULT_BILLING_MINUTE, member_size(BILLING_DATE_TIME_ACTION, u8_minute));
    to_eeprom(STORAGE_EEPROM_DLMS_BILLING_ACTION_DATETIME_ADDR + offsetof(BILLING_DATE_TIME_ACTION, u8_second), DEFAULT_BILLING_SECOND, member_size(BILLING_DATE_TIME_ACTION, u8_second));
    
    to_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_epoch), RTC_time->epoch, member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_epoch));
}

void set_data_logging_times(st_RTC_time* RTC_time, uint8_t from_mid_night)
{
    uint32_t u32_temp;
    uint8_t loop;
    stTime_struct temp_time;
    get_RTC_dateTime_with_epoch(RTC_time);
    /* start of day epoch */
    temp_time.year = RTC_time->dateTime.year;
    temp_time.month = RTC_time->dateTime.month;
    temp_time.day = RTC_time->dateTime.day;
    temp_time.hour = 0;
    temp_time.minute = 0;
    temp_time.second = 0;
    //REVIEW: Rakesh, if else not required.
    if(from_mid_night)
    {
      temp_time.hour = 0;
      temp_time.minute = 0;
      temp_time.second = 0;
    }
    else
    {
      temp_time.hour = RTC_time->dateTime.hour;
      temp_time.minute = (RTC_time->dateTime.minute*60 + RTC_time->dateTime.minute)/from_eeprom(STORAGE_EEPROM_DLMS_BLK_LOAD_CAP_PERIOD_ADDR, STORAGE_EEPROM_DLMS_BLK_LOAD_CAP_PERIOD_SIZE);
      temp_time.second = 0;
    }
    
    u32_temp = Time_Convert_TO2TS(&temp_time);
    /* blk load profile default capture slot */
    to_eeprom(STORAGE_EEPROM_DLMS_BLK_LOAD_BACKUP_ADDR + offsetof(ONE_BLOCK_ENERGY_DATA_LOG, u32_epoch), ((u32_temp/86400)*86400)/DEFAULT_BLOCK_LOAD_CAPTURE_PERIOD, member_size(ONE_BLOCK_ENERGY_DATA_LOG, u32_epoch));
    
    /* daily load profile epoch */
    to_eeprom(STORAGE_EEPROM_DLMS_DAILY_BACKUP_ADDR + offsetof(ONE_DAILY_ENERGY_DATA_LOG, u32_epoch), u32_temp, member_size(ONE_DAILY_ENERGY_DATA_LOG, u32_epoch));
    
    /* MD integration default integration slot */
    to_eeprom(STORAGE_EEPROM_DLMS_MD_DATA_BACKUP_ADDR + offsetof(ONE_MD_LOG, u32_epoch), RTC_time->epoch/DEFAULT_MD_INTEGRATION_PERIOD, member_size(ONE_MD_LOG, u32_epoch));
    
    /*default bill MD Dates */
    temp_time.year = RTC_time->dateTime.year;
    temp_time.month = RTC_time->dateTime.month;
    temp_time.day = 1;
    temp_time.hour = 0;
    temp_time.minute = 0;
    temp_time.second = 0;
    
    u32_temp = Time_Convert_TO2TS(&temp_time);
    to_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KW_Epoch), u32_temp, member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KW_Epoch));
    to_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KVA_Epoch), u32_temp, member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KVA_Epoch));
    for(loop = 0; loop < 8; loop++)
    {
      to_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KW_TZ_Epoch[loop]), u32_temp, member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KW_TZ_Epoch[loop]));
      to_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KVA_TZ_Epoch[loop]), u32_temp, member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_MD_KVA_TZ_Epoch[loop]));
    }
}

//uint32_t get_next_bill_date(st_RTC_time* RTC_time)
//{
//  stTime_struct temp_time;
  
//  temp_time.year = RTC_time->dateTime.year;
//  temp_time.month = RTC_time->dateTime.month;
//  temp_time.day = from_eeprom(STORAGE_EEPROM_DLMS_BILLING_ACTION_DATETIME_ADDR + offsetof(BILLING_DATE_TIME_ACTION, u8_day), member_size(BILLING_DATE_TIME_ACTION, u8_day));
//  temp_time.hour = from_eeprom(STORAGE_EEPROM_DLMS_BILLING_ACTION_DATETIME_ADDR + offsetof(BILLING_DATE_TIME_ACTION, u8_hour), member_size(BILLING_DATE_TIME_ACTION, u8_hour));
//  temp_time.minute = from_eeprom(STORAGE_EEPROM_DLMS_BILLING_ACTION_DATETIME_ADDR + offsetof(BILLING_DATE_TIME_ACTION, u8_minute), member_size(BILLING_DATE_TIME_ACTION, u8_minute));
//  temp_time.second = from_eeprom(STORAGE_EEPROM_DLMS_BILLING_ACTION_DATETIME_ADDR + offsetof(BILLING_DATE_TIME_ACTION, u8_second), member_size(BILLING_DATE_TIME_ACTION, u8_second));
    
//  if(RTC_time->dateTime.day >= temp_time.day)
//  {
//    temp_time = add_months(&temp_time, 1);
//  }
//  return Time_Convert_TO2TS(&temp_time);
//}

void set_bill_date(st_RTC_time* RTC_time, uint8_t *data)
{
  stTime_struct temp_time;
  uint32_t u32_temp;
  to_eeprom(STORAGE_EEPROM_DLMS_BILLING_ACTION_DATETIME_ADDR + offsetof(BILLING_DATE_TIME_ACTION, u8_day), data[0], member_size(BILLING_DATE_TIME_ACTION, u8_day));
  to_eeprom(STORAGE_EEPROM_DLMS_BILLING_ACTION_DATETIME_ADDR + offsetof(BILLING_DATE_TIME_ACTION, u8_hour), data[1], member_size(BILLING_DATE_TIME_ACTION, u8_hour));
  to_eeprom(STORAGE_EEPROM_DLMS_BILLING_ACTION_DATETIME_ADDR + offsetof(BILLING_DATE_TIME_ACTION, u8_minute), data[2], member_size(BILLING_DATE_TIME_ACTION, u8_minute));
  to_eeprom(STORAGE_EEPROM_DLMS_BILLING_ACTION_DATETIME_ADDR + offsetof(BILLING_DATE_TIME_ACTION, u8_second), data[3], member_size(BILLING_DATE_TIME_ACTION, u8_second));
  
  temp_time = RTC_time->dateTime;
  temp_time.day = data[0];
  temp_time.hour = data[1];
  temp_time.minute = data[2];
  temp_time.second = data[3];
  if(RTC_time->epoch < Time_Convert_TO2TS(&temp_time))
  {
    if(temp_time.month == 1)
    {
        temp_time.month = 12;
        temp_time.year--;
    }
    else
    {
      temp_time.month--;
    }
  }
  u32_temp = Time_Convert_TO2TS(&temp_time);
  to_eeprom(STORAGE_EEPROM_DLMS_BILLING_BACKUP_ADDR + offsetof(ONE_MONTH_ENERGY_DATA_LOG, u32_epoch), u32_temp, member_size(ONE_MONTH_ENERGY_DATA_LOG, u32_epoch));
}

void get_bill_date(uint8_t *data)
{
  data[0] = from_eeprom(STORAGE_EEPROM_DLMS_BILLING_ACTION_DATETIME_ADDR + offsetof(BILLING_DATE_TIME_ACTION, u8_day), member_size(BILLING_DATE_TIME_ACTION, u8_day));
  data[1] = from_eeprom(STORAGE_EEPROM_DLMS_BILLING_ACTION_DATETIME_ADDR + offsetof(BILLING_DATE_TIME_ACTION, u8_hour), member_size(BILLING_DATE_TIME_ACTION, u8_hour));
  data[2] = from_eeprom(STORAGE_EEPROM_DLMS_BILLING_ACTION_DATETIME_ADDR + offsetof(BILLING_DATE_TIME_ACTION, u8_minute), member_size(BILLING_DATE_TIME_ACTION, u8_minute));
  data[3] = from_eeprom(STORAGE_EEPROM_DLMS_BILLING_ACTION_DATETIME_ADDR + offsetof(BILLING_DATE_TIME_ACTION, u8_second), member_size(BILLING_DATE_TIME_ACTION, u8_second));
}

void generate_bill_on_demand(st_RTC_time* RTC_time)
{
  log_billing_profile(RTC_time, 1);
}

void clear_non_roll_over(void)
{
  uint32_t eeprom_addr;
  DLMS_STORAGE FAR_PTR_* p_storage = (DLMS_STORAGE FAR_PTR_*)g_memory_map_event_table[PROFILE_TYPE_NON_ROLLOVER_EVENTS];
  buffer_info_t buf_info = {0};
  eeprom_addr = p_storage[0].address;
  
  write_page_eeprom(eeprom_addr, (uint8_t*)&buf_info, sizeof(buffer_info_t));
}

void init_DataLogging(void)
{
    cal_zone = read_eeprom(STORAGE_EEPROM_DLMS_RUNNING_ZONE_ID_ADDR);
}