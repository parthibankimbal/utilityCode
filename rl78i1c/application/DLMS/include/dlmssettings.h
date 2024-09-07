//
// --------------------------------------------------------------------------
//  Gurux Ltd
//
//
//
// Filename:        $HeadURL:  $
//
// Version:         $Revision:  $,
//                  $Date:  $
//                  $Author: $
//
// Copyright (c) Gurux Ltd
//
//---------------------------------------------------------------------------

#ifndef DLMS_SETTINGS_H
#define DLMS_SETTINGS_H

#include <stdint.h>
#include "enums.h"
#include "replydata.h"
#include "ciphering.h"

  typedef struct
  {
    union
    {
      struct
      {
        uint8_t window_size : 6;
        uint8_t is_Streaming : 1;
        uint8_t is_last_block : 1;
      };
      uint8_t control_byte;
    };
  }Control_byte_st;

  typedef struct
  {
    uint16_t block_number;
    uint16_t block_number_ack;
    Control_byte_st control_byte_st;
    unsigned short client_block_number;
    unsigned short client_block_number_ack;
    Control_byte_st client_control_byte_st;
  }GBT_transfer;

  typedef struct
  {
    // Client to server challenge.
    gxByteBuffer ctoSChallenge;

    // Server to Client challenge.
    gxByteBuffer stoCChallenge;

        // Invoke ID.
    int invokeID;

    //Long Invoke ID.
    int longInvokeID;

    // Priority.
    DLMS_PRIORITY priority;

    // Service class.
    DLMS_SERVICE_CLASS serviceClass;

    // Client address.
    unsigned long clientAddress;
    //Server address.
    unsigned long serverAddress;

    unsigned char useLogicalNameReferencing;
    DLMS_INTERFACE_TYPE interfaceType;
    DLMS_AUTHENTICATION authentication;
    gxByteBuffer password;
        /**
         * Long data count.
         */
    unsigned short count;

    /**
     * Long data index.
     */
    unsigned short index;

    /**
     * Long data position. This is used to save position of long read/write.
     */
    unsigned short position;

    /**
    * DLMS version number.
    */
    unsigned char dlmsVersionNumber;

    /**
    * Max PDU size used in communicating.
    */
    unsigned short maxPduSize;

    /**
    * Max PDU size that server uses. Client can ask anything, but server will decide.
    */
    unsigned short maxServerPDUSize;

    /**
    * HDLC sender frame sequence number.
    */
    unsigned char senderFrame;

    /**
    * HDLC receiver block sequence number.
    */
    unsigned char receiverFrame;
    unsigned char ExpectedreceiverFrame;
    /**
    * Server functionality is not supported at the moment in ANSI C version.
    */
    unsigned char server;
    unsigned char isAuthenticationRequired;
    unsigned char conformanceBlock[3];
    int maxInfoTX;
    int maxInfoRX;
    int windowSizeTX;
    int windowSizeRX;
    unsigned int packetIndex;
    // Block packet index.
    unsigned long blockIndex;
    //Is connected to the meter.
    unsigned char connected;
    ciphering cipher;
    unsigned char SendData;
    unsigned char server_add_size;
    unsigned char IsLastReceivedFrameInfo;
    unsigned char Unsupported_service;
    GBT_transfer gbt_transfer;
    uint8_t port;
  } dlmsSettings;

  typedef struct
  {
    const uint8_t* template;
    uint16_t sz_template;
    uint16_t start_entry;
    uint16_t num_entries;
    uint16_t entries_remaining;
    //TODO: Rakesh sept 2022 (later)test load profile get when adding new entry in the logs.
    uint16_t total_entries;
    const uint16_t* column_szs;
  }Msg_Info;

  typedef struct
  {
    /**
    * Executed command.
    */
    DLMS_COMMAND command;
     /**
      * Extra data from PDU.
      */
    gxByteBuffer data;
  } gxLongTransaction;

  typedef struct
  {
    dlmsSettings base;

    gxReplyData info;
    /**
    * Received data.
    */
    gxByteBuffer receivedData;

    /**
    * Reply data.
    */
    gxByteBuffer replyData;

    gxByteBuffer longTransaction;
    /**
    * Is server initialized.
    */
    uint8_t reset_server;
    unsigned char initialized;
    unsigned char HDLCinitialized;
    Msg_Info messages;
  } dlmsServerSettings;

  //Initialize server.
  void svr_init(
    dlmsServerSettings* settings,
    unsigned char useLogicalNameReferencing,
    DLMS_INTERFACE_TYPE interfaceType);

  //Initialize client.
  void cl_init(
    dlmsSettings* settings,
    unsigned char useLogicalNameReferencing,
    int clientAddress,
    int serverAddress,
    DLMS_AUTHENTICATION authentication,
    const char* password,
    DLMS_INTERFACE_TYPE interfaceType);

  //Clear DLMS settings.
  void cl_clear(
    dlmsSettings* settings);

  void svr_clear(
    dlmsServerSettings* settings);

  //Reset block index.
  void resetBlockIndex(
    dlmsSettings* settings);

  void resetFrameSequence(
    dlmsSettings* settings);

  unsigned char checkFrame(
    dlmsSettings* settings,
    unsigned char frame);

  // Increase receiver sequence.
  //
  // @param value
  //            Frame value.
  // Increased receiver frame sequence.
  unsigned char increaseReceiverSequence(
    unsigned char value);

  // Increase sender sequence.
  //
  // @param value
  //            Frame value.
  // Increased sender frame sequence.
  unsigned char increaseSendSequence(
    unsigned char value);

  unsigned char getNextSend(
    dlmsSettings* settings);

  unsigned char getReceiverReady(
    dlmsSettings* settings);

  unsigned char isCiphered(
    ciphering* cipher);

#endif //DLMS_SETTINGS_H