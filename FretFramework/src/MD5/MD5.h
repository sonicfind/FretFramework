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
    class ValueProcessor
    {
        static constexpr uint32_t SHIFTTABLE[4][4] =
        {
            { 7, 12, 17, 22 },
            { 5,  9, 14, 20 },
            { 4, 11, 16, 23 },
            { 6, 10, 15, 21 },
        };

    public:
        uint32_t m_buffer[4];

        template<int __ROUND>
        void calculate(const uint32_t(&_integerValues)[16])
        {
            static_assert(0 <= __ROUND && __ROUND < 4);
            static constexpr uint32_t SHIFTS[4] = SHIFTTABLE[__ROUND];

            for (int j = 0; j < 16;)
            {
                for (int i = 0; i < 4; ++i, ++j)
                {
                    uint32_t value = _integerValues[j] + m_buffer[3];
                    if constexpr (__ROUND == 0)
                        value += (m_buffer[0] & m_buffer[1]) | (~m_buffer[0] & m_buffer[2]);
                    else if constexpr (__ROUND == 1)
                        value += (m_buffer[2] & m_buffer[0]) | (~m_buffer[2] & m_buffer[1]);
                    else if constexpr (__ROUND == 2)
                        value += m_buffer[0] ^ m_buffer[1] ^ m_buffer[2];
                    else
                        value += m_buffer[1] ^ (m_buffer[0] | ~m_buffer[2]);

                    m_buffer[3] = m_buffer[2];
                    m_buffer[2] = m_buffer[1];
                    m_buffer[1] = m_buffer[0];
                    m_buffer[0] += _rotl(value, SHIFTS[i]);
                }
            }
        }
    };

	uint32_t m_value[4];
    ValueProcessor m_processor;

public:
	void computeHash(const unsigned char* input, const size_t length);
	void display() const;
	friend bool operator<(const MD5& lhs, const MD5& rhs);
	friend bool operator==(const MD5& lhs, const MD5& rhs);
 
private:
	void evaluateBlock(const void* _Ptr);
};
