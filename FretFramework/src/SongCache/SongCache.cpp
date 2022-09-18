#include "SongCache.h"

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

	for (auto& entry : m_songs)
		TaskQueue::addTask([&entry] { entry->finalizeScan(); });

	TaskQueue::waitForCompletedTasks();
	fillCategories();
	testWrite();
}

void SongCache::testWrite()
{
	std::unordered_map<const SongEntry*, CacheIndexNode> nodes;
	auto titles = m_category_title.addFileCacheNodes(nodes);
	auto artist = m_category_artist.addFileCacheNodes(nodes);
	auto album = m_category_album.addFileCacheNodes(nodes);
	auto genre = m_category_genre.addFileCacheNodes(nodes);
	auto year = m_category_year.addFileCacheNodes(nodes);
	auto charter = m_category_charter.addFileCacheNodes(nodes);
	auto playlist = m_category_playlist.addFileCacheNodes(nodes);

	std::fstream outFile("cache.bin", std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
	auto writeStringVector = [&outFile](const std::vector<const UnicodeString*>& strings)
	{
		const uint32_t numStrings = (uint32_t)strings.size();
		outFile.write((char*)&numStrings, 4);
		for (auto str : strings)
			str->writeToWebTypedFile(outFile);
	};

	writeStringVector(titles);
	writeStringVector(artist);
	writeStringVector(album);
	writeStringVector(genre);
	writeStringVector(year);
	writeStringVector(charter);
	writeStringVector(playlist);

	{
		const uint32_t numNodes = (uint32_t)nodes.size();
		outFile.write((char*)&numNodes, 4);
	}

	for (auto& node : nodes)
	{

		outFile.write((char*)&node.second, sizeof(CacheIndexNode));
	}
	outFile.close();
}

void SongCache::removeDuplicates()
{
	auto endIter = std::unique(m_songs.begin(), m_songs.end(),
		[](const std::unique_ptr<SongEntry>& first, const std::unique_ptr<SongEntry>& second)
		{
			return first->areHashesEqual(*second);
		});
	m_songs.erase(endIter, m_songs.end());
}

void SongCache::fillCategories()
{
	for (std::unique_ptr<SongEntry>& ptr : m_songs)
	{
		SongEntry* const song = ptr.get();

		SongEntry::setSortAttribute(SongAttribute::TITLE);
		m_category_title.add(song);
		m_category_artist.add(song);
		m_category_genre.add(song);
		m_category_year.add(song);
		m_category_charter.add(song);

		SongEntry::setSortAttribute(SongAttribute::ALBUM);
		m_category_album.add(song);
		m_category_artistAlbum.add(song);

		SongEntry::setSortAttribute(SongAttribute::PLAYLIST);
		m_category_playlist.add(song);
	}
}

void SongCache::push(std::unique_ptr<SongEntry>& song)
{
	std::scoped_lock lock(m_mutex);
	auto iter = std::lower_bound(m_songs.begin(), m_songs.end(), song,
		[](const std::unique_ptr<SongEntry>& first, const std::unique_ptr<SongEntry>& second)
		{
			return first->isHashLessThan(*second);
		});

	m_songs.emplace(iter, std::move(song));
}

void SongCache::scanDirectory(const std::filesystem::path& directory)
{
	static const std::filesystem::path NAME_BCH(U"notes.bch");
	static const std::filesystem::path NAME_CHT(U"notes.cht");
	static const std::filesystem::path NAMES_MIDI[2] = { U"notes.mid", U"notes.midi" };
	static const std::filesystem::path NAME_CHART(U"notes.chart");
	static const std::filesystem::path NAME_INI(U"song.ini");

	try
	{
		std::filesystem::path chartPaths[4];
		bool hasIni = false;

		std::vector<std::filesystem::path> directories;
		for (const auto& file : std::filesystem::directory_iterator(directory))
		{
			if (file.is_directory())
				directories.emplace_back(file.path());
			else
			{
				const std::filesystem::path& path = file.path();
				const std::u32string filename = path.filename().u32string();
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

		if (!hasIni)
		{
			chartPaths[0].clear();
			chartPaths[2].clear();
		}

		for (int i = 0; i < 4; ++i)
			if (!chartPaths[i].empty())
			{
				auto song = std::make_unique<SongEntry>(chartPaths[i]);
				if (song->scan(hasIni, i == 0 || i == 2))
				{
					g_songCache.push(song);
					return;
				}
				break;
			}

		for (auto& directory : directories)
			TaskQueue::addTask([dir = std::move(directory)]
				{
					scanDirectory(dir);
				});
	}
	catch (...) {}
	return;
}
