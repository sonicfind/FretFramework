#include "SongCache.h"

std::filesystem::path SongCache::s_location;
bool SongCache::s_allowDuplicates = false;

std::vector<std::unique_ptr<SongEntry>> SongCache::s_songs;
std::mutex SongCache::s_mutex;

ByTitle       SongCache::s_category_title;
ByArtist      SongCache::s_category_artist;
ByAlbum       SongCache::s_category_album;
ByGenre       SongCache::s_category_genre;
ByYear        SongCache::s_category_year;
ByCharter     SongCache::s_category_charter;
ByPlaylist    SongCache::s_category_playlist;
ByArtistAlbum SongCache::s_category_artistAlbum;

std::vector<const std::filesystem::path*> SongCache::s_directories;
std::mutex SongCache::s_directoryMutex;
bool SongCache::s_doCacheWrite;
std::vector<const SongEntry*> SongCache::s_cacheComparison;

void SongCache::setLocation(const std::filesystem::path& cacheLocation) { s_location = cacheLocation; }

void SongCache::clear()
{
	s_category_artistAlbum.clear();
	s_category_title.clear();
	s_category_artist.clear();
	s_category_album.clear();
	s_category_genre.clear();
	s_category_year.clear();
	s_category_charter.clear();
	s_category_playlist.clear();
	s_songs.clear();
}

void SongCache::loadCacheFile()
{
	s_doCacheWrite = false;
	try
	{
		static constexpr auto readStringVector = [](const unsigned char*& ptr)
		{
			uint32_t numStrings;
			memcpy(&numStrings, ptr, sizeof(uint32_t));
			ptr += sizeof(uint32_t);

			std::vector<UnicodeString> strings;
			for (uint32_t i = 0; i < numStrings; ++i)
			{
				const uint32_t length = WebType::read(ptr);
				strings.push_back(UnicodeString::bufferToU32(ptr, length));
				ptr += length;
			}

			return strings;
		};

		FilePointers file("cache.bin");
		const unsigned char* currPtr = file.begin();

		{
			uint32_t version;
			memcpy(&version, currPtr, sizeof(uint32_t));

			if (version != s_CACHE_VERSION)
			{
				s_doCacheWrite = true;
				return;
			}
		}

		currPtr += sizeof(uint32_t);

		const auto titles = readStringVector(currPtr);
		const auto artists = readStringVector(currPtr);
		const auto albums = readStringVector(currPtr);
		const auto genres = readStringVector(currPtr);
		const auto years = readStringVector(currPtr);
		const auto charters = readStringVector(currPtr);
		const auto playlists = readStringVector(currPtr);

		const auto processNode = [&](const unsigned char* dataPtr, StorageDriveType type)
		{
			std::unique_ptr<SongEntry> entry = std::make_unique<SongEntry>();
			entry->setDriveType(type);
			switch (entry->readFromCache(dataPtr))
			{
			case SongEntry::CacheStatus::UNCHANGED:
			{
				CacheIndexNode indices;
				memcpy(&indices, dataPtr, sizeof(CacheIndexNode));
				entry->setSongInfoFromCache(
					artists[indices.m_artistIndex],
					titles[indices.m_titleIndex],
					albums[indices.m_albumIndex],
					genres[indices.m_genreIndex],
					years[indices.m_yearIndex],
					charters[indices.m_charterIndex],
					playlists[indices.m_playlistIndex]);

				addDirectoryEntry(&entry->getDirectory());
				push(entry);
				break;
			}
			case SongEntry::CacheStatus::CHANGED:
				addDirectoryEntry(&entry->getDirectory());
				push(entry);
				__fallthrough;
			case SongEntry::CacheStatus::NOT_PRESENT:
				s_doCacheWrite = true;
				break;
			}
		};

		const uint32_t numNodes = [&currPtr] {
			uint32_t count;
			memcpy(&count, currPtr, sizeof(uint32_t));
			currPtr += sizeof(uint32_t);
			return count; }();

		for (uint32_t i = 0; i < numNodes; ++i)
		{
			const StorageDriveType type = static_cast<StorageDriveType>(*currPtr++);

			uint32_t length;
			memcpy(&length, currPtr, sizeof(uint32_t));
			currPtr += sizeof(uint32_t);

			if (type == SSD)
				TaskQueue::addTask([processNode, currPtr] {
					processNode(currPtr, SSD);
					});
			else
				processNode(currPtr, HDD);

			currPtr += length;
		}

		TaskQueue::waitForCompletedTasks();
		if (!s_doCacheWrite)
		{
			s_cacheComparison.reserve(s_songs.size());
			for (const auto& song : s_songs)
				s_cacheComparison.push_back(song.get());
		}
		std::cout << "Cache read success" << std::endl;
	}
	catch (...)
	{
		s_doCacheWrite = true;
	}
}

