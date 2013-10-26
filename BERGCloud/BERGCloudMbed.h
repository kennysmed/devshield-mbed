/*
 *  BERGCloud library for mbed
 *
 *  Copyright (c) 2013 BERG Ltd.
 *
 *  TODO: License
 */

#ifndef BERGCLOUDMBED_H
#define BERGCLOUDMBED_H

#include "mbed.h"
#include "BERGCloudBase.h"

#ifdef BERGCLOUD_PACK_UNPACK
#include <string>
#include "MessageBase.h"
#endif

class CBERGCloud : public CBERGCloudBase
{
public:
  void begin(PinName _MOSIPin, PinName _MISOPin, PinName _SCLKPin, PinName _nSSELPin);
  void end();
  using CBERGCloudBase::print;
  /* Methods using std::string class */
  bool print(std::string& s);
private:
  virtual uint16_t SPITransaction(uint8_t *pDataOut, uint8_t *pDataIn, uint16_t dataSize, bool finalCS);
  virtual void timerReset(void);
  virtual uint32_t timerRead_mS(void);
  SPI *m_pSPI;
  DigitalOut *m_pnSSELPin;
  Timer *m_pTimer;
};

#ifdef BERGCLOUD_PACK_UNPACK

class CMessage : public CMessageBase
{
public:
  using CMessageBase::pack;
  using CMessageBase::unpack;
  /* Methods using std::string class */
  bool pack(std::string& s);
  bool unpack(std::string& s);
};

#endif // #ifdef BERGCLOUD_PACK_UNPACK

#endif // #ifndef BERGCLOUDMBED_H
