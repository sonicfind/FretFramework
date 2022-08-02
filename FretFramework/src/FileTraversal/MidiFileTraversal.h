#pragma once
#include "FileTraversal.h"
#include "Variable Types/UnicodeString.h"
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

	const unsigned char* m_nextTrack;

	uint16_t m_format;
	uint16_t m_numTracks;
	uint16_t m_tickRate;

	uint16_t m_trackCount = 0;
	std::string m_trackname;

	size_t m_eventCount = 0;
	uint32_t m_delta = 0;
	uint32_t m_tickPosition = 0;

	unsigned char m_midiEvent = 0;
	unsigned char m_eventType = 0;
	unsigned char m_channel = 0;

	struct
	{
		unsigned char midiValue = 0;
		unsigned char velocity = 0;
	} m_note;

	struct
	{
		unsigned char controller = 0;
		unsigned char value = 0;
	} m_controlChange;

	std::string m_sysex;
	std::u32string m_text;

	uint32_t m_microsecondsPerQuarter = 0;

	struct TimeSig
	{
		unsigned char numerator = 0;
		unsigned char denominator = 0;
		unsigned char metronome = 0;
		unsigned char num32nds = 0;
	} m_timeSig;

public:
	MidiTraversal(const std::filesystem::path& path);
	bool validateChunk();
	bool checkNextChunk() const;
	const unsigned char* findNextChunk() const;
	bool doesNextTrackExist();
	void setNextTrack(const unsigned char* location);
	
	bool next() override;
	void skipTrack() override;

	bool scanNext();

	std::string& getSysex() { return m_sysex; }
	std::u32string& getText() { return m_text; }

	uint16_t getTickRate() const { return m_tickRate; }
	uint16_t getTrackNumber() const { return m_trackCount; }
	const std::string& getTrackName() const { return m_trackname; }

	uint32_t getPosition() const { return m_tickPosition; }
	unsigned char getEventType() const { return m_eventType; }
	size_t getEventNumber() const { return m_eventCount; }

	unsigned char getMidiNote() const { return m_note.midiValue; }
	unsigned char getVelocity() const { return m_note.velocity; }
	uint32_t getMicrosecondsPerQuarter() const { return m_microsecondsPerQuarter; }
	TimeSig& getTimeSig() { return m_timeSig; }

	unsigned const char* getCurrent() { return m_current; }
	operator bool() const { return m_current && m_current < m_end&& m_trackCount < m_numTracks; }
};
