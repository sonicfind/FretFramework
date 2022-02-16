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

class Song
{
protected:
	std::string m_filename;

	std::map<uint32_t, SyncValues> m_sync;
	std::map<uint32_t, std::string> m_sectionMarkers;
	std::map<uint32_t, std::vector<std::string>> m_globalEvents;

	NodeTrack<GuitarNote_5Fret> m_leadGuitar;
	NodeTrack<GuitarNote_6Fret> m_leadGuitar_6;
	NodeTrack<GuitarNote_5Fret> m_bassGuitar;
	NodeTrack<GuitarNote_6Fret> m_bassGuitar_6;
	NodeTrack<GuitarNote_5Fret> m_rhythmGuitar;
	NodeTrack<GuitarNote_5Fret> m_coopGuitar;
	NodeTrack<DrumNote<4, DrumPad_Pro>> m_drums;
	NodeTrack<DrumNote<5, DrumPad>> m_drums_5Lane;
	
public:
	Song(const std::string& filename);
	void fill(std::fstream& inFile);
	std::string getFilename();
	void setFilename(const std::string& filename);
	virtual void save() const = 0;

protected:
	void save(std::fstream& outFile) const;
	virtual void readMetadata(std::fstream& inFile) = 0;
	virtual void writeMetadata(std::fstream& outFile) const = 0;
	virtual void readSync(std::fstream& inFile) = 0;
	virtual void writeSync(std::fstream& outFile) const = 0;
	virtual void readEvents(std::fstream& inFile) = 0;
	virtual void writeEvents(std::fstream& outFile) const = 0;
	virtual void readNoteTrack(std::fstream& inFile, const std::string& func) = 0;
	virtual void writeNoteTracks(std::fstream& outFile) const = 0;
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
