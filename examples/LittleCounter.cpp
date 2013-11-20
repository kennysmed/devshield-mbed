/*
    LittleCounter - A simple demonstration of how you use the BERG Cloud mbed
                    client libraries to fetch and send data to BERG Cloud. For
                    more info see http://bergcloud.com/

    This example code is in the public domain.

    https://github.com/bergcloud/devshield-mbed
*/

#include "mbed.h"
#include "BERGCloudMbed.h"
#include <sstream> // std::stringstream

// Select the pins to be used for the SPI interface
#if defined(TARGET_LPC176X) || defined(TARGET_LPC11UXX)
// mbed DIP40 pinout
#define MOSI_PIN p5
#define MISO_PIN p6
#define SCLK_PIN p7
#define nSSEL_PIN p8
#endif

#if defined(TARGET_KL25Z)
// Arduino compatible pinout
#define MOSI_PIN D11
#define MISO_PIN D12
#define SCLK_PIN D13
#define nSSEL_PIN D10
#endif

#if !defined(MOSI_PIN) || !defined(MISO_PIN) || !defined(SCLK_PIN) || !defined(nSSEL_PIN)
#error Please add the pin names for your microcontroller.
#endif

// The Project Key ties this code into a Project on developer.bergcloud.com
const uint8_t PROJECT_KEY[BC_KEY_SIZE_BYTES] = \
    {0x8B,0x05,0xF7,0x25,0x10,0x54,0x0A,0xE4,0x7C,0x35,0xEE,0xE7,0x26,0xDC,0xD5,0xA8};

// The version of your code
#define VERSION 0x0001

// Define your commands and events here, according to the schema from bergcloud.com
#define COMMAND_SET_COUNTER   0x01
#define COMMAND_GREET         0x02
#define EVENT_COUNTER_CHANGED 0x01

// Forward declarations
void loop(void);

// These two methods correspond to the two commands defined within the Little Counter
// project on the BERG Cloud developer site
void handleSetCounter(BERGCloudMessage&);
void handleGreet(BERGCloudMessage&);

// Create an instance of the BERGCloudMbed class
BERGCloudMbed BERGCloud;

// The counter we will increment and send up to the cloud
int32_t counter;

int main(void)
{
  BERGCloud.begin(MOSI_PIN, MISO_PIN, SCLK_PIN, nSSEL_PIN);
  printf("--- mbed reset ---\r\n");

  counter = 0; // Initialise our counter

  // Attempt to connect with our project key and build version
  if (BERGCloud.connect(PROJECT_KEY, VERSION)) {
    printf("Connected to network");
  } else {
    printf("BERGCloud.connect() returned false.");
  }
  
  // Run continuously
  while (1)
  {
    // The loop() function handles commands and events
    loop();
  }
  
}

void loop(void)
{ 
  // The ID of the command we've received. 
  uint8_t commandID;
  
  // The command and event objects we use below    
  BERGCloudMessage command, event;

  // Some simple string manipulation

  ///////////////////////////////////////////////////////////////
  // Fetching commands                                         //
  ///////////////////////////////////////////////////////////////
  
  printf("Poll for command... ");
  if (BERGCloud.pollForCommand(command, commandID)) {

    // Print the 
    printf("got command with ID: %u\r\n", commandID);
    
    // Here we can map the command IDs to method calls within our code
    switch (commandID) {
      case COMMAND_SET_COUNTER:
        // Command one contains an integer
        handleSetCounter(command);
        break;
      case COMMAND_GREET:
        // Command two is a simple message containing a string
        handleGreet(command);
        break;
      // Default case, we don't have a match for the Command ID
      default:
        printf("WARNING: Unknown command\r\n");   
    }
  } else {
    // No command!
    printf("none.\r\n");
  }
  
  ///////////////////////////////////////////////////////////////
  // Sending events                                            //
  ///////////////////////////////////////////////////////////////

  printf("Sending an event... ");

  // In this Little Counter example we send up a string and a counter
  //
  // Packing is very straight forward. Just define your
  // BERGCloudMessage object and call pack() passing in each type
  // you wish to encode.
  
  // There is currently a 64 byte limit on the size of events, so be
  // careful with how much data you're sending up!
  
  event.pack("BERG");  // Pack a string
  event.pack(counter); // Pack an unsigned int32
  
  // Send the event object
  if (BERGCloud.sendEvent(EVENT_COUNTER_CHANGED, event))  {
    printf("ok\r\n");
  } else {
    printf("failed/busy\r\n");
  }
  
  counter++; // Increment our counter each time we loop around

  // A simple delay to rate limit what we send up to the cloud
  wait_ms(5000);
}

void handleSetCounter(BERGCloudMessage &command) {
  string prefixText;
  int32_t newCounterVal;
  stringstream counterText;
  
  prefixText = "Counter set to";

  // We're expecting an integer value for the counter
  if (command.unpack(newCounterVal)) {
    
    printf("Decoded newCounterVal as: %i\r\n", newCounterVal);
    
    // Set the global to our new value
    counter = newCounterVal;

    // Convert to string (see std::stringstream)
    counterText << newCounterVal;

    // Show the string on the OLED screen with display()
    BERGCloud.clearDisplay();
    
    // Print in reverse order
    BERGCloud.display(counterText.str().c_str());
    BERGCloud.display(prefixText.c_str());
  } else {
    printf("WARNING: unpacking the new counter value failed\r\n");
  }
}

void handleGreet(BERGCloudMessage &command) {
  string prefixText, suffixText, finalText;
  int32_t number;
  
  prefixText = "Hello, ";

  // We're expecting a string and a number for display-text, so
  // we attempt to decode these types in turn

  if (command.unpack(suffixText)) {
    printf("Decoded text: '%s'\r\n", suffixText.c_str());

    // Concatenate our strings
    finalText = prefixText + suffixText;

    // Show the string on the OLED screen with display()
    BERGCloud.clearDisplay();
    BERGCloud.display(finalText.c_str());
  } else {
    printf("WARNING: unpacking text failed\r\n");
  }
  
  // For this command, we can optionally be passed an integer
  // and we log this out on the serial console.
  // 
  // This is to demonstrate serializing multiple values within
  // one command.

  if (command.unpack(number)) {
    printf("Decoded number: %i\r\n", number);
  } else {
    printf("No additional number given\r\n");
  }
}

