#include "SongCache/SongCache.h"
#include "Song/Song.h"
#include "FileChecks/FilestreamCheck.h"
#include <list>
#include <iostream>

void scan();
void fullScan();

// Cache saving is not yet implemented so no path is given
SongCache g_songCache{ std::filesystem::path() };
bool g_benchmark = false;

int main()
{
	const char* const localeName = ".UTF8";
	std::setlocale(LC_ALL, localeName);
	std::locale::global(std::locale(localeName));
	while (true)
	{
		std::cout << "Drag and drop a file to the console (type \"loop\" to toggle a loop benchmark, \"scan\" for chart validation, or \"quit\" to exit the app)\n";
		std::cout << "Loop Benchmark: " << (g_benchmark ? "Enabled" : "Disabled") << "\nInput: ";
		std::string filename;
		std::getline(std::cin, filename);
		std::transform(filename.begin(), filename.end(), filename.begin(),
			[](unsigned char c) { return std::tolower(c); });

		if (filename == "quit")
			break;

		std::cout << std::endl;

		if (filename == "loop")
		{
			g_benchmark = !g_benchmark;
			continue;
		}

		try
		{
			if (filename == "scan")
				scan();
			else
			{
				if (filename[0] == '\"')
					filename = filename.substr(1, filename.length() - 2);

				std::filesystem::path path(filename);
				if (!g_benchmark)
				{
					Song song(path);
					song.save();
					std::getline(std::cin, filename);
				}
				else
				{
					long long total = 0;
					int i = 0;
					for (; i < 10000 && total < 60000000; ++i)
					{
						Song song;
						auto t1 = std::chrono::high_resolution_clock::now();
						song.load(path);
						auto t2 = std::chrono::high_resolution_clock::now();
						long long count = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
						std::cout << "Load test " << i + 1 << " took " << count / 1000.0 << " milliseconds\n";
						total += count;
					}
					std::cout << "Load test took " << total / 1000 << " milliseconds\n";
					std::cout << "Each load took " << total / (i * 1000.0f) << " milliseconds on average\n";
					std::cout << std::endl;
				}
			}
		}
		catch (std::runtime_error err)
		{
			std::cout << err.what() << std::endl;
		}
		std::cout << std::endl;
	}

	SongBase::deleteTracks();
	return 0;
}

void scan()
{
	while (true)
	{
		std::cout << "Scan Mode - Drag and drop a file to the console (type \"loop\" to toggle a loop benchmark, \"full\" for a directory scan test, or \"quit\" to exit to main loop)\n";
		std::cout << "Loop Benchmark: " << (g_benchmark ? "Enabled" : "Disabled") << "\nInput: ";
		std::string filename;
		std::getline(std::cin, filename);
		std::transform(filename.begin(), filename.end(), filename.begin(),
			[](unsigned char c) { return std::tolower(c); });

		if (filename == "loop")
		{
			g_benchmark = !g_benchmark;
			std::cout << std::endl;
			continue;
		}

		if (filename == "full")
		{
			std::cout << std::endl;
			fullScan();
			break;
		}

		if (filename == "quit")
			break;

		std::cout << std::endl;

		if (filename[0] == '\"')
			filename = filename.substr(1, filename.length() - 2);

		std::filesystem::path path(filename);
		if (!g_benchmark)
		{
			SongScan song;
			song.scan(path);
			song.getHash().display();
		}
		else
		{
			long long total = 0;
			int i = 0;
			for (; i < 10000 && total < 60000000; ++i)
			{
				SongScan song;
				auto t1 = std::chrono::high_resolution_clock::now();
				song.scan(path);
				song.wait();
				auto t2 = std::chrono::high_resolution_clock::now();
				long long count = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
				std::cout << "Scan test " << i + 1 << " took " << count / 1000.0 << " milliseconds\n";
				total += count;
			}
			std::cout << "Scan test took " << total / 1000 << " milliseconds\n";
			std::cout << "Each scan took " << total / (i * 1000.0f) << " milliseconds on average\n";
			std::cout << std::endl;
		}
	}
}

void fullScan()
{
	while (true)
	{
		std::vector<std::filesystem::path> directories;
		std::cout << "Full Scan Mode - Drag and drop a directory to the console (type \"loop\" to toggle a loop benchmark, \"multi\" to input multiple directories, or \"quit\" to exit to main loop)\n";
		std::cout << "Loop Benchmark: " << (g_benchmark ? "Enabled\n" : "Disabled\n");
		std::cout << "Duplicates " << (g_songCache.areDuplicatesAllowed() ? "allowed" : "disallowed") << " (\"toggle\" to change this setting)\n";
		std::cout << "Input: ";
		std::string filename;
		std::getline(std::cin, filename);
		if (filename[0] == '\"')
			filename = filename.substr(1, filename.length() - 2);

		if (filename == "quit")
			break;

		if (filename == "loop")
		{
			g_benchmark = !g_benchmark;
			std::cout << std::endl;
			continue;
		}

		if (filename == "toggle")
		{
			g_songCache.toggleDuplicates();
			std::cout << std::endl;
			continue;
		}

		if (filename == "multi")
		{
			while (true)
			{
				std::cout << "Multi-Directory Scan Mode - Drag and drop a directory to the console (type \"loop\" to toggle a loop benchmark, \"done\" when all directories are added, or \"quit\" to exit to main loop)\n";
				std::cout << "Loop Benchmark: " << (g_benchmark ? "Enabled\n" : "Disabled\n");
				std::cout << "Duplicates " << (g_songCache.areDuplicatesAllowed() ? "allowed" : "disallowed") << " (\"toggle\" to change this setting)\n";
				std::cout << "Input: ";
				std::string filename;
				std::getline(std::cin, filename);

				if (filename[0] == '\"')
					filename = filename.substr(1, filename.length() - 2);

				if (filename == "quit")
					return;

				if (filename == "loop")
				{
					g_benchmark = !g_benchmark;
					std::cout << std::endl;
					continue;
				}

				if (filename == "toggle")
				{
					g_songCache.toggleDuplicates();
					std::cout << std::endl;
					continue;
				}

				if (filename == "done")
					break;

				directories.push_back(filename);
				std::cout << std::endl;
			}
		}
		else
			directories.push_back(filename);

		std::cout << std::endl;

		if (!g_benchmark)
		{
			auto t1 = std::chrono::high_resolution_clock::now();
			g_songCache.scan(directories);
			auto t2 = std::chrono::high_resolution_clock::now();

			long long count = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
			std::cout << "Full scan complete - # of songs: " << g_songCache.getNumSongs() << std::endl;
			std::cout << "Time taken: " << count / 1000 << " milliseconds\n";
			std::cout << std::endl;
		}
		else
		{
			long long total = 0;
			int i = 0;
			for (; i < 10000 && total < 60000000; ++i)
			{
				auto t1 = std::chrono::high_resolution_clock::now();
				g_songCache.scan(directories);
				auto t2 = std::chrono::high_resolution_clock::now();

				long long count = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
				std::cout << "Full scan " << i + 1 << " took " << count / 1000.0 << " milliseconds\n";
				total += count;
			}

			std::cout << "Full Scan test took " << total / 1000 << " milliseconds\n";
			std::cout << "Each full scan took " << total / (i * 1000.0f) << " milliseconds on average\n";
			std::cout << std::endl;
		}
	}
}
