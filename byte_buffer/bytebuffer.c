#include "bytebuffer.h"

unsigned char assert_val;
void user_assert(unsigned char *value)
{
    *value = 1;
}
void bb_zero(
  gxByteBuffer* arr,
  unsigned short index,
  unsigned short count)
{
  if (index + count > arr->capacity)
  {
    count = arr->capacity - index;
  }
  if (arr->size < index + count)
  {
    arr->size = index + count;
  }
  memset(arr->data + index, 0, count);
}

void bb_setUInt8(
  gxByteBuffer* arr,
  unsigned char item)
{
  bb_setUInt8ByIndex(arr, arr->size, item);
  ++arr->size;
}

void bb_setUInt8ByIndex(
  gxByteBuffer* arr,
  unsigned short index,
  unsigned char item)
{
  if (arr->capacity == 0)
  {
    arr->size = 0;
  }
  if ((arr->capacity == 0) || (arr->size + 1 > arr->capacity))
  {
#ifdef WIN32
    assert(0);
#else
    user_assert(&assert_val);
#endif
    return;
  }
  if (arr->size + 1 >= index)
  {
    arr->data[index] = item;
  }
}

void bb_setUInt16(
  gxByteBuffer* arr,
  unsigned short item)
{
  bb_setUInt16ByIndex(arr, arr->size, item);
  arr->size += 2;
}

void bb_setUInt16ByIndex(
  gxByteBuffer* arr,
  unsigned short index,
  unsigned short item)
{
  if (arr->capacity == 0)
  {
    arr->size = 0;
  }
  if (arr->capacity == 0 || arr->size + 2 > arr->capacity)
  {
#ifdef WIN32
    assert(0);
#else
    user_assert(&assert_val);
#endif
    return;
  }
  if (arr->size + 2 >= index)
  {
    arr->data[index] = (item >> 8) & 0xFF;
    arr->data[index + 1] = item & 0xFF;
  }
}

void bb_setUInt32(
  gxByteBuffer* arr,
  unsigned long item)
{
  bb_setUInt32ByIndex(arr, arr->size, item);
  arr->size += 4;
}

void bb_setUInt32ByIndex(
  gxByteBuffer* arr,
  unsigned short index,
  unsigned long item)
{
  if (arr->capacity == 0)
  {
    arr->size = 0;
  }
  if (arr->capacity == 0 || arr->size + 4 > arr->capacity)
  {
#ifdef WIN32
    assert(0);
#else
    user_assert(&assert_val);
#endif
    return;
  }
  if (arr->size + 4 >= index)
  {
    ((unsigned char*)arr->data)[index] = (item >> 24) & 0xFF;
    ((unsigned char*)arr->data)[index + 1] = (item >> 16) & 0xFF;
    ((unsigned char*)arr->data)[index + 2] = (item >> 8) & 0xFF;
    ((unsigned char*)arr->data)[index + 3] = item & 0xFF;
  }
}

void bb_setInt8(
  gxByteBuffer* arr,
  char item)
{
  bb_setUInt8(arr, (unsigned char)item);
}

void bb_set(
  gxByteBuffer* arr,
  const unsigned char* pSource,
  unsigned short count)
{
  if (arr->size + count > arr->capacity)
  {
#ifdef WIN32
    assert(0);
#else
    user_assert(&assert_val);
#endif
    return;
  }
  memcpy(arr->data + arr->size, pSource, count);
  arr->size += count;
}

void bb_set2(
  gxByteBuffer* arr,
  gxByteBuffer* data, unsigned short index,
  unsigned short count)
{
  if (data != NULL)
  {
    if (count == (unsigned short)-1)
    {
      count = data->size - index;
    }
    bb_set(arr, data->data + index, count);
    data->position += count;
  }
}

void bb_insert(
  gxByteBuffer* arr,
  gxByteBuffer* data, unsigned short index)
{
  if ((data != NULL) && (data->size != 0) && ((arr->size + data->size) <= arr->capacity))
  {
    if (data->size == (unsigned short)-1)
    {
      data->size = data->size - index;
    }
    bb_move(arr, arr->position, arr->position + data->size, arr->size - arr->position);
    memcpy(&arr->data[index], data->data, data->size);
  }
}

void bb_addString(
  gxByteBuffer* arr,
  const char* value)
{
  if (value != NULL)
  {
    int len = (int)strlen(value);
    if (len > 0)
    {
      bb_set(arr, (const unsigned char*)value, len);
    }
  }
}

void bb_attach(
  gxByteBuffer* arr,
  unsigned char* value,
  unsigned short count)
{
  arr->data = value;
  arr->capacity = arr->size = count;
  arr->position = 0;
}

void bb_remove(
  gxByteBuffer* arr)
{
  arr->data = NULL;
  arr->capacity = arr->size = 0;
  arr->position = 0;
}

