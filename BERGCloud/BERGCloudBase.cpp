/*

BERGCloud library common API

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


#define __STDC_LIMIT_MACROS /* Include C99 stdint defines in C++ code */
#include <stdint.h>
#include <stddef.h>
#include <string.h> /* For memcpy() */

#include "BERGCloudBase.h"

#define SPI_POLL_TIMEOUT_MS 1000
#define SPI_SYNC_TIMEOUT_MS 10000

#define CONNECT_POLL_RATE_MS 250

uint8_t CBERGCloudBase::nullProductID[BC_PRODUCT_KEY_SIZE_BYTES] = {0};

bool CBERGCloudBase::transaction(_BC_SPI_TRANSACTION *pTr)
{
  uint16_t i, j;
  uint8_t rxByte;
  bool timeout;
  uint8_t dataSize;
  uint16_t groupSize;
  uint16_t dataCRC;
  uint16_t calcCRC;
  uint8_t header[SPI_HEADER_SIZE_BYTES];
  uint8_t footer[SPI_FOOTER_SIZE_BYTES];

  /* Check synchronisation */
  if (!m_synced)
  {
    timerReset();

    do {
      rxByte = SPITransaction(SPI_PROTOCOL_PAD, true);
      timeout = timerRead_mS() > SPI_SYNC_TIMEOUT_MS;

    } while ((rxByte != SPI_PROTOCOL_RESET) && !timeout);

    if (timeout)
    {
      _LOG("Timeout, sync (CBERGCloudBase::transaction)\r\n");
      return false;
    }

    /* Resynchronisation successful */
    m_synced = true;
  }

  /* Calculate total data size */
  dataSize = 0;

  for (i=0; i<_TX_GROUPS; i++)
  {
    dataSize += pTr->tx[i].dataSize;
  }

  /* Initialise CRC */
  calcCRC = 0xffff;

  /* Create header */
  header[0] = pTr->command;
  header[1] = 0x00; /* Reserved */
  header[2] = 0x00; /* Reserved */
  header[3] = dataSize;

  /* Send header */
  for (i=0; i<sizeof(header); i++)
  {

    calcCRC = Crc16(header[i], calcCRC);
    rxByte = SPITransaction(header[i], false);

    if (rxByte == SPI_PROTOCOL_RESET)
    {
      _LOG("Reset, send header (CBERGCloudBase::transaction)\r\n");
      return false;
    }

    if (rxByte != SPI_PROTOCOL_PAD)
    {
      _LOG("SyncErr, send header (CBERGCloudBase::transaction)\r\n");
      m_synced = false;
      return false;
    }
  }

  /* Send data groups */
  for (i=0; i<_TX_GROUPS; i++)
  {
    for (j=0; j<pTr->tx[i].dataSize; j++)
    {
      calcCRC = Crc16(pTr->tx[i].pBuffer[j], calcCRC);
      rxByte = SPITransaction(pTr->tx[i].pBuffer[j], false);

      if (rxByte == SPI_PROTOCOL_RESET)
      {
        _LOG("Reset, send data (CBERGCloudBase::transaction)\r\n");
        return false;
      }

      if (rxByte != SPI_PROTOCOL_PAD)
      {
        _LOG("SyncErr, send data (CBERGCloudBase::transaction)\r\n");
        m_synced = false;
        return false;
      }
    }
  }

  /* Create footer */
  footer[0] = calcCRC >> 8;
  footer[1] = calcCRC & 0xff;

  /* Send footer */
  for (i=0; i<sizeof(footer); i++)
  {
    rxByte = SPITransaction(footer[i], false);

    if (rxByte == SPI_PROTOCOL_RESET)
    {
      _LOG("Reset, send footer (CBERGCloudBase::transaction)\r\n");
      return false;
    }

    if (rxByte != SPI_PROTOCOL_PAD)
    {
      _LOG("SyncErr, send footer (CBERGCloudBase::transaction)\r\n");
      m_synced = false;
      return false;
    }
  }

  /* Poll for response */
  timerReset();

  do {
    rxByte = SPITransaction(SPI_PROTOCOL_PAD, false);

    if (rxByte == SPI_PROTOCOL_RESET)
    {
      _LOG("Reset, poll (CBERGCloudBase::transaction)\r\n");
      return false;
    }

    if (rxByte == SPI_PROTOCOL_PENDING)
    {
      /* Waiting for data; reset timeout */
      timerReset();
    }

    timeout = timerRead_mS() > SPI_POLL_TIMEOUT_MS;

  } while (((rxByte == SPI_PROTOCOL_PAD) || (rxByte == SPI_PROTOCOL_PENDING)) && !timeout);

  if (timeout)
  {
    _LOG("Timeout, poll (CBERGCloudBase::transaction)\r\n");
    m_synced = false;
    return false;
  }

  /* Initialise CRC */
  calcCRC = 0xffff;

  /* Read header, we already have the first byte */
  header[0] = rxByte;
  calcCRC = Crc16(header[0], calcCRC);

  for (i=1; i < SPI_HEADER_SIZE_BYTES; i++)
  {
    header[i] = SPITransaction(SPI_PROTOCOL_PAD, false);
    calcCRC = Crc16(header[i], calcCRC);
  }

  /* Get data size */
  dataSize = header[3];

  /* Read data groups */
  for (i=0; i<_RX_GROUPS; i++)
  {
    groupSize = pTr->rx[i].bufferSize;
    j = 0; /* Start of the group buffer */

    while((dataSize > 0) && (groupSize > 0))
    {
      pTr->rx[i].pBuffer[j] = SPITransaction(SPI_PROTOCOL_PAD, false);
      calcCRC = Crc16(pTr->rx[i].pBuffer[j], calcCRC);

      /* Next */
      j++;

      /* Update the total data remaining and the space remaining in this group. */
      dataSize--;
      groupSize--;
    }

    if (pTr->rx[i].pDataSize != NULL)
    {
      /* Return the number of bytes used in this buffer */
      *pTr->rx[i].pDataSize = pTr->rx[i].bufferSize - groupSize;
    }
  }

  if (dataSize > 0)
  {
    /* Too much data sent */
    _LOG("SizeErr, allocate rx groups (2) (CBERGCloudBase::transaction)\r\n");
    m_synced = false;
    return false;
  }

  /* Read CRC; set nCS high */
  dataCRC = SPITransaction(SPI_PROTOCOL_PAD, false); /* MSByte */
  dataCRC <<= 8;
  dataCRC |= SPITransaction(SPI_PROTOCOL_PAD, true /* nCS -> high */); /* LSByte */

  /* Compare with calculated CRC */
  if (calcCRC != dataCRC)
  {
    /* Invalid CRC */
    _LOG("CRCErr, read data (CBERGCloudBase::transaction)\r\n");
    m_synced = false;
    return false;
  }

  /* Get reponse code */
  m_lastResponse = header[0];

  return (m_lastResponse == SPI_RSP_SUCCESS);
}

