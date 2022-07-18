#include "FileTraversal.h"
#include "FileChecks/FilestreamCheck.h"


Traversal::Traversal(const std::filesystem::path& path)
	: m_next(nullptr)
	, m_filePointers(std::make_shared<FilePointers>(path))
	, m_file(m_filePointers->m_file)
	, m_end(m_filePointers->m_end)
	, m_current(m_file) {}
