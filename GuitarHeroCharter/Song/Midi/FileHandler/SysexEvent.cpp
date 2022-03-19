#include "MidiFile.h"

namespace MidiFile
{
	MidiChunk_Track::SysexEvent::SysexEvent(unsigned char syntax, std::fstream& inFile)
		: MidiEvent(syntax)
		, m_length(inFile)
		, m_data(new char[m_length + 1]())
	{
		inFile.read(m_data, m_length);
	}

	MidiChunk_Track::SysexEvent::~SysexEvent()
	{
		if (m_data)
			delete[m_length + 1] m_data;
	}

	uint32_t MidiChunk_Track::SysexEvent::getSize() const
	{
		return m_length.getValue() + m_length.getSize() + 1;
	}
}
