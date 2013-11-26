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

#ifndef BERGCLOUDMESSAGEBASE_H
#define BERGCLOUDMESSAGEBASE_H

#include "BERGCloudConfig.h"
#include "BERGCloudMessageBuffer.h"
#include "BERGCloudLogPrint.h"

#define _LOG_PACK_ERROR_NO_SPACE    _LOG("Pack: Out of space.\r\n")
#define _LOG_UNPACK_ERROR_TYPE      _LOG("Unpack: Can't convert to this variable type.\r\n")
#define _LOG_UNPACK_ERROR_RANGE     _LOG("Unpack: Value out of range for this variable type.\r\n")
#define _LOG_UNPACK_ERROR_NO_DATA   _LOG("Unpack: No more data.\r\n")

#define IN_RANGE(value, min, max) ((value >= min) && (value <= max))

#define MAX_MAP_KEY_STRING_LENGTH (16)

class BERGCloudMessageBase : public BERGCloudMessageBuffer
{
public:
  BERGCloudMessageBase(void);
  ~BERGCloudMessageBase(void);

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
  bool pack(uint8_t *data, uint16_t sizeInBytes);
  /* Pack a null-terminated C string */
  bool pack(const char *string);

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
  bool unpack(char *string, uint32_t maxSizeInBytes);
  /* Unpack an array of data */
  bool unpack(uint8_t *data, uint32_t maxSizeInBytes, uint32_t *sizeInBytes = NULL);

protected:
  /* Internal methods */
  uint16_t strlen(const char *string);
  bool strcompare(const char *s1, const char *s2);
  bool pack_raw_header(uint16_t sizeInBytes);
  bool pack_raw_data(uint8_t *data, uint16_t sizeInBytes);
  bool unpack_raw_header(uint16_t *sizeInBytes);
  bool unpack_raw_data(uint8_t *data, uint16_t packedSizeInBytes, uint16_t bufferSizeInBytes);
  bool getInteger(void *value, bool valueIsSigned, int32_t min, uint32_t max);
};

#endif // #ifndef BERGCLOUDMESSAGEBASE_H
