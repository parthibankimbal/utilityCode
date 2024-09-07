/*
 * emeter-communication.c
 *
 * 
 *
 * Copyright (C) 2015 Texas Instruments Incorporated - http://www.ti.com/ 
 * ALL RIGHTS RESERVED 
 *                                                                                                                                                                                                                                                                     
*/

#include "communication.h"
#include "server.h"

#if !defined(NULL)
#define NULL    (void *) 0
#endif
type_RF_send_end_callback RF_send_end_callback = 0;

#if defined(RENESAS_CONTROLLER)
    #if defined(UART_4_PORTS)
    serial_msg_buf_t tx_msg[4];
    serial_msg_buf_t rx_msg[4];
    #elif defined(UART_3_PORTS)
    serial_msg_buf_t tx_msg[3];
    serial_msg_buf_t rx_msg[3];
    #elif defined(UART_2_PORTS)
    serial_msg_buf_t tx_msg[2];
    serial_msg_buf_t rx_msg[2];
    #elif defined(UART_1_PORTS)
    serial_msg_buf_t tx_msg[1];
    serial_msg_buf_t rx_msg[1];
    #endif
#endif

#if defined(RENESAS_CONTROLLER)
void serialComm_Timer_Callback(void)
{
    uint8_t i;
    for(i=0;i<MAX_UART_PORTS;i++)
    {
      if(rx_msg[i].inter_Activity_timeout<DEFAULT_RF_TICK_INTERACTIVITY)
      {
        rx_msg[i].inter_Activity_timeout++;
      }
      //if((rx_msg[i].inter_Activity_timeout>=DEFAULT_TICK_PROCESS_FRAME) && (rx_msg[i].len != 0) && (rx_msg[i].Process_Frame == 0))
      //{
      //  rx_msg[i].Process_Frame = 1;
      //}
      if(tx_msg[i].Tx_timeout<DEFAULT_TICK_TX_TIME_OUT)
      {
        tx_msg[i].Tx_timeout++;
      }
      if(rx_msg[i].Tx_timeout<DEFAULT_TICK_PROCESS_FRAME)
      {
        rx_msg[i].Tx_timeout++;
      }
    }
}

void Rx_serial_data(uint8_t port,uint8_t data)
{
  rx_msg[port].Tx_timeout = 0;
  if (rx_msg[port].inter_Activity_timeout >= DEFAULT_TICK_INTEROCTAT)
  {
    /* TODO: Rakesh to make communication library as wrraper lib
    and remove dependancy of application layer. methods to wrap as resetting buffers, connection timeout etc. */
    
    //bb_reset(&lnHdlc_uart_port[port].receivedData);
    //rx_msg[port].len = 0;
    rx_msg[port].buffer_reset = 1;
    
    if(port == RF_COMM_PORT)
    {
      if (rx_msg[port].inter_Activity_timeout >= DEFAULT_RF_TICK_INTERACTIVITY)
      {
        //svr_reset(&lnHdlc_uart_port[port]);
        rx_msg[port].server_reset = 1;
      }
    }
    else
    {
      if (rx_msg[port].inter_Activity_timeout >= DEFAULT_TICK_INTERACTIVITY)
      {
        //svr_reset(&lnHdlc_uart_port[port]);
        rx_msg[port].server_reset = 1;
      }
    }
  }
  rx_msg[port].inter_Activity_timeout = 0;
  
  if(rx_msg[port].len < COMM_BUFFER_SIZE)
  {
    rx_msg[port].buf.uint8[rx_msg[port].len++] = data;
  }
  else
  {
    rx_msg[port].len = 0;
  }
  return;
}

void send_message(uint8_t port, uint8_t *buf, uint16_t len)
{ 
    uint16_t u16SMIndex;
    for (u16SMIndex = 0; u16SMIndex < len; u16SMIndex++)
    {
            tx_msg[port].buf.uint8[u16SMIndex] = buf[u16SMIndex];
    }
    tx_msg[port].ptr = 0;
    tx_msg[port].len = len;
    tx_msg[port].next_msg_send_wait = 1;
    switch (port)
    {
    case 0: R_UART0_Send(tx_msg[port].buf.uint8,len); break;
#if (MAX_UART_PORTS > 1)            
    case 1: R_UART2_Send(tx_msg[port].buf.uint8,len); break;
#endif
#if (MAX_UART_PORTS > 2)
    case 2: R_UART1_Send(tx_msg[port].buf.uint8,len); break;
#endif
#if (MAX_UART_PORTS > 3)
    case 3: R_UART3_Send(tx_msg[port].buf.uint8,len); break;
#endif
    default:
            break;
    }
}

void UART_send_complete(uint8_t port)
{
    tx_msg[port].next_msg_send_wait = 0;
    tx_msg[port].Tx_timeout = 0;
    R_METER_CMD_UART_SendEndCallback();
    if(RF_send_end_callback != 0)
    {
      RF_send_end_callback();
    }
}

#endif
