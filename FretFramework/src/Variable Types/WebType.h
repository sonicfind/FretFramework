#pragma once
#include <stdint.h>
#include <fstream>

class WebType
{
	uint32_t m_value;

public:
	constexpr explicit WebType() : m_value(0) {}
	WebType(const unsigned char*& dataPtr);
	WebType(uint32_t value);
	void copyToBuffer(char*& dataPtr) const;
	void writeToFile(std::fstream& outFile) const;
	WebType& operator=(uint32_t value);
	WebType& operator=(const WebType& value) = default;
	operator uint32_t() const;

	static void copyToBuffer(const uint32_t& value, char*& buffer);
	static void writeToFile(const uint32_t& value, std::fstream& outFile);
};
