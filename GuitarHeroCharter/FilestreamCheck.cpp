#include "FilestreamCheck.h"
FilestreamCheck::InvalidFileException::InvalidFileException() throw()
	: std::runtime_error("The file location provided is invalid")
{
}

std::fstream FilestreamCheck::getFileStream(const std::filesystem::path& filepath, const std::ios_base::openmode mode)
{
	std::fstream filestream(filepath, mode);
	if (filestream.is_open())
		return filestream;
	else
		throw InvalidFileException();
}
