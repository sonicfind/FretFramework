#pragma once
#include "Song.h"
class Chart :
    public Song
{
	WritableModifier<uint32_t> offset{ "Offset" };
	WritableModifier<uint32_t> ticks_per_beat{ "Resolution" };
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
	Chart(const std::string filename);
	void save() const;

private:
	void readMetadata(std::fstream& inFile) override;
	void writeMetadata(std::fstream& outFile) const override;
	void readSync(std::fstream& inFile) override;
	void writeSync(std::fstream& outFile) const override;
	void readEvents(std::fstream& inFile) override;
	void writeEvents(std::fstream& outFile) const override;
	void readNoteTrack(std::fstream& inFile, const std::string& func) override;
	void writeNoteTracks(std::fstream& outFile) const override;
};

