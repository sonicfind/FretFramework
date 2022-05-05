#include "Vocal.h"

void Vocal::setLyric(const std::string& text)
{
	m_lyric = text;
}

void Vocal::save_cht(int lane, std::fstream& outFile) const
{
	outFile << ' ' << lane << " \"" << m_lyric << '\"';
}
