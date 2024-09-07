#include "em_energies.h"
#include "eeprom_storage.h"

EM_ENERGY_DATA loggedEnergies;
EM_USER_ENERGY_COUNTER myEnergies;
e_metering_mode_type e_metering_mode;

void GetEnergyCounter(EM_ENERGY_DATA *energies)
{
#if 1
  WPR_em_get_energies(energies);
#else
  EM_ENERGY_COUNTER em_energy_counter;
  EM_ENERGY_VALUE em_energy_value;
  DI();
  EM_GetEnergyCounter(&em_energy_counter);
  EI();
  EM_EnergyCounterToEnergyValue(&em_energy_counter, &em_energy_value);
  
  energies->counter.active_imp = (uint64_t)em_energy_value.active_imp;
  energies->counter.active_exp = (uint64_t)em_energy_value.active_exp;
  energies->counter.reactive_ind_imp = (uint64_t)em_energy_value.reactive_ind_imp;
  energies->counter.reactive_ind_exp = (uint64_t)em_energy_value.reactive_ind_exp;
  /*seems bug in lib cap_exp and cap_imp swapped*/
  /*either swap here or in em_energies_wrp*/
  energies->counter.reactive_cap_imp = (uint64_t)em_energy_value.reactive_cap_exp;
  energies->counter.reactive_cap_exp = (uint64_t)em_energy_value.reactive_cap_imp;
  energies->counter.apparent_imp = (uint64_t)em_energy_value.apparent_imp;
  energies->counter.apparent_exp = (uint64_t)em_energy_value.apparent_exp;
  
  
  energies->remainder.active_imp = em_energy_value.active_imp - (uint64_t)em_energy_value.active_imp;
  energies->remainder.active_exp = em_energy_value.active_exp - (uint64_t)em_energy_value.active_exp;
  energies->remainder.reactive_ind_imp = em_energy_value.reactive_ind_imp - (uint64_t)em_energy_value.reactive_ind_imp;
  energies->remainder.reactive_ind_exp = em_energy_value.reactive_ind_exp - (uint64_t)em_energy_value.reactive_ind_exp;
  /*seems bug in lib cap_exp and cap_imp swapped*/
  /*either swap here or in em_energies_wrp*/
  energies->remainder.reactive_cap_imp = em_energy_value.reactive_cap_exp - (uint64_t)em_energy_value.reactive_cap_exp;
  energies->remainder.reactive_cap_exp = em_energy_value.reactive_cap_imp - (uint64_t)em_energy_value.reactive_cap_imp;
  energies->remainder.apparent_imp = em_energy_value.apparent_imp - (uint64_t)em_energy_value.apparent_imp;
  energies->remainder.apparent_exp = em_energy_value.apparent_exp - (uint64_t)em_energy_value.apparent_exp;
#endif // 1
}

void SetEnergyCounter(EM_ENERGY_DATA *energies)
{
#if 1
  WPR_em_set_energies(energies);
#else
  EM_ENERGY_COUNTER em_energy_counter;
  EM_ENERGY_VALUE em_energy_value;
  EM_OPERATION_DATA temp_op_data;
  
  
  em_energy_value.active_imp = energies->counter.active_imp + energies->remainder.active_imp;
  em_energy_value.active_exp = energies->counter.active_exp + energies->remainder.active_exp;
  em_energy_value.reactive_ind_imp = energies->counter.reactive_ind_imp + energies->remainder.reactive_ind_imp;
  em_energy_value.reactive_ind_exp = energies->counter.reactive_ind_exp + energies->remainder.reactive_ind_exp;
  /*seems bug in lib cap_exp and cap_imp swapped*/
  /*either swap here or in em_energies_wrp*/
  em_energy_value.reactive_cap_exp = energies->counter.reactive_cap_imp + energies->remainder.reactive_cap_imp;
  em_energy_value.reactive_cap_imp = energies->counter.reactive_cap_exp + energies->remainder.reactive_cap_exp;
  em_energy_value.apparent_imp = energies->counter.apparent_imp + energies->remainder.apparent_imp;
  em_energy_value.apparent_exp = energies->counter.apparent_exp + energies->remainder.apparent_exp;
  
  
  EM_EnergyValueToEnergyCounter(&em_energy_counter, &em_energy_value);
  EM_GetOperationData(&temp_op_data);
  
  temp_op_data.energy_counter.active_imp = 0;
  temp_op_data.energy_counter.active_exp = 0;
  temp_op_data.energy_counter.reactive_ind_imp = 0;
  temp_op_data.energy_counter.reactive_ind_exp = 0;
  temp_op_data.energy_counter.reactive_cap_imp = 0;
  temp_op_data.energy_counter.reactive_cap_exp = 0;
  temp_op_data.energy_counter.apparent_imp = 0;
  temp_op_data.energy_counter.apparent_exp = 0;

  temp_op_data.remainder.active_imp = 0;
  temp_op_data.remainder.active_exp = 0;
  temp_op_data.remainder.reactive_ind_imp = 0;
  temp_op_data.remainder.reactive_ind_exp = 0;
  temp_op_data.remainder.reactive_cap_imp = 0;
  temp_op_data.remainder.reactive_cap_exp = 0;
  temp_op_data.remainder.apparent_imp = 0;
  temp_op_data.remainder.apparent_exp = 0;
    
  EM_SetOperationData(&temp_op_data);
  EM_AddEnergyCounter(&em_energy_counter);

#endif // 1
}

