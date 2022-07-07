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

//////////////////////////////

void MD5::generate(const unsigned char* input, const unsigned char* const end)
{
    uint64_t numBits = 8 * (end - input);
    while (!m_interrupt && input + blocksizeinBytes <= end)
    {
        transform(reinterpret_cast<const uint32_t*>(input));
        input += blocksizeinBytes;
    }

    if (!m_interrupt)
    {
        uint64_t index = end - input;
        if (index > 0)
        {
            uint8_t buffer[blocksizeinBytes];

            memcpy(buffer, input, index);
            buffer[index++] = 128;
            if (index > 56)
            {
                memset(buffer + index, 0, blocksizeinBytes - index);
                transform(reinterpret_cast<uint32_t*>(buffer));

                memset(buffer, 0, 56);
            }
            else
                memset(buffer + index, 0, 56 - index);

            *reinterpret_cast<uint64_t*>(buffer + 56) = numBits;
            transform(reinterpret_cast<uint32_t*>(buffer));
        }
    }

    m_finished = true;
    m_condition.notify_one();
}

inline void round1(uint32_t(&values)[4], const uint32_t integer, const uint32_t shift)
{
    uint32_t f = (values[1] & values[2]) | (~values[1] & values[3]);
    uint32_t h = values[1] + _rotl(f + values[0] + integer, shift);
    values[0] = values[3];
    values[3] = values[2];
    values[2] = values[1];
    values[1] = h;
}

inline void round2(uint32_t(&values)[4], const uint32_t integer, const uint32_t shift)
{
    uint32_t f = (values[3] & values[1]) | (~values[3] & values[2]);
    uint32_t h = values[1] + _rotl(f + values[0] + integer, shift);
    values[0] = values[3];
    values[3] = values[2];
    values[2] = values[1];
    values[1] = h;
}

inline void round3(uint32_t(&values)[4], const uint32_t integer, const uint32_t shift)
{
    uint32_t  f = values[1] ^ values[2] ^ values[3];
    uint32_t h = values[1] + _rotl(f + values[0] + integer, shift);
    values[0] = values[3];
    values[3] = values[2];
    values[2] = values[1];
    values[1] = h;
}

inline void round4(uint32_t(&values)[4], const uint32_t integer, const uint32_t shift)
{
    uint32_t f = values[2] ^ (values[1] | ~values[3]);
    uint32_t h = values[1] + _rotl(f + values[0] + integer, shift);
    values[0] = values[3];
    values[3] = values[2];
    values[2] = values[1];
    values[1] = h;
}

