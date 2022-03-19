#include "MidiFile.h"

namespace MidiFile
{
	MidiChunk_Track::MidiEvent::MidiEvent(unsigned char syntax, bool running)
		: m_syntax(syntax)
		, m_isRunningStatus(running) {}

	uint32_t MidiChunk_Track::MidiEvent::getSize() const
	{
		if (!m_isRunningStatus)
			return 1;
		else
			return 0;
	}

	MidiChunk_Track::MidiEvent_Note::MidiEvent_Note(unsigned char syntax, std::fstream& inFile, bool running)
		: MidiEvent(syntax, running)
	{
		inFile.read((char*)&m_note, 1);
		inFile.read((char*)&m_velocity, 1);
	}

	uint32_t MidiChunk_Track::MidiEvent_Note::getSize() const
	{
		return MidiEvent::getSize() + 2;
	}

	MidiChunk_Track::MidiEvent_ControlChange::MidiEvent_ControlChange(unsigned char syntax, std::fstream& inFile, bool running)
		: MidiEvent(syntax, running)
	{
		inFile.read((char*)&m_controller, 1);
		inFile.read((char*)&m_newValue, 1);
	}

	uint32_t MidiChunk_Track::MidiEvent_ControlChange::getSize() const
	{
		return MidiEvent::getSize() + 2;
	}

	MidiChunk_Track::MidiEvent_Single::MidiEvent_Single(unsigned char syntax, std::fstream& inFile, bool running)
		: MidiEvent(syntax, running)
	{
		inFile.read((char*)&m_value, 1);
	}

	uint32_t MidiChunk_Track::MidiEvent_Single::getSize() const
	{
		return MidiEvent::getSize() + 1;
	}

	MidiChunk_Track::MidiEvent_Double::MidiEvent_Double(unsigned char syntax, std::fstream& inFile, bool running)
		: MidiEvent(syntax, running)
	{
		inFile.read((char*)&m_value_1, 1);
		inFile.read((char*)&m_value_2, 1);
	}

	uint32_t MidiChunk_Track::MidiEvent_Double::getSize() const
	{
		return MidiEvent::getSize() + 2;
	}
}
