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

public:
	static uint32_t read(const unsigned char*& dataPtr);
	static void write(uint32_t value, std::fstream& outFile);
};
