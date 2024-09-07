#include "push_schedular.h"
#include "epoch.h"

#ifdef OLD_METHOD
void check_push_time(st_push_schedular* st_push, uint32_t epoch_time)
{
  uint8_t i, hour, minute, second;
  for (i = 0; i < st_push->total_actions; i++)
  {
    hour = st_push->push_time[i].hour;
    minute = st_push->push_time[i].minute;
    second = st_push->push_time[i].second;

    if (st_push->last_epoch_push_time < epoch_time)
    {
      if (((epoch_time % 60) == second) && (second != 0xFF) && (minute == 0xFF) && (hour == 0xFF))
      {
        st_push->is_push_action = 1;
        st_push->last_epoch_push_time = epoch_time - 1;
      }

      else if (((epoch_time % 3600) / 60 == minute) && (minute != 0xFF) && (((epoch_time % 60) == second) || (second == 0xFF)) && (hour == 0xFF))
      {
        st_push->is_push_action = 1;
        st_push->last_epoch_push_time = epoch_time - 1;
      }

      else if (((epoch_time % 86400) / 3600 == hour) && (hour != 0xFF) && (((epoch_time % 3600) / 60 == minute) || (minute == 0xFF)) && (((epoch_time % 60) == second) || (second == 0xFF)))
      {
        st_push->is_push_action = 1;
        st_push->last_epoch_push_time = epoch_time - 1;
      }
    }
  }
}
#else
void check_push_time(st_push_schedular* st_push, uint32_t epoch_time)
{
  uint8_t push_flag;
  uint8_t num_action;
  st_time_sturct* push_time;
  stTime_struct dateTime;

  UnixToDatteTime(epoch_time, &dateTime);
  for (num_action = 0; num_action < st_push->total_actions; num_action++)
  {
    push_time = &st_push->push_time[num_action];
    push_flag = 1;
    if (push_time->year != 255)
    {
      push_flag &= (push_time->year == dateTime.year) ? 1 : 0;
    }

    if (push_time->month != 255)
    {
      push_flag &= (push_time->month == dateTime.month) ? 1 : 0;
    }

    if (push_time->day != 255)
    {
      push_flag &= (push_time->day == dateTime.day) ? 1 : 0;
    }

    if (push_time->hour != 255)
    {
      push_flag &= (push_time->hour == dateTime.hour) ? 1 : 0;
    }

    if (push_time->minute != 255)
    {
      push_flag &= (push_time->minute == dateTime.minute) ? 1 : 0;
    }

    if (push_time->second != 255)
    {
      push_flag &= (push_time->second == dateTime.second) ? 1 : 0;
    }

    if ((push_time->year == 255) && (push_time->month == 255) && (push_time->day == 255) && (push_time->hour == 255) && (push_time->minute == 255) && (push_time->second == 255))
    {
      push_flag = 0;
    }
    
    if (push_flag)
    {
      st_push->is_push_action |= push_flag;
      st_push->last_epoch_push_time = epoch_time;
    }
  }
  return;
}

uint8_t check_greater_time(uint32_t epoch_time, st_time_sturct* st_actionTime)
{
  uint32_t u32CGTTemp;
  stTime_struct dateTime;
  stTime_struct action_dateTime;

  UnixToDatteTime(epoch_time, &dateTime);
  if (st_actionTime->year == 0xFF)
  {
    action_dateTime.year = dateTime.year;
  }
  else
  {
    action_dateTime.year = st_actionTime->year;
  }
  if (st_actionTime->month == 0xFF)
  {
    action_dateTime.month = dateTime.month;
  }
  else
  {
    action_dateTime.month = st_actionTime->month;
  }

  if (st_actionTime->day == 0xFF)
  {
    action_dateTime.day = dateTime.day;
  }
  else
  {
    action_dateTime.day = st_actionTime->day;
  }
  if (st_actionTime->hour == 0xFF)
  {
    action_dateTime.hour = dateTime.hour;
  }
  else
  {
    action_dateTime.hour = st_actionTime->hour;
  }
  if (st_actionTime->minute == 0xFF)
  {
    action_dateTime.minute = dateTime.minute;
  }
  else
  {
    action_dateTime.minute = st_actionTime->minute;
  }
  if (st_actionTime->second == 0xFF)
  {
    action_dateTime.second = dateTime.second;
  }
  else
  {
    action_dateTime.second = st_actionTime->second;
  }
  u32CGTTemp = Time_Convert_TO2TS(&action_dateTime);
  
  if ((st_actionTime->year == 255) && (st_actionTime->month == 255) && (st_actionTime->day == 255) && (st_actionTime->hour == 255) && (st_actionTime->minute == 255) && (st_actionTime->second == 255))
  {
      u32CGTTemp = 0;
  }
    
  return ((epoch_time >= u32CGTTemp) ? 1 : 0);
}

#endif