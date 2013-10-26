/*

BERGCloud message pack/unpack

Based on MessagePack http://msgpack.org/

Copyright (c) 2013 BERG Ltd. http://bergcloud.com/

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

#ifndef MESSAGEBASE_H
#define MESSAGEBASE_H

#include "BERGCloudConfig.h"
#include "Buffer.h"
#include "LogPrint.h"

#define _LOG_PACK_ERROR_NO_SPACE    _LOG("Pack: Out of space.\r\n")
#define _LOG_UNPACK_ERROR_TYPE      _LOG("Unpack: Can't convert to this variable type.\r\n")
#define _LOG_UNPACK_ERROR_NO_DATA   _LOG("Unpack: No more data.\r\n")

#define IN_RANGE(value, min, max) ((value >= min) && (value <= max))

class CMessageBase : public CBuffer
{
public:
  CMessageBase(void);
  ~CMessageBase(void);

  /* Remove all items */
  void clear(void);

  /*
   *  Pack methods
   */

  /* Pack an unsigned integer */
  bool pack(uint8_t n);
  bool pack(uint16_t n);
  bool pack(uint32_t n);
  /* Pack a signed integer */
  bool pack(int8_t n);
  bool pack(int16_t n);
  bool pack(int32_t n);
  /* Pack a float */
  bool pack(float n);
  /* Pack a boolean */
  bool pack(bool n);

  /* Pack a nil type */
  bool pack_nil(void);
  /* Pack an array header, giving the number of items that will follow */
  bool pack_array(uint16_t items);
  /* Pack a map header, giving the number of key-value pairs that will follow */
  bool pack_map(uint16_t items);

  /* Pack an array of data */
  bool pack(uint8_t *pData, uint16_t sizeInBytes);
  /* Pack a null-terminated C string */
  bool pack(const char *pString);

  /*
   *  Unpack methods
   */

  /* Unpack an unsigned integer */
  bool unpack(uint8_t& n);
  bool unpack(uint16_t& n);
  bool unpack(uint32_t& n);
  /* Unpack a signed integer */
  bool unpack(int8_t& n);
  bool unpack(int16_t& n);
  bool unpack(int32_t& n);
  /* Unpack a float */
  bool unpack(float& n);
  /* Unpack a boolean */
  bool unpack(bool& n);

  /* Unpack a nil type */
  bool unpack_nil(void);

  /* Unpack an array header, this gives the number of items that follow */
  bool unpack_array(uint16_t& items);
  /* Unpack a map header, this gives the number of key-value pairs that follow */
  bool unpack_map(uint16_t& items);

  /* Get the messagePack type of the next item without unpacking it, */
  /* returns false if there are no more items */
  bool unpack_peek(uint8_t& messagePackType);

#ifdef BERGCLOUD_LOG
  /* Prints the type of the next item without unpacking it, */
  /* returns false if there are no more items */
  bool unpack_peek(void);
  /* Prints the type of all items without unpacking them */
  void print(void);
  /* Print the raw MessagePack bytes of a message */
  void print_bytes(void);
#endif

  /* Skip the next item */
  bool unpack_skip(void);
  /* Restart unpacking from the beginning */
  bool unpack_restart();
  /* Moves to the value associated with the map key 'key' */
  bool unpack_find(const char *key);
  /* Moves to the value associated with array index 'i' */
  bool unpack_find(uint16_t i);

  /* Unpack a null-terminated C string */
  bool unpack(char *pString, uint32_t maxSizeInBytes);
  /* Unpack an array of data */
  bool unpack(uint8_t *pData, uint32_t maxSizeInBytes, uint32_t *pSizeInBytes = NULL);

protected:
  /* Internal methods */
  uint16_t strlen(const char *pString);
  bool strcompare(const char *pS1, const char *pS2);
  bool pack_raw_header(uint16_t sizeInBytes);
  bool pack_raw_data(uint8_t *pData, uint16_t sizeInBytes);
  bool unpack_raw_header(uint16_t *sizeInBytes);
  bool unpack_raw_data(uint8_t *pData, uint16_t packedSizeInBytes, uint16_t bufferSizeInBytes);
  bool getUnsignedInteger(uint32_t *pValue, uint8_t maxBytes);
  bool getSignedInteger(int32_t *pValue, uint8_t maxBytes);
};

#endif // #ifndef MESSAGEBASE_H
