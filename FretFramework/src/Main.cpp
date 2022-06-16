#include "Song/Song.h"
#include "FileChecks/FilestreamCheck.h"
#include <iostream>

void benchmark();

void scan();
void scanBenchmark();
void fullScan();
int scanStep(const std::filesystem::path& path);

int main()
{
	const char* const localeName = ".UTF8";
	std::setlocale(LC_ALL, localeName);
	std::locale::global(std::locale(localeName));
	while (true)
	{
		std::cout << "Drag and drop a file to the console (type \"test\" for a loop import benchmark, \"scan\" for chart validation, or \"quit\" to exit the app): ";
		std::string filename;
		std::getline(std::cin, filename);
		std::transform(filename.begin(), filename.end(), filename.begin(),
			[](unsigned char c) { return std::tolower(c); });

		if (filename == "quit")
			break;

		try
		{
			if (filename == "test")
				benchmark();
			else if (filename == "scan")
				scan();
			else
			{
				if (filename[0] == '\"')
					filename = filename.substr(1, filename.length() - 2);

				std::filesystem::path path(filename);
				Song song(path);
				song.save();
				std::getline(std::cin, filename);
			}
		}
		catch (std::runtime_error err)
		{
			std::cout << err.what() << std::endl;
		}
	}
	Song::deleteTracks();
	return 0;
}

void benchmark()
{
	std::cout << "Benchmark Mode - Drag and drop a file to the console (type \"quit\" to exit to main loop): ";
	std::string filename;
	std::getline(std::cin, filename);
	if (filename[0] == '\"')
		filename = filename.substr(1, filename.length() - 2);

	if (filename == "quit")
		return;

	std::filesystem::path path = filename;
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

void scan()
{
	std::cout << "Scan Mode - Drag and drop a file to the console (type \"test\" for a loop scan benchmark, \"full\" for a directory scan test, or \"quit\" to exit to main loop): ";
	std::string filename;
	std::getline(std::cin, filename);
	std::transform(filename.begin(), filename.end(), filename.begin(),
		[](unsigned char c) { return std::tolower(c); });

	if (filename == "test")
	{
		scanBenchmark();
		return;
	}

	if (filename == "full")
	{
		fullScan();
		return;
	}

	if (filename == "quit")
		return;

	if (filename[0] == '\"')
		filename = filename.substr(1, filename.length() - 2);

	std::filesystem::path path(filename);
	Song song;
	song.scan(path);
}

void scanBenchmark()
{
	std::cout << "Scan Benchmark Mode - Drag and drop a file to the console (type \"quit\" to exit to main loop): ";
	std::string filename;
	std::getline(std::cin, filename);
	if (filename[0] == '\"')
		filename = filename.substr(1, filename.length() - 2);

	if (filename == "quit")
		return;

	std::filesystem::path path = filename;
	long long total = 0;
	int i = 0;
	for (; i < 10000 && total < 60000000; ++i)
	{
		Song song;
		auto t1 = std::chrono::high_resolution_clock::now();
		song.scan(path);
		auto t2 = std::chrono::high_resolution_clock::now();
		long long count = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
		std::cout << "import test " << i + 1 << " took " << count / 1000.0 << " milliseconds\n";
		total += count;
	}
	std::cout << "import test took " << total / 1000 << " milliseconds\n";
	std::cout << "each import took " << total / (i * 1000.0f) << " milliseconds on average\n";
}

void fullScan()
{
	std::cout << "Full Scan Mode - Drag and drop a directory to the console (type \"quit\" to exit to main loop): ";
	std::string filename;
	std::getline(std::cin, filename);
	if (filename[0] == '\"')
		filename = filename.substr(1, filename.length() - 2);

	if (filename == "quit")
		return;

	auto t1 = std::chrono::high_resolution_clock::now();
	int numSongs = scanStep(filename);
	std::cout << "Full scan complete - # of songs: " << numSongs << std::endl;
	auto t2 = std::chrono::high_resolution_clock::now();
	long long count = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
	std::cout << "Total scan took " << count / 1000 << " milliseconds total\n";
}

int scanStep(const std::filesystem::path& path)
{
	Song song;
	std::filesystem::path dir(path);
	dir += '\\';
	if (song.scan(dir))
	{
		// Do something with the ini data +
		// Generate the file hash +
		// Add song scan data to list
		//std::cout << "Song loaded\n";
		return 1;
	}
	else
	{
		int count = 0;
		for (auto const& file : std::filesystem::directory_iterator(path))
		{
			if (file.is_directory())
				count += scanStep(file);
		}
		return count;
	}
}
