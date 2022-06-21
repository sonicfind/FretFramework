/* MD5
 converted to C++ class by Frank Thilo (thilo@unix-ag.org)
 for bzflag (http://www.bzflag.org)
 
   based on:
 
   md5.h and md5.c
   reference implementation of RFC 1321
 
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
 
#ifndef BZF_MD5_H
#define BZF_MD5_H
#include <stdint.h>
#include <compare>

class MD5
{
	static const size_t blocksizeinBytes = 64;
	static const size_t numInt4sinBlock = 16;
	static const uint32_t integerTable[64];
	static const uint32_t shiftTable[64];

	uint32_t result[4] = { 0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476 };
    bool m_forceStop = false;

public:
	void generate(const unsigned char* input, const unsigned char* const end);
    void forceStop() { m_forceStop = true; }
	void display() const;
    auto operator<=>(const MD5& other) const
    {
        const uint64_t* result64 = reinterpret_cast<const uint64_t*>(result);
        const uint64_t* other64 = reinterpret_cast<const uint64_t*>(other.result);
        if (result64[1] == other64[1])
            return result64[0] <=> other64[0];
        else
            return result64[1] <=> other64[1];
    }

    bool operator==(const MD5& other) const
    {
        return result[0] == other.result[0] &&
            result[1] == other.result[1] &&
            result[2] == other.result[2] &&
            result[3] == other.result[3];
    }

    bool operator!=(const MD5& other) const
    {
        return result[0] != other.result[0] ||
            result[1] != other.result[1] ||
            result[2] != other.result[2] ||
            result[3] != other.result[3];
    }
 
private:
	void transform(const uint32_t block[numInt4sinBlock]);
};

#endif