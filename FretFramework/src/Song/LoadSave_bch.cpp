#include "Song.h"
#include "FileChecks/FilestreamCheck.h"
#include <iostream>

struct BCHHeader
{
	const char tag[4] = { 'B', 'C', 'H', 'F' };
	const uint32_t trackLength = 6;
	uint16_t version = 2;
	uint16_t tickRate = 480;
	uint16_t numInstruments = 0;
	BCHHeader(uint16_t ver, uint16_t rate) : version(ver), tickRate(rate) {}
};

void Song::loadFile(BCHTraversal&& traversal)
{
	m_version_bch = traversal.extractU16();
	m_tickrate = traversal.extractU16();

	Sustainable::setForceThreshold(m_tickrate / 3);
	Sustainable::setsustainThreshold(m_tickrate / 3);

	traversal.move(2);
	while (traversal.canParseNewChunk())
	{
		if (traversal.validateChunk("INST") || traversal.validateChunk("VOCL"))
		{
			const unsigned char ID = traversal.getTrackID();
			if (ID < 11)
				s_noteTracks[ID]->load_bch(traversal);
			else
				traversal.skipTrack();
		}
		else if (traversal.validateChunk("SYNC"))
		{
			while (traversal.next())
			{
				try
				{
					// Starts the values at the current location with the previous set of values
					if (m_sync.back().first < traversal.getPosition())
						m_sync.push_back({ traversal.getPosition(), m_sync.back().second.copy() });

					if (traversal.getEventType() == 1)
					{
						uint32_t bpm = traversal.extractU32();
						m_sync.back().second.setBPM(60000000.0f / bpm);
					}
					else if (traversal.getEventType() == 2)
						m_sync.back().second.setTimeSig(traversal.extractChar(), traversal.extractChar());
				}
				catch (std::runtime_error err)
				{
					std::cout << "Event #" << traversal.getEventNumber() << " - Position " << traversal.getPosition() << ": " << err.what() << std::endl;
				}
			}
		}
		else if (traversal.validateChunk("EVTS"))
		{
			while (traversal.next())
			{
				try
				{
					switch (traversal.getEventType())
					{
					case 3:
						if (m_globalEvents.empty() || m_globalEvents.back().first < traversal.getPosition())
						{
							static std::pair<uint32_t, std::vector<std::u32string>> pairNode;
							pairNode.first = traversal.getPosition();
							m_globalEvents.push_back(pairNode);
						}

						m_globalEvents.back().second.push_back(traversal.extractText());
						break;
					case 4:
						if (m_sectionMarkers.empty() || m_sectionMarkers.back().first < traversal.getPosition())
							m_sectionMarkers.push_back({ traversal.getPosition(), traversal.extractText() });
						break;
					}
				}
				catch (std::runtime_error err)
				{
					std::cout << "Event #" << traversal.getEventNumber() << " - Position " << traversal.getPosition() << ": " << err.what() << std::endl;
				}
			}
		}
		else
		{
			const unsigned char* const sync = traversal.findNextChunk("SYNC");
			const unsigned char* const evts = traversal.findNextChunk("EVTS");
			const unsigned char* const inst = traversal.findNextChunk("INST");
			const unsigned char* const vocl = traversal.findNextChunk("VOCL");
			if (sync && (!evts || sync < evts) && (!inst || sync < inst) && (!vocl || sync < vocl))
				traversal.setNextTrack(sync);
			else if (evts && (!inst || evts < inst) && (!vocl || evts < vocl))
				traversal.setNextTrack(evts);
			else if (inst && (!vocl || inst < vocl))
				traversal.setNextTrack(inst);
			else
				traversal.setNextTrack(vocl);
			traversal.skipTrack();
		}
	}
}

void Song::saveFile_Bch() const
{
	std::fstream outFile = FilestreamCheck::getFileStream(m_fullPath, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
	BCHHeader header(m_version_bch, m_tickrate);
	outFile.write((char*)&header, 14);

	// Sync

	outFile.write("SYNC", 4);

	uint32_t trackLength = 0;
	auto trackStart = outFile.tellp();
	outFile.write((char*)&trackLength, 4);

	// Header data
	uint32_t numEvents = 0;
	outFile.write((char*)&numEvents, 4);

	uint32_t prevPosition = 0;
	for (const auto& sync : m_sync)
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

	// Events

	outFile.write("EVTS", 4);

	trackLength = 0;
	trackStart = outFile.tellp();
	outFile.write((char*)&trackLength, 4);

	// Header data
	numEvents = 0;
	outFile.write((char*)&numEvents, 4);

	prevPosition = 0;
	auto sectIter = m_sectionMarkers.begin();
	for (auto eventIter = m_globalEvents.begin(); eventIter != m_globalEvents.end(); ++eventIter)
	{
		while (sectIter != m_sectionMarkers.end() && sectIter->first <= eventIter->first)
		{
			WebType::writeToFile(sectIter->first - prevPosition, outFile);
			outFile.put(4);
			sectIter->second.writeToBCH(outFile);
			prevPosition = sectIter->first;
			++sectIter;
			++numEvents;
		}

		uint32_t delta = eventIter->first - prevPosition;
		for (const auto& str : eventIter->second)
		{
			WebType::writeToFile(delta, outFile);
			outFile.put(3);
			UnicodeString::U32ToBCH(str, outFile);
			delta = 0;
			++numEvents;
		}
		prevPosition = eventIter->first;
	}

	while (sectIter != m_sectionMarkers.end())
	{
		WebType::writeToFile(sectIter->first - prevPosition, outFile);
		outFile.put(4);
		sectIter->second.writeToBCH(outFile);
		prevPosition = sectIter->first;
		++sectIter;
		++numEvents;
	}

	trackEnd = outFile.tellp();
	trackLength = (uint32_t)(trackEnd - trackStart) - 4;

	outFile.seekp(trackStart);
	outFile.write((char*)&trackLength, 4);
	outFile.write((char*)&numEvents, 4);
	outFile.seekp(trackEnd);
	outFile.flush();
	
	for (const auto& track : s_noteTracks)
		if (track->save_bch(outFile))
			++header.numInstruments;

	outFile.seekp(0);
	outFile.write((char*)&header, 14);
	outFile.close();
}
