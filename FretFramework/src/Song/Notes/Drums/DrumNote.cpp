#include "DrumNote.h"
bool DrumNote::s_is5Lane = false;
void DrumNote::checkFlam()
{
	int numActive = 0;
	for (int i = 0; i < 4; ++i)
		if (m_colors[i])
			++numActive;
	if (m_fifthLane)
		++numActive;

	m_isFlamed = numActive < 2;
}

bool DrumNote::init(size_t lane, uint32_t sustain)
{
	if (!Note<4, DrumPad_Pro, DrumPad_Bass>::init(lane, sustain))
	{
		s_is5Lane = true;
		m_fifthLane.init(sustain);
	}
	return true;
}

void DrumNote::init_chartV1(int lane, uint32_t sustain)
{
	if (lane == 0)
		m_special.init(sustain);
	else if (lane < 5)
		m_colors[lane - 1].init(sustain);
	else if (lane == 5)
	{
		s_is5Lane = true;
		m_fifthLane.init(sustain);
	}
	else if (lane == 32)
		m_special.m_isDoubleBass = true;
	else if (lane >= 66 && lane <= 68)
		m_colors[lane - 65].modify('C');
	else
		throw InvalidNoteException(lane);
}

void DrumNote::init_cht_single(const char* str)
{
	// Read note
	int lane, count;
	if (sscanf_s(str, " %i%n", &lane, &count) != 1)
		throw EndofLineException();

	str += count;
	unsigned char color = lane & 127;
	uint32_t sustain = 0;
	if (lane & 128)
	{
		if (sscanf_s(str, " %lu%n", &sustain, &count) != 1)
			throw EndofLineException();
		str += count;
	}

	if (color > 5)
		throw InvalidNoteException(color);

	Hittable* note = nullptr;
	Modifiable* mod = nullptr;
	if (color == 0)
	{
		note = &m_special;
		mod = &m_special;
	}
	else if (color < 5)
	{
		note = &m_colors[color - 1];
		mod = &m_colors[color - 1];
	}
	else
	{
		note = &m_fifthLane;
		mod = &m_fifthLane;
		s_is5Lane = true;
	}
	note->init(sustain);

	// Read modifiers
	int numMods;
	if (sscanf_s(str, " %i%n", &numMods, &count) == 1)
	{
		str += count;
		char modifier;
		for (int i = 0;
			i < numMods && sscanf_s(str, " %c%n", &modifier, 1, &count) == 1;
			++i)
		{
			str += count;
			if (modifier == 'F')
			{
				if (!m_isFlamed)
					checkFlam();
				else
					m_isFlamed = false;
			}
			else
				mod->modify(modifier);
		}
	}
}

void DrumNote::init_cht_chord(const char* str)
{
	int colors;
	int count;
	if (sscanf_s(str, " %i%n", &colors, &count) != 1)
		throw EndofLineException();
	
	str += count;
	int numAdded = 0;
	int lane;
	for (int i = 0;
		i < colors && sscanf_s(str, " %i%n", &lane, &count) == 1;
		++i)
	{
		unsigned char color = lane & 127;
		uint32_t sustain = 0;
		if (lane & 128)
		{
			str += count;
			if (sscanf_s(str, " %lu%n", &sustain, &count) != 1)
				throw EndofLineException();
		}

		if (color <= 5)
		{
			if (color == 0)
				m_special.init(sustain);
			else if (color < 5)
				m_colors[color - 1].init(sustain);
			else
			{
				m_fifthLane.init(sustain);
				s_is5Lane = true;
			}
			++numAdded;
		}
		str += count;
	}

	if (numAdded == 0)
		throw InvalidNoteException();
}

bool DrumNote::modify(char modifier, bool toggle)
{
	if (modifier == 'F')
	{
		if (!m_isFlamed)
			checkFlam();
		else
			m_isFlamed = false;
		return true;
	}
	return false;
}

bool DrumNote::modifyColor(int lane, char modifier)
{
	if (lane == 0)
		return m_special.modify(modifier);
	else if (lane < 5)
		return m_colors[lane - 1].modify(modifier);
	else
		return m_fifthLane.modify(modifier);
}

void DrumNote::modify_cht(const char* str)
{
	int numMods;
	int count;
	if (sscanf_s(str, " %i%n", &numMods, &count) == 1)
	{
		str += count;
		char modifier;
		for (int i = 0;
			i < numMods && sscanf_s(str, " %c%n", &modifier, 1, &count) == 1;
			++i)
		{
			switch (modifier)
			{
			case 'F':
				m_isFlamed = true;
				break;
			case '+':
				m_special.m_isDoubleBass = true;
				break;
			default:
			{
				str += count;

				int lane;
				if (sscanf_s(str, " %i%n", &lane, &count) != 1)
					return;

				if (lane > 0)
					m_colors[lane - 1].modify(modifier);
			}
			}
			str += count;
		}
	}
}

void DrumNote::save_cht(const uint32_t position, std::fstream& outFile) const
{
	uint32_t numActive = Note<4, DrumPad_Pro, DrumPad_Bass>::write_notes_cht(position, outFile);
	if (m_fifthLane)
		m_fifthLane.save_cht(5, outFile);

	int numMods = m_isFlamed ? 1 : 0;
	if (m_special && m_special.m_isDoubleBass)
		++numMods;

	for (int i = 0; i < 4; ++i)
	{
		if (m_colors[i])
		{
			if (m_colors[i].m_isCymbal)
				++numMods;

			if (m_colors[i].m_isAccented || m_colors[i].m_isGhosted)
				++numMods;
		}
	}

	if (m_fifthLane &&
		(m_fifthLane.m_isAccented || m_fifthLane.m_isGhosted))
		++numMods;

	if (numMods > 0)
	{
		if (numActive == 1)
		{
			outFile << ' ' << numMods;
			if (m_isFlamed)
				outFile << " F";

			if (m_special)
				m_special.save_modifier_cht(outFile);
			else if (m_fifthLane)
				m_fifthLane.save_modifier_cht(outFile);
			else
			{
				int i = 0;
				while (!m_colors[i])
					++i;

				m_colors[i].save_modifier_cht(outFile);
			}
		}
		else
		{
			outFile << "\n\t\t" << position << " = M " << numMods;

			if (m_isFlamed)
				outFile << " F";

			if (m_special)
				m_special.save_modifier_cht(0, outFile);

			for (int i = 0; i < 4; ++i)
				if (m_colors[i])
					m_colors[i].save_modifier_cht(i + 1, outFile);

			if (m_fifthLane)
				m_fifthLane.save_modifier_cht(5, outFile);
		}
	}
	outFile << '\n';
}

void DrumNote::resetLaning()
{
	s_is5Lane = false;
}
