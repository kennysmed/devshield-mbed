/*

BERGCloud message pack/unpack

Based on MessagePack http://msgpack.org/

Copyright (c) 2013 BERG Cloud Ltd. http://bergcloud.com/

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#define __STDC_LIMIT_MACROS /* Include C99 stdint defines in C++ code */
#include <stdint.h>
#include <stddef.h> /* For NULL */
#include <string.h> /* For memcpy() */
#include "BERGCloudMessageBase.h"

BERGCloudMessageBase::BERGCloudMessageBase(void)
{
}

BERGCloudMessageBase::~BERGCloudMessageBase(void)
{
}

#define _MP_FIXNUM_POS_MIN  0x00
#define _MP_FIXNUM_POS_MAX  0x7f
#define _MP_FIXMAP_MIN      0x80
#define _MP_FIXMAP_MAX      0x8f
#define _MP_FIXARRAY_MIN    0x90
#define _MP_FIXARRAY_MAX    0x9f
#define _MP_FIXRAW_MIN      0xa0
#define _MP_FIXRAW_MAX      0xbf
#define _MP_NIL             0xc0
#define _MP_BOOL_FALSE      0xc2
#define _MP_BOOL_TRUE       0xc3
#define _MP_FLOAT           0xca
#define _MP_DOUBLE          0xcb
#define _MP_UINT8           0xcc
#define _MP_UINT16          0xcd
#define _MP_UINT32          0xce
#define _MP_UINT64          0xcf
#define _MP_INT8            0xd0
#define _MP_INT16           0xd1
#define _MP_INT32           0xd2
#define _MP_INT64           0xd3
#define _MP_RAW16           0xda
#define _MP_RAW32           0xdb
#define _MP_ARRAY16         0xdc
#define _MP_ARRAY32         0xdd
#define _MP_MAP16           0xde
#define _MP_MAP32           0xdf
#define _MP_FIXNUM_NEG_MIN  0xe0
#define _MP_FIXNUM_NEG_MAX  0xff

#define _MAX_FIXRAW         (_MP_FIXRAW_MAX - _MP_FIXRAW_MIN)
#define _MAX_FIXARRAY       (_MP_FIXARRAY_MAX - _MP_FIXARRAY_MIN)
#define _MAX_FIXMAP         (_MP_FIXMAP_MAX - _MP_FIXMAP_MIN)

uint16_t BERGCloudMessageBase::strlen(const char *string)
{
  uint16_t strLen = 0;

  /* Find string length */
  if (string != NULL)
  {
    while ((strLen < UINT16_MAX) && (*string != '\0'))
    {
       string++;
       strLen++;
    }
  }

  return strLen;
}

bool BERGCloudMessageBase::strcompare(const char *s1, const char *s2)
{
  uint16_t count = 0;

  if ((s1 == NULL) || (s2==NULL))
  {
    return false;
  }

  while ((*s1 != '\0') && (*s2 != '\0'))
  {
    if (*s1++ != *s2++)
    {
        return false;
    }

    if (count++ == UINT16_MAX)
    {
        return false;
    }
  }

  /* identical */
  return true;
}


/*
    Pack methods
*/

bool BERGCloudMessageBase::pack(uint8_t n)
{
  if (!available(sizeof(n) + 1))
  {
    _LOG_PACK_ERROR_NO_SPACE;
    return false;
  }

  add(_MP_UINT8);
  add(n);
  return true;
}

bool BERGCloudMessageBase::pack(uint16_t n)
{
  if (!available(sizeof(n) + 1))
  {
    _LOG_PACK_ERROR_NO_SPACE;
    return false;
  }

  add(_MP_UINT16);
  add((uint8_t)(n >> 8));
  add((uint8_t)n);
  return true;
}

bool BERGCloudMessageBase::pack(uint32_t n)
{
  if (!available(sizeof(n) + 1))
  {
    _LOG_PACK_ERROR_NO_SPACE;
    return false;
  }

  add(_MP_UINT32);
  add((uint8_t)(n >> 24));
  add((uint8_t)(n >> 16));
  add((uint8_t)(n >> 8));
  add((uint8_t)n);
  return true;
}

