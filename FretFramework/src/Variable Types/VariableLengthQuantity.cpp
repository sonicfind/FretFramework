#include "VariableLengthQuantity.h"
#include <string>

VariableLengthQuantity::InvalidIntegerException::InvalidIntegerException(uint32_t value)
	: std::runtime_error("Integer value cannot exceed 134217728 (value: " + std::to_string(value) + ")") {}

uint32_t VariableLengthQuantity::read(const unsigned char*& dataPtr)
{
	uint32_t value = 0;
	while (*dataPtr >= 128)
	{
		value |= *dataPtr++ & 127;
		value <<= 7;
	}
	value |= *dataPtr++ & 127;
	return value;
}

void VariableLengthQuantity::write(uint32_t value, std::fstream& outFile)
{
	if (value & (15 << 28))
		throw InvalidIntegerException(value);

	char writeBuffer[4];
	char* bufferStart = writeBuffer + 3;
	size_t bufferSize = 1;

	*bufferStart = value & 127;
	for (value >>= 7; value > 0; value >>= 7)
	{
		*--bufferStart = (value & 127) | 128;
		++bufferSize;
	}
	outFile.write(bufferStart, bufferSize);
}
