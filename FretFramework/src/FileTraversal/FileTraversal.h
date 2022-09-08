#pragma once
#include "FilePointers.h"

class Traversal
{
public:
	class NoParseException : public std::runtime_error
	{
	public:
		NoParseException() : std::runtime_error("can not parse this data") {}
	};

protected:
	const unsigned char* m_current;
	const unsigned char* const m_end;

	Traversal(const FilePointers& file);

public:
	virtual void skipTrack() = 0;
	virtual ~Traversal() {}

	operator bool() const { return m_current && m_current < m_end; }
};


