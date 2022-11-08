#pragma once
#include "FileTraversal.h"
#include "Variable Types/UnicodeString.h"
#include "Variable Types/WebType.h"
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
	const unsigned char* m_next = nullptr;
	const unsigned char* m_nextTrack = nullptr;

	size_t m_eventCount = 0;
	uint32_t m_tickPosition = 0;
	unsigned char m_eventType = 0;
	unsigned char m_trackID = 0;

	static uint32_t ptrToUint32(const void* ptr)
	{
		return *reinterpret_cast<const uint32_t*>(ptr);
	}

public:
	BCHTraversal(const FilePointers& file);
	bool canParseNewChunk() { return m_current <= m_end - 8; }
	bool validateChunk(const char(&str)[5]);
	bool checkNextChunk(const char(&str)[5]) const;
	const unsigned char* findNextChunk(const char(&str)[5]) const;
	bool doesNextTrackExist();
	void setNextTrack(const unsigned char* location);
	
	bool next();
	void skipTrack() override;

	void move(size_t count);

	template <typename T>
	bool extract(T& value)
	{
		if (m_current + sizeof(T) <= m_next)
		{
			value = *reinterpret_cast<const T*>(m_current);
			m_current += sizeof(T);
			return true;
		}
		return false;
	}

	template <typename T>
	T extract()
	{
		T value;
		if (!extract(value))
			throw NoParseException();

		return value;
	}


	bool extractWebType(uint32_t& value);
	uint32_t extractWebType();

	std::u32string extractText();
	std::u32string extractLyric();

	template <typename T>
	bool testExtract() const noexcept
	{
		return m_current + sizeof(T) <= m_next;
	}

	bool testExtractWebType() const noexcept
	{
		return WebType::getEndPoint(m_current) <= m_next;
	}
	
	unsigned char getTrackID() const { return m_trackID; }
	uint32_t getPosition() const { return m_tickPosition; }
	unsigned char getEventType() const { return m_eventType; }
	size_t getEventNumber() const { return m_eventCount; }

	bool operator==(char c) const { return *m_current == c; }
	bool operator!=(char c) const { return *m_current != c; }
	operator bool() const { return m_current && m_current < m_end; }
};
