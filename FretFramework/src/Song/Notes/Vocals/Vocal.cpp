#include "Vocal.h"

void Vocal::init(char pitch, uint32_t duration)
{
	m_pitch = pitch;
	m_duration = duration;
}

void Vocal::init(TextTraversal& traversal)
{
	try
	{
		setLyric(traversal.extractLyric());
		if (uint32_t pitch; traversal.extract(pitch))
			init(pitch, traversal.extractU32());
	}
	catch (Traversal::NoParseException)
	{
		throw EndofLineException();
	}
}

void Vocal::init(BCHTraversal& traversal)
{
	try
	{
		unsigned char length = traversal.extractChar();
		setLyric(traversal.extractLyric(length));

		// Read pitch
		if (unsigned char pitch; traversal.extract(pitch))
			init(pitch, traversal.extractVarType());
	}
	catch (Traversal::NoParseException)
	{
		throw EndofEventException();
	}
}

void Vocal::setLyric(const UnicodeString& lyric)
{
	m_lyric = lyric;
	if (m_lyric->length() > 255)
		m_lyric->resize(255);
}

void Vocal::setLyric(UnicodeString&& lyric)
{
	m_lyric = std::move(lyric);
	if (m_lyric->length() > 255)
		m_lyric->resize(255);
}

void Vocal::save_cht(std::fstream& outFile) const
{
	outFile << " \"" << m_lyric << '\"';
	if (m_pitch != 0)
		outFile << ' ' << (int)m_pitch << ' ' << m_duration;
	outFile << '\n';
}

void Vocal::save_bch(int lane, char*& outPtr) const
{
	*outPtr++ = (char)lane;
	const std::string str = m_lyric.toString();
	WebType((uint32_t)str.length()).copyToBuffer(outPtr);
	memcpy(outPtr, str.data(), str.length());
	outPtr += str.length();

	if (m_pitch != 0)
	{
		*outPtr++ = m_pitch;
		m_duration.copyToBuffer(outPtr);
	}
}
