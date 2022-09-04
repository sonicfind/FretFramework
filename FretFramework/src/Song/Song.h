#pragma once
#include "Modifiers/Modifiers.h"
#include "Chords/GuitarNote/GuitarNote_cht.hpp"
#include "Chords/GuitarNote/GuitarNote_bch.hpp"
#include "Chords/Keys.h"
#include "Drums/DrumNote_cht.hpp"
#include "Drums/DrumNote_bch.hpp"
#include "Tracks/InstrumentalTracks/InstrumentalTrack_cht.hpp"
#include "Tracks/InstrumentalTracks/InstrumentalTrack_bch.hpp"
#include "Tracks/InstrumentalTracks/DrumTrack/DrumTrack_Legacy.h"
#include "Tracks/VocalTracks/VocalTrack_cht.hpp"
#include "Tracks/VocalTracks/VocalTrack_bch.hpp"
#include "Sync/SyncValues.h"
#include "MD5/MD5.h"
#include <filesystem>

enum class Instrument
{
	Guitar_lead,
	Guitar_lead_6,
	Guitar_bass,
	Guitar_bass_6,
	Guitar_rhythm,
	Guitar_coop,
	Keys,
	Drums_4,
	Drums_5,
	Vocals,
	Harmonies,
	Drums_Legacy,
	None
};

enum class SongAttribute
{
	TITLE,
	ARTIST,
	ALBUM,
	GENRE,
	YEAR,
	CHARTER,
	PLAYLIST
};

class Song
{
	// 0 -  Guitar 5
	// 1 -  Guitar 6
	// 2 -  Bass 5
	// 3 -  Bass 6
	// 4 -  Rhythm
	// 5 -  Co-op
	// 6 -  Keys
	// 7 -  Drums 4
	// 8 -  Drums 5
	// 9 -  Vocals
	// 10 - Harmonies
	static std::unique_ptr<NoteTrack> const s_noteTracks[11];

	std::filesystem::path m_directory;
	UnicodeString m_directory_playlist;

	std::filesystem::path m_chartFile;
	std::filesystem::path m_fullPath;
	std::u32string m_midiSequenceName;

	MD5 m_hash;

	uint16_t m_version_bch = 1;
	static constexpr UINT16Modifier s_VERSION_CHT{ "FileVersion", 2 };

public:
	Song();
	Song(const std::filesystem::path& filepath, bool hasIni = false);

	constexpr void setFullPath(const std::filesystem::path& path);
	void setDirectory(const std::filesystem::path& directory);
	void setChartFile(const char32_t* filename);

	static constexpr void clearTracks()
	{
		for (const auto& track : s_noteTracks)
			track->clear();
	}

	static SongAttribute s_sortAttribute;
	static constexpr void setSortAttribute(SongAttribute attribute) { s_sortAttribute = attribute; }

	
	template<SongAttribute Attribute>
	constexpr const UnicodeString& getAttribute() const
	{
		if constexpr (Attribute == SongAttribute::TITLE)
			return *m_name;
		else if constexpr (Attribute == SongAttribute::ARTIST)
			return *m_artist;
		else if constexpr (Attribute == SongAttribute::ALBUM)
			return *m_album;
		else if constexpr (Attribute == SongAttribute::GENRE)
			return *m_genre;
		else if constexpr (Attribute == SongAttribute::YEAR)
			return *m_year;
		else if constexpr (Attribute == SongAttribute::CHARTER)
			return *m_charter;
		else if constexpr (Attribute == SongAttribute::PLAYLIST)
		{
			if (auto const playlist = getModifier<const StringModifier>("playlist"))
				return playlist->m_string;
			return m_directory_playlist;
		}
	}

	// Compares only by the file's hash
	bool operator==(const Song& other) const;
	bool operator<(const Song& other) const;
	bool isHashLessThan(const Song& other) const;

	std::unique_ptr<NoteTrack_Scan> m_noteTrackScans[11] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	std::filesystem::file_time_type m_last_modified;

public:

	bool scan();
	constexpr bool validate();
	void displayScanResult() const;
	std::filesystem::path getDirectory() const { return m_directory; }

private:
	void scanFile(TextTraversal&& traversal);
	void scanFile(BCHTraversal&&  traversal);
	void scanFile(MidiTraversal&& traversal);
	void finalizeScan();




	std::vector<std::pair<uint32_t, SyncValues>> m_sync;
	std::vector<std::pair<uint32_t, UnicodeString>> m_sectionMarkers;
	std::vector<std::pair<uint32_t, std::vector<std::u32string>>> m_globalEvents;

