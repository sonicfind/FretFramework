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

//////////////////////////////

void MD5::generate(const unsigned char* input, const size_t length)
{
    const unsigned char* const endofFile = input + length;
    const unsigned char* const endofLoop = endofFile - blocksizeinBytes;
    const uint64_t numBits = 8 * length;

    while (input <= endofLoop)
    {
        transform(reinterpret_cast<const uint32_t*>(input));
        input += blocksizeinBytes;
    }

    uint8_t buffer[blocksizeinBytes];
    size_t leftover = endofFile - input;
    memcpy(buffer, input, leftover);
    buffer[leftover++] = 0x80;
    if (leftover > 56)
    {
        memset(buffer + leftover, 0, blocksizeinBytes - leftover);
        transform(reinterpret_cast<uint32_t*>(buffer));

        memset(buffer, 0, 56);
    }
    else
        memset(buffer + leftover, 0, 56 - leftover);

    *reinterpret_cast<uint64_t*>(buffer + 56) = numBits;
    transform(reinterpret_cast<uint32_t*>(buffer));
}

// apply MD5 algo on a block
void MD5::transform(const uint32_t block[numInt4sinBlock])
{
    tmpValues[4] = result[0];
    tmpValues[1] = result[1];
    tmpValues[2] = result[2];
    tmpValues[3] = result[3];

    processValues<1, shiftTable[0]>(block[0]  + integerTable[0 ]);
    processValues<1, shiftTable[1]>(block[1]  + integerTable[1 ]);
    processValues<1, shiftTable[2]>(block[2]  + integerTable[2 ]);
    processValues<1, shiftTable[3]>(block[3]  + integerTable[3 ]);
    processValues<1, shiftTable[0]>(block[4]  + integerTable[4 ]);
    processValues<1, shiftTable[1]>(block[5]  + integerTable[5 ]);
    processValues<1, shiftTable[2]>(block[6]  + integerTable[6 ]);
    processValues<1, shiftTable[3]>(block[7]  + integerTable[7 ]);
    processValues<1, shiftTable[0]>(block[8]  + integerTable[8 ]);
    processValues<1, shiftTable[1]>(block[9]  + integerTable[9 ]);
    processValues<1, shiftTable[2]>(block[10] + integerTable[10]);
    processValues<1, shiftTable[3]>(block[11] + integerTable[11]);
    processValues<1, shiftTable[0]>(block[12] + integerTable[12]);
    processValues<1, shiftTable[1]>(block[13] + integerTable[13]);
    processValues<1, shiftTable[2]>(block[14] + integerTable[14]);
    processValues<1, shiftTable[3]>(block[15] + integerTable[15]);

    processValues<2, shiftTable[4]>(block[1]  + integerTable[16]);
    processValues<2, shiftTable[5]>(block[6]  + integerTable[17]);
    processValues<2, shiftTable[6]>(block[11] + integerTable[18]);
    processValues<2, shiftTable[7]>(block[0]  + integerTable[19]);
    processValues<2, shiftTable[4]>(block[5]  + integerTable[20]);
    processValues<2, shiftTable[5]>(block[10] + integerTable[21]);
    processValues<2, shiftTable[6]>(block[15] + integerTable[22]);
    processValues<2, shiftTable[7]>(block[4]  + integerTable[23]);
    processValues<2, shiftTable[4]>(block[9]  + integerTable[24]);
    processValues<2, shiftTable[5]>(block[14] + integerTable[25]);
    processValues<2, shiftTable[6]>(block[3]  + integerTable[26]);
    processValues<2, shiftTable[7]>(block[8]  + integerTable[27]);
    processValues<2, shiftTable[4]>(block[13] + integerTable[28]);
    processValues<2, shiftTable[5]>(block[2]  + integerTable[29]);
    processValues<2, shiftTable[6]>(block[7]  + integerTable[30]);
    processValues<2, shiftTable[7]>(block[12] + integerTable[31]);

    processValues<3, shiftTable[8] >(block[5]  + integerTable[32]);
    processValues<3, shiftTable[9] >(block[8]  + integerTable[33]);
    processValues<3, shiftTable[10]>(block[11] + integerTable[34]);
    processValues<3, shiftTable[11]>(block[14] + integerTable[35]);
    processValues<3, shiftTable[8] >(block[1]  + integerTable[36]);
    processValues<3, shiftTable[9] >(block[4]  + integerTable[37]);
    processValues<3, shiftTable[10]>(block[7]  + integerTable[38]);
    processValues<3, shiftTable[11]>(block[10] + integerTable[39]);
    processValues<3, shiftTable[8] >(block[13] + integerTable[40]);
    processValues<3, shiftTable[9] >(block[0]  + integerTable[41]);
    processValues<3, shiftTable[10]>(block[3]  + integerTable[42]);
    processValues<3, shiftTable[11]>(block[6]  + integerTable[43]);
    processValues<3, shiftTable[8] >(block[9]  + integerTable[44]);
    processValues<3, shiftTable[9] >(block[12] + integerTable[45]);
    processValues<3, shiftTable[10]>(block[15] + integerTable[46]);
    processValues<3, shiftTable[11]>(block[2]  + integerTable[47]);

    processValues<4, shiftTable[12]>(block[0]  + integerTable[48]);
    processValues<4, shiftTable[13]>(block[7]  + integerTable[49]);
    processValues<4, shiftTable[14]>(block[14] + integerTable[50]);
    processValues<4, shiftTable[15]>(block[5]  + integerTable[51]);
    processValues<4, shiftTable[12]>(block[12] + integerTable[52]);
    processValues<4, shiftTable[13]>(block[3]  + integerTable[53]);
    processValues<4, shiftTable[14]>(block[10] + integerTable[54]);
    processValues<4, shiftTable[15]>(block[1]  + integerTable[55]);
    processValues<4, shiftTable[12]>(block[8]  + integerTable[56]);
    processValues<4, shiftTable[13]>(block[15] + integerTable[57]);
    processValues<4, shiftTable[14]>(block[6]  + integerTable[58]);
    processValues<4, shiftTable[15]>(block[13] + integerTable[59]);
    processValues<4, shiftTable[12]>(block[4]  + integerTable[60]);
    processValues<4, shiftTable[13]>(block[11] + integerTable[61]);
    processValues<4, shiftTable[14]>(block[2]  + integerTable[62]);
    processValues<4, shiftTable[15]>(block[9]  + integerTable[63]);

    result[0] += tmpValues[4];
    result[1] += tmpValues[1];
    result[2] += tmpValues[2];
    result[3] += tmpValues[3];
}

bool MD5::operator<(const MD5& other) const
{
    const uint64_t* result64 = reinterpret_cast<const uint64_t*>(result);
    const uint64_t* other64 = reinterpret_cast<const uint64_t*>(other.result);

    if (result64[1] == other64[1])
        return result64[0] < other64[0];

    return result64[1] < other64[1];
}

bool MD5::operator==(const MD5& other) const
{
    return result[0] == other.result[0] &&
        result[1] == other.result[1] &&
        result[2] == other.result[2] &&
        result[3] == other.result[3];
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
