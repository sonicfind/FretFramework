#include "MidiFile.h"
#include "../../BinaryFileCheck.h"

namespace MidiFile
{
	MidiChunk::MidiChunk(const char(&type)[5], uint32_t length)
	{
		memcpy(m_header.type, type, 4);
		m_header.length = length;
	}

	MidiChunk::MidiChunk(std::fstream& inFile)
	{
		static const char* expected[2] = {"MThd", "MTrk"};
		if (!BinaryFile::chunkValidation(m_header.type, expected, 2, inFile))
			throw BinaryFile::IncorrectChunkMarkException((uint32_t)inFile.tellg(), "MThd or MTrk", m_header.type);
		byteSwap_read(inFile, m_header.length);
	}

	void MidiChunk::writeToFile(std::fstream& outFile) const
	{
		outFile.write(m_header.type, 4);
		byteSwap_write(outFile, m_header.length);
	}
}
