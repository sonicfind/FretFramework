#include "ThreadedQueue.h"

ThreadedQueue::ThreadedQueue()
	: m_threadCount(std::thread::hardware_concurrency())
	, m_numActiveThreads(m_threadCount)
{
	m_threads.reserve(m_threadCount);
	for (unsigned int i = 0; i < m_threadCount; ++i)
		m_threads.emplace_back(&ThreadedQueue::threadFunc, this);
}

void ThreadedQueue::waitUntilFinished()
{
	std::unique_lock lock(m_mutex);
	m_mainCondition.wait(lock, [&] { return m_queue.empty() && m_numActiveThreads == 0; });
}

ThreadedQueue::~ThreadedQueue()
{
	m_mutex.lock();
	while (!m_queue.empty())
		m_queue.pop();
	m_quit = true;
	m_threadCondition.notify_all();
	m_mutex.unlock();
}

void ThreadedQueue::add(std::unique_ptr<Node>&& obj)
{
	m_mutex.lock();
	m_queue.push(std::move(obj));
	m_threadCondition.notify_one();
	m_mutex.unlock();
}

void ThreadedQueue::threadFunc()
{
	auto pop = [&] () -> std::unique_ptr<Node>
	{
		std::unique_lock lock(m_mutex);
		--m_numActiveThreads;
		m_mainCondition.notify_one();
		m_threadCondition.wait(lock, [&] { return !m_queue.empty() || m_quit; });

		if (!m_quit)
		{
			++m_numActiveThreads;
			std::unique_ptr<Node> obj = std::move(m_queue.front());
			m_queue.pop();
			return obj;
		}
		return nullptr;
	};

	while (std::unique_ptr<Node> obj = pop())
		obj->process();
}

ThreadedQueue g_threadedQueuePool;
