#include "Song.h"
#include "Tracks/BasicTracks/BasicTrack_bch.hpp"
#include "Tracks/VocalTracks/VocalTrack_bch.hpp"
#include "..\FilestreamCheck.h"
#include <iostream>

void Song::saveFile_Bch(const std::filesystem::path& filepath) const
{
	std::fstream outFile = FilestreamCheck::getFileStream(filepath, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
	auto fileStart = outFile.tellp();
	BCHHeader header(m_tickrate.m_value);
	outFile.write((char*)&header, 14);

	outFile.write("SYNC", 4);
	auto trackStart = outFile.tellp();
	uint32_t trackLength = 0;
	uint32_t numEvents = 0;
	outFile.write((char*)&trackLength, 4);
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

	outFile.write("EVTS", 4);
	trackStart = outFile.tellp();
	numEvents = 0;
	outFile.write((char*)&trackLength, 4);
	outFile.write((char*)&numEvents, 4);

	prevPosition = 0;
	auto sectIter = m_sectionMarkers.begin();
	for (auto eventIter = m_globalEvents.begin(); eventIter != m_globalEvents.end(); ++eventIter)
	{
		while (sectIter != m_sectionMarkers.end() && sectIter->first <= eventIter->first)
		{
			VariableLengthQuantity(sectIter->first - prevPosition).writeToFile(outFile);
			outFile.put(4);
			VariableLengthQuantity((uint32_t)sectIter->second.size()).writeToFile(outFile);
			outFile.write(sectIter->second.c_str(), sectIter->second.size());
			prevPosition = sectIter->first;
			++sectIter;
			++numEvents;
		}

		VariableLengthQuantity delta(eventIter->first - prevPosition);
		for (const auto& str : eventIter->second)
		{
			delta.writeToFile(outFile);
			outFile.put(3);
			VariableLengthQuantity((uint32_t)str.size()).writeToFile(outFile);
			outFile.write(str.c_str(), str.size());
			delta = 0;
			++numEvents;
		}
		prevPosition = eventIter->first;
	}

	while (sectIter != m_sectionMarkers.end())
	{
		VariableLengthQuantity(sectIter->first - prevPosition).writeToFile(outFile);
		outFile.put(4);
		VariableLengthQuantity((uint32_t)sectIter->second.size()).writeToFile(outFile);
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
	header.length = (uint32_t)(outFile.tellp() - fileStart) - 14;
	outFile.seekp(fileStart);
	outFile.write((char*)&header, 14);
	outFile.close();
}
