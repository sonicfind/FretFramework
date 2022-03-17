#pragma once
#include <stdint.h>
#include <fstream>

class Effect
{
protected:
	uint32_t m_duration = 0;
public:
	Effect(uint32_t duration);
	virtual void save_chart(uint32_t position, std::fstream& outFile) = 0;
	virtual ~Effect() {}
};

class StarPowerPhrase : public Effect
{
	using Effect::Effect;
	void save_chart(uint32_t position, std::fstream& outFile);
};

class StarPowerActivation : public Effect
{
	using Effect::Effect;
	void save_chart(uint32_t position, std::fstream& outFile);
};
