/* MD5
 converted to C++ class by Frank Thilo (thilo@unix-ag.org)
 for bzflag (http://www.bzflag.org)
 
   based on:
 
   md5.h and md5.c
   reference implemantion of RFC 1321
 
   Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.
 
License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.
 
License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.
 
RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.
 
These notices must be retained in any copies of any part of this
documentation and/or software.
 
*/
 
/* interface header */
#include "md5.h"
 
/* system implementation headers */
#include <cstring>
#include <intrin.h>
 
///////////////////////////////////////////////

const uint32_t MD5::integerTable[64] =
{
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

const uint32_t MD5::shiftTable[64] =
{
    7, 12, 17, 22,
    7, 12, 17, 22,
    7, 12, 17, 22,
    7, 12, 17, 22,
    5,  9, 14, 20,
    5,  9, 14, 20,
    5,  9, 14, 20,
    5,  9, 14, 20,
    4, 11, 16, 23,
    4, 11, 16, 23,
    4, 11, 16, 23,
    4, 11, 16, 23,
    6, 10, 15, 21,
    6, 10, 15, 21,
    6, 10, 15, 21,
    6, 10, 15, 21
};


//////////////////////////////

void MD5::generate(const unsigned char* input, const unsigned char* const end)
{
  uint64_t count = 8 *(end - input);
  while (input + blocksizeinBytes <= end)
  {
      transform(input);
      input += blocksizeinBytes;
  }

  uint64_t index = end - input;
  if (index > 0)
  {
      uint8_t buffer[blocksizeinBytes];

      memcpy(buffer, input, index);
      buffer[index++] = 128;
      if (index > 56)
      {
          memset(buffer + index, 0, blocksizeinBytes - index);
          transform(buffer);

          memset(buffer, 0, 56);
      }
      else
          memset(buffer + index, 0, 56 - index);

      *reinterpret_cast<uint64_t*>(buffer + 56) = count;
      transform(buffer);
  }
}
 
//////////////////////////////

// apply MD5 algo on a block
void MD5::transform(const unsigned char block[blocksizeinBytes])
{
  uint32_t a = result[0], b = result[1], c = result[2], d = result[3], m[numInt4sinBlock];

  memcpy(m, block, blocksizeinBytes);

  for (int i = 0; i < 64; ++i)
  {
      uint32_t f, g, h;
      if (i < 16)
      {
          f = (b & c) | (~b & d);
          g = i;
      }
      else if (i < 32)
      {
          f = (d & b) | (~d & c);
          g = (5 * i + 1) % 16;
      }
      else if (i < 48)
      {
          f = b ^ c ^ d;
          g = (3 * i + 5) % 16;
      }
      else
      {
          f = c ^ (b | ~d);
          g = (7 * i) % 16;
      }

      h = _rotl(f + a + integerTable[i] + m[g], shiftTable[i]);
      a = d;
      d = c;
      c = b;
      b += h;
  }
 
  result[0] += a;
  result[1] += b;
  result[2] += c;
  result[3] += d;
}
 
//////////////////////////////
#include <iostream>
 
// return hex representation of digest as string
void MD5::display() const
{
    std::cout << "Hash: ";
    unsigned char str[16];
    memcpy(str, result, 16);
    for (unsigned char num : str)
        printf_s("%02x", num);
    std::cout << '\n';
}

bool MD5::operator<(const MD5& other) const
{
    for (int i = 3; i >= 0; --i)
        if (result[i] < other.result[i])
            return true;
        else if (result[i] > other.result[i])
            return false;
    return false;
}

bool MD5::operator<=(const MD5& other) const
{
    for (int i = 3; i >= 0; --i)
        if (result[i] < other.result[i])
            return true;
        else if (result[i] > other.result[i])
            return false;
    return true;
}

bool MD5::operator>(const MD5& other) const
{
    for (int i = 3; i >= 0; --i)
        if (result[i] > other.result[i])
            return true;
        else if (result[i] < other.result[i])
            return false;
    return false;
}

bool MD5::operator>=(const MD5& other) const
{
    for (int i = 3; i >= 0; --i)
        if (result[i] > other.result[i])
            return true;
        else if (result[i] < other.result[i])
            return false;
    return true;
}

bool MD5::operator==(const MD5& other) const
{
    for (int i = 3; i >= 0; --i)
        if (result[i] != other.result[i])
            return false;
    return true;
}

bool MD5::operator!=(const MD5& other) const
{
    for (int i = 3; i >= 0; --i)
        if (result[i] != other.result[i])
            return true;
    return false;
}
 