void SongCache::finalize()
{
	s_directories.clear();

	for (auto& entry : s_songs)
		TaskQueue::addTask(
			[&entry]
			{
				entry->finalizeScan();
				addToCategories(entry.get());
			});

	TaskQueue::waitForCompletedTasks();
	
	if (s_doCacheWrite ||
		[] {
		if (s_songs.size() == s_cacheComparison.size())
		{
			for (size_t i = 0; i < s_songs.size(); ++i)
				if (s_songs[i].get() != s_cacheComparison[i])
					return true;
			return false;
		}
		return true; }())
		testWrite();
	s_cacheComparison.clear();
}

void SongCache::testWrite()
{
	std::unordered_map<const SongEntry*, CacheIndexNode> nodes;
	auto titles = s_category_title.addFileCacheNodes(nodes);
	auto artist = s_category_artist.addFileCacheNodes(nodes);
	auto album = s_category_album.addFileCacheNodes(nodes);
	auto genre = s_category_genre.addFileCacheNodes(nodes);
	auto year = s_category_year.addFileCacheNodes(nodes);
	auto charter = s_category_charter.addFileCacheNodes(nodes);
	auto playlist = s_category_playlist.addFileCacheNodes(nodes);

	std::fstream outFile("cache.bin", std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
	auto writeStringVector = [&outFile](const std::vector<const UnicodeString*>& strings)
	{
		const uint32_t numStrings = (uint32_t)strings.size();
		outFile.write((char*)&numStrings, 4);
		for (auto str : strings)
			str->writeToWebTypedFile(outFile);
	};

	outFile.write((char*)&s_CACHE_VERSION, 4);

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
		node.first->writeToCache(outFile);
		outFile.write((char*)&node.second, sizeof(CacheIndexNode));
	}
	outFile.close();
	std::cout << "Cache write success" << std::endl;
}

void SongCache::addToCategories(SongEntry* const entry)
{
	s_category_title.add(entry);
	s_category_artist.add(entry);
	s_category_genre.add(entry);
	s_category_year.add(entry);
	s_category_charter.add(entry);

	s_category_album.add<SongAttribute::ALBUM>(entry);
	s_category_artistAlbum.add<SongAttribute::ALBUM>(entry);

	s_category_playlist.add<SongAttribute::PLAYLIST>(entry);
}

void SongCache::push(std::unique_ptr<SongEntry>& song)
{
	std::scoped_lock lock(s_mutex);
	auto iter = std::lower_bound(s_songs.begin(), s_songs.end(), song,
		[](const std::unique_ptr<SongEntry>& first, const std::unique_ptr<SongEntry>& second)
		{
			return first->isHashLessThan(*second);
		});

	if (iter == s_songs.end() || !(*iter)->areHashesEqual(*song))
		s_songs.emplace(iter, std::move(song));
	else if (s_allowDuplicates)
	{
		do
		{
			if ((*iter)->getDirectory() < song->getDirectory())
				++iter;
			else
				break;
		} while (iter != s_songs.end() && (*iter)->areHashesEqual(*song));
		s_songs.emplace(iter, std::move(song));
	}
	else if (song->getDirectory() < (*iter)->getDirectory())
		*iter = std::move(song);
}

void SongCache::addDirectoryEntry(const std::filesystem::path* directory)
{
	std::scoped_lock lock(s_directoryMutex);
	auto iter = std::lower_bound(s_directories.begin(), s_directories.end(), directory,
		[](const std::filesystem::path* const first, const std::filesystem::path* const second)
		{
			return *first < *second;
		});

	s_directories.insert(iter, directory);
}

bool SongCache::compareDirectory(const std::filesystem::path& directory)
{
	auto iter = std::lower_bound(s_directories.begin(), s_directories.end(), directory,
		[](const std::filesystem::path* const first, const std::filesystem::path& second)
		{
			return *first < second;
		});

	return iter != s_directories.end() && directory == **iter;
}