void read_energies_from_backend(EM_USER_ENERGY_COUNTER *energies)
{
    GetEnergyCounter(&loggedEnergies);
    if (e_metering_mode == e_Import_Only)
    {
      energies->total.energy[Active_Imp] = loggedEnergies.counter.active_imp + loggedEnergies.counter.active_exp;
      energies->total.energy[Reactive_Ind_Imp] = loggedEnergies.counter.reactive_ind_imp + loggedEnergies.counter.reactive_ind_exp;
      energies->total.energy[Reactive_Cap_Imp] = loggedEnergies.counter.reactive_cap_imp + loggedEnergies.counter.reactive_cap_exp;
      energies->total.energy[Apparent_Imp] = loggedEnergies.counter.apparent_imp + loggedEnergies.counter.apparent_exp;
      
      energies->accumulated.energy[Active_Imp] = loggedEnergies.counter.active_imp - energies->stored.energy[Active_Imp] + loggedEnergies.counter.active_exp;
      energies->accumulated.energy[Reactive_Ind_Imp] = loggedEnergies.counter.reactive_ind_imp - energies->stored.energy[Reactive_Ind_Imp] + loggedEnergies.counter.reactive_ind_exp;
      energies->accumulated.energy[Reactive_Cap_Imp] = loggedEnergies.counter.reactive_cap_imp - energies->stored.energy[Reactive_Cap_Imp] + loggedEnergies.counter.reactive_cap_exp;
      energies->accumulated.energy[Apparent_Imp] = loggedEnergies.counter.apparent_imp - energies->stored.energy[Apparent_Imp] + loggedEnergies.counter.apparent_exp;
      
      energies->remainder.energy[Active_Imp] = loggedEnergies.remainder.active_imp + loggedEnergies.remainder.active_exp;
      energies->remainder.energy[Reactive_Ind_Imp] = loggedEnergies.remainder.reactive_ind_imp + loggedEnergies.remainder.reactive_ind_exp;
      energies->remainder.energy[Reactive_Cap_Imp] = loggedEnergies.remainder.reactive_cap_imp + loggedEnergies.remainder.reactive_cap_exp;
      energies->remainder.energy[Apparent_Imp] = loggedEnergies.remainder.apparent_imp + loggedEnergies.remainder.apparent_exp;
    }
    else if (e_metering_mode == e_Import_Export)
    {
      energies->total.energy[Active_Imp] = loggedEnergies.counter.active_imp;
      energies->total.energy[Active_Exp] = loggedEnergies.counter.active_exp;
      energies->total.energy[Reactive_Ind_Imp] = loggedEnergies.counter.reactive_ind_imp;
      energies->total.energy[Reactive_Ind_Exp] = loggedEnergies.counter.reactive_ind_exp;
      energies->total.energy[Reactive_Cap_Imp] = loggedEnergies.counter.reactive_cap_imp;
      energies->total.energy[Reactive_Cap_Exp] = loggedEnergies.counter.reactive_cap_exp;
      energies->total.energy[Apparent_Imp] = loggedEnergies.counter.apparent_imp;
      energies->total.energy[Apparent_Exp] = loggedEnergies.counter.apparent_exp;
      
      if(loggedEnergies.counter.active_imp > energies->stored.energy[Active_Imp])
      {
        energies->accumulated.energy[Active_Imp] = loggedEnergies.counter.active_imp - energies->stored.energy[Active_Imp];
      }
      
      if(loggedEnergies.counter.active_exp > energies->stored.energy[Active_Exp])
      {
        energies->accumulated.energy[Active_Exp] = loggedEnergies.counter.active_exp - energies->stored.energy[Active_Exp];
      }
      
      if(loggedEnergies.counter.reactive_ind_imp > energies->stored.energy[Reactive_Ind_Imp])
      {
        energies->accumulated.energy[Reactive_Ind_Imp] = loggedEnergies.counter.reactive_ind_imp - energies->stored.energy[Reactive_Ind_Imp];
      }
      
      if(loggedEnergies.counter.reactive_ind_exp > energies->stored.energy[Reactive_Ind_Exp])
      {
        energies->accumulated.energy[Reactive_Ind_Exp] = loggedEnergies.counter.reactive_ind_exp - energies->stored.energy[Reactive_Ind_Exp];
      }
      
      if(loggedEnergies.counter.reactive_cap_imp > energies->stored.energy[Reactive_Cap_Imp])
      {
        energies->accumulated.energy[Reactive_Cap_Imp] = loggedEnergies.counter.reactive_cap_imp - energies->stored.energy[Reactive_Cap_Imp];
      }
      if(loggedEnergies.counter.reactive_cap_exp > energies->stored.energy[Reactive_Cap_Exp])
      {
        energies->accumulated.energy[Reactive_Cap_Exp] = loggedEnergies.counter.reactive_cap_exp - energies->stored.energy[Reactive_Cap_Exp];
      }
      
      if(loggedEnergies.counter.apparent_imp > energies->stored.energy[Apparent_Imp])
      {
        energies->accumulated.energy[Apparent_Imp] = loggedEnergies.counter.apparent_imp - energies->stored.energy[Apparent_Imp];
      }
      
      if(loggedEnergies.counter.apparent_exp > energies->stored.energy[Apparent_Exp])
      {
        energies->accumulated.energy[Apparent_Exp] = loggedEnergies.counter.apparent_exp - energies->stored.energy[Apparent_Exp];
      }
      
      energies->remainder.energy[Active_Imp] = loggedEnergies.remainder.active_imp;
      energies->remainder.energy[Active_Exp] = loggedEnergies.remainder.active_exp;
      energies->remainder.energy[Reactive_Ind_Imp] = loggedEnergies.remainder.reactive_ind_imp;
      energies->remainder.energy[Reactive_Ind_Exp] = loggedEnergies.remainder.reactive_ind_exp;
      energies->remainder.energy[Reactive_Cap_Imp] = loggedEnergies.remainder.reactive_cap_imp;
      energies->remainder.energy[Reactive_Cap_Exp] = loggedEnergies.remainder.reactive_cap_exp;
      energies->remainder.energy[Apparent_Imp] = loggedEnergies.remainder.apparent_imp;
      energies->remainder.energy[Apparent_Exp] = loggedEnergies.remainder.apparent_exp;
    }
}

