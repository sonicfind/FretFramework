#include "Vocal.h"

void Vocal::init(TextTraversal& traversal)
{
	try
	{
		setLyric(traversal.extractLyric());
		if (uint32_t pitch; traversal.extract(pitch))
		{
			m_pitch = (char)pitch;
			init(traversal.extractU32());
		}
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
		{
			m_pitch = pitch;
			init(traversal.extractVarType());
		}
	}
	catch (Traversal::NoParseException)
	{
		throw EndofEventException();
	}
}

void Vocal::setLyric(const std::string& text)
{
	if (m_lyric.length() <= 255)
		m_lyric = text;
	else
		m_lyric = text.substr(0, 255);
}

void Vocal::save_cht(int lane, std::stringstream& buffer) const
{
	buffer << ' ' << lane << " \"" << m_lyric << '\"';
}

void Vocal::save_bch(int lane, char*& outPtr) const
{
	*outPtr++ = (char)lane;
	*outPtr++ = (char)m_lyric.length();
	memcpy(outPtr, m_lyric.data(), m_lyric.length());
	outPtr += m_lyric.length();
}
