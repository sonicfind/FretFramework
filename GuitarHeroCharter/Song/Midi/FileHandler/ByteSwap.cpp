#include "MidiFile.h"

void MidiFile::byteSwap_read(std::fstream& inFile, uint16_t& value)
{
	inFile.read((char*)&value, 2);
	value = _byteswap_ushort(value);
}

void MidiFile::byteSwap_write(std::fstream& outFile, uint16_t value)
{
	value = _byteswap_ushort(value);
	outFile.write((char*)&value, 2);
}

void MidiFile::byteSwap_read(std::fstream& inFile, uint32_t& value)
{
	inFile.read((char*)&value, 4);
	value = _byteswap_ulong(value);
}

void MidiFile::byteSwap_write(std::fstream& outFile, uint32_t value)
{
	value = _byteswap_ulong(value);
	outFile.write((char*)&value, 4);
}
