/*

BERGCloud library common API

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
#include <stddef.h>
#include <string.h> /* For memset() */

#include "BERGCloudBase.h"

#define SPI_POLL_TIMEOUT_MS 1000
#define SPI_SYNC_TIMEOUT_MS 10000

#define CONNECT_POLL_RATE_MS 250

/* MessagePack for named commands and events */
#define _MP_FIXRAW_MIN      0xa0
#define _MP_FIXRAW_MAX      0xbf
#define _MAX_FIXRAW         (_MP_FIXRAW_MAX - _MP_FIXRAW_MIN)

uint8_t BERGCloudBase::nullKey[BC_KEY_SIZE_BYTES] = {0};

bool BERGCloudBase::_transaction(_BC_SPI_TRANSACTION *tr)
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
  if (!synced)
  {
    timerReset();

    do {
      rxByte = SPITransaction(SPI_PROTOCOL_PAD, true);
      timeout = timerRead_mS() > SPI_SYNC_TIMEOUT_MS;

    } while ((rxByte != SPI_PROTOCOL_RESET) && !timeout);

    if (timeout)
    {
      _LOG("Timeout, sync (BERGCloudBase::transaction)\r\n");
      return false;
    }

    /* Resynchronisation successful */
    synced = true;
  }

  /* Calculate total data size */
  dataSize = 0;

  for (i=0; i<_TX_GROUPS; i++)
  {
    dataSize += tr->tx[i].dataSize;
  }

  /* Initialise CRC */
  calcCRC = 0xffff;

  /* Create header */
  header[0] = tr->command;
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
      _LOG("Reset, send header (BERGCloudBase::transaction)\r\n");
      return false;
    }

    if (rxByte != SPI_PROTOCOL_PAD)
    {
      _LOG("SyncErr, send header (BERGCloudBase::transaction)\r\n");
      synced = false;
      return false;
    }
  }

  /* Send data groups */
  for (i=0; i<_TX_GROUPS; i++)
  {
    for (j=0; j<tr->tx[i].dataSize; j++)
    {
      calcCRC = Crc16(tr->tx[i].buffer[j], calcCRC);
      rxByte = SPITransaction(tr->tx[i].buffer[j], false);

      if (rxByte == SPI_PROTOCOL_RESET)
      {
        _LOG("Reset, send data (BERGCloudBase::transaction)\r\n");
        return false;
      }

      if (rxByte != SPI_PROTOCOL_PAD)
      {
        _LOG("SyncErr, send data (BERGCloudBase::transaction)\r\n");
        synced = false;
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
      _LOG("Reset, send footer (BERGCloudBase::transaction)\r\n");
      return false;
    }

    if (rxByte != SPI_PROTOCOL_PAD)
    {
      _LOG("SyncErr, send footer (BERGCloudBase::transaction)\r\n");
      synced = false;
      return false;
    }
  }

  /* Poll for response */
  timerReset();

  do {
    rxByte = SPITransaction(SPI_PROTOCOL_PAD, false);

    if (rxByte == SPI_PROTOCOL_RESET)
    {
      _LOG("Reset, poll (BERGCloudBase::transaction)\r\n");
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
    _LOG("Timeout, poll (BERGCloudBase::transaction)\r\n");
    synced = false;
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
    groupSize = tr->rx[i].bufferSize;
    j = 0; /* Start of the group buffer */

    while((dataSize > 0) && (groupSize > 0))
    {
      tr->rx[i].buffer[j] = SPITransaction(SPI_PROTOCOL_PAD, false);
      calcCRC = Crc16(tr->rx[i].buffer[j], calcCRC);

      /* Next */
      j++;

      /* Update the total data remaining and the space remaining in this group. */
      dataSize--;
      groupSize--;
    }

    if (tr->rx[i].dataSize != NULL)
    {
      /* Return the number of bytes used in this buffer */
      *tr->rx[i].dataSize = tr->rx[i].bufferSize - groupSize;
    }
  }

  if (dataSize > 0)
  {
    /* Too much data sent */
    _LOG("SizeErr, read data (BERGCloudBase::transaction)\r\n");
    synced = false;
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
    _LOG("CRCErr, read data (BERGCloudBase::transaction)\r\n");
    synced = false;
    return false;
  }

  /* Get reponse code */
  lastResponse = header[0];

  return (lastResponse == SPI_RSP_SUCCESS);
}

bool BERGCloudBase::transaction(_BC_SPI_TRANSACTION *tr)
{
  bool result;

  /* For thread synchronisation */
  lockTake();
  result = _transaction(tr);
  lockRelease();

  return result;
}

void BERGCloudBase::initTransaction(_BC_SPI_TRANSACTION *tr)
{
  memset(tr, 0x00, sizeof(_BC_SPI_TRANSACTION));
}

bool BERGCloudBase::pollForCommand(uint8_t *commandBuffer, uint16_t commandBufferSize, uint16_t& commandSize, uint8_t& commandID)
{
  /* Returns TRUE if a valid command has been received */

  _LOG("pollForCommand() methods returning a command ID number have been deprecated.\r\n");

  _BC_SPI_TRANSACTION tr;
  uint8_t cmdID[2] = {0};
  uint16_t cmdIDSize = 0;

  initTransaction(&tr);

  tr.command = SPI_CMD_POLL_FOR_COMMAND;

  tr.rx[0].buffer = cmdID;
  tr.rx[0].bufferSize = sizeof(cmdID);
  tr.rx[0].dataSize = &cmdIDSize;

  tr.rx[1].buffer = commandBuffer;
  tr.rx[1].bufferSize = commandBufferSize;
  tr.rx[1].dataSize = &commandSize;

  if (transaction(&tr))
  {
    commandID = cmdID[1];
    return true;
  }

  commandID = 0;
  commandSize = 0;
  return false;
}

bool BERGCloudBase::pollForCommand(uint8_t *commandBuffer, uint16_t commandBufferSize, uint16_t& commandSize, char *commandName, uint8_t commandNameMaxSize)
{
  /* Returns TRUE if a valid command has been received */

  _BC_SPI_TRANSACTION tr;
  uint8_t cmdID[2] = {0};
  uint16_t cmdIDSize = 0;
  uint8_t commandNameSize;
  uint8_t originalCommandNameSize;
  uint8_t msgPackByte;
  uint16_t command;

  if ((commandName == NULL) || (commandNameMaxSize < 2))
  {
    return false;
  }

  initTransaction(&tr);

  tr.command = SPI_CMD_POLL_FOR_COMMAND;

  tr.rx[0].buffer = cmdID;
  tr.rx[0].bufferSize = sizeof(cmdID);
  tr.rx[0].dataSize = &cmdIDSize;

  tr.rx[1].buffer = commandBuffer;
  tr.rx[1].bufferSize = commandBufferSize;
  tr.rx[1].dataSize = &commandSize;

  if (transaction(&tr))
  {
    command = (cmdID[0] << 8) | cmdID[1];
    if (command == BC_COMMAND_NAMED_PACKED)
    {
      /* Get command name string size */
      msgPackByte = *commandBuffer;

      if ((msgPackByte <_MP_FIXRAW_MIN) || (msgPackByte > _MP_FIXRAW_MAX))
      {
        /* Invalid */
        return false;
      }

      commandNameSize = originalCommandNameSize = msgPackByte - _MP_FIXRAW_MIN;

      /* Limit to the size of the buffer provided */
      if (commandNameSize > (commandNameMaxSize-1)) /* -1 for null terminator */
      {
        commandNameSize = (commandNameMaxSize-1);
      }

      /* Copy command name string as a null-terminated C string */
      bytecpy((uint8_t *)commandName, (commandBuffer+1), commandNameSize); /* +1 for messagePack fixraw byte */
      commandName[commandNameSize] = '\0';

      /* Move up remaining packed data, update size */
      commandSize -= (originalCommandNameSize + 1); /* +1 for messagePack fixraw byte */
      bytecpy(commandBuffer, commandBuffer + (originalCommandNameSize + 1), commandSize);
      return true;
    }
  }

  *commandName = '\0';
  commandSize = 0;
  return false;
}

#ifdef BERGCLOUD_PACK_UNPACK
bool BERGCloudBase::pollForCommand(BERGCloudMessageBuffer& buffer, uint8_t& commandID)
{
  /* Returns TRUE if a valid command has been received */

  _LOG("pollForCommand() methods returning a command ID number have been deprecated.\r\n");

  _BC_SPI_TRANSACTION tr;
  uint8_t cmdID[2] = {0};
  uint16_t cmdIDSize = 0;
  uint16_t dataSize = 0;

  initTransaction(&tr);
  buffer.clear();

  tr.command = SPI_CMD_POLL_FOR_COMMAND;

  tr.rx[0].buffer = cmdID;
  tr.rx[0].bufferSize = sizeof(cmdID);
  tr.rx[0].dataSize = &cmdIDSize;

  tr.rx[1].buffer = buffer.ptr();
  tr.rx[1].bufferSize = buffer.size();
  tr.rx[1].dataSize = &dataSize;

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

bool BERGCloudBase::pollForCommand(BERGCloudMessageBuffer& buffer, char *commandName, uint8_t commandNameMaxSize)
{
  /* Returns TRUE if a valid command has been received */

  _BC_SPI_TRANSACTION tr;
  uint8_t cmdID[2] = {0};
  uint16_t cmdIDSize = 0;
  uint16_t dataSize = 0;
  uint8_t commandNameSize;
  uint8_t originalCommandNameSize;
  uint8_t msgPackByte;
  uint16_t command;

  if ((commandName == NULL) || (commandNameMaxSize < 2))
  {
    return false;
  }

  initTransaction(&tr);
  buffer.clear();

  tr.command = SPI_CMD_POLL_FOR_COMMAND;

  tr.rx[0].buffer = cmdID;
  tr.rx[0].bufferSize = sizeof(cmdID);
  tr.rx[0].dataSize = &cmdIDSize;

  tr.rx[1].buffer = buffer.ptr();
  tr.rx[1].bufferSize = buffer.size();
  tr.rx[1].dataSize = &dataSize;

  if (transaction(&tr))
  {
    command = (cmdID[0] << 8) | cmdID[1];
    if (command == BC_COMMAND_NAMED_PACKED)
    {
      /* Get command name string size */
      msgPackByte = *buffer.ptr();

      if ((msgPackByte <_MP_FIXRAW_MIN) || (msgPackByte > _MP_FIXRAW_MAX))
      {
        /* Invalid */
        return false;
      }

      commandNameSize = originalCommandNameSize = msgPackByte - _MP_FIXRAW_MIN;

      /* Limit to the size of the buffer provided */
      if (commandNameSize > (commandNameMaxSize-1)) /* -1 for null terminator */
      {
        commandNameSize = (commandNameMaxSize-1);
      }

      /* Copy command name string as a null-terminated C string */
      bytecpy((uint8_t *)commandName, (buffer.ptr()+1), commandNameSize); /* +1 for messagePack fixraw byte */
      commandName[commandNameSize] = '\0';

      /* Move up remaining packed data, update size */
      dataSize -= (originalCommandNameSize + 1); /* +1 for messagePack fixraw byte */
      bytecpy(buffer.ptr(), buffer.ptr() + (originalCommandNameSize + 1), dataSize);

      buffer.used(dataSize);
      return true;
    }
  }

  buffer.used(0);
  *commandName = '\0';
  return false;
}
#endif

bool BERGCloudBase::_sendEvent(uint8_t eventCode, uint8_t *eventBuffer, uint16_t eventSize, uint8_t command)
{
  /* Returns TRUE if the event is sent successfully */

  _LOG("sendEvent() methods using an eventCode number have been deprecated.\r\n");

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
  tr.tx[0].buffer = (uint8_t *)header;
  tr.tx[0].dataSize = sizeof(header);
  tr.tx[1].buffer = eventBuffer;
  tr.tx[1].dataSize = (uint8_t)eventSize;

  return transaction(&tr);
}

bool BERGCloudBase::sendEvent(uint8_t eventCode, uint8_t *eventBuffer, uint16_t eventSize, bool packed)
{

  return _sendEvent(eventCode, eventBuffer, eventSize, packed ? SPI_CMD_SEND_EVENT_PACKED : SPI_CMD_SEND_EVENT_RAW);
}

bool BERGCloudBase::sendEvent(const char *eventName, uint8_t *eventBuffer, uint16_t eventSize, bool packed)
{
  /* Returns TRUE if the event is sent successfully */

  _BC_SPI_TRANSACTION tr;
  uint8_t headerSize = 5; /* Four bytes of SPI header, one byte of messagePack type */
  uint8_t header[5 + _MAX_FIXRAW] = {0}; /* Header size plus maximum name string size */

  if (!packed)
  {
    /* We only support packed data now */
    return false;
  }

  if ((eventName == NULL) || (eventName[0] == '\0'))
  {
    _LOG("Event name must be at least one character.\r\n");
    return false;
  }

  /* Create SPI header */
  header[0] = BC_EVENT_NAMED_PACKED & BC_EVENT_ID_MASK;
  header[1] = 0;
  header[2] = 0;
  header[3] = 0;

  /* Create string header in messagePack format */
  header[4] = _MP_FIXRAW_MIN;
  while ((*eventName != '\0') && (headerSize < sizeof(header)))
  {
    /* Copy string, update messagePack byte */
    header[4]++;
    header[headerSize++] = *eventName++;
  }

  if (eventSize > ((uint16_t)SPI_MAX_PAYLOAD_SIZE_BYTES - headerSize))
  {
    _LOG("Event is too big.\r\n");
    return false;
  }

  initTransaction(&tr);

  tr.command = SPI_CMD_SEND_EVENT_PACKED;
  tr.tx[0].buffer = (uint8_t *)header;
  tr.tx[0].dataSize = headerSize;
  tr.tx[1].buffer = eventBuffer;
  tr.tx[1].dataSize = eventSize;

  return transaction(&tr);
}

#ifdef BERGCLOUD_PACK_UNPACK
bool BERGCloudBase::sendEvent(uint8_t eventCode, BERGCloudMessageBuffer& buffer)
{
  bool result;

  result = _sendEvent(eventCode, buffer.ptr(), buffer.used(), SPI_CMD_SEND_EVENT_PACKED);

  buffer.clear();
  return result;
}

bool BERGCloudBase::sendEvent(const char *eventName, BERGCloudMessageBuffer& buffer)
{
  /* Returns TRUE if the event is sent successfully */

  _BC_SPI_TRANSACTION tr;
  uint8_t headerSize = 5; /* Four bytes of SPI header, one byte of messagePack type */
  uint8_t header[5 + _MAX_FIXRAW] = {0}; /* Header size plus maximum name string size */

  if ((eventName == NULL) || (eventName[0] == '\0'))
  {
    _LOG("Event name must be at least one character.\r\n");
    return false;
  }

  /* Create SPI header */
  header[0] = BC_EVENT_NAMED_PACKED & BC_EVENT_ID_MASK;
  header[1] = 0;
  header[2] = 0;
  header[3] = 0;

  /* Create string header in messagePack format */
  header[4] = _MP_FIXRAW_MIN;
  while ((*eventName != '\0') && (headerSize < sizeof(header)))
  {
    /* Copy string, update messagePack byte */
    header[4]++;
    header[headerSize++] = *eventName++;
  }

  if (buffer.used() > ((uint16_t)SPI_MAX_PAYLOAD_SIZE_BYTES - headerSize))
  {
    _LOG("Event is too big.\r\n");
    return false;
  }

  initTransaction(&tr);

  tr.command = SPI_CMD_SEND_EVENT_PACKED;
  tr.tx[0].buffer = (uint8_t *)header;
  tr.tx[0].dataSize = headerSize;
  tr.tx[1].buffer = buffer.ptr();
  tr.tx[1].dataSize = buffer.used();

  return transaction(&tr);
}
#endif

bool BERGCloudBase::getConnectionState(uint8_t& state)
{
  _BC_SPI_TRANSACTION tr;

  initTransaction(&tr);

  tr.command = SPI_CMD_GET_CONNECT_STATE;
  tr.rx[0].buffer = &state;
  tr.rx[0].bufferSize = sizeof(state);

  return transaction(&tr);
}

bool BERGCloudBase::getSignalQuality(int8_t& rssi, uint8_t& lqi)
{
  _BC_SPI_TRANSACTION tr;

  initTransaction(&tr);

  tr.command = SPI_CMD_GET_SIGNAL_QUALITY;
  tr.rx[0].buffer = (uint8_t *)&rssi;
  tr.rx[0].bufferSize = sizeof(rssi);
  tr.rx[1].buffer = &lqi;
  tr.rx[1].bufferSize = sizeof(lqi);

  return transaction(&tr);
}

bool BERGCloudBase::connect(const uint8_t (&key)[BC_KEY_SIZE_BYTES], uint16_t version, bool waitForConnected)
{
  _BC_SPI_TRANSACTION tr;
  uint16_t hostType = BC_HOST_UNKNOWN;
  uint8_t connectData [sizeof(version) + sizeof(hostType)];

  uint8_t lastState = BC_CONNECT_STATE_DISCONNECTED;
  uint8_t state;

#ifndef BERGCLOUD_NO_HOST_TYPE
  /* Get host type */
  hostType = getHostType();
#endif

  initTransaction(&tr);

  connectData[0] = hostType;
  connectData[1] = hostType >> 8;
  connectData[2] = version;
  connectData[3] = version >> 8;

  tr.command = SPI_CMD_SEND_ANNOUNCE;
  tr.tx[0].buffer = (uint8_t *)key;
  tr.tx[0].dataSize = sizeof(key);
  tr.tx[1].buffer = connectData;
  tr.tx[1].dataSize = sizeof(connectData);

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

bool BERGCloudBase::connect(const char *key, uint16_t version, bool waitForConnected)
{
  unsigned int tmp_key[BC_KEY_SIZE_BYTES] = {0};
  uint8_t _key[BC_KEY_SIZE_BYTES] = {0};
  uint8_t i;

  /* Convert key from ASCII */
  if (key != NULL)
  {
    if (sscanf(key, "%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x",
      &tmp_key[0],  &tmp_key[1],  &tmp_key[2],  &tmp_key[3],
      &tmp_key[4],  &tmp_key[5],  &tmp_key[6],  &tmp_key[7],
      &tmp_key[8],  &tmp_key[9],  &tmp_key[10], &tmp_key[11],
      &tmp_key[12], &tmp_key[13], &tmp_key[14], &tmp_key[15]) == 16)
    {
      for (i=0; i<sizeof(_key); i++)
      {
        _key[i] = (uint8_t)tmp_key[i];
      }
    }
  }

  return connect(_key, version, waitForConnected);
}

bool BERGCloudBase::getClaimingState(uint8_t& state)
{
  _BC_SPI_TRANSACTION tr;

  initTransaction(&tr);

  tr.command = SPI_CMD_GET_CLAIM_STATE;
  tr.rx[0].buffer = &state;
  tr.rx[0].bufferSize = sizeof(state);

  return transaction(&tr);
}

bool BERGCloudBase::getClaimcode(const char (&claimcode)[BC_CLAIMCODE_SIZE_BYTES])
{
  _BC_SPI_TRANSACTION tr;

  initTransaction(&tr);

  tr.command = SPI_CMD_GET_CLAIMCODE;
  tr.rx[0].buffer = (uint8_t *)claimcode;
  tr.rx[0].bufferSize = sizeof(claimcode);

  return transaction(&tr);
}

bool BERGCloudBase::getEUI64(uint8_t type, uint8_t (&eui64)[BC_EUI64_SIZE_BYTES])
{
  _BC_SPI_TRANSACTION tr;

  initTransaction(&tr);

  tr.command = SPI_CMD_GET_EUI64;
  tr.tx[0].buffer = &type;
  tr.tx[0].dataSize = sizeof(uint8_t);
  tr.rx[0].buffer = eui64;
  tr.rx[0].bufferSize = sizeof(eui64);

  return transaction(&tr);
}

bool BERGCloudBase::getDeviceAddress(uint8_t (&address)[BC_ADDRESS_SIZE_BYTES])
{
  _BC_SPI_TRANSACTION tr;

  initTransaction(&tr);

  tr.command = SPI_CMD_GET_ADDRESS;
  tr.rx[0].buffer = address;
  tr.rx[0].bufferSize = sizeof(address);

  return transaction(&tr);
}

bool BERGCloudBase::setDisplayStyle(uint8_t style)
{
  _BC_SPI_TRANSACTION tr;

  initTransaction(&tr);

  tr.command = SPI_CMD_SET_DISPLAY_STYLE;
  tr.tx[0].buffer = &style;
  tr.tx[0].dataSize = sizeof(style);

  return transaction(&tr);
}

bool BERGCloudBase::clearDisplay(void)
{
  return setDisplayStyle(BC_DISPLAY_CLEAR);
}

bool BERGCloudBase::display(const char *text)
{
  _BC_SPI_TRANSACTION tr;
  uint8_t strLen = 0;
  const char *tmp = text;

  if (text == NULL)
  {
    return false;
  }

  initTransaction(&tr);

  /* Get string length excluding terminator */
  while ((*tmp++ != '\0') && (strLen < UINT8_MAX))
  {
    strLen++;
  }

  tr.command = SPI_CMD_DISPLAY_PRINT;
  tr.tx[0].buffer = (uint8_t *)text;
  tr.tx[0].dataSize = strLen;

  return transaction(&tr);
}

uint16_t BERGCloudBase::Crc16(uint8_t data, uint16_t crc)
{
  /* CRC16 CCITT (0x1021) */

  uint8_t s;
  uint16_t t;

  s = data ^ (crc >> 8);
  t = s ^ (s >> 4);
  return (crc << 8) ^ t ^ (t << 5) ^ (t << 12);
}

uint8_t BERGCloudBase::SPITransaction(uint8_t dataOut, bool finalCS)
{
  uint8_t dataIn = 0;

  SPITransaction(&dataOut, &dataIn, (uint16_t)1, finalCS);

  return dataIn;
}

void BERGCloudBase::lockTake(void)
{
}

void BERGCloudBase::lockRelease(void)
{
}

void BERGCloudBase::begin(void)
{
  synced = false;
  lastResponse = SPI_RSP_SUCCESS;

  /* Print library version */
  _LOG("\r\nBERGCloud library version ");
  _LOG_HEX(BERGCLOUD_LIB_VERSION >> 8);
  _LOG(".");
  _LOG_HEX(BERGCLOUD_LIB_VERSION & 0xff);
  _LOG("\r\n");
}

void BERGCloudBase::end(void)
{
}

void BERGCloudBase::bytecpy(uint8_t *dst, uint8_t *src, uint16_t size)
{
  /* memcpy() cannot be used when buffers overlap */
  while (size-- > 0)
  {
    *dst++ = *src++;
  }
}