unsigned char forword_cyclic(unsigned char val, unsigned char max)
{
	val=val+1;
	val=val%max;

	return val;
}

unsigned char back_cyclic(unsigned char val, unsigned char max)
{
    if (val == 0)
    {
        val = max;
    }
    val--;

    return val;
}

void read_energies_from_storage(EM_USER_ENERGY_COUNTER *energies)
{
    unsigned int arr_index, index;
    uint64_t temp_energy = 0;
    //read kwh, kvah, rkvah_lead,rkvah_lag from memory
    unsigned char checksum_fail_ctr, prev_checksum_fail_ctr;
    unsigned int reading_checksum, reading_checksum_calc;
    get_metering_mode();
    for(index=0;index<8;index++)
    {
        prev_checksum_fail_ctr=0;
        while(1)
        {
            checksum_fail_ctr=0;
            energies->stored.energy[index] = 0;
            energies->accumulated.energy[index] = 0;
            
            for(arr_index=0;arr_index<KWH_ARR;arr_index++)
            {
                temp_energy=from_eeprom((KWH_LOC+(index*KWH_ARR*KWH_VAL_SIZE)+(arr_index*KWH_VAL_SIZE)),KWH_VAL_SIZE);
                reading_checksum=from_eeprom((KWH_CHKSUM_LOC+(index*KWH_ARR*KWH_CHKSUM_SIZE)+(arr_index*KWH_CHKSUM_SIZE)),KWH_CHKSUM_SIZE);
                reading_checksum_calc=R_CRC_Calculate_Fresh((uint8_t*)&temp_energy,KWH_VAL_SIZE);
                if(reading_checksum_calc==reading_checksum)
                {
                    if(energies->stored.energy[index]<temp_energy)
                    {
                        energies->stored.energy[index]=temp_energy;
                        energies->index.value[index]=arr_index;
                    }
                }
                else
                {
                    checksum_fail_ctr++;
                }
            }

            if(checksum_fail_ctr!=prev_checksum_fail_ctr)
            {
                prev_checksum_fail_ctr=checksum_fail_ctr;
            }
            else
            {
                break;
            }
        }
    }
    read_page_eeprom(KWH_REMAINDER_VALUE_LOC, (uint8_t*)&energies->remainder, KWH_REMAINDER_VALUE_LOG_SIZE);
    if(energies->remainder.crc != R_CRC_Calculate_Fresh((uint8_t*)&energies->remainder, KWH_REMAINDER_VALUE_LOG_SIZE - 2))
    {
      memset((uint8_t*)&energies->remainder, 0, KWH_REMAINDER_VALUE_LOG_SIZE);
    }
    
    energies->total.energy[Active_Imp] = energies->stored.energy[0];
    energies->total.energy[Active_Exp] = energies->stored.energy[1];
    energies->total.energy[Reactive_Ind_Imp] = energies->stored.energy[2];
    energies->total.energy[Reactive_Ind_Exp] = energies->stored.energy[3];
    energies->total.energy[Reactive_Cap_Imp] = energies->stored.energy[4];
    energies->total.energy[Reactive_Cap_Exp] = energies->stored.energy[5];
    energies->total.energy[Apparent_Imp] = energies->stored.energy[6];
    energies->total.energy[Apparent_Exp] = energies->stored.energy[7];
    
    loggedEnergies.counter.active_imp = energies->stored.energy[0];
    loggedEnergies.counter.reactive_ind_imp = energies->stored.energy[2];
    loggedEnergies.counter.reactive_cap_imp = energies->stored.energy[4];
    loggedEnergies.counter.apparent_imp = energies->stored.energy[6];
    
    loggedEnergies.remainder.active_imp = energies->remainder.energy[0];
    loggedEnergies.remainder.reactive_ind_imp = energies->remainder.energy[2];
    loggedEnergies.remainder.reactive_cap_imp = energies->remainder.energy[4];
    loggedEnergies.remainder.apparent_imp = energies->remainder.energy[6];
    
    if (e_metering_mode == e_Import_Export)
    {
      loggedEnergies.counter.active_exp = energies->stored.energy[1];
      loggedEnergies.counter.reactive_ind_exp = energies->stored.energy[3];
      loggedEnergies.counter.reactive_cap_exp = energies->stored.energy[5];
      loggedEnergies.counter.apparent_exp = energies->stored.energy[7];
    
      loggedEnergies.remainder.active_exp = energies->remainder.energy[1];
      loggedEnergies.remainder.reactive_ind_exp = energies->remainder.energy[3];
      loggedEnergies.remainder.reactive_cap_exp = energies->remainder.energy[5];
      loggedEnergies.remainder.apparent_exp = energies->remainder.energy[7];
    }
    else
    {
      loggedEnergies.counter.active_exp = 0;
      loggedEnergies.counter.reactive_ind_exp = 0;
      loggedEnergies.counter.reactive_cap_exp = 0;
      loggedEnergies.counter.apparent_exp = 0;
    
      loggedEnergies.remainder.active_exp = 0;
      loggedEnergies.remainder.reactive_ind_exp = 0;
      loggedEnergies.remainder.reactive_cap_exp = 0;
      loggedEnergies.remainder.apparent_exp = 0;
    }
    SetEnergyCounter(&loggedEnergies);
    
//    if (((load_val[0] < 100) ||(load_val[1] < 100)) && (calib_flag == 0xAA) && (CAL_JUMP_IN & (1 << CAL_JUMP_PIN)) != 0) //todo Harjeet
//    { //Reset meter if meter number is set and reading is 0
//        WDTCTL = WDT_ARST_1_9;//once
//        while(1);
//    }
}

