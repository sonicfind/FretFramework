#include "SongCache/SongCache.h"
#include "Song/Song.h"
#include "FileChecks/FilestreamCheck.h"
#include <list>
#include <iostream>

Song g_song;

template <bool bench>
void load(const std::filesystem::path& path)
{
	constexpr int numIterations = bench ? 10000 : 1;
	long long total = 0;
	int i = 0;
	for (; i < numIterations && total < 60000000; ++i)
	{
		auto t1 = std::chrono::high_resolution_clock::now();
		g_song.loadFrom(path);
		auto t2 = std::chrono::high_resolution_clock::now();

		long long count = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

		if constexpr (bench)
			total += count;
		else
		{
			std::cout << "Load took " << count / 1000.0 << " milliseconds\n";

			g_song.save();
			std::string discard;
			std::getline(std::cin, discard);
		}
	}

	if constexpr (bench)
	{
		std::cout << "Load test took " << total / 1000 << " milliseconds\n";
		std::cout << "# of loads:    " << i << '\n';
		std::cout << "Each load took " << total / (i * 1000.0f) << " milliseconds on average\n";
		std::cout << std::endl;
	}
}

void directoryScanPrompt();

bool g_benchmark = false;

int main()
{
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
				directoryScanPrompt();
			else
			{
				if (filename[0] == '\"')
					filename = filename.substr(1, filename.length() - 2);

				if (!g_benchmark)
					load<false>(std::filesystem::path(filename));
				else
					load<true>(std::filesystem::path(filename));
			}
		}
		catch (std::runtime_error err)
		{
			std::cout << err.what() << std::endl;
		}
		std::cout << std::endl;
	}
	return 0;
}

void directoryScanPrompt()
{
	typedef std::pair<std::filesystem::path, DriveType> SongDirectory;

	static constexpr auto runScan = [](const std::vector<SongDirectory>& directories)
	{
		SongCache::clear();
		auto t1 = std::chrono::high_resolution_clock::now();
		for (auto& directory : directories)
		{
			if (directory.second == SSD)
				SongCache::scanDirectory<SSD>(directory.first);
			else
				SongCache::scanDirectory<HDD>(directory.first);
		}

		TaskQueue::waitForCompletedTasks();
		SongCache::finalize();
		auto t2 = std::chrono::high_resolution_clock::now();

		std::cout << "Scan took   " << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1000 << " milliseconds\n";
		if (SongCache::getNumSongs() > 1)
			std::cout << "# of songs: " << SongCache::getNumSongs() << std::endl;
		else if (SongCache::getNumSongs() == 1)
			SongCache::displayResultOfFirstSong();
		else
			std::cout << "No songs found" << std::endl;
		std::cout << std::endl;
	};

	while (true)
	{
		try
		{
			std::vector<SongDirectory> directories;
			while (true)
			{
				std::cout << "Recursive Directory Scan Mode - Drag and drop a directory to the console\n";
				if (!directories.empty())
				{
					for (const auto& dir : directories)
						std::cout << "Directory: " << dir.first << '\n';
					std::cout << "\n\"Done\" - start scan\n";
				}
				std::cout << "\"Dupe\" - toggle allowing duplicate songs [" << (SongCache::areDuplicatesAllowed() ? "allowed]\n" : "disallowed]\n");
				std::cout << "\"Quit\" - exit to main\n";
				std::cout << "Input: ";

				std::string filename;
				std::getline(std::cin, filename);
				if (filename[0] == '\"')
					filename = filename.substr(1, filename.length() - 2);

				if (filename.size() == 4)
				{
					std::transform(filename.begin(), filename.end(), filename.begin(),
						[](unsigned char c) { return std::tolower(c); });

					if (filename == "done")
					{
						std::cout << std::endl;
						if (directories.empty())
							continue;
						break;
					}
					
					if (filename == "quit")
						return;

					if (filename == "dupe")
					{
						SongCache::toggleDuplicates();
						std::cout << std::endl;
						continue;
					}
				}

				if (std::filesystem::exists(filename))
				{
					std::cout << std::endl;
					std::string discard;
					while (true)
					{
						std::cout << "Enter Drive Type\nS - SSD\nH - HDD\nQ - Don't add\n";
						std::cout << "Input: ";
						std::getline(std::cin, discard);
						switch (discard[0])
						{
						case 's':
						case 'S':
							directories.push_back({ filename, SSD });
							goto NextLoop;
						case 'h':
						case 'H':
							directories.push_back({ filename, HDD });
							__fallthrough;
						case 'q':
						case 'Q':
							goto NextLoop;
						}
					}
				}

			NextLoop:
				std::cout << std::endl;
			}

			runScan(directories);
		}
		catch (std::runtime_error err)
		{
			std::cout << err.what() << std::endl;
		}
	}
}
