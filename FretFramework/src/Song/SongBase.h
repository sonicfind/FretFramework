#pragma once
#include "Modifiers/Modifiers.h"
#include "Chords/GuitarNote/GuitarNote_cht.hpp"
#include "Chords/GuitarNote/GuitarNote_bch.hpp"
#include "Chords/Keys.h"
#include "Drums/DrumNote_cht.hpp"
#include "Drums/DrumNote_bch.hpp"
#include "Tracks/InstrumentalTracks/InstrumentalTrack_cht.hpp"
#include "Tracks/InstrumentalTracks/InstrumentalTrack_bch.hpp"
#include "Tracks/InstrumentalTracks/DrumTrack/DrumTrack_Legacy.h"
#include "Tracks/VocalTracks/VocalTrack_cht.hpp"
#include "Tracks/VocalTracks/VocalTrack_bch.hpp"
#include "Ini/IniFile.h"
#include "FileHasher/FileHasher.h"
#include <filesystem>
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

class SongBase
{
protected:
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
	static FileHasher s_fileHasher;

	std::filesystem::path m_filepath;
	std::shared_ptr<MD5> m_hash;

	IniFile m_ini;

	NumberModifier<float> m_offset{ "Offset" };
	NumberModifier<uint16_t> m_version_cht{ "FileVersion", 2 };
	uint16_t m_version_bch = 1;

	struct
	{
		StringModifier          name{ "Name" };
		StringModifier          artist{ "Artist" };
		StringModifier          charter{ "Charter" };
		StringModifier          album{ "Album" };
		StringModifier          year{ "Year" };
		NumberModifier<int16_t> difficulty{ "Difficulty" };
		NumberModifier<float>   preview_start_time{ "PreviewStart" };
		NumberModifier<float>   preview_end_time{ "PreviewEnd" };
		StringModifier          genre{ "Genre" };
	} m_songInfo;

public:
	SongBase();

	std::filesystem::path getPath() const { return m_filepath; }
	virtual void wait();
	MD5 getHash() {
		m_hash->wait();
		return *m_hash;
	}

	static void deleteTracks();
};
