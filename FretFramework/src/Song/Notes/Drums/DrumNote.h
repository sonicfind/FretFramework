#pragma once
#include "Drums.h"
#include "Note.h"

class DrumNote : public Note<4, DrumPad_Pro, DrumPad_Bass>
{
	static bool s_is5Lane;
public:
	using Note<4, DrumPad_Pro, DrumPad_Bass>::m_special;
	using Note<4, DrumPad_Pro, DrumPad_Bass>::m_colors;
	DrumPad m_fifthLane;
	Toggleable m_isFlamed;

private:
	void checkFlam();

public:
	using Note<4, DrumPad_Pro, DrumPad_Bass>::Note;
	void init_chartV1(int lane, uint32_t sustain);
	bool init(size_t lane, uint32_t sustain = 0);
	void init_cht_single(const char* str);
	void init_cht_chord(const char* str);
	bool modify(char modifier, bool toggle = true);
	bool modifyColor(int lane, char modifier);
	void modify_cht(const char* str);
	void save_cht(const uint32_t position, std::fstream& outFile) const;
	static void resetLaning();

	void operator*=(float multiplier)
	{
		Note<4, DrumPad_Pro, DrumPad_Bass>::operator*=(multiplier);
		m_fifthLane *= multiplier;
	}

	uint32_t getNumActiveColors() const
	{
		uint32_t num = m_fifthLane ? 1 : 0;
		return num + Note::getNumActiveColors();
	}

	static uint32_t getLaneSize()
	{
		if (s_is5Lane)
			return 5;
		else
			return 4;
	}
};
