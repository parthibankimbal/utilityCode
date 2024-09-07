#include "em_energies_wrp.h"

void WPR_em_get_energies(EM_ENERGY_DATA *energies)
{
  double temp_dt;
  EM_OPERATION_DATA test_op_data = {0};
  
  EM_GetOperationData(&test_op_data);
  
  temp_dt = test_op_data.energy_counter.active_imp/((double)FACTORY_DEFAULT_METER_CONSTANT/1000.0);
  energies->counter.active_imp = (uint64_t)temp_dt;
  energies->remainder.active_imp = temp_dt - energies->counter.active_imp;
  energies->remainder.active_imp += (test_op_data.remainder.active_imp/((double)FACTORY_DEFAULT_METER_CONSTANT/1000.0))/0xFFFFFFFF;
  
  temp_dt = test_op_data.energy_counter.active_exp/((double)FACTORY_DEFAULT_METER_CONSTANT/1000.0);
  energies->counter.active_exp = (uint64_t)temp_dt;
  energies->remainder.active_exp = temp_dt - energies->counter.active_exp;
  energies->remainder.active_exp += (test_op_data.remainder.active_exp/((double)FACTORY_DEFAULT_METER_CONSTANT/1000.0))/0xFFFFFFFF;
  
  temp_dt = test_op_data.energy_counter.reactive_ind_imp/((double)FACTORY_DEFAULT_METER_CONSTANT/1000.0);
  energies->counter.reactive_ind_imp = (uint64_t)temp_dt;
  energies->remainder.reactive_ind_imp = temp_dt - energies->counter.reactive_ind_imp;
  energies->remainder.reactive_ind_imp += (test_op_data.remainder.reactive_ind_imp/((double)FACTORY_DEFAULT_METER_CONSTANT/1000.0))/0xFFFFFFFF;
  
  temp_dt = test_op_data.energy_counter.reactive_ind_exp/((double)FACTORY_DEFAULT_METER_CONSTANT/1000.0);
  energies->counter.reactive_ind_exp = (uint64_t)temp_dt;
  energies->remainder.reactive_ind_exp = temp_dt - energies->counter.reactive_ind_exp;
  energies->remainder.reactive_ind_exp += (test_op_data.remainder.reactive_ind_exp/((double)FACTORY_DEFAULT_METER_CONSTANT/1000.0))/0xFFFFFFFF;
  /*seems bug in lib cap_exp and cap_imp swapped*/
  /*either swap here or in em_energies*/
  temp_dt = test_op_data.energy_counter.reactive_cap_exp/((double)FACTORY_DEFAULT_METER_CONSTANT/1000.0);
  energies->counter.reactive_cap_imp = (uint64_t)temp_dt;
  energies->remainder.reactive_cap_imp = temp_dt - energies->counter.reactive_cap_imp;
  energies->remainder.reactive_cap_imp += (test_op_data.remainder.reactive_cap_exp/((double)FACTORY_DEFAULT_METER_CONSTANT/1000.0))/0xFFFFFFFF;
  /*seems bug in lib cap_exp and cap_imp swapped*/
  /*either swap here or in em_energies*/
  temp_dt = test_op_data.energy_counter.reactive_cap_imp/((double)FACTORY_DEFAULT_METER_CONSTANT/1000.0);
  energies->counter.reactive_cap_exp = (uint64_t)temp_dt;
  energies->remainder.reactive_cap_exp = temp_dt - energies->counter.reactive_cap_exp;
  energies->remainder.reactive_cap_exp += (test_op_data.remainder.reactive_cap_imp/((double)FACTORY_DEFAULT_METER_CONSTANT/1000.0))/0xFFFFFFFF;
  
  temp_dt = test_op_data.energy_counter.apparent_imp/((double)FACTORY_DEFAULT_METER_CONSTANT/1000.0);
  energies->counter.apparent_imp = (uint64_t)temp_dt;
  energies->remainder.apparent_imp = temp_dt - energies->counter.apparent_imp;
  energies->remainder.apparent_imp += (test_op_data.remainder.apparent_imp/((double)FACTORY_DEFAULT_METER_CONSTANT/1000.0))/0xFFFFFFFF;
  
  temp_dt = test_op_data.energy_counter.apparent_exp/((double)FACTORY_DEFAULT_METER_CONSTANT/1000.0);
  energies->counter.apparent_exp = (uint64_t)temp_dt;
  energies->remainder.apparent_exp = temp_dt - energies->counter.apparent_exp;
  energies->remainder.apparent_exp += (test_op_data.remainder.apparent_exp/((double)FACTORY_DEFAULT_METER_CONSTANT/1000.0))/0xFFFFFFFF;
}

