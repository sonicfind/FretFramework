#include "FilestreamCheck.h"
std::fstream FilestreamCheck::getFileStream(const std::filesystem::path& filepath, const std::ios_base::openmode mode)
{
	std::fstream filestream(filepath, mode);
	if (filestream.is_open())
		return filestream;
	else
		throw InvalidFileException();
}
