#include "Song.h"
#include "Tracks/BasicTracks/BasicTrack_bch.hpp"
#include "Tracks/VocalTracks/VocalTrack_bch.hpp"
#include "FileTraversal/BinaryFileTraversal.h"
#include "FileChecks/FilestreamCheck.h"
#include <iostream>

struct BCHHeader
{
	const char tag[4] = { 'B', 'C', 'H', 'F' };
	const uint32_t headerLength = 6;
	const uint32_t trackLength = 0;
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
	const uint16_t numInstruments = traversal;

	if (!traversal.validateChunk("SYNC"))
		throw BinaryTraversal::InvalidChunkTagException("SYNC");

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

	if (!traversal.validateChunk("EVTS"))
		throw BinaryTraversal::InvalidChunkTagException("EVTS");

	while (traversal.next())
	{
		if (traversal.getEventType() == 3)
		{
			if (m_globalEvents.empty() || m_globalEvents.back().first < traversal.getPosition())
			{
				static std::pair<uint32_t, std::vector<std::string>> pairNode;
				pairNode.first = traversal.getPosition();
				m_globalEvents.push_back(pairNode);
			}

			m_globalEvents.back().second.push_back(traversal.extractText());
		}
		else if (traversal.getEventType() == 4)
		{
			if (m_sectionMarkers.empty() || m_sectionMarkers.back().first < traversal.getPosition())
				m_sectionMarkers.push_back({ traversal.getPosition(), traversal.extractText() });
		}
	}

	for (int i = 0; i < numInstruments; ++i)
	{
		if (!traversal.validateChunk("INST"))
		{
			std::cout << "Could not parse Instrument track " << ++i << "\'s tag " << std::endl;
			if (!traversal.skipToNextChunk("INST"))
				break;
		}
		// Instrument ID
		switch (traversal.extract())
		{
		case 0:
			m_leadGuitar.load_bch(traversal);
			break;
		case 1:
			m_leadGuitar_6.load_bch(traversal);
			break;
		case 2:
			m_bassGuitar.load_bch(traversal);
			break;
		case 3:
			m_bassGuitar_6.load_bch(traversal);
			break;
		case 4:
			m_rhythmGuitar.load_bch(traversal);
			break;
		case 5:
			m_coopGuitar.load_bch(traversal);
			break;
		case 7:
			m_drums.load_bch(traversal);
			break;
		case 8:
			m_vocals.load_bch(traversal);
			break;
		case 9:
			m_harmonies.load_bch(traversal);
			break;
		default:
			traversal.skipTrack();
		}
	}
}

void Song::saveFile_Bch(const std::filesystem::path& filepath) const
{
	std::fstream outFile = FilestreamCheck::getFileStream(filepath, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
	BCHHeader header(m_version_bch, m_tickrate.m_value);
	outFile.write((char*)&header, 18);

	// Sync

	outFile.write("SYNC", 4);

	const uint32_t headerLength = 4;
	outFile.write((char*)&headerLength, 4);

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
	trackLength = (uint32_t)(trackEnd - trackStart) - (headerLength + 4);

	outFile.seekp(trackStart);
	outFile.write((char*)&trackLength, 4);
	outFile.write((char*)&numEvents, 4);
	outFile.seekp(trackEnd);

	// Events

	outFile.write("EVTS", 4);
	outFile.write((char*)&headerLength, 4);

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
	trackLength = (uint32_t)(trackEnd - trackStart) - (headerLength + 4);

	outFile.seekp(trackStart);
	outFile.write((char*)&trackLength, 4);
	outFile.write((char*)&numEvents, 4);
	outFile.seekp(trackEnd);
	outFile.flush();
	
	if (m_leadGuitar.save_bch(outFile))
		++header.numInstruments;
	if (m_leadGuitar_6.save_bch(outFile))
		++header.numInstruments;
	if (m_bassGuitar.save_bch(outFile))
		++header.numInstruments;
	if (m_bassGuitar_6.save_bch(outFile))
		++header.numInstruments;
	if (m_rhythmGuitar.save_bch(outFile))
		++header.numInstruments;
	if (m_coopGuitar.save_bch(outFile))
		++header.numInstruments;
	if (m_drums.save_bch(outFile))
		++header.numInstruments;
	if (m_vocals.save_bch(outFile))
		++header.numInstruments;
	if (m_harmonies.save_bch(outFile))
		++header.numInstruments;
	outFile.seekp(0);
	outFile.write((char*)&header, 18);
	outFile.close();
}
