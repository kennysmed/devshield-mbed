/*
 *  BERGCloud library for mbed
 *
 *  Copyright (c) 2013 BERG Ltd.
 *
 *  TODO: License
 */

#include <cstdint>
#include <cstddef>
#include <cstdarg>

#include "BERGCloudMbed.h"

uint16_t CBERGCloud::SPITransaction(uint8_t *pDataOut, uint8_t *pDataIn, uint16_t dataSize, bool finalCS)
{
  uint16_t i;

  if ( (pDataOut == NULL) || (pDataIn == NULL) || (m_pSPI == NULL) )
  {
    _LOG("Invalid parameter (CBERGCloud::SPITransaction)\r\n");
    return 0;
  }

  m_pnSSELPin->write(0);

  for (i = 0; i < dataSize; i++)
  {
    *pDataIn++ = m_pSPI->write(*pDataOut++);
  }

  if (finalCS)
  {
    m_pnSSELPin->write(1);
  }

  return dataSize;
}

void CBERGCloud::timerReset(void)
{
  m_pTimer->reset();
}

uint32_t CBERGCloud::timerRead_mS(void)
{
  return m_pTimer->read_ms();
}

void CBERGCloud::begin(PinName _MOSIPin, PinName _MISOPin, PinName _SCLKPin, PinName _nSSELPin)
{
  /* Call base class method */
  CBERGCloudBase::begin();

  /* Configure nSSEL control pin */
  m_pnSSELPin = new DigitalOut(_nSSELPin);

  if (m_pnSSELPin == NULL)
  {
    _LOG("m_pnSSELPin is NULL (CBERGCloud::begin)\r\n");
    return;
  }

  m_pnSSELPin->write(1);

  /* Configure SPI */
  m_pSPI = new SPI(_MOSIPin, _MISOPin, _SCLKPin);

  if (m_pSPI  == NULL)
  {
    _LOG("m_pSPI is NULL (CBERGCloud::begin)\r\n");
    delete m_pnSSELPin;
    return;
  }

  m_pSPI->format(8, 0); /* 8-bits; SPI MODE 0 */
  m_pSPI->frequency(4000000); /* 4MHz */

  /* Configure timer */
  m_pTimer = new Timer();

  if (m_pTimer  == NULL)
  {
    _LOG("m_pTimer is NULL (CBERGCloud::begin)\r\n");
    delete m_pnSSELPin;
    delete m_pSPI;
    return;
  }

  m_pTimer->start();
}

void CBERGCloud::end()
{
  if (m_pnSSELPin != NULL)
  {
    delete m_pnSSELPin;
  }

  if (m_pSPI != NULL)
  {
    delete m_pSPI;
  }

  if (m_pTimer != NULL)
  {
    delete m_pTimer;
  }

  /* Call base class method */
  CBERGCloudBase::end();
}

bool CBERGCloud::print(std::string& s)
{
  return print(s.c_str());
}

#ifdef BERGCLOUD_PACK_UNPACK

bool CMessage::pack(std::string& s)
{
  return pack(s.c_str());
}

bool CMessage::unpack(std::string& s)
{
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

  std::string tmp(m_data[m_read], sizeInBytes);
  m_read += sizeInBytes;

  s = tmp;

  return true;
}

#endif // #ifdef BERGCLOUD_PACK_UNPACK
