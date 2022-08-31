#include "IniFile.h"
#include "FileChecks/FilestreamCheck.h"

const StringModifier IniFile::s_DEFAULT_NAME{ "name" , U"Unknown Title" };
const StringModifier IniFile::s_DEFAULT_ARTIST{ "artist", U"Unknown Artist" };
const StringModifier IniFile::s_DEFAULT_ALBUM{ "album", U"Unknown Album" };
const StringModifier IniFile::s_DEFAULT_GENRE{ "genre", U"Unknown Genre" };
const StringModifier IniFile::s_DEFAULT_YEAR{ "year", U"Unknown Year" };
const StringModifier IniFile::s_DEFAULT_CHARTER{ "charter", U"Unknown Charter" };
const UINT32Modifier IniFile::s_DEFAULT_SONG_LENGTH{ "song_length" };

static std::pair<std::string_view, std::unique_ptr<TxtFileModifier>(*)()> constexpr PREDEFINED_MODIFIERS[]
{
#define M_PAIR(inputString, ModifierType, outputString)\
				 { inputString, []() -> std::unique_ptr<TxtFileModifier> { return std::make_unique<ModifierType>(outputString); } }
		M_PAIR("album",                                StringModifier,     "album"),
		M_PAIR("album_track",                          UINT32Modifier,     "album_track"),
		M_PAIR("artist",                               StringModifier,     "artist"),

		M_PAIR("background",                           StringModifier,     "background"),
		M_PAIR("banner_link_a",                        StringModifier,     "banner_link_a"),
		M_PAIR("banner_link_b",                        StringModifier,     "banner_link_b"),
		M_PAIR("bass_type",                            UINT32Modifier,     "bass_type"),
		M_PAIR("boss_battle",                          BooleanModifier,    "boss_battle"),

		M_PAIR("cassettecolor",                        UINT32Modifier,     "cassettecolor"),
		M_PAIR("charter",                              StringModifier,     "charter"),
		M_PAIR("count",                                UINT32Modifier,     "count"),
		M_PAIR("cover",                                StringModifier,     "cover"),

		M_PAIR("dance_type",                           UINT32Modifier,     "dance_type"),
		M_PAIR("delay",                                FloatModifier,      "delay"),
		M_PAIR("diff_band",                            BooleanModifier,    "diff_band"),
		M_PAIR("diff_bass",                            INT32Modifier,      "diff_bass"),
		M_PAIR("diff_bass_real",                       INT32Modifier,      "diff_bass_real"),
		M_PAIR("diff_bass_real_22",                    INT32Modifier,      "diff_bass_real_22"),
		M_PAIR("diff_bassghl",                         INT32Modifier,      "diff_bassghl"),
		M_PAIR("diff_dance",                           INT32Modifier,      "diff_dance"),
		M_PAIR("diff_drums",                           INT32Modifier,      "diff_drums"),
		M_PAIR("diff_drums_real",                      INT32Modifier,      "diff_drums_real"),
		M_PAIR("diff_drums_real_ps",                   INT32Modifier,      "diff_drums_real_ps"),
		M_PAIR("diff_guitar",                          INT32Modifier,      "diff_guitar"),
		M_PAIR("diff_guitar_coop",                     INT32Modifier,      "diff_guitar_coop"),
		M_PAIR("diff_guitar_real",                     INT32Modifier,      "diff_guitar_real"),
		M_PAIR("diff_guitar_real_22",                  INT32Modifier,      "diff_guitar_real_22"),
		M_PAIR("diff_guitarghl",                       INT32Modifier,      "diff_guitarghl"),
		M_PAIR("diff_keys",                            INT32Modifier,      "diff_keys"),
		M_PAIR("diff_keys_real",                       INT32Modifier,      "diff_keys_real"),
		M_PAIR("diff_keys_real_ps",                    INT32Modifier,      "diff_keys_real_ps"),
		M_PAIR("diff_rhythm",                          INT32Modifier,      "diff_rhythm"),
		M_PAIR("diff_vocals",                          INT32Modifier,      "diff_vocals"),
		M_PAIR("diff_vocals_harm",                     INT32Modifier,      "diff_vocals_harm"),
		M_PAIR("drum_fallback_blue",                   BooleanModifier,    "drum_fallback_blue"),

		M_PAIR("early_hit_window_size",                StringModifier,     "early_hit_window_size"),
		M_PAIR("eighthnote_hopo",                      UINT32Modifier,     "eighthnote_hopo"),
		M_PAIR("end_events",                           BooleanModifier,    "end_events"),
		M_PAIR("eof_midi_import_drum_accent_velocity", UINT16Modifier,     "eof_midi_import_drum_accent_velocity"),
		M_PAIR("eof_midi_import_drum_ghost_velocity",  UINT16Modifier,     "eof_midi_import_drum_ghost_velocity"),

		M_PAIR("five_lane_drums",                      BooleanModifier,    "five_lane_drums"),
		M_PAIR("frets",                                StringModifier,     "charter"),

		M_PAIR("genre",                                StringModifier,     "genre"),
		M_PAIR("guitar_type",                          UINT32Modifier,     "guitar_type"),

		M_PAIR("hopo_frequency",                       UINT32Modifier,     "hopo_frequency"),

		M_PAIR("icon",                                 StringModifier,     "icon"),

		M_PAIR("keys_type",                            UINT32Modifier,     "keys_type"),
		M_PAIR("kit_type",                             UINT32Modifier,     "kit_type"),

		M_PAIR("link_name_a",                          StringModifier,     "link_name_a"),
		M_PAIR("link_name_b",                          StringModifier,     "link_name_b"),
		M_PAIR("loading_phrase",                       StringModifier,     "loading_phrase"),
		M_PAIR("lyrics",                               BooleanModifier,    "lyrics"),

		M_PAIR("modchart",                             BooleanModifier,    "modchart"),
		M_PAIR("multiplier_note",                      UINT16Modifier,     "star_power_note"),

		M_PAIR("name",                                 StringModifier,     "name"),

		M_PAIR("playlist",                             StringModifier,     "playlist"),
		M_PAIR("playlist_track",                       UINT32Modifier,     "playlist_track"),
		M_PAIR("preview",                              FloatArrayModifier, "preview"),
		M_PAIR("preview_end_time",                     FloatModifier,      "preview_end_time"),
		M_PAIR("preview_start_time",                   FloatModifier,      "preview_start_time"),

		M_PAIR("pro_drum",                             BooleanModifier,    "pro_drums"),
		M_PAIR("pro_drums",                            BooleanModifier,    "pro_drums"),

		M_PAIR("rating",                               UINT32Modifier,     "rating"),
		M_PAIR("real_bass_22_tuning",                  UINT32Modifier,     "real_bass_22_tuning"),
		M_PAIR("real_bass_tuning",                     UINT32Modifier,     "real_bass_tuning"),
		M_PAIR("real_guitar_22_tuning",                UINT32Modifier,     "real_guitar_22_tuning"),
		M_PAIR("real_guitar_tuning",                   UINT32Modifier,     "real_guitar_tuning"),
		M_PAIR("real_keys_lane_count_left",            UINT32Modifier,     "real_keys_lane_count_left"),
		M_PAIR("real_keys_lane_count_right",           UINT32Modifier,     "real_keys_lane_count_right"),

		M_PAIR("scores",                               StringModifier,     "scores"),
		M_PAIR("scores_ext",                           StringModifier,     "scores_ext"),
		M_PAIR("song_length",                          UINT32Modifier,     "song_length"),
		M_PAIR("star_power_note",                      UINT16Modifier,     "star_power_note"),
		M_PAIR("sub_genre",                            StringModifier,     "sub_genre"),
		M_PAIR("sub_playlist",                         StringModifier,     "sub_playlist"),
		M_PAIR("sustain_cutoff_threshold",             UINT32Modifier,     "sustain_cutoff_threshold"),
		M_PAIR("sysex_high_hat_ctrl",                  BooleanModifier,    "sysex_high_hat_ctrl"),
		M_PAIR("sysex_open_bass",                      BooleanModifier,    "sysex_open_bass"),
		M_PAIR("sysex_pro_slide",                      BooleanModifier,    "sysex_pro_slide"),
		M_PAIR("sysex_rimshot",                        BooleanModifier,    "sysex_rimshot"),
		M_PAIR("sysex_slider",                         BooleanModifier,    "sysex_slider"),

		M_PAIR("tags",                                 StringModifier,     "tags"),
		M_PAIR("track",                                UINT32Modifier,     "album_track"),
		M_PAIR("tutorial",                             BooleanModifier,    "tutorial"),

		M_PAIR("unlock_completed",                     StringModifier,     "unlock_completed"),
		M_PAIR("unlock_id",                            StringModifier,     "unlock_id"),
		M_PAIR("unlock_require",                       StringModifier,     "unlock_require"),
		M_PAIR("unlock_text",                          StringModifier,     "unlock_text"),

		M_PAIR("version",                              UINT32Modifier,     "version"),
		M_PAIR("video",                                StringModifier,     "video"),
		M_PAIR("video_end_time",                       UINT32Modifier,     "video_end_time"),
		M_PAIR("video_loop",                           BooleanModifier,    "video_loop"),
		M_PAIR("video_start_time",                     UINT32Modifier,     "video_start_time"),
		M_PAIR("vocal_gender",                         UINT32Modifier,     "vocal_gender"),

		M_PAIR("year",                                 StringModifier,     "year"),
	#undef M_PAIR
};

