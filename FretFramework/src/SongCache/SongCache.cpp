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

void SongCache::finalize()
{

	for (auto& entry : s_songs)
		TaskQueue::addTask(
			[&entry]
			{
				entry->finalizeScan();
				addToCategories(entry.get());
			});

	TaskQueue::waitForCompletedTasks();
	testWrite();
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
