/*
    HelloWorld - A simple demonstration of how you use the BERG Cloud mbed
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

#define PRODUCT_VERSION 0x0001

const uint8_t PRODUCT_KEY[BC_PRODUCT_KEY_SIZE_BYTES] =  \
    {0xE4,0xBF,0xEE,0xD0,0x4E,0x25,0x43,0x9A,0x63,0x7F,0x32,0x15,0x95,0x08,0x4B,0x5E};

// Define your commands and events here, according to the schema from bergcloud.com

#define EXAMPLE_EVENT_ID 0x01

void loop(void);
uint32_t counter;

int main(void) {
  BERGCloud.begin(MOSI_PIN, MISO_PIN, SCLK_PIN, nSSEL_PIN);
  printf("--- reset ---\r\n");
  counter = 0;

  if (BERGCloud.connect(PRODUCT_KEY, PRODUCT_VERSION)) {
    printf("Connected to network\r\n");
  } else {
    printf("connect() returned false.\r\n");
  }
  
  // Simple loop to emulate Arduino
  while(1) {
    loop();
  }
}

void loop(void)
{
  uint8_t commandID;
  BERGCloudMessage command, event;
  uint16_t argCount;
  string text;
  uint16_t count;
  char buffer[20];
  
  printf("Poll for command... ");
  if (BERGCloud.pollForCommand(command, commandID)) {
    printf("Got command 0x");
    printf("%02X\r\n", commandID);

    // Print out the types of encoded data in this command
    command.print();
    
    printf("\r\nRAW BYTES:\r\n");
    command.print_bytes();
    
    
    switch (commandID) {
      case 0x01: {
        command.unpack_array(argCount);
        
        printf("Got %d items\r\n", argCount);
        
        uint8_t peekvar;
        command.peek(&peekvar);
        printf("PEEK HEX: %02x\r\n", peekvar);
        
        if (command.unpack(text)) {
          printf(" Text = %s\r\n", text.c_str());
          BERGCloud.display(text);
        } else {
          printf(" Failed to decode text\r\n");
        }
        
        if (command.unpack(count)) {
          printf(" Got count %d", count);
          sprintf(buffer, "%04d", count);
          string data = buffer;
          BERGCloud.display(data);
        }
      }
    }
  } else {
    printf("none.\r\n");
  }

  // Pause to make the debug output easier to read
  wait_ms(1000);

  //////////////////////////////////////////////////
  // Now we start to send our event information.
  
  printf("Sending an event... ");
  
  // Send our test event. It contains a string and the counter value
  event.pack("BERG");
  event.pack(counter);

  // Increment each loop iteration
  counter++;

  // Send the event to the device web service
  if (BERGCloud.sendEvent(EXAMPLE_EVENT_ID, event)) {
    printf("ok\r\n");
  } else {
    printf("failed/busy\r\n");
  }
}

