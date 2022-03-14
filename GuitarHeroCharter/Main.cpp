#include "Song/Song.h"
#include "FilestreamCheck.h"
#include <iostream>
int main()
{
	std::string filename;
	std::getline(std::cin, filename);
	if (filename[0] == '\"')
		filename = filename.substr(1, filename.length() - 2);

	try
	{
		std::filesystem::path path(filename);
		Song song(path);
		song.save();
	}
	catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	}
	return 0;
}