bool BERGCloudMessageBase::pack(int8_t n)
{
  if (!available(sizeof(n) + 1))
  {
    _LOG_PACK_ERROR_NO_SPACE;
    return false;
  }

  add(_MP_INT8);
  add((uint8_t)n);
  return true;
}

bool BERGCloudMessageBase::pack(int16_t n)
{
  if (!available(sizeof(n) + 1))
  {
    _LOG_PACK_ERROR_NO_SPACE;
    return false;
  }

  add(_MP_INT16);
  add((uint8_t)(n >> 8));
  add((uint8_t)n);
  return true;
}

bool BERGCloudMessageBase::pack(int32_t n)
{
  if (!available(sizeof(n) + 1))
  {
    _LOG_PACK_ERROR_NO_SPACE;
    return false;
  }

  add(_MP_INT32);
  add((uint8_t)(n >> 24));
  add((uint8_t)(n >> 16));
  add((uint8_t)(n >> 8));
  add((uint8_t)n);
  return true;
}

bool BERGCloudMessageBase::pack(float n)
{
  uint32_t data;

  if (!available(sizeof(n) + 1))
  {
    _LOG_PACK_ERROR_NO_SPACE;
    return false;
  }

  /* Convert to data */
  memcpy(&data, &n, sizeof(float));

  add(_MP_FLOAT);
  add((uint8_t)(data >> 24));
  add((uint8_t)(data >> 16));
  add((uint8_t)(data >> 8));
  add((uint8_t)data);
  return true;
}

bool BERGCloudMessageBase::pack(bool n)
{
  /*
      Note that Arduino redefines 'true' and 'false' in Arduino.h.
      You can undefine them in your code to make them type 'bool' again:
      #undef true
      #undef false
  */

  if (!available(1))
  {
    _LOG_PACK_ERROR_NO_SPACE;
    return false;
  }

  add(n ? _MP_BOOL_TRUE : _MP_BOOL_FALSE);
  return true;
}

bool BERGCloudMessageBase::pack_nil(void)
{
  if (!available(1))
  {
    _LOG_PACK_ERROR_NO_SPACE;
    return false;
  }

  add(_MP_NIL);
  return true;
}

bool BERGCloudMessageBase::pack_array(uint16_t items)
{
  if (items <= _MAX_FIXARRAY)
  {
    /* Use fix array */
    if (!available(items + 1))
    {
      _LOG_PACK_ERROR_NO_SPACE;
      return false;
    }

    add(_MP_FIXARRAY_MIN + items);
  }
  else
  {
    /* Use array 16 */
    if (!available(items + 1 + sizeof(uint16_t)))
    {
      _LOG_PACK_ERROR_NO_SPACE;
      return false;
    }

    add(_MP_ARRAY16);
    add((uint8_t)(items >> 8));
    add((uint8_t)items);
  }

  return true;
}

bool BERGCloudMessageBase::pack_map(uint16_t items)
{
  if (items <= _MAX_FIXMAP)
  {
    /* Use fix map */
    if (!available(items + 1))
    {
      _LOG_PACK_ERROR_NO_SPACE;
      return false;
    }

    add(_MP_FIXMAP_MIN + items);
  }
  else
  {
    /* Use map 16 */
    if (!available(items + 1 + sizeof(uint16_t)))
    {
      _LOG_PACK_ERROR_NO_SPACE;
      return false;
    }

    add(_MP_MAP16);
    add((uint8_t)(items >> 8));
    add((uint8_t)items);
  }

  return true;
}

bool BERGCloudMessageBase::pack(uint8_t *data, uint16_t sizeInBytes)
{
  /* Pack data */
  if (!pack_raw_header(sizeInBytes))
  {
    return false;
  }

  return pack_raw_data(data, sizeInBytes);
}

