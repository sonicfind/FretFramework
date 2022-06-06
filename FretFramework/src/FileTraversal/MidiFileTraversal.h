#pragma once
#include "FileTraversal.h"
#include <stdexcept>

class MidiTraversal : public Traversal
{
public:
	class InvalidChunkTagException : public std::runtime_error
	{
	public:
		InvalidChunkTagException(const char(&tag)[5]) : std::runtime_error("tag " + std::string(tag) + " was not parsed") {}
	};

private:
	class NoParseException : public std::runtime_error
	{
	public:
		NoParseException() : std::runtime_error("can not parse this data") {}
	};

	uint16_t m_format;
	uint16_t m_numTracks;
	uint16_t m_tickRate;

	uint16_t m_trackCount = 0;

	const unsigned char* m_nextTrack;
	size_t m_eventCount = 0;
	uint32_t m_tickPosition = 0;

	unsigned char m_midiEvent = 0;
	unsigned char m_eventType = 0;

public:
	MidiTraversal(const std::filesystem::path& path);
	bool validateChunk();
	bool checkNextChunk() const;
	const unsigned char* findNextChunk() const;
	bool doesNextTrackExist();
	void setNextTrack(const unsigned char* location);
	
	bool next() override;
	void move(size_t count) override;
	void skipTrack() override;
	unsigned char extractChar() override;
	bool extract(unsigned char& value) override;

	std::string extractText();
	

	uint16_t getTickRate() const { return m_tickRate; }
	uint16_t getTrackNumber() const { return m_trackCount; }
	uint32_t getPosition() const { return m_tickPosition; }
	unsigned char getEventType() const { return m_eventType; }
	size_t getEventNumber() const { return m_eventCount; }

	unsigned const char* getCurrent() { return m_current; }

	const unsigned char& operator[](int i) const
	{ 
		if (m_current + i >= m_next)
			throw "Nah boi";

		return m_current[i];
	}
	operator bool() const { return m_current && m_current < m_end && m_trackCount < m_numTracks; }
};
