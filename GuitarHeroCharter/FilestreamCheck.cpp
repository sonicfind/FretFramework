#include "FilestreamCheck.h"
std::fstream FilestreamCheck::getFileStream(const std::string& filename, const std::ios_base::openmode mode)
{
	std::fstream filestream(filename, mode);
	if (filestream.is_open())
		return filestream;
	else
		throw InvalidFileException();
}
