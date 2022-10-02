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
#include "FileTraversal/FilePointers.h"
#include <fstream>

class MD5
{
    uint32_t m_value[4]
    {
        0x67452301,
        0xefcdab89,
        0x98badcfe,
        0x10325476,
    };

public:
	void computeHash(const FilePointers& _File);
    MD5() = default;
    MD5(MD5&&) = default;
    MD5(const MD5&) = default;
    MD5& operator=(MD5&&) = default;
    MD5& operator=(const MD5&) = default;
    
    void display() const;
    void writeToCache(std::fstream& outFile) const;
    void readFromCache(const unsigned char*& _inFile);
	friend bool operator<(const MD5& _lhs, const MD5& _rhs)
    {
        const uint64_t* result64 = reinterpret_cast<const uint64_t*>(_lhs.m_value);
        const uint64_t* other64 = reinterpret_cast<const uint64_t*>(_rhs.m_value);

        if (result64[1] == other64[1])
            return result64[0] < other64[0];

        return result64[1] < other64[1];
    }

    friend bool operator==(const MD5& _lhs, const MD5& _rhs)
    {
        return _lhs.m_value[0] == _rhs.m_value[0] &&
            _lhs.m_value[1] == _rhs.m_value[1] &&
            _lhs.m_value[2] == _rhs.m_value[2] &&
            _lhs.m_value[3] == _rhs.m_value[3];
    }
};
