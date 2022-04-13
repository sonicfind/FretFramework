#include "MidiFile.h"

namespace MidiFile
{
	MidiChunk_Track::MidiChunk_Track(const std::string& name)
		: MidiChunk("MTrk")
	{
		m_events[0].push_back(new MetaEvent_Text(3, name));
	}

	MidiChunk_Track::MidiChunk_Track()
		: MidiChunk("MTrk")
	{
	}

	MidiChunk_Track::~MidiChunk_Track()
	{
		for (auto& vec : m_events)
			for (auto ev : vec.second)
				delete ev;
	}

	void MidiChunk_Track::writeToFile(std::fstream& outFile)
	{
		// Need this position to jump back to
		std::streampos start = outFile.tellp();
		MidiChunk::writeToFile(outFile);

		uint32_t position = 0;
		// For utilizing the running MidiEvent property
		unsigned char prevSyntax = 0;
		for (const auto& vec : m_events)
		{
			VariableLengthQuantity delta(vec.first - position);
			for (const auto& ev : vec.second)
			{
				delta.writeToFile(outFile);
				ev->writeToFile(prevSyntax, outFile);
				delta = 0;
			}
			position = vec.first;
		}

		// Closes out the track with an End Event
		VariableLengthQuantity(0).writeToFile(outFile);
		MetaEvent_End().writeToFile(prevSyntax, outFile);

		std::streampos end = outFile.tellp();
		m_header.length = (uint32_t)(end - start) - 8;
		outFile.seekp(start);
		// Write the chunk's header data a second time but with the actual length of the chunk
		MidiChunk::writeToFile(outFile);
		outFile.seekp(end);
	}

    void MidiChunk_Track::addEvent(uint32_t position, MidiEvent* ev)
    {
		m_events[position].push_back(ev);
    }

	MidiChunk_Track::MidiEvent* MidiChunk_Track::parseEvent(unsigned char& syntax, std::fstream& inFile)
	{
		unsigned char tmpSyntax = 0;
		inFile.read((char*)&tmpSyntax, 1);
		if (tmpSyntax & 0b10000000)
			syntax = tmpSyntax;
		else
			inFile.seekg(-1, std::ios_base::cur);

		switch (syntax)
		{
		case 0xFF:
		{
			unsigned char type = 0;
			inFile.read((char*)&type, 1);
			if (type < 16)
				return static_cast<MidiChunk_Track::MidiEvent*>(new MidiChunk_Track::MetaEvent_Text(type, inFile));
			else
			{
				switch (type)
				{
				case 0x20:
					return static_cast<MidiChunk_Track::MidiEvent*>(new MidiChunk_Track::MetaEvent_ChannelPrefix(inFile));
				case 0x2F:
					return static_cast<MidiChunk_Track::MidiEvent*>(new MidiChunk_Track::MetaEvent_End(inFile));
				case 0x51:
					return static_cast<MidiChunk_Track::MidiEvent*>(new MidiChunk_Track::MetaEvent_Tempo(inFile));
				case 0x54:
					return static_cast<MidiChunk_Track::MidiEvent*>(new MidiChunk_Track::MetaEvent_SMPTE(inFile));
				case 0x58:
					return static_cast<MidiChunk_Track::MidiEvent*>(new MidiChunk_Track::MetaEvent_TimeSignature(inFile));
				case 0x59:
					return static_cast<MidiChunk_Track::MidiEvent*>(new MidiChunk_Track::MetaEvent_KeySignature(inFile));
				default:
					return static_cast<MidiChunk_Track::MidiEvent*>(new MidiChunk_Track::MetaEvent_Data(type, inFile));
				}
			}
			break;
		}
		case 0xF0:
		case 0xF7:
			return static_cast<MidiChunk_Track::MidiEvent*>(new MidiChunk_Track::SysexEvent(syntax, inFile));
		case 0x80:
		case 0x90:
			return static_cast<MidiChunk_Track::MidiEvent*>(new MidiChunk_Track::MidiEvent_Note(syntax, inFile));
		case 0xB0:
			return static_cast<MidiChunk_Track::MidiEvent*>(new MidiChunk_Track::MidiEvent_ControlChange(syntax, inFile));
		case 0xC0:
		case 0xD0:
		case 0xF3:
			return static_cast<MidiChunk_Track::MidiEvent*>(new MidiChunk_Track::MidiEvent_Single(syntax, inFile));
		case 0xA0:
		case 0xE0:
		case 0xF2:
			return static_cast<MidiChunk_Track::MidiEvent*>(new MidiChunk_Track::MidiEvent_Double(syntax, inFile));
		default:
			return static_cast<MidiChunk_Track::MidiEvent*>(new MidiChunk_Track::MidiEvent(syntax));
		}
	}
}
