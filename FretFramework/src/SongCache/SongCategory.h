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
};

using ByTitle       = SongCategory<CategoryNode, SongAttribute::TITLE>;
using ByArtist      = SongCategory<CategoryNode, SongAttribute::ARTIST>;
using ByAlbum       = SongCategory<CategoryNode, SongAttribute::ALBUM>;
using ByGenre       = SongCategory<CategoryNode, SongAttribute::GENRE>;
using ByYear        = SongCategory<CategoryNode, SongAttribute::YEAR>;
using ByCharter     = SongCategory<CategoryNode, SongAttribute::CHARTER>;
using ByPlaylist    = SongCategory<CategoryNode, SongAttribute::PLAYLIST>;
using ByArtistAlbum = SongCategory<ByAlbum,      SongAttribute::ARTIST>;
