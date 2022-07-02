#include "VariableLengthQuantity.h"
#include <string>

VariableLengthQuantity::InvalidIntegerException::InvalidIntegerException(uint32_t value)
	: std::runtime_error("Integer value cannot exceed 134217728 (value: " + std::to_string(value) + ")") {}

VariableLengthQuantity::VariableLengthQuantity(const unsigned char*& dataPtr)
	: m_value(*dataPtr & 127)
{
	while (*dataPtr >= 128)
	{
		m_value <<= 7;
		m_value |= *++dataPtr & 127;
	}
	++dataPtr;
}

VariableLengthQuantity::VariableLengthQuantity(uint32_t value)
{
	operator=(value);
}

VariableLengthQuantity& VariableLengthQuantity::operator=(uint32_t value)
{
	if (value & (15 << 28))
		throw InvalidIntegerException(value);

	m_value = value;
	return *this;
}

char VariableLengthQuantity::s_writeBuffer[4];
char* VariableLengthQuantity::s_bufferStart;
size_t VariableLengthQuantity::s_bufferSize;
void VariableLengthQuantity::setBuffer() const
{
	s_bufferStart = s_writeBuffer + 3;
	s_bufferSize = 1;

	*s_bufferStart = m_value & 127;
	for (uint32_t cmp = m_value >> 7; cmp > 0; cmp >>= 7)
	{
		*--s_bufferStart = (cmp & 127) | 128;
		++s_bufferSize;
	}
}

void VariableLengthQuantity::copyToBuffer(char*& dataPtr) const
{
	setBuffer();
	memcpy(dataPtr, s_bufferStart, s_bufferSize);
	dataPtr += s_bufferSize;
}

void VariableLengthQuantity::writeToFile(std::fstream& outFile) const
{
	setBuffer();
	outFile.write(s_bufferStart, s_bufferSize);
}

VariableLengthQuantity::operator uint32_t() const
{
	return m_value;
}

void VariableLengthQuantity::discard(const unsigned char*& dataPtr)
{
	while (*dataPtr >= 128)
		++dataPtr;
	++dataPtr;
}
