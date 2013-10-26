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


#ifndef BERGCLOUDBASE_H
#define BERGCLOUDBASE_H

#include "BERGCloudConfig.h"
#include "BERGCloudConst.h"
#include "Buffer.h"
#include "LogPrint.h"

#define BERGCLOUD_LIB_VERSION (0x0100)

#define _TX_GROUPS (2)
#define _RX_GROUPS (2)

typedef struct {
  uint8_t *pBuffer;
  uint16_t dataSize;
} _BC_TX_GROUP;

typedef struct {
  uint8_t *pBuffer;
  uint16_t bufferSize;
  uint16_t *pDataSize;
} _BC_RX_GROUP;

typedef struct {
  uint8_t command;
  _BC_TX_GROUP tx[_TX_GROUPS];
  _BC_RX_GROUP rx[_RX_GROUPS];
} _BC_SPI_TRANSACTION;

class CBERGCloudBase
{
public:
  /* Check for a command */
  bool pollForCommand(uint8_t *pCommandBuffer, uint16_t commandBufferSize, uint16_t& commandSize, uint8_t& commandID);
  bool pollForCommand(CBuffer& buffer, uint8_t& commandID);
  /* Send an event */
  bool sendEvent(uint8_t eventCode, uint8_t *pEventBuffer, uint16_t eventSize);
  bool sendEvent(uint8_t eventCode, CBuffer& buffer);
  /* Get the connection state */
  bool getConnectionState(uint8_t& state);
  /* Get the last-hop signal quality */
  bool getSignalQuality(int8_t& rssi, uint8_t& lqi);
  /* Connect */
  bool connect(const uint8_t (&productID)[BC_PRODUCT_KEY_SIZE_BYTES] = nullProductID, uint32_t version = 0, bool waitForConnected = false);
  /* Check if the device has been claimed */
  bool getClaimingState(uint8_t& state);
  /* Get the current claimcode */
  bool getClaimcode(char (&claimcode)[BC_CLAIMCODE_SIZE_BYTES]);
  /* Get the EUI64 identifier for this node, its parent or the network coordinator */
  bool getEUI64(uint8_t type, uint8_t (&eui64)[BC_EUI64_SIZE_BYTES]);
  /* Get the Device Address */
  bool getDeviceAddress(uint8_t (&address)[BC_ADDRESS_SIZE_BYTES]);
  /* Set the display style for the OLED display. This also clears the display. */
  bool setDisplayStyle(uint8_t style);
  /* Clear the OLED display */
  bool clearDisplay(void);
  /* Print a line of text on the OLED display */
  bool print(const char *pText);

  /* Internal methods */
public:
  uint8_t m_lastResponse;
  static uint8_t nullProductID[16];
protected:
  void begin(void);
  void end(void);
  uint16_t Crc16(uint8_t data, uint16_t crc);
  virtual uint16_t SPITransaction(uint8_t *pDataOut, uint8_t *pDataIn, uint16_t dataSize, bool finalCS) = 0;
  virtual void timerReset(void) = 0;
  virtual uint32_t timerRead_mS(void) = 0;
private:
  uint8_t SPITransaction(uint8_t data, bool finalCS);
  void initTransaction(_BC_SPI_TRANSACTION *pTr);
  bool transaction(_BC_SPI_TRANSACTION *tr);
  bool _sendEvent(uint8_t eventCode, uint8_t *pEventBuffer, uint16_t eventSize, uint8_t command);
  bool m_synced;
};

#endif // #ifndef BERGCLOUDBASE_H
