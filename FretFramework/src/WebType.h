#pragma once

#include <stdint.h>
#include <stdexcept>
#include <fstream>

class WebType
{
	uint32_t m_value;
	static char s_writeBuffer[5];
	static size_t s_bufferSize;

	void setBuffer() const;
public:
	WebType(const unsigned char*& dataPtr);
	WebType(uint32_t value);
	void copyToBuffer(char*& dataPtr) const;
	void writeToFile(std::fstream& outFile) const;
	WebType& operator=(uint32_t value);
	WebType& operator=(const WebType& value) = default;
	operator uint32_t() const;

	static void discard(const unsigned char*& dataPtr);
};
