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

void SongCache::finalize()
{
	if (!m_allowDuplicates)
		removeDuplicates();

	fillCategories();
}

void SongCache::removeDuplicates()
{
	auto endIter = std::unique(m_songs.begin(), m_songs.end(),
		[](const std::unique_ptr<Song>& first, const std::unique_ptr<Song>& second)
		{
			return first->areHashesEqual(*second);
		});
	m_songs.erase(endIter, m_songs.end());
}

void SongCache::fillCategories()
{
	for (std::unique_ptr<Song>& ptr : m_songs)
	{
		Song* const song = ptr.get();
		song->setBaseModifiers();

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

long long SongCache::scan(const std::vector<std::filesystem::path>& baseDirectories)
{
	clear();
	auto t1 = std::chrono::high_resolution_clock::now();

	if (m_location.empty() || !std::filesystem::exists(m_location))
		for (auto& directory : baseDirectories)
			TaskQueue::addTask(std::make_unique<Task_SongScan>(directory));

	TaskQueue::waitForCompletedTasks();
	finalize();
	auto t2 = std::chrono::high_resolution_clock::now();

	return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

void SongCache::push(std::unique_ptr<Song>& song)
{
	std::scoped_lock lock(m_mutex);
	auto iter = std::lower_bound(m_songs.begin(), m_songs.end(), song,
		[](const std::unique_ptr<Song>& first, const std::unique_ptr<Song>& second)
		{
			return first->isHashLessThan(*second);
		});

	m_songs.emplace(iter, std::move(song));
}

void Task_SongScan::process() const noexcept
{
	static const std::filesystem::path validFiles[] =
	{
		U"notes.bch",
		U"notes.cht",
		U"notes.mid",
		U"notes.midi",
		U"notes.chart",
		U"song.ini"
	};

	try
	{
		std::filesystem::path chartPaths[4];
		bool hasIni = false;

		std::vector<std::filesystem::path> directories;
		for (const auto& file : std::filesystem::directory_iterator(m_baseDirectory))
		{
			if (file.is_directory())
				directories.emplace_back(file.path());
			else
			{
				const std::filesystem::path& path = file.path();
				const std::u32string filename = path.filename().u32string();
				if (filename == validFiles[4])
					chartPaths[3] = path;
				else if (filename == validFiles[2] || filename == validFiles[3])
					chartPaths[2] = path;
				else if (filename == validFiles[0])
					chartPaths[0] = path;
				else if (filename == validFiles[1])
					chartPaths[1] = path;
				else if (filename == validFiles[5])
					hasIni = true;
			}
		}

		for (int i = 0; i < 4; ++i)
			if (!chartPaths[i].empty() && (hasIni || i & 1))
			{
				auto song = std::make_unique<Song>(chartPaths[i], hasIni);
				if (song->scan())
					g_songCache.push(song);
				return;
			}

		for (const auto& directory : directories)
			TaskQueue::addTask(std::make_unique<Task_SongScan>(directory));
	}
	catch (...) {}
	return;
}
