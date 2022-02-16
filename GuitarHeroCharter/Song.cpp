#include "Song.h"

Song::Song(const std::string& filename)
	: m_filename(filename) {}

void Song::fill(std::fstream& inFile)
{
	std::string line;
	while (std::getline(inFile, line))
	{
		// Ensures that we're entering a scope
		if (line.find('[') != std::string::npos)
		{
			// Skip '{' line
			inFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

			if (line.find("Song") != std::string::npos)
				readMetadata(inFile);
			else if (line.find("SyncTrack") != std::string::npos)
				readSync(inFile);
			else if (line.find("Events") != std::string::npos)
				readEvents(inFile);
			else
				readNoteTrack(inFile, line);
		}
	}
}

std::string Song::getFilename()
{
	return m_filename;
}

void Song::setFilename(const std::string& filename)
{
	m_filename = filename;
}

void Song::save(std::fstream& outFile) const
{
	writeMetadata(outFile);
	writeSync(outFile);
	writeEvents(outFile);
	writeNoteTracks(outFile);
}
