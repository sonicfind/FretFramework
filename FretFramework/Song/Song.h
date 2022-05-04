#pragma once
#include "SyncValues.h"
#include "NodeTrack.h"
#include "VocalTrack.h"
#include "Chart/Modifiers.h"
#include <filesystem>
enum class Instrument
{
	Guitar_lead,
	Guitar_lead_6,
	Guitar_bass,
	Guitar_bass_6,
	Guitar_rhythm,
	Guitar_coop,
	Drums,
	Drums_5,
	Vocals,
	Keys,
	None
};

class Song
{
	std::filesystem::path m_filepath;

	std::vector<std::pair<uint32_t, SyncValues>> m_sync;
	std::vector<std::pair<uint32_t, std::string>> m_sectionMarkers;
	std::vector<std::pair<uint32_t, std::vector<std::string>>> m_globalEvents;

	NodeTrack<GuitarNote<5>> m_leadGuitar  { "LeadGuitar" };
	NodeTrack<GuitarNote<6>> m_leadGuitar_6{ "LeadGuitar_GHL" };
	NodeTrack<GuitarNote<5>> m_bassGuitar  { "BassGuitar" };
	NodeTrack<GuitarNote<6>> m_bassGuitar_6{ "BassGuitar_GHL" };
	NodeTrack<GuitarNote<5>> m_rhythmGuitar{ "RhythmGuitar" };
	NodeTrack<GuitarNote<5>> m_coopGuitar  { "CoopGuitar" };
	NodeTrack<DrumNote>      m_drums       { "Drums" };
	VocalTrack<1>            m_vocals      { "Vocals" };
	VocalTrack<3>            m_harmonies   { "Harmonies" };

	WritableModifier<float>    m_offset                  { "Offset" };
	WritableModifier<uint16_t> m_version                 { "FileVersion", 1 };
	WritableModifier<uint16_t> m_tickrate                { "Resolution", 192, 192 };
	WritableModifier<uint16_t> m_hopo_frequency          { "hopo_frequency", 64, 64};
	WritableModifier<uint16_t> m_sustain_cutoff_threshold{ "sustain_cutoff_threshold", 64, 64 };
	struct
	{
		WritableModifier<std::string> name              { "Name" };
		WritableModifier<std::string> artist            { "Artist" };
		WritableModifier<std::string> charter           { "Charter" };
		WritableModifier<std::string> album             { "Album" };
		WritableModifier<uint32_t>    year              { "Year" };
		WritableModifier<int32_t>     difficulty        { "Difficulty" };
		WritableModifier<uint32_t>    preview_start_time{ "PreviewStart" };
		WritableModifier<uint32_t>    preview_end_time  { "PreviewEnd" };
		WritableModifier<std::string> genre             { "Genre" };
	} m_songInfo;

	struct
	{
		WritableModifier<std::string> music { "MusicStream" };
		WritableModifier<std::string> guitar{ "GuitarStream" };
		WritableModifier<std::string> bass  { "BassStream" };
		WritableModifier<std::string> rhythm{ "RhythmStream" };
		WritableModifier<std::string> keys  { "KeysStream" };
		WritableModifier<std::string> drum  { "DrumStream" };
		WritableModifier<std::string> drum_2{ "Drum2Stream" };
		WritableModifier<std::string> drum_3{ "Drum3Stream" };
		WritableModifier<std::string> drum_4{ "Drum4Stream" };
		WritableModifier<std::string> vocals{ "VocalStream" };
		WritableModifier<std::string> crowd { "CrowdStream" };
	} m_audioStreams;
	
public:
	Song(const std::filesystem::path& filepath);
	void save() const;

	std::filesystem::path getFilepath();
	void setFilepath(const std::filesystem::path& filename);

private:
	void loadFile_Cht();
	void loadFile_Midi();
	void saveFile_Cht(const std::filesystem::path& filepath) const;
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