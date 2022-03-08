#include "Chart.h"
#include "..\..\FilestreamCheck.h"
void Chart::save() const
{
	Song::save(std::ios_base::out | std::ios_base::trunc);
}

void Chart::fillFromFile(std::fstream& inFile)
{
	std::string line;
	while (std::getline(inFile, line))
	{
		// Ensures that we're entering a scope
		if (line.find('[') != std::string::npos)
		{
			// Skip '{' line
			inFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

			if (line.find("Song") != std::string::npos)
				readMetadata(inFile);
			else if (line.find("SyncTrack") != std::string::npos)
				readSync(inFile);
			else if (line.find("Events") != std::string::npos)
				readEvents(inFile);
			else
				readNoteTrack(inFile, line);
		}
	}
}

void Chart::readMetadata(std::fstream& inFile)
{
	std::string line;
	while (std::getline(inFile, line) && line.find('}') == std::string::npos)
	{
		std::stringstream ss(line);
		std::string str;
		ss >> str;
		ss.ignore(5, '=');

		// Utilize short circuiting to stop if a read was valid
		m_songInfo.name.read(str, ss) ||
		m_songInfo.artist.read(str, ss) ||
		m_songInfo.charter.read(str, ss) ||
		m_songInfo.album.read(str, ss) ||
		m_songInfo.year.read(str, ss) ||

		offset.read(str, ss) ||
		ticks_per_beat.read(str, ss) ||

		m_songInfo.difficulty.read(str, ss) ||
		m_songInfo.preview_start_time.read(str, ss) ||
		m_songInfo.preview_end_time.read(str, ss) ||
		m_songInfo.genre.read(str, ss) ||

		m_audioStreams.music.read(str, ss) ||
		m_audioStreams.guitar.read(str, ss) ||
		m_audioStreams.bass.read(str, ss) ||
		m_audioStreams.rhythm.read(str, ss) ||
		m_audioStreams.keys.read(str, ss) ||
		m_audioStreams.drum.read(str, ss) ||
		m_audioStreams.drum_2.read(str, ss) ||
		m_audioStreams.drum_3.read(str, ss) ||
		m_audioStreams.drum_4.read(str, ss) ||
		m_audioStreams.vocals.read(str, ss) ||
		m_audioStreams.crowd.read(str, ss);
	}
}

void Chart::writeMetadata(std::fstream& outFile) const
{
	outFile << "[Chart]\n{\n";
	m_songInfo.name.write(outFile);
	m_songInfo.artist.write(outFile);
	m_songInfo.charter.write(outFile);
	m_songInfo.album.write(outFile);
	m_songInfo.year.write(outFile);

	offset.write(outFile);
	ticks_per_beat.write(outFile);

	m_songInfo.difficulty.write(outFile);
	m_songInfo.preview_start_time.write(outFile);
	m_songInfo.preview_end_time.write(outFile);
	m_songInfo.genre.write(outFile);

	m_audioStreams.music.write(outFile);
	m_audioStreams.guitar.write(outFile);
	m_audioStreams.bass.write(outFile);
	m_audioStreams.rhythm.write(outFile);
	m_audioStreams.keys.write(outFile);
	m_audioStreams.drum.write(outFile);
	m_audioStreams.drum_2.write(outFile);
	m_audioStreams.drum_3.write(outFile);
	m_audioStreams.drum_4.write(outFile);
	m_audioStreams.vocals.write(outFile);
	m_audioStreams.crowd.write(outFile);
	outFile << "}\n";
}

void Chart::readSync(std::fstream& inFile)
{
	std::string line;
	// Default the first sync node to always mark
	SyncValues prev(true, true);
	while (std::getline(inFile, line) && line.find('}') == std::string::npos)
	{
		std::stringstream ss(line);
		uint32_t position;
		ss >> position;
		ss.ignore(5, '=');
		// Starts the values at the current location with the previous set of values
		// Only if there is no node already here
		m_sync.insert({ position, prev });
		
		std::string type;
		ss >> type;
		if (type[0] == 'T')
		{
			uint32_t numerator, denom;
			ss >> numerator >> denom;
			m_sync.at(position).setTimeSig(numerator, denom);
		}
		else if (type[0] == 'B')
		{
			float bpm;
			ss >> bpm;
			m_sync.at(position).setBPM(bpm * .001f);
		}
		prev = m_sync.at(position);
		prev.unmarkBPM();
		prev.unmarkTimeSig();
	}
}

