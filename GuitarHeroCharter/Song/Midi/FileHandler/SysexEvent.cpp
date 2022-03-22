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

    void MidiChunk_Track::SysexEvent::writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const
    {
		// Reordered overload that skips over the syntax check
		MidiEvent::writeToFile(outFile, prevSyntax);
		m_length.writeToFile(outFile);
		if (m_data)
			outFile.write(m_data, m_length);
    }

	MidiChunk_Track::SysexEvent::~SysexEvent()
	{
		if (m_data)
			delete[m_length + 1] m_data;
	}
}
