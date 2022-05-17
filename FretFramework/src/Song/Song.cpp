#include "Song.h"
#include "..\FilestreamCheck.h"
#include <iostream>
using namespace MidiFile;

Song::Song() : m_sync({ {0, SyncValues(true, true)} }) {}

Song::Song(const std::filesystem::path& filepath)
	: m_sync({ {0, SyncValues(true, true)} })
{
	load(filepath);
}

void Song::load(const std::filesystem::path& filepath)
{
	m_filepath = filepath;
	m_version = 1;
	if (m_filepath.extension() == ".chart" || m_filepath.extension() == ".cht")
		loadFile_Cht();
	else if (m_filepath.extension() == ".mid" || m_filepath.extension() == "midi")
		loadFile_Midi();
	else if (m_filepath.extension() == ".bch")
		loadFile_Bch();
	else
		throw InvalidExtensionException(m_filepath.extension().string());
	m_version = 2;
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
			case 'n':
			case 'N':
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
	m_hopo_frequency *= multiplier;
	Sustainable::setForceThreshold(m_hopo_frequency);
	m_sustain_cutoff_threshold *= multiplier;
	Sustainable::setsustainThreshold(m_sustain_cutoff_threshold);

	m_leadGuitar.adjustTicks(multiplier);
	m_leadGuitar_6.adjustTicks(multiplier);
	m_bassGuitar.adjustTicks(multiplier);
	m_bassGuitar_6.adjustTicks(multiplier);
	m_coopGuitar.adjustTicks(multiplier);
	m_rhythmGuitar.adjustTicks(multiplier);
	m_drums.adjustTicks(multiplier);
	m_vocals.adjustTicks(multiplier);
	m_harmonies.adjustTicks(multiplier);
}
