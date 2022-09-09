#pragma once
#include <stdint.h>
#include <stdexcept>
#include <fstream>

namespace VariableLengthQuantity
{
	class InvalidIntegerException : public std::runtime_error
	{
	public:
		InvalidIntegerException(uint32_t value);
	};

	uint32_t read(const unsigned char*& dataPtr);
	void write(uint32_t value, std::fstream& outFile);
};
