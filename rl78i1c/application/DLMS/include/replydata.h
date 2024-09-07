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

#ifndef GXREPLYDATA_H
#define GXREPLYDATA_H
#include "bytebuffer.h"
#include "enums.h"

  typedef struct
  {
    /**
    * Is more data available.
    */
    DLMS_DATA_REQUEST_TYPES moreData;
    /**
     * Received command.
     */
    DLMS_COMMAND command;
    /**
     * Received data.
     */
    gxByteBuffer data;
    /**
     * Is frame complete.
     */
    unsigned char complete;
       /**
        * Packet length.
        */
    int packetLength;
    /**
     * Try Get value.
     */
    unsigned char peek;
  } gxReplyData;

  unsigned char reply_isMoreData(gxReplyData* reply);

  void reply_clear(gxReplyData* reply);

#endif //GXREPLYDATA_H