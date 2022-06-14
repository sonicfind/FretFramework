#pragma once
#include <filesystem>
#include <string_view>

class Traversal
{
public:
	class NoParseException : public std::runtime_error
	{
	public:
		NoParseException() : std::runtime_error("can not parse this data") {}
	};

protected:
	static unsigned char* s_file;
	static const unsigned char* s_end;
	const unsigned char* m_current;
	const unsigned char* m_next;

	Traversal(const std::filesystem::path& path);

public:
	virtual bool next() = 0;
	virtual void move(size_t count) = 0;
	virtual void skipTrack() = 0;
	virtual unsigned char extractChar() = 0;
	virtual bool extract(unsigned char& value) = 0;
	virtual ~Traversal();

	operator bool() const { return m_current && m_current < s_end; }
};