bool BERGCloudMessageBase::pack(const char *string)
{
  /* Pack a null-terminated C string */
  uint16_t strLen;

  strLen = BERGCloudMessageBase::strlen(string);

  return pack((uint8_t *)string, strLen);
}

/* Separate header and data methods are provided for raw data*/
/* so that Arduino strings may be packed without having to create */
/* a temporary buffer first. */

bool BERGCloudMessageBase::pack_raw_header(uint16_t sizeInBytes)
{
  if (sizeInBytes <= _MAX_FIXRAW)
  {
    /* Use fix raw */
    if (!available(sizeInBytes + 1))
    {
      _LOG_PACK_ERROR_NO_SPACE;
      return false;
    }

    add(_MP_FIXRAW_MIN + sizeInBytes);
  }
  else
  {
    /* Use raw 16 */
    if (!available(sizeInBytes + 1 + sizeof(uint16_t)))
    {
      _LOG_PACK_ERROR_NO_SPACE;
      return false;
    }

    add(_MP_RAW16);
    add((uint8_t)(sizeInBytes >> 8));
    add((uint8_t)sizeInBytes);
  }

  return true;
}

bool BERGCloudMessageBase::pack_raw_data(uint8_t *data, uint16_t sizeInBytes)
{
  /* Add data */
  while (sizeInBytes-- > 0)
  {
    add(*data++);
  }

  return true;
}

/*
    Unpack methods
*/

bool BERGCloudMessageBase::unpack_peek(uint8_t& messagePackType)
{
  return peek(&messagePackType);
}

#ifdef BERGCLOUD_LOG
bool BERGCloudMessageBase::unpack_peek(void)
{
  uint8_t type;

  if (!peek(&type))
  {
    return false;
  }

  if (type <= _MP_FIXNUM_POS_MAX)
  {
    _LOG("Positive integer\r\n");
    return true;
  }

  if (IN_RANGE(type, _MP_FIXNUM_NEG_MIN, _MP_FIXNUM_NEG_MAX))
  {
    _LOG("Negative integer\r\n");
    return true;
  }

  if ((type == _MP_MAP16) || (type == _MP_MAP32) || IN_RANGE(type, _MP_FIXMAP_MIN, _MP_FIXMAP_MAX))
  {
    _LOG("Map\r\n");
    return true;
  }

  if ((type == _MP_ARRAY16) || (type == _MP_ARRAY32) || IN_RANGE(type, _MP_FIXARRAY_MIN, _MP_FIXARRAY_MAX))
  {
    _LOG("Array\r\n");
    return true;
  }

  if ((type == _MP_RAW16) || (type == _MP_RAW32) || IN_RANGE(type, _MP_FIXRAW_MIN, _MP_FIXRAW_MAX))
  {
    _LOG("Raw\r\n");
    return true;
  }

  if ((type ==_MP_UINT8) || (type == _MP_UINT16) || (type == _MP_UINT32) || (type == _MP_UINT64))
  {
    _LOG("Unsigned integer\r\n");
    return true;
  }

  if ((type ==_MP_INT8) || (type == _MP_INT16) || (type == _MP_INT32) || (type == _MP_INT64))
  {
    _LOG("Signed integer\r\n");
    return true;
  }

  if (type == _MP_NIL)
  {
    _LOG("Nil\r\n");
    return true;
  }

  if (type == _MP_BOOL_FALSE)
  {
    _LOG("Boolean false\r\n");
    return true;
  }

  if (type == _MP_BOOL_TRUE)
  {
    _LOG("Boolean true\r\n");
    return true;
  }

  if (type == _MP_FLOAT)
  {
    _LOG("Float\r\n");
    return true;
  }

  if (type == _MP_DOUBLE)
  {
    _LOG("Double\r\n");
    return true;
  }

  _LOG("Unknown\r\n");
  return false;
}

