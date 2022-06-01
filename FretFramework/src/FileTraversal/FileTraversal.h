#pragma once
#include <filesystem>
#include <string_view>

class Traversal
{
protected:
	unsigned char* m_file;
	const unsigned char* m_end;
	const unsigned char* m_current;
	const unsigned char* m_next;

	Traversal(const std::filesystem::path& path);

public:
	virtual bool next() = 0;
	virtual void move(size_t count) = 0;
	virtual void skipTrack() = 0;
	virtual unsigned char extract() = 0;
	virtual ~Traversal();
	operator bool() const { return m_current && m_current < m_end; }
};
