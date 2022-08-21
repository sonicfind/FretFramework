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
	uint32_t m_value[4]{};

public:
	void computeHash(const unsigned char* input, const size_t length);
	void display() const;
	friend bool operator<(const MD5& lhs, const MD5& rhs);
	friend bool operator==(const MD5& lhs, const MD5& rhs);
 
private:
	void evaluateBlock(const void* _Ptr);
};
