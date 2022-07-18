#include "FileHasher.h"

FileHasher::FileHasher()
	: m_threadCount(std::thread::hardware_concurrency() >= 4 ? std::thread::hardware_concurrency() / 2 : 1)
{
	m_threads.reserve(m_threadCount);
}

FileHasher::~FileHasher()
{
	stopThreads();
}

void FileHasher::startThreads()
{
	m_status = ACTIVE;
	for (unsigned int i = 0; i < m_threadCount; ++i)
		m_threads.emplace_back(&FileHasher::hashThread, this);
}

void FileHasher::stopThreads()
{
	if (m_status == ACTIVE)
	{
		m_status = INACTIVE;
		m_condition.notify_all();

		for (unsigned int i = 0; i < m_threadCount; ++i)
			m_threads[i].join();

		m_threads.clear();
	}
}

void FileHasher::addNode(std::shared_ptr<MD5>& hash, std::shared_ptr<FilePointers>& filePointers)
{
	m_queue.push({ hash, filePointers });
	m_condition.notify_one();
}

void FileHasher::hashThread()
{
	std::mutex mutex;
	std::unique_lock lk(mutex);
	while (true)
	{
		if (m_queue.empty() && m_status == ACTIVE)
			m_condition.wait(lk);

		if (m_status == INACTIVE)
			return;

		while (auto opt = m_queue.pop_front())
		{
			HashNode& node = opt.value();
			node.hash->generate(node.file->m_file, node.file->m_end);
		}
	}
}
