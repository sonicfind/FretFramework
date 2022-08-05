#include "FileHasher.h"

FileHasher::FileHasher()
	: m_threadCount(std::thread::hardware_concurrency() >= 4 ? std::thread::hardware_concurrency() / 2 : 1)
	, m_statuses(std::make_unique<std::atomic<ThreadStatus>[]>(m_threadCount))
{
	m_threads.reserve(m_threadCount);
	for (unsigned int i = 0; i < m_threadCount; ++i)
		m_threads.emplace_back(&FileHasher::hashThread, this, std::ref(m_statuses[i]));
}

FileHasher::~FileHasher()
{
	stopThreads();

	for (unsigned int i = 0; i < m_threadCount; ++i)
		m_statuses[i] = QUIT;
	m_condition.notify_all();

	for (unsigned int i = 0; i < m_threadCount; ++i)
		m_threads[i].join();
}

void FileHasher::addNode(std::shared_ptr<MD5>& hash, const FilePointers& filePointers)
{
	m_queue.push({ hash, filePointers });
}

void FileHasher::hashThread(std::atomic<ThreadStatus>& status)
{
	std::mutex mutex;
	std::unique_lock lk(mutex);
	do 
	{
		status = ACTIVE;
		while (auto opt = m_queue.pop_front())
		{
			HashNode& node = opt.value();
			node.hash->generate(node.file.begin(), node.file.size());
		}

		status = IDLE;
		status.notify_one();

		m_condition.wait(lk);
	} while (status != QUIT);
}

void FileHasher::startThreads()
{
	m_queue.start();
	m_condition.notify_all();
}

void FileHasher::stopThreads()
{
	m_queue.stop();
	for (unsigned int i = 0; i < m_threadCount; ++i)
		m_statuses[i].wait(ACTIVE);
}
