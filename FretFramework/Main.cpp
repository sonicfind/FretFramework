#include "Song/Song.h"
#include "FilestreamCheck.h"
#include <iostream>
int main()
{
	std::string filename;
	std::getline(std::cin, filename);
	if (filename[0] == '\"')
		filename = filename.substr(1, filename.length() - 2);

	std::filesystem::path path(filename);

	try
	{
		Song song(path);
		song.save();
	}
	catch (EndofFileException EoF)
	{
		std::cout << "Error: The end of file " << path.filename() << " was reached unexpectedly" << std::endl;
	}
	catch (std::runtime_error err)
	{
		std::cout << err.what() << std::endl;
	}
	return 0;
}
