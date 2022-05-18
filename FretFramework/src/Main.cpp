#include "Song/Song.h"
#include "FileChecks/FilestreamCheck.h"
#include <iostream>
int main()
{
	std::cout << "Drag and drop a file to the console (or type \"test\" for a loop import benchmark): ";
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
			std::cout << "Benchmark Mode - Drag and drop a file to the console: ";
			std::getline(std::cin, filename);
			if (filename[0] == '\"')
				filename = filename.substr(1, filename.length() - 2);

			path = filename;

			long long total = 0;
			int i = 0;
			for (; i < 10000 && total < 60000000; ++i)
			{
				Song song;
				auto t1 = std::chrono::high_resolution_clock::now();
				song.load(path);
				auto t2 = std::chrono::high_resolution_clock::now();
				long long count = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
				std::cout << "import test " << i + 1 << " took " << count / 1000.0 << " milliseconds\n";
				total += count;
			}
			std::cout << "import test took " << total / 1000 << " milliseconds\n";
			std::cout << "each import took " << total / (i * 1000.0f) << " milliseconds on average\n";
		}
	}
	catch (std::runtime_error err)
	{
		std::cout << err.what() << std::endl;
	}
	return 0;
}
