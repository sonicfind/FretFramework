#pragma once
#include <stdint.h>
#include <fstream>

class Phrase
{
protected:
	const char m_midiNote;
	const int  m_chtType;

	Phrase(char midi, int cht);
public:
	virtual void save_cht(uint32_t position, std::fstream& outFile, const char* const tabs = "\t\t") const;
	virtual ~Phrase() {}
	char getMidiNote() const { return m_midiNote; }
	// 1 instead of 0 as midi can't function correctly otherwise
	virtual uint32_t getDuration() { return 1; }
};

class SustainablePhrase : public Phrase
{
	uint32_t m_duration = 0;
protected:
	SustainablePhrase(char midi, int cht, uint32_t duration);

public:
	void save_cht(uint32_t position, std::fstream& outFile, const char* const tabs = "\t\t") const;
	uint32_t getDuration() const { return m_duration; }
};

class StarPowerPhrase : public SustainablePhrase
{
public:
	StarPowerPhrase(uint32_t duration);
};

class StarPowerActivation : public SustainablePhrase
{
public:
	StarPowerActivation(uint32_t duration);
};

class Solo : public SustainablePhrase
{
public:
	Solo(uint32_t duration);
};

// AKA rolls
class Tremolo : public SustainablePhrase
{
public:
	Tremolo(uint32_t duration);
};

// AKA Special rolls (multiple hands/frets)
class Trill : public SustainablePhrase
{
public:
	Trill(uint32_t duration);
};

class LyricLine : public SustainablePhrase
{
public:
	LyricLine(uint32_t duration);
};

class RangeShift : public SustainablePhrase
{
public:
	RangeShift(uint32_t duration);
};

class LyricShift : public Phrase
{
public:
	LyricShift();
};

class HarmonyLine : public SustainablePhrase
{
public:
	HarmonyLine(uint32_t duration);
};