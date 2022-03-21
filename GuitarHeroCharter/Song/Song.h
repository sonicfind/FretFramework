#pragma once
#include "SyncValues.h"
#include "NodeTrack.h"
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

	std::map<uint32_t, SyncValues> m_sync;
	std::map<uint32_t, std::string> m_sectionMarkers;
	std::map<uint32_t, std::vector<std::string>> m_globalEvents;

	NodeTrack<GuitarNote<5>> m_leadGuitar;
	NodeTrack<GuitarNote<6>> m_leadGuitar_6;
	NodeTrack<GuitarNote<5>> m_bassGuitar;
	NodeTrack<GuitarNote<6>> m_bassGuitar_6;
	NodeTrack<GuitarNote<5>> m_rhythmGuitar;
	NodeTrack<GuitarNote<5>> m_coopGuitar;
	NodeTrack<DrumNote<4, DrumPad_Pro>> m_drums;
	NodeTrack<DrumNote<5, DrumPad>> m_drums_5Lane;

	WritableModifier<uint32_t> m_offset{ "Offset" };
	WritableModifier<uint32_t> m_ticks_per_beat{ "Resolution" };
	struct
	{
		WritableModifier<std::string> name{ "Name" };
		WritableModifier<std::string> artist{ "Artist" };
		WritableModifier<std::string> charter{ "Charter" };
		WritableModifier<std::string> album{ "Album" };
		WritableModifier<std::string> year{ "Year" };
		WritableModifier<int32_t> difficulty{ "Difficulty" };
		WritableModifier<uint32_t> preview_start_time{ "PreviewStart" };
		WritableModifier<uint32_t> preview_end_time{ "PreviewEnd" };
		WritableModifier<std::string> genre{ "Genre" };
	} m_songInfo;

	struct
	{
		WritableModifier<std::string> music{ "MusicStream" };
		WritableModifier<std::string> guitar{ "GuitarStream" };
		WritableModifier<std::string> bass{ "BassStream" };
		WritableModifier<std::string> rhythm{ "RhythmStream" };
		WritableModifier<std::string> keys{ "KeysStream" };
		WritableModifier<std::string> drum{ "DrumStream" };
		WritableModifier<std::string> drum_2{ "Drum2Stream" };
		WritableModifier<std::string> drum_3{ "Drum3Stream" };
		WritableModifier<std::string> drum_4{ "Drum4Stream" };
		WritableModifier<std::string> vocals{ "VocalStream" };
		WritableModifier<std::string> crowd{ "CrowdStream" };
	} m_audioStreams;
	
public:
	Song(const std::filesystem::path& filepath);
	void save() const;

	std::filesystem::path getFilepath();
	void setFilepath(const std::filesystem::path& filename);

protected:
	void loadFile_Chart();
	void loadFile_Midi();
	void saveFile_Chart(const std::filesystem::path& filepath) const;
	void saveFile_Midi(const std::filesystem::path& filepath) const;
	
};

template<class T>
auto getElement(std::map<uint32_t, T>& map, const uint32_t position)
{
	auto iter = map.upper_bound(position);
	if (iter != map.begin())
		--iter;
	return iter;
}

template<class T>
auto getElement(const std::map<uint32_t, T>& map, const uint32_t position)
{
	auto iter = map.upper_bound(position);
	if (iter != map.begin())
		--iter;
	return iter;
}
