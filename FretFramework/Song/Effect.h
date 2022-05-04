#pragma once
#include <stdint.h>
#include <fstream>

class Effect
{
protected:
	const char m_midiNote;
	const int  m_chtType;

	Effect(char midi, int cht);
public:
	virtual void save_cht(uint32_t position, std::fstream& outFile, const char* const tabs = "\t\t") const;
	virtual ~Effect() {}
	char getMidiNote() const { return m_midiNote; }
};

class SustainableEffect : public Effect
{
	uint32_t m_duration = 0;
protected:
	SustainableEffect(char midi, int cht, uint32_t duration);

public:
	void save_cht(uint32_t position, std::fstream& outFile, const char* const tabs = "\t\t") const;
	uint32_t getDuration() const { return m_duration; }
};

class StarPowerPhrase : public SustainableEffect
{
public:
	StarPowerPhrase(uint32_t duration);
};

class StarPowerActivation : public SustainableEffect
{
public:
	StarPowerActivation(uint32_t duration);
};

class Solo : public SustainableEffect
{
public:
	Solo(uint32_t duration);
};

// AKA rolls
class Tremolo : public SustainableEffect
{
public:
	Tremolo(uint32_t duration);
};

// AKA Special rolls (multiple hands/frets)
class Trill : public SustainableEffect
{
public:
	Trill(uint32_t duration);
};

class LyricLine : public SustainableEffect
{
public:
	LyricLine(uint32_t duration);
};

class RangeShift : public SustainableEffect
{
public:
	RangeShift(uint32_t duration);
};

class LyricShift : public SustainableEffect
{
public:
	LyricShift(uint32_t duration);
};

class HarmonyLine : public SustainableEffect
{
public:
	HarmonyLine(uint32_t duration);
};
