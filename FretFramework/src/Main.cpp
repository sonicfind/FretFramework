#include "Song/Song.h"
#include "FilestreamCheck.h"
#include <iostream>
int main()
{
	std::string filename;
	std::getline(std::cin, filename);
	std::transform(filename.begin(), filename.end(), filename.begin(),
		[](unsigned char c) { return std::tolower(c); });
	bool doTest = filename == "test";
	std::filesystem::path path;
	try
	{
		if (!doTest)
		{
			if (filename[0] == '\"')
				filename = filename.substr(1, filename.length() - 2);

			path = filename;
			Song song(path);
			song.save();
		}
		else
		{
			std::getline(std::cin, filename);
			if (filename[0] == '\"')
				filename = filename.substr(1, filename.length() - 2);

			path = filename;

			long long total = 0;
			int i = 0;
			for (; i < 10000 && total < 60000000; ++i)
			{
				auto t1 = std::chrono::high_resolution_clock::now();
				Song song(path);
				auto t2 = std::chrono::high_resolution_clock::now();
				long long count = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
				std::cout << "import test " << i + 1 << " took " << count / 1000.0 << " milliseconds\n";
				total += count;
			}
			std::cout << "import test took " << total / 1000 << " milliseconds\n";
			std::cout << "each import took " << total / double(i * 1000) << " milliseconds on average\n";
		}
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
