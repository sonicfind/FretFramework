#pragma once
#include "Sync/SyncTrack.h"
#include <fstream>
class NoteTrack_Scan
{
protected:
	int m_scanValaue;

public:
	NoteTrack_Scan(int defaultValue = 0) : m_scanValaue(defaultValue) {}
	int getValue() const { return m_scanValaue; }
	void addFromValue(const int value) { m_scanValaue |= value; }
	virtual void scan_cht(TextTraversal& traversal) = 0;
	virtual void scan_bch(BCHTraversal& traversal) = 0;
	virtual ~NoteTrack_Scan() {}
};

class NoteTrack
{
protected:
	const char m_instrumentID;

public:
	static unsigned char s_starPowerReadNote;
	static SyncTrack s_syncTrack;

	const char* const m_name;
	NoteTrack(const char* name, char instrumentID)
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
