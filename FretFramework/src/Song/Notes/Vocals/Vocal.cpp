#include "Vocal.h"

void Vocal::setLyric(const std::string& text)
{
	if (m_lyric.length() <= 255)
		m_lyric = text;
	else
		m_lyric = text.substr(0, 255);
}

void Vocal::save_cht(int lane, std::fstream& outFile) const
{
	outFile << ' ' << lane << " \"" << m_lyric << '\"';
}

void Vocal::save_bch(int lane, char*& outPtr) const
{
	*outPtr++ = (char)lane;
	*outPtr++ = (char)m_lyric.length();
	memcpy(outPtr, m_lyric.data(), m_lyric.length());
	outPtr += m_lyric.length();
}
