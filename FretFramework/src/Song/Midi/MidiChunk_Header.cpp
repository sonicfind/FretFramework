#include "MidiFile.h"

namespace MidiFile
{
	MidiChunk_Header::MidiChunk_Header(uint16_t tickRate)
		: MidiChunk("MThd", 6)
		, m_format(1)
		, m_numTracks(0)
		, m_tickRate(tickRate) {}

	void MidiChunk_Header::writeToFile(std::fstream& outFile) const
	{
		MidiChunk::writeToFile(outFile);
		byteSwap_write(outFile, m_format);
		byteSwap_write(outFile, m_numTracks);
		byteSwap_write(outFile, m_tickRate);
	}
}
