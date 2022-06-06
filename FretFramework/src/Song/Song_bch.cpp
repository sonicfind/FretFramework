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

void Song::loadFile_Bch()
{
	BCHTraversal traversal(m_filepath);
	if (!traversal.validateChunk("BCHF"))
		throw BCHTraversal::InvalidChunkTagException("BCHF");

	m_version_bch = traversal.extractU16();
	m_tickrate = traversal.extractU16();

	if (m_ini.m_eighthnote_hopo)
		m_ini.m_hopo_frequency.setDefault(m_tickrate / 2);
	else
		m_ini.m_hopo_frequency.setDefault(m_tickrate / 3);

	m_ini.m_sustain_cutoff_threshold.setDefault(m_tickrate / 3);
	Sustainable::setForceThreshold(m_ini.m_hopo_frequency);
	Sustainable::setsustainThreshold(m_ini.m_sustain_cutoff_threshold);

	const uint16_t noteTracksToParse = traversal.extractU16();
	uint16_t noteTrackCount = 0;
	while (traversal)
	{
		if (traversal.validateChunk("SYNC"))
		{
			while (traversal.next())
			{
				try
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
				catch (std::runtime_error err)
				{
					std::cout << "Event #" << traversal.getEventNumber() << " - Position " << traversal.getPosition() << ": " << err.what() << std::endl;
				}
			}
		}
		else if (traversal.validateChunk("INST") || traversal.validateChunk("VOCL"))
		{
			if (noteTrackCount < noteTracksToParse)
			{
				// Instrument ID
				const unsigned char ID = traversal.getTrackID();
				if (ID < 11)
				{
					try
					{
						s_noteTracks[ID]->load_bch(traversal);
					}
					catch (std::runtime_error err)
					{
						std::cout << "NoteTrack #" << noteTrackCount << ": ";
						if (ID < 9)
							std::cout << "could not parse number of difficulties";
						else
							std::cout << "could not parse \"isPlayable\" byte";
						traversal.skipTrack();
					}
				}
				else
					traversal.skipTrack();
				++noteTrackCount;
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
