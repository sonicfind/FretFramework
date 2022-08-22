#include "SongCache.h"
#include <set>

// Cache saving is not yet implemented so no path is given
SongCache g_songCache{ std::filesystem::path() };

SongCache::SongCache(const std::filesystem::path& cacheLocation) : m_location(cacheLocation) {}

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
	m_songs.clear();
}

void SongCache::stopScan()
{
	g_threadedQueuePool.waitUntilFinished();
	finalize();
}

void SongCache::finalize()
{
	if (!m_allowDuplicates)
		validateSongList();

	fillCategories();
}

void SongCache::validateSongList()
{
	std::sort(m_songs.begin(), m_songs.end(),
		[](const std::unique_ptr<Song>& first, const std::unique_ptr<Song>& second)
		{
			return first->isHashLessThan(*second);
		});

	auto endIter = std::unique(m_songs.begin(), m_songs.end(),
		[](const std::unique_ptr<Song>& first, const std::unique_ptr<Song>& second)
		{
			return *first == *second;
		});
	m_songs.erase(endIter, m_songs.end());
}

void SongCache::fillCategories()
{
	for (std::unique_ptr<Song>& song : m_songs)
	{
		Song* const ptr = song.get();
		Song::setSortAttribute(SongAttribute::TITLE);
		m_category_title.add(ptr);
		m_category_artist.add(ptr);
		m_category_genre.add(ptr);
		m_category_year.add(ptr);
		m_category_charter.add(ptr);

		Song::setSortAttribute(SongAttribute::ALBUM);
		m_category_album.add(ptr);
		m_category_artistAlbum.add(ptr);

		Song::setSortAttribute(SongAttribute::PLAYLIST);
		m_category_playlist.add(ptr);
	}
}

long long SongCache::scan(const std::vector<std::filesystem::path>& baseDirectories)
{
	clear();
	auto t1 = std::chrono::high_resolution_clock::now();

	if (m_location.empty() || !std::filesystem::exists(m_location))
		for (const std::filesystem::path& directory : baseDirectories)
			scanDirectory(directory);

	stopScan();
	auto t2 = std::chrono::high_resolution_clock::now();

	return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

void SongCache::scanDirectory(const std::filesystem::path& directory)
{
	static const std::filesystem::path chartnames[6] =
	{
		U"notes.chart",
		U"notes.mid",
		U"notes.midi",
		U"notes.bch",
		U"notes.cht",
		U"song.ini"
	};

	std::vector<std::filesystem::path> directories;
	bool hasIni = false;

	// In order of precendence
	// .bch
	// .cht
	// .mid
	// .chart
	std::filesystem::path chartPaths[4];

	for (const auto& file : std::filesystem::directory_iterator(directory))
	{
		const std::filesystem::path& path = file.path();
		if (file.is_directory())
			directories.emplace_back(path);
		else
		{
			const std::filesystem::path filename = path.filename();
			if (filename == chartnames[0])
				chartPaths[3] = path;
			else if (filename == chartnames[1] || filename == chartnames[2])
				chartPaths[2] = path;
			else if (filename == chartnames[3])
				chartPaths[0] = path;
			else if (filename == chartnames[4])
				chartPaths[1] = path;
			else if (filename == chartnames[5])
				hasIni = true;
		}
	}

	if (!try_addChart(chartPaths, hasIni))
		for (const auto& dir : directories)
			scanDirectory(dir);
}

bool SongCache::try_addChart(const std::filesystem::path(&chartPaths)[4], bool hasIni)
{
	for (int i = 0; i < 4; ++i)
		if (!chartPaths[i].empty() && (hasIni || i & 1))
		{
			g_threadedQueuePool.add(std::make_unique<ScanQueueNode>(chartPaths[i], hasIni));
			return true;
		}
	return false;
}

void SongCache::push(std::unique_ptr<Song>& song)
{
	m_mutex.lock();
	m_songs.emplace_back(std::move(song));
	m_mutex.unlock();
}

void ScanQueueNode::process() const noexcept
{
	auto song = std::make_unique<Song>(m_filepath);
	if (song->scan(m_hasIni))
		g_songCache.push(song);
}
