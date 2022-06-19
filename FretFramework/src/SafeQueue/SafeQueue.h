#pragma once
#include <queue>
#include <mutex>

template<class T>
class SafeQueue
{
	std::queue<T> m_queue;
	std::mutex m_mutex;

public:
	void push(const T& obj)
	{
		m_mutex.lock();
		m_queue.push(obj);
		m_mutex.unlock();
	}

	T& front()
	{
		return m_queue.front();
	}

	void pop()
	{
		m_mutex.lock();
		m_queue.pop();
		m_mutex.unlock();
	}

	bool empty() { return m_queue.empty(); }
};
