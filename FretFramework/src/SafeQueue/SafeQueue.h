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
		std::scoped_lock lk(m_mutex);
		m_queue.push(obj);
	}

	void push(T&& obj)
	{
		std::scoped_lock lk(m_mutex);
		m_queue.push(std::move(obj));
	}

	T& front()
	{
		std::scoped_lock lk(m_mutex);
		return m_queue.front();
	}

	void pop()
	{
		std::scoped_lock lk(m_mutex);
		m_queue.pop();
	}

	bool empty()
	{
		std::scoped_lock lk(m_mutex);
		return m_queue.empty();
	}
};
