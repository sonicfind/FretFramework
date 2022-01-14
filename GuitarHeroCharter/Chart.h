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

	void reset() { m_value = T(); }
};

template<>
bool WritableModifier<std::string>::read(const std::string& name, std::stringstream& ss);

template<class T>
class PolledModifier : public WritableModifier<T>
{
public:
	using WritableModifier<T>::WritableModifier;
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
			WritableModifier<std::string> year					{ "Year" };
			WritableModifier<int32_t> difficulty			{ "Difficulty" };
			WritableModifier<uint32_t> preview_start_time	{ "PreviewStart" };
			WritableModifier<uint32_t> preview_end_time		{ "PreviewEnd" };
			WritableModifier<std::string> genre				{ "Genre" };
			WritableModifier<std::string> media_type		{ "MediaType" };
		} m_songInfo;

		struct
		{
			PolledModifier<std::string> music	{ "MusicStream" };
			PolledModifier<std::string> guitar	{ "GuitarStream" };
			PolledModifier<std::string> bass	{ "BassStream" };
			PolledModifier<std::string> rhythm	{ "RhythmStream" };
			PolledModifier<std::string> keys	{ "KeysStream" };
			PolledModifier<std::string> drum	{ "DrumStream" };
			PolledModifier<std::string> drum_2	{ "Drum2Stream" };
			PolledModifier<std::string> drum_3	{ "Drum3Stream" };
			PolledModifier<std::string> drum_4	{ "Drum4Stream" };
			PolledModifier<std::string> vocals	{ "VocalStream" };
			PolledModifier<std::string> crowd	{ "CrowdStream" };
		} m_audioStreams;

		bool read(std::stringstream& ss);
	} m_iniData;
	std::map<uint32_t, SyncTrack> m_syncTracks;
	std::map<uint32_t, std::string> m_sectionMarkers;
public:
	Chart();
	void reset();
	void readFromFile(std::ifstream& inFile);

private:
	void readMetadata(std::ifstream& inFile);
	void readSync(std::ifstream& inFile);
	void readEvents(std::ifstream& inFile);
	void readNoteTrack(std::ifstream& inFile, const std::string& func);
};

template<class T>
T& getElement(std::map<uint32_t, T>& map, uint32_t position)
{
	auto iter = map.lower_bound(position);
	if (iter != map.begin())
		--iter;
	return iter->second;
}