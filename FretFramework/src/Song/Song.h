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
	static struct Tracks
	{
		InstrumentalTrack<GuitarNote<5>>            lead_5    { "[LeadGuitar]", 0 };
		InstrumentalTrack<GuitarNote<6>>            lead_6    { "[LeadGuitar_GHL]", 1 };
		InstrumentalTrack<GuitarNote<5>>            bass_5    { "[BassGuitar]", 2 };
		InstrumentalTrack<GuitarNote<6>>            bass_6    { "[BassGuitar]", 3 };
		InstrumentalTrack<GuitarNote<5>>            rhythm    { "[RhythmGuitar]", 4 };
		InstrumentalTrack<GuitarNote<5>>            coop      { "[CoopGuitar]", 5 };
		InstrumentalTrack<Keys<5>>                  keys      { "[Keys]", 6 };
		InstrumentalTrack<DrumNote<4, DrumPad_Pro>> drums4_pro{ "[Drums_4Lane]", 7 };
		InstrumentalTrack<DrumNote<5, DrumPad>>     drums5    { "[Drums_5Lane]", 8 };
		VocalTrack<1>                               vocals    { "[Vocals]", 9 };
		VocalTrack<3>                               harmonies { "[Harmonies]", 10 };

		// 0/1 -  Guitar 5/6
		// 2/3 -  Bass 5/6
		// 4   -  Rhythm
		// 5   -  Co-op
		// 6   -  Keys
		// 7/8 -  Drums 4/5
		// 9/10 - Vocals/Harmonies
		NoteTrack* const trackArray[11] =
		{
			&lead_5,
			&lead_6,
			&bass_5,
			&bass_6,
			&rhythm,
			&coop,
			&keys,
			&drums4_pro,
			&drums5,
			&vocals,
			&harmonies
		};
	} s_noteTracks;

	std::filesystem::path m_directory;
	UnicodeString m_directory_playlist;

	std::filesystem::path m_chartFile;
	std::filesystem::path m_fullPath;
	std::u32string m_midiSequenceName;

	MD5 m_hash;

	uint16_t m_version_bch = 1;
	static constexpr uint16_t s_VERSION_CHT = 2;

public:
	Song();
	Song(const std::filesystem::path& filepath, bool hasIni = false);

	constexpr void setFullPath(const std::filesystem::path& path);
	void setDirectory(const std::filesystem::path& directory);
	void setChartFile(const char32_t* filename);

	static constexpr void clearTracks()
	{
		for (NoteTrack* const track : s_noteTracks.trackArray)
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
			if (auto playlist = getModifier("playlist"))
				return playlist->getValue<UnicodeString>();
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

	uint16_t m_tickrate = 192;

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
	std::vector<TxtFileModifier> m_modifiers;
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

	const TxtFileModifier* const getModifier(const std::string_view modifierName) const
	{
		for (const TxtFileModifier& modifier : m_modifiers)
			if (modifier.getName() == modifierName)
				return &modifier;
		return nullptr;
	}

	TxtFileModifier* const getModifier(const std::string_view modifierName)
	{
		for (TxtFileModifier& modifier : m_modifiers)
			if (modifier.getName() == modifierName)
				return &modifier;
		return nullptr;
	}

	void removeModifier(const std::string_view modifierName);

	template <class T>
	void setModifier(const std::string_view modifierName, const T& value)
	{
		if (TxtFileModifier* modifier = getModifier(modifierName))
			modifier->setValue(value);
		else
			m_modifiers.push_back({ modifierName, value });
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
