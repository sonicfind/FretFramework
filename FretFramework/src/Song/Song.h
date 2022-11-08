#pragma once
#include "SongListEntry.h"
#include "Tracks/InstrumentalTracks/InstrumentalTrack_cht.hpp"
#include "Tracks/InstrumentalTracks/InstrumentalTrack_bch.hpp"
#include "Tracks/InstrumentalTracks/DrumTrack/DrumTrack_Legacy.h"
#include "Tracks/VocalTracks/VocalTrack_cht.hpp"
#include "Tracks/VocalTracks/VocalTrack_bch.hpp"

#include "Sync/SyncValues.h"
#include <filesystem>

class Song
{
	static SongListEntry s_baseEntry;

	struct
	{
		InstrumentalTrack<GuitarNote<5>>            lead_5    { "[LeadGuitar]", 0 };
		InstrumentalTrack<GuitarNote<6>>            lead_6    { "[LeadGuitar_GHL]", 1 };
		InstrumentalTrack<GuitarNote<5>>            bass_5    { "[BassGuitar]", 2 };
		InstrumentalTrack<GuitarNote<6>>            bass_6    { "[BassGuitar]", 3 };
		InstrumentalTrack<GuitarNote<5>>            rhythm    { "[RhythmGuitar]", 4 };
		InstrumentalTrack<GuitarNote<5>>            coop      { "[CoopGuitar]", 5 };
		InstrumentalTrack<Keys<5>>                  keys      { "[Keys]", 6 };
		InstrumentalTrack<DrumNote<4, DrumPad_Pro>> drums4_pro{ "[Drums_4Lane]", 7 };
		InstrumentalTrack<DrumNote<5, DrumPad>>     drums5    { "[Drums_5Lane]", 8 };
		VocalTrack<1>                               vocals    { "[Vocals]", 9 };
		VocalTrack<3>                               harmonies { "[Harmonies]", 10 };

		// 0/1 -  Guitar 5/6
		// 2/3 -  Bass 5/6
		// 4   -  Rhythm
		// 5   -  Co-op
		// 6   -  Keys
		// 7/8 -  Drums 4/5
		// 9/10 - Vocals/Harmonies
		NoteTrack* const trackArray[11] =
		{
			&lead_5,
			&lead_6,
			&bass_5,
			&bass_6,
			&rhythm,
			&coop,
			&keys,
			&drums4_pro,
			&drums5,
			&vocals,
			&harmonies
		};
	} m_noteTracks;

	uint16_t m_tickrate = 192;
	std::vector<std::pair<uint32_t, SyncValues>> m_sync;
	std::vector<std::pair<uint32_t, UnicodeString>> m_sectionMarkers;
	std::vector<std::pair<uint32_t, std::vector<std::u32string>>> m_globalEvents;

	std::u32string m_midiSequenceName;
	SongListEntry* m_currentSongEntry = &s_baseEntry;

public:
	void newSong();
	void loadFrom(const std::filesystem::path& chartPath);
	void loadFrom(SongListEntry* const entry);
	void save();

	void reset();

	
	void setTickRate(uint16_t tickRate);

	class InvalidFileException : public std::runtime_error
	{
	public:
		InvalidFileException(const std::string& file) : std::runtime_error("Error: \"" + file + "\" is not a valid chart file") {}
	};

private:
	static constexpr uint16_t S_VERSION_BCH = 1;
	static constexpr uint16_t s_VERSION_CHT = 2;

	void load();

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
