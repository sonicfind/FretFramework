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
