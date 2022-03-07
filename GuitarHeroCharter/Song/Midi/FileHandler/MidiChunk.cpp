#include "MidiFile.h"

namespace MidiFile
{
	MidiChunk::MidiChunk(const char(&type)[5], uint32_t length)
	{
		memcpy(m_header.type, type, 4);
		m_header.length = length;
	}

	MidiChunk::MidiChunk(std::fstream& inFile)
	{
		inFile.read(m_header.type, 4);
		if (!strstr(m_header.type, "MThd") && !strstr(m_header.type, "MTrk"))
			throw ChunkTagNotFoundException();
		byteSwap_read(inFile, m_header.length);
	}

	void MidiChunk::writeToFile(std::fstream& outFile) const
	{
		outFile.write(m_header.type, 4);
		byteSwap_write(outFile, m_header.length);
	}

	char const* MidiChunk::ChunkTagNotFoundException::what() const
	{
		return "No MThd nor MTrk chunk tag found";
	}
}
