#pragma once
#include <stdint.h>
#include <stdexcept>
#include <fstream>

class VariableLengthQuantity
{
	class InvalidIntegerException : public std::runtime_error
	{
	public:
		InvalidIntegerException(uint32_t value);
	};

	uint32_t m_value;
	static char s_writeBuffer[4];
	static char* s_bufferStart;
	static size_t s_bufferSize;

	void setBuffer() const;
public:
	VariableLengthQuantity(const unsigned char*& dataPtr);
	VariableLengthQuantity(uint32_t value);
	void copyToBuffer(char*& dataPtr) const;
	void writeToFile(std::fstream& outFile) const;
	VariableLengthQuantity& operator=(uint32_t value);
	VariableLengthQuantity& operator=(const VariableLengthQuantity& value) = default;
	operator uint32_t() const;

	static void discard(const unsigned char*& dataPtr);
};
