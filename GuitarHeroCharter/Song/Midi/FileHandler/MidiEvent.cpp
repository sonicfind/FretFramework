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

	void MidiChunk_Track::MidiEvent_Double::writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const
	{
		MidiEvent::writeToFile(prevSyntax, outFile);
		outFile.write((char*)&m_value_1, 1);
		outFile.write((char*)&m_value_2, 1);
	}
}
