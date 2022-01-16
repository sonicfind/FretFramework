#include "Chart.h"
template<>
bool WritableModifier<std::string>::read(const std::string& name, std::stringstream& ss)
{
	if (name.find(m_name) != std::string::npos)
	{
		ss.ignore(1, ' ');
		std::getline(ss, m_value);
		m_value = m_value.substr(1, m_value.length() - 2);
		return true;
	}
	return false;
}

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
			getElement(m_syncTracks, position).addEvent(position, ev.substr(2, ev.length() - 3));
	}
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
			getElement(m_syncTracks, position).readNote(ss, ins, diff, position);
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
