#include "FilePointers.h"
#include "FileChecks/FilestreamCheck.h"

FilePointers::FilePointers(const std::filesystem::path& path)
{
	FILE* inFile = FilestreamCheck::getFile(path, L"rb");
	_fseek_nolock(inFile, 0, SEEK_END);
	m_fileSize = _ftell_nolock(inFile);
	_fseek_nolock(inFile, 0, SEEK_SET);
	
	m_fileData = new unsigned char[m_fileSize + 1];
	if (_fread_nolock(m_fileData, m_fileSize, 1, inFile) != 1)
		throw std::runtime_error("Uh, shoot");
	_fclose_nolock(inFile);

	m_fileData[m_fileSize] = 0;
}

FilePointers::~FilePointers()
{
	delete[m_fileSize + 1] m_fileData;
}