void bb_reset(
  gxByteBuffer* arr)
{
  arr->size = 0;
  arr->position = 0;
}

void bb_clear(
  gxByteBuffer* arr)
{
  arr->capacity = 0;
  arr->size = 0;
  arr->position = 0;
}

int bb_getUInt8(
  gxByteBuffer* arr,
  unsigned char* value)
{
  if (arr->position >= arr->size)
  {
#ifdef WIN32
    assert(0);
#else
    user_assert(&assert_val);
#endif
    return DLMS_ERROR_CODE_OUTOFMEMORY;
  }
  *value = ((unsigned char*)arr->data)[arr->position];
  ++arr->position;
  return 0;
}

int bb_getInt8(
  gxByteBuffer* arr,
  char* value)
{
  if (arr->position >= arr->size)
  {
#ifdef WIN32
    assert(0);
#else
    user_assert(&assert_val);
#endif
    return DLMS_ERROR_CODE_OUTOFMEMORY;
  }
  *value = (char)((unsigned char*)arr->data)[arr->position];
  ++arr->position;
  return 0;
}

int bb_getUInt8ByIndex(
  gxByteBuffer* arr,
  unsigned short index,
  unsigned char* value)
{
  if (index >= arr->size)
  {
#ifdef WIN32
    assert(0);
#else
    user_assert(&assert_val);
#endif
    return DLMS_ERROR_CODE_OUTOFMEMORY;
  }
  *value = ((unsigned char*)arr->data)[index];
  return 0;
}

int bb_getUInt16(
  gxByteBuffer* arr,
  unsigned short* value)
{
  if (arr->position + 2 > arr->size)
  {
#ifdef WIN32
    assert(0);
#else
    user_assert(&assert_val);
#endif
    return DLMS_ERROR_CODE_OUTOFMEMORY;
  }
  *value = ((unsigned char*)arr->data)[arr->position] << 8 |
    ((unsigned char*)arr->data)[arr->position + 1];
  arr->position += 2;
  return 0;
}

int bb_getUInt32(
  gxByteBuffer* arr,
  unsigned long* value)
{
  *value = 0;
  if (arr->position + 4 > arr->size)
  {
#ifdef WIN32
    assert(0);
#else
    user_assert(&assert_val);
#endif
    return DLMS_ERROR_CODE_OUTOFMEMORY;
  }
  *value = (((unsigned char*)arr->data)[arr->position]);
  *value <<= 8;
  *value |= (((unsigned char*)arr->data)[arr->position + 1]);
  *value <<= 8;
  *value |= (((unsigned char*)arr->data)[arr->position + 2]);
  *value <<= 8;
  *value |= (((unsigned char*)arr->data)[arr->position + 3]);
  arr->position += 4;
  return 0;
}

int bb_getUInt16ByIndex(
  gxByteBuffer* arr,
  unsigned short index,
  unsigned short* value)
{
  if (index + 1 > arr->size)
  {
#ifdef WIN32
    assert(0);
#else
    user_assert(&assert_val);
#endif
    return DLMS_ERROR_CODE_OUTOFMEMORY;
  }
  *value = ((unsigned char*)arr->data)[index] << 8 |
    ((unsigned char*)arr->data)[index + 1];
  return 0;
}

void* mymemmove(void* dest, const void* src, size_t n)
{
  unsigned char* pd = dest;
  const unsigned char* ps = src;
  
  if (ps < pd)
    for (pd += n, ps += n; n--;)
      *--pd = *--ps;
  else
    while (n--)
      *pd++ = *ps++;
  return dest;
}

int bb_move(
  gxByteBuffer* bb,
  unsigned short srcPos,
  unsigned short destPos,
  unsigned short count)
{
  if (count != 0)
  {
    if (bb->capacity < destPos + count)
    {
#ifdef WIN32
      assert(0);
#else
      user_assert(&assert_val);
#endif
      return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    mymemmove(bb->data + destPos, bb->data + srcPos, count);
    bb->size = (destPos + count);
    if (bb->position > bb->size)
    {
      bb->position = bb->size;
    }
  }
  return DLMS_ERROR_CODE_OK;
}

unsigned char bb_compare(
  gxByteBuffer* bb,
  unsigned char* buff,
  unsigned short length)
{
  unsigned char equal;
  if (bb->size - bb->position < length)
  {
    return 0;
  }
  equal = memcmp(bb->data + bb->position, buff, length) == 0;
  if (equal)
  {
    bb->position += length;
  }
  return equal;
}

int bb_get(
  gxByteBuffer* bb,
  unsigned char* value,
  unsigned short count)
{
  if (bb == NULL || value == NULL || bb->size - bb->position < count)
  {
#ifdef WIN32
    assert(0);
#else
    user_assert(&assert_val);
#endif
    return DLMS_ERROR_CODE_OUTOFMEMORY;
  }
  memcpy(value, bb->data + bb->position, count);
  bb->position += count;
  return 0;
}