void Chart::writeSync(std::fstream& outFile) const
{
	outFile << "[SyncTrack]\n{\n";
	for (const auto& sync : m_sync)
		sync.second.writeSync_chart(sync.first, outFile);
	outFile << "}\n";
}

void Chart::readEvents(std::fstream& inFile)
{
	std::string line;
	while (std::getline(inFile, line) && line.find('}') == std::string::npos)
	{
		std::stringstream ss(line);
		uint32_t position;
		ss >> position;
		ss.ignore(10, 'E');

		std::string ev;
		std::getline(ss, ev);
		// Substr calls to remove leading spaces and "" characters
		if (ev.find("section") != std::string::npos)
			m_sectionMarkers[position] = ev.substr(10, ev.length() - 11);
		else
			m_globalEvents[position].push_back(ev.substr(2, ev.length() - 3));
	}
}

void Chart::writeEvents(std::fstream& outFile) const
{
	outFile << "[Events]\n{\n";
	auto sectIter = m_sectionMarkers.begin();
	for (auto eventIter = m_globalEvents.begin(); eventIter != m_globalEvents.end(); ++eventIter)
	{
		while (sectIter != m_sectionMarkers.end() && sectIter->first <= eventIter->first)
		{
			outFile << "  " << sectIter->first << " = E \"section " << sectIter->second << "\"\n";
			++sectIter;
		}

		for (const auto& str : eventIter->second)
			outFile << "  " << eventIter->first << " = E \"" << str << "\"\n";
	}

	while (sectIter != m_sectionMarkers.end())
	{
		outFile << "  " << sectIter->first << " = E \"section " << sectIter->second << "\"\n";
		++sectIter;
	}
	outFile << "}\n";
}

void Chart::readNoteTrack(std::fstream& inFile, const std::string& func)
{
	Instrument ins = Instrument::None;
	if (func.find("Single") != std::string::npos)
		ins = Instrument::Guitar_lead;
	else if (func.find("DoubleGuitar") != std::string::npos)
		ins = Instrument::Guitar_coop;
	else if (func.find("DoubleBass") != std::string::npos)
		ins = Instrument::Guitar_bass;
	else if (func.find("DoubleRhythm") != std::string::npos)
		ins = Instrument::Guitar_rhythm;
	else if (func.find("Drums") != std::string::npos)
		ins = Instrument::Drums;
	else if (func.find("GHLGuitar") != std::string::npos)
		ins = Instrument::Guitar_lead_6;
	else if (func.find("GHLBass") != std::string::npos)
		ins = Instrument::Guitar_bass_6;
	else
	{
		inFile.ignore(std::numeric_limits<std::streamsize>::max(), '}');
		inFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		return;
	}

	int diff = 0;
	if (func.find("Expert") == std::string::npos)
	{
		if (func.find("Hard") != std::string::npos)
			diff = 1;
		else if (func.find("Medium") != std::string::npos)
			diff = 2;
		else if (func.find("Easy") != std::string::npos)
			diff = 3;
	}

	bool version2 = m_filepath.extension() == ".chart2";
	switch (ins)
	{
	case Instrument::Guitar_lead:
		m_leadGuitar[diff].read_chart(inFile, version2);
		break;
	case Instrument::Guitar_lead_6:
		m_leadGuitar_6[diff].read_chart(inFile, version2);
		break;
	case Instrument::Guitar_bass:
		m_bassGuitar[diff].read_chart(inFile, version2);
		break;
	case Instrument::Guitar_bass_6:
		m_bassGuitar_6[diff].read_chart(inFile, version2);
		break;
	case Instrument::Guitar_rhythm:
		m_rhythmGuitar[diff].read_chart(inFile, version2);
		break;
	case Instrument::Guitar_coop:
		m_coopGuitar[diff].read_chart(inFile, version2);
		break;
	case Instrument::Drums:
		m_drums[diff].read_chart(inFile, version2);
		break;
	case Instrument::Drums_5:
		m_drums_5Lane[diff].read_chart(inFile, version2);
	}
}

void Chart::writeNoteTracks(std::fstream& outFile) const
{
	m_leadGuitar.write_chart("Single", outFile);
	m_leadGuitar_6.write_chart("GHLGuitar", outFile);
	m_bassGuitar.write_chart("DoubleBass", outFile);
	m_bassGuitar_6.write_chart("GHLBass", outFile);
	m_rhythmGuitar.write_chart("DoubleRhythm", outFile);
	m_coopGuitar.write_chart("DoubleGuitar", outFile);
	m_drums.write_chart("Drums", outFile);
	m_drums_5Lane.write_chart("Drums5Lane", outFile);
}
