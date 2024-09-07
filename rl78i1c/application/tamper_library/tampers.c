#include "tampers.h"
#include "main.h"
#include "eeprom_storage.h"
#include "SingleWireOp.h"
//TODO:Rakesh replace hard coded values with MACRO

Event_Type event_type;
St_tamper st_tamper;
static ST_MAGNET_STAT stMagnet;    //REVIEW: Make static... done
const unsigned short event_ids[] = {1,  3,  5,  7,  9,  11,  51,  53,  55,  57,  59,  61,  63,  65,  67,  69,  201,  203,  205,  207,  209,  215,  251};
ST_RTC_TIME_SYNC stRTCTimeSync;

uint32_t cyclic(uint32_t val, uint32_t max)
{
  val++;
  return val % max;
}

float32_t cal_percentage(float32_t percentage, float32_t value)
{
  float32_t  ret = (value * percentage) / 100;
  return ret;
}

float32_t max_current(void)
{
  uint8_t i;
  
  float32_t maximum_current = GET_U_REAL_PH_CURRENT();
  if (maximum_current < GET_U_REAL_NEU_CURRENT())
  {
    maximum_current = GET_U_REAL_NEU_CURRENT();
  }
  return maximum_current;
}

float32_t min_current(void)
{
  uint8_t i;
  float32_t minimum_current = GET_U_REAL_PH_CURRENT();
  if (minimum_current > GET_U_REAL_NEU_CURRENT())
  {
    minimum_current = GET_U_REAL_NEU_CURRENT();
  }
  return minimum_current;
}

float32_t current_unbalance(void)
{
  return max_current() - min_current();
}

float32_t max_voltage(void)
{
  return GET_U_REAL_PH_VOLTAGE();
}

float32_t min_voltage(void)
{
  return GET_U_REAL_PH_VOLTAGE();
}

float32_t voltage_unbalance(void)
{
  return max_voltage() - min_voltage();
}

void ac_magnet_test(void)
{
    if (((stMagnet.byMagSense != MAGNET_IN_STAT_1) || (stMagnet.byMagSense2 != MAGNET_IN_STAT_2)) && (IS_MAINS_PRESENT()))
    {
        stMagnet.byMagSense = MAGNET_IN_STAT_1;
        stMagnet.byMagSense2 = MAGNET_IN_STAT_2;
        stMagnet.wMagTime = 0;
        stMagnet.byMagPulsesCnt++;
    }
}

void get_over_voltage_tamper(void)
{
  float32_t maximum_voltage = max_voltage();
  *(uint32_t*)&st_tamper.tampers_bits.tamper_bits &= ~(1UL << TAMPER_OVER_VOLTAGE_STAT); //clear local event before checking live status
  if (maximum_voltage > cal_percentage(OVER_VOLT_PERCENT, V_BASIC))
  {
    *(uint32_t*)&st_tamper.tampers_bits.tamper_bits |= (1UL << TAMPER_OVER_VOLTAGE_STAT);
  }
  else if (((maximum_voltage > cal_percentage(OVER_VOLT_PERCENT, V_BASIC))
      /* || st_tamper.g_tampers_bits.bits.magnet_detected */)
    && ((*(uint32_t*)&st_tamper.g_tampers_bits.tamper_bits) & (1UL << TAMPER_OVER_VOLTAGE_STAT)))
  {
    *(uint32_t*)&st_tamper.tampers_bits.tamper_bits |= (1UL << TAMPER_OVER_VOLTAGE_STAT);
  }
}

void get_low_voltage_tamper(void)
{
  uint8_t i;
  *(uint32_t*)&st_tamper.tampers_bits.tamper_bits &= ~(1UL << TAMPER_LOW_VOLTAGE_STAT); //clear local event before checking live status

  if (GET_U_REAL_PH_VOLTAGE() < cal_percentage(LOW_VOLT_PERCENT, V_BASIC))
  {
    *(uint32_t*)&st_tamper.tampers_bits.tamper_bits |= (1UL << TAMPER_LOW_VOLTAGE_STAT);
  }
  else if (((GET_U_REAL_PH_VOLTAGE() < cal_percentage(LOW_VOLT_PERCENT, V_BASIC))
      /* || st_tamper.g_tampers_bits.bits.magnet_detected */)
    && ((*(uint32_t*)&st_tamper.g_tampers_bits.tamper_bits) & (1UL << TAMPER_LOW_VOLTAGE_STAT)))
  {
    *(uint32_t*)&st_tamper.tampers_bits.tamper_bits |= (1UL << TAMPER_LOW_VOLTAGE_STAT);
  }
}

