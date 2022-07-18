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

	std::optional<T> pop_front()
	{
		std::optional<T> result;
		std::scoped_lock lk(m_mutex);
		if (!m_queue.empty())
		{
			result = std::move(m_queue.front());
			m_queue.pop();
		}
		return result;
	}

	bool empty()
	{
		std::scoped_lock lk(m_mutex);
		return m_queue.empty();
	}
};
