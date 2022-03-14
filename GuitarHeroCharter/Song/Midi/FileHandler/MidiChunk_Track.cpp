#include "MidiFile.h"

namespace MidiFile
{
	MidiChunk_Track::MidiChunk_Track(std::fstream& inFile)
		: MidiChunk(inFile)
	{
		uint32_t index = 0;
		uint32_t position = 0;
		unsigned char syntax = (char)0x90;
		while (index < m_header.length)
		{
			VariableLengthQuantity delta(inFile);
			position += delta;
			m_events.try_emplace(position);
			unsigned char tmpSyntax;
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
						m_name = text->m_data;
				}
				else
				{
					switch (type)
					{
					case 0x20:
						m_events.at(position).push_back(new MetaEvent_ChannelPrefix(type, inFile));
						break;
					case 0x2F:
						m_events.at(position).push_back(new MetaEvent_End(type, inFile));
						return;
					case 0x51:
						m_events.at(position).push_back(new MetaEvent_Tempo(type, inFile));
						break;
					case 0x54:
						m_events.at(position).push_back(new MetaEvent_SMPTE(type, inFile));
						break;
					case 0x58:
						m_events.at(position).push_back(new MetaEvent_TimeSignature(type, inFile));
						break;
					case 0x59:
						m_events.at(position).push_back(new MetaEvent_KeySignature(type, inFile));
						break;
					case 0x7F:
						m_events.at(position).push_back(new MetaEvent_Data(type, inFile));
						break;
					default:
						m_events.at(position).push_back(new MetaEvent(type, inFile));
					}
				}
				break;
			}
			case 0xF0:
				m_events.at(position).push_back(new SysexEvent(syntax, inFile, true));
				break;
			case 0x80:
			case 0x90:
				m_events.at(position).push_back(new MidiEvent_Note(syntax, inFile, !(tmpSyntax & 0b10000000)));
				break;
			case 0xB0:
				m_events.at(position).push_back(new MidiEvent_ControlChange(syntax, inFile, !(tmpSyntax & 0b10000000)));
				break;
			}
			index += m_events.at(position).back()->getSize() + delta.getSize();
		}
	}

	MidiChunk_Track::~MidiChunk_Track()
	{
		for (auto& vec : m_events)
			for (auto ev : vec.second)
				delete ev;
	}

	void MidiChunk_Track::writeToFile(std::fstream& outFile) const
	{
	}
}
