#pragma once
#include "FileTraversal\TextFileTraversal.h"
#include "FileTraversal\BCHFileTraversal.h"
#include <fstream>

class NoteTrack_Scan
{
protected:
	int m_scanValue;

public:
	int getValue() const { return m_scanValue; }
	void addFromValue(const int value) { m_scanValue |= value; }
	virtual void scan_cht(TextTraversal& traversal) = 0;
	virtual void scan_bch(BCHTraversal& traversal) = 0;
	virtual ~NoteTrack_Scan() {}

	virtual std::string toString() = 0;
};
