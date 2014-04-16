/*

BERGCloud constant definitions

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

/*
 * Sizes of things
 */

#define BC_EUI64_SIZE_BYTES            8
#define BC_ADDRESS_SIZE_BYTES          8
#define BC_CLAIMCODE_SIZE_BYTES        20
#define BC_KEY_SIZE_BYTES              16
#define BC_PRINT_MAX_CHARS             26

/*
 * Network commands
 */

#define BC_EVENT_ANNOUNCE              0xA000
#define BC_COMMAND_SET_ADDRESS         0xB000

#define BC_COMMAND_START_RAW           0xC000
#define BC_COMMAND_START_PACKED        0xC100
#define BC_COMMAND_NAMED_PACKED        0xC17F
#define BC_COMMAND_ID_MASK             0x00FF
#define BC_COMMAND_FORMAT_MASK         0xFF00

#define BC_COMMAND_DISPLAY_IMAGE       0xD000
#define BC_COMMAND_DISPLAY_TEXT        0xD001

#define BC_EVENT_START_RAW             0xE000
#define BC_EVENT_START_PACKED          0xE100
#define BC_EVENT_NAMED_PACKED          0xE17F
#define BC_EVENT_ID_MASK               0x00FF
#define BC_EVENT_FORMAT_MASK           0xFF00

#define BC_COMMAND_FIRMWARE_ARDUINO    0xF010
#define BC_COMMAND_FIRMWARE_MBED       0xF020

/*
 * SPI bus commands
 */

#define SPI_CMD_GET_CONNECT_STATE      0x80
#define SPI_CMD_GET_CLAIMCODE          0x81
#define SPI_CMD_GET_CLAIM_STATE        0x82
#define SPI_CMD_GET_SIGNAL_QUALITY     0x83
#define SPI_CMD_GET_EUI64              0x90

#define SPI_CMD_SEND_ANNOUNCE          0xA0
#define SPI_CMD_GET_ADDRESS            0xB0
#define SPI_CMD_POLL_FOR_COMMAND       0xC0
#define SPI_CMD_SET_DISPLAY_STYLE      0xD0
#define SPI_CMD_DISPLAY_PRINT          0xD1
#define SPI_CMD_SEND_EVENT_RAW         0xE0
#define SPI_CMD_SEND_EVENT_PACKED      0xE1

#define SPI_PROTOCOL_PAD               0xff
#define SPI_PROTOCOL_PENDING           0xfa
#define SPI_PROTOCOL_RESET             0xf5

/* For SPI_CMD_GET_CONNECT_STATE */
#define BC_CONNECT_STATE_CONNECTED     0x00
#define BC_CONNECT_STATE_CONNECTING    0x01
#define BC_CONNECT_STATE_DISCONNECTED  0x02

/* For SPI_CMD_GET_EUI64 */
#define BC_EUI64_NODE                  0x00
#define BC_EUI64_PARENT                0x01
#define BC_EUI64_COORDINATOR           0x02

/* For SPI_CMD_SEND_ANNOUNCE */
#define BC_HOST_UNKNOWN                0x0000
#define BC_HOST_ARDUINO                0x1000
#define BC_HOST_MBED                   0x2000
#define BC_HOST_LINUX                  0x3000

/* For SPI_CMD_GET_CLAIM_STATE */
#define BC_CLAIM_STATE_CLAIMED         0x00
#define BC_CLAIM_STATE_NOT_CLAIMED     0x01

/* For SPI_CMD_DISPLAY_STYLE */
#define BC_DISPLAY_NONE                0x00
#define BC_DISPLAY_STYLE_ONE_LINE      0x01
#define BC_DISPLAY_STYLE_TWO_LINES     0x02
#define BC_DISPLAY_STYLE_FOUR_LINES    0x04

#define BC_DISPLAY_CLEAR               0xc0
/* Clear the display without changing the style */

/* For SPI_CMD_SEND_EVENT_ */
#define SPI_EVENT_HEADER_SIZE_BYTES    4

/* SPI data sizes */
#define SPI_MAX_PACKET_SIZE_BYTES      128
#define SPI_HEADER_SIZE_BYTES          4
#define SPI_FOOTER_SIZE_BYTES          2

#define SPI_MAX_PAYLOAD_SIZE_BYTES \
(SPI_MAX_PACKET_SIZE_BYTES - (SPI_HEADER_SIZE_BYTES + SPI_FOOTER_SIZE_BYTES))

/* Response status values */
#define SPI_RSP_SUCCESS                0x00
#define SPI_RSP_INVALID_COMMAND        0x01
#define SPI_RSP_BUSY                   0x02
#define SPI_RSP_NO_DATA                0x03
#define SPI_RSP_SEND_FAILED            0x04
#define SPI_RSP_NO_FREE_BUFFERS        0x05
