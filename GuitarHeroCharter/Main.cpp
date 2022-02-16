#include "Chart.h"
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
		std::fstream inFile = FilestreamCheck::getFileStream(filename, std::ios_base::in);
		if (filename.find(".chart") != std::string::npos)
			song = new Chart(filename);

		song->fill(inFile);
		inFile.close();

		filename = song->getFilename();
		song->setFilename(filename + ".test");
		song->save();
		delete song;
	}
	catch (FilestreamCheck::InvalidFileException e)
	{
		std::cout << e.what() << std::endl;
	}
	return 0;
}