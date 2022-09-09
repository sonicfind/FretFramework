#include "WebType.h"

uint32_t WebType::read(const unsigned char*& dataPtr)
{
	uint32_t value = *dataPtr++;
	if (value >= 254)
	{
		const size_t size = 2 + 2ULL * (value == 255);
		memcpy(&value, dataPtr, size);
		dataPtr += size;
	}
	return value;
}

void WebType::copyToBuffer(const uint32_t value, char*& buffer)
{
	if (value <= 253)
		*buffer++ = (unsigned char)value;
	else
	{
		const bool is32bit = value > UINT16_MAX;
		const size_t size = 2 + 2ULL * is32bit;
		*buffer++ = 254 + is32bit;
		memcpy(buffer, &value, size);
		buffer += size;
	}
}

void WebType::writeToFile(const uint32_t value, std::fstream& outFile)
{
	if (value <= 253)
		outFile.put((char)value);
	else
	{
		const bool is32bit = value > UINT16_MAX;
		outFile.put((char)254 + is32bit);
		outFile.write((char*)&value, 2 + 2ULL * is32bit);
	}
}
