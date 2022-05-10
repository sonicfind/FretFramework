#include "Note_cht.hpp"
#include "DrumNote.h"

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

void DrumNote::init_cht_single(TextTraversal& traversal)
{
	// Read note
	uint32_t lane;
	if (!traversal.extractUInt(lane))
		throw EndofLineException();

	unsigned char color = lane & 127;
	uint32_t sustain = 0;
	if (lane & 128)
		if (!traversal.extractUInt(sustain))
			throw EndofLineException();

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
	uint32_t numMods;
	if (traversal.extractUInt(numMods))
	{
		for (uint32_t i = 0; i < numMods; ++i)
		{
			switch (traversal.getChar())
			{
			case 'f':
			case 'F':
				if (!m_isFlamed)
					checkFlam();
				else
					m_isFlamed = false;
				break;
			default:
				mod->modify(traversal.getChar());
			}
			traversal.move(1);
		}
	}
}

void DrumNote::init_cht_chord(TextTraversal& traversal)
{
	uint32_t colors;
	if (traversal.extractUInt(colors))
	{
		int numAdded = 0;
		uint32_t lane;
		for (uint32_t i = 0; i < colors; ++i)
		{
			if (!traversal.extractUInt(lane))
				throw EndofLineException();

			unsigned char color = lane & 127;
			uint32_t sustain = 0;
			if (lane & 128)
				if (!traversal.extractUInt(sustain))
					throw EndofLineException();

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
		}

		if (numAdded == 0)
			throw InvalidNoteException();
	}
	else
		throw EndofLineException();
}

void DrumNote::modify_cht(TextTraversal& traversal)
{
	uint32_t numMods;
	if (traversal.extractUInt(numMods))
	{
		for (uint32_t i = 0; i < numMods; ++i)
		{
			char mod = traversal.getChar();
			traversal.move(1);
			switch (mod)
			{
			case 'F':
				m_isFlamed = true;
				break;
			case '+':
				m_special.m_isDoubleBass = true;
				break;
			default:
			{
				uint32_t lane;
				if (!traversal.extractUInt(lane))
					return;

				if (lane == 5)
					m_fifthLane.modify(mod);
				else if (0 < lane && lane < 5)
					m_colors[lane - 1].modify(mod);
			}
			}
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
