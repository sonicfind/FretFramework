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

FILE* FilestreamCheck::getFile(const std::filesystem::path& filepath, const wchar_t* mode)
{
	FILE* file;
	if (_wfopen_s(&file, filepath.c_str(), mode) == 0 && file)
		return file;
	else
		throw InvalidFileException(filepath);
}
