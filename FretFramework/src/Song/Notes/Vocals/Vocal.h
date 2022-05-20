#pragma once
#include "Base Nodes/Pitched.h"

// m_isActive will be used to determine whether the note is "singable"
class Vocal : public Pitched
{
	std::string m_lyric;
public:
	void setLyric(const std::string& text);
	std::string getLyric() const { return m_lyric; }
	void save_cht(int lane, std::stringstream& buffer) const;
	void save_bch(int lane, char*& outPtr) const;
};
