#include "FileTraversal.h"
#include "FileChecks/FilestreamCheck.h"

FileHasher Traversal::s_fileHasher;

Traversal::Traversal(const std::filesystem::path& path)
	: m_next(nullptr)
	, m_filePointers(std::make_shared<FilePointers>(path))
	, m_file(m_filePointers->m_file)
	, m_end(m_filePointers->m_end)
	, m_current(m_file) {}

void Traversal::addMD5toThreadQueue(std::shared_ptr<MD5>& md5)
{
	s_fileHasher.addNode(md5, m_filePointers);
}