void BERGCloudMessageBase::print(void)
{
  uint16_t last_read;

  /* Remember the current read position in the raw data */
  last_read = bytesRead;

  /* Start reading from the beginning of the data */
  restart();

  /* Print all items */
  while(unpack_peek())
  {
    unpack_skip();
  }

  /* Return to the last position */
  bytesRead = last_read;
}

void BERGCloudMessageBase::print_bytes(void)
{
  uint16_t size = used();
  uint8_t *data = ptr();

  while (size-- > 0)
  {
    _LOG_HEX(*data);
    _LOG(" ");
    data++;
  }
  _LOG("\r\n");
}
#endif

bool BERGCloudMessageBase::getInteger(void *value, bool valueIsSigned, int32_t min, uint32_t max)
{
  uint8_t type;
  uint32_t unsignedValue;
  int32_t signedValue;

  /* Look at next type */
  if (!peek(&type))
  {
    _LOG_UNPACK_ERROR_NO_DATA;
    return false;
  }

  /*
   * Unsigned types
   */

  if (type <= _MP_FIXNUM_POS_MAX)
  {
    /* Read positive fix num value */
    unsignedValue = read();
  }
  else if (type == _MP_UINT8)
  {
    if (!remaining(sizeof(uint8_t)))
    {
      _LOG_UNPACK_ERROR_NO_DATA;
      return false;
    }

    /* Read type */
    read();

    /* Read 8-bit unsigned integer */
    unsignedValue = read();
  }
  else if (type == _MP_UINT16)
  {
    if (!remaining(sizeof(uint16_t)))
    {
      _LOG_UNPACK_ERROR_NO_DATA;
      return false;
    }

    /* Read type */
    read();

    /* Read 16-bit unsigned integer */
    unsignedValue = read();
    unsignedValue <<= 8;
    unsignedValue |= read();
  }
  else if (type == _MP_UINT32)
  {
    if (!remaining(sizeof(uint32_t)))
    {
      _LOG_UNPACK_ERROR_NO_DATA;
      return false;
    }

    /* Read type */
    read();

    /* Read 32-bit unsigned integer */
    unsignedValue = read();
    unsignedValue <<= 8;
    unsignedValue |= read();
    unsignedValue <<= 8;
    unsignedValue |= read();
    unsignedValue <<= 8;
    unsignedValue |= read();
  }
  else
  {
    /*
     * Signed values
     */

    if (IN_RANGE(type, _MP_FIXNUM_NEG_MIN, _MP_FIXNUM_NEG_MAX))
    {
      /* Read negative fix num value */
      uint8_t temp;
      temp = read();
      signedValue = (int8_t)temp; /* Convert with sign extension */
    }
    else if (type == _MP_INT8)
    {
      if (!remaining(sizeof(int8_t)))
      {
        _LOG_UNPACK_ERROR_NO_DATA;
        return false;
      }

      /* Read type */
      read();

      /* Read 8-bit signed integer */
      uint8_t temp;
      temp = read();
      signedValue = (int8_t)temp; /* Convert with sign extension */
    }
    else if (type == _MP_INT16)
    {
      if (!remaining(sizeof(int16_t)))
      {
        _LOG_UNPACK_ERROR_NO_DATA;
        return false;
      }

      /* Read type */
      read();

      /* Read 16-bit signed integer */
      uint16_t temp;
      temp = read();
      temp <<= 8;
      temp |= read();
      signedValue = (int16_t)temp; /* Convert with sign extension */
    }
    else if (type == _MP_INT32)
    {
      if (!remaining(sizeof(int32_t)))
      {
        _LOG_UNPACK_ERROR_NO_DATA;
        return false;
      }

      /* Read type */
      read();

      /* Read 32-bit signed integer */
      uint32_t temp;
      temp = read();
      temp <<= 8;
      temp |= read();
      temp <<= 8;
      temp |= read();
      temp <<= 8;
      temp |= read();
      signedValue = (int32_t)temp; /* Convert */
    }
    else
    {
      /* Can't convert this type */
      _LOG_UNPACK_ERROR_TYPE;
      return false;
    }

    /* Signed value decoded check range */
    if (signedValue > 0)
    {
      if ((uint32_t)signedValue > max)
      {
        _LOG_UNPACK_ERROR_RANGE;
        return false;
      }
    }
    else if (signedValue < min)
    {
      _LOG_UNPACK_ERROR_RANGE;
      return false;
    }

    if (valueIsSigned)
    {
      *(int32_t *)value = signedValue;
    }
    else
    {
      /* Convert to unsigned */
      *(uint32_t *)value = (uint32_t)signedValue;
    }

    /* Success */
    return true;
  }

  /* Unsigned value decoded; check range */
  if (unsignedValue > max)
  {
    _LOG_UNPACK_ERROR_RANGE;
    return false;
  }

  if (valueIsSigned)
  {
    /* Convert to signed */
    *(int32_t *)value = (int32_t)unsignedValue;
  }
  else
  {
    *(uint32_t *)value = unsignedValue;
  }

  /* Success */
  return true;
}

