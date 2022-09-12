#pragma once
#include "NoteExceptions.h"
#include "FileTraversal/TextFileTraversal.h"
#include "FileTraversal/BCHFileTraversal.h"

// m_isActive will be used to determine whether the note is "singable"
class Vocal
{
	std::u32string m_lyric;
	unsigned char m_pitch = 0;
	uint32_t m_duration = 0;

public:
	constexpr explicit Vocal() {}

	void init(char pitch, uint32_t duration);
	void init(TextTraversal& traversal);
	void init(BCHTraversal& traversal);
	void setLyric(const std::u32string& lyric);
	void setLyric(std::u32string&& lyric);

	void save_cht(std::fstream& outFile) const;
	void save_bch(int lane, char*& outPtr) const;

	std::u32string getLyric() const { return m_lyric; }
	char getPitch() const { return m_pitch; }
	uint32_t getDuration() const { return m_duration; }

	bool isPitched() const { return m_pitch != 0; }
	void operator*=(float multiplier) { m_duration = uint32_t(m_duration * multiplier); }

	static bool isEventPlayable(TextTraversal& traversal);
	static bool isEventPlayable(BCHTraversal& traversal);
};
