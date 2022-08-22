#include "SongCache/SongCache.h"
#include "FileChecks/FilestreamCheck.h"
#include <list>
#include <iostream>

template <bool bench>
void load(const std::filesystem::path& path)
{
	constexpr int numIterations = bench ? 10000 : 1;
	long long total = 0;
	int i = 0;
	for (; i < numIterations && total < 60000000; ++i)
	{
		Song song(path);
		auto t1 = std::chrono::high_resolution_clock::now();
		song.load();
		auto t2 = std::chrono::high_resolution_clock::now();

		long long count = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
		std::cout << "Load took " << count / 1000.0 << " milliseconds\n";

		if constexpr (bench)
			total += count;
		else
		{
			song.save();
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

			const std::filesystem::path path(filename);
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

				for (const auto& file : std::filesystem::directory_iterator(path.parent_path()))
					if (file.path().filename() == U"song.ini")
					{
						hasIni = true;
						break;
					}
			}
			else
			{
				for (const auto& file : std::filesystem::directory_iterator(path))
				{
					if (file.is_regular_file())
					{
						const std::filesystem::path shortname = file.path().filename();
						if (shortname == U"song.ini")
							hasIni = true;
						else if (shortname == U"notes.bch")
							chartPaths[0] = file.path();
						else if (shortname == U"notes.cht")
							chartPaths[1] = file.path();
						else if (shortname == U"notes.mid" || shortname == U"notes.midi")
							chartPaths[2] = file.path();
						else if (shortname == U"notes.chart")
							chartPaths[3] = file.path();
					}
				}
			}

			for (int pathIndex = 0; pathIndex < 4; ++pathIndex)
				if (!chartPaths[pathIndex].empty() && (hasIni || pathIndex & 1))
				{
					if (!g_benchmark)
					{
						Song song(chartPaths[pathIndex]);
						auto t1 = std::chrono::high_resolution_clock::now();
						song.scan(hasIni);
						auto t2 = std::chrono::high_resolution_clock::now();
						long long count = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
						std::cout << "Scan took " << count / 1000.0 << " milliseconds\n";
						song.displayScanResult();
					}
					else
					{
						long long total = 0;
						int i = 0;
						for (; i < 10000 && total < 60000000; ++i)
						{
							Song song(chartPaths[pathIndex]);
							auto t1 = std::chrono::high_resolution_clock::now();
							song.scan(hasIni);
							auto t2 = std::chrono::high_resolution_clock::now();
							long long count = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
							std::cout << "Scan test " << i + 1 << " took " << count / 1000.0 << " milliseconds\n";
							total += count;
						}
						std::cout << "Scan test took " << total / 1000 << " milliseconds\n";
						std::cout << "Each scan took " << total / (i * 1000.0f) << " milliseconds on average\n";
						std::cout << std::endl;
					}
					break;
				}
				else if (pathIndex == 3)
					std::cout << "Not a valid chart directory" << std::endl;
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
		long long count = g_songCache.scan(directories);
		std::cout << "Full scan " << i + 1 << " took " << count / 1000.0 << " milliseconds\n";

		if constexpr (bench)
			total += count;
	}

	std::cout << "Full Scan test took " << total / 1000 << " milliseconds\n";
	if constexpr (bench)
	{
		std::cout << "# of full scans:    " << i << '\n';
		std::cout << "Each full scan took " << total / (i * 1000.0f) << " milliseconds on average\n";
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
					std::cout << "\"Done\" - start scan\n";
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
