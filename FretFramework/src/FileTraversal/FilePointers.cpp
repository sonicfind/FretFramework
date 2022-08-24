#include "FilePointers.h"
#include "FileChecks/FilestreamCheck.h"

FilePointers::FilePointers(const std::filesystem::path& path)
{
	FILE* inFile = FilestreamCheck::getFile(path, L"rb");
	_fseek_nolock(inFile, 0, SEEK_END);
	m_fileSize = _ftell_nolock(inFile);
	_fseek_nolock(inFile, 0, SEEK_SET);
	
	m_fileData = std::make_unique<unsigned char[]>(m_fileSize + 1);
	_fread_nolock(m_fileData.get(), m_fileSize, 1, inFile);
	_fclose_nolock(inFile);

	m_fileData[m_fileSize] = 0;
}
