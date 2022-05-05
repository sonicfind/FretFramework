#include "MidiFile.h"

namespace MidiFile
{
	MidiChunk_Track::MidiEvent::MidiEvent(unsigned char syntax)
		: m_syntax(syntax) {}

    void MidiChunk_Track::MidiEvent::writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const
    {
		if (m_syntax != prevSyntax)
			outFile.write((char*)&m_syntax, 1);
		prevSyntax = m_syntax;
    }

	void MidiChunk_Track::MidiEvent::writeToFile(std::fstream& outFile, unsigned char& prevSyntax) const
	{
		outFile.write((char*)&m_syntax, 1);
		prevSyntax = m_syntax;
	}

	// Going the route of using Note On and Note Off syntaxes instead of velocity for instruments
	MidiChunk_Track::MidiEvent_Note::MidiEvent_Note(unsigned char syntax, unsigned char note, unsigned char velocity)
		: MidiEvent(syntax)
		, m_note(note)
		, m_velocity(velocity) {}

	void MidiChunk_Track::MidiEvent_Note::writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const
	{
		MidiEvent::writeToFile(prevSyntax, outFile);
		outFile.write((char*)&m_note, 1);
		outFile.write((char*)&m_velocity, 1);
	}

	void MidiChunk_Track::MidiEvent_ControlChange::writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const
	{
		MidiEvent::writeToFile(prevSyntax, outFile);
		outFile.write((char*)&m_controller, 1);
		outFile.write((char*)&m_newValue, 1);
	}
}
