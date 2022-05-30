#pragma once
#include "FileTraversal\TextFileTraversal.h"
#include "FileTraversal\BinaryFileTraversal.h"
#include <fstream>
class NoteTrack
{
protected:
	const char* const m_name;
	const char m_instrumentID;

public:
	static unsigned char s_starPowerReadNote;
	NoteTrack(const char* name, char instrumentID)
		: m_name(name)
		, m_instrumentID(instrumentID) {}

	virtual void load_cht(TextTraversal& traversal) = 0;
	virtual void save_cht(std::fstream& outFile) const = 0;

	virtual void load_bch(BinaryTraversal& traversal) = 0;
	virtual bool save_bch(std::fstream& outFile) const = 0;

	// Returns whether any difficulty in this track contains notes
	// ONLY checks for notes
	virtual bool hasNotes() const = 0;
	// Returns whether this track contains notes, special phrases, or other events
	virtual bool occupied() const = 0;
	virtual void clear() = 0;
	virtual void adjustTicks(float multiplier) = 0;
	virtual ~NoteTrack() {}
};
