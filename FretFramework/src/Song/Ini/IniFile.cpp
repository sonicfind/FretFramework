#include "IniFile.h"
#include "FileChecks/FilestreamCheck.h"

const StringModifier           IniFile::s_DEFAULT_NAME{ "name" , U"Unknown Title" };
const StringModifier           IniFile::s_DEFAULT_ARTIST{ "artist", U"Unknown Artist" };
const StringModifier           IniFile::s_DEFAULT_ALBUM{ "album", U"Unknown Album" };
const StringModifier           IniFile::s_DEFAULT_GENRE{ "genre", U"Unknown Genre" };
const StringModifier           IniFile::s_DEFAULT_YEAR{ "year", U"Unknown Year" };
const StringModifier           IniFile::s_DEFAULT_CHARTER{ "charter", U"Unknown Charter" };
const NumberModifier<uint32_t> IniFile::s_DEFAULT_SONG_LENGTH{ "song_length" };

std::unique_ptr<TxtFileModifier> IniFile::extractModifierFromFile(TextTraversal& _traversal)
{
	static std::pair<std::string_view, std::unique_ptr<TxtFileModifier>(*)()> constexpr PREDEFINED_MODIFIERS[]
	{
	#define M_PAIR(inputString, ModifierType, outputString)\
				 { inputString, []() -> std::unique_ptr<TxtFileModifier> { return std::make_unique<ModifierType>(outputString); } }
		M_PAIR("album",                                StringModifier,           "album"),
		M_PAIR("album_track",                          NumberModifier<uint32_t>, "album_track"),
		M_PAIR("artist",                               StringModifier,           "artist"),

		M_PAIR("background",                           StringModifier,           "background"),
		M_PAIR("banner_link_a",                        StringModifier,           "banner_link_a"),
		M_PAIR("banner_link_b",                        StringModifier,           "banner_link_b"),
		M_PAIR("bass_type",                            NumberModifier<uint32_t>, "bass_type"),
		M_PAIR("boss_battle",                          BooleanModifier,          "boss_battle"),

		M_PAIR("cassettecolor",                        NumberModifier<uint32_t>, "cassettecolor"),
		M_PAIR("charter",                              StringModifier,           "charter"),
		M_PAIR("count",                                NumberModifier<uint32_t>, "count"),
		M_PAIR("cover",                                StringModifier,           "cover"),

		M_PAIR("dance_type",                           NumberModifier<uint32_t>, "dance_type"),
		M_PAIR("delay",                                NumberModifier<float>,    "delay"),
		M_PAIR("diff_band",                            BooleanModifier,          "diff_band"),
		M_PAIR("diff_bass",                            NumberModifier<int32_t>,  "diff_bass"),
		M_PAIR("diff_bass_real",                       NumberModifier<int32_t>,  "diff_bass_real"),
		M_PAIR("diff_bass_real_22",                    NumberModifier<int32_t>,  "diff_bass_real_22"),
		M_PAIR("diff_bassghl",                         NumberModifier<int32_t>,  "diff_bassghl"),
		M_PAIR("diff_dance",                           NumberModifier<int32_t>,  "diff_dance"),
		M_PAIR("diff_drums",                           NumberModifier<int32_t>,  "diff_drums"),
		M_PAIR("diff_drums_real",                      NumberModifier<int32_t>,  "diff_drums_real"),
		M_PAIR("diff_drums_real_ps",                   NumberModifier<int32_t>,  "diff_drums_real_ps"),
		M_PAIR("diff_guitar",                          NumberModifier<int32_t>,  "diff_guitar"),
		M_PAIR("diff_guitar_coop",                     NumberModifier<int32_t>,  "diff_guitar_coop"),
		M_PAIR("diff_guitar_real",                     NumberModifier<int32_t>,  "diff_guitar_real"),
		M_PAIR("diff_guitar_real_22",                  NumberModifier<int32_t>,  "diff_guitar_real_22"),
		M_PAIR("diff_guitarghl",                       NumberModifier<int32_t>,  "diff_guitarghl"),
		M_PAIR("diff_keys",                            NumberModifier<int32_t>,  "diff_keys"),
		M_PAIR("diff_keys_real",                       NumberModifier<int32_t>,  "diff_keys_real"),
		M_PAIR("diff_keys_real_ps",                    NumberModifier<int32_t>,  "diff_keys_real_ps"),
		M_PAIR("diff_rhythm",                          NumberModifier<int32_t>,  "diff_rhythm"),
		M_PAIR("diff_vocals",                          NumberModifier<int32_t>,  "diff_vocals"),
		M_PAIR("diff_vocals_harm",                     NumberModifier<int32_t>,  "diff_vocals_harm"),
		M_PAIR("drum_fallback_blue",                   BooleanModifier,          "drum_fallback_blue"),

		M_PAIR("early_hit_window_size",                StringModifier,           "early_hit_window_size"),
		M_PAIR("eighthnote_hopo",                      NumberModifier<uint32_t>, "eighthnote_hopo"),
		M_PAIR("end_events",                           BooleanModifier,          "end_events"),
		M_PAIR("eof_midi_import_drum_accent_velocity", NumberModifier<uint16_t>, "eof_midi_import_drum_accent_velocity"),
		M_PAIR("eof_midi_import_drum_ghost_velocity",  NumberModifier<uint16_t>, "eof_midi_import_drum_ghost_velocity" ),

		M_PAIR("five_lane_drums",                      BooleanModifier,          "five_lane_drums"),
		M_PAIR("frets",                                StringModifier,           "charter"),

		M_PAIR("genre",                                StringModifier,           "genre"),
		M_PAIR("guitar_type",                          NumberModifier<uint32_t>, "guitar_type"),

		M_PAIR("hopo_frequency",                       NumberModifier<uint32_t>, "hopo_frequency"),

		M_PAIR("icon",                                 StringModifier,           "icon"),

		M_PAIR("keys_type",                            NumberModifier<uint32_t>, "keys_type"),
		M_PAIR("kit_type",                             NumberModifier<uint32_t>, "kit_type"),

		M_PAIR("link_name_a",                          StringModifier,           "link_name_a"),
		M_PAIR("link_name_b",                          StringModifier,           "link_name_b"),
		M_PAIR("loading_phrase",                       StringModifier,           "loading_phrase"),
		M_PAIR("lyrics",                               BooleanModifier,          "lyrics"),

		M_PAIR("modchart",                             BooleanModifier,          "modchart"),
		M_PAIR("multiplier_note",                      NumberModifier<uint16_t>, "multiplier_note"),

		M_PAIR("name",                                 StringModifier,           "name"),

		M_PAIR("playlist",                             StringModifier,           "playlist"),
		M_PAIR("playlist_track",                       NumberModifier<uint32_t>, "playlist_track"),
		M_PAIR("preview",                              FloatArrayModifier,       "preview"),
		M_PAIR("preview_end_time",                     NumberModifier<float>,    "preview_end_time"),
		M_PAIR("preview_start_time",                   NumberModifier<float>,    "preview_start_time"),

		M_PAIR("pro_drum",                             BooleanModifier,          "pro_drum"),
		M_PAIR("pro_drums",                            BooleanModifier,          "pro_drums"),

		M_PAIR("rating",                               NumberModifier<uint32_t>, "rating"),
		M_PAIR("real_bass_22_tuning",                  NumberModifier<uint32_t>, "real_bass_22_tuning"),
		M_PAIR("real_bass_tuning",                     NumberModifier<uint32_t>, "real_bass_tuning"),
		M_PAIR("real_guitar_22_tuning",                NumberModifier<uint32_t>, "real_guitar_22_tuning"),
		M_PAIR("real_guitar_tuning",                   NumberModifier<uint32_t>, "real_guitar_tuning"),
		M_PAIR("real_keys_lane_count_left",            NumberModifier<uint32_t>, "real_keys_lane_count_left"),
		M_PAIR("real_keys_lane_count_right",           NumberModifier<uint32_t>, "real_keys_lane_count_right"),

		M_PAIR("scores",                               StringModifier,           "scores"),
		M_PAIR("scores_ext",                           StringModifier,           "scores_ext"),
		M_PAIR("song_length",                          NumberModifier<uint32_t>, "song_length"),
		M_PAIR("star_power_note",                      NumberModifier<uint16_t>, "star_power_note"),
		M_PAIR("sub_genre",                            StringModifier,           "sub_genre"),
		M_PAIR("sub_playlist",                         StringModifier,           "sub_playlist"),
		M_PAIR("sustain_cutoff_threshold",             NumberModifier<uint32_t>, "sustain_cutoff_threshold"),
		M_PAIR("sysex_high_hat_ctrl",                  BooleanModifier,          "sysex_high_hat_ctrl"),
		M_PAIR("sysex_open_bass",                      BooleanModifier,          "sysex_open_bass"),
		M_PAIR("sysex_pro_slide",                      BooleanModifier,          "sysex_pro_slide"),
		M_PAIR("sysex_rimshot",                        BooleanModifier,          "sysex_rimshot"),
		M_PAIR("sysex_slider",                         BooleanModifier,          "sysex_slider"),

		M_PAIR("tags",                                 StringModifier,           "tags"),
		M_PAIR("track",                                NumberModifier<uint32_t>, "track"),
		M_PAIR("tutorial",                             BooleanModifier,          "tutorial"),

		M_PAIR("unlock_completed",                     StringModifier,           "unlock_completed"),
		M_PAIR("unlock_id",                            StringModifier,           "unlock_id"),
		M_PAIR("unlock_require",                       StringModifier,           "unlock_require"),
		M_PAIR("unlock_text",                          StringModifier,           "unlock_text"),

		M_PAIR("version",                              NumberModifier<uint32_t>, "version"),
		M_PAIR("video",                                StringModifier,           "video"),
		M_PAIR("video_end_time",                       NumberModifier<uint32_t>, "video_end_time"),
		M_PAIR("video_loop",                           BooleanModifier,          "video_loop"),
		M_PAIR("video_start_time",                     NumberModifier<uint32_t>, "video_start_time"),
		M_PAIR("vocal_gender",                         NumberModifier<uint32_t>, "vocal_gender"),

		M_PAIR("year",                                 StringModifier,           "year"),
	#undef M_PAIR
	};

	const auto modifierName = _traversal.extractModifierName();
	auto pairIter = std::lower_bound(std::begin(PREDEFINED_MODIFIERS), std::end(PREDEFINED_MODIFIERS), modifierName,
		[](const std::pair<std::string_view, std::unique_ptr<TxtFileModifier>(*)()>& pair, const std::string_view str)
		{
			return pair.first < str;
		});

	std::unique_ptr<TxtFileModifier> newModifier;
	if (pairIter != std::end(PREDEFINED_MODIFIERS) && modifierName == pairIter->first)
		newModifier = pairIter->second();
	else
		newModifier = std::make_unique<StringModifier>(modifierName);

	newModifier->read(_traversal);
	return newModifier;
}

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

	if (m_song_length = getModifier<NumberModifier<uint32_t>>("song_length"); !m_song_length)
		m_song_length = static_cast<NumberModifier<uint32_t>*>(m_modifiers.emplace_back(std::make_unique<NumberModifier<uint32_t>>(s_DEFAULT_SONG_LENGTH)).get());
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
				m_modifiers.push_back(extractModifierFromFile(traversal));

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
