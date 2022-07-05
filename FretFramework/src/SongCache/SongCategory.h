#pragma once
#include "Song/Song.h"
#include <set>
#include <map>

class CategoryNode
{
	struct ElementCmp
	{
		bool operator()(const Song* lhs, const Song* rhs) const
		{
			return *lhs < *rhs;
		}
	};
	std::set<Song*, ElementCmp> m_songs;

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

template <class Element, SongAttribute attribute>
class SongCategory
{
	struct UnicodeStringCmp
	{
		bool operator()(const UnicodeString* lhs, const UnicodeString* rhs) const
		{
			return *lhs < *rhs;
		}
	};

	std::map<const UnicodeString*, Element, UnicodeStringCmp> m_elements;

public:
	void add(Song* song)
	{
		Song::setAttributeType(attribute);
		m_elements[&song->getAttribute()].add(song);
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
		Song::setAttributeType(SongAttribute::TITLE);
		m_elements[song->getAttribute().getLowerCase()[0]].add(song);
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