bool BERGCloudMessageBase::unpack_skip(void)
{
  /* Skip next item */

  uint8_t type;
  uint32_t bytesToSkip = 0;

  /* Must be at least one byte of data */
  if (!remaining(sizeof(uint8_t)))
  {
    _LOG_UNPACK_ERROR_NO_DATA;
    return false;
  }

  /* Read type */
  type = read();

  if ((type == _MP_UINT8) || (type == _MP_INT8))
  {
    bytesToSkip = 1;
  }
  else if ((type == _MP_UINT16) || (type == _MP_INT16))
  {
    bytesToSkip = 2;
  }
  else if ((type == _MP_UINT32) || (type == _MP_INT32) || (type == _MP_FLOAT))
  {
    bytesToSkip = 4;
  }
  else if ((type == _MP_UINT64) || (type == _MP_INT64) || (type == _MP_DOUBLE))
  {
    bytesToSkip = 8;
  }
  else if ((type == _MP_ARRAY16) || (type == _MP_MAP16))
  {
    /* TODO: This could skip all of the array/map elements too. */
    bytesToSkip = 2;
  }
  else if ((type == _MP_ARRAY32) || (type == _MP_MAP32))
  {
    /* TODO: This could skip all of the array/map elements too. */
    bytesToSkip = 4;
  }
  else if (type == _MP_RAW16)
  {
    if (!remaining(sizeof(uint16_t)))
    {
      _LOG_UNPACK_ERROR_NO_DATA;
      return false;
    }

    /* Read 16-bit unsigned integer, data size */
    bytesToSkip = read();
    bytesToSkip = bytesToSkip << 8;
    bytesToSkip |= read();
  }
  else if (type == _MP_RAW32)
  {
    if (!remaining(sizeof(uint32_t)))
    {
      _LOG_UNPACK_ERROR_NO_DATA;
      return false;
    }

    /* read 32-bit unsigned integer, data size */
    bytesToSkip = read();
    bytesToSkip = bytesToSkip << 8;
    bytesToSkip |= read();
    bytesToSkip = bytesToSkip << 8;
    bytesToSkip |= read();
    bytesToSkip = bytesToSkip << 8;
    bytesToSkip |= read();
  }
  else if (IN_RANGE(type, _MP_FIXRAW_MIN, _MP_FIXRAW_MAX))
  {
    bytesToSkip = type - _MP_FIXRAW_MIN;
  }

  if (!remaining(bytesToSkip))
  {
    _LOG_UNPACK_ERROR_NO_DATA;
    return false;
  }

  while (bytesToSkip-- > 0)
  {
    /* Discard data */
    read();
  }

  /* Success */
  return true;
}

bool BERGCloudMessageBase::unpack(uint8_t& n)
{
  uint32_t temp;

  if (!getInteger(&temp, false, 0, UINT8_MAX))
  {
    return false;
  }

  n = (uint8_t)temp;
  return true;
}

bool BERGCloudMessageBase::unpack(uint16_t& n)
{
  uint32_t temp;

  if (!getInteger(&temp, false, 0, UINT16_MAX))
  {
    return false;
  }

  n = (uint16_t)temp;
  return true;
}

