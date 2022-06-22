#pragma once
#include "FileTraversal.h"
#include "Variable Types/UnicodeString.h"
#include <stdexcept>

class BCHTraversal : public Traversal
{
public:
	class InvalidChunkTagException : public std::runtime_error
	{
	public:
		InvalidChunkTagException(const char(&tag)[5]) : std::runtime_error("tag " + std::string(tag) + " was not parsed") {}
	};

private:
	const unsigned char* m_nextTrack;
	size_t m_eventCount = 0;
	uint32_t m_tickPosition = 0;
	unsigned char m_eventType = 0;
	unsigned char m_trackID = 0;

public:
	BCHTraversal(const std::filesystem::path& path);
	bool validateChunk(const char(&str)[5]);
	bool checkNextChunk(const char(&str)[5]) const;
	const unsigned char* findNextChunk(const char(&str)[5]) const;
	bool doesNextTrackExist();
	void setNextTrack(const unsigned char* location);
	
	bool next() override;
	void move(size_t count) override;
	void skipTrack() override;
	unsigned char extractChar() override;
	bool extract(unsigned char& value) override;

	uint32_t extractU32();
	bool extract(uint32_t& value);
	uint16_t extractU16();
	bool extract(uint16_t& value);

	UnicodeString extractText();
	UnicodeString extractLyric(uint32_t length);
	uint32_t extractVarType();
	bool extractVarType(uint32_t& value);
	
	unsigned char getTrackID() const { return m_trackID; }
	uint32_t getPosition() const { return m_tickPosition; }
	unsigned char getEventType() const { return m_eventType; }
	size_t getEventNumber() const { return m_eventCount; }

	bool operator==(char c) const { return *m_current == c; }
	bool operator!=(char c) const { return *m_current != c; }
	operator bool() const { return m_current && m_current < m_end; }
};
