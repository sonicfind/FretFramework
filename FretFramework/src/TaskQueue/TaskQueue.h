#pragma once
#include <queue>
#include <mutex>

class TaskQueue
{
public:
	class Task
	{
	public:
		virtual void process() const noexcept = 0;
		virtual ~Task() = default;
	};

private:
	static std::jthread s_threads[64];
	static size_t s_threadCount;
	static size_t s_numActiveThreads;

	static std::queue<std::unique_ptr<Task>> s_queue;
	static std::mutex s_mutex;
	
	static std::condition_variable s_mainCondition;
	static std::condition_variable s_threadCondition;
	static bool s_stopFlag;

public:
	static void startThreads(size_t threadCount);
	static void stopThreads();
	static void addTask(std::unique_ptr<Task>&& obj);
	static void waitForCompletedTasks();

private:
	TaskQueue();
	~TaskQueue();

	const static TaskQueue s_CONTROLLER;
};
