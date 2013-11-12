/*
    SimpleCounter - A simple demonstration of how you use the BERG Cloud mbed
                    client libraries to fetch and send data to BERG Cloud. For
                    more info see http://bergcloud.com/

    This example code is in the public domain.

    https://github.com/bergcloud/devshield-mbed
*/

#include "mbed.h"
#include <BERGCloudMbed.h>

BERGCloudMbed BERGCloud;

// Select the pins to be used for the SPI interface

#if defined(TARGET_LPC176X)
// mbed DIP40 pinout
#define MOSI_PIN  p5
#define MISO_PIN  p6
#define SCLK_PIN  p7
#define nSSEL_PIN p8
#endif

#if defined(TARGET_KL25Z)
// Arduino compatible pinout
#define MOSI_PIN  D11
#define MISO_PIN  D12
#define SCLK_PIN  D13
#define nSSEL_PIN D10
#endif

#if !defined(MOSI_PIN) || !defined(MISO_PIN) || !defined(SCLK_PIN) || !defined(nSSEL_PIN)
#error Please add the pin names for your microcontroller.
#endif

// These values should be edited to reflect your Product setup on bergcloud.com

#define PRODUCT_VERSION 0x00000001

const uint8_t PRODUCT_KEY[BC_PRODUCT_KEY_SIZE_BYTES] =  \
    { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };

// Define your commands and events here, according to the schema from bergcloud.com

#define EXAMPLE_EVENT_ID 0x01

void loop(void);
uint32_t counter;

int main(void)
{
  BERGCloud.begin(MOSI_PIN, MISO_PIN, SCLK_PIN, nSSEL_PIN);
  printf("--- reset ---\r\n");
  counter = 0;

  if (BERGCloud.connect(PRODUCT_KEY, PRODUCT_VERSION))
  {
    printf("Connected to network\r\n");
  }
  else
  {
    printf("connect() returned false.\r\n");
  }
  
  while(1)
  {
    loop();
  }
}

void loop(void)
{
  uint8_t a;
  uint8_t eui64[BC_EUI64_SIZE_BYTES];
  uint8_t address[BC_ADDRESS_SIZE_BYTES];
  char claimcode[BC_CLAIMCODE_SIZE_BYTES];
  uint32_t i;
  uint8_t commandID;
  int8_t rssi;
  uint8_t lqi;
  BERGCloudMessage command, event;
  string text;
  unsigned int value;
  
  wait_ms(1000);

  printf("Poll for command... ");
  if (BERGCloud.pollForCommand(command, commandID))
  {
  
    printf("Got command 0x");
    printf("%02X", commandID);
    printf(" containing:\r\n");

    command.print();

    if (command.unpack(value))
    {
      printf("got value = ");
      printf("%i\r\n", value);
    }
    else
    {
      printf("unpack(int) returned false.\r\n");
    }

    if (command.unpack(text))
    {
      printf("got text = ");
      printf("%s\r\n", text.c_str());
    }
    else
    {
      printf("unpack(text) returned false.\r\n");
    }
  }
  else
  {
    printf("none.\r\n");
  }

  wait_ms(1000);

  printf("Sending an event... ");
  
  event.pack("BERG");
  event.pack(counter);

  counter++;

  if (BERGCloud.sendEvent(EXAMPLE_EVENT_ID, event))
  {
    printf("ok\r\n");
  }
  else
  {
    printf("failed/busy\r\n");
  }

  wait_ms(1000);

  // The following method calls are examples of how you query the
  // network and BERG Cloud shield state from within your mbed code
  
  if (BERGCloud.getConnectionState(a))
  {
    switch(a)
    {
    case BC_CONNECT_STATE_CONNECTED:
      printf("Connection state: Connected\r\n");
      break;
    case BC_CONNECT_STATE_CONNECTING:
      printf("Connection state: Connecting...\r\n");
      break;
    case BC_CONNECT_STATE_DISCONNECTED:
      printf("Connection state: Disconnected\r\n");
      break;
    default:
      printf("Connection state: Unknown!\r\n");
      break;
    }
  }
  else
  {
    printf("getConnectionState() returned false.\r\n");
  }

  if (BERGCloud.getClaimingState(a))
  {
    switch(a)
    {
    case BC_CLAIM_STATE_CLAIMED:
      printf("Claim State: Claimed\r\n");
      break;
    case BC_CLAIM_STATE_NOT_CLAIMED:
      printf("Claim State: Not Claimed\r\n");
      break;
    default:
      printf("Claim State: Unknown!\r\n");
      break;
    }
  }
  else
  {
    printf("getClaimingState() returned false.\r\n");
  }

  if (BERGCloud.getClaimcode(claimcode))
  {
    printf("Claim code: ");
    printf("%s\r\n", claimcode);
  }
  else
  {
    printf("getClaimcode() returned false.\r\n");
  }

  if (BERGCloud.getEUI64(BC_EUI64_NODE, eui64))
  {
    printf("Node EUI64: 0x");
    for (i=0; i < sizeof(eui64); i++)
    {
      if (eui64[7-i] < 0x10)
      {
        printf("0");
      }
      printf("%02X", eui64[7-i]);
    }
    printf("\r\n");
  }
  else
  {
    printf("getEUI64(BC_EUI64_NODE) returned false.\r\n");
  }

  if (BERGCloud.getEUI64(BC_EUI64_PARENT, eui64))
  {
    printf("Parent EUI64: 0x");
    for (i=0; i < sizeof(eui64); i++)
    {
      if (eui64[7-i] < 0x10)
      {
        printf("0");
      }
      printf("%02X", eui64[7-i]);
    }
    printf("\r\n");
  }
  else
  {
    printf("getEUI64(BC_EUI64_PARENT) returned false.\r\n");
  }

  if (BERGCloud.getEUI64(BC_EUI64_COORDINATOR, eui64))
  {
    printf("Coordinator EUI64: 0x");
    for (i=0; i < sizeof(eui64); i++)
    {
      if (eui64[7-i] < 0x10)
      {
        printf("0");
      }
      printf("%02X", eui64[7-i]);
    }
    printf("\r\n");
  }
  else
  {
    printf("getEUI64(BC_EUI64_COORDINATOR) returned false.\r\n");
  }

  if (BERGCloud.getSignalQuality(rssi, lqi))
  {
    printf("Last-hop signal quality: RSSI ");
    printf("%i", rssi);
    printf(" dBm, LQI ");
    printf("%i", lqi);
    printf("/255\r\n");
  }
  else
  {
    printf("getSignalQuality returned false.\r\n");
  }

  if (BERGCloud.getDeviceAddress(address))
  {
    printf("Device Address: 0x");
    for (i=0; i < sizeof(address); i++)
    {
      if (address[7-i] < 0x10)
      {
        printf("0");
      }
      printf("%02x", address[7-i]);
    }
    printf("\r\n");
  }
  else
  {
    printf("getDeviceAddress() returned false.\r\n");
  }
}