bool BERGCloudMessageBase::unpack(uint32_t& n)
{
  uint32_t temp;

  if (!getInteger(&temp, false, 0, UINT32_MAX))
  {
    return false;
  }

  n = temp;
  return true;
}

bool BERGCloudMessageBase::unpack(int8_t& n)
{
  int32_t temp;

  if (!getInteger(&temp, true, INT8_MIN, INT8_MAX))
  {
    return false;
  }

  n = (int8_t)temp;
  return true;
}

bool BERGCloudMessageBase::unpack(int16_t& n)
{
  int32_t temp;

  if (!getInteger(&temp, true, INT16_MIN, INT16_MAX))
  {
    return false;
  }

  n = (uint16_t)temp;
  return true;
}

bool BERGCloudMessageBase::unpack(int32_t& n)
{
  int32_t temp;

  if (!getInteger(&temp, true, INT32_MIN, INT32_MAX))
  {
    return false;
  }

  n = temp;
  return true;
}

bool BERGCloudMessageBase::unpack(float& n)
{
  /* Try to decode the next messagePack item as an 4-byte float */
  uint32_t data;
  uint8_t type;

  /* Look at next type */
  if (!peek(&type))
  {
    _LOG_UNPACK_ERROR_NO_DATA;
    return false;
  }

  if (type == _MP_FLOAT)
  {
    if (!remaining(sizeof(float)))
    {
      _LOG_UNPACK_ERROR_NO_DATA;
      return false;
    }

    /* Read type */
    read();

    /* Read 32-bit float */
    data = read();
    data = data << 8;
    data |= read();
    data = data << 8;
    data |= read();
    data = data << 8;
    data |= read();

    /* Convert to float */
    memcpy(&n, &data, sizeof(float));

    /* Success */
    return true;
  }

  /* Can't convert this type */
  _LOG_UNPACK_ERROR_TYPE;
  return false;
}

bool BERGCloudMessageBase::unpack(bool& n)
{
  /* Try to decode the next messagePack item as boolean */
  uint8_t type;

  /* Look at next type */
  if (!peek(&type))
  {
    _LOG_UNPACK_ERROR_NO_DATA;
    return false;
  }

  if ((type == _MP_BOOL_FALSE) || (type == _MP_BOOL_TRUE))
  {
    n = (read() == _MP_BOOL_TRUE);

    /* Success */
    return true;
  }

  /* Can't convert this type */
  _LOG_UNPACK_ERROR_TYPE;
  return false;
}

bool BERGCloudMessageBase::unpack_nil(void)
{
  /* Try to decode the next messagePack item as nil */
  uint8_t type;

  /* Look at next type */
  if (!peek(&type))
  {
    _LOG_UNPACK_ERROR_NO_DATA;
    return false;
  }

  if (type == _MP_NIL)
  {
    /* Read type */
    read();

    /* Success */
    return true;
  }

  /* Can't convert this type */
  _LOG_UNPACK_ERROR_TYPE;
  return false;
}

bool BERGCloudMessageBase::unpack_array(uint16_t& items)
{
  /* Try to decode the next messagePack item as array */
  uint8_t type;

  /* Look at next type */
  if (!peek(&type))
  {
    _LOG_UNPACK_ERROR_NO_DATA;
    return false;
  }

  if (IN_RANGE(type, _MP_FIXARRAY_MIN, _MP_FIXARRAY_MAX))
  {
    /* Read fix num value */
    items = read() - _MP_FIXARRAY_MIN;

    /* Success */
    return true;
  }

  if (type == _MP_ARRAY16)
  {
    if (!remaining(sizeof(uint16_t)))
    {
      _LOG_UNPACK_ERROR_NO_DATA;
      return false;
    }

    /* Read type */
    read();

    /* Read 16-bit unsigned integer */
    items = read();
    items = items << 8;
    items |= read();

    /* Success */
    return true;
  }

  if (type == _MP_ARRAY32)
  {
    /* Not yet supported */
    _LOG_UNPACK_ERROR_TYPE;
    return false;
  }

  /* Can't convert this type */
  _LOG_UNPACK_ERROR_TYPE;
  return false;
}