void write_energies_to_storage(EM_USER_ENERGY_COUNTER *energies, uint8_t forceWrite)
{
    unsigned int index;
    unsigned char loop;
    
    /*variables for validation*/
    unsigned char Last_reading_counter = 0, Last_reading_index;
    unsigned short Last_reading_checksum, Last_reading_checksum_calc;
    uint64_t Last_reading_val;
    
    static uint8_t log_energies = LOG_ENERGIES_ACCUMULATION_TIME;
    static const uint16_t accumulated_energy_count = ACCUMULATED_ENERGY_COUNT;
    
    if((log_energies-- == 0) || (forceWrite != 0))
    {
      log_energies = LOG_ENERGIES_ACCUMULATION_TIME;
      for (index = 0; index < 8; index++)
      {
        if(energies->accumulated.energy[index] > (accumulated_energy_count*(LOG_ENERGIES_ACCUMULATION_TIME + 1)))
        {
          energies->accumulated.energy[index] = 0;
          energies->total.energy[index] = energies->stored.energy[index];
          restore_energies_in_metrology(energies);
        }
        else if(energies->accumulated.energy[index] >= MINIMUM_ENERGY_COUNT)
        {
          energies->stored.energy[index] += energies->accumulated.energy[index];
          energies->accumulated.energy[index] = 0;
          /*Read last value*/
          Last_reading_index = energies->index.value[index];
          Last_reading_counter = 0;
          for (loop = 0; loop < MAX_BACKWARD_TRY; loop++)
          {
            Last_reading_val = from_eeprom((KWH_LOC + (index * KWH_ARR * KWH_VAL_SIZE) + (Last_reading_index * KWH_VAL_SIZE)), KWH_VAL_SIZE);
            Last_reading_checksum = from_eeprom(KWH_CHKSUM_LOC + (index * KWH_ARR * KWH_CHKSUM_SIZE) + (Last_reading_index * KWH_CHKSUM_SIZE), KWH_CHKSUM_SIZE);
            Last_reading_checksum_calc = R_CRC_Calculate_Fresh((uint8_t*)&Last_reading_val,KWH_VAL_SIZE);   //crc8(Last_reading_val);
            Last_reading_counter++;
            if (((energies->stored.energy[index] > Last_reading_val) && 
                (Last_reading_checksum == Last_reading_checksum_calc)) || 
                  (energies->stored.energy[index] < Last_reading_val) || (Last_reading_val == 0))
            {
                break;
            }
            /*Go back if any issue in the last reading*/
            Last_reading_index = back_cyclic(Last_reading_index, KWH_ARR);
          }
          /**/
          
          /*Error report if any*/
          if((energies->stored.energy[index] == 0) || ((Last_reading_counter == MAX_BACKWARD_TRY)) ||
             ((energies->stored.energy[index] - Last_reading_val) > ((accumulated_energy_count*(LOG_ENERGIES_ACCUMULATION_TIME + 1)) * 2 * Last_reading_counter))|| 
               (energies->stored.energy[index] < Last_reading_val))
          {
            //read_valid_issue++;
            //to_eeprom(READ_VALID_ISSUE_LOC, read_valid_issue, 4);
            //WDTCTL = WDT_ARST_1_9;//TODO:Rakesh WDTCTL
            while (1);
          }
          /**/
          
          for (loop = 0; loop < MAX_FORWARD_TRY; loop++)
          {
            /*Get new possition*/
            energies->index.value[index] = forword_cyclic(energies->index.value[index], KWH_ARR);
            /**/

            /*Write data to memory*/
            to_eeprom(KWH_LOC + (index * KWH_ARR * KWH_VAL_SIZE) + (energies->index.value[index] * KWH_VAL_SIZE), energies->stored.energy[index], KWH_VAL_SIZE);
            Last_reading_checksum = R_CRC_Calculate_Fresh((uint8_t*)&energies->stored.energy[index],KWH_VAL_SIZE);
            to_eeprom(KWH_CHKSUM_LOC + (index * KWH_ARR * KWH_CHKSUM_SIZE) + (energies->index.value[index] * KWH_CHKSUM_SIZE), Last_reading_checksum, KWH_CHKSUM_SIZE);
            /**/

            /*Verify written value*/
            Last_reading_val = from_eeprom((KWH_LOC + (index * KWH_ARR * KWH_VAL_SIZE) + (energies->index.value[index] * KWH_VAL_SIZE)), KWH_VAL_SIZE);
            Last_reading_checksum = from_eeprom(KWH_CHKSUM_LOC + (index * KWH_ARR * KWH_CHKSUM_SIZE) + (energies->index.value[index] * KWH_CHKSUM_SIZE), KWH_CHKSUM_SIZE);
            Last_reading_checksum_calc = R_CRC_Calculate_Fresh((uint8_t*)&Last_reading_val,KWH_VAL_SIZE);
            if ((energies->stored.energy[index] == Last_reading_val) && (Last_reading_checksum == Last_reading_checksum_calc))
            {
                break;
            }
            /**/
            
            /*Error report if any*/
            if (loop == (MAX_FORWARD_TRY - 1))
            {
                //write_valid_issue++;
                //to_eeprom(WRITE_VALID_ISSUE_LOC, write_valid_issue, 4);
                //WDTCTL = WDT_ARST_1_9;//TODO:Rakesh WDTCTL
                while (1);
            }
            /**/
          }
          /**/
        }
      }
      if(forceWrite)
      {
        /* Write remainder energies*/
        energies->remainder.crc = R_CRC_Calculate_Fresh((uint8_t*)&energies->remainder, KWH_REMAINDER_VALUE_LOG_SIZE - 2);
        write_page_eeprom(KWH_REMAINDER_VALUE_LOC, (uint8_t*)&energies->remainder, KWH_REMAINDER_VALUE_LOG_SIZE);
      }
    }
    if(log_energies > LOG_ENERGIES_ACCUMULATION_TIME)
    {
      log_energies = LOG_ENERGIES_ACCUMULATION_TIME;
    }
    return;
}

