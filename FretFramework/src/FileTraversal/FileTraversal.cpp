#include "FileTraversal.h"
#include "FileChecks/FilestreamCheck.h"

unsigned char* Traversal::s_file = nullptr;
const unsigned char* Traversal::s_end = nullptr;
Traversal::Traversal(const std::filesystem::path& path)
	: m_next(nullptr)
{
	FILE* inFile = FilestreamCheck::getFile(path, L"rb");
	fseek(inFile, 0, SEEK_END);
	size_t length = ftell(inFile);
	fseek(inFile, 0, SEEK_SET);

	s_file = new unsigned char[length + 1]();
	s_end = s_file + length;
	fread(s_file, 1, length, inFile);
	fclose(inFile);

	m_current = s_file;
}

Traversal::~Traversal()
{
	delete[s_end - s_file + 1] s_file;
}