bool BERGCloudMessageBase::unpack_map(uint16_t& items)
{
  /* Try to decode the next messagePack item as array */
  uint8_t type;

  /* Look at next type */
  if (!peek(&type))
  {
    _LOG_UNPACK_ERROR_NO_DATA;
    return false;
  }

  if (IN_RANGE(type, _MP_FIXMAP_MIN, _MP_FIXMAP_MAX))
  {
    /* Read fix num value */
    items = read() - _MP_FIXMAP_MIN;

    /* Success */
    return true;
  }

  if (type == _MP_MAP16)
  {
    if (!remaining(sizeof(uint16_t)))
    {
      _LOG_UNPACK_ERROR_NO_DATA;
      return false;
    }

    /* Read type */
    read();

    /* Read 16-bit unsigned integer */
    items = read();
    items = items << 8;
    items |= read();

    /* Success */
    return true;
  }

  if (type == _MP_MAP32)
  {
    /* Not yet supported */
    _LOG_UNPACK_ERROR_TYPE;
    return true;
  }

  /* Can't convert this type */
  _LOG_UNPACK_ERROR_TYPE;
  return false;
}

/* Separate header and data methods are provided for raw data*/
/* so that Arduino strings may be unpacked without having to create */
/* a temporary buffer first. */

bool BERGCloudMessageBase::unpack_raw_header(uint16_t *sizeInBytes)
{
  uint8_t type;

  /* Look at next type */
  if (!peek(&type))
  {
    _LOG_UNPACK_ERROR_NO_DATA;
    return false;
  }

  if (IN_RANGE(type, _MP_FIXRAW_MIN, _MP_FIXRAW_MAX))
  {
    /* Read fix raw value */
    *sizeInBytes = read() - _MP_FIXRAW_MIN;

    /* Success */
    return true;
  }

  if (type == _MP_RAW16)
  {
    if (!remaining(sizeof(uint16_t)))
    {
      _LOG_UNPACK_ERROR_NO_DATA;
      return false;
    }

    /* Read type */
    read();

    /* Read 16-bit unsigned integer */
    *sizeInBytes = read();
    *sizeInBytes = *sizeInBytes << 8;
    *sizeInBytes |= read();

    /* Success */
    return true;
  }

  if (type == _MP_RAW32)
  {
    if (!remaining(sizeof(uint32_t)))
    {
      _LOG_UNPACK_ERROR_NO_DATA;
      return false;
    }

    /* Read type */
    read();

    /* read 32-bit unsigned integer */
    *sizeInBytes = read();
    *sizeInBytes = *sizeInBytes << 8;
    *sizeInBytes |= read();
    *sizeInBytes = *sizeInBytes << 8;
    *sizeInBytes |= read();
    *sizeInBytes = *sizeInBytes << 8;
    *sizeInBytes |= read();

    /* Success */
    return true;
  }

  /* Can't convert this type */
  _LOG_UNPACK_ERROR_TYPE;
  return false;
}

bool BERGCloudMessageBase::unpack_raw_data(uint8_t *pData, uint16_t packedSizeInBytes, uint16_t bufferSizeInBytes)
{
  uint8_t data;

  /* Unpack all bytes */
  while (packedSizeInBytes-- > 0)
  {
    data = read();
    
    /* Only write up to the buffer size */
    if (bufferSizeInBytes-- > 0)
    {
      *pData++ = data;
    }
  }

  return true;
}

