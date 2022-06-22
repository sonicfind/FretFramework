#pragma once
#include "Base Nodes/Pitched.h"
#include "NoteExceptions.h"
#include "FileTraversal/TextFileTraversal.h"
#include "FileTraversal/BCHFileTraversal.h"

// m_isActive will be used to determine whether the note is "singable"
class Vocal : public Pitched
{
	UnicodeString m_lyric;
public:
	using Pitched::init;
	void init(TextTraversal& traversal);
	void init(BCHTraversal& traversal);
	void setLyric(const UnicodeString& text);
	UnicodeString getLyric() const { return m_lyric; }
	void save_cht(int lane, std::stringstream& buffer) const;
	void save_bch(int lane, char*& outPtr) const;
};
