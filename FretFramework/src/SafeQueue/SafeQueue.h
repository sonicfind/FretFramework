#pragma once
#include <queue>
#include <mutex>

template<class T>
class SafeQueue
{
	std::queue<T> m_queue;
	std::mutex m_mutex;
	std::condition_variable m_condition;
	bool active = false;

public:
	void start() { active = true; }
	void stop()
	{ 
		active = false; 
		m_condition.notify_all();
	}

	void push(T&& obj)
	{
		m_mutex.lock();
		m_queue.push(std::move(obj));
		m_mutex.unlock();
		m_condition.notify_one();
	}

	std::optional<T> pop_front()
	{
		std::mutex localMutex;
		std::unique_lock lk(localMutex);
		m_condition.wait(lk, [&] { return !m_queue.empty() || !active; });

		m_mutex.lock();
		if (!m_queue.empty())
		{
			std::optional<T> result(std::move(m_queue.front()));
			m_queue.pop();
			m_mutex.unlock();
			return result;
		}
		m_mutex.unlock();
		return {};
	}
};
