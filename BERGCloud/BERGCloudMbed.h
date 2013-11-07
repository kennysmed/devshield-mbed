/*

BERGCloud library for mbed

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

#ifndef BERGCLOUDMBED_H
#define BERGCLOUDMBED_H

#include "mbed.h"
#include <string>
#include "BERGCloudBase.h"

#ifdef BERGCLOUD_PACK_UNPACK
#include "BERGCloudMessageBase.h"
#endif

class BERGCloudMbed : public BERGCloudBase
{
public:
  void begin(PinName _MOSIPin, PinName _MISOPin, PinName _SCLKPin, PinName _nSSELPin);
  void end();
  using BERGCloudBase::print;
  /* Methods using std::string class */
  bool print(std::string& s);
private:
  virtual uint16_t SPITransaction(uint8_t *dataOut, uint8_t *dataIn, uint16_t dataSize, bool finalCS);
  virtual void timerReset(void);
  virtual uint32_t timerRead_mS(void);
  SPI *spi;
  DigitalOut *nSSELPin;
  Timer *timer;
};

#ifdef BERGCLOUD_PACK_UNPACK

class BERGCloudMessage : public BERGCloudMessageBase
{
public:
  using BERGCloudMessageBase::pack;
  using BERGCloudMessageBase::unpack;
  /* Methods using std::string class */
  bool pack(std::string& s);
  bool unpack(std::string& s);
};

#endif // #ifdef BERGCLOUD_PACK_UNPACK

#endif // #ifndef BERGCLOUDMBED_H
