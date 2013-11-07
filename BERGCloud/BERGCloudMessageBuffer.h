/*

Simple message buffer

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

#ifndef BERGCLOUDMESSAGEBUFFER_H
#define BERGCLOUDMESSAGEBUFFER_H

#define __STDC_LIMIT_MACROS /* Include C99 stdint defines in C++ code */
#include <stdint.h>

/* Default buffer size */
#ifndef BUFFER_SIZE_BYTES
#define BUFFER_SIZE_BYTES 64
#endif

class BERGCloudMessageBuffer
{
public:
  BERGCloudMessageBuffer();
  uint16_t size(void);
  uint8_t *ptr(void);
  void clear(void);

  /* Methods for writing to the buffer */
  uint16_t used(void);
  void used(uint16_t used);
  uint16_t available(void);
  bool available(uint16_t required);
  void add(uint8_t data);

  /* Methods for reading from the buffer */
  bool peek(uint8_t *data);
  uint8_t read(void);
  uint16_t remaining(void);
  bool remaining(uint16_t required);
  void restart(void);

protected:
  uint8_t buffer[BUFFER_SIZE_BYTES];
  uint16_t bytesWritten;
  uint16_t bytesRead;
};

#endif // #ifndef BERGCLOUDMESSAGEBUFFER_H
