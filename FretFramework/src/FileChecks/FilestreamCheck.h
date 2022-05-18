#pragma once
#include <fstream>
#include <filesystem>
namespace FilestreamCheck
{
	class InvalidFileException : public std::runtime_error
	{
	public:
		InvalidFileException(const std::filesystem::path& filepath) throw();
	};

	/*
	* Generates an fstream
	* @throws InvalidFileException Thrown if filename can not be reached
	*/
	std::fstream getFileStream(const std::filesystem::path& filepath, const std::ios_base::openmode mode);
	/*
	* Generates an basic FILE pointer
	* @throws InvalidFileException Thrown if filename can not be reached
	*/
	FILE* getFile(const std::filesystem::path& filepath, const wchar_t* mode);
};

