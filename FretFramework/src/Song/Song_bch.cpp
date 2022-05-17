#include "Song.h"
#include "Tracks/BasicTracks/BasicTrack_bch.hpp"
#include "Tracks/VocalTracks/VocalTrack_bch.hpp"
#include "..\FilestreamCheck.h"
#include <iostream>

struct BCHChunk
{
	char tag[4] = { 'B', 'C', 'H', 'F' };
	uint32_t length = 0;
};

struct BCHHeader : public BCHChunk
{
	uint16_t version = 2;
	uint16_t tickRate = 480;
	uint16_t numInstruments = 0;
	BCHHeader() = default;
	BCHHeader(uint16_t rate)
		: tickRate(rate) {}
};

void Song::loadFile_Bch()
{
	FILE* inFile = FilestreamCheck::getFile(m_filepath, L"rb");
	BCHHeader header;
	if (fread(&header, 14, 1, inFile) != 1 ||
		strncmp(header.tag, "BCHF", 4) != 0)
		throw "You dun goofed";

	unsigned char* file = new unsigned char[header.length + 1];
	const unsigned char* const end = file + header.length;
	fread(file, 1, header.length, inFile);
	fclose(inFile);

	const unsigned char* current = file;
	const unsigned char* next = nullptr;

	BCHChunk chunk;

	if (current + 8 > end)
		throw "You dun goofed";
	
	memcpy(&chunk, current, sizeof(BCHChunk));
	if (strncmp(chunk.tag, "SYNC", 4) != 0)
		throw "You dun goofed";

	current += sizeof(BCHChunk);
	next = current + chunk.length;
	current += 4;

	if (next > end)
		throw "Bruh";

	uint32_t position = 0;
	while (current < next)
	{
		position += WebType(current);
		char type = *current++;
		WebType length(current);

		// Starts the values at the current location with the previous set of values
		if (m_sync.back().first < position)
		{
			static SyncValues prev;
			prev = m_sync.back().second;
			m_sync.push_back({ position, prev });
		}

		if (type == 1)
		{
			uint32_t bpm = 0;
			memcpy(&bpm, current, 4);
			m_sync.back().second.setBPM(60000000.0f / bpm);
		}
		else if (type == 2)
			m_sync.back().second.setTimeSig(current[0], current[1]);
		current += length;
	}
	current = next;

	if (current + 8 > end)
		throw "You dun goofed";

	memcpy(&chunk, current, sizeof(BCHChunk));
	if (strncmp(chunk.tag, "EVTS", 4) != 0)
		throw "You dun goofed";

	current += sizeof(BCHChunk);
	next = current + chunk.length;
	current += 4;

	if (next > end)
	{
		if (header.numInstruments)
			throw "Bruh";
		next = end;
	}

	{
		position = 0;
		while (current < next)
		{
			position += WebType(current);
			char type = *current++;
			WebType length(current);

			if (current + length > next)
				length = char(next - current);

			if (type == 3)
			{
				std::string_view ev((const char*)current, length);
				if (m_globalEvents.empty() || m_globalEvents.back().first < position)
				{
					static std::pair<uint32_t, std::vector<std::string>> pairNode;
					pairNode.first = position;
					m_globalEvents.push_back(pairNode);
				}

				m_globalEvents.back().second.push_back(std::string(ev));
			}
			else if (type == 4)
			{
				std::string_view ev((const char*)current, length);
				if (m_sectionMarkers.empty() || m_sectionMarkers.back().first < position)
					m_sectionMarkers.push_back({ position, std::string(ev) });
			}
			current += length;
		}
		current = next;
	}

	for (int i = 0; current < end && i < header.numInstruments; ++i)
	{
		memcpy(&chunk, current, sizeof(BCHChunk));
		if (strncmp(chunk.tag, "INST", 4) != 0)
			throw "You dun goofed";

		current += sizeof(BCHChunk);
		next = current + chunk.length;

		if (i + 1 < header.numInstruments)
			if (strncmp((const char*)next, "INST", 4) != 0)
				next = (const unsigned char*)strstr((const char*)current, "INST");

		if (next == nullptr || next > end)
			next = end;

		if (current < next)
		{
			// Instrument ID
			switch (*current++)
			{
			case 0:
				m_leadGuitar.load_bch(current, next);
				break;
			case 1:
				m_leadGuitar_6.load_bch(current, next);
				break;
			case 2:
				m_bassGuitar.load_bch(current, next);
				break;
			case 3:
				m_bassGuitar_6.load_bch(current, next);
				break;
			case 4:
				m_rhythmGuitar.load_bch(current, next);
				break;
			case 5:
				m_coopGuitar.load_bch(current, next);
				break;
			case 7:
				m_drums.load_bch(current, next);
				break;
			case 8:
				m_vocals.load_bch(current, next);
				break;
			case 9:
				m_harmonies.load_bch(current, next);
				break;
			}
		}
		current = next;
	}

	delete[header.length + 1] file;
}

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
