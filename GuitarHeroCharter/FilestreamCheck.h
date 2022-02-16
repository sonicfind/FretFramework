#pragma once
#include <fstream>
namespace FilestreamCheck
{
	struct InvalidFileException : public std::exception
	{
		const char* what() const throw ()
		{
			return "The file location provided is invalid";
		}
	};

	/*
	* Generates an fstream
	* @throws InvalidFileException Thrown if filename can not be reached
	*/
	std::fstream getFileStream(const std::string& filename, const std::ios_base::openmode mode);
};

