#pragma once
#include "SongBase.h"
#include "Sync/SyncValues.h"

class Song : public SongBase
{
	std::vector<std::pair<uint32_t, SyncValues>> m_sync;
	std::vector<std::pair<uint32_t, UnicodeString>> m_sectionMarkers;
	std::vector<std::pair<uint32_t, std::vector<UnicodeString>>> m_globalEvents;

	NumberModifier<uint16_t> m_tickrate { "Resolution", 192, 192 };

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
	
public:
	Song();
	Song(const std::filesystem::path& filepath);
	~Song();
	
	void load(const std::filesystem::path& filepath);
	void save();

	void setFilepath(const std::filesystem::path& filename);
private:
	void loadFile_Cht();
	void saveFile_Cht(const std::filesystem::path& filepath) const;

	void loadFile_Bch();
	void saveFile_Bch(const std::filesystem::path& filepath) const;

	void loadFile_Midi();
	void saveFile_Midi(const std::filesystem::path& filepath) const;

public:
	void setTickRate(uint16_t tickRate);

	class InvalidFileException : public std::runtime_error
	{
	public:
		InvalidFileException(const std::string& file) : std::runtime_error("Error: \"" + file + "\" is not a valid chart file") {}
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
