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

#ifndef GXHELPERS_H
#define GXHELPERS_H

#include <stdint.h>
#include "bytebuffer.h"

    int32_t hlp_getObjectCount(gxByteBuffer* buff);

    //Get count size in bytes.
    int8_t hlp_getObjectCountSizeInBytes(unsigned long count);

    // Set count of items.
    void hlp_setObjectCount(unsigned long count, gxByteBuffer* buff);

#endif //GXHELPERS_H
