#include "MidiFile.h"

namespace MidiFile
{
	MidiChunk::MidiChunk(const char(&type)[5], uint32_t length)
	{
		memcpy(m_header.type, type, 4);
		m_header.length = length;
	}

	void MidiChunk::writeToFile(std::fstream& outFile) const
	{
		outFile.write(m_header.type, 4);
		byteSwap_write(outFile, m_header.length);
	}
}
