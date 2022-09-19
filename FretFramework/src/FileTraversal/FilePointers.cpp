#include "FilePointers.h"
#include "FileChecks/FilestreamCheck.h"

void FilePointers::read(FILE* _inFile)
{
	m_fileData = new unsigned char[m_fileSize + 1];
	if (_fread_nolock(m_fileData, m_fileSize, 1, _inFile) != 1)
	{
		delete[m_fileSize + 1] m_fileData;
		throw std::runtime_error("Uh, shoot");
	}
	_fclose_nolock(_inFile);

	m_fileData[m_fileSize] = 0;
}

FilePointers::FilePointers(const std::filesystem::path& path)
{
	FILE* inFile = FilestreamCheck::getFile(path, L"rb");
	_fseek_nolock(inFile, 0, SEEK_END);
	m_fileSize = _ftell_nolock(inFile);
	_fseek_nolock(inFile, 0, SEEK_SET);
	read(inFile);
}

FilePointers::FilePointers(const std::filesystem::directory_entry& entry)
	: m_fileSize(entry.file_size())
{
	FILE* inFile = FilestreamCheck::getFile(entry.path(), L"rb");
	read(inFile);
}

FilePointers::~FilePointers()
{
	delete[m_fileSize + 1] m_fileData;
}
