#include "SongCache.h"
#include <set>

SongCache::SongCache(const std::filesystem::path& cacheLocation)
	: m_location(cacheLocation)
{
	unsigned int numThreads = std::thread::hardware_concurrency() >= 4 ? std::thread::hardware_concurrency() / 2 : 1;
	for (unsigned int i = 0; i < numThreads; ++i)
		m_threads.emplace_back(std::thread(&SongCache::runScanner, this, std::ref(m_sets.emplace_back())));
	m_setIter = m_sets.begin();
}

SongCache::~SongCache()
{
	m_status = EXIT;
	for (auto& set : m_sets)
		set.condition.notify_one();

	for (auto& thr : m_threads)
		thr.join();
}

void SongCache::scan(const std::vector<std::filesystem::path>& baseDirectories)
{
	m_category_artistAlbum.clear();
	m_category_title.clear();
	m_category_artist.clear();
	m_category_album.clear();
	m_category_genre.clear();
	m_category_year.clear();
	m_category_charter.clear();
	m_category_playlist.clear();
	m_songlist.clear();
	if (m_location.empty() || !std::filesystem::exists(m_location))
		for (const std::filesystem::path& directory : baseDirectories)
			scanDirectory(directory);
	finalize();
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
			auto iter = m_setIter++;
			if (m_setIter == m_sets.end())
				m_setIter = m_sets.begin();

			iter->queue.push({ m_songlist.emplace_back(chartPaths[i]), hasIni });
			iter->condition.notify_one();
			return true;
		}
	return false;
}

void SongCache::finalize()
{
	std::unique_lock lk(m_sharedMutex);
	for (auto& set : m_sets)
		m_sharedCondition.wait(lk, [&] { return set.queue.empty(); });

	if (!m_allowDuplicates)
		validateSongList();
	else
		validateSongList_allowDuplicates();
	fillCategories();
}

void SongCache::validateSongList()
{
	std::vector<typename std::list<Song>::iterator> finalSetList;
	finalSetList.reserve(m_songlist.size());

	Song::setAttributeType(SongAttribute::MD5_HASH);
	for (typename std::list<Song>::iterator iter = m_songlist.begin(); iter != m_songlist.end();)
		if (iter->isValid())
		{
			iter->wait();
			auto position = std::upper_bound(finalSetList.begin(), finalSetList.end(), iter,
				[](const typename std::list<Song>::iterator& key, const typename std::list<Song>::iterator& cmp) {
					return *key < *cmp;
				});

			if (position == finalSetList.begin() || **(position - 1) != *iter)
			{
				finalSetList.emplace(position, iter);
				++iter;
			}
			else
			{
				--position;
				if ((*position)->getDirectory() <= iter->getDirectory())
				{
					m_songlist.erase(iter++);
					continue;
				}
				else
				{
					m_songlist.erase(*position);
					*position = iter;
					++iter;
				}
			}
		}
		else
			m_songlist.erase(iter++);
}

void SongCache::validateSongList_allowDuplicates()
{
	for (auto iter = m_songlist.begin(); iter != m_songlist.end();)
		if (iter->isValid())
			++iter;
		else
			m_songlist.erase(iter++);
}

void SongCache::fillCategories()
{
	for (Song& song : m_songlist)
	{
		m_category_title.add(&song);
		m_category_artist.add(&song);
		m_category_album.add(&song);
		m_category_genre.add(&song);
		m_category_year.add(&song);
		m_category_charter.add(&song);
		m_category_playlist.add(&song);
		m_category_artistAlbum.add(&song);
	}
}

void SongCache::runScanner(ThreadSet& set)
{
	std::unique_lock lk(set.mutex);
	do
	{
		while (!set.queue.empty())
		{
			ScanQueueNode& scan = set.queue.front();
			scan.song.scan_full(scan.hasIni);
			set.queue.pop();
		}
		m_sharedCondition.notify_one();
		set.condition.wait(lk);
	} while (m_status != EXIT);
}
