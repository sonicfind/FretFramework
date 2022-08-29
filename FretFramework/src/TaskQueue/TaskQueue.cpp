#include "TaskQueue.h"

const TaskQueue TaskQueue::s_CONTROLLER;

std::jthread TaskQueue::s_threads[64];
size_t TaskQueue::s_threadCount = 0;
size_t TaskQueue::s_numActiveThreads;

std::queue<std::unique_ptr<TaskQueue::Task>> TaskQueue::s_queue;
std::mutex TaskQueue::s_mutex;

std::condition_variable TaskQueue::s_mainCondition;
std::condition_variable TaskQueue::s_threadCondition;
bool TaskQueue::s_stopFlag = false;

void TaskQueue::startThreads(size_t threadCount)
{
	if (s_threadCount != 0)
	{
		stopThreads();
		for (size_t i = 0; i < s_threadCount; ++i)
			s_threads[i].join();
		s_stopFlag = false;
	}

	if (threadCount >= 64)
		threadCount = 64;
	else if (threadCount < 4)
		threadCount = 4;

	s_numActiveThreads = s_threadCount = threadCount;
	for (size_t i = 0; i < s_threadCount; ++i)
		s_threads[i] = std::jthread(
			[&] ()
			{
				auto pop = [&]() -> std::unique_ptr<Task>
				{
					std::unique_ptr<Task> obj;
					std::unique_lock lock(s_mutex);
					--s_numActiveThreads;
					if (s_numActiveThreads == 0)
						s_mainCondition.notify_one();

					s_threadCondition.wait(lock, [&] { return !s_queue.empty() || s_stopFlag; });

					if (!s_stopFlag)
					{
						++s_numActiveThreads;
						obj = std::move(s_queue.front());
						s_queue.pop();
					}
					return obj;
				};

				while (std::unique_ptr<Task> obj = pop())
					obj->process();
			});
}

void TaskQueue::stopThreads()
{
	std::scoped_lock lock(s_mutex);
	s_stopFlag = true;
	s_threadCondition.notify_all();
}

void TaskQueue::waitForCompletedTasks()
{
	std::unique_lock lock(s_mutex);
	s_mainCondition.wait(lock, [&] { return s_queue.empty() && s_numActiveThreads == 0; });
}

void TaskQueue::addTask(std::unique_ptr<Task>&& obj)
{
	std::scoped_lock lock(s_mutex);
	s_queue.push(std::move(obj));
	s_threadCondition.notify_one();
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