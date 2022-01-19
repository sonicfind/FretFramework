#pragma once
#include "Modifiers.h"
#include "SyncValues.h"
#include "NodeTrack.h"
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

class Chart
{
	struct IniData
	{
		WritableModifier<uint32_t> offset			{ "Offset" };
		WritableModifier<uint32_t> ticks_per_beat	{ "Resolution" };
		struct
		{
			WritableModifier<std::string> name				{ "Name" };
			WritableModifier<std::string> artist			{ "Artist" };
			WritableModifier<std::string> charter			{ "Charter" };
			WritableModifier<std::string> album				{ "Album" };
			WritableModifier<std::string> year				{ "Year" };
			WritableModifier<int32_t> difficulty			{ "Difficulty" };
			WritableModifier<uint32_t> preview_start_time	{ "PreviewStart" };
			WritableModifier<uint32_t> preview_end_time		{ "PreviewEnd" };
			WritableModifier<std::string> genre				{ "Genre" };
			WritableModifier<std::string> media_type		{ "MediaType" };
		} m_songInfo;

		struct
		{
			WritableModifier<std::string> music 	{ "MusicStream" };
			WritableModifier<std::string> guitar	{ "GuitarStream" };
			WritableModifier<std::string> bass  	{ "BassStream" };
			WritableModifier<std::string> rhythm	{ "RhythmStream" };
			WritableModifier<std::string> keys  	{ "KeysStream" };
			WritableModifier<std::string> drum  	{ "DrumStream" };
			WritableModifier<std::string> drum_2	{ "Drum2Stream" };
			WritableModifier<std::string> drum_3	{ "Drum3Stream" };
			WritableModifier<std::string> drum_4	{ "Drum4Stream" };
			WritableModifier<std::string> vocals	{ "VocalStream" };
			WritableModifier<std::string> crowd 	{ "CrowdStream" };
		} m_audioStreams;

		bool read(std::stringstream& ss);
		void write(std::ofstream& outFile) const;
	} m_iniData;

	std::map<uint32_t, SyncValues> m_sync;
	std::map<uint32_t, std::string> m_sectionMarkers;

	NodeTrack<GuitarNote_5Fret> m_leadGuitar;
	NodeTrack<GuitarNote_6Fret> m_leadGuitar_6;
	NodeTrack<GuitarNote_5Fret> m_bassGuitar;
	NodeTrack<GuitarNote_6Fret> m_bassGuitar_6;
	NodeTrack<GuitarNote_5Fret> m_rhythmGuitar;
	NodeTrack<GuitarNote_5Fret> m_coopGuitar;
	NodeTrack<DrumNote<4, DrumPad_Pro>> m_drums;
	NodeTrack<DrumNote<5, DrumPad>> m_drums_5Lane;
	std::map<uint32_t, std::vector<std::string>> m_globalEvents;
public:
	Chart() = default;
	Chart(std::ifstream& inFile);
	void write_chart(std::ofstream& outFile, bool version2 = false) const;

private:
	void readMetadata(std::ifstream& inFile);
	void readSync(std::ifstream& inFile);
	void writeSync(std::ofstream& outFile) const;
	void readEvents(std::ifstream& inFile);
	void writeEvents(std::ofstream& outFile) const;
	void readNoteTrack(std::ifstream& inFile, const std::string& func);
	void writeNoteTracks_chart(std::ofstream& outFile, bool version2 = false) const;
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
