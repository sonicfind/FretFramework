#include "FilestreamCheck.h"
FilestreamCheck::InvalidFileException::InvalidFileException(const std::filesystem::path& filepath) throw()
	: std::runtime_error("Error: " + filepath.string() + " could not be located")
{
}

std::fstream FilestreamCheck::getFileStream(const std::filesystem::path& filepath, const std::ios_base::openmode mode)
{
	std::fstream filestream(filepath, mode);
	if (filestream.is_open())
		return filestream;
	else
		throw InvalidFileException(filepath);
}
