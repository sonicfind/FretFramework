#pragma once
#include "FileTraversal.h"
#include "Variable Types/UnicodeString.h"
#include "Variable Types/VariableLengthQuantity.h"
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
	MidiTraversal(const FilePointers& file);
	bool validateChunk();
	bool checkNextChunk() const;
	const unsigned char* findNextChunk() const;
	bool doesNextTrackExist();
	void setNextTrack(const unsigned char* location);
	
	void skipTrack() override;

	template <bool loadFullText = true>
	bool next()
	{
		if (m_current >= m_nextTrack)
			return false;

		m_tickPosition += VariableLengthQuantity(m_current);
		unsigned char tmp = *m_current++;
		if (tmp < 0xF0)
		{
			if (tmp >= 128)
			{
				m_channel = tmp & 15;
				m_midiEvent = tmp & 240;

				if (m_midiEvent == 0x80 || m_midiEvent == 0x90)
				{
					m_note.midiValue = *m_current++;
					m_note.velocity = *m_current++;
				}
				else if (m_midiEvent == 0xB0)
				{
					m_controlChange.controller = *m_current++;
					m_controlChange.value = *m_current++;
				}
				else if (m_midiEvent == 0xA0 || m_midiEvent == 0xE0)
					m_current += 2;
				else
					++m_current;
			}
			else if (m_midiEvent == 0x80 || m_midiEvent == 0x90)
			{
				m_note.midiValue = tmp;
				m_note.velocity = *m_current++;
			}
			else if (m_midiEvent == 0xB0)
			{
				m_controlChange.controller = tmp;
				m_controlChange.value = *m_current++;
			}
			else if (m_midiEvent == 0xA0 || m_midiEvent == 0xE0 || m_midiEvent == 0xF2)
				++m_current;
			else if (m_midiEvent != 0xC0 && m_midiEvent != 0xD0 && m_midiEvent != 0xF3)
				return false;

			m_eventType = m_midiEvent;
		}
		else if (tmp == 0xFF)
		{
			m_eventType = *m_current++;
			const uint32_t length = VariableLengthQuantity(m_current);
			if (m_eventType < 16)
			{
				if constexpr (loadFullText)
					m_text = UnicodeString::bufferToU32(m_current, length);
				else
					m_text = (char)(*m_current);
			}
			else if (m_eventType == 0x51)
			{
				m_microsecondsPerQuarter = 0;
				memcpy((char*)&m_microsecondsPerQuarter + 1, m_current, 3);
				m_microsecondsPerQuarter = _byteswap_ulong(m_microsecondsPerQuarter);
			}
			else if (m_eventType == 0x58)
				memcpy(&m_timeSig, m_current, 4);
			else if (m_eventType == 0x2F)
				return false;

			m_current += length;
		}
		else
		{
			m_eventType = tmp;
			if (tmp == 0xF7 || tmp == 0xF0)
			{
				uint32_t length = VariableLengthQuantity(m_current);
				m_sysex.assign((const char*)m_current, length);
				m_current += length;
			}
			else if (tmp == 0xF2)
			{
				m_midiEvent = m_eventType;
				m_current += 2;
			}
			else if (tmp == 0xF3)
			{
				m_midiEvent = m_eventType;
				++m_current;
			}
		}

		++m_eventCount;
		return true;
	}

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