// apply MD5 algo on a block
void MD5::transform(const uint32_t block[numInt4sinBlock])
{
    uint32_t values[4];
    memcpy(values, result, sizeof(uint32_t) * 4);

    round1(values, block[0] + integerTable[0], shiftTable[0]);
    round1(values, block[1] + integerTable[1], shiftTable[1]);
    round1(values, block[2] + integerTable[2], shiftTable[2]);
    round1(values, block[3] + integerTable[3], shiftTable[3]);
    round1(values, block[4] + integerTable[4], shiftTable[0]);
    round1(values, block[5] + integerTable[5], shiftTable[1]);
    round1(values, block[6] + integerTable[6], shiftTable[2]);
    round1(values, block[7] + integerTable[7], shiftTable[3]);
    round1(values, block[8] + integerTable[8], shiftTable[0]);
    round1(values, block[9] + integerTable[9], shiftTable[1]);
    round1(values, block[10] + integerTable[10], shiftTable[2]);
    round1(values, block[11] + integerTable[11], shiftTable[3]);
    round1(values, block[12] + integerTable[12], shiftTable[0]);
    round1(values, block[13] + integerTable[13], shiftTable[1]);
    round1(values, block[14] + integerTable[14], shiftTable[2]);
    round1(values, block[15] + integerTable[15], shiftTable[3]);

    round2(values, block[1] + integerTable[16], shiftTable[4]);
    round2(values, block[6] + integerTable[17], shiftTable[5]);
    round2(values, block[11] + integerTable[18], shiftTable[6]);
    round2(values, block[0] + integerTable[19], shiftTable[7]);
    round2(values, block[5] + integerTable[20], shiftTable[4]);
    round2(values, block[10] + integerTable[21], shiftTable[5]);
    round2(values, block[15] + integerTable[22], shiftTable[6]);
    round2(values, block[4] + integerTable[23], shiftTable[7]);
    round2(values, block[9] + integerTable[24], shiftTable[4]);
    round2(values, block[14] + integerTable[25], shiftTable[5]);
    round2(values, block[3] + integerTable[26], shiftTable[6]);
    round2(values, block[8] + integerTable[27], shiftTable[7]);
    round2(values, block[13] + integerTable[28], shiftTable[4]);
    round2(values, block[2] + integerTable[29], shiftTable[5]);
    round2(values, block[7] + integerTable[30], shiftTable[6]);
    round2(values, block[12] + integerTable[31], shiftTable[7]);

    round3(values, block[5] + integerTable[32], shiftTable[8]);
    round3(values, block[8] + integerTable[33], shiftTable[9]);
    round3(values, block[11] + integerTable[34], shiftTable[10]);
    round3(values, block[14] + integerTable[35], shiftTable[11]);
    round3(values, block[1] + integerTable[36], shiftTable[8]);
    round3(values, block[4] + integerTable[37], shiftTable[9]);
    round3(values, block[7] + integerTable[38], shiftTable[10]);
    round3(values, block[10] + integerTable[39], shiftTable[11]);
    round3(values, block[13] + integerTable[40], shiftTable[8]);
    round3(values, block[0] + integerTable[41], shiftTable[9]);
    round3(values, block[3] + integerTable[42], shiftTable[10]);
    round3(values, block[6] + integerTable[43], shiftTable[11]);
    round3(values, block[9] + integerTable[44], shiftTable[8]);
    round3(values, block[12] + integerTable[45], shiftTable[9]);
    round3(values, block[15] + integerTable[46], shiftTable[10]);
    round3(values, block[2] + integerTable[47], shiftTable[11]);

    round4(values, block[0] + integerTable[48], shiftTable[12]);
    round4(values, block[7] + integerTable[49], shiftTable[13]);
    round4(values, block[14] + integerTable[50], shiftTable[14]);
    round4(values, block[5] + integerTable[51], shiftTable[15]);
    round4(values, block[12] + integerTable[52], shiftTable[12]);
    round4(values, block[3] + integerTable[53], shiftTable[13]);
    round4(values, block[10] + integerTable[54], shiftTable[14]);
    round4(values, block[1] + integerTable[55], shiftTable[15]);
    round4(values, block[8] + integerTable[56], shiftTable[12]);
    round4(values, block[15] + integerTable[57], shiftTable[13]);
    round4(values, block[6] + integerTable[58], shiftTable[14]);
    round4(values, block[13] + integerTable[59], shiftTable[15]);
    round4(values, block[4] + integerTable[60], shiftTable[12]);
    round4(values, block[11] + integerTable[61], shiftTable[13]);
    round4(values, block[2] + integerTable[62], shiftTable[14]);
    round4(values, block[9] + integerTable[63], shiftTable[15]);

    result[0] += values[0];
    result[1] += values[1];
    result[2] += values[2];
    result[3] += values[3];
}

using namespace std::chrono_literals;

void MD5::wait()
{
    std::unique_lock lk(m_mutex);
    while (!m_finished)
        m_condition.wait_for(lk, 10ms);
}

//////////////////////////////
#include <iostream>

// displays hex representation of hash to console
void MD5::display() const
{
    std::cout << "Hash: ";
    unsigned char str[16];
    memcpy(str, result, 16);
    for (unsigned char num : str)
        printf_s("%02x", num);
    std::cout << '\n';
}