	UINT16Modifier m_tickrate{ "Resolution", 192 };

	static void traverseCHTSongSection(Song* const song, TextTraversal& traversal, const auto& modifierMap)
	{
		while (traversal && traversal != '}' && traversal != '[')
		{
			try
			{
				const auto name = traversal.extractModifierName();
				auto iter = std::lower_bound(begin(modifierMap), end(modifierMap), name,
					[](const std::pair<std::string_view, size_t>& pair, const std::string_view str)
					{
						return pair.first < str;
					});

				if (iter != end(modifierMap) && name == iter->first)
					reinterpret_cast<TxtFileModifier*>((char*)song + iter->second)->read(traversal);
			}
			catch (...) {}
			traversal.next();
		}
	}

public:
	void load();
	void save();

	
	void setTickRate(uint16_t tickRate);

	class InvalidFileException : public std::runtime_error
	{
	public:
		InvalidFileException(const std::string& file) : std::runtime_error("Error: \"" + file + "\" is not a valid chart file") {}
	};
private:
	void loadFile(TextTraversal&& traversal);
	void loadFile(BCHTraversal&&  traversal);
	void loadFile(MidiTraversal&& traversal);

	void saveFile_Cht() const;
	void saveFile_Bch() const;
	void saveFile_Midi() const;


	////////////////////
	////////////////////
	//   Ini Section
	////////////////////
	////////////////////

	static const     UnicodeString s_DEFAULT_NAME;
	static const     UnicodeString s_DEFAULT_ARTIST;
	static const     UnicodeString s_DEFAULT_ALBUM;
	static const     UnicodeString s_DEFAULT_GENRE;
	static const     UnicodeString s_DEFAULT_YEAR;
	static const     UnicodeString s_DEFAULT_CHARTER;
	static constexpr uint32_t      s_DEFAULT_SONG_LENGTH = 0;

	bool m_hasIniFile = false;
	std::vector<std::unique_ptr<TxtFileModifier>> m_modifiers;
	const UnicodeString* m_name;
	const UnicodeString* m_artist;
	const UnicodeString* m_album;
	const UnicodeString* m_genre;
	const UnicodeString* m_year;
	const UnicodeString* m_charter;
	const uint32_t*      m_song_length;

	const UnicodeString* getArtist() const { return m_artist; }
	const UnicodeString* getName() const { return m_name; }
	const UnicodeString* getAlbum() const { return m_album; }
	const UnicodeString* getGenre() const { return m_genre; }
	const UnicodeString* getYear() const { return m_year; }
	const UnicodeString* getCharter() const { return m_charter; }
	const uint32_t* getSongLength() const { return m_song_length; }

	bool load_Ini(std::filesystem::path directory);
	bool save_Ini(std::filesystem::path directory) const;

public:
	void setBaseModifiers();

	template <class ModifierType = TxtFileModifier>
	ModifierType* const getModifier(const std::string_view modifierName) const
	{
		static_assert(std::is_base_of_v<TxtFileModifier, ModifierType>);

		for (const std::unique_ptr<TxtFileModifier>& modifier : m_modifiers)
			if (modifier->getName() == modifierName)
				return dynamic_cast<ModifierType*>(modifier.get());
		return nullptr;
	}

	void removeModifier(const std::string_view modifierName);
	void removeModifier(TxtFileModifier* modifier);

	template <class ModifierType = TxtFileModifier>
	void setModifier(const std::string_view modifierName, const auto& value)
	{
		ModifierType* modifier = getModifier<ModifierType>(modifierName);
		if (!modifier)
			modifier = static_cast<ModifierType*>(m_modifiers.emplace_back(std::make_unique<ModifierType>(modifierName)).get());

		*modifier = value;
	}
};

template<class T>
auto getElement(std::vector<std::pair<uint32_t, T>>& vec, const uint32_t position)
{
	auto iter = std::upper_bound(vec.begin(), vec.end(), position,
		[](uint32_t position, const std::pair<uint32_t, T>& pair) {
			return position < pair.first;
		});

	if (iter != vec.begin())
		--iter;
	return iter;
}

template<class T>
auto getElement(const std::vector<std::pair<uint32_t, T>>& vec, const uint32_t position)
{
	auto iter = std::upper_bound(vec.begin(), vec.end(), position,
		[](uint32_t position, const std::pair<uint32_t, T>& pair) {
			return position < pair.first;
		});

	if (iter != vec.begin())
		--iter;
	return iter;
}
