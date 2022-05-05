#include "BinaryFileCheck.h"

BinaryFile::IncorrectChunkMarkException::IncorrectChunkMarkException(uint32_t position, const char* byteOrder, const char* received)
	: std::runtime_error("Error at file position " + std::to_string(position - 4) + ": incorrect chunk header mark of " + byteOrder + " (received: " + received + ')') {}

bool BinaryFile::chunkValidation(char(&buffer)[5], const char** expected, int numPossibilities, std::fstream& inFile)
{
	inFile.read(buffer, 4);
	for (int i = 0; i < numPossibilities; ++i)
		if (strstr(buffer, expected[i]))
			return true;
	return false;
}
