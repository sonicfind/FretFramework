#pragma once
#include "SyncTrack.h"
template <class T>
class WritableModifier
{
	std::string_view m_name;
public:
	T m_value{};

	WritableModifier(const char* str) : m_name(str) {}

	bool read(const std::string& name, std::stringstream& ss)
	{
		if (name.find(m_name) != std::string::npos)
		{
			ss >> m_value;
			return true;
		}
		return false;
	}

	void write(std::ofstream& outFile) const
	{
		outFile << "  " << m_name << " = " << m_value << '\n';
	}

	void reset() { m_value = T(); }
};

template<>
bool WritableModifier<std::string>::read(const std::string& name, std::stringstream& ss);

template<>
void WritableModifier<std::string>::write(std::ofstream& outFile) const;

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
	std::map<uint32_t, SyncTrack> m_syncTracks;
	std::map<uint32_t, std::string> m_sectionMarkers;
public:
	Chart() = default;
	Chart(std::ifstream& inFile);
	void write_chart(std::ofstream& outFile) const;

private:
	void readMetadata(std::ifstream& inFile);
	void readSync(std::ifstream& inFile);
	void writeSync(std::ofstream& outFile) const;
	void readEvents(std::ifstream& inFile);
	void writeEvents(std::ofstream& outFile) const;
	void readNoteTrack(std::ifstream& inFile, const std::string& func);
	void writeNoteTracks_chart(std::ofstream& outFile) const;
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
