#pragma once
#include <queue>
#include <mutex>
#include <functional>

class TaskQueue
{
	static std::jthread s_threads[64];
	static size_t s_threadCount;
	static size_t s_numActiveThreads;

	static std::queue<std::function<void()>> s_queue;
	static std::mutex s_mutex;
	
	static std::condition_variable s_mainCondition;
	static std::condition_variable s_threadCondition;
	static bool s_stopFlag;

public:
	static void startThreads(size_t threadCount);
	static void stopThreads();
	static void addTask(const std::function<void()>& func);
	static void waitForCompletedTasks();

private:
	TaskQueue();
	~TaskQueue();

	const static TaskQueue s_CONTROLLER;
};
