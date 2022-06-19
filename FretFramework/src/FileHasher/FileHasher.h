#pragma once
#include "SafeQueue/SafeQueue.h"
#include "MD5/MD5.h"
#include "FileTraversal/FileTraversal.h"
class FileHasher
{
	struct HashNode
	{
		std::shared_ptr<MD5> hash;
		std::shared_ptr<FilePointers> file;
	};

	enum
	{
		WAITING_FOR_EXIT,
		EXIT
	} m_status;

	struct ThreadSet
	{
		SafeQueue<HashNode> queue;

		std::mutex mutex;
		std::condition_variable condition;
	};

	std::mutex m_sharedMutexes[2];
	std::condition_variable m_sharedCondition;

	typename std::list<ThreadSet>::iterator m_setIter;
	std::list<ThreadSet> m_sets;
	std::vector<std::thread> m_threads;

public:
	FileHasher();
	~FileHasher();

	void waitForQueues();
	void addNode(std::shared_ptr<MD5>& hash, Traversal& traversal);

private:
	void runHasher(ThreadSet& set);
};

