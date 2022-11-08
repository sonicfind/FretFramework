#include "SongListEntry.h"

enum class Instrument
{
	Guitar_lead,
	Guitar_lead_6,
	Guitar_bass,
	Guitar_bass_6,
	Guitar_rhythm,
	Guitar_coop,
	Keys,
	Drums_4,
	Drums_5,
	Vocals,
	Harmonies,
	Drums_Legacy,
	None
};

void SongListEntry::scanFile(TextTraversal&& traversal)
{
	uint16_t version = 0;
	InstrumentalTrack_Scan<DrumNote_Legacy> drumsLegacy_scan;
	while (traversal)
	{
		if (traversal != '[')
		{
			traversal.next();
			continue;
		}
		
		traversal.setTrackName();
		traversal.next();

		if (traversal == '{')
			traversal.next();

		if (traversal.isTrackName("[Song]"))
		{
			static std::pair<std::string_view, ModifierNode> constexpr PREDEFINED_MODIFIERS[]
			{
				{ "Album",        { "album", ModifierNode::STRING_CHART } },
				{ "Artist",       { "artist", ModifierNode::STRING_CHART } },
				{ "Charter",      { "charter", ModifierNode::STRING_CHART } },
				{ "Difficulty",   { "diff_band", ModifierNode::INT32} },
				{ "FileVersion",  { "FileVersion", ModifierNode::UINT16} },
				{ "Genre",        { "genre", ModifierNode::STRING_CHART } },
				{ "Name",         { "name", ModifierNode::STRING_CHART } },
				{ "Offset",       { "delay", ModifierNode::FLOAT} },
				{ "PreviewEnd",   { "preview_end_time", ModifierNode::FLOAT} },
				{ "PreviewStart", { "preview_start_time", ModifierNode::FLOAT} },
				{ "Year",         { "year", ModifierNode::STRING_CHART } },
			};

			auto [ver, tick] = readModifiersFromChart(PREDEFINED_MODIFIERS, traversal);
			version = ver;
		}
		else if (traversal.isTrackName("[SyncTrack]") || traversal.isTrackName("[Events]"))
			traversal.skipTrack();
		else if (version > 1)
		{
			int i = 0;
			while (i < 11 && !traversal.isTrackName(s_NOTETRACKNAMES[i]))
				++i;

			if (i < 11)
				m_noteTrackScans.scanArray[i]->scan_cht(traversal);
			else
				traversal.skipTrack();
		}
		else
		{
			int difficulty = -1;
			if (traversal.cmpTrackName("[Expert"))
				difficulty = 3;
			else if (traversal.cmpTrackName("[Hard"))
				difficulty = 2;
			else if (traversal.cmpTrackName("[Medium"))
				difficulty = 1;
			else if (traversal.cmpTrackName("[Easy"))
				difficulty = 0;

			Instrument ins = Instrument::None;
			if (traversal.cmpTrackName("Single]"))
				ins = Instrument::Guitar_lead;
			else if (traversal.cmpTrackName("DoubleGuitar]"))
				ins = Instrument::Guitar_coop;
			else if (traversal.cmpTrackName("DoubleBass]"))
				ins = Instrument::Guitar_bass;
			else if (traversal.cmpTrackName("DoubleRhythm]"))
				ins = Instrument::Guitar_rhythm;
			else if (traversal.cmpTrackName("Drums]"))
			{
				if (TxtFileModifier* fiveLaneDrums = getModifier("five_lane_drums"))
				{
					if (fiveLaneDrums->getValue<bool>())
						ins = Instrument::Drums_5;
					else
						ins = Instrument::Drums_4;
				}
				else if (drumsLegacy_scan.isFiveLane())
					ins = Instrument::Drums_5;
				else if (drumsLegacy_scan.isDrum4Pro())
					ins = Instrument::Drums_4;
				else
					ins = Instrument::Drums_Legacy;
						
			}
			else if (traversal.cmpTrackName("Keys]"))
				ins = Instrument::Keys;
			else if (traversal.cmpTrackName("GHLGuitar]"))
				ins = Instrument::Guitar_lead_6;
			else if (traversal.cmpTrackName("GHLBass]"))
				ins = Instrument::Guitar_bass_6;

			if (ins != Instrument::None && difficulty != -1)
			{
				switch (ins)
				{
				case Instrument::Guitar_lead:
					m_noteTrackScans.lead_5.scan_chart_V1(difficulty, traversal);
					break;
				case Instrument::Guitar_lead_6:
					m_noteTrackScans.lead_6.scan_chart_V1(difficulty, traversal);
					break;
				case Instrument::Guitar_bass:
					m_noteTrackScans.bass_5.scan_chart_V1(difficulty, traversal);
					break;
				case Instrument::Guitar_bass_6:
					m_noteTrackScans.bass_6.scan_chart_V1(difficulty, traversal);
					break;
				case Instrument::Guitar_rhythm:
					m_noteTrackScans.rhythm.scan_chart_V1(difficulty, traversal);
					break;
				case Instrument::Guitar_coop:
					m_noteTrackScans.coop.scan_chart_V1(difficulty, traversal);
					break;
				case Instrument::Keys:
					m_noteTrackScans.keys.scan_chart_V1(difficulty, traversal);
					break;
				case Instrument::Drums_Legacy:
					drumsLegacy_scan.scan_chart_V1(difficulty, traversal);
					break;
				case Instrument::Drums_4:
					m_noteTrackScans.drums4_pro.scan_chart_V1(difficulty, traversal);
					break;
				case Instrument::Drums_5:
					m_noteTrackScans.drums5.scan_chart_V1(difficulty, traversal);
					break;
				}
			}
			else
				traversal.skipTrack();
		}
	}

	m_noteTrackScans.scanArray[7 + drumsLegacy_scan.isFiveLane()]->m_scanValue |= drumsLegacy_scan.m_scanValue;
}
