#include "FileTraversal.h"
#include "FileChecks/FilestreamCheck.h"

std::mutex FilePointers::s_mutex;

FilePointers::FilePointers(const std::filesystem::path& path)
	: m_path(path)
	, m_file(nullptr)
	, m_end(nullptr)
{
	s_mutex.lock();

	FILE* inFile = FilestreamCheck::getFile(m_path, L"rb");
	fseek(inFile, 0, SEEK_END);
	size_t length = ftell(inFile);
	fseek(inFile, 0, SEEK_SET);

	m_file = new unsigned char[length + 1]();
	fread(m_file, 1, length, inFile);
	fclose(inFile);

	m_end = m_file + length;

	s_mutex.unlock();
}

FilePointers::~FilePointers()
{
	delete[m_end - m_file + 1] m_file;
}

Traversal::Traversal(const std::filesystem::path& path)
	: m_next(nullptr)
	, m_filePointers(std::make_shared<FilePointers>(path))
	, m_file(m_filePointers->m_file)
	, m_end(m_filePointers->m_end)
	, m_current(m_file) {}