void IniFile::setBaseModifiers()
{
	if (m_artist = getModifier<StringModifier>("artist"); !m_artist)
		m_artist = static_cast<StringModifier*>(m_modifiers.emplace_back(std::make_unique<StringModifier>(s_DEFAULT_ARTIST)).get());
	
	if (m_name = getModifier<StringModifier>("name"); !m_name)
		m_name = static_cast<StringModifier*>(m_modifiers.emplace_back(std::make_unique<StringModifier>(s_DEFAULT_NAME)).get());

	if (m_album = getModifier<StringModifier>("album"); !m_album)
		m_album = static_cast<StringModifier*>(m_modifiers.emplace_back(std::make_unique<StringModifier>(s_DEFAULT_ALBUM)).get());

	if (m_genre = getModifier<StringModifier>("genre"); !m_genre)
		m_genre = static_cast<StringModifier*>(m_modifiers.emplace_back(std::make_unique<StringModifier>(s_DEFAULT_GENRE)).get());

	if (m_year = getModifier<StringModifier>("year"); !m_year)
		m_year = static_cast<StringModifier*>(m_modifiers.emplace_back(std::make_unique<StringModifier>(s_DEFAULT_YEAR)).get());

	if (m_charter = getModifier<StringModifier>("charter"); !m_charter)
		m_charter = static_cast<StringModifier*>(m_modifiers.emplace_back(std::make_unique<StringModifier>(s_DEFAULT_CHARTER)).get());

	if (m_song_length = getModifier<UINT32Modifier>("song_length"); !m_song_length)
		m_song_length = static_cast<UINT32Modifier*>(m_modifiers.emplace_back(std::make_unique<UINT32Modifier>(s_DEFAULT_SONG_LENGTH)).get());
}

