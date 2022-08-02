#pragma once
/* MD5
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
 
#include <stdint.h>
#include <intrin.h>

class MD5
{
	static constexpr size_t blocksizeinBytes = 64;
	static constexpr size_t numInt4sinBlock = 16;
	static constexpr uint32_t integerTable[64] =
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

	static constexpr uint32_t shiftTable[16] =
	{
		7, 12, 17, 22,
		5,  9, 14, 20,
		4, 11, 16, 23,
		6, 10, 15, 21,
	};

	uint32_t result[4] = { 0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476 };
	uint32_t tmpValues[5] = {};

	bool m_interrupt = false;

public:
	void generate(const unsigned char* input, const unsigned char* const end);
	void interrupt() { m_interrupt = true; }
	void display() const;
	bool operator<(const MD5& other) const;
	bool operator==(const MD5& other) const;
 
private:
	void transform(const uint32_t block[numInt4sinBlock]);

	template<int round, uint32_t shift>
	void processValues(uint32_t value)
	{
		static_assert(1 <= round && round < 5);
		if constexpr (round == 1)
			value += (tmpValues[1] & tmpValues[2]) | (~tmpValues[1] & tmpValues[3]);
		else if constexpr (round == 2)
			value += (tmpValues[3] & tmpValues[1]) | (~tmpValues[3] & tmpValues[2]);
		else if constexpr (round == 3)
			value += tmpValues[1] ^ tmpValues[2] ^ tmpValues[3];
		else
			value += tmpValues[2] ^ (tmpValues[1] | ~tmpValues[3]);

		tmpValues[0] = tmpValues[1] + _rotl(value + tmpValues[4], shift);
		tmpValues[4] = tmpValues[3];
		tmpValues[3] = tmpValues[2];
		tmpValues[2] = tmpValues[1];
		tmpValues[1] = tmpValues[0];
	}
};
