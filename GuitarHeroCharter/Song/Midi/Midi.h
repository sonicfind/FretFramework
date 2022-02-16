#pragma once
#include "..\Song.h"
class Midi :
    public Song
{

public:
	// Inherited via Song
	void save() const override;

private:
	void fillFromFile(std::fstream& inFile) override;
	void readMetadata(std::fstream& inFile) override;
	void writeMetadata(std::fstream& outFile) const override;
	void readSync(std::fstream& inFile) override;
	void writeSync(std::fstream& outFile) const override;
	void readEvents(std::fstream& inFile) override;
	void writeEvents(std::fstream& outFile) const override;
	void readNoteTrack(std::fstream& inFile, const std::string& func) override;
	void writeNoteTracks(std::fstream& outFile) const override;
};
