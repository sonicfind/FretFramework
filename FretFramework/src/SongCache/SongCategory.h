#pragma once
#include "Song/SongEntry.h"
#include <set>
#include <map>


struct PointerCompare
{
	template <class T>
	bool operator()(const T* const lhs, const T* const rhs) const
	{
		return *lhs < *rhs;
	}
};

struct CacheFileNode
{
	std::u32string m_directory;
	uint64_t m_lastModified[2]{};
	std::string m_chartFile;
	uint32_t m_titleIndex = UINT32_MAX;
	uint32_t m_artistIndex = UINT32_MAX;
	uint32_t m_albumIndex = UINT32_MAX;
	uint32_t m_genreIndex = UINT32_MAX;
	uint32_t m_yearIndex = UINT32_MAX;
	uint32_t m_charterIndex = UINT32_MAX;
	uint32_t m_playlistIndex = UINT32_MAX;
	unsigned char m_tracks[11]{};
	uint32_t previewRange[2]{};
	std::u32string m_title;
	std::u32string m_icon;
	uint16_t m_albumTrack = UINT16_MAX;
	uint16_t m_playlistTrack = UINT16_MAX;
	uint32_t m_songLength = 0;
	MD5 m_hash;
};

class CategoryNode
{
	std::set<SongEntry*, PointerCompare> m_songs;

public:
	void add(SongEntry* song)
	{
		m_songs.insert(song);
	}

	void clear()
	{
		m_songs.clear();
	}

	template <SongAttribute Attribute>
	void setCacheNodeIndices(const uint32_t index, std::map<const SongEntry*, CacheFileNode>& _cacheNodes) const
	{
		for (const SongEntry* const entry : m_songs)
		{
			if      constexpr (Attribute == SongAttribute::TITLE)    _cacheNodes[entry].m_titleIndex = index;
			else if constexpr (Attribute == SongAttribute::ARTIST)   _cacheNodes[entry].m_artistIndex = index;
			else if constexpr (Attribute == SongAttribute::ALBUM)    _cacheNodes[entry].m_albumIndex = index;
			else if constexpr (Attribute == SongAttribute::GENRE)    _cacheNodes[entry].m_genreIndex = index;
			else if constexpr (Attribute == SongAttribute::YEAR)     _cacheNodes[entry].m_yearIndex = index;
			else if constexpr (Attribute == SongAttribute::CHARTER)  _cacheNodes[entry].m_charterIndex = index;
			else if constexpr (Attribute == SongAttribute::PLAYLIST) _cacheNodes[entry].m_playlistIndex = index;
		}
	}
};

template <class Element, SongAttribute Attribute>
class SongCategory
{
	std::map<const UnicodeString*, Element, PointerCompare> m_elements;

public:
	void add(SongEntry* song)
	{
		m_elements[song->getAttribute<Attribute>()].add(song);
	}

	void clear()
	{
		m_elements.clear();
	}

	std::vector<const UnicodeString*> addToFileCache(std::map<const SongEntry*, CacheFileNode>& _cacheNodes) const
	{
		std::vector<const UnicodeString*> strings;
		uint32_t index = 0;
		for (const auto& element : m_elements)
		{
			strings.push_back(element.first);
			element.second.setCacheNodeIndices<Attribute>(index++, _cacheNodes);
		}
		return strings;
	}
};

template <>
class SongCategory<CategoryNode, SongAttribute::TITLE>
{
	std::map<char32_t, CategoryNode> m_elements;

public:
	void add(SongEntry* song)
	{
		m_elements[song->getAttribute<SongAttribute::TITLE>()->getLowerCase()[0]].add(song);
	}

	void clear()
	{
		m_elements.clear();
	}

	std::vector<char32_t> addToFileCache(std::map<const SongEntry*, CacheFileNode>& _cacheNodes) const
	{
		std::vector<char32_t> characters;
		uint32_t index = 0;
		for (const auto& element : m_elements)
		{
			characters.push_back(element.first);
			element.second.setCacheNodeIndices<SongAttribute::TITLE>(index++, _cacheNodes);
		}
		return characters;
	}
};

using ByTitle       = SongCategory<CategoryNode, SongAttribute::TITLE>;
using ByArtist      = SongCategory<CategoryNode, SongAttribute::ARTIST>;
using ByAlbum       = SongCategory<CategoryNode, SongAttribute::ALBUM>;
using ByGenre       = SongCategory<CategoryNode, SongAttribute::GENRE>;
using ByYear        = SongCategory<CategoryNode, SongAttribute::YEAR>;
using ByCharter     = SongCategory<CategoryNode, SongAttribute::CHARTER>;
using ByPlaylist    = SongCategory<CategoryNode, SongAttribute::PLAYLIST>;
using ByArtistAlbum = SongCategory<ByAlbum,      SongAttribute::ARTIST>;
