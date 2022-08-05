#pragma once
#include "SafeQueue/SafeQueue.h"
#include "MD5/MD5.h"
#include "FileTraversal/FilePointers.h"
class FileHasher
{
	struct HashNode
	{
		std::shared_ptr<MD5> hash;
		FilePointers file;
	};
	SafeQueue<HashNode> m_queue;

	enum ThreadStatus
	{
		IDLE,
		ACTIVE,
		QUIT
	};

	const unsigned int m_threadCount;
	std::unique_ptr<std::atomic<ThreadStatus>[]> m_statuses;
	std::vector<std::thread> m_threads;

	std::condition_variable m_condition;

public:
	FileHasher();
	~FileHasher();

	void startThreads();
	void stopThreads();
	void addNode(std::shared_ptr<MD5>& hash, const FilePointers& filePointers);

private:
	void hashThread(std::atomic<ThreadStatus>& status);
};

