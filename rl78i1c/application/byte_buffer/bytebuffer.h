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

#ifndef BYTE_BUFFER_H
#define BYTE_BUFFER_H



#include <stdlib.h>
#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)
#include <assert.h>
#if _MSC_VER > 1400
#include <crtdbg.h>
#endif
#endif
#include <string.h>
#include <stdio.h>
#include "errorcodes.h"

#define byte_swap(x)    ((((x) << 8) & 0xFF00) | (((x) >> 8) & 0xFF))
#define INJECT8(x)      ((uint16_t) (x) & 0xFF)
#define INJECT16(x)     (((uint16_t) (x) >> 8) & 0xFF), ((uint16_t) (x) & 0xFF)
#define INJECT32(x)     (((uint32_t) (x) >> 24) & 0xFF), (((uint32_t) (x) >> 16) & 0xFF), (((uint32_t) (x) >> 8) & 0xFF), ((uint32_t) (x) & 0xFF)
#define INJECT64(x)     (((uint64_t) (x) >> 56) & 0xff), (((uint64_t) (x) >> 48) & 0xff),(((uint64_t) (x) >> 40) & 0xff),(((uint64_t) (x) >> 32) & 0xff),(((uint64_t) (x) >> 24) & 0xff),(((uint64_t) (x) >> 16) & 0xff),(((uint64_t) (x) >> 8) & 0xff),(((uint64_t) (x) & 0xff))

#define INJECT_F32(x)     (((uint32_t) (x) >> 24) & 0xFF), (((uint32_t) (x) >> 16) & 0xFF), (((uint32_t) (x) >> 8) & 0xFF), ((uint32_t) (x) & 0xFF)

#define REV_ORDER16_t(x) ((((x) << 8) & 0xFF00) | (((x) >> 8) & 0xFF))
#define REV_ORDER32_t(x) (((x) >> 24) & 0xFF) | (((x) >> 8) & 0xFF00) | (((x) << 8) & 0xFF0000) | (((x) << 24) & 0xFF000000)
#define REV_ORDER64_t(x) (((x) >> 56) & 0xFF) | (((x) >> 40) & 0xFF00) | (((x) >> 24) & 0xFF0000) | (((x) >> 8) & 0xFF000000) | (((x) << 8) & 0xFF00000000) | (((x) << 24) & 0xFF0000000000) | (((x) << 40) & 0xFF000000000000) | (((x) << 56) & 0xFF00000000000000)

#define INJECT32X(x)     ((uint32_t) (x) & 0xFF),(((uint32_t) (x) >> 8) & 0xFF), (((uint32_t) (x) >> 16) & 0xFF), (((uint32_t) (x) >> 24) & 0xFF)
#define INJECT16X(x)     ((uint16_t) (x) & 0xFF),(((uint16_t) (x) >> 8) & 0xFF)

#define VECTOR_CAPACITY 50

typedef struct
{
  unsigned char* data;
  unsigned short capacity;
  unsigned short size;
  unsigned short position;
} gxByteBuffer;

/*
* Fill buffer it with zeros.
*/
void bb_zero(
  gxByteBuffer* bb,
  unsigned short index,
  unsigned short count);

//Set new data to the gxByteBuffer.
void bb_setUInt8(
  gxByteBuffer* bb,
  unsigned char item);

void bb_setUInt8ByIndex(
  gxByteBuffer* bb,
  unsigned short index,
  unsigned char item);

void bb_setUInt16(
  gxByteBuffer* bb,
  unsigned short item);

void bb_setUInt16ByIndex(
  gxByteBuffer* bb,
  unsigned short index,
  unsigned short item);

void bb_setUInt32(
  gxByteBuffer* bb,
  unsigned long item);

void bb_setUInt32ByIndex(
  gxByteBuffer* bb,
  unsigned short index,
  unsigned long item);

void bb_set(
  gxByteBuffer* bb,
  const unsigned char* pSource,
  unsigned short count);

void bb_set2(
  gxByteBuffer* bb,
  gxByteBuffer* data,
  unsigned short index,
  unsigned short count);

void bb_insert(
  gxByteBuffer* arr,
  gxByteBuffer* data, unsigned short index);

void bb_addString(
  gxByteBuffer* bb,
  const char* value);

void bb_attach(
  gxByteBuffer* arr,
  unsigned char* value,
  unsigned short count);
void bb_remove(
  gxByteBuffer* arr);

void bb_reset(
  gxByteBuffer* arr);

void bb_clear(
  gxByteBuffer* bb);

int bb_getUInt8(
  gxByteBuffer* bb,
  unsigned char* value);

int bb_getUInt8ByIndex(
  gxByteBuffer* bb,
  unsigned short index,
  unsigned char* value);

int bb_getUInt16(
  gxByteBuffer* bb,
  unsigned short* value);

int bb_getUInt32(
  gxByteBuffer* bb,
  unsigned long* value);

int bb_get(
  gxByteBuffer* bb,
  unsigned char* value,
  unsigned short count);

int bb_getInt8(
  gxByteBuffer* arr,
  char* value);

int bb_getUInt16ByIndex(
  gxByteBuffer* bb,
  unsigned short index,
  unsigned short* value);

//Move data in size byte array.
int bb_move(
  gxByteBuffer* ba,
  unsigned short srcPos,
  unsigned short destPos,
  unsigned short count);

/**
   * Compares, whether two given arrays are similar starting from current
   * position.
   *
   * @param bb
   *            Bytebuffer to compare.
   * @param arr
   *            Array to compare.
   * @param length
   *            Array length.
   * @return True, if arrays are similar. False, if the arrays differ.
   */
unsigned char bb_compare(
  gxByteBuffer* bb,
  unsigned char* buff,
  unsigned short length);

void bb_setInt8(
  gxByteBuffer* arr,
  char item);

#endif //BYTE_BUFFER_H