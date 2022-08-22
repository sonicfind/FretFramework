#pragma once
#include <queue>
#include <mutex>

class ThreadedQueue
{
public:
	class Node
	{
	public:
		virtual void process() const noexcept = 0;
		virtual ~Node() = default;
	};

public:
	const unsigned int m_threadCount;
	std::vector<std::jthread> m_threads;
	int m_numActiveThreads;

	std::queue<std::unique_ptr<Node>> m_queue;
	std::mutex m_mutex;
	
	std::condition_variable m_mainCondition;
	std::condition_variable m_threadCondition;
	bool m_quit = false;

public:
	ThreadedQueue();
	~ThreadedQueue();
	void add(std::unique_ptr<Node>&& obj);
	void waitUntilFinished();

private:
	void threadFunc();
};

extern ThreadedQueue g_threadedQueuePool;
