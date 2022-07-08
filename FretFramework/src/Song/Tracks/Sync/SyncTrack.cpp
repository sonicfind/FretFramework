#include "SyncTrack.h"
#include "../Midi/MidiFile.h"
#include <iostream>

void SyncTrack::clear()
{
	m_values = { {0, SyncValues(true, true)} };
}

void SyncTrack::load(TextTraversal& traversal)
{
	clear();
	while (traversal && traversal != '}' && traversal != '[')
	{
		uint32_t position = UINT32_MAX;
		try
		{
			position = traversal.extractU32();

			// Ensures ascending order
			if (m_values.back().first > position)
				throw "position out of order (previous:  " + std::to_string(m_values.back().first) + ')';

			// Starts the values at the current location with the previous set of values
			if (m_values.back().first < position)
			{
				static SyncValues prev;
				prev = m_values.back().second;
				m_values.push_back({ position, prev });
			}

			traversal.skipEqualsSign();

			if (strncmp(traversal.getCurrent(), "TS", 2) == 0)
			{
				traversal.move(2);
				uint32_t numerator = traversal.extractU32(), denom = 2;

				// Denom is optional, so use the no throw version
				traversal.extract(denom);
				m_values.back().second.setTimeSig(numerator, denom);
			}
			else
			{
				switch (traversal.extractChar())
				{
				case 'b':
				case 'B':
					m_values.back().second.setBPM(traversal.extractU32() * .001f);
					break;
				case 'a':
				case 'A':
					m_values.back().second.setAnchor(traversal.extractU32());
				}
			}
		}
		catch (std::runtime_error err)
		{
			if (position != UINT32_MAX)
				std::cout << "Line " << traversal.getLineNumber() << " - Position: " << position << err.what() << std::endl;
			else
				std::cout << "Line " << traversal.getLineNumber() << ": position could not be parsed" << std::endl;
		}
		catch (const std::string& str)
		{
			std::cout << "Line " << traversal.getLineNumber() << " - Position: " << position << str << std::endl;
		}

		traversal.next();
	}
}

void SyncTrack::load(BCHTraversal& traversal)
{
	clear();
	while (traversal.next())
	{
		try
		{
			// Starts the values at the current location with the previous set of values
			if (m_values.back().first < traversal.getPosition())
			{
				static SyncValues prev;
				prev = m_values.back().second;
				m_values.push_back({ traversal.getPosition(), prev });
			}

			if (traversal.getEventType() == 1)
			{
				uint32_t bpm = traversal.extractU32();
				m_values.back().second.setBPM(60000000.0f / bpm);
			}
			else if (traversal.getEventType() == 2)
				m_values.back().second.setTimeSig(traversal.extractChar(), traversal.extractChar());
		}
		catch (std::runtime_error err)
		{
			std::cout << "Event #" << traversal.getEventNumber() << " - Position " << traversal.getPosition() << ": " << err.what() << std::endl;
		}
	}
}

void SyncTrack::load(MidiTraversal& traversal)
{
	clear();
	do
	{
		if (traversal.getEventType() == 0x51 || traversal.getEventType() == 0x58)
		{
			// Starts the values at the current location with the previous set of values
			if (m_values.back().first < traversal.getPosition())
			{
				static SyncValues prev;
				prev = m_values.back().second;
				m_values.push_back({ traversal.getPosition(), prev });
			}

			if (traversal.getEventType() == 0x51)
			{
				uint32_t microsecondsPerQuarter = 0;
				memcpy((char*)&microsecondsPerQuarter + 1, traversal.getCurrent(), 3);
				m_values.back().second.setBPM(60000000.0f / _byteswap_ulong(microsecondsPerQuarter));
			}
			else
				m_values.back().second.setTimeSig(traversal[0], traversal[1]);
		}
	} while (traversal.next() && traversal.getEventType() != 0x2F);
}

void SyncTrack::save_cht(std::fstream& outFile)
{
	outFile << "[SyncTrack]\n{\n";
	for (const auto& sync : m_values)
		sync.second.writeSync_cht(sync.first, outFile);
	outFile << "}\n";
}

void SyncTrack::save_bch(std::fstream& outFile)
{
	outFile.write("SYNC", 4);

	uint32_t trackLength = 0;
	auto trackStart = outFile.tellp();
	outFile.write((char*)&trackLength, 4);

	// Header data
	uint32_t numEvents = 0;
	outFile.write((char*)&numEvents, 4);

	uint32_t prevPosition = 0;
	for (const auto& sync : m_values)
	{
		numEvents += sync.second.writeSync_bch(sync.first - prevPosition, outFile);
		prevPosition = sync.first;
	}

	auto trackEnd = outFile.tellp();
	trackLength = (uint32_t)(trackEnd - trackStart) - 4;

	outFile.seekp(trackStart);
	outFile.write((char*)&trackLength, 4);
	outFile.write((char*)&numEvents, 4);
	outFile.seekp(trackEnd);
}

using namespace MidiFile;
void SyncTrack::save_mid(std::fstream& outFile, const UnicodeString& sequenceName)
{
	MidiChunk_Track sync;
	if (!sequenceName.empty())
		sync.addEvent(0, new MidiChunk_Track::MetaEvent_Text(3, sequenceName));

	for (const auto& values : m_values)
	{
		auto timeSig = values.second.getTimeSig();
		if (timeSig.first)
			sync.addEvent(values.first, new MidiChunk_Track::MetaEvent_TimeSignature(timeSig.first, timeSig.second, 24));

		float bpm = values.second.getBPM();
		if (bpm > 0)
			sync.addEvent(values.first, new MidiChunk_Track::MetaEvent_Tempo((uint32_t)roundf(60000000.0f / bpm)));
	}
	sync.writeToFile(outFile);
}
