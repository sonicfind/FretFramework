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

#include "MD5.h"
#include <cstring>

void MD5::computeHash(const unsigned char* _input, const size_t _length)
{
    static constexpr size_t BLOCKSIZEINBYTES = 64;
    static constexpr size_t NUMINT32INBLOCK = 16;

    m_value[0] = 0x67452301;
    m_value[1] = 0xefcdab89;
    m_value[2] = 0x98badcfe;
    m_value[3] = 0x10325476;

    const unsigned char* const endofFile = _input + _length;
    const unsigned char* const endofLoop = endofFile - BLOCKSIZEINBYTES;
    const uint64_t numBits = 8 * _length;

    while (_input <= endofLoop)
    {
        evaluateBlock(_input);
        _input += BLOCKSIZEINBYTES;
    }

    char block[BLOCKSIZEINBYTES];
    size_t leftover = endofFile - _input;
    memcpy(block, _input, leftover);
    block[leftover++] = (char)0x80;

    if (leftover > 56)
    {
        memset(block + leftover, 0, BLOCKSIZEINBYTES - leftover);
        evaluateBlock(block);

        memset(block, 0, 56);
    }
    else
        memset(block + leftover, 0, 56 - leftover);

    *reinterpret_cast<uint64_t*>(block + 56) = numBits;
    evaluateBlock(block);
}

void MD5::evaluateBlock(const void* _Ptr)
{
    static constexpr uint32_t INTEGERTABLE[64] =
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

    const uint32_t* block = static_cast<const uint32_t*>(_Ptr);

    m_processor.m_buffer[0] = m_value[1];
    m_processor.m_buffer[1] = m_value[2];
    m_processor.m_buffer[2] = m_value[3];
    m_processor.m_buffer[3] = m_value[0];
    m_processor.calculate<0>({ block[0 ] + INTEGERTABLE[0 ],
                               block[1 ] + INTEGERTABLE[1 ], 
                               block[2 ] + INTEGERTABLE[2 ], 
                               block[3 ] + INTEGERTABLE[3 ],
                               block[4 ] + INTEGERTABLE[4 ],
                               block[5 ] + INTEGERTABLE[5 ],
                               block[6 ] + INTEGERTABLE[6 ],
                               block[7 ] + INTEGERTABLE[7 ],
                               block[8 ] + INTEGERTABLE[8 ],
                               block[9 ] + INTEGERTABLE[9 ],
                               block[10] + INTEGERTABLE[10],
                               block[11] + INTEGERTABLE[11],
                               block[12] + INTEGERTABLE[12],
                               block[13] + INTEGERTABLE[13],
                               block[14] + INTEGERTABLE[14],
                               block[15] + INTEGERTABLE[15] });

    m_processor.calculate<1>({ block[1 ] + INTEGERTABLE[16],
                               block[6 ] + INTEGERTABLE[17], 
                               block[11] + INTEGERTABLE[18], 
                               block[0 ] + INTEGERTABLE[19],
                               block[5 ] + INTEGERTABLE[20],
                               block[10] + INTEGERTABLE[21],
                               block[15] + INTEGERTABLE[22],
                               block[4 ] + INTEGERTABLE[23],
                               block[9 ] + INTEGERTABLE[24],
                               block[14] + INTEGERTABLE[25],
                               block[3 ] + INTEGERTABLE[26],
                               block[8 ] + INTEGERTABLE[27],
                               block[13] + INTEGERTABLE[28],
                               block[2 ] + INTEGERTABLE[29],
                               block[7 ] + INTEGERTABLE[30],
                               block[12] + INTEGERTABLE[31] });

    m_processor.calculate<2>({ block[5 ] + INTEGERTABLE[32],
                               block[8 ] + INTEGERTABLE[33], 
                               block[11] + INTEGERTABLE[34], 
                               block[14] + INTEGERTABLE[35],
                               block[1 ] + INTEGERTABLE[36],
                               block[4 ] + INTEGERTABLE[37],
                               block[7 ] + INTEGERTABLE[38],
                               block[10] + INTEGERTABLE[39],
                               block[13] + INTEGERTABLE[40],
                               block[0 ] + INTEGERTABLE[41],
                               block[3 ] + INTEGERTABLE[42],
                               block[6 ] + INTEGERTABLE[43],
                               block[9 ] + INTEGERTABLE[44],
                               block[12] + INTEGERTABLE[45],
                               block[15] + INTEGERTABLE[46],
                               block[2 ] + INTEGERTABLE[47] });

    m_processor.calculate<3>({ block[0 ] + INTEGERTABLE[48],
                               block[7 ] + INTEGERTABLE[49], 
                               block[14] + INTEGERTABLE[50], 
                               block[5 ] + INTEGERTABLE[51],
                               block[12] + INTEGERTABLE[52],
                               block[3 ] + INTEGERTABLE[53],
                               block[10] + INTEGERTABLE[54],
                               block[1 ] + INTEGERTABLE[55],
                               block[8 ] + INTEGERTABLE[56],
                               block[15] + INTEGERTABLE[57],
                               block[6 ] + INTEGERTABLE[58],
                               block[13] + INTEGERTABLE[59],
                               block[4 ] + INTEGERTABLE[60],
                               block[11] + INTEGERTABLE[61],
                               block[2 ] + INTEGERTABLE[62],
                               block[9 ] + INTEGERTABLE[63] });

    m_value[0] += m_processor.m_buffer[3];
    m_value[1] += m_processor.m_buffer[0];
    m_value[2] += m_processor.m_buffer[1];
    m_value[3] += m_processor.m_buffer[2];
}

bool operator<(const MD5& _lhs, const MD5& _rhs)
{
    const uint64_t* result64 = reinterpret_cast<const uint64_t*>(_lhs.m_value);
    const uint64_t* other64 = reinterpret_cast<const uint64_t*>(_rhs.m_value);

    if (result64[1] == other64[1])
        return result64[0] < other64[0];

    return result64[1] < other64[1];
}

bool operator==(const MD5& _lhs, const MD5& _rhs)
{
    return _lhs.m_value[0] == _rhs.m_value[0] &&
        _lhs.m_value[1] == _rhs.m_value[1] &&
        _lhs.m_value[2] == _rhs.m_value[2] &&
        _lhs.m_value[3] == _rhs.m_value[3];
}

//////////////////////////////
#include <iostream>

// displays hex representation of hash to console
void MD5::display() const
{
    std::cout << "Hash: ";
    unsigned char str[16];
    memcpy(str, m_value, 16);
    for (unsigned char num : str)
        printf_s("%02x", num);
    std::cout << '\n';
}
