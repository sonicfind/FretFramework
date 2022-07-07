#include "FileHasher.h"

FileHasher::FileHasher()
{
	unsigned int numThreads = std::thread::hardware_concurrency() >= 4 ? std::thread::hardware_concurrency() / 2 : 1;
	for (unsigned int i = 0; i < numThreads; ++i)
		m_threads.emplace_back(std::thread(&FileHasher::runHasher, this, std::ref(m_sets.emplace_back())));
	m_setIter = m_sets.begin();
}

FileHasher::~FileHasher()
{
	m_status = EXIT;
	for (auto& set : m_sets)
		set.condition.notify_one();

	for (auto& thr : m_threads)
		thr.join();
}

void FileHasher::runHasher(ThreadSet& set)
{
	std::unique_lock lk(set.mutex);
	do
	{
		while (!set.queue.empty())
		{
			HashNode node = set.queue.front();
			node.hash->generate(node.file->m_file, node.file->m_end);
			set.queue.pop();
		}

		m_sharedCondition.notify_one();
		set.condition.wait(lk);
	} while (m_status != EXIT);
}

void FileHasher::addNode(std::shared_ptr<MD5>& hash, Traversal& traversal)
{
	m_sharedMutex.lock();
	auto iter = m_setIter++;
	if (m_setIter == m_sets.end())
		m_setIter = m_sets.begin();
	m_sharedMutex.unlock();

	hash->prepareForHash();
	iter->queue.push({hash, traversal.m_filePointers});
	iter->condition.notify_one();
	
}