void CBERGCloudBase::initTransaction(_BC_SPI_TRANSACTION *pTr)
{
  memset(pTr, 0x00, sizeof(_BC_SPI_TRANSACTION));
}

bool CBERGCloudBase::pollForCommand(uint8_t *pCommandBuffer, uint16_t commandBufferSize, uint16_t& commandSize, uint8_t& commandID)
{
  /* Returns TRUE if a valid command has been received */

  _BC_SPI_TRANSACTION tr;
  uint8_t cmdID[2] = {0};
  uint16_t cmdIDSize = 0;

  initTransaction(&tr);

  tr.command = SPI_CMD_POLL_FOR_COMMAND;

  tr.rx[0].pBuffer = cmdID;
  tr.rx[0].bufferSize = sizeof(cmdID);
  tr.rx[0].pDataSize = &cmdIDSize;

  tr.rx[1].pBuffer = pCommandBuffer;
  tr.rx[1].bufferSize = commandBufferSize;
  tr.rx[1].pDataSize = &commandSize;

  if (transaction(&tr))
  {
    commandID = cmdID[1];
    return true;
  }

  commandID = 0;
  commandSize = 0;
  return false;
}

bool CBERGCloudBase::pollForCommand(CBuffer& buffer, uint8_t& commandID)
{
  /* Returns TRUE if a valid command has been received */

  _BC_SPI_TRANSACTION tr;
  uint8_t cmdID[2] = {0};
  uint16_t cmdIDSize = 0;
  uint16_t dataSize = 0;

  initTransaction(&tr);
  buffer.clear();

  tr.command = SPI_CMD_POLL_FOR_COMMAND;

  tr.rx[0].pBuffer = cmdID;
  tr.rx[0].bufferSize = sizeof(cmdID);
  tr.rx[0].pDataSize = &cmdIDSize;

  tr.rx[1].pBuffer = buffer.ptr();
  tr.rx[1].bufferSize = buffer.size();
  tr.rx[1].pDataSize = &dataSize;

  if (transaction(&tr))
  {
    commandID = cmdID[1];
    buffer.used(dataSize);
    return true;
  }

  commandID = 0;
  buffer.used(0);
  return false;
}

