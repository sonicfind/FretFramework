#pragma once
#include <stdint.h>
#include <fstream>

class Effect
{
public:
	virtual void save_cht(uint32_t position, std::fstream& outFile) const {}
	virtual ~Effect() {}
	virtual char getMidiNote() const = 0;
};

class SustainableEffect : public Effect
{
protected:
	uint32_t m_duration = 0;
public:
	SustainableEffect(uint32_t duration);
	uint32_t getDuration() const { return m_duration; }
};

class StarPowerPhrase : public SustainableEffect
{
	using SustainableEffect::SustainableEffect;
	void save_cht(uint32_t position, std::fstream& outFile) const;
	char getMidiNote() const;
};

class StarPowerActivation : public SustainableEffect
{
	using SustainableEffect::SustainableEffect;
	void save_cht(uint32_t position, std::fstream& outFile) const;
	char getMidiNote() const;
};

class Solo : public SustainableEffect
{
	using SustainableEffect::SustainableEffect;
	void save_cht(uint32_t position, std::fstream& outFile) const;
	char getMidiNote() const;
};

// AKA rolls
class Tremolo : public SustainableEffect
{
	using SustainableEffect::SustainableEffect;
	void save_cht(uint32_t position, std::fstream& outFile) const;
	char getMidiNote() const;
};

// AKA Special rolls (multiple hands/frets)
class Trill : public SustainableEffect
{
	using SustainableEffect::SustainableEffect;
	void save_cht(uint32_t position, std::fstream& outFile) const;
	char getMidiNote() const;
};
