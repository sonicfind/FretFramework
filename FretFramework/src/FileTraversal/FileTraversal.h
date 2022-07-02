#pragma once
#include <filesystem>
#include <mutex>

struct FilePointers
{
	std::filesystem::path m_path;
	unsigned char* m_file;
	const unsigned char* m_end;

	FilePointers(const std::filesystem::path& path);
	~FilePointers();
};

class Traversal
{
	friend class FileHasher;
	std::shared_ptr<FilePointers> m_filePointers;

public:
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

	operator bool() const { return m_current && m_current < m_end; }
};