void WPR_em_set_energies(EM_ENERGY_DATA *energies)
{
  double temp_dt;
  EM_OPERATION_DATA test_op_data = {0};
  
  temp_dt = ((double)energies->counter.active_imp + (double)energies->remainder.active_imp)*((double)FACTORY_DEFAULT_METER_CONSTANT/1000.0);
  test_op_data.energy_counter.active_imp = (uint64_t)temp_dt;
  test_op_data.remainder.active_imp = (temp_dt - (uint64_t)temp_dt)*0xFFFFFFFF;
  
  temp_dt = ((double)energies->counter.active_exp + (double)energies->remainder.active_exp)*((double)FACTORY_DEFAULT_METER_CONSTANT/1000.0);
  test_op_data.energy_counter.active_exp = (uint64_t)temp_dt;
  test_op_data.remainder.active_exp = (temp_dt - (uint64_t)temp_dt)*0xFFFFFFFF;
  
  temp_dt = ((double)energies->counter.reactive_ind_imp + (double)energies->remainder.reactive_ind_imp)*((double)FACTORY_DEFAULT_METER_CONSTANT/1000.0);
  test_op_data.energy_counter.reactive_ind_imp = (uint64_t)temp_dt;
  test_op_data.remainder.reactive_ind_imp = (temp_dt - (uint64_t)temp_dt)*0xFFFFFFFF;
  
  temp_dt = ((double)energies->counter.reactive_ind_exp + (double)energies->remainder.reactive_ind_exp)*((double)FACTORY_DEFAULT_METER_CONSTANT/1000.0);
  test_op_data.energy_counter.reactive_ind_exp = (uint64_t)temp_dt;
  test_op_data.remainder.reactive_ind_exp = (temp_dt - (uint64_t)temp_dt)*0xFFFFFFFF;
  /*seems bug in lib cap_exp and cap_imp swapped*/
  /*either swap here or in em_energies*/
  temp_dt = ((double)energies->counter.reactive_cap_imp + (double)energies->remainder.reactive_cap_imp)*((double)FACTORY_DEFAULT_METER_CONSTANT/1000.0);
  test_op_data.energy_counter.reactive_cap_exp = (uint64_t)temp_dt;
  test_op_data.remainder.reactive_cap_exp = (temp_dt - (uint64_t)temp_dt)*0xFFFFFFFF;
  /*seems bug in lib cap_exp and cap_imp swapped*/
  /*either swap here or in em_energies*/
  temp_dt = ((double)energies->counter.reactive_cap_exp + (double)energies->remainder.reactive_cap_exp)*((double)FACTORY_DEFAULT_METER_CONSTANT/1000.0);
  test_op_data.energy_counter.reactive_cap_imp = (uint64_t)temp_dt;
  test_op_data.remainder.reactive_cap_imp = (temp_dt - (uint64_t)temp_dt)*0xFFFFFFFF;
  
  temp_dt = ((double)energies->counter.apparent_imp + (double)energies->remainder.apparent_imp)*((double)FACTORY_DEFAULT_METER_CONSTANT/1000.0);
  test_op_data.energy_counter.apparent_imp = (uint64_t)temp_dt;
  test_op_data.remainder.apparent_imp = (temp_dt - (uint64_t)temp_dt)*0xFFFFFFFF;
  
  temp_dt = ((double)energies->counter.apparent_exp + (double)energies->remainder.apparent_exp)*((double)FACTORY_DEFAULT_METER_CONSTANT/1000.0);
  test_op_data.energy_counter.apparent_exp = (uint64_t)temp_dt;
  test_op_data.remainder.apparent_exp = (temp_dt - (uint64_t)temp_dt)*0xFFFFFFFF;
  
  EM_SetOperationData(&test_op_data);
}

void get_operational_EM_data(EM_OPERATION_DATA *test_op_data)
{
  EM_GetOperationData(test_op_data);
}

void set_operational_EM_data(EM_OPERATION_DATA *test_op_data)
{
  EM_SetOperationData(test_op_data);
}