bool BERGCloudMessageBase::unpack(char *pString, uint32_t maxSizeInBytes)
{
  /* Try to decode a null-terminated C string */
  uint16_t sizeInBytes;

  if (!unpack_raw_header(&sizeInBytes))
  {
    return false;
  }

  if (!remaining(sizeInBytes))
  {
    _LOG_UNPACK_ERROR_NO_DATA;
    return false;
  }

  if (!unpack_raw_data((uint8_t *)pString, sizeInBytes, maxSizeInBytes - 1)) /* -1 to allow space for null terminator */
  {
    return false;
  }

  /* Add null terminator */
  pString[sizeInBytes] = '\0';

  /* Success */
  return true;
}

bool BERGCloudMessageBase::unpack(uint8_t *pData, uint32_t maxSizeInBytes, uint32_t *pSizeInBytes)
{
  /* Try to decode a block of raw data */
  uint16_t sizeInBytes;

  if (!unpack_raw_header(&sizeInBytes))
  {
    return false;
  }

  if (!remaining(sizeInBytes))
  {
    _LOG_UNPACK_ERROR_NO_DATA;
    return false;
  }

  if (pSizeInBytes != NULL)
  {
    *pSizeInBytes = sizeInBytes;
  }

  return unpack_raw_data(pData, sizeInBytes, maxSizeInBytes);
}

bool BERGCloudMessageBase::unpack_find(const char *key)
{
  /* Search for a string key in a map; in this simple */
  /* implementation maps cannot contain maps or arrays */
  uint16_t last_read;
  uint16_t map_items;
  uint8_t type;
  char keyString[MAX_MAP_KEY_STRING_LENGTH+1]; /* +1 for null terminator */

  if (key == NULL)
  {
     return false;
  }

  if (strlen(key) > MAX_MAP_KEY_STRING_LENGTH)
  {
    _LOG("Unpack: Key name too long.\r\n");
    return false;
  }

  /* Remember the current read position in the raw data */
  last_read = bytesRead;

  /* Start reading from the beginning of the data */
  restart();

  while(peek(&type))
  {
    if (IN_RANGE(type, _MP_FIXMAP_MIN, _MP_FIXMAP_MAX) || (type == _MP_MAP16)) /* _MP_MAP32 not yet supported */
    {
      /* Map found, get number of items */
      unpack_map(map_items);

      /* Iterate through the key-value pairs */
      while (map_items-- > 0)
      {
        if (unpack(keyString, sizeof(keyString)))
        {
          /* String key found */
          if (strcompare(keyString, key))
          {
              /* Match found */
              return true;
          }
          else
          {
              /* No match, skip this value */
              unpack_skip();
          }
        }
        else
        {
          /* Not a suitable string key; skip this key and value */
          unpack_skip();
          unpack_skip();
        }
      }
    }
    else
    {
      /* Not a map */
      unpack_skip();
    }
  }

  /* Not found; return to last position */
  bytesRead = last_read;
  return false;
}

bool BERGCloudMessageBase::unpack_find(uint16_t i)
{
  /* Search for an index in an array; in this simple */
  /* implementation arrays cannot contain maps or arrays */
  uint16_t last_read;
  uint16_t array_items;
  uint16_t item;
  uint8_t type;

  /* Remember the current read position in the raw data */
  last_read = bytesRead;

  /* Start reading from the beginning of the data */
  restart();

  while(peek(&type))
  {
    if (IN_RANGE(type, _MP_FIXARRAY_MIN, _MP_FIXARRAY_MAX) || (type == _MP_ARRAY16)) /* _MP_ARRAY32 not yet supported */
    {
      /* Array found, get number of items */
      unpack_array(array_items);

      /* Assume items are numbered starting from one */
      item = 1;

      if (i == 0)
      {
        _LOG("Unpack: Array indexes start from 1.\r\n");
        return false;
      }

      /* Iterate through the values in the array */
      while (array_items-- > 0)
      {
        if (item++ == i)
        {
            /* Found it */
            return true;
        }
        else
        {
          /* Skip this item */
          unpack_skip();
        }
      }
    }
    else
    {
      /* Not an array */
      unpack_skip();
    }
  }

  /* Not found; return to last position */
  bytesRead = last_read;
  return false;
}
