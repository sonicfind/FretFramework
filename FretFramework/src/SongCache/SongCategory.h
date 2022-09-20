#pragma once
#include "Song/SongEntry.h"
#include <set>
#include <map>
#include <unordered_map>
#include <mutex>

struct CacheIndexNode
{
	uint32_t m_titleIndex = UINT32_MAX;
	uint32_t m_artistIndex = UINT32_MAX;
	uint32_t m_albumIndex = UINT32_MAX;
	uint32_t m_genreIndex = UINT32_MAX;
	uint32_t m_yearIndex = UINT32_MAX;
	uint32_t m_charterIndex = UINT32_MAX;
	uint32_t m_playlistIndex = UINT32_MAX;
};

class CategoryNode
{
	std::vector<SongEntry*> m_songs;

public:
	template <SongAttribute SortAttribute = SongAttribute::UNSPECIFIED>
	void add(SongEntry* const song)
	{
		auto iter = std::lower_bound(m_songs.begin(), m_songs.end(), song,
			[](const SongEntry* const first, const SongEntry* const second)
			{
				return first->isLowerOrdered<SortAttribute>(*second);
			});

		m_songs.insert(iter, song);
	}

	void clear()
	{
		m_songs.clear();
	}

	template <SongAttribute Attribute>
	void setCacheNodeIndices(const uint32_t index, std::unordered_map<const SongEntry*, CacheIndexNode>& _cacheNodes) const
	{
		for (const SongEntry* const entry : m_songs)
		{
			if      constexpr (Attribute == SongAttribute::ARTIST)   _cacheNodes[entry].m_artistIndex = index;
			else if constexpr (Attribute == SongAttribute::ALBUM)    _cacheNodes[entry].m_albumIndex = index;
			else if constexpr (Attribute == SongAttribute::GENRE)    _cacheNodes[entry].m_genreIndex = index;
			else if constexpr (Attribute == SongAttribute::YEAR)     _cacheNodes[entry].m_yearIndex = index;
			else if constexpr (Attribute == SongAttribute::CHARTER)  _cacheNodes[entry].m_charterIndex = index;
			else if constexpr (Attribute == SongAttribute::PLAYLIST) _cacheNodes[entry].m_playlistIndex = index;
		}
	}

	void setTitleIndices(std::vector<const UnicodeString*>& _strings, std::unordered_map<const SongEntry*, CacheIndexNode>& _cacheNodes) const
	{
		for (const SongEntry* const entry : m_songs)
		{
			if (_strings.empty() || _strings.back()->get() != entry->getName().get())
				_strings.push_back(&entry->getName());

			_cacheNodes[entry].m_titleIndex = (uint32_t)_strings.size() - 1;
		}
	}
};

template <class Element, SongAttribute Attribute>
class SongCategory
{
	struct UTFCompare
	{
		bool operator()(const UnicodeString* const lhs, const UnicodeString* const rhs) const
		{
			return *lhs < *rhs;
		}
	};

	std::mutex m_mutex;
	std::map<const UnicodeString*, Element, UTFCompare> m_elements;

public:

	template <SongAttribute SortAttribute = SongAttribute::UNSPECIFIED>
	void add(SongEntry* song)
	{
		std::scoped_lock lck(m_mutex);
		m_elements[song->getAttribute<Attribute>()].add<SortAttribute>(song);
	}

	void clear()
	{
		m_elements.clear();
	}

	std::vector<const UnicodeString*> addFileCacheNodes(std::unordered_map<const SongEntry*, CacheIndexNode>& _cacheNodes) const
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
	std::mutex m_mutex;
	std::map<char32_t, CategoryNode> m_elements;

public:

	template <SongAttribute SortAttribute = SongAttribute::UNSPECIFIED>
	void add(SongEntry* song)
	{
		std::scoped_lock lck(m_mutex);
		m_elements[song->getAttribute<SongAttribute::TITLE>()->getLowerCase()[0]].add(song);
	}

	void clear()
	{
		m_elements.clear();
	}

	std::vector<const UnicodeString*> addFileCacheNodes(std::unordered_map<const SongEntry*, CacheIndexNode>& _cacheNodes) const
	{
		std::vector<const UnicodeString*> strings;
		for (const auto& element : m_elements)
			element.second.setTitleIndices(strings, _cacheNodes);
		return strings;
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
