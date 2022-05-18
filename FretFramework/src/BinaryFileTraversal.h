#pragma once
#include "FileTraversal.h"
#include "WebType.h"

class BinaryTraversal : public Traversal
{
	class InvalidChunkTagException : public std::runtime_error
	{
	public:
		InvalidChunkTagException(const char(&tag)[5]) : std::runtime_error(" invalid chunk tag") {}
	};

	class NoParseException : public std::runtime_error
	{
	public:
		NoParseException() : std::runtime_error("can not parse this data") {}
	};
	const unsigned char* m_nextTrack;
	size_t m_eventCount = 0;
	uint32_t m_tickPosition = 0;
	char m_eventType = 0;

public:
	BinaryTraversal(const std::filesystem::path& path);
	bool validateChunk(const char(&str)[5]);

	bool next() override;
	void move(size_t count) override;
	void skipTrack() override;
	bool extract(uint32_t& value) override;
	bool extract(uint16_t& value) override;
	bool extract(unsigned char& value);
	bool extractVarType(uint32_t& value);

	std::string extractText();
	std::string extractText(uint32_t length);
	uint32_t extractVarType();
	unsigned char extract() override;

	uint32_t getPosition() const { return m_tickPosition; }
	char getEventType() const { return m_eventType; }
	size_t getEventNumber() const { return m_eventCount; }

	bool operator==(char c) const { return *m_current == c; }
	bool operator!=(char c) const { return *m_current != c; }
	operator bool() const { return m_current && m_current < m_end; }
	operator uint32_t();
	operator uint16_t();
};
