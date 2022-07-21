#pragma once
#include "SafeQueue/SafeQueue.h"
#include "MD5/MD5.h"
#include "FileTraversal/FilePointers.h"
class FileHasher
{
	struct HashNode
	{
		std::shared_ptr<MD5> hash;
		std::shared_ptr<FilePointers> file;
	};
	SafeQueue<HashNode> m_queue;

	struct ThreadSet
	{
		enum ThreadStatus
		{
			ACTIVE,
			IDLE,
			STOP,
			QUIT
		};
		std::atomic<ThreadStatus> status = IDLE;

		std::condition_variable idleCondition;
	};
	const unsigned int m_threadCount;
	ThreadSet* m_threadSets;
	std::vector<std::thread> m_threads;

	std::condition_variable m_runningCondition;

public:
	FileHasher();
	~FileHasher();

	void startThreads();
	void stopThreads();
	void addNode(std::shared_ptr<MD5>& hash, std::shared_ptr<FilePointers>& filePointers);

private:
	void hashThread(ThreadSet& set);
};

