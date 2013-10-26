/*

Simple buffer

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

#include "Buffer.h"

CBuffer::CBuffer(void)
{
  clear();
}

void CBuffer::clear(void)
{
  m_written = 0; /* Number of bytes written */
  m_read = 0;    /* Number of bytes read */
}

void CBuffer::restart(void)
{
  /* Restart reading from the beginning */
  m_read = 0;
}

uint16_t CBuffer::size(void)
{
  /* Get total size of the buffer */
  return BUFFER_SIZE_BYTES;
}

uint16_t CBuffer::used(void)
{
  /* Get mumber of bytes used in the buffer */
  return m_written;
}

void CBuffer::used(uint16_t used)
{
  /* Set number of bytes used in the buffer */
  m_written = used;
}

uint16_t CBuffer::available(void)
{
  /* Get space available in the buffer */
  return BUFFER_SIZE_BYTES - m_written;
}

bool CBuffer::available(uint16_t required)
{
  /* Test if space is available for the number of bytes required */
  return (BUFFER_SIZE_BYTES - m_written) >= required;
}

void CBuffer::add(uint8_t data)
{
  /* Write a byte to the buffer; no checks */
    m_data[m_written++] = data;
}

bool CBuffer::peek(uint8_t *data)
{
  /* Peek at the next byte in the buffer */
  if (m_read >= m_written)
  {
    /* No more data */
    return false;
  }

  
  *data = m_data[m_read]; /* No increment */
  return true;
}

uint8_t CBuffer::read(void)
{
  /* Read the next byte from the buffer; no checks */
  return m_data[m_read++];
}

uint16_t CBuffer::remaining(void)
{
  /* Returns the number of bytes that have been written but not read */
  return m_written - m_read;
}

bool CBuffer::remaining(uint16_t required)
{
  /* Test if data is remaining for the number of bytes required */
  return (m_written - m_read) >= required;
}

uint8_t *CBuffer::ptr(void)
{
  return m_data;
}
