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
#include "Ini/IniFile.h"
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
protected:
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

	MD5 m_hash;
	IniFile m_ini;

	uint16_t m_version_bch = 1;
	NumberModifier<uint16_t> m_version_cht{ "FileVersion", 2 };
	NumberModifier<float>    m_offset     { "Offset" };

	struct
	{
		StringModifier          name              { "Name",    false };
		StringModifier          artist            { "Artist",  false };
		StringModifier          charter           { "Charter", false };
		StringModifier          album             { "Album",   false };
		StringModifier          year              { "Year",    false };
		NumberModifier<int16_t> difficulty        { "Difficulty"     };
		NumberModifier<float>   preview_start_time{ "PreviewStart"   };
		NumberModifier<float>   preview_end_time  { "PreviewEnd"     };
		StringModifier          genre             { "Genre",   false };
	} m_songInfo;

public:
	Song() = default;
	Song(const std::filesystem::path& filepath);

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
			return m_ini.m_name;
		else if constexpr (Attribute == SongAttribute::ARTIST)
			return m_ini.m_artist;
		else if constexpr (Attribute == SongAttribute::ALBUM)
			return m_ini.m_album;
		else if constexpr (Attribute == SongAttribute::GENRE)
			return m_ini.m_genre;
		else if constexpr (Attribute == SongAttribute::YEAR)
			return m_ini.m_year;
		else if constexpr (Attribute == SongAttribute::CHARTER)
			return m_ini.m_charter;
		else if constexpr (Attribute == SongAttribute::PLAYLIST)
		{
			if (!m_ini.m_playlist.m_string->empty())
				return m_ini.m_playlist;
			return m_directory_playlist;
		}
		else 
			return m_ini.m_name;
	}

	// Compares only by the file's hash
	bool operator==(const Song& other) const;
	bool operator<(const Song& other) const;
	bool isHashLessThan(const Song& other) const;

	std::unique_ptr<NoteTrack_Scan> m_noteTrackScans[11] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	std::filesystem::file_time_type m_last_modified;

public:

	bool scan(bool hasIni);
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

	NumberModifier<uint16_t> m_tickrate{ "Resolution", 192, 192 };

	struct
	{
		StringModifier music { "MusicStream" , false };
		StringModifier guitar{ "GuitarStream", false };
		StringModifier bass  { "BassStream"  , false };
		StringModifier rhythm{ "RhythmStream", false };
		StringModifier keys  { "KeysStream"  , false };
		StringModifier drum  { "DrumStream"  , false };
		StringModifier drum_2{ "Drum2Stream" , false };
		StringModifier drum_3{ "Drum3Stream" , false };
		StringModifier drum_4{ "Drum4Stream" , false };
		StringModifier vocals{ "VocalStream" , false };
		StringModifier crowd { "CrowdStream" , false };
	} m_audioStreams;

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
