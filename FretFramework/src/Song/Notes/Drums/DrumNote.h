#pragma once
#include "Drums.h"
#include "Note.h"

class DrumNote : public Note<5, DrumPad_Pro, DrumPad_Bass>
{
	static bool s_is5Lane;
public:
	using Note<5, DrumPad_Pro, DrumPad_Bass>::m_special;
	using Note<5, DrumPad_Pro, DrumPad_Bass>::m_colors;
	Toggleable m_isFlamed;

private:
	void checkFlam();

public:
	using Note<5, DrumPad_Pro, DrumPad_Bass>::Note;
	void init_chartV1(unsigned char lane, uint32_t sustain);
	void init(unsigned char lane, uint32_t sustain = 0);

	void modify(char modifier, unsigned char lane = 0);
	void modify_binary(char modifier, unsigned char lane = 0);

	void save_cht(const uint32_t position, std::fstream& outFile) const;
	uint32_t save_bch(const uint32_t position, std::fstream& outFile) const;
	static bool isFiveLane() { return s_is5Lane; }
	static void resetLaning();

	void operator*=(float multiplier)
	{
		Note<5, DrumPad_Pro, DrumPad_Bass>::operator*=(multiplier);
	}
};
