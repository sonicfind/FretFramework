#include "SongCache.h"
#include <set>

SongCache::SongCache(const std::filesystem::path& cacheLocation)
	: m_location(cacheLocation)
	, m_threadCount(std::thread::hardware_concurrency() >= 4 ? std::thread::hardware_concurrency() / 2 : 1)
{
	m_threads.reserve(m_threadCount);
}

SongCache::~SongCache()
{
	stopThreads();
}

void SongCache::startThreads()
{
	m_status = ACTIVE;
	for (unsigned int i = 0; i < m_threadCount; ++i)
		m_threads.emplace_back(&SongCache::scanThread, this);

	Traversal::startHasher();
}

void SongCache::stopThreads()
{
	if (m_status == ACTIVE)
	{
		m_status = INACTIVE;
		m_condition.notify_all();

		for (unsigned int i = 0; i < m_threadCount; ++i)
			m_threads[i].join();

		m_threads.clear();

		Traversal::stopHasher();
	}
}

void SongCache::clear()
{
	m_category_artistAlbum.clear();
	m_category_title.clear();
	m_category_artist.clear();
	m_category_album.clear();
	m_category_genre.clear();
	m_category_year.clear();
	m_category_charter.clear();
	m_category_playlist.clear();
	for (Song* song : m_songs)
		delete song;
	m_songs.clear();
}

long long SongCache::scan(const std::vector<std::filesystem::path>& baseDirectories)
{
	clear();

	startThreads();

	auto t1 = std::chrono::high_resolution_clock::now();
	if (m_location.empty() || !std::filesystem::exists(m_location))
		for (const std::filesystem::path& directory : baseDirectories)
			scanDirectory(directory);
	finalize();
	auto t2 = std::chrono::high_resolution_clock::now();

	return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

void SongCache::scanDirectory(const std::filesystem::path& directory)
{
	std::vector<std::filesystem::directory_entry> directories;
	bool hasIni = false;

	// In order of precendence
	// .bch
	// .cht
	// .mid
	// .chart
	std::filesystem::path chartPaths[4];

	for (const auto& file : std::filesystem::directory_iterator(directory))
	{
		if (file.is_directory())
			directories.push_back(file);
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
				hasIni = true;
		}
	}

	if (!try_addChart(chartPaths, hasIni))
		for (const auto& dir : directories)
			scanDirectory(dir);
}

bool SongCache::try_addChart(const std::filesystem::path (&chartPaths)[4], bool hasIni)
{
	for (int i = 0; i < 4; ++i)
		if (!chartPaths[i].empty() && (hasIni || i & 1))
		{
			m_scanQueue.push({ new Song (chartPaths[i]), hasIni});
			m_condition.notify_one();
			return true;
		}
	return false;
}

void SongCache::finalize()
{
	stopThreads();

	if (!m_allowDuplicates)
		validateSongList();

	fillCategories();
}

void SongCache::validateSongList()
{
	std::sort(m_songs.begin(), m_songs.end(),
		[](const Song* const first, const Song* const second)
		{
			return first->isHashLessThan(*second); 
		});

	auto endIter = std::unique(m_songs.begin(), m_songs.end(),
		[](const Song* const first, const Song* const second)
		{
			if (*first == *second)
			{
				delete second;
				return true;
			}
			return false;
		});
	m_songs.erase(endIter, m_songs.end());
}

void SongCache::fillCategories()
{
	for (Song* song : m_songs)
	{
		Song::setSortAttribute(SongAttribute::TITLE);
		m_category_title.add(song);
		m_category_artist.add(song);
		m_category_genre.add(song);
		m_category_year.add(song);
		m_category_charter.add(song);

		Song::setSortAttribute(SongAttribute::ALBUM);
		m_category_album.add(song);
		m_category_artistAlbum.add(song);
		
		Song::setSortAttribute(SongAttribute::PLAYLIST);
		m_category_playlist.add(song);
	}
}

void SongCache::scanThread()
{
	std::mutex mutex;
	std::unique_lock lk(mutex);
	while (true)
	{
		if (m_scanQueue.empty() && m_status == ACTIVE)
			m_condition.wait(lk);

		if (m_status == INACTIVE)
			return;

		while (auto opt = m_scanQueue.pop_front())
		{
			ScanQueueNode& scan = opt.value();
			if (scan.song->scan_full(scan.hasIni))
			{
				std::scoped_lock scplk(m_mutex);
				m_songs.push_back(scan.song);
			}
			else
				delete scan.song;
		}
	}
}
