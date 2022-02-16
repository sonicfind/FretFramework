#include "Midi.h"
void Midi::save() const
{
	Song::save(std::ios_base::in | std::ios_base::trunc | std::ios_base::binary);
}

void Midi::fillFromFile(std::fstream& inFile)
{
}

void Midi::readMetadata(std::fstream& inFile)
{
}

void Midi::writeMetadata(std::fstream& outFile) const
{
}

void Midi::readSync(std::fstream& inFile)
{
}

void Midi::writeSync(std::fstream& outFile) const
{
}

void Midi::readEvents(std::fstream& inFile)
{
}

void Midi::writeEvents(std::fstream& outFile) const
{
}

void Midi::readNoteTrack(std::fstream& inFile, const std::string& func)
{
}

void Midi::writeNoteTracks(std::fstream& outFile) const
{
}
