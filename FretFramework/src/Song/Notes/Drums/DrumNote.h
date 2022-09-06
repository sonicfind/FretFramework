#pragma once
#include "Drums.h"
#include "InstrumentalNote.h"

template<int numPads, class PadType>
class DrumNote : public InstrumentalNote<numPads, PadType, DrumPad_Bass>
{
public:
	using InstrumentalNote<numPads, PadType, DrumPad_Bass>::m_colors;
	using InstrumentalNote<numPads, PadType, DrumPad_Bass>::m_special;
	Toggleable m_isFlamed;

private:
	void checkFlam()
	{
		int numActive = 0;
		for (const auto& pad : m_colors)
			if (pad)
				++numActive;
		m_isFlamed = numActive < 2;
	}

public:
	constexpr explicit DrumNote() : InstrumentalNote<numPads, PadType, DrumPad_Bass>() {}
	void init_chartV1(const unsigned char lane, const uint32_t sustain);

	using InstrumentalNote<numPads, PadType, DrumPad_Bass>::modify;
	void modify(char modifier, unsigned char lane = 0)
	{
		switch (modifier)
		{
		case 'F':
		case 'f':
			m_isFlamed.toggle();
			break;
		default:
			if (lane == 0)
				m_special.modify(modifier);
			else if (lane <= numPads)
				m_colors[lane - 1].modify(modifier);
		}
	}

	void modify_binary(char modifier, unsigned char lane = 0)
	{
		if (modifier & 1)
			m_isFlamed.toggle();

		if (modifier < 0)
		{
			if (lane == 0)
				m_special.modify_binary(modifier);
			else if (lane <= numPads)
				m_colors[lane - 1].modify_binary(modifier);
		}
	}

	static bool testIndex_chartV1(const unsigned char lane)
	{
		return lane <= numPads;
	}

protected:
	int write_modifiers_single(std::stringstream& buffer) const;
	int write_modifiers_chord(std::stringstream& buffer) const;
	void write_modifiers_single(char*& buffer) const;
	char write_modifiers_chord(char*& buffer) const;
};

template<>
void DrumNote<4, DrumPad_Pro>::init_chartV1(unsigned char lane, uint32_t sustain);
template<>
void DrumNote<5, DrumPad>::init_chartV1(unsigned char lane, uint32_t sustain);


class DrumNote_Legacy : public DrumNote<5, DrumPad_Pro>
{
public:
	void init_chartV1(unsigned char lane, uint32_t sustain);
	void init(unsigned char lane, uint32_t sustain = 0);

	using DrumNote<5, DrumPad_Pro>::modify;
	void modify(char modifier, unsigned char lane = 0);
	void modify_binary(char modifier, unsigned char lane = 0);
	operator DrumNote<4, DrumPad_Pro>() const;
	operator DrumNote<5, DrumPad>() const;
};
