#include "MidiFile.h"

namespace MidiFile
{
	MidiChunk_Track::MidiEvent::MidiEvent(char syntax, bool running = false)
		: m_syntax(syntax)
		, m_isRunningStatus(running) {}

	uint32_t MidiChunk_Track::MidiEvent::getSize() const
	{
		if (!m_isRunningStatus)
			return 1;
		else
			return 0;
	}

	void MidiChunk_Track::MidiEvent_Note::fillFromFile(std::fstream& inFile)
	{
		inFile >> m_note;
		inFile >> m_velocity;
	}

	uint32_t MidiChunk_Track::MidiEvent_Note::getSize() const
	{
		return MidiEvent::getSize() + 2;
	}
}
