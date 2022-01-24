#include "Chart.h"


Chart::Chart(std::ifstream& inFile)
{
	std::string line;
	while (std::getline(inFile, line))
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

void Chart::write_chart(std::ofstream& outFile) const
{
	m_iniData.write(outFile);
	writeSync(outFile);
	writeEvents(outFile);
	writeNoteTracks_chart(outFile);
}

void Chart::readMetadata(std::ifstream& inFile)
{
	std::string line;
	while (std::getline(inFile, line) && line.find('}') == std::string::npos)
	{
		std::stringstream ss(line);
		m_iniData.read(ss);
	}
}

void Chart::readSync(std::ifstream& inFile)
{
	std::string line;
	while (std::getline(inFile, line) && line.find('}') == std::string::npos)
	{
		std::stringstream ss(line);
		uint32_t position;
		ss >> position;
		ss.ignore(5, '=');
		m_syncTracks[position].setSyncValues(ss);
	}
}

void Chart::writeSync(std::ofstream& outFile) const
{
	outFile << "[SyncTrack]\n{\n";
	const SyncTrack* prev = nullptr;
	for (const auto& sync : m_syncTracks)
	{
		sync.second.writeSync(sync.first, outFile, prev);
		prev = &sync.second;
	}
	outFile << "}\n";
}

void Chart::readEvents(std::ifstream& inFile)
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
			getElement(m_syncTracks, position)->second.addEvent(position, ev.substr(2, ev.length() - 3));
	}
}

void Chart::writeEvents(std::ofstream& outFile) const
{
	auto sectIter = m_sectionMarkers.begin();
	uint32_t end = sectIter != m_sectionMarkers.end() ? sectIter->first : UINT32_MAX;
	auto writeSection = [&]()
	{
		outFile << "  " << sectIter->first << " = E \"section " << sectIter->second << "\"\n";
		++sectIter;
		if (sectIter != m_sectionMarkers.end())
			end = sectIter->first;
		else
			end = UINT32_MAX;
	};

	outFile << "[Events]\n{\n";
	for (auto trackIter = m_syncTracks.begin(); trackIter != m_syncTracks.end();)
	{
		auto curr = trackIter++;
		uint32_t start = curr->first;
		if (start == end)
			writeSection();

		while (trackIter == m_syncTracks.end() || start < trackIter->first)
		{
			bool sectionWasHit = curr->second.writeEvents(start, end, outFile);
			start = end;
			if (sectionWasHit)
				writeSection();
			else
			{
				while (sectIter != m_sectionMarkers.end() &&
					(trackIter == m_syncTracks.end() || end < trackIter->first))
					writeSection();
				break;
			}
		}
	}
	outFile << "}\n";
}

void Chart::readNoteTrack(std::ifstream& inFile, const std::string& func)
{
	std::string line;
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

	DifficultyLevel diff = DifficultyLevel::Expert;
	if (func.find("Expert") != std::string::npos)
		diff = DifficultyLevel::Expert;
	else if (func.find("Hard") != std::string::npos)
		diff = DifficultyLevel::Hard;
	else if (func.find("Medium") != std::string::npos)
		diff = DifficultyLevel::Medium;
	else if (func.find("Easy") != std::string::npos)
		diff = DifficultyLevel::Easy;

	while (std::getline(inFile, line) && line.find('}') == std::string::npos)
	{
		if (ins != Instrument::None)
		{
			std::stringstream ss(line);
			uint32_t position;
			ss >> position;
			ss.ignore(5, '=');
			getElement(m_syncTracks, position)->second.readNote(position, ins, diff, ss);
		}
	}
}

void Chart::writeNoteTracks_chart(std::ofstream& outFile) const
{
	static std::string_view difficultyStrings[] = { "Expert", "Hard", "Medium", "Easy" };
	static std::string_view instrumentStrings[] = { "Single", "GHLGuitar", "DoubleBass", "GHLBass", "DoubleRhythm", "DoubleGuitar", "Drums", "Drums5Lane"};
	for (int ins = 0; ins < static_cast<int>(Instrument::Vocals); ++ins)
	{
		for (int diff = 0; diff < 4; ++diff)
		{
			auto iter = m_syncTracks.begin();
			while (iter != m_syncTracks.end() && !iter->second.hasNotes(static_cast<Instrument>(ins), diff))
				++iter;

			if (iter != m_syncTracks.end())
			{
				outFile << "[" << difficultyStrings[diff] << instrumentStrings[ins] << "]\n{\n";
				while (iter != m_syncTracks.end())
				{
					iter->second.writeNoteTracks_chart(static_cast<Instrument>(ins), diff, outFile);
					++iter;
				}
				outFile << "}\n";
			}
		}
	}
	
}

bool Chart::IniData::read(std::stringstream& ss)
{
	std::string str;
	ss >> str;
	ss.ignore(5, '=');

	return m_songInfo.name.read(str, ss) ||
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
		m_songInfo.media_type.read(str, ss) ||

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

void Chart::IniData::write(std::ofstream& outFile) const
{
	outFile << "[Song]\n{\n";
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
	m_songInfo.media_type.write(outFile);

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
