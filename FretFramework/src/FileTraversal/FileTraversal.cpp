#include "FileTraversal.h"
#include "FileChecks/FilestreamCheck.h"

std::thread Traversal::s_hashThread = std::thread(hashThread);
std::mutex Traversal::s_mutex;
std::condition_variable Traversal::s_condition;
Traversal::HashStatus Traversal::s_hashStatus = WAITING_FOR_EXIT;
std::queue<Traversal::HashNode> Traversal::s_hashes;

void Traversal::hashThread()
{
	std::unique_lock lk(s_mutex);
	while (true)
	{
		while (s_hashStatus != EXIT && s_hashes.empty())
			s_condition.wait(lk);

		if (s_hashStatus == EXIT)
			break;

		HashNode& node = s_hashes.front();
		node.hash->generate(node.file->file, node.file->end - node.file->file);
		s_hashes.pop();

		s_condition.notify_one();
	}
}

void Traversal::endHashThread()
{
	std::unique_lock lk(s_mutex);
	while (!s_hashes.empty())
		s_condition.wait(lk);

	s_hashStatus = EXIT;
	s_condition.notify_one();
}

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

void Traversal::generateHash(std::shared_ptr<MD5>& hash)
{
	s_hashes.emplace(hash, m_filePointers);
	s_condition.notify_one();
}

Traversal::FilePointers::~FilePointers()
{
	delete[end - file + 1] file;
}
