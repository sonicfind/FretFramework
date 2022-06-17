#pragma once
#include <filesystem>
#include <string_view>
#include "MD5/MD5.h"
#include <queue>
#include <thread>
#include <condition_variable>

class Traversal
{
protected:
	struct FilePointers
	{
		unsigned char* file;
		const unsigned char* end;
		~FilePointers();
	};

	struct HashNode
	{
		std::shared_ptr<MD5> hash;
		std::shared_ptr<FilePointers> file;
	};

private:
	static std::thread s_hashThread;
	static enum HashStatus
	{
		WAITING,
		USING_QUEUE,
		EXIT
	} s_hashStatus;
	static std::mutex s_mutex;
	static std::condition_variable s_condition;

	static std::queue<HashNode> s_hashes;

	static void hashThread();

	std::shared_ptr<FilePointers> m_filePointers;

public:
	static void waitForHashThread();
	static void endHashThread();

	class NoParseException : public std::runtime_error
	{
	public:
		NoParseException() : std::runtime_error("can not parse this data") {}
	};

protected:
	unsigned char*& m_file;
	const unsigned char*& m_end;
	const unsigned char* m_current;
	const unsigned char* m_next;

	Traversal(const std::filesystem::path& path);

public:
	virtual bool next() = 0;
	virtual void move(size_t count) = 0;
	virtual void skipTrack() = 0;
	virtual unsigned char extractChar() = 0;
	virtual bool extract(unsigned char& value) = 0;
	virtual ~Traversal() {}
	void generateHash(std::shared_ptr<MD5>& hash);

	operator bool() const { return m_current && m_current < m_end; }
};
