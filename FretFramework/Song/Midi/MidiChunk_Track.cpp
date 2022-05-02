#include "MidiFile.h"

namespace MidiFile
{
	MidiChunk_Track::MidiChunk_Track(const std::string& name)
		: MidiChunk("MTrk")
	{
		m_events.push_back({ 0, {new MetaEvent_Text(3, name)} });
	}

	MidiChunk_Track::MidiChunk_Track()
		: MidiChunk("MTrk")
	{
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

		// Closes out the track with an End Event
		VariableLengthQuantity(0).writeToFile(outFile);
		MetaEvent_End().writeToFile(prevSyntax, outFile);

		std::streampos end = outFile.tellp();
		m_header.length = (uint32_t)(end - start) - 8;
		outFile.seekp(start);
		// Write the chunk's header data a second time but with the actual length of the chunk
		MidiChunk::writeToFile(outFile);
		outFile.seekp(end);
		outFile.flush();
	}

    void MidiChunk_Track::addEvent(uint32_t position, MidiEvent* ev)
    {
		VectorIteration::try_emplace(m_events, position).push_back(ev);
    }
}
