#include "TaskQueue.h"

const TaskQueue TaskQueue::s_CONTROLLER;

std::jthread TaskQueue::s_threads[64];
size_t TaskQueue::s_threadCount = 0;
size_t TaskQueue::s_numActiveThreads;

std::queue<std::function<void()>> TaskQueue::s_queue;
std::mutex TaskQueue::s_mutex;

std::condition_variable TaskQueue::s_mainCondition;
std::condition_variable TaskQueue::s_threadCondition;
void(*TaskQueue::s_taskFunction)(const std::function<void()>&) = nullptr;

void TaskQueue::startThreads(size_t threadCount)
{
	if (s_threadCount != 0)
	{
		stopThreads();
		for (size_t i = 0; i < s_threadCount; ++i)
			s_threads[i].join();
	}

	s_threadCount = threadCount;

	if (s_threadCount == 0)
	{
		s_taskFunction = TaskQueue::run;
		return;
	}

	s_taskFunction = TaskQueue::add;
	if (s_threadCount >= 64)
		s_threadCount = 64;
	s_numActiveThreads = s_threadCount;

	for (size_t i = 0; i < s_threadCount; ++i)
		s_threads[i] = std::jthread(
			[&] (std::stop_token token) noexcept
			{
				auto pop = [&]() -> std::function<void()>
				{
					std::function<void()> func = nullptr;
					std::unique_lock lock(s_mutex);
					--s_numActiveThreads;
					if (s_numActiveThreads == 0)
						s_mainCondition.notify_one();

					s_threadCondition.wait(lock, [&] { return !s_queue.empty() || token.stop_requested(); });

					if (!token.stop_requested())
					{
						++s_numActiveThreads;
						func = s_queue.front();
						s_queue.pop();
					}
					return func;
				};

				while (std::function<void()> func = pop())
					func();
			});
}

void TaskQueue::stopThreads()
{
	std::scoped_lock lock(s_mutex);
	for (size_t i = 0; i < s_threadCount; ++i)
		s_threads[i].request_stop();
	s_threadCondition.notify_all();
}

void TaskQueue::waitForCompletedTasks()
{
	if (s_threadCount == 0)
		return;

	std::unique_lock lock(s_mutex);
	s_mainCondition.wait(lock, [&] { return s_queue.empty() && s_numActiveThreads == 0; });
}

void TaskQueue::addTask(const std::function<void()>& func)
{
	s_taskFunction(func);
}

TaskQueue::TaskQueue()
{
	size_t threadCount = std::thread::hardware_concurrency();
	TaskQueue::startThreads(threadCount);
}

TaskQueue::~TaskQueue()
{
	TaskQueue::stopThreads();
}

void TaskQueue::add(const std::function<void()>& func)
{
	std::scoped_lock lock(s_mutex);
	s_queue.push(func);
	s_threadCondition.notify_one();
}

void TaskQueue::run(const std::function<void()>& func)
{
	func();
}
