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
		SongEntry entry(path);
		entry.load_Ini();
		auto t1 = std::chrono::high_resolution_clock::now();
		g_song.load(&entry);
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

		Song::clearTracks();
	}

	if constexpr (bench)
	{
		std::cout << "Load test took " << total / 1000 << " milliseconds\n";
		std::cout << "# of loads:    " << i << '\n';
		std::cout << "Each load took " << total / (i * 1000.0f) << " milliseconds on average\n";
		std::cout << std::endl;
	}
}

void scanPrompt();
void fullScanPrompt();

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
				scanPrompt();
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

void scanPrompt()
{
	static const std::filesystem::path NAME_BCH(U"notes.bch");
	static const std::filesystem::path NAME_CHT(U"notes.cht");
	static const std::filesystem::path NAMES_MIDI[2] = { U"notes.mid", U"notes.midi" };
	static const std::filesystem::path NAME_CHART(U"notes.chart");
	static const std::filesystem::path NAME_INI(U"song.ini");

	while (true)
	{
		try
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
				fullScanPrompt();
				break;
			}

			if (filename == "quit")
				break;

			std::cout << std::endl;

			if (filename[0] == '\"')
				filename = filename.substr(1, filename.length() - 2);

			std::filesystem::path chartPaths[4];
			bool hasIni = false;

			std::filesystem::path path(filename);
			if (std::filesystem::is_regular_file(path))
			{
				const std::filesystem::path shortname = path.filename();
				if (shortname == U"notes.bch")
					chartPaths[0] = path;
				else if (shortname == U"notes.cht")
					chartPaths[1] = path;
				else if (shortname == U"notes.mid" || shortname == U"notes.midi")
					chartPaths[2] = path;
				else if (shortname == U"notes.chart")
					chartPaths[3] = path;

				hasIni = std::filesystem::exists(path.replace_filename(U"song.ini"));
			}
			else
			{
				for (const auto& file : std::filesystem::directory_iterator(path))
				{
					if (file.is_regular_file())
					{
						const std::filesystem::path shortname = file.path().filename();
						if (filename == NAME_CHART)
							chartPaths[3] = path;
						else if (filename == NAMES_MIDI[0] || filename == NAMES_MIDI[1])
							chartPaths[2] = path;
						else if (filename == NAME_BCH)
							chartPaths[0] = path;
						else if (filename == NAME_CHT)
							chartPaths[1] = path;
						else if (filename == NAME_INI)
							hasIni = true;
					}
				}
			}

			for (int pathIndex = 0; pathIndex < 4; ++pathIndex)
			{
				if (!chartPaths[pathIndex].empty())
				{
					const bool iniRequired = pathIndex == 0 || pathIndex == 2;
					long long total = 0;
					int i = 0;
					const int maxCount = g_benchmark ? 10000 : 1;
					for (; i < maxCount && total < 60000000; ++i)
					{
						SongEntry song(chartPaths[pathIndex]);
						auto t1 = std::chrono::high_resolution_clock::now();
						if (!song.scan(hasIni, iniRequired))
						{
							std::cout << "Scan failed\n";
							goto LeaveLoop;
						}
						auto t2 = std::chrono::high_resolution_clock::now();
						long long count = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

						if (!g_benchmark)
						{
							std::cout << "Scan took " << count / 1000.0 << " milliseconds\n";
							song.displayScanResult();
						}
						else
							total += count;
					}

					if (g_benchmark)
					{
						std::cout << "Scan test took " << total / 1000 << " milliseconds\n";
						std::cout << "Each scan took " << total / (i * 1000.0f) << " milliseconds on average\n";
						std::cout << std::endl;
					}

				LeaveLoop:
					break;
				}
				else if (pathIndex == 3)
					std::cout << "Not a valid chart directory" << std::endl;
			}
		}
		catch (std::runtime_error err)
		{
			std::cout << err.what() << std::endl;
		}
		std::cout << std::endl;
	}
}

template <bool bench>
void runFullScan(const std::vector<std::filesystem::path>& directories)
{
	constexpr int numIterations = bench ? 1000 : 1;
	long long total = 0;
	int i = 0;

	for (; i < numIterations && total < 60000000; ++i)
	{
		g_songCache.clear();
		auto t1 = std::chrono::high_resolution_clock::now();
		for (auto& directory : directories)
			SongCache::scanDirectory(directory);

		TaskQueue::waitForCompletedTasks();
		g_songCache.finalize();
		auto t2 = std::chrono::high_resolution_clock::now();

		total += std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
	}

	if constexpr (bench)
	{
		std::cout << "Full Scan test took " << total / 1000 << " milliseconds\n";
		std::cout << "# of full scans:    " << i << '\n';
		std::cout << "Each full scan took " << total / (i * 1000.0f) << " milliseconds on average\n";
	}
	else
	{
		std::cout << "Full Scan took " << total / 1000 << " milliseconds\n";
		std::cout << "# of songs:    " << g_songCache.getNumSongs() << std::endl;
	}
	std::cout << std::endl;
}

void fullScanPrompt()
{
	while (true)
	{
		try
		{
			std::vector<std::filesystem::path> directories;
			while (true)
			{
				std::cout << "Full Scan Mode - Drag and drop a directory to the console\n";
				if (!directories.empty())
				{
					for (const auto& dir : directories)
						std::cout << "Directory: " << dir << '\n';
					std::cout << "\n\"Done\" - start scan\n";
				}
				std::cout << "\"Loop\" - toggle looped benchmarking [" << (g_benchmark ? "Enabled]\n" : "Disabled]\n");
				std::cout << "\"Dupe\" - toggle allowing duplicate songs [" << (g_songCache.areDuplicatesAllowed() ? "allowed]\n" : "disallowed]\n");
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

					if (filename == "loop")
					{
						g_benchmark = !g_benchmark;
						std::cout << std::endl;
						continue;
					}

					if (filename == "dupe")
					{
						g_songCache.toggleDuplicates();
						std::cout << std::endl;
						continue;
					}
				}

				directories.push_back(filename);
				std::cout << std::endl;
			}

			if (g_benchmark)
				runFullScan<true>(directories);
			else
				runFullScan<false>(directories);
		}
		catch (std::runtime_error err)
		{
			std::cout << err.what() << std::endl;
		}
	}
}
