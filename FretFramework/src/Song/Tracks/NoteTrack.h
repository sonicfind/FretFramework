#pragma once
#include "FileTraversal\TextFileTraversal.h"
#include "FileTraversal\BCHFileTraversal.h"
#include <fstream>

class NoteTrack_Scan
{
protected:
	int m_scanValue;

public:
	NoteTrack_Scan(int defaultValue = 0) : m_scanValue(defaultValue) {}
	int getValue() const { return m_scanValue; }
	void addFromValue(const int value) { m_scanValue |= value; }
	virtual void scan_cht(TextTraversal& traversal) = 0;
	virtual void scan_bch(BCHTraversal& traversal) = 0;
	virtual ~NoteTrack_Scan() {}

	virtual std::string toString() = 0;
};

class NoteTrack
{
protected:
	const char m_instrumentID;

public:
	static unsigned char s_starPowerReadNote;

	const char* const m_name;
	constexpr NoteTrack(const char* name, char instrumentID)
		: m_name(name)
		, m_instrumentID(instrumentID) {}

	virtual void scan_cht(TextTraversal& traversal, std::unique_ptr<NoteTrack_Scan>& track) const = 0;
	virtual void load_cht(TextTraversal& traversal) = 0;
	virtual void save_cht(std::fstream& outFile) const = 0;

	virtual void scan_bch(BCHTraversal& traversal, std::unique_ptr<NoteTrack_Scan>& track) const = 0;
	virtual void load_bch(BCHTraversal& traversal) = 0;
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
