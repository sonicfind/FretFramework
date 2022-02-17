#include "Song.h"
#include "..\FilestreamCheck.h"

Song::Song(const std::filesystem::path& filepath)
	: m_filepath(filepath) {}

void Song::fillFromFile()
{
	std::fstream inFile = FilestreamCheck::getFileStream(m_filepath, std::ios_base::in);
	fillFromFile(inFile);
	inFile.close();
}

std::filesystem::path Song::getFilepath()
{
	return m_filepath;
}

void Song::setFilepath(const std::filesystem::path& filename)
{
	m_filepath = filename;
}

void Song::save(std::ios_base::openmode mode) const
{
	try
	{
		std::filesystem::path outPath = m_filepath;
		if (outPath.extension() != ".test")
		{
			outPath += ".test";

			std::fstream outFile = FilestreamCheck::getFileStream(outPath, mode);
			writeMetadata(outFile);
			writeSync(outFile);
			writeEvents(outFile);
			writeNoteTracks(outFile);
			outFile.close();
		}
	}
	catch (FilestreamCheck::InvalidFileException e)
	{

	}
}
