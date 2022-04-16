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

	MidiChunk_Track::MidiEvent_Note::MidiEvent_Note(unsigned char syntax, std::fstream& inFile)
		: MidiEvent(syntax)
	{
		inFile.read((char*)&m_note, 1);
		inFile.read((char*)&m_velocity, 1);
	}

	MidiChunk_Track::MidiEvent_Note::MidiEvent_Note(unsigned char syntax, unsigned char note, std::fstream& inFile)
		: MidiEvent(syntax)
		, m_note(note)
	{
		inFile.read((char*)&m_velocity, 1);
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

	MidiChunk_Track::MidiEvent_ControlChange::MidiEvent_ControlChange(unsigned char syntax, std::fstream& inFile)
		: MidiEvent(syntax)
	{
		inFile.read((char*)&m_controller, 1);
		inFile.read((char*)&m_newValue, 1);
	}

	MidiChunk_Track::MidiEvent_ControlChange::MidiEvent_ControlChange(unsigned char syntax, unsigned char controller, std::fstream& inFile)
		: MidiEvent(syntax)
		, m_controller(controller)
	{
		inFile.read((char*)&m_newValue, 1);
	}

	void MidiChunk_Track::MidiEvent_ControlChange::writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const
	{
		MidiEvent::writeToFile(prevSyntax, outFile);
		outFile.write((char*)&m_controller, 1);
		outFile.write((char*)&m_newValue, 1);
	}

	MidiChunk_Track::MidiEvent_Single::MidiEvent_Single(unsigned char syntax, std::fstream& inFile)
		: MidiEvent(syntax)
	{
		inFile.read((char*)&m_value, 1);
	}

	MidiChunk_Track::MidiEvent_Single::MidiEvent_Single(unsigned char syntax, unsigned char value)
		: MidiEvent(syntax)
		, m_value(value)
	{
	}

	void MidiChunk_Track::MidiEvent_Single::writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const
	{
		MidiEvent::writeToFile(prevSyntax, outFile);
		outFile.write((char*)&m_value, 1);
	}

	MidiChunk_Track::MidiEvent_Double::MidiEvent_Double(unsigned char syntax, std::fstream& inFile)
		: MidiEvent(syntax)
	{
		inFile.read((char*)&m_value_1, 1);
		inFile.read((char*)&m_value_2, 1);
	}

	MidiChunk_Track::MidiEvent_Double::MidiEvent_Double(unsigned char syntax, unsigned char value_1, std::fstream& inFile)
		: MidiEvent(syntax)
		, m_value_1(value_1)
	{
		inFile.read((char*)&m_value_2, 1);
	}

	void MidiChunk_Track::MidiEvent_Double::writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const
	{
		MidiEvent::writeToFile(prevSyntax, outFile);
		outFile.write((char*)&m_value_1, 1);
		outFile.write((char*)&m_value_2, 1);
	}
}
