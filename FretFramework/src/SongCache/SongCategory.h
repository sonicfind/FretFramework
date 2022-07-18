#pragma once
#include "Song/Song.h"
#include <set>
#include <map>

template <class T>
struct PointerCompare
{
	bool operator()(const T* const lhs, const T* const rhs) const
	{
		return *lhs < *rhs;
	}
};

class CategoryNode
{
	std::set<Song*, PointerCompare<Song>> m_songs;

public:
	void add(Song* song)
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
	std::map<const UnicodeString*, Element, PointerCompare<UnicodeString>> m_elements;

public:
	void add(Song* song)
	{
		m_elements[&song->getAttribute<Attribute>()].add(song);
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
	void add(Song* song)
	{
		m_elements[song->getAttribute<SongAttribute::TITLE>().getLowerCase()[0]].add(song);
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
