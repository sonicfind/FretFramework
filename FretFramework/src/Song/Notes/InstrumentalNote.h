#pragma once
#include <stdint.h>
#include "NoteExceptions.h"
#include "FileTraversal/TextFileTraversal.h"
#include "FileTraversal/BCHFileTraversal.h"
#include <fstream>
#include <sstream>

template <int numColors, class NoteType>
class InstrumentalNote_NoSpec
{
public:
	NoteType m_colors[numColors];

	virtual void init(unsigned char lane, uint32_t sustain = 0)
	{
		if (lane > numColors)
			throw InvalidNoteException(lane);

		m_colors[lane - 1].init(sustain);
	}

	void init_chartV1(unsigned char lane, uint32_t sustain);
private:
	unsigned char read_note(TextTraversal& traversal);
	unsigned char read_note(BCHTraversal& traversal);

public:

	void init_single(TextTraversal& traversal);
	void init_chord(TextTraversal& traversal);
	void init_single(BCHTraversal& traversal);
	void init_chord(BCHTraversal& traversal);

	virtual void modify(char modifier, unsigned char lane = 0) {}
	virtual void modify_binary(char modifier, unsigned char lane = 0) {}
	void modify(TextTraversal& traversal);
	void modify(BCHTraversal& traversal);

protected:
	virtual int write_notes(std::stringstream& buffer) const;
	virtual int write_modifiers_single(std::stringstream& buffer) const;
	virtual int write_modifiers_chord(std::stringstream& buffer) const;
	virtual char write_notes(char*& buffer) const;
	virtual void write_modifiers_single(char*& buffer) const;
	virtual char write_modifiers_chord(char*& buffer) const;

public:
	constexpr explicit InstrumentalNote_NoSpec() {}
	constexpr virtual ~InstrumentalNote_NoSpec() {}
	void save_cht(uint32_t position, std::fstream& outFile) const;
	uint32_t save_bch(std::fstream& outFile) const;

	virtual uint32_t getNumActive() const
	{
		uint32_t num = 0;
		for (int lane = 0; lane < numColors; ++lane)
			if (m_colors[lane])
				++num;
		return num;
	}

	bool operator==(const InstrumentalNote_NoSpec& note) const
	{
		for (int i = 0; i < numColors; ++i)
			if (m_colors[i] != note.m_colors[i])
				return false;
		return true;
	}

	bool operator!=(const InstrumentalNote_NoSpec& note) const
	{
		return !operator==(note);
	}

	void operator*=(float multiplier)
	{
		for (auto& col : m_colors)
			col *= multiplier;
	}

	static uint32_t getLaneSize()
	{
		return numColors;
	}
};

template <int numColors, class NoteType, class SpecialType>
class InstrumentalNote : public InstrumentalNote_NoSpec<numColors, NoteType>
{
public:
	using InstrumentalNote_NoSpec<numColors, NoteType>::m_colors;
	SpecialType m_special;

	void init(unsigned char lane, uint32_t sustain = 0)
	{
		if (lane == 0)
			m_special.init(sustain);
		else
			InstrumentalNote_NoSpec<numColors, NoteType>::init(lane, sustain);
	}

private:
	int write_notes(std::stringstream& buffer) const;
	char write_notes(char*& buffer) const;

public:
	constexpr explicit InstrumentalNote() : InstrumentalNote_NoSpec<numColors, NoteType>() {}
	virtual uint32_t getNumActive() const
	{
		uint32_t num = InstrumentalNote_NoSpec<numColors, NoteType>::getNumActive();
		if (m_special)
			++num;
		return num;
	}

	bool operator==(const InstrumentalNote& note) const
	{
		return m_special == note.m_special && InstrumentalNote_NoSpec<numColors, NoteType>::operator==(note);
	}

	bool operator!=(const InstrumentalNote& note) const
	{
		return !operator==(note);
	}

	void operator*=(float multiplier)
	{
		InstrumentalNote_NoSpec<numColors, NoteType>::operator*=(multiplier);
		m_special *= multiplier;
	}
};
