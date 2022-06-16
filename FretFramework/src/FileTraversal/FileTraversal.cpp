#include "FileTraversal.h"
#include "FileChecks/FilestreamCheck.h"

Traversal::Traversal(const std::filesystem::path& path)
	: m_next(nullptr)
	, m_filePointers(std::make_shared<FilePointers>())
	, m_file(m_filePointers->file)
	, m_end(m_filePointers->end)
{
	FILE* inFile = FilestreamCheck::getFile(path, L"rb");
	fseek(inFile, 0, SEEK_END);
	size_t length = ftell(inFile);
	fseek(inFile, 0, SEEK_SET);

	m_file = new unsigned char[length + 1]();
	m_end = m_file + length;
	fread(m_file, 1, length, inFile);
	fclose(inFile);

	m_current = m_file;
}

Traversal::FilePointers::~FilePointers()
{
	delete[end - file + 1] file;
}