bool CBERGCloudBase::_sendEvent(uint8_t eventCode, uint8_t *pEventBuffer, uint16_t eventSize, uint8_t command)
{
  /* Returns TRUE if the event is sent successfully */

  _BC_SPI_TRANSACTION tr;
  uint8_t header[4] = {0};

  if (eventSize > (SPI_MAX_PAYLOAD_SIZE_BYTES - sizeof(header)))
  {
    /* Too big */
    return false;
  }

  header[0] = eventCode;

  initTransaction(&tr);

  tr.command = command;
  tr.tx[0].pBuffer = (uint8_t *)header;
  tr.tx[0].dataSize = sizeof(header);
  tr.tx[1].pBuffer = pEventBuffer;
  tr.tx[1].dataSize = (uint8_t)eventSize;

  return transaction(&tr);
}

bool CBERGCloudBase::sendEvent(uint8_t eventCode, uint8_t *pEventBuffer, uint16_t eventSize)
{
  return _sendEvent(eventCode, pEventBuffer, eventSize, SPI_CMD_SEND_EVENT_RAW);
}

bool CBERGCloudBase::sendEvent(uint8_t eventCode, CBuffer& buffer)
{
  bool result;

  result = _sendEvent(eventCode, buffer.ptr(), buffer.used(), SPI_CMD_SEND_EVENT_PACKED);

  buffer.clear();
  return result;
}

bool CBERGCloudBase::getConnectionState(uint8_t& state)
{
  _BC_SPI_TRANSACTION tr;

  initTransaction(&tr);

  tr.command = SPI_CMD_GET_CONNECT_STATE;
  tr.rx[0].pBuffer = &state;
  tr.rx[0].bufferSize = sizeof(state);

  return transaction(&tr);
}

bool CBERGCloudBase::getSignalQuality(int8_t& rssi, uint8_t& lqi)
{
  _BC_SPI_TRANSACTION tr;

  initTransaction(&tr);

  tr.command = SPI_CMD_GET_SIGNAL_QUALITY;
  tr.rx[0].pBuffer = (uint8_t *)&rssi;
  tr.rx[0].bufferSize = sizeof(rssi);
  tr.rx[1].pBuffer = &lqi;
  tr.rx[1].bufferSize = sizeof(lqi);

  return transaction(&tr);
}

bool CBERGCloudBase::connect(const uint8_t (&productID)[BC_PRODUCT_KEY_SIZE_BYTES], uint32_t version, bool waitForConnected)
{
  _BC_SPI_TRANSACTION tr;
  uint8_t version_be[sizeof(version)];

  uint8_t lastState = BC_CONNECT_STATE_DISCONNECTED;
  uint8_t state;

  initTransaction(&tr);

  version_be[0] = version >> 24;
  version_be[1] = version >> 16;
  version_be[2] = version >> 8;
  version_be[3] = version;

  tr.command = SPI_CMD_SEND_PRODUCT_ANNOUNCE;
  tr.tx[0].pBuffer = (uint8_t *)productID;
  tr.tx[0].dataSize = sizeof(productID);
  tr.tx[1].pBuffer = version_be;
  tr.tx[1].dataSize = sizeof(version_be);

  if (!transaction(&tr))
  {
    return false;
  }

  if (waitForConnected)
  {
    /* Poll until connected */
    do {
      timerReset();
      while (timerRead_mS() < CONNECT_POLL_RATE_MS);

      if (!getConnectionState(state))
      {
        return false;
      }

      if (state != lastState)
      {
        switch (state)
        {
          case BC_CONNECT_STATE_CONNECTED:
            _LOG("connect: Connected\r\n");
            break;
          case BC_CONNECT_STATE_CONNECTING:
            _LOG("connect: Connecting...\r\n");
            break;
          default:
          case BC_CONNECT_STATE_DISCONNECTED:
            _LOG("connect: Disconnected\r\n");
            break;
        }

        lastState = state;
      }

    } while (state != BC_CONNECT_STATE_CONNECTED);
  }

  return true;
}

