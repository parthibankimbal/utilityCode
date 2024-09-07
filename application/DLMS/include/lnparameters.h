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

#ifndef LN_PARAMETERS_H
#define LN_PARAMETERS_H

#include "dlmssettings.h"
#include "bytebuffer.h"
#include "enums.h"
#include "date.h"

/**
* LN Parameters
*/
typedef struct
{
    /**
     * DLMS settings.
     */
    dlmsSettings *settings;
    /**
     * DLMS command.
     */
    DLMS_COMMAND command;
    /**
     * Request type.
     */
    int requestType;
    /**
     * Attribute descriptor.
     */
    gxByteBuffer attributeDescriptor;
    /**
     * Data.
     */
    gxByteBuffer m_Data;
    /**
     * Send date and time. This is used in Data notification messages.
     */
    struct tm time;
    /**
     * Reply status.
     */
    int status;
    /**
     * Are there more data to send or more data to receive.
     */
    unsigned char multipleBlocks;
    /**
     * Is this last block in send.
     */
    unsigned char lastBlock;
    /**
     * Block index.
     */
    int blockIndex;
} lnParameters;

#endif //LN_PARAMETERS_H
