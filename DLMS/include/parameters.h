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

#ifndef PARAMETERS_H
#define PARAMETERS_H

#include "dlmssettings.h"
#include "date.h"
/**
* SN Parameters
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
    gxByteBuffer* attributeDescriptor;
    /**
     * Data.
     */
    gxByteBuffer* data;
    /**
     * Send date and time. This is used in Data notification messages.
     */
    struct tm* time;
    /**
     * Item Count.
     */
    int count;

    /**
     * Are there more data to send or more data to receive.
     */
    unsigned char multipleBlocks;

    /**
     * Block index.
     */
    unsigned short blockIndex;
} gxSNParameters;

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
    gxByteBuffer* attributeDescriptor;
    /**
     * Data.
     */
    gxByteBuffer* data;
    /**
     * Send date and time. This is used in Data notification messages.
     */
    struct tm* time;
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
    unsigned long blockIndex;
} gxLNParameters;

void params_initLN(
    gxLNParameters *target,
    dlmsSettings* settings,
    DLMS_COMMAND command,
    unsigned char commandType,
    gxByteBuffer* attributeDescriptor,
    gxByteBuffer* data,
    unsigned char status);

#endif //PARAMETERS_H