void get_current_reverse_tamper(void)
{
  uint8_t i, is_rev = 0;
#ifdef SINGLE_PHASE_METER
  for (i = 0; i < 1; i++)
#else
  for (i = 0; i < 3; i++)
#endif
  {
    *(uint32_t*)&st_tamper.tampers_bits.tamper_bits &= ~(1UL << (i + TAMPER_R_PH_CURRENT_REVERSE_STAT)); //clear local event before checking live status
    if(e_metering_mode == e_Import_Only)
    {
#ifdef SINGLE_PHASE_METER
  #ifdef UTILITY_JNK
        if ((IS_REVERSE() == 1) && (GET_U_PH_VOLTAGE() > cal_percentage(60, V_BASIC))
            && (GET_U_ACTIVE_CURRENT() > cal_percentage(10, I_BASIC)))
  #elif ((UTILITY_INTELLI) || (UTILITY_APRAAVA) || (UTILITY_PURBANCHAL))
      if((fabs(TOTAL_REAL_ACTIVE_POWER()) > 48.0) && (IS_REVERSE() == 1))
  #endif
#else
      if ((IS_REVERSE() == 1) && (GET_U_REAL_PH_VOLTAGE() > cal_percentage(60, V_BASIC))
        && (GET_U_REAL_ACTIVE_CURRENT() > cal_percentage(10, I_BASIC)))
#endif
      {
        *(uint32_t*)&st_tamper.tampers_bits.tamper_bits |= (1UL << (i + TAMPER_R_PH_CURRENT_REVERSE_STAT));
        is_rev = 1;
      }
#ifdef SINGLE_PHASE_METER
  #ifdef UTILITY_JNK
      else if (((GET_U_PH_VOLTAGE() < cal_percentage(60, V_BASIC))
          || (GET_U_ACTIVE_CURRENT() < cal_percentage(10, I_BASIC)))
  #elif ((UTILITY_INTELLI) || (UTILITY_APRAAVA) || (UTILITY_PURBANCHAL))
      else if ((fabs(TOTAL_REAL_ACTIVE_POWER()) <= 48.0)
  #endif
#else
      else if (((GET_U_REAL_PH_VOLTAGE() < cal_percentage(60, V_BASIC))
        || (GET_U_REAL_ACTIVE_CURRENT() < cal_percentage(10, I_BASIC))
        /* || st_tamper.g_tampers_bits.bits.magnet_detected */)
#endif
        && ((*(uint32_t*)&st_tamper.g_tampers_bits.tamper_bits) & (1UL << (i + TAMPER_R_PH_CURRENT_REVERSE_STAT))))
      {
        *(uint32_t*)&st_tamper.tampers_bits.tamper_bits |= (1UL << (i + TAMPER_R_PH_CURRENT_REVERSE_STAT));
      }
    }
  }
  if(is_rev)
  {
    e_LCD_icon_status.reverse = 1;
  }
  else
  {
    e_LCD_icon_status.reverse = 0;
  }
}

void get_current_unbalance_or_earth_loading_tamper(void)
{
  float32_t current_ub = current_unbalance();
  float32_t maximum_current = max_current();
  float32_t minimum_current = min_current();
#ifdef SINGLE_PHASE_METER
  *(uint32_t*)&st_tamper.tampers_bits.tamper_bits &= ~(1UL << TAMPER_EARTH_LOADING_STAT); //clear local event before checking live status
 #ifdef UTILITY_JNK
  if ((current_ub > cal_percentage(10, maximum_current))
      && (maximum_current > cal_percentage(10, I_BASIC))
      && (min_voltage() > cal_percentage(60, V_BASIC)))
 #elif ((UTILITY_INTELLI) || (UTILITY_APRAAVA) || (UTILITY_PURBANCHAL))
      if ((current_ub > cal_percentage(4, maximum_current)) && (fabs(TOTAL_REAL_ACTIVE_POWER()) > 48.0))
 #endif
  {
    *(uint32_t*)&st_tamper.tampers_bits.tamper_bits |= (1UL << TAMPER_EARTH_LOADING_STAT);
  }
 #ifdef UTILITY_JNK
  else if (((current_ub > cal_percentage(10, maximum_current))
   || (maximum_current < cal_percentage(10, I_BASIC))
   || (min_voltage() < cal_percentage(60, V_BASIC))
   /* || st_tamper.g_tampers_bits.bits.magnet_detected */)
      && ((*(uint32_t*)&st_tamper.g_tampers_bits.tamper_bits) & (1UL << TAMPER_EARTH_LOADING_STAT)))
 #elif ((UTILITY_INTELLI) || (UTILITY_APRAAVA) || (UTILITY_PURBANCHAL))
  else if (((current_ub > cal_percentage(4, maximum_current))
      || (fabs(TOTAL_REAL_ACTIVE_POWER()) <= 48.0)
      /* || st_tamper.g_tampers_bits.bits.magnet_detected */)
      && ((*(uint32_t*)&st_tamper.g_tampers_bits.tamper_bits) & (1UL << TAMPER_EARTH_LOADING_STAT)))
 #endif
  {
    *(uint32_t*)&st_tamper.tampers_bits.tamper_bits |= (1UL << TAMPER_EARTH_LOADING_STAT);
  }
  if(st_tamper.tampers_bits.bits.earth_loading)
  {
    e_LCD_icon_status.earth_loading = 1;
  }
  else
  {
    e_LCD_icon_status.earth_loading = 0;
  }
#else
  *(uint32_t*)&st_tamper.tampers_bits.tamper_bits &= ~(1UL << TAMPER_CURRENT_UNBALANCE_STAT); //clear local event before checking live status
  if ((current_ub > cal_percentage(30, maximum_current))
    && (minimum_current > cal_percentage(10, I_BASIC))
    && (min_voltage() > cal_percentage(60, V_BASIC)))
  {
    *(uint32_t*)&st_tamper.tampers_bits.tamper_bits |= (1UL << TAMPER_CURRENT_UNBALANCE_STAT);
  }
  else if (((current_ub > cal_percentage(30, maximum_current))
    || (minimum_current < cal_percentage(10, I_BASIC))
    || (min_voltage() < cal_percentage(60, V_BASIC))
      /* || st_tamper.g_tampers_bits.bits.magnet_detected */)
    && ((*(uint32_t*)&st_tamper.g_tampers_bits.tamper_bits) & (1UL << TAMPER_CURRENT_UNBALANCE_STAT)))
  {
    *(uint32_t*)&st_tamper.tampers_bits.tamper_bits |= (1UL << TAMPER_CURRENT_UNBALANCE_STAT);
  }
#endif
}

void get_over_current_tamper(void)
{
  *(uint32_t*)&st_tamper.tampers_bits.tamper_bits &= ~(1UL << TAMPER_OVER_CURRENT_STAT); //clear local event before checking live status
  if(relay_output_state() == eLATCH_CLOSE)
  {
    if ((max_current() > st_tamper.over_current_value)
        /* || (st_tamper.g_tampers_bits.bits.over_current
       && st_tamper.g_tampers_bits.bits.magnet_detected) */)
    {
      *(uint32_t*)&st_tamper.tampers_bits.tamper_bits |= (1UL << TAMPER_OVER_CURRENT_STAT);
    }
  }
  else if(st_tamper.g_tampers_bits.bits.over_current)
  {
    *(uint32_t*)&st_tamper.tampers_bits.tamper_bits |= (1UL << TAMPER_OVER_CURRENT_STAT);
  }
  st_over_limit_tamper_status.over_current_live = st_tamper.tampers_bits.bits.over_current;
  st_over_limit_tamper_status.over_current_registered = st_tamper.g_tampers_bits.bits.over_current;
}

void get_over_load_tamper(void)
{
  *(uint32_t*)&st_tamper.tampers_bits.tamper_bits &= ~(1UL << TAMPER_OVER_LOAD_STAT); //clear local event before checking live status
  if(relay_output_state() == eLATCH_CLOSE)
  {
    if((fabs(TOTAL_REAL_ACTIVE_POWER()) > st_tamper.over_load_value)
        /* || (st_tamper.g_tampers_bits.bits.over_load
        && st_tamper.g_tampers_bits.bits.magnet_detected )*/)
    {
      *(uint32_t*)&st_tamper.tampers_bits.tamper_bits |= (1UL << TAMPER_OVER_LOAD_STAT);
    }
  }
  else if(st_tamper.g_tampers_bits.bits.over_load)
  {
    *(uint32_t*)&st_tamper.tampers_bits.tamper_bits |= (1UL << TAMPER_OVER_LOAD_STAT);
  }
  st_over_limit_tamper_status.over_load_live = st_tamper.tampers_bits.bits.over_load;
  st_over_limit_tamper_status.over_load_registered = st_tamper.g_tampers_bits.bits.over_load;
}

void get_magnet_tamper(void)
{
  *(uint32_t*)&st_tamper.tampers_bits.tamper_bits &= ~(1UL << TAMPER_MAGNET_STAT); //clear local event before checking live status
  if (stMagnet.wMagTime < 15)
  {
      stMagnet.wMagTime++;
  }
  else
  {
      stMagnet.wMagTime = 15;
      stMagnet.byMagPulsesCnt = 0;
      stMagnet.byMagStat = 0;
  }
  if (stMagnet.byMagPulsesCnt >= 10)//if (stMagnet.byMagPulsesCnt >= 6)
  {
      stMagnet.byMagPulsesCnt = 10;
      //stMagnet.byMagPulsesCnt = 6;
      stMagnet.byMagStat = 1;
  }
  if (((MAGNET_1_APPLIED && MAGNET_2_APPLIED) || (0 != stMagnet.byMagStat)) && (IS_MAINS_PRESENT()))
  {
    e_LCD_icon_status.magnet = 1;
    if (is_serial_number_updated(0))
    {
        *(uint32_t*)&st_tamper.tampers_bits.tamper_bits |= (1UL << TAMPER_MAGNET_STAT);
    }
  }
  else
  {
    e_LCD_icon_status.magnet = 0;
  }
}

extern uint8_t sample_capature_start, sample_ready;
extern int32_t datalogBuf_v[];
uint8_t g_byNeuCurPresent = 0;
float g_fPhaseCurrent = 0;
    uint8_t res_cap_ND_detected = 0;
    uint8_t DC_ND_detected = 0;
uint8_t check_res_cap_ND(void)
{
    static int16_t counter = 0;
    
    uint8_t i, max_index = 0;
    int32_t max = 0, last;
    if (sample_capature_start < 10)
    {
        sample_capature_start++;
        if (0 != st_tamper.g_tampers_bits.bits.neutral_disturbance)
        {
            counter = ND_DETECTION_TIME;
        }
        else
        {
            counter = -ND_DETECTION_TIME;
        }
    }
    g_fPhaseCurrent = GET_U_REAL_PH_CURRENT();
    if (sample_ready)
    {
        for (i = 0; i < 40; i++)
        {
            if (max < datalogBuf_v[i])
            {
                max = datalogBuf_v[i];
                max_index = i;
            }
        }
        if ((GET_U_REAL_NEU_CURRENT() < 0.05))
        {
            g_byNeuCurPresent = 0;
        }
        else
        {
            g_byNeuCurPresent = 1;
        }
        if ((max_index < 17) && (0 == g_byNeuCurPresent))
        {
            if (counter < ND_DETECTION_TIME)
            {
                counter++;
            }
            for (i = max_index; i < 37; i++)
            {
                if (datalogBuf_v[i] < datalogBuf_v[i + 1])
                {
                    counter = -ND_DETECTION_TIME;
                }
            }
        }
        else if ((counter > -ND_DETECTION_TIME))
        {
            counter--;
        }

        if (counter >= ND_DETECTION_TIME)
        {
            res_cap_ND_detected = 1;
        }
        else if (counter <= -ND_DETECTION_TIME)
        {
            res_cap_ND_detected = 0;
        }
        sample_ready = 0;
    }

    if (g_dc_detected_count >= DC_DETECTION_COUNT_THRESHOLD)
    {
        DC_ND_detected = 1;
    }
    else if (g_dc_detected_count <= -DC_RESTORATION_COUNT_THRESHOLD)
    {
        DC_ND_detected = 0;
    }
    return (res_cap_ND_detected || DC_ND_detected);
}
void get_neutral_disturbance(void)
{
  *(uint32_t*)&st_tamper.tampers_bits.tamper_bits &= ~(1UL << TAMPER_NEUTRAL_DISTURBANCE_STAT); //clear local event before checking live status
  
  if(check_res_cap_ND() && (GET_U_REAL_ACTIVE_CURRENT() > NEUTRAL_DISTURBANCE_CHK_CURRENT))
  {
    *(uint32_t*)&st_tamper.tampers_bits.tamper_bits |= (1UL << TAMPER_NEUTRAL_DISTURBANCE_STAT);
  }
  else if((check_res_cap_ND() || (GET_U_REAL_ACTIVE_CURRENT() <= NEUTRAL_DISTURBANCE_CHK_CURRENT) /* || st_tamper.g_tampers_bits.bits.magnet_detected */)
    && st_tamper.g_tampers_bits.bits.neutral_disturbance)
  {
    *(uint32_t*)&st_tamper.tampers_bits.tamper_bits |= (1UL << TAMPER_NEUTRAL_DISTURBANCE_STAT);
  }
}

void get_single_wire_operation(void)
{
    static uint8_t pulse_disabled = 0;
    *(uint32_t*)&st_tamper.tampers_bits.tamper_bits &= ~(1UL << TAMPER_SINGLE_WIRE_STAT);
    e_LCD_icon_status.single_wire = 0;

    if((IS_NMISS_ACTIVE() || ((GET_U_REAL_PH_VOLTAGE() < 10) && (GET_U_REAL_ACTIVE_CURRENT() > 0.55))))
    {
      *(uint32_t*)&st_tamper.tampers_bits.tamper_bits |= (1UL << TAMPER_SINGLE_WIRE_STAT);
      e_LCD_icon_status.single_wire = 1;
      if (0 == g_uSingleWirePulsesEnable)       //REVIEW: this flag use?
      {
          singleWireDisablePulses();
          //singleWireDisableDisplay();
          pulse_disabled = 1;
      }
    }
    else if ((IS_NMISS_ACTIVE() || (GET_U_REAL_ACTIVE_CURRENT() < 0.55)
        /* || st_tamper.g_tampers_bits.bits.magnet_detected */)
      && ((*(uint32_t*)&st_tamper.g_tampers_bits.tamper_bits) & (1UL << TAMPER_SINGLE_WIRE_STAT)))
    {
      *(uint32_t*)&st_tamper.tampers_bits.tamper_bits |= (1UL << TAMPER_SINGLE_WIRE_STAT);
    }
    else if(pulse_disabled)
    {
      pulse_disabled = 0;
      singleWireEnablePulses();
      singleWireEnableDisplay();
      g_uSingleWirePulsesEnable = 0;
    }
}

void get_low_PF_tamper(void)
{
  uint8_t i;
  float pf;
  
  *(uint32_t*)&st_tamper.tampers_bits.tamper_bits &= ~(1UL << TAMPER_LOW_PF_STAT); //clear local event before checking live status
  
  pf = fabs(GET_REAL_PF());
  if (
    (pf < cal_percentage(LOW_PF_VAL, UNITY_PF))
    && (GET_U_REAL_ACTIVE_CURRENT() > cal_percentage(10, I_BASIC))
    && (GET_U_REAL_PH_VOLTAGE() > cal_percentage(60, V_BASIC))
    )
  {
    *(uint32_t*)&st_tamper.tampers_bits.tamper_bits |= (1L << TAMPER_LOW_PF_STAT);
  }
  else if (((pf < cal_percentage(60, UNITY_PF))
    || (GET_U_REAL_ACTIVE_CURRENT() < cal_percentage(10, I_BASIC))
    || (GET_U_REAL_PH_VOLTAGE() < cal_percentage(60, V_BASIC))
    /* || st_tamper.g_tampers_bits.bits.magnet_detected */)
    && ((*(uint32_t*)&st_tamper.g_tampers_bits.tamper_bits) & (1UL << TAMPER_LOW_PF_STAT)))
  {
    *(uint32_t*)&st_tamper.tampers_bits.tamper_bits |= (1UL << TAMPER_LOW_PF_STAT);
  }
}

void get_cover_open_tamper(uint32_t epoch)
{
  *(uint32_t*)&st_tamper.tampers_bits.tamper_bits &= ~(1UL << TAMPER_METER_COVER_OPEN_STAT); //clear local event before checking live status
  if(is_serial_number_updated(0))
  {
    if (st_tamper.g_tampers_bits.bits.cover_open != 1)
    {
      //if (st_tamper.St_cover_open.enable_time < epoch)        //REVIEW: removed as cover open occurs only after serial number is updated and serial number is updated only after cover is fixed
      {
        if (IS_COVER_OPEN())
        {
          *(uint32_t*)&st_tamper.tampers_bits.tamper_bits |= (1UL << TAMPER_METER_COVER_OPEN_STAT);
        }
      }
    }
    else
    {
      *(uint32_t*)&st_tamper.tampers_bits.tamper_bits |= (1UL << TAMPER_METER_COVER_OPEN_STAT);
    }
  }
}

void get_RF_removal_tamper(uint32_t epoch)
{
  *(uint32_t*)&st_tamper.tampers_bits.tamper_bits &= ~(1UL << TAMPER_RF_REMOVAL_STAT); //clear local event before checking live status
  if ((IS_RF_LINK_MISSING()) && (is_serial_number_updated(0)))
  {
    *(uint32_t*)&st_tamper.tampers_bits.tamper_bits |= (1UL << TAMPER_RF_REMOVAL_STAT);
  }
}

void filter_events(void)
{
//if nd dont occur   vm,hv,lv,vub
    if(st_tamper.tampers_bits.bits.neutral_disturbance)
    {
      if(st_tamper.g_tampers_bits.bits.r_ph_volt_missing == 0)
      {
        st_tamper.tampers_bits.bits.r_ph_volt_missing = 0;
      }
      if(st_tamper.g_tampers_bits.bits.y_ph_volt_missing == 0)
      {
        st_tamper.tampers_bits.bits.y_ph_volt_missing = 0;
      }
      if(st_tamper.g_tampers_bits.bits.b_ph_volt_missing == 0)
      {
        st_tamper.tampers_bits.bits.b_ph_volt_missing = 0;
      }
      if(st_tamper.g_tampers_bits.bits.over_volt == 0)
      {
        st_tamper.tampers_bits.bits.over_volt = 0;
      }
      if(st_tamper.g_tampers_bits.bits.low_volt == 0)
      {
        st_tamper.tampers_bits.bits.low_volt = 0;
      }
      if(st_tamper.g_tampers_bits.bits.volt_unbalance == 0)
      {
        st_tamper.tampers_bits.bits.volt_unbalance = 0;
      }
    }
//if  vm, dont occur iub
//if  vm, dont occur  lv,vub
	if (st_tamper.tampers_bits.bits.r_ph_volt_missing
           || st_tamper.tampers_bits.bits.y_ph_volt_missing
           || st_tamper.tampers_bits.bits.b_ph_volt_missing)
	{
          if (st_tamper.g_tampers_bits.bits.current_unbalance == 0)
          {
            st_tamper.tampers_bits.bits.current_unbalance = 0;
          }
          if (st_tamper.g_tampers_bits.bits.low_volt == 0)
          {
            st_tamper.tampers_bits.bits.low_volt = 0;
          }
          if (st_tamper.g_tampers_bits.bits.volt_unbalance == 0)
          {
            st_tamper.tampers_bits.bits.volt_unbalance = 0;
          }
	}
//if  vm, dont occur im
	if (st_tamper.tampers_bits.bits.r_ph_volt_missing)
	{
          if (st_tamper.g_tampers_bits.bits.r_ph_volt_missing == 0)
          {
            st_tamper.tampers_bits.bits.r_ph_current_missing = 0;
          }
	}
//if  vm, dont occur im
	if (st_tamper.tampers_bits.bits.y_ph_volt_missing)
	{
          if (st_tamper.g_tampers_bits.bits.y_ph_volt_missing == 0)
          {
            st_tamper.tampers_bits.bits.y_ph_current_missing = 0;
          }
	}
//if  vm, dont occur im
	if (st_tamper.tampers_bits.bits.b_ph_volt_missing)
	{
         if (st_tamper.g_tampers_bits.bits.b_ph_volt_missing == 0)
         {
           st_tamper.tampers_bits.bits.b_ph_current_missing = 0;
          }
	}

//if  hv,lv dont occur vub
	if ((st_tamper.tampers_bits.bits.over_volt) || (st_tamper.tampers_bits.bits.low_volt))
	{
          if (st_tamper.g_tampers_bits.bits.volt_unbalance == 0)
          {
            st_tamper.tampers_bits.bits.volt_unbalance = 0;
          }
	}
//if im dont occur iub
//if over current dont occur iub
	if (st_tamper.tampers_bits.bits.r_ph_current_missing
           || st_tamper.tampers_bits.bits.y_ph_current_missing
           || st_tamper.tampers_bits.bits.b_ph_current_missing
           || st_tamper.tampers_bits.bits.over_current)
	{
        if(st_tamper.g_tampers_bits.bits.current_unbalance == 0)
        {
          st_tamper.tampers_bits.bits.current_unbalance = 0;
        }
    }
}

void generate_tamper_warning(void)
{
    uint32_t tamper_ptr = st_tamper.g_tampers_bits.tamper_bits;
    
    if((tamper_ptr & (1L << TAMPER_R_PH_VOLTAGE_MISSING_STAT)) == (1L << TAMPER_R_PH_VOLTAGE_MISSING_STAT))
    {
      e_LCD_icon_status.warning = 1;
    }
    else if((tamper_ptr & (1L << TAMPER_Y_PH_VOLTAGE_MISSING_STAT)) == (1L << TAMPER_Y_PH_VOLTAGE_MISSING_STAT))
    {
      e_LCD_icon_status.warning = 1;
    }
    else if((tamper_ptr & (1L << TAMPER_B_PH_VOLTAGE_MISSING_STAT)) == (1L << TAMPER_B_PH_VOLTAGE_MISSING_STAT))
    {
      e_LCD_icon_status.warning = 1;
    }
    else if((tamper_ptr & (1L << TAMPER_OVER_VOLTAGE_STAT)) == (1L << TAMPER_OVER_VOLTAGE_STAT))
    {
      e_LCD_icon_status.warning = 1;
    }
    else if((tamper_ptr & (1L << TAMPER_LOW_VOLTAGE_STAT)) == (1L << TAMPER_LOW_VOLTAGE_STAT))
    {
      e_LCD_icon_status.warning = 1;
    }
    else if((tamper_ptr & (1L << TAMPER_VOLTAGE_UNBALANCE)) == (1L << TAMPER_VOLTAGE_UNBALANCE))
    {
      e_LCD_icon_status.warning = 1;
    }
    else if((tamper_ptr & (1L << TAMPER_R_PH_CURRENT_MISSING_STAT)) == (1L << TAMPER_R_PH_CURRENT_MISSING_STAT))
    {
      e_LCD_icon_status.warning = 1;
    }
    else if((tamper_ptr & (1L << TAMPER_Y_PH_CURRENT_MISSING_STAT)) == (1L << TAMPER_Y_PH_CURRENT_MISSING_STAT))
    {
      e_LCD_icon_status.warning = 1;
    }
    else if((tamper_ptr & (1L << TAMPER_B_PH_CURRENT_MISSING_STAT)) == (1L << TAMPER_B_PH_CURRENT_MISSING_STAT))
    {
      e_LCD_icon_status.warning = 1;
    }
    else if((tamper_ptr & (1 << TAMPER_CURRENT_UNBALANCE_STAT)) == (1L << TAMPER_CURRENT_UNBALANCE_STAT))
    {
      e_LCD_icon_status.warning = 1;
    }
    else if((tamper_ptr & (1 << TAMPER_OVER_CURRENT_STAT)) == (1L << TAMPER_OVER_CURRENT_STAT))
    {
      e_LCD_icon_status.warning = 1;
    }
    else if((tamper_ptr & (1L << TAMPER_OVER_LOAD_STAT)) == (1L << TAMPER_OVER_LOAD_STAT))
    {
      e_LCD_icon_status.warning = 1;
    }
    else if((tamper_ptr & (1L << TAMPER_LOW_PF_STAT)) == (1L << TAMPER_LOW_PF_STAT))
    {
      e_LCD_icon_status.warning = 1;
    }
//    else if((tamper_ptr & (1L << TAMPER_METER_LOAD_CONNECTION_STAT)) == (1L << TAMPER_METER_LOAD_CONNECTION_STAT))
//    {
//      e_LCD_icon_status.warning = 1;
//    }
    else if((tamper_ptr & (1L << TAMPER_CURRENT_BYPASS_STAT)) == (1L << TAMPER_CURRENT_BYPASS_STAT))
    {
      e_LCD_icon_status.warning = 1;
    }
    else if((tamper_ptr & (1L << TAMPER_NEUTRAL_DISTURBANCE_STAT)) == (1L << TAMPER_NEUTRAL_DISTURBANCE_STAT))
    {
      e_LCD_icon_status.warning = 1;
    }
    else
    {
      e_LCD_icon_status.warning = 0;
    }
}

#if defined SINGLE_PHASE_METER
void startDummyPower(void)
{
    EM_SET_MAGNET_TAMPER(FALSE);
    DUMMY_POWER_START(FALSE);
    if ((GET_U_REAL_PH_VOLTAGE() < 10.0) && (GET_U_ACTIVE_CURRENT() < 0.020))
    {
        return;
    }
    if ((((MAGNET_1_APPLIED && MAGNET_2_APPLIED) || (0 != stMagnet.byMagStat)) && (IS_MAINS_PRESENT())) && (is_serial_number_updated(0)))
    {
        EM_SET_MAGNET_TAMPER(TRUE);
        DUMMY_POWER_START(TRUE);
    }
    if (st_tamper.tampers_bits.bits.neutral_disturbance)
    {
        DUMMY_POWER_START(TRUE);
    }
    if (st_tamper.tampers_bits.bits.single_wire)
    {
        DUMMY_POWER_START(TRUE);
    }
}
#endif
//check tamper every second
void call_tamper_func(uint32_t epoch, uint8_t mains_stat)
{
  uint8_t i;
  Event_Type tmp_event_type;
  static uint32_t last_epoch = 0;

  if (last_epoch != epoch)
  {
    if(mains_stat)
    {
#ifndef SINGLE_PHASE_METER
      get_voltage_missing_tamper();
      get_voltage_unbalance_tamper();
      get_current_miss_tamper();
      get_current_bypass_tamper();
#endif
      get_over_voltage_tamper();
      get_low_voltage_tamper();
      get_current_reverse_tamper();
      get_current_unbalance_or_earth_loading_tamper();
      get_over_current_tamper();
      get_over_load_tamper();
#if defined WHOLE_CURRENT_METER || defined SINGLE_PHASE_METER
      load_control_polling_process(st_tamper.g_tampers_bits.bits.over_load, st_tamper.g_tampers_bits.bits.over_current);
#endif
      get_magnet_tamper();
      get_neutral_disturbance();
      get_low_PF_tamper();
      filter_events();
      switch_weld_tamper_scan(epoch);
    }
#if defined SINGLE_PHASE_METER
    //if(mains_stat == 0)
    //{
      get_single_wire_operation();
    //}
#endif
    is_cover_open();
    get_RF_removal_tamper(epoch);
    get_cover_open_tamper(epoch);
    generate_tamper_warning();
    
#if defined SINGLE_PHASE_METER
    startDummyPower();
#endif

    //Event registration
    for (i = 0; i < (uint8_t)TOTAL_EVENT_TYPES; i++)
    {
      if(!mains_stat)
      {
        if((i == TAMPER_METER_COVER_OPEN_STAT) || (i == TAMPER_RF_REMOVAL_STAT) || (i == TAMPER_SINGLE_WIRE_STAT))
        {
          ;
        }
        else
        {
          continue;
        }
      }
      if (i <= TAMPER_VOLTAGE_UNBALANCE) //voltage tamper
      {
        tmp_event_type = VOLT_EVENT;
      }
      else if (i <= TAMPER_EARTH_LOADING_STAT) //current tamper
      {
        tmp_event_type = AMP_EVENT;
      }
      else if (i <= TAMPER_OVER_LOAD_STAT) //other tamper
      {
        tmp_event_type = OTHER_EVENT;
      }
      else if (i == TAMPER_METER_COVER_OPEN_STAT) //other tamper
      {
        tmp_event_type = NOROLL_EVENT;
      }

      if ((*(uint32_t*)&st_tamper.tampers_bits.tamper_bits & (1UL << i)) == (1UL << i))
      {
        st_tamper._event_restoration_time[i] = 0;
        if (st_tamper._event_presistent_time[i] < st_tamper.event_presistent_time[i])
        {
          st_tamper._event_presistent_time[i]++;
        }
        else if ((*(uint32_t*)&st_tamper.tampers_bits.tamper_bits & (1UL << i)) != (*(uint32_t*)&st_tamper.g_tampers_bits.tamper_bits & (1UL << i)))
        {
          //store Tamper
          *(uint32_t*)&st_tamper.g_tampers_bits.tamper_bits |= (1UL << i);
          
          if((i == TAMPER_OVER_LOAD_STAT) || (i == TAMPER_OVER_CURRENT_STAT))
          {
            setLocalControl();
          }
          to_eeprom(G_TAMPER_STATUS_BITS_LOC, *(uint32_t*)&st_tamper.g_tampers_bits.tamper_bits, G_TAMPER_STATUS_BITS_SIZE);
          if(tmp_event_type != NOROLL_EVENT)
          {
            to_eeprom(TOTAL_TAMPERS_COUNT_LOC, from_eeprom(TOTAL_TAMPERS_COUNT_LOC, TOTAL_TAMPERS_COUNT_SIZE) + 1, TOTAL_TAMPERS_COUNT_SIZE);
          }
          store_event_data(tmp_event_type, event_ids[i], EPOCH);
        }
      }
      else
      {
        st_tamper._event_presistent_time[i] = 0;
        if (st_tamper._event_restoration_time[i] < st_tamper.event_restoration_time[i])
        {
          st_tamper._event_restoration_time[i]++;
        }
        else if ((*(uint32_t*)&st_tamper.tampers_bits.tamper_bits & (1UL << i)) != (*(uint32_t*)&st_tamper.g_tampers_bits.tamper_bits & (1UL << i)))
        {
          //restore Tamper
          *(uint32_t*)&st_tamper.g_tampers_bits.tamper_bits &= ~(1UL << i);
          to_eeprom(G_TAMPER_STATUS_BITS_LOC, *(uint32_t*)&st_tamper.g_tampers_bits.tamper_bits, G_TAMPER_STATUS_BITS_SIZE);
          store_event_data(tmp_event_type, event_ids[i] + 1, EPOCH);
          if((i == TAMPER_OVER_LOAD_STAT) || (i == TAMPER_OVER_CURRENT_STAT))
          {
            st_over_limit_tamper_status.over_load_registered = st_tamper.g_tampers_bits.bits.over_load;
            st_over_limit_tamper_status.over_current_registered = st_tamper.g_tampers_bits.bits.over_current;
            resetLocalControl();
          }
        }
      }
    }
  }
  return;
}

void store_event_data(Event_Type event_type, unsigned short event_id, uint32_t event_time)
{
  
  ONE_EVENT_ENERGY_DATA_LOG profile_log;//st in datalog.h

  if ((event_id % 2) && ((event_id < 151) || (event_id >= 201)))
  {
    st_latest_event latest_event;
    latest_event.id = event_id;
    latest_event.time = event_time;
    
    write_page_eeprom(LATEST_EVENT_OCCURED_LOC, (uint8_t *)&latest_event, LATEST_EVENT_OCCURED_SIZE);
  }

  else if (!(event_id % 2) && ((event_id < 151) || (event_id >= 201)))
  {
    st_latest_event latest_event;
    latest_event.id = event_id;
    latest_event.time = event_time;
    
    write_page_eeprom(LATEST_EVENT_RESTORED_LOC, (uint8_t *)&latest_event, LATEST_EVENT_RESTORED_SIZE);
  }
  
  if(event_id != 101)
  {
    st_tamper.Trigger_Byte |= _BV(event_type);
  }

  if(event_type == PFAIL_EVENT)
  {
    if(event_id == 101)
    {
      st_tamper.g_tampers_bits.bits.first_breath = 0;
      st_tamper.g_tampers_bits.bits.last_gasp = 1;
    }
    else if(event_id == 102)
    {
      st_tamper.g_tampers_bits.bits.first_breath = 1;
      st_tamper.g_tampers_bits.bits.last_gasp = 0;
    }
    to_eeprom(G_TAMPER_STATUS_BITS_LOC, *(uint32_t*)&st_tamper.g_tampers_bits.tamper_bits, G_TAMPER_STATUS_BITS_SIZE);
  }
  if (event_type == TRANSACT_EVENT)
  {
    to_eeprom(TOTAL_PROGRAMMING_COUNT_LOC, from_eeprom(TOTAL_PROGRAMMING_COUNT_LOC, TOTAL_PROGRAMMING_COUNT_SIZE) + 1, TOTAL_PROGRAMMING_COUNT_SIZE);
  }
  
  if ((event_type == VOLT_EVENT) || (event_type == AMP_EVENT) || (event_type == OTHER_EVENT))
  {
    //REVIEW: Rakesh, log_energies_callback
    profile_log.u16_Tamper_ID = event_id;
    profile_log.u32_epoch = event_time;
    profile_log.u16_volt = (uint16_t)(GET_U_PH_VOLTAGE()*100);
    profile_log.u32_current_phase = (uint32_t)(GET_U_PH_CURRENT()*100);
    profile_log.u32_current_neutral = (uint32_t)(GET_U_NEU_CURRENT()*100);
    profile_log.u8_pf = (uint8_t)(fabs(GET_PF())*100);
    profile_log.u64_energy_val[0] = GET_CUMULATIVE_ENERGY(Active_Imp);
    profile_log.u64_energy_val[1] = GET_CUMULATIVE_ENERGY(Apparent_Imp);
    profile_log.u64_kwh_expo = GET_CUMULATIVE_ENERGY(Active_Exp);
    profile_log.u32_tamper_count = get_tamper_counts();
  }
  else
  {
    profile_log.u16_Tamper_ID = event_id;
    profile_log.u32_epoch = event_time;
  }
  if (((event_type == VOLT_EVENT) || (event_type == AMP_EVENT) || (event_type == OTHER_EVENT) || (event_type == NOROLL_EVENT))
      && (!((event_id == 211) || (event_id == 213)))
      && ((event_id % 2) != 0))

  {
      write_page_eeprom(STORAGE_EEPROM_DLMS_LAST_TAMPER_OCCUR_ADDR, (uint8_t*)&profile_log, STORAGE_EEPROM_DLMS_LAST_TAMPER_OCCUR_SIZE);
  }
  else if (((event_type == VOLT_EVENT) || (event_type == AMP_EVENT) || (event_type == OTHER_EVENT))
      && (!((event_id == 212) || (event_id == 214)))
      && ((event_id % 2) == 0))

  {
      write_page_eeprom(STORAGE_EEPROM_DLMS_LAST_TAMPER_RESTORE_ADDR, (uint8_t*)&profile_log, STORAGE_EEPROM_DLMS_LAST_TAMPER_RESTORE_SIZE);
  }
  R_DLMS_DataLog_SetEntry((tag_dlms_profile_type_t) event_type, (uint8_t*)&profile_log);
  ESW_function();

  return;
}

void log_bill_bit(uint8_t set_bit_value)
{
  st_tamper.g_tampers_bits.bits.billing_counter_increment = set_bit_value;
  to_eeprom(G_TAMPER_STATUS_BITS_LOC, *(uint32_t*)&st_tamper.g_tampers_bits.tamper_bits, G_TAMPER_STATUS_BITS_SIZE);
  ESW_function();
}

void ESW_function(void)
{
  uint8_t i;
  uint8_t ESW_Push_Word_128[16];
  uint8_t ESW_BAK_Word_128[16];
  uint8_t ESWF_Word_128[16];
  uint8_t ESW_Word_128[16] = {0};
  
  read_page_eeprom(ESW_BAK_WORD_LOC, ESW_BAK_Word_128, 16);
  //0
  if (st_tamper.g_tampers_bits.bits.r_ph_volt_missing)
  {
    ESW_Word_128[IS_15959_TAMPER_R_PH_VOLTAGE_MISSING_STAT / 8] |= (1 << IS_15959_TAMPER_R_PH_VOLTAGE_MISSING_STAT);
  }
  else
  {
    ESW_Word_128[IS_15959_TAMPER_R_PH_VOLTAGE_MISSING_STAT / 8] &= ~(1 << IS_15959_TAMPER_R_PH_VOLTAGE_MISSING_STAT);
  }
  //1
  if (st_tamper.g_tampers_bits.bits.y_ph_volt_missing)
  {
    ESW_Word_128[IS_15959_TAMPER_Y_PH_VOLTAGE_MISSING_STAT / 8] |= (1 << IS_15959_TAMPER_Y_PH_VOLTAGE_MISSING_STAT);
  }
  else
  {
    ESW_Word_128[IS_15959_TAMPER_Y_PH_VOLTAGE_MISSING_STAT / 8] &= ~(1 << IS_15959_TAMPER_Y_PH_VOLTAGE_MISSING_STAT);
  }
  //2
  if (st_tamper.g_tampers_bits.bits.b_ph_volt_missing)
  {
    ESW_Word_128[IS_15959_TAMPER_B_PH_VOLTAGE_MISSING_STAT / 8] |= (1 << IS_15959_TAMPER_B_PH_VOLTAGE_MISSING_STAT);
  }
  else
  {
    ESW_Word_128[IS_15959_TAMPER_B_PH_VOLTAGE_MISSING_STAT / 8] &= ~(1 << IS_15959_TAMPER_B_PH_VOLTAGE_MISSING_STAT);
  }
  //3
  if (st_tamper.g_tampers_bits.bits.over_volt)
  {
    ESW_Word_128[IS_15959_TAMPER_OVER_VOLTAGE_STAT / 8] |= (1 << IS_15959_TAMPER_OVER_VOLTAGE_STAT);
  }
  else
  {
    ESW_Word_128[IS_15959_TAMPER_OVER_VOLTAGE_STAT / 8] &= ~(1 << IS_15959_TAMPER_OVER_VOLTAGE_STAT);
  }
  //4
  if (st_tamper.g_tampers_bits.bits.low_volt)
  {
    ESW_Word_128[IS_15959_TAMPER_LOW_VOLTAGE_STAT / 8] |= (1 << IS_15959_TAMPER_LOW_VOLTAGE_STAT);
  }
  else
  {
    ESW_Word_128[IS_15959_TAMPER_LOW_VOLTAGE_STAT / 8] &= ~(1 << IS_15959_TAMPER_LOW_VOLTAGE_STAT);
  }
  //5
  if (st_tamper.g_tampers_bits.bits.volt_unbalance)
  {
    ESW_Word_128[IS_15959_TAMPER_VOLTAGE_UNBALANCE / 8] |= (1 << IS_15959_TAMPER_VOLTAGE_UNBALANCE);
  }
  else
  {
    ESW_Word_128[IS_15959_TAMPER_VOLTAGE_UNBALANCE / 8] &= ~(1 << IS_15959_TAMPER_VOLTAGE_UNBALANCE);
  }
  //6
  if (st_tamper.g_tampers_bits.bits.r_ph_current_reverse)
  {
    ESW_Word_128[IS_15959_TAMPER_R_PH_CURRENT_REVERSE_STAT / 8] |= (1 << IS_15959_TAMPER_R_PH_CURRENT_REVERSE_STAT);
  }
  else
  {
    ESW_Word_128[IS_15959_TAMPER_R_PH_CURRENT_REVERSE_STAT / 8] &= ~(1 << IS_15959_TAMPER_R_PH_CURRENT_REVERSE_STAT);
  }
  //7
  if (st_tamper.g_tampers_bits.bits.y_ph_current_reverse)
  {
    ESW_Word_128[IS_15959_TAMPER_Y_PH_CURRENT_REVERSE_STAT / 8] |= (1 << IS_15959_TAMPER_Y_PH_CURRENT_REVERSE_STAT);
  }
  else
  {
    ESW_Word_128[IS_15959_TAMPER_Y_PH_CURRENT_REVERSE_STAT / 8] &= ~(1 << IS_15959_TAMPER_Y_PH_CURRENT_REVERSE_STAT);
  }
  //8
  if (st_tamper.g_tampers_bits.bits.b_ph_current_reverse)
  {
    ESW_Word_128[IS_15959_TAMPER_B_PH_CURRENT_REVERSE_STAT / 8] |= (1 << (IS_15959_TAMPER_B_PH_CURRENT_REVERSE_STAT % 8));
  }
  else
  {
    ESW_Word_128[IS_15959_TAMPER_B_PH_CURRENT_REVERSE_STAT / 8] &= ~(1 << (IS_15959_TAMPER_B_PH_CURRENT_REVERSE_STAT % 8));
  }
  //9
  if (st_tamper.g_tampers_bits.bits.r_ph_current_missing)
  {
    ESW_Word_128[IS_15959_TAMPER_R_PH_CURRENT_MISSING_STAT / 8] |= (1 << (IS_15959_TAMPER_R_PH_CURRENT_MISSING_STAT % 8));
  }
  else
  {
    ESW_Word_128[IS_15959_TAMPER_R_PH_CURRENT_MISSING_STAT / 8] &= ~(1 << (IS_15959_TAMPER_R_PH_CURRENT_MISSING_STAT % 8));
  }
  //10
  if (st_tamper.g_tampers_bits.bits.y_ph_current_missing)
  {
    ESW_Word_128[IS_15959_TAMPER_Y_PH_CURRENT_MISSING_STAT / 8] |= (1 << (IS_15959_TAMPER_Y_PH_CURRENT_MISSING_STAT % 8));
  }
  else
  {
    ESW_Word_128[IS_15959_TAMPER_Y_PH_CURRENT_MISSING_STAT / 8] &= ~(1 << (IS_15959_TAMPER_Y_PH_CURRENT_MISSING_STAT % 8));
  }
  //11
  if (st_tamper.g_tampers_bits.bits.b_ph_current_missing)
  {
    ESW_Word_128[IS_15959_TAMPER_B_PH_CURRENT_MISSING_STAT / 8] |= (1 << (IS_15959_TAMPER_B_PH_CURRENT_MISSING_STAT % 8));
  }
  else
  {
    ESW_Word_128[IS_15959_TAMPER_B_PH_CURRENT_MISSING_STAT / 8] &= ~(1 << (IS_15959_TAMPER_B_PH_CURRENT_MISSING_STAT % 8));
  }
  //12
  if (st_tamper.g_tampers_bits.bits.current_unbalance)
  {
    ESW_Word_128[IS_15959_TAMPER_CURRENT_UNBALANCE_STAT / 8] |= (1 << (IS_15959_TAMPER_CURRENT_UNBALANCE_STAT % 8));
  }
  else
  {
    ESW_Word_128[IS_15959_TAMPER_CURRENT_UNBALANCE_STAT / 8] &= ~(1 << (IS_15959_TAMPER_CURRENT_UNBALANCE_STAT % 8));
  }
  //13
  if (st_tamper.g_tampers_bits.bits.current_bypass)
  {
    ESW_Word_128[IS_15959_TAMPER_CURRENT_BYPASS_STAT / 8] |= (1 << (IS_15959_TAMPER_CURRENT_BYPASS_STAT % 8));
  }
  else
  {
    ESW_Word_128[IS_15959_TAMPER_CURRENT_BYPASS_STAT / 8] &= ~(1 << (IS_15959_TAMPER_CURRENT_BYPASS_STAT % 8));
  }
  //14
  if (st_tamper.g_tampers_bits.bits.over_current)
  {
    ESW_Word_128[IS_15959_TAMPER_OVER_CURRENT_STAT / 8] |= (1 << (IS_15959_TAMPER_OVER_CURRENT_STAT % 8));
  }
  else
  {
    ESW_Word_128[IS_15959_TAMPER_OVER_CURRENT_STAT / 8] &= ~(1 << (IS_15959_TAMPER_OVER_CURRENT_STAT % 8));
  }
  //15
  if (st_tamper.g_tampers_bits.bits.magnet_detected)
  {
    ESW_Word_128[IS_15959_TAMPER_MAGNET_STAT / 8] |= (1 << (IS_15959_TAMPER_MAGNET_STAT % 8));
  }
  else
  {
    ESW_Word_128[IS_15959_TAMPER_MAGNET_STAT / 8] &= ~(1 << (IS_15959_TAMPER_MAGNET_STAT % 8));
  }
  //16
  if (st_tamper.g_tampers_bits.bits.neutral_disturbance)
  {
    ESW_Word_128[IS_15959_TAMPER_NEUTRAL_DISTURBANCE_STAT / 8] |= (1 << (IS_15959_TAMPER_NEUTRAL_DISTURBANCE_STAT % 8));
  }
  else
  {
    ESW_Word_128[IS_15959_TAMPER_NEUTRAL_DISTURBANCE_STAT / 8] &= ~(1 << (IS_15959_TAMPER_NEUTRAL_DISTURBANCE_STAT % 8));
  }
  //17
  if (st_tamper.g_tampers_bits.bits.low_PF)
  {
    ESW_Word_128[IS_15959_TAMPER_LOW_PF_STAT / 8] |= (1 << (IS_15959_TAMPER_LOW_PF_STAT % 8));
  }
  else
  {
    ESW_Word_128[IS_15959_TAMPER_LOW_PF_STAT / 8] &= ~(1L << (IS_15959_TAMPER_LOW_PF_STAT % 8));
  }
  //18
  if (st_tamper.g_tampers_bits.bits.earth_loading)
  {
    ESW_Word_128[IS_15959_TAMPER_EARTH_LOADING_STAT / 8] |= (1 << (IS_15959_TAMPER_EARTH_LOADING_STAT % 8));
  }
  else
  {
    ESW_Word_128[IS_15959_TAMPER_EARTH_LOADING_STAT / 8] &= ~(1 << (IS_15959_TAMPER_EARTH_LOADING_STAT % 8));
  }
  //19
  if (st_tamper.g_tampers_bits.bits.cover_open)
  {
    ESW_Word_128[IS_15959_TAMPER_METER_COVER_OPEN_STAT / 8] |= (1 << (IS_15959_TAMPER_METER_COVER_OPEN_STAT % 8));
  }

  //20
  if (st_tamper.g_tampers_bits.bits.load_disable)
  {
    ESW_Word_128[IS_15959_TAMPER_METER_LOAD_STAT / 8] |= (1 << (IS_15959_TAMPER_METER_LOAD_STAT % 8));
  }
  else
  {
    ESW_Word_128[IS_15959_TAMPER_METER_LOAD_STAT / 8] &= ~(1 << (IS_15959_TAMPER_METER_LOAD_STAT % 8));
  }
  //21
  if (st_tamper.g_tampers_bits.bits.last_gasp)
  {
    ESW_Word_128[IS_15959_TAMPER_LAST_GASP_STAT / 8] |= (1 << (IS_15959_TAMPER_LAST_GASP_STAT % 8));
  }
  else
  {
    ESW_Word_128[IS_15959_TAMPER_LAST_GASP_STAT / 8] &= ~(1 << (IS_15959_TAMPER_LAST_GASP_STAT % 8));
  }
  //22
  if (st_tamper.g_tampers_bits.bits.first_breath)
  {
    ESW_Word_128[IS_15959_TAMPER_FIRST_BREATH_STAT / 8] |= (1 << (IS_15959_TAMPER_FIRST_BREATH_STAT % 8));
  }
  else
  {
    ESW_Word_128[IS_15959_TAMPER_FIRST_BREATH_STAT / 8] &= ~(1 << (IS_15959_TAMPER_FIRST_BREATH_STAT % 8));
  }
  //23
  if (st_tamper.g_tampers_bits.bits.billing_counter_increment)
  {
    ESW_Word_128[IS_15959_TAMPER_BILLING_COUNT_INCREMENT_STAT / 8] |= (1 << (IS_15959_TAMPER_BILLING_COUNT_INCREMENT_STAT % 8));
  }
  else
  {
    ESW_Word_128[IS_15959_TAMPER_BILLING_COUNT_INCREMENT_STAT / 8] &= ~(1 << (IS_15959_TAMPER_BILLING_COUNT_INCREMENT_STAT % 8));
  }
  
  write_page_eeprom(ESW_WORD_LOC, ESW_Word_128, ESW_WORD_SIZE);
  read_page_eeprom(ESWF_WORD_LOC, ESWF_Word_128, ESWF_WORD_SIZE);
  for (i = 0; i < ESW_WORD_SIZE; i++)
  {
    ESW_Push_Word_128[i] = ESWF_Word_128[i] & ESW_Word_128[i];
  }
  if (memcmp(ESW_BAK_Word_128, ESW_Push_Word_128, ESW_WORD_SIZE) != 0)
  {
    st_tamper.Trigger_Byte |= _BV(EVENTS_BIT);
    write_page_eeprom(ESW_BAK_WORD_LOC, ESW_Push_Word_128, ESW_WORD_SIZE);
  }
}

void set_default_tampers_params(void)
{
    uint8_t loop;
    uint8_t eswf_ptr[] = DEFAULT_ESWF_BITS;
    uint8_t ESWF_BITS[16];
    
    memset(ESWF_BITS, 0 ,sizeof(ESWF_BITS));
    for (loop = 0; loop < 128; loop++)
    {
      ESWF_BITS[loop / 8] |= ((((eswf_ptr[((loop) / 8)] >> loop % 8))& 0x01) << (7 - (loop % 8)));
    }
    write_page_eeprom(ESWF_WORD_LOC, ESWF_BITS, ESWF_WORD_SIZE);
    to_eeprom(TAMPER_OCC_TIME_LOC,DEFAULT_TAMPER_OCCUR_TIME, TAMPER_OCC_TIME_SIZE);
    to_eeprom(TAMPER_RES_TIME_LOC,DEFAULT_TAMPER_RESTORE_TIME, TAMPER_RES_TIME_SIZE);
    to_eeprom(CUOPEN_ENABLE_TIME_LOC, DEFAULT_COVER_OPEN_EPOCH, CUOPEN_ENABLE_TIME_SIZE);
    
    set_tamper_occurance_time(DEFAULT_TAMPER_OCCUR_TIME);
    set_tamper_restoration_time(DEFAULT_TAMPER_RESTORE_TIME);
    
    to_eeprom(EVENT_PRESISTENT_TIME_LOC + (TAMPER_OCC_TIME_SIZE*TAMPER_MAGNET_STAT), MAGNET_TAMPER_OCCURANCE_TIME, TAMPER_OCC_TIME_SIZE);
    to_eeprom(EVENT_RESTORATION_TIME_LOC + (TAMPER_RES_TIME_SIZE*TAMPER_MAGNET_STAT), MAGNET_TAMPER_RESTORATION_TIME, TAMPER_RES_TIME_SIZE);
    
    to_eeprom(EVENT_PRESISTENT_TIME_LOC + (TAMPER_OCC_TIME_SIZE*TAMPER_MAGNET_STAT), MAGNET_TAMPER_OCCURANCE_TIME, TAMPER_OCC_TIME_SIZE);
    to_eeprom(EVENT_RESTORATION_TIME_LOC + (TAMPER_RES_TIME_SIZE*TAMPER_MAGNET_STAT), MAGNET_TAMPER_RESTORATION_TIME, TAMPER_RES_TIME_SIZE);
    
    to_eeprom(EVENT_PRESISTENT_TIME_LOC + (TAMPER_OCC_TIME_SIZE*TAMPER_METER_COVER_OPEN_STAT), DEFAULT_COVER_OPEN_COUNTER, TAMPER_OCC_TIME_SIZE);
    to_eeprom(EVENT_RESTORATION_TIME_LOC + (TAMPER_RES_TIME_SIZE*TAMPER_METER_COVER_OPEN_STAT), DEFAULT_COVER_OPEN_COUNTER, TAMPER_RES_TIME_SIZE);
    
    to_eeprom(EVENT_PRESISTENT_TIME_LOC + (TAMPER_OCC_TIME_SIZE*TAMPER_RF_REMOVAL_STAT), RF_TEST_TIME, TAMPER_OCC_TIME_SIZE);
    to_eeprom(EVENT_RESTORATION_TIME_LOC + (TAMPER_RES_TIME_SIZE*TAMPER_RF_REMOVAL_STAT), RF_TEST_TIME, TAMPER_RES_TIME_SIZE);
    
    to_eeprom(EVENT_PRESISTENT_TIME_LOC + (TAMPER_OCC_TIME_SIZE*TAMPER_OVER_LOAD_STAT), DEFAULT_MINIMUM_OVER_THRESOLD_DURATION, TAMPER_OCC_TIME_SIZE);
    to_eeprom(EVENT_RESTORATION_TIME_LOC + (TAMPER_RES_TIME_SIZE*TAMPER_OVER_LOAD_STAT), DEFAULT_MINIMUM_UNDER_THRESOLD_DURATION, TAMPER_RES_TIME_SIZE);
    
    to_eeprom(EVENT_PRESISTENT_TIME_LOC + (TAMPER_OCC_TIME_SIZE*TAMPER_OVER_CURRENT_STAT), DEFAULT_MINIMUM_OVER_THRESOLD_DURATION, TAMPER_OCC_TIME_SIZE);
    to_eeprom(EVENT_RESTORATION_TIME_LOC + (TAMPER_RES_TIME_SIZE*TAMPER_OVER_CURRENT_STAT), DEFAULT_MINIMUM_UNDER_THRESOLD_DURATION, TAMPER_RES_TIME_SIZE);
    
#ifdef SINGLE_PHASE_METER
    to_eeprom(EVENT_PRESISTENT_TIME_LOC + (TAMPER_OCC_TIME_SIZE*TAMPER_NEUTRAL_DISTURBANCE_STAT), ND_OCCUR_RESTORE_TIME, TAMPER_OCC_TIME_SIZE);
    to_eeprom(EVENT_RESTORATION_TIME_LOC + (TAMPER_RES_TIME_SIZE*TAMPER_NEUTRAL_DISTURBANCE_STAT), ND_OCCUR_RESTORE_TIME, TAMPER_RES_TIME_SIZE);
    
    to_eeprom(EVENT_PRESISTENT_TIME_LOC + (TAMPER_OCC_TIME_SIZE*TAMPER_SINGLE_WIRE_STAT), SINGLE_WIRE_OCCUR_RESTORE_TIME, TAMPER_OCC_TIME_SIZE);
    to_eeprom(EVENT_RESTORATION_TIME_LOC + (TAMPER_RES_TIME_SIZE*TAMPER_SINGLE_WIRE_STAT), SINGLE_WIRE_OCCUR_RESTORE_TIME, TAMPER_RES_TIME_SIZE);
#endif
    set_over_current_value(DEFAULT_OVER_CURRENT_VAL);
    set_over_load_value(DEFAULT_OVER_LOAD_VAL);
    set_over_bypass_value(DEFAULT_BYPASS_CURRENT_VAL);
    set_minimum_over_duration(DEFAULT_MINIMUM_OVER_THRESOLD_DURATION);
    set_minimum_under_duration(DEFAULT_MINIMUM_UNDER_THRESOLD_DURATION);
    //clear tamper bits
    to_eeprom(G_TAMPER_STATUS_BITS_LOC, 0, G_TAMPER_STATUS_BITS_SIZE);
    //clear power cumulative on time
    to_eeprom(CUM_POWER_ON_TIME_LOC, 0, CUM_POWER_ON_TIME_SIZE);
}

void set_tamper_occurance_time(uint16_t time)
{
  uint8_t loop;
  for(loop = 0; loop < TOTAL_EVENT_TYPES; loop++)
  {
    if((loop == TAMPER_MAGNET_STAT) || 
       (loop == TAMPER_METER_COVER_OPEN_STAT) || 
       (loop == TAMPER_RF_REMOVAL_STAT) || 
       (loop == TAMPER_OVER_LOAD_STAT) ||
       (loop == TAMPER_OVER_CURRENT_STAT) 
#ifdef SINGLE_PHASE_METER
       ||
       (loop == TAMPER_NEUTRAL_DISTURBANCE_STAT)
       ||
       (loop == TAMPER_SINGLE_WIRE_STAT)
#endif
     )
    {
      continue;
    }
    to_eeprom(EVENT_PRESISTENT_TIME_LOC + (TAMPER_OCC_TIME_SIZE*loop), time, TAMPER_OCC_TIME_SIZE);
    st_tamper.event_presistent_time[loop] = time;
  }
}

void set_tamper_restoration_time(uint16_t time)
{
  uint8_t loop;
  for(loop = 0; loop < TOTAL_EVENT_TYPES; loop++)
  {
    if((loop == TAMPER_MAGNET_STAT) || 
       (loop == TAMPER_METER_COVER_OPEN_STAT) || 
       (loop == TAMPER_RF_REMOVAL_STAT) || 
       (loop == TAMPER_OVER_LOAD_STAT) ||
       (loop == TAMPER_OVER_CURRENT_STAT)
#ifdef SINGLE_PHASE_METER
       ||
       (loop == TAMPER_NEUTRAL_DISTURBANCE_STAT)
#endif
     )
    {
      continue;
    }
    to_eeprom(EVENT_RESTORATION_TIME_LOC + (TAMPER_RES_TIME_SIZE*loop), time, TAMPER_RES_TIME_SIZE);
    st_tamper.event_restoration_time[loop] = time;
  }
}
void init_tampers_params(void)
{
    uint8_t loop;
    St_tamper *tamper = &st_tamper;
    memset(tamper, 0, sizeof(St_tamper));
    tamper->St_cover_open.enable_time = from_eeprom(CUOPEN_ENABLE_TIME_LOC, CUOPEN_ENABLE_TIME_SIZE);
    for(loop = 0; loop < TOTAL_EVENT_TYPES; loop++)
    {
        tamper->event_presistent_time[loop] = from_eeprom(EVENT_PRESISTENT_TIME_LOC + (TAMPER_OCC_TIME_SIZE*loop), TAMPER_OCC_TIME_SIZE);
        tamper->event_restoration_time[loop] = from_eeprom(EVENT_RESTORATION_TIME_LOC + (TAMPER_RES_TIME_SIZE*loop), TAMPER_RES_TIME_SIZE);
    }
    tamper->g_tampers_bits.tamper_bits = from_eeprom(G_TAMPER_STATUS_BITS_LOC, G_TAMPER_STATUS_BITS_SIZE);
    get_over_current_value();
    get_over_load_value();
    get_over_bypass_value();
    get_minimum_over_duration();
    get_minimum_under_duration();
}

void restore_over_limit_events(void)
{
#if defined WHOLE_CURRENT_METER || defined SINGLE_PHASE_METER
    if (st_tamper.g_tampers_bits.bits.over_current)
    {
      st_tamper.g_tampers_bits.bits.over_current = 0;
      to_eeprom(G_TAMPER_STATUS_BITS_LOC, *(uint32_t*)&st_tamper.g_tampers_bits.tamper_bits, G_TAMPER_STATUS_BITS_SIZE);
      store_event_data(AMP_EVENT, 68, EPOCH);
    }
    if (st_tamper.g_tampers_bits.bits.over_load)
    {
      st_tamper.g_tampers_bits.bits.over_load = 0;
      to_eeprom(G_TAMPER_STATUS_BITS_LOC, *(uint32_t*)&st_tamper.g_tampers_bits.tamper_bits, G_TAMPER_STATUS_BITS_SIZE);
      store_event_data(OTHER_EVENT, 216, EPOCH);
    }
#endif
}

void clear_cover_open(void)
{
    reset_cover_open_time();
    clear_non_roll_over();
}

uint32_t get_tamper_counts(void)
{
    return from_eeprom(TOTAL_TAMPERS_COUNT_LOC, TOTAL_TAMPERS_COUNT_SIZE);
}

uint32_t get_programming_counts(void)
{
    return from_eeprom(TOTAL_PROGRAMMING_COUNT_LOC, TOTAL_PROGRAMMING_COUNT_SIZE);
}

void update_cumu_power_on_time(uint32_t epoch)
{
    static uint32_t last_epoch = 0;
    if(last_epoch != epoch)
    {
        last_epoch = epoch;
        st_tamper.Cum_Power_On_Time++;
    }
}

uint32_t get_cumu_power_on_time(void)
{
    return st_tamper.Cum_Power_On_Time;
}

void set_cumu_power_on_time(uint32_t time)
{
    st_tamper.Cum_Power_On_Time = time;
}

void update_cumu_power_off_time(uint32_t epoch)
{
    static uint32_t last_epoch = 0;
    if(last_epoch != epoch)
    {
        last_epoch = epoch;
        st_tamper.Cum_Power_Off_Time++;
    }
}

uint32_t get_cumu_power_off_time(void)
{
    return st_tamper.Cum_Power_Off_Time;
}

void set_cumu_power_off_time(uint32_t time)
{
    st_tamper.Cum_Power_Off_Time = time;
}

void set_tamper_push_bit(tamper_stats tamper_bit)
{
    st_tamper.Trigger_Byte |= (1 << tamper_bit);
    to_eeprom(G_TAMPER_STATUS_BITS_LOC, *(uint32_t*)&st_tamper.g_tampers_bits.tamper_bits, G_TAMPER_STATUS_BITS_SIZE);
}

void clear_tamper_push_bit(tamper_stats tamper_bit)
{
    st_tamper.Trigger_Byte &= ~(1 << tamper_bit);
    to_eeprom(G_TAMPER_STATUS_BITS_LOC, *(uint32_t*)&st_tamper.g_tampers_bits.tamper_bits, G_TAMPER_STATUS_BITS_SIZE);
}

float32_t get_over_current_value(void)
{
  read_page_eeprom(STORAGE_EEPROM_DLMS_OVER_CURRENT_VALUE_ADDR, (uint8_t *)&st_tamper.over_current_value, STORAGE_EEPROM_DLMS_OVER_CURRENT_VALUE_SIZE);
  return st_tamper.over_current_value;
}

void set_over_current_value(float32_t value)
{
  if(value > MAXIMUM_OVER_CURRENT_VALUE)
  {
    value = MAXIMUM_OVER_CURRENT_VALUE;
  }
  else if(value < MINIMUM_OVER_CURRENT_VALUE)
  {
    value = MINIMUM_OVER_CURRENT_VALUE;
  }
  st_tamper.over_current_value = value;
  write_page_eeprom(STORAGE_EEPROM_DLMS_OVER_CURRENT_VALUE_ADDR, (uint8_t *)&st_tamper.over_current_value, STORAGE_EEPROM_DLMS_OVER_CURRENT_VALUE_SIZE);
}

float32_t get_over_load_value(void)
{
  read_page_eeprom(STORAGE_EEPROM_DLMS_OVER_LOAD_VALUE_ADDR, (uint8_t *)&st_tamper.over_load_value, STORAGE_EEPROM_DLMS_OVER_LOAD_VALUE_SIZE);
  return st_tamper.over_load_value;
}

void set_over_load_value(float32_t value)
{
  if(value > MAXIMUM_OVER_LOAD_VALUE)
  {
    value = MAXIMUM_OVER_LOAD_VALUE;
  }
  else if(value < MINIMUM_OVER_LOAD_VALUE)
  {
    value = MINIMUM_OVER_LOAD_VALUE;
  }
  st_tamper.over_load_value = value;
  write_page_eeprom(STORAGE_EEPROM_DLMS_OVER_LOAD_VALUE_ADDR, (uint8_t *)&st_tamper.over_load_value, STORAGE_EEPROM_DLMS_OVER_LOAD_VALUE_SIZE);
}

uint32_t get_over_bypass_value(void)
{
  read_page_eeprom(STORAGE_EEPROM_DLMS_OVER_BYPASS_VALUE_ADDR, (uint8_t *)&st_tamper.over_bypass_value, STORAGE_EEPROM_DLMS_OVER_BYPASS_VALUE_SIZE);
  if(st_tamper.over_bypass_value > MAXIMUM_BYPASS_VALUE)
  {
    st_tamper.over_bypass_value = MAXIMUM_BYPASS_VALUE;
  }
  else if(st_tamper.over_bypass_value < MINIMUM_BYPASS_VALUE)
  {
    st_tamper.over_bypass_value = MINIMUM_BYPASS_VALUE;
  }
  return st_tamper.over_bypass_value;
}

void set_over_bypass_value(uint32_t value)
{
  if(value > MAXIMUM_BYPASS_VALUE)
  {
    value = MAXIMUM_BYPASS_VALUE;
  }
  else if(value < MINIMUM_BYPASS_VALUE)
  {
    value = MINIMUM_BYPASS_VALUE;
  }
  st_tamper.over_bypass_value = value;
  write_page_eeprom(STORAGE_EEPROM_DLMS_OVER_BYPASS_VALUE_ADDR, (uint8_t *)&st_tamper.over_bypass_value, STORAGE_EEPROM_DLMS_OVER_BYPASS_VALUE_SIZE);
}


uint32_t get_minimum_over_duration(void)
{
  read_page_eeprom(STORAGE_EEPROM_DLMS_MINIMUM_OVER_THRESOLD_DURATION_ADDR, (uint8_t *)&st_tamper.minimum_thresold_over_duration, STORAGE_EEPROM_DLMS_MINIMUM_OVER_THRESOLD_DURATION_SIZE);
  return st_tamper.minimum_thresold_over_duration;
}

void set_minimum_over_duration(uint32_t value)
{
  if(value > MAXIMUM_MINIMUM_OVER_THRESOLD_DURATION)
  {
    value = MAXIMUM_MINIMUM_OVER_THRESOLD_DURATION;
  }
  else if(value < MINIMUM_MINIMUM_OVER_THRESOLD_DURATION)
  {
    value = MINIMUM_MINIMUM_OVER_THRESOLD_DURATION;
  }
  st_tamper.minimum_thresold_over_duration = value;
  write_page_eeprom(STORAGE_EEPROM_DLMS_MINIMUM_OVER_THRESOLD_DURATION_ADDR, (uint8_t *)&st_tamper.minimum_thresold_over_duration, STORAGE_EEPROM_DLMS_MINIMUM_OVER_THRESOLD_DURATION_SIZE);
  st_tamper.event_presistent_time[TAMPER_OVER_LOAD_STAT] = value;
  st_tamper.event_presistent_time[TAMPER_OVER_CURRENT_STAT] = value;
  to_eeprom(EVENT_PRESISTENT_TIME_LOC + (TAMPER_OCC_TIME_SIZE*TAMPER_OVER_LOAD_STAT), value, TAMPER_OCC_TIME_SIZE);
  to_eeprom(EVENT_PRESISTENT_TIME_LOC + (TAMPER_OCC_TIME_SIZE*TAMPER_OVER_CURRENT_STAT), value, TAMPER_OCC_TIME_SIZE);
}

uint32_t get_minimum_under_duration(void)
{
  read_page_eeprom(STORAGE_EEPROM_DLMS_MINIMUM_UNDER_THRESOLD_DURATION_ADDR, (uint8_t *)&st_tamper.minimum_thresold_under_duration, STORAGE_EEPROM_DLMS_MINIMUM_UNDER_THRESOLD_DURATION_SIZE);
  return st_tamper.minimum_thresold_under_duration;
}

void set_minimum_under_duration(uint32_t value)
{
  if(value > MAXIMUM_MINIMUM_UNDER_THRESOLD_DURATION)
  {
    value = MAXIMUM_MINIMUM_UNDER_THRESOLD_DURATION;
  }
  else if(value < MINIMUM_MINIMUM_UNDER_THRESOLD_DURATION)
  {
    value = MINIMUM_MINIMUM_UNDER_THRESOLD_DURATION;
  }
  st_tamper.minimum_thresold_under_duration = value;
  write_page_eeprom(STORAGE_EEPROM_DLMS_MINIMUM_UNDER_THRESOLD_DURATION_ADDR, (uint8_t *)&st_tamper.minimum_thresold_under_duration, STORAGE_EEPROM_DLMS_MINIMUM_UNDER_THRESOLD_DURATION_SIZE);
  st_tamper.event_restoration_time[TAMPER_OVER_LOAD_STAT] = value;
  st_tamper.event_restoration_time[TAMPER_OVER_CURRENT_STAT] = value;
  to_eeprom(EVENT_RESTORATION_TIME_LOC + (TAMPER_RES_TIME_SIZE*TAMPER_OVER_LOAD_STAT), value, TAMPER_RES_TIME_SIZE);
  to_eeprom(EVENT_RESTORATION_TIME_LOC + (TAMPER_RES_TIME_SIZE*TAMPER_OVER_CURRENT_STAT), value, TAMPER_RES_TIME_SIZE);
}

void register_disconnect_event_callback(st_RTC_time* RTC_time)
{
  st_tamper.g_tampers_bits.bits.load_disable = 1;
  to_eeprom(G_TAMPER_STATUS_BITS_LOC, *(uint32_t*)&st_tamper.g_tampers_bits.tamper_bits, G_TAMPER_STATUS_BITS_SIZE);
  store_event_data(CTRL_EVENT, 301, RTC_time->epoch);
}

void register_connect_event_callback(st_RTC_time* RTC_time)
{
  st_tamper.g_tampers_bits.bits.load_disable = 0;
  to_eeprom(G_TAMPER_STATUS_BITS_LOC, *(uint32_t*)&st_tamper.g_tampers_bits.tamper_bits, G_TAMPER_STATUS_BITS_SIZE);
  store_event_data(CTRL_EVENT, 302, RTC_time->epoch);
}

uint8_t is_RF_missing(void)
{
  return st_tamper.g_tampers_bits.bits.rf_removed;
}

uint8_t is_cover_open(void)
{
 if(is_serial_number_updated(0))
 {
   if ((IS_COVER_OPEN()) || (st_tamper.g_tampers_bits.bits.cover_open == 1))
   {
     e_LCD_icon_status.cover_open = 1;
     e_LCD_icon_status.DG_Cover = 0;
     return 1;
   }
 }
 else
 {
   if (IS_COVER_OPEN())
   {
     e_LCD_icon_status.DG_Cover = 1;
   }
   else
   {
     e_LCD_icon_status.DG_Cover = 0;
   }
 }
 e_LCD_icon_status.cover_open = 0;
 return 0;
}

void reset_cover_open_time(void)
{
  st_tamper.g_tampers_bits.bits.cover_open = 0;
  st_tamper.St_cover_open.enable_time = DEFAULT_COVER_OPEN_EPOCH;
  to_eeprom(CUOPEN_ENABLE_TIME_LOC, DEFAULT_COVER_OPEN_EPOCH, CUOPEN_ENABLE_TIME_SIZE);
}