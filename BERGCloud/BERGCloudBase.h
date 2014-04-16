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


#ifndef BERGCLOUDBASE_H
#define BERGCLOUDBASE_H

#include "BERGCloudConfig.h"
#include "BERGCloudConst.h"
#include "BERGCloudLogPrint.h"

#ifdef BERGCLOUD_PACK_UNPACK
#include "BERGCloudMessageBuffer.h"
#endif

#define BERGCLOUD_LIB_VERSION (0x0200)

#define _TX_GROUPS (2)
#define _RX_GROUPS (2)

typedef struct {
  uint8_t *buffer;
  uint16_t dataSize;
} _BC_TX_GROUP;

typedef struct {
  uint8_t *buffer;
  uint16_t bufferSize;
  uint16_t *dataSize;
} _BC_RX_GROUP;

typedef struct {
  uint8_t command;
  _BC_TX_GROUP tx[_TX_GROUPS];
  _BC_RX_GROUP rx[_RX_GROUPS];
} _BC_SPI_TRANSACTION;

class BERGCloudBase
{
public:
  /* Check for a command */
  bool pollForCommand(uint8_t *commandBuffer, uint16_t commandBufferSize, uint16_t& commandSize, uint8_t& commandID);
  bool pollForCommand(uint8_t *commandBuffer, uint16_t commandBufferSize, uint16_t& commandSize, char *commandName, uint8_t commandNameMaxSize);
#ifdef BERGCLOUD_PACK_UNPACK
  bool pollForCommand(BERGCloudMessageBuffer& buffer, uint8_t& commandID);
  bool pollForCommand(BERGCloudMessageBuffer& buffer, char *commandName, uint8_t commandNameMaxSize);
#endif
  /* Send an event */
  bool sendEvent(uint8_t eventCode, uint8_t *eventBuffer, uint16_t eventSize, bool packed = true);
  bool sendEvent(const char *eventName, uint8_t *eventBuffer, uint16_t eventSize, bool packed = true);
#ifdef BERGCLOUD_PACK_UNPACK
  bool sendEvent(uint8_t eventCode, BERGCloudMessageBuffer& buffer);
  bool sendEvent(const char *eventName, BERGCloudMessageBuffer& buffer);
#endif
  /* Get the connection state */
  bool getConnectionState(uint8_t& state);
  /* Get the last-hop signal quality */
  bool getSignalQuality(int8_t& rssi, uint8_t& lqi);
  /* Connect */
  virtual bool connect(const uint8_t (&key)[BC_KEY_SIZE_BYTES] = nullKey, uint16_t version = 0, bool waitForConnected = false);
  virtual bool connect(const char *key = NULL, uint16_t version = 0, bool waitForConnected = false);
  /* Check if the device has been claimed */
  bool getClaimingState(uint8_t& state);
  /* Get the current claimcode */
  virtual bool getClaimcode(const char (&claimcode)[BC_CLAIMCODE_SIZE_BYTES]);
  /* Get the EUI64 identifier for this node, its parent or the network coordinator */
  virtual bool getEUI64(uint8_t type, uint8_t (&eui64)[BC_EUI64_SIZE_BYTES]);
  /* Get the Device Address */
  virtual bool getDeviceAddress(uint8_t (&address)[BC_ADDRESS_SIZE_BYTES]);
  /* Set the display style for the OLED display. This also clears the display. */
  bool setDisplayStyle(uint8_t style);
  /* Clear the OLED display */
  bool clearDisplay(void);
  /* Display a line of text on the OLED display */
  bool display(const char *text);

  /* Internal methods */
public:
  uint8_t lastResponse;
  static uint8_t nullKey[BC_KEY_SIZE_BYTES];
protected:
  void begin(void);
  void end(void);
  uint16_t Crc16(uint8_t data, uint16_t crc);
  virtual uint16_t SPITransaction(uint8_t *dataOut, uint8_t *dataIn, uint16_t dataSize, bool finalCS) = 0;
  virtual void timerReset(void) = 0;
  virtual uint32_t timerRead_mS(void) = 0;
  virtual uint16_t getHostType(void) = 0;
private:
  uint8_t SPITransaction(uint8_t data, bool finalCS);
  void initTransaction(_BC_SPI_TRANSACTION *tr);
  bool _transaction(_BC_SPI_TRANSACTION *tr);
  bool transaction(_BC_SPI_TRANSACTION *tr);
  bool _sendEvent(uint8_t eventCode, uint8_t *eventBuffer, uint16_t eventSize, uint8_t command);
  void bytecpy(uint8_t *dst, uint8_t *src, uint16_t size);
  void lockTake(void);
  void lockRelease(void);
  bool synced;
};

#endif // #ifndef BERGCLOUDBASE_H
