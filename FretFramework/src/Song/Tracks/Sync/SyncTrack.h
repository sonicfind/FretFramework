#pragma once
#include "SyncValues.h"
#include "FileTraversal/TextFileTraversal.h"
#include "FileTraversal/BCHFileTraversal.h"
#include "FileTraversal/MidiFileTraversal.h"

class SyncTrack
{
	std::vector<std::pair<uint32_t, SyncValues>> m_values;

public:
	void clear();
	void load(TextTraversal& traversal);
	void load(BCHTraversal& traversal);
	void load(MidiTraversal& traversal);
	void save_cht(std::fstream& outFile);
	void save_bch(std::fstream& outFile);
	void save_mid(std::fstream& outFile, const UnicodeString& sequenceName);

	auto begin() { return m_values.begin(); }
	auto end() { return m_values.end(); }
};

