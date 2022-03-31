#pragma once
#include <stdint.h>
#include <fstream>

class Effect
{
protected:
	uint32_t m_duration = 0;
public:
	Effect(uint32_t duration);
	uint32_t getDuration() const { return m_duration; }
	virtual void save_chart(uint32_t position, std::fstream& outFile) = 0;
	virtual ~Effect() {}
	virtual char getMidiNote() const = 0;
};

class StarPowerPhrase : public Effect
{
	using Effect::Effect;
	void save_chart(uint32_t position, std::fstream& outFile);
	char getMidiNote() const;
};

class StarPowerActivation : public Effect
{
	using Effect::Effect;
	void save_chart(uint32_t position, std::fstream& outFile);
	char getMidiNote() const;
};

// AKA rolls
class Tremolo : public Effect
{
	using Effect::Effect;
	void save_chart(uint32_t position, std::fstream& outFile);
	char getMidiNote() const;
};

// AKA Special rolls (multiple hands/frets)
class Trill : public Effect
{
	using Effect::Effect;
	void save_chart(uint32_t position, std::fstream& outFile);
	char getMidiNote() const;
};
