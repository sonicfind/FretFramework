#include "Song\Chart\Chart.h"
#include "FilestreamCheck.h"
#include <iostream>
int main()
{
	std::string filename;
	std::getline(std::cin, filename);
	if (filename[0] == '\"')
		filename = filename.substr(1, filename.length() - 2);

	Song* song = nullptr;
	try
	{
		std::filesystem::path path(filename);
		if (path.extension() == ".chart")
			song = new Chart(path);

		song->fillFromFile();
		song->save();
		delete song;
	}
	catch (FilestreamCheck::InvalidFileException e)
	{
		std::cout << e.what() << std::endl;
	}
	return 0;
}