bool CBERGCloudBase::getClaimingState(uint8_t& state)
{
  _BC_SPI_TRANSACTION tr;

  initTransaction(&tr);

  tr.command = SPI_CMD_GET_CLAIM_STATE;
  tr.rx[0].pBuffer = &state;
  tr.rx[0].bufferSize = sizeof(state);

  return transaction(&tr);
}

bool CBERGCloudBase::getClaimcode(char (&claimcode)[BC_CLAIMCODE_SIZE_BYTES])
{
  _BC_SPI_TRANSACTION tr;

  initTransaction(&tr);

  tr.command = SPI_CMD_GET_CLAIMCODE;
  tr.rx[0].pBuffer = (uint8_t *)claimcode;
  tr.rx[0].bufferSize = sizeof(claimcode);

  return transaction(&tr);
}

bool CBERGCloudBase::getEUI64(uint8_t type, uint8_t (&eui64)[BC_EUI64_SIZE_BYTES])
{
  _BC_SPI_TRANSACTION tr;

  initTransaction(&tr);

  tr.command = SPI_CMD_GET_EUI64;
  tr.tx[0].pBuffer = &type;
  tr.tx[0].dataSize = sizeof(uint8_t);
  tr.rx[0].pBuffer = eui64;
  tr.rx[0].bufferSize = sizeof(eui64);

  return transaction(&tr);
}

bool CBERGCloudBase::getDeviceAddress(uint8_t (&address)[BC_ADDRESS_SIZE_BYTES])
{
  _BC_SPI_TRANSACTION tr;

  initTransaction(&tr);

  tr.command = SPI_CMD_GET_ADDRESS;
  tr.rx[0].pBuffer = address;
  tr.rx[0].bufferSize = sizeof(address);

  return transaction(&tr);
}

bool CBERGCloudBase::setDisplayStyle(uint8_t style)
{
  _BC_SPI_TRANSACTION tr;

  initTransaction(&tr);

  tr.command = SPI_CMD_SET_DISPLAY_STYLE;
  tr.tx[0].pBuffer = &style;
  tr.tx[0].dataSize = sizeof(style);

  return transaction(&tr);
}

bool CBERGCloudBase::clearDisplay(void)
{
  return setDisplayStyle(BC_DISPLAY_CLEAR);
}

bool CBERGCloudBase::print(const char *pString)
{
  _BC_SPI_TRANSACTION tr;
  uint8_t strLen = 0;
  const char *pTmp = pString;

  if (pString == NULL)
  {
    return false;
  }

  initTransaction(&tr);

  /* Get string length excluding terminator */
  while ((*pTmp++ != '\0') && (strLen < UINT8_MAX))
  {
    strLen++;
  }

  tr.command = SPI_CMD_DISPLAY_PRINT;
  tr.tx[0].pBuffer = (uint8_t *)pString;
  tr.tx[0].dataSize = strLen;

  return transaction(&tr);
}

uint16_t CBERGCloudBase::Crc16(uint8_t data, uint16_t crc)
{
  /* From Ember's code */
  crc = (crc >> 8) | (crc << 8);
  crc ^= data;
  crc ^= (crc & 0xff) >> 4;
  crc ^= (crc << 8) << 4;

  crc ^= ( (uint8_t) ( (uint8_t) ( (uint8_t) (crc & 0xff) ) << 5)) |
    ((uint16_t) ( (uint8_t) ( (uint8_t) (crc & 0xff)) >> 3) << 8);

  return crc;
}

uint8_t CBERGCloudBase::SPITransaction(uint8_t dataOut, bool finalCS)
{
  uint8_t dataIn = 0;

  SPITransaction(&dataOut, &dataIn, (uint16_t)1, finalCS);

  return dataIn;
}

void CBERGCloudBase::begin(void)
{
  m_synced = false;
  m_lastResponse = SPI_RSP_SUCCESS;
}

void CBERGCloudBase::end(void)
{
}
