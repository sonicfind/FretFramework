#pragma once
#include "Base Nodes/Sustainable.h"
#include "InstrumentalNote.h"

template <int numColors>
class Keys : public InstrumentalNote_NoSpec<numColors, Sustainable>
{
public:
	using InstrumentalNote_NoSpec<numColors, Sustainable>::m_colors;

	constexpr explicit Keys() : InstrumentalNote_NoSpec<numColors, Sustainable>() {}

	void init_chartV1(const unsigned char lane, const uint32_t sustain)
	{
		if (lane < numColors)
			m_colors[lane].init(sustain);
		else
			throw InvalidNoteException(lane);
	}

	uint32_t getLongestSustain() const
	{
		uint32_t sustain = 0;
		for (const auto& color : m_colors)
			if (color && color.getSustain() > sustain)
				sustain = color.getSustain();
		return sustain;
	}
};
