#include "Song.h"
#include "FileTraversal/BinaryFileTraversal.h"
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

void Song::loadFile_Bch()
{
	BinaryTraversal traversal(m_filepath);
	if (!traversal.validateChunk("BCHF"))
		throw BinaryTraversal::InvalidChunkTagException("BCHF");

	m_version_bch = traversal;
	m_tickrate = traversal;
	uint16_t instrumentsToParse = traversal;

	while (traversal.operator bool())
	{
		if (traversal.validateChunk("SYNC"))
		{
			traversal.move(4);
			while (traversal.next())
			{
				// Starts the values at the current location with the previous set of values
				if (m_sync.back().first < traversal.getPosition())
				{
					static SyncValues prev;
					prev = m_sync.back().second;
					m_sync.push_back({ traversal.getPosition(), prev });
				}

				if (traversal.getEventType() == 1)
				{
					uint32_t bpm = traversal;
					m_sync.back().second.setBPM(60000000.0f / bpm);
				}
				else if (traversal.getEventType() == 2)
					m_sync.back().second.setTimeSig(traversal.extract(), traversal.extract());
			}
		}
		else if (traversal.validateChunk("EVTS"))
		{
			traversal.move(4);
			while (traversal.next())
			{
				switch (traversal.getEventType())
				{
				case 3:
					if (m_globalEvents.empty() || m_globalEvents.back().first < traversal.getPosition())
					{
						static std::pair<uint32_t, std::vector<std::string>> pairNode;
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
		}
		else if (traversal.validateChunk("INST"))
		{
			if (instrumentsToParse > 0)
			{
				// Instrument ID
				const unsigned char ID = traversal.getTrackID();
				if (ID < 11)
					s_noteTracks[ID]->load_bch(traversal);
				else
					traversal.skipTrack();
				--instrumentsToParse;
			}
			else
				traversal.skipTrack();
		}
		else
		{
			const unsigned char* const sync = traversal.findNextChunk("SYNC");
			const unsigned char* const evts = traversal.findNextChunk("EVTS");
			const unsigned char* const inst = traversal.findNextChunk("INST");
			if (sync && (!evts || sync < evts) && (!inst || sync < inst))
				traversal.setNextTrack(sync);
			else if (evts && (!sync || evts < sync) && (!inst || evts < inst))
				traversal.setNextTrack(evts);
			else if (inst && (!sync || inst < sync) && (!evts || inst < evts))
				traversal.setNextTrack(inst);
			else
				traversal.setNextTrack(nullptr);
			traversal.skipTrack();
		}
	}
}

void Song::saveFile_Bch(const std::filesystem::path& filepath) const
{
	std::fstream outFile = FilestreamCheck::getFileStream(filepath, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
	BCHHeader header(m_version_bch, m_tickrate.m_value);
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
			WebType(sectIter->first - prevPosition).writeToFile(outFile);
			outFile.put(4);
			WebType((uint32_t)sectIter->second.size()).writeToFile(outFile);
			outFile.write(sectIter->second.c_str(), sectIter->second.size());
			prevPosition = sectIter->first;
			++sectIter;
			++numEvents;
		}

		WebType delta(eventIter->first - prevPosition);
		for (const auto& str : eventIter->second)
		{
			delta.writeToFile(outFile);
			outFile.put(3);
			WebType((uint32_t)str.size()).writeToFile(outFile);
			outFile.write(str.c_str(), str.size());
			delta = 0;
			++numEvents;
		}
		prevPosition = eventIter->first;
	}

	while (sectIter != m_sectionMarkers.end())
	{
		WebType(sectIter->first - prevPosition).writeToFile(outFile);
		outFile.put(4);
		WebType((uint32_t)sectIter->second.size()).writeToFile(outFile);
		outFile.write(sectIter->second.c_str(), sectIter->second.size());
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
	
	for (const NoteTrack* const track : s_noteTracks)
		if (track->save_bch(outFile))
			++header.numInstruments;

	outFile.seekp(0);
	outFile.write((char*)&header, 14);
	outFile.close();
}
