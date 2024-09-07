/*
 * emeter-communication.h
 *
 * 
 *
 * Copyright (C) 2015 Texas Instruments Incorporated - http://www.ti.com/ 
 * ALL RIGHTS RESERVED 
 *                                                                                                                                                                                                                                                                     
*/
#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "typedef.h"
#include "r_cg_macrodriver.h"
#include "r_cg_sau.h"
#include "r_meter_cmd.h"
#include "server.h"
#include "bytebuffer.h"

#define RENESAS_CONTROLLER
#define UART_2_PORTS

#define DEFAULT_TICK_INTEROCTAT         1000
#define DEFAULT_RF_TICK_INTERACTIVITY   60000
#define DEFAULT_TICK_INTERACTIVITY      20000

#define DEFAULT_TICK_PROCESS_FRAME      100
#define DEFAULT_TICK_TX_TIME_OUT        2000

typedef void (*type_RF_send_end_callback)(void);
extern type_RF_send_end_callback RF_send_end_callback;

typedef enum
{
  OPTICAL_COMM_PORT,
  RF_COMM_PORT
}COMM_PORT;

typedef union
{
#if 1
#define COMM_BUFFER_SIZE 1600
    uint8_t uint8[COMM_BUFFER_SIZE];
    uint16_t uint16[COMM_BUFFER_SIZE/2];
#else
    uint8_t uint8[4 + 12 + MAX_IEC1107_MSG_BODY];
    uint16_t uint16[(4 + 12 + MAX_IEC1107_MSG_BODY)/2];
#endif
} serial_msg_t;

/* Incoming or outgoing serial message buffer */
typedef struct
{
    serial_msg_t buf;
    uint16_t ptr;
    uint16_t len;
    uint32_t inter_Activity_timeout;
    uint16_t Tx_timeout;
    uint8_t next_msg_send_wait;
    uint8_t Process_Frame;
    uint8_t server_reset;
    uint8_t buffer_reset;
} serial_msg_buf_t;

#if defined(RENESAS_CONTROLLER)
    #if defined(UART_4_PORTS)
    extern serial_msg_buf_t tx_msg[4];
    extern serial_msg_buf_t rx_msg[4];
    #define MAX_UART_PORTS      4
    #elif defined(UART_3_PORTS)
    extern serial_msg_buf_t tx_msg[3];
    extern serial_msg_buf_t rx_msg[3];
    #define MAX_UART_PORTS      3
    #elif defined(UART_2_PORTS)
    extern serial_msg_buf_t tx_msg[2];
    extern serial_msg_buf_t rx_msg[2];
    #define MAX_UART_PORTS      2
    #elif defined(UART_1_PORTS)
    extern serial_msg_buf_t tx_msg[1];
    extern serial_msg_buf_t rx_msg[1];
    #define MAX_UART_PORTS      1
    #endif

    void Rx_serial_data(uint8_t port,uint8_t data);
    void send_message(uint8_t port, uint8_t *buf, uint16_t len);
    void UART_send_complete(uint8_t port);
    void serialComm_Timer_Callback(void);

#endif

#endif //COMMUNICATION_H
