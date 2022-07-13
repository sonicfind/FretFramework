#pragma once
#include "NoteExceptions.h"
#include "Variable Types/WebType.h"
#include "FileTraversal/TextFileTraversal.h"
#include "FileTraversal/BCHFileTraversal.h"

// m_isActive will be used to determine whether the note is "singable"
class Vocal
{
	UnicodeString m_lyric;
	unsigned char m_pitch;
	WebType m_duration;

public:
	constexpr explicit Vocal() : m_pitch(0) {}

	void init(char pitch, uint32_t duration);
	void init(TextTraversal& traversal);
	void init(BCHTraversal& traversal);
	void setLyric(const UnicodeString& lyric);
	void setLyric(UnicodeString&& lyric);

	void save_cht(std::fstream& outFile) const;
	void save_bch(int lane, char*& outPtr) const;

	UnicodeString getLyric() const { return m_lyric; }
	char getPitch() const { return m_pitch; }
	uint32_t getDuration() const { return m_duration; }

	bool isPitched() const { return m_pitch != 0; }
	void operator*=(float multiplier) { m_duration = uint32_t(m_duration * multiplier); }
};
