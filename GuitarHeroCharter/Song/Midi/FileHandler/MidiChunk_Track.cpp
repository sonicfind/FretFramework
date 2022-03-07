#include "MidiFile.h"

namespace MidiFile
{
	MidiChunk_Track::MidiChunk_Track(std::fstream& inFile)
		: MidiChunk(inFile)
	{
		int index = 0;
		uint32_t position = 0;
		char syntax = 0x90;
		while (index < m_header.length)
		{
			VariableLengthQuantity delta(inFile);
			position += delta;
			char tmpSyntax;
			inFile >> tmpSyntax;
			if (tmpSyntax & 0b10000000)
				syntax = tmpSyntax;
			else
				inFile.seekg(-1, std::ios_base::cur);

			switch (syntax)
			{
			case 0xFF:
			{
				char type;
				inFile >> type;
				if (type < 16)
				{
					MetaEvent_Text* text = new MetaEvent_Text(type, inFile);
					m_events[position].push_back(text);
					if (type == 3)
						m_name = text->m_data;
				}
				else
				{
					switch (type)
					{
					case 0x20:
						m_events[position].push_back(new MetaEvent_ChannelPrefix(type, inFile));
						break;
					case 0x2F:
						m_events[position].push_back(new MetaEvent_End(type, inFile));
						return;
					case 0x51:
						m_events[position].push_back(new MetaEvent_Tempo(type, inFile));
						break;
					case 0x54:
						m_events[position].push_back(new MetaEvent_SMPTE(type, inFile));
						break;
					case 0x58:
						m_events[position].push_back(new MetaEvent_TimeSignature(type, inFile));
						break;
					case 0x59:
						m_events[position].push_back(new MetaEvent_KeySignature(type, inFile));
						break;
					case 0x7F:
						m_events[position].push_back(new MetaEvent_Data(type, inFile));
						break;
					default:
						m_events[position].push_back(new MetaEvent(type, inFile));
					}
				}
				break;
			}
			case 0xF0:
				m_events[position].push_back(new SysexEvent(syntax, inFile));
				break;
			case 0x80:
			case 0x90:
				m_events[position].push_back(new MidiEvent_Note(syntax, !(tmpSyntax & 0b10000000)));
			}
			m_events[position].back()->fillFromFile(inFile);
			index += m_events[position].back()->getSize() + delta.getSize();
		}
	}

	MidiChunk_Track::~MidiChunk_Track()
	{
		for (auto& vec : m_events)
			for (auto ev : vec.second)
				delete ev;
	}

	MidiChunk_Track::SysexEvent::SysexEvent(char syntax, std::fstream& inFile)
		: MidiEvent(syntax)
		, m_length(inFile)
		, m_data(nullptr) {}

	void MidiChunk_Track::SysexEvent::fillFromFile(std::fstream& inFile)
	{
		m_data = new char[m_length];
		inFile.read(m_data, m_length);
	}

	MidiChunk_Track::SysexEvent::~SysexEvent()
	{
		if (m_data)
			delete[m_length] m_data;
	}

	uint32_t MidiChunk_Track::SysexEvent::getSize() const
	{
		return m_length.getValue() + m_length.getSize() + 1;
	}
}