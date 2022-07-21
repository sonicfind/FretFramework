#include "FileHasher.h"

FileHasher::FileHasher()
	: m_threadCount(std::thread::hardware_concurrency() >= 4 ? std::thread::hardware_concurrency() / 2 : 1)
{
	m_threadSets = new ThreadSet[m_threadCount];
	m_threads.reserve(m_threadCount);
	for (unsigned int i = 0; i < m_threadCount; ++i)
		m_threads.emplace_back(&FileHasher::hashThread, this, std::ref(m_threadSets[i]));
}

FileHasher::~FileHasher()
{
	stopThreads();
	for (unsigned int i = 0; i < m_threadCount; ++i)
	{
		m_threadSets[i].status = ThreadSet::QUIT;
		m_threadSets[i].idleCondition.notify_one();
	}

	for (unsigned int i = 0; i < m_threadCount; ++i)
		m_threads[i].join();

	delete[m_threadCount] m_threadSets;
}

void FileHasher::addNode(std::shared_ptr<MD5>& hash, std::shared_ptr<FilePointers>& filePointers)
{
	m_queue.push({ hash, filePointers });
	m_runningCondition.notify_one();
}

void FileHasher::hashThread(ThreadSet& set)
{
	std::mutex mutex;
	std::unique_lock lk(mutex);
	while (set.status != ThreadSet::QUIT)
	{
		while (set.status == ThreadSet::ACTIVE)
		{
			if (m_queue.empty())
				m_runningCondition.wait(lk);

			while (auto opt = m_queue.pop_front())
			{
				HashNode& node = opt.value();
				node.hash->generate(node.file->m_file, node.file->m_end);
			}
		}

		if (set.status == ThreadSet::STOP)
		{
			set.status = ThreadSet::IDLE;
			set.status.notify_one();
		}

		set.idleCondition.wait(lk);
	}
}

void FileHasher::startThreads()
{
	for (unsigned int i = 0; i < m_threadCount; ++i)
	{
		m_threadSets[i].status = ThreadSet::ACTIVE;
		m_threadSets[i].idleCondition.notify_one();
	}
}

void FileHasher::stopThreads()
{
	for (unsigned int i = 0; i < m_threadCount; ++i)
		m_threadSets[i].status = ThreadSet::STOP;
	m_runningCondition.notify_all();

	for (unsigned int i = 0; i < m_threadCount; ++i)
		m_threadSets[i].status.wait(ThreadSet::STOP);
}
