#pragma once
#include <stdint.h>
#include <fstream>

namespace WebType
{
	uint32_t read(const unsigned char*& dataPtr);
	const unsigned char* getEndPoint(const unsigned char* dataPtr);

	void copyToBuffer(const uint32_t value, char*& buffer);
	void writeToFile(const uint32_t value, std::fstream& outFile);

	typedef uint32_t WebType_t;
}
