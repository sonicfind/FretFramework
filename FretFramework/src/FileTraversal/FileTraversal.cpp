#include "FileTraversal.h"
#include "FileChecks/FilestreamCheck.h"

Traversal::Traversal(const FilePointers& file)
	: m_current(file.begin())
	, m_end(file.end()) {}
