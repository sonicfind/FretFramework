#include "Song.h"
#include "FileChecks/FilestreamCheck.h"
#include <iostream>
using namespace MidiFile;

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
NoteTrack* const Song::s_noteTracks[11] =
{
	new InstrumentalTrack<GuitarNote<5>>           ("[LeadGuitar]", 0),
	new InstrumentalTrack<GuitarNote<6>>           ("[LeadGuitar_GHL]", 1),
	new InstrumentalTrack<GuitarNote<5>>           ("[BassGuitar]", 2),
	new InstrumentalTrack<GuitarNote<6>>           ("[BassGuitar_GHL]", 3),
	new InstrumentalTrack<GuitarNote<5>>           ("[RhythmGuitar]", 4),
	new InstrumentalTrack<GuitarNote<5>>           ("[CoopGuitar]", 5),
	new InstrumentalTrack<Keys<5>>                 ("[Keys]", 6),
	new InstrumentalTrack<DrumNote<4, DrumPad_Pro>>("[Drums_4Lane]", 7),
	new InstrumentalTrack<DrumNote<5, DrumPad>>    ("[Drums_5Lane]", 8),
	new VocalTrack<1>                              ("[Vocals]", 9),
	new VocalTrack<3>                              ("[Harmonies]", 10),
};

void Song::deleteTracks()
{
	for (NoteTrack* track : s_noteTracks)
		delete track;
}

Song::Song() : m_sync({ {0, SyncValues(true, true)} }) {}

Song::Song(const std::filesystem::path& filepath)
	: m_sync({ {0, SyncValues(true, true)} })
{
	load(filepath);
}

Song::~Song()
{
	for (NoteTrack* track : s_noteTracks)
		track->clear();
}

void Song::load(const std::filesystem::path& filepath)
{
	m_filepath = filepath;
	if (m_filepath.extension() == ".chart" || m_filepath.extension() == ".cht")
		loadFile_Cht();
	else if (m_filepath.extension() == ".mid" || m_filepath.extension() == "midi")
		loadFile_Midi();
	else if (m_filepath.extension() == ".bch")
		loadFile_Bch();
	else
		throw InvalidExtensionException(m_filepath.extension().string());
}

void Song::save() const
{
	try
	{
		std::filesystem::path outPath = m_filepath;
		while (true)
		{
			char answer = -1;
			std::cout << "Valid Options for Export: C - .cht | M - .mid | B - .bch\n";
			std::cout << "Q/q - Do not save\n";
			std::cout << "Select Chart Type: ";
			std::cin >> answer;
			std::cin.clear();
			switch (answer)
			{
			case 'c':
			case 'C':
				outPath.replace_extension(".cht");
				saveFile_Cht(outPath);
				return;
			case 'm':
			case 'M':
				outPath.replace_extension(".mid.test");
				saveFile_Midi(outPath);
				return;
			case 'b':
			case 'B':
				outPath.replace_extension(".bch");
				saveFile_Bch(outPath);
				return;
			case 'q':
			case 'Q':
				return;
			}
		}
	}
	catch (FilestreamCheck::InvalidFileException e)
	{

	}
}

std::filesystem::path Song::getFilepath()
{
	return m_filepath;
}

void Song::setFilepath(const std::filesystem::path& filename)
{
	m_filepath = filename;
}

void Song::setTickRate(uint16_t tickRate)
{
	float multiplier = float(tickRate) / m_tickrate;
	m_tickrate = tickRate;
	for (auto& sync : m_sync)
		sync.first = uint32_t(sync.first * multiplier);
	for (auto& sect : m_sectionMarkers)
		sect.first = uint32_t(sect.first * multiplier);
	for (auto& vec : m_globalEvents)
		vec.first = uint32_t(vec.first * multiplier);

	// Sets the threshold default for forcing guitar notes and for sustains
	// Automatically sets each threshold to 1/3 of the tickrate if they are at their default values
	m_ini.m_hopo_frequency *= multiplier;
	Sustainable::setForceThreshold(m_ini.m_hopo_frequency);
	m_ini.m_sustain_cutoff_threshold *= multiplier;
	Sustainable::setsustainThreshold(m_ini.m_sustain_cutoff_threshold);

	for (auto& track : s_noteTracks)
		track->adjustTicks(multiplier);
}
