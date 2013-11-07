/*

Debug logging helpers

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

#ifndef BERGCLOUDLOGPRINT_H
#define BERGCLOUDLOGPRINT_H

#ifdef BERGCLOUD_LOG
#ifdef ARDUINO
#include <Arduino.h>
// Workaround for http://gcc.gnu.org/bugzilla/show_bug.cgi?id=34734
// Credit: http://forum.arduino.cc/index.php/topic,85840.0.html
#ifdef PROGMEM
#undef PROGMEM
#define PROGMEM __attribute__((section(".progmem.data")))
#endif // #ifdef PROGMEM
#define _LOG(x) Serial.print(F(x))
#define _LOG_HEX(x) if ((x) < 0x10) Serial.print(F("0")); Serial.print((x), HEX)
#else // #ifdef ARDUINO
#include <stdio.h>
#define _LOG(x) printf(x)
#define _LOG_HEX(x) printf("%02X", (x))
#endif // #ifdef ARDUINO
#else // #ifdef BERGCLOUD_LOG
#define _LOG(x)
#endif // #ifdef BERGCLOUD_LOG

#endif // #ifndef BERGCLOUDLOGPRINT_H
