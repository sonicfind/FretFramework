#include "Song.h"

void Song::scanFile(TextTraversal&& traversal)
{
	int version = 0;
	InstrumentalTrack_Scan<DrumNote_Legacy>* drumsLegacy_scan = nullptr;
	do
	{
		if (traversal != '[')
		{
			traversal.next();
			continue;
		}

		traversal.next();

		if (traversal == '{')
			traversal.next();

		if (traversal.isTrackName("[Song]"))
		{
			static std::pair<std::string_view, std::unique_ptr<TxtFileModifier>(*)()> constexpr PREDEFINED_MODIFIERS[]
			{
			#define M_PAIR(inputString, ModifierType, outputString)\
						{ inputString, []() -> std::unique_ptr<TxtFileModifier> { return std::make_unique<ModifierType>(outputString); } }
				M_PAIR("Album",        StringModifier_Chart, "album"),
				M_PAIR("Artist",       StringModifier_Chart, "artist"),
				M_PAIR("Charter",      StringModifier_Chart, "charter"),
				M_PAIR("Difficulty",   INT32Modifier,        "diff_band"),
				M_PAIR("FileVersion",  UINT16Modifier,       "FileVersion"),
				M_PAIR("Genre",        StringModifier_Chart, "genre"),
				M_PAIR("Name",         StringModifier_Chart, "name"),
				M_PAIR("Offset",       FloatModifier,        "delay"),
				M_PAIR("PreviewEnd",   FloatModifier,        "preview_end_time"),
				M_PAIR("PreviewStart", FloatModifier,        "preview_start_time"),
				M_PAIR("Year",         StringModifier_Chart, "year"),
			#undef M_PAIR
			};

			bool versionChecked = false;
			while (traversal && traversal != '}' && traversal != '[')
			{
				if (auto modifier = traversal.extractModifier(PREDEFINED_MODIFIERS))
				{
					const std::string_view name = modifier->getName();
					if (name[0] == 'F')
					{
						if (!versionChecked)
						{
							modifier->read(traversal);
							version = static_cast<UINT16Modifier*>(modifier.get())->m_value;
							versionChecked = true;
						}
					}
					else if (!m_hasIniFile || !getModifier(name))
					{
						modifier->read(traversal);
						m_modifiers.push_back(std::move(modifier));
					}
				}
				traversal.next();
			}
		}
		else if (traversal.isTrackName("[SyncTrack]") || traversal.isTrackName("[Events]"))
			traversal.skipTrack();
		else if (version > 1)
		{
			int i = 0;
			while (i < 11 && !traversal.isTrackName(s_noteTracks[i]->m_name))
				++i;

			if (i < 11)
				s_noteTracks[i]->scan_cht(traversal, m_noteTrackScans[i]);
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
				if (BooleanModifier* fiveLaneDrums = getModifier<BooleanModifier>("five_lane_drums"))
				{
					if (fiveLaneDrums->m_boolean)
						ins = Instrument::Drums_5;
					else
						ins = Instrument::Drums_4;
				}
				else if (drumsLegacy_scan && drumsLegacy_scan->isFiveLane())
					ins = Instrument::Drums_5;
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
					reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[0].get())->scan_chart_V1(difficulty, traversal, m_noteTrackScans[0]);
					break;
				case Instrument::Guitar_lead_6:
					reinterpret_cast<InstrumentalTrack<GuitarNote<6>>*>(s_noteTracks[1].get())->scan_chart_V1(difficulty, traversal, m_noteTrackScans[1]);
					break;
				case Instrument::Guitar_bass:
					reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[2].get())->scan_chart_V1(difficulty, traversal, m_noteTrackScans[2]);
					break;
				case Instrument::Guitar_bass_6:
					reinterpret_cast<InstrumentalTrack<GuitarNote<6>>*>(s_noteTracks[3].get())->scan_chart_V1(difficulty, traversal, m_noteTrackScans[3]);
					break;
				case Instrument::Guitar_rhythm:
					reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[4].get())->scan_chart_V1(difficulty, traversal, m_noteTrackScans[4]);
					break;
				case Instrument::Guitar_coop:
					reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[5].get())->scan_chart_V1(difficulty, traversal, m_noteTrackScans[5]);
					break;
				case Instrument::Keys:
					reinterpret_cast<InstrumentalTrack<Keys<5>>*>(s_noteTracks[6].get())->scan_chart_V1(difficulty, traversal, m_noteTrackScans[6]);
					break;
				case Instrument::Drums_Legacy:
					if (drumsLegacy_scan == nullptr)
						drumsLegacy_scan = new InstrumentalTrack_Scan<DrumNote_Legacy>;
					drumsLegacy_scan->scan_chart_V1(difficulty, traversal);
					break;
				case Instrument::Drums_4:
					reinterpret_cast<InstrumentalTrack<DrumNote<4, DrumPad_Pro>>*>(s_noteTracks[7].get())->scan_chart_V1(difficulty, traversal, m_noteTrackScans[7]);
					break;
				case Instrument::Drums_5:
					reinterpret_cast<InstrumentalTrack<DrumNote<5, DrumPad>>*>(s_noteTracks[8].get())->scan_chart_V1(difficulty, traversal, m_noteTrackScans[8]);
					break;
				}
			}
			else
				traversal.skipTrack();
		}
	} while (traversal);

	if (drumsLegacy_scan)
	{
		if (drumsLegacy_scan->getValue() > 0)
		{
			if (!drumsLegacy_scan->isFiveLane())
				m_noteTrackScans[7] = std::make_unique<InstrumentalTrack_Scan<DrumNote<4, DrumPad_Pro>>>();
			else
				m_noteTrackScans[8] = std::make_unique<InstrumentalTrack_Scan<DrumNote<5, DrumPad>>>();
			m_noteTrackScans[7 + drumsLegacy_scan->isFiveLane()]->addFromValue(drumsLegacy_scan->getValue());
		}
		delete drumsLegacy_scan;
	}
}
