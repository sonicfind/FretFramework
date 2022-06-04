#pragma once
#include "Modifiers/Modifiers.h"
#include "Sync/SyncValues.h"
#include "Chords/GuitarNote/GuitarNote_cht.hpp"
#include "Chords/GuitarNote/GuitarNote_bch.hpp"
#include "Chords/Keys.h"
#include "Drums/DrumNote_cht.hpp"
#include "Drums/DrumNote_bch.hpp"
#include "Tracks/InstrumentalTracks/InstrumentalTrack_cht.hpp"
#include "Tracks/InstrumentalTracks/InstrumentalTrack_bch.hpp"
#include "Tracks/VocalTracks/VocalTrack_cht.hpp"
#include "Tracks/VocalTracks/VocalTrack_bch.hpp"
#include <filesystem>
#include "Ini/IniFile.h"
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
	static NoteTrack* const s_noteTracks[11];

	std::filesystem::path m_filepath;
	IniFile m_ini;

	std::vector<std::pair<uint32_t, SyncValues>> m_sync;
	std::vector<std::pair<uint32_t, std::string>> m_sectionMarkers;
	std::vector<std::pair<uint32_t, std::vector<std::string>>> m_globalEvents;

	NumberModifier<float>    m_offset                  { "Offset" };
	NumberModifier<uint16_t> m_version_cht             { "FileVersion", 2 };
	NumberModifier<uint16_t> m_tickrate                { "Resolution", 192, 192 };
	struct
	{
		StringModifier          name              { "Name" };
		StringModifier          artist            { "Artist" };
		StringModifier          charter           { "Charter" };
		StringModifier          album             { "Album" };
		StringModifier          year              { "Year" };
		NumberModifier<int16_t> difficulty        { "Difficulty" };
		NumberModifier<float>   preview_start_time{ "PreviewStart" };
		NumberModifier<float>   preview_end_time  { "PreviewEnd" };
		StringModifier          genre             { "Genre" };
	} m_songInfo;

	struct
	{
		StringModifier music { "MusicStream" };
		StringModifier guitar{ "GuitarStream" };
		StringModifier bass  { "BassStream" };
		StringModifier rhythm{ "RhythmStream" };
		StringModifier keys  { "KeysStream" };
		StringModifier drum  { "DrumStream" };
		StringModifier drum_2{ "Drum2Stream" };
		StringModifier drum_3{ "Drum3Stream" };
		StringModifier drum_4{ "Drum4Stream" };
		StringModifier vocals{ "VocalStream" };
		StringModifier crowd { "CrowdStream" };
	} m_audioStreams;

	uint16_t m_version_bch = 1;
	
public:
	Song();
	Song(const std::filesystem::path& filepath);
	~Song();
	static void deleteTracks();
	void load(const std::filesystem::path& filepath);
	void save();

	std::filesystem::path getFilepath();
	void setFilepath(const std::filesystem::path& filename);
	void setTickRate(uint16_t tickRate);

private:
	void loadFile_Cht();
	void saveFile_Cht(const std::filesystem::path& filepath) const;

	void loadFile_Bch();
	void saveFile_Bch(const std::filesystem::path& filepath) const;

	void loadFile_Midi();
	void saveFile_Midi(const std::filesystem::path& filepath) const;

public:
	class InvalidExtensionException : public std::runtime_error
	{
	public:
		InvalidExtensionException(const std::string& ext) : std::runtime_error("Error: " + ext + " is not a supported chart extension") {}
	};
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
