#include "FilePointers.h"
#include "FileChecks/FilestreamCheck.h"

FilePointers::FilePointers(const std::filesystem::path& path)
{
	FILE* inFile = FilestreamCheck::getFile(path, L"rb");
	fseek(inFile, 0, SEEK_END);
	m_fileSize = ftell(inFile);
	fseek(inFile, 0, SEEK_SET);
	
	m_fileData = std::make_shared<unsigned char[]>(m_fileSize + 1);
	fread(m_fileData.get(), m_fileSize, 1, inFile);
	fclose(inFile);

	m_fileData[m_fileSize] = 0;
}
