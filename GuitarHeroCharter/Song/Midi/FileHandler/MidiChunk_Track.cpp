#include "MidiFile.h"

namespace MidiFile
{
	MidiChunk_Track::MidiChunk_Track(std::fstream& inFile)
		: MidiChunk(inFile)
	{
		std::streampos end = inFile.tellg() + std::streampos(m_header.length);
		uint32_t position = 0;
		unsigned char syntax = (char)0x90;
		while (inFile.tellg() < end)
		{
			VariableLengthQuantity delta(inFile);
			position += delta;
			m_events.try_emplace(position);
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
				{
					MetaEvent_Text* text = new MetaEvent_Text(type, inFile);
					m_events.at(position).push_back(text);
					if (type == 3)
						m_name = text->m_text;
				}
				else
				{
					switch (type)
					{
					case 0x20:
						m_events.at(position).push_back(new MetaEvent_ChannelPrefix(inFile));
						break;
					case 0x2F:
						m_events.at(position).push_back(new MetaEvent_End(inFile));
						return;
					case 0x51:
						m_events.at(position).push_back(new MetaEvent_Tempo(inFile));
						break;
					case 0x54:
						m_events.at(position).push_back(new MetaEvent_SMPTE(inFile));
						break;
					case 0x58:
						m_events.at(position).push_back(new MetaEvent_TimeSignature(inFile));
						break;
					case 0x59:
						m_events.at(position).push_back(new MetaEvent_KeySignature(inFile));
						break;
					default:
						m_events.at(position).push_back(new MetaEvent_Data(type, inFile));
					}
				}
				break;
			}
			case 0xF0:
			case 0xF7:
				m_events.at(position).push_back(new SysexEvent(syntax, inFile));
				break;
			case 0x80:
			case 0x90:
				m_events.at(position).push_back(new MidiEvent_Note(syntax, inFile));
				break;
			case 0xB0:
				m_events.at(position).push_back(new MidiEvent_ControlChange(syntax, inFile));
				break;
			case 0xC0:
			case 0xD0:
			case 0xF3:
				m_events.at(position).push_back(new MidiEvent_Single(syntax, inFile));
				break;
			case 0xA0:
			case 0xE0:
			case 0xF2:
				m_events.at(position).push_back(new MidiEvent_Double(syntax, inFile));
				break;
			default:
				m_events.at(position).push_back(new MidiEvent(syntax));
			}
		}
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

		std::streampos end = outFile.tellp();
		m_header.length = (uint32_t)(end - start) - 8;
		outFile.seekp(start);
		// Write the chunk's header data a second time but with the actual length of the chunk
		MidiChunk::writeToFile(outFile);
		outFile.seekp(end);
	}
}
