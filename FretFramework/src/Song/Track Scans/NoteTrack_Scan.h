#pragma once
#include "FileTraversal\TextFileTraversal.h"
#include "FileTraversal\BCHFileTraversal.h"
#include <fstream>

class NoteTrack_Scan
{
public:
	unsigned char m_scanValue;

	virtual void scan_cht(TextTraversal& traversal) = 0;
	virtual void scan_bch(BCHTraversal& traversal) = 0;
	virtual ~NoteTrack_Scan() {}

	virtual std::string toString() = 0;
};
