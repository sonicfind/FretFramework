#include "MidiFile.h"

namespace MidiFile
{
	MidiChunk_Track::SysexEvent::SysexEvent(unsigned char diff, unsigned char id, unsigned char status)
		: MidiEvent(0xF0)
		, m_difficulty(diff)
		, m_phraseID(id)
		, m_status(status) {}

    void MidiChunk_Track::SysexEvent::writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const
    {
		
		// Reordered overload that skips over the syntax check
		MidiEvent::writeToFile(outFile, prevSyntax);
		unsigned char length = 8;
		outFile << length;
		outFile.write((char*)this, 8);
    }
}
