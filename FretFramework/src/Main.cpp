#include "Song/Song.h"
#include "FileChecks/FilestreamCheck.h"
#include <queue>
#include <list>
#include <iostream>

void benchmark();

void scan();
void scanBenchmark();
void fullScan();
void scanStep(const std::filesystem::path& path);

void vec(const std::filesystem::path& chart, const std::filesystem::path& ini, const std::vector<std::filesystem::path>& audioFiles);
void example();

std::list<Song> g_songs;

enum scanStatus
{
	IDLE,
	START,
	WAIT,
	EXIT
};

scanStatus g_scanStatus = scanStatus::IDLE;

struct SongScan
{
	std::list<Song>::iterator song;
	std::filesystem::path iniPath;
	std::filesystem::path chartPath;
	std::vector<std::filesystem::path> audioFiles;
};
std::queue<SongScan> g_songScans;

std::mutex g_mutex;
std::condition_variable g_condition;

int main()
{
	// start scanning thread
	std::thread thr(example);

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
	g_scanStatus = EXIT;
	g_condition.notify_one();
	thr.join();

	Traversal::endHashThread();
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

	if (!g_songs.empty())
		g_songs.clear();

	auto t1 = std::chrono::high_resolution_clock::now();
	scanStep(filename);

	std::unique_lock lk(g_mutex);
	while (!g_songScans.empty())
		g_condition.wait(lk);
	
	for (auto iter = g_songs.begin(); iter != g_songs.end();)
		if (iter->isValid())
			++iter;
		else
			g_songs.erase(iter++);

	Traversal::waitForHashThread();

	auto t2 = std::chrono::high_resolution_clock::now();

	long long count = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
	std::cout << "Full scan complete - # of songs: " << g_songs.size() << std::endl;
	std::cout << "Total scan took " << count / 1000 << " milliseconds total\n";
}

void scanStep(const std::filesystem::path& path)
{
	std::vector<std::filesystem::directory_entry> entries;
	std::vector<std::filesystem::path> audioFiles;
	std::filesystem::path iniPath;

	// In order of precendence
	// .bch
	// .cht
	// .mid
	// .chart
	std::filesystem::path chartPaths[4];

	for (const auto& file : std::filesystem::directory_iterator(path))
	{
		if (file.is_directory())
			entries.push_back(file);
		else
		{
			const std::filesystem::path filename = file.path().filename();
			if (filename == "notes.bch")
				chartPaths[0] = file.path();
			else if (filename == "notes.cht")
				chartPaths[1] = file.path();
			else if (filename == "notes.mid")
				chartPaths[2] = file.path();
			else if (filename == "notes.chart")
				chartPaths[3] = file.path();
			else if (filename == "song.ini")
				iniPath = file.path();
			else if ((filename.extension() == ".ogg" || filename.extension() == ".wav" || filename.extension() == ".mp3" || filename.extension() == ".opus" || filename.extension() == ".flac") &&
				(filename.stem() == "song" ||
					filename.stem() == "guitar" ||
					filename.stem() == "bass" ||
					filename.stem() == "rhythm" ||
					filename.stem() == "keys" ||
					filename.stem() == "vocals_1" || filename.stem() == "vocals_2" ||
					filename.stem() == "drums_1" || filename.stem() == "drums_2" || filename.stem() == "drums_3" || filename.stem() == "drums_4"))
				audioFiles.push_back(file.path());
		}
	}

	for (int i = 0; i < 4; ++i)
		if (!chartPaths[i].empty() && (!iniPath.empty() || i & 1))
		{
			vec(iniPath, chartPaths[i], audioFiles);
			return;
		}

	for (const auto& dir : entries)
		scanStep(dir);	
}

void vec(const std::filesystem::path& chart, const std::filesystem::path& ini, const std::vector<std::filesystem::path>& audioFiles)
{
	g_songs.emplace_back();
	g_songScans.emplace(--g_songs.end(), chart, ini, audioFiles);
	g_condition.notify_one();
}

void example()
{		
	std::unique_lock lk(g_mutex);
	while (true)
	{
		while (g_scanStatus != EXIT && g_songScans.empty())
			g_condition.wait(lk);

		if (g_scanStatus == EXIT)
			break;

		SongScan scan = g_songScans.front();
		scan.song->scan_full(scan.chartPath, scan.iniPath, scan.audioFiles);
		g_songScans.pop();
		g_condition.notify_one();
	}
}