uint8_t get_metering_mode(void)
{
  read_page_eeprom(METERING_MODE_VAL_LOC, (uint8_t*)&e_metering_mode, METERING_MODE_VAL_SIZE);
  return (uint8_t)e_metering_mode;
}

void set_metering_mode(uint8_t metering_mode)
{
  if ((e_metering_mode != (e_metering_mode_type)metering_mode) && (((e_metering_mode_type)metering_mode == e_Import_Only) || ((e_metering_mode_type)metering_mode == e_Import_Export)))
  {
    GetEnergyCounter(&loggedEnergies);
    e_metering_mode = (e_metering_mode_type)metering_mode;
    write_page_eeprom(METERING_MODE_VAL_LOC, &metering_mode, METERING_MODE_VAL_SIZE);

    if (e_metering_mode == e_Import_Only)
    {
      myEnergies.accumulated.energy[Active_Exp] = loggedEnergies.counter.active_exp - myEnergies.stored.energy[Active_Exp];
      myEnergies.accumulated.energy[Reactive_Ind_Exp] = loggedEnergies.counter.reactive_ind_exp - myEnergies.stored.energy[Reactive_Ind_Exp];
      myEnergies.accumulated.energy[Reactive_Cap_Exp] = loggedEnergies.counter.reactive_cap_exp - myEnergies.stored.energy[Reactive_Cap_Exp];
      myEnergies.accumulated.energy[Apparent_Exp] = loggedEnergies.counter.apparent_exp - myEnergies.stored.energy[Apparent_Exp];
      myEnergies.remainder.energy[Active_Exp] = loggedEnergies.remainder.active_exp;
      myEnergies.remainder.energy[Reactive_Ind_Exp] = loggedEnergies.remainder.reactive_ind_exp;
      myEnergies.remainder.energy[Reactive_Cap_Exp] = loggedEnergies.remainder.reactive_cap_exp;
      myEnergies.remainder.energy[Apparent_Exp] = loggedEnergies.remainder.apparent_exp;
      
      loggedEnergies.counter.active_exp = 0;
      loggedEnergies.counter.reactive_ind_exp = 0;
      loggedEnergies.counter.reactive_cap_exp = 0;
      loggedEnergies.counter.apparent_exp = 0;
      loggedEnergies.remainder.active_exp = 0;
      loggedEnergies.remainder.reactive_ind_exp = 0;
      loggedEnergies.remainder.reactive_cap_exp = 0;
      loggedEnergies.remainder.apparent_exp = 0;
    }
    else if (e_metering_mode == e_Import_Export)
    {
      loggedEnergies.counter.active_imp = myEnergies.total.energy[Active_Imp];
      loggedEnergies.counter.reactive_ind_imp = myEnergies.total.energy[Reactive_Ind_Imp];
      loggedEnergies.counter.reactive_cap_imp = myEnergies.total.energy[Reactive_Cap_Imp];
      loggedEnergies.counter.apparent_imp = myEnergies.total.energy[Apparent_Imp];
      loggedEnergies.counter.active_exp = myEnergies.total.energy[Active_Exp];
      loggedEnergies.counter.reactive_ind_exp = myEnergies.total.energy[Reactive_Ind_Exp];
      loggedEnergies.counter.reactive_cap_exp = myEnergies.total.energy[Reactive_Cap_Exp];
      loggedEnergies.counter.apparent_exp = myEnergies.total.energy[Apparent_Exp];
      
      loggedEnergies.remainder.active_imp = myEnergies.remainder.energy[Active_Imp];
      loggedEnergies.remainder.reactive_ind_imp = myEnergies.remainder.energy[Reactive_Ind_Imp];
      loggedEnergies.remainder.reactive_cap_imp = myEnergies.remainder.energy[Reactive_Cap_Imp];
      loggedEnergies.remainder.apparent_imp = myEnergies.remainder.energy[Apparent_Imp];
      loggedEnergies.remainder.active_exp = myEnergies.remainder.energy[Active_Exp];
      loggedEnergies.remainder.reactive_ind_exp = myEnergies.remainder.energy[Reactive_Ind_Exp];
      loggedEnergies.remainder.reactive_cap_exp = myEnergies.remainder.energy[Reactive_Cap_Exp];
      loggedEnergies.remainder.apparent_exp = myEnergies.remainder.energy[Apparent_Exp];
    }
    write_energies_to_storage(&myEnergies, 1); //force write
    SetEnergyCounter(&loggedEnergies);
  }
}

