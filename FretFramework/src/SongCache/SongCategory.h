#pragma once
#include "Song/Song.h"

template <class Element>
class CategoryNode
{
	std::vector<const Element*> m_elements;

public:
	void add(const Element& element)
	{
		auto iter = std::upper_bound(m_elements.begin(), m_elements.end(), element,
			[](const Element& insert, const Element* const node) {
				return insert < *node;
			});

		m_elements.insert(iter, &element);
	}

	void clear()
	{
		m_elements.clear();
	}

	const Element* const front() const { return m_elements.front(); }
};

template <class Key, class Element, SongAttribute attribute>
class SongCategory
{
	std::vector<std::pair<Key, CategoryNode<Element>>> m_elements;

public:
	void add(const Element& element)
	{
		Song::setAttributeType(attribute);
		VectorIteration::try_emplace(m_elements, element.getAttribute()).add(element);
	}

	void clear()
	{
		m_elements.clear();
	}

	auto begin()
	{
		return m_elements.begin();
	}

	auto end()
	{
		return m_elements.end();
	}
};

template <>
void SongCategory<char32_t, Song, SongAttribute::TITLE>::add(const Song& element);

using ByTitle       = SongCategory<char32_t,             Song, SongAttribute::TITLE>;
using ByArtist      = SongCategory<const UnicodeString*, Song, SongAttribute::ARTIST>;
using ByAlbum       = SongCategory<const UnicodeString*, Song, SongAttribute::ALBUM>;
using ByGenre       = SongCategory<const UnicodeString*, Song, SongAttribute::GENRE>;
using ByYear        = SongCategory<const UnicodeString*, Song, SongAttribute::YEAR>;
using ByCharter     = SongCategory<const UnicodeString*, Song, SongAttribute::CHARTER>;
using ByPlaylist    = SongCategory<const UnicodeString*, Song, SongAttribute::PLAYLIST>;

template <>
class SongCategory<const UnicodeString*, ByAlbum, SongAttribute::ARTIST>
{
	std::vector<std::pair<const UnicodeString*, ByAlbum>> m_elements;

public:
	void add(const Song& element)
	{
		Song::setAttributeType(SongAttribute::ARTIST);
		VectorIteration::try_emplace(m_elements, element.getAttribute()).add(element);
	}

	void clear()
	{
		m_elements.clear();
	}

	auto begin()
	{
		return m_elements.begin();
	}

	auto end()
	{
		return m_elements.end();
	}
};

using ByArtistAlbum = SongCategory<const UnicodeString*, ByAlbum, SongAttribute::ARTIST>;