void IniFile::removeAllOf(const std::string_view modifierName)
{
	for (auto iter = begin(m_modifiers); iter != end(m_modifiers);)
		if ((*iter)->getName() == modifierName)
			m_modifiers.erase(iter++);
		else
			++iter;
}

void IniFile::removeModifier(TxtFileModifier*& modifier)
{
	for (auto iter = begin(m_modifiers); iter != end(m_modifiers); ++iter)
		if (iter->get() == modifier)
		{
			m_modifiers.erase(iter++);
			modifier = nullptr;
			return;
		}
}

void IniFile::load(std::filesystem::path filepath)
{
	filepath /= U"song.ini";

	try
	{
		FilePointers fileData(filepath);
		TextTraversal traversal(fileData);

		while (traversal && traversal != '[')
			traversal.next();

		if (!traversal)
			return;

		if (traversal.getLowercaseTrackName() == "[song]")
		{
			while (traversal.next())
				m_modifiers.push_back(traversal.extractModifier(PREDEFINED_MODIFIERS));

			if (auto* year = getModifier<StringModifier>("year");
				year && year->m_string[0] == ',')
			{
				auto iter = year->m_string->begin() + 1;
				while (iter != year->m_string->end() && *iter == ' ')
					++iter;
				year->m_string->erase(year->m_string->begin(), iter);
			}

			setBaseModifiers();
			m_isLoaded = true;
		}
	}
	catch (...)
	{
	}
}

bool IniFile::save(std::filesystem::path filepath)
{
	// Starts with the parent directory
	filepath /= U"song.ini";

	if (std::filesystem::exists(filepath))
	{
		std::filesystem::path copy = filepath;
		copy.replace_extension("ini.backup");
		std::filesystem::rename(filepath, copy);
	}

	std::fstream outFile = FilestreamCheck::getFileStream(filepath, std::ios_base::out | std::ios_base::trunc);
	outFile << "[Song]\n";
	for (const auto& modifier : m_modifiers)
		modifier->write_ini(outFile);
	outFile.close();
	return true;
}