void restore_energies_in_metrology(EM_USER_ENERGY_COUNTER *energies)
{
    loggedEnergies.counter.active_imp = energies->stored.energy[0];
    loggedEnergies.counter.reactive_ind_imp = energies->stored.energy[2];
    loggedEnergies.counter.reactive_cap_imp = energies->stored.energy[4];
    loggedEnergies.counter.apparent_imp = energies->stored.energy[6];
    
    loggedEnergies.remainder.active_imp = energies->remainder.energy[0];
    loggedEnergies.remainder.reactive_ind_imp = energies->remainder.energy[2];
    loggedEnergies.remainder.reactive_cap_imp = energies->remainder.energy[4];
    loggedEnergies.remainder.apparent_imp = energies->remainder.energy[6];
    if (e_metering_mode == e_Import_Export)
    {
      loggedEnergies.counter.active_exp = energies->stored.energy[1];
      loggedEnergies.counter.reactive_ind_exp = energies->stored.energy[3];
      loggedEnergies.counter.reactive_cap_exp = energies->stored.energy[5];
      loggedEnergies.counter.apparent_exp = energies->stored.energy[7];
    
      loggedEnergies.remainder.active_exp = energies->remainder.energy[1];
      loggedEnergies.remainder.reactive_ind_exp = energies->remainder.energy[3];
      loggedEnergies.remainder.reactive_cap_exp = energies->remainder.energy[5];
      loggedEnergies.remainder.apparent_exp = energies->remainder.energy[7];
    }
    else
    {
      loggedEnergies.counter.active_exp = 0;
      loggedEnergies.counter.reactive_ind_exp = 0;
      loggedEnergies.counter.reactive_cap_exp = 0;
      loggedEnergies.counter.apparent_exp = 0;
    
      loggedEnergies.remainder.active_exp = 0;
      loggedEnergies.remainder.reactive_ind_exp = 0;
      loggedEnergies.remainder.reactive_cap_exp = 0;
      loggedEnergies.remainder.apparent_exp = 0;
    }
    SetEnergyCounter(&loggedEnergies);
}