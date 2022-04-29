#pragma once
#include <exception>
#include <string>
#include <fstream>

namespace BinaryFile
{
	class IncorrectChunkMarkException : public std::runtime_error
	{
	public:
		IncorrectChunkMarkException(uint32_t position, const char* byteOrder, const char* received);
	};

	bool chunkValidation(char(&buffer)[5], const char** expected, int numPossibilities, std::fstream& inFile);
}
