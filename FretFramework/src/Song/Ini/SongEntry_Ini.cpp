#include "Song/SongEntry.h"
#include "Song/Modifiers/ModifierNode.h"
#include "FileChecks/FilestreamCheck.h"

const UnicodeString SongEntry::s_DEFAULT_NAME{ U"Unknown Title" };
const UnicodeString SongEntry::s_DEFAULT_ARTIST{ U"Unknown Artist" };
const UnicodeString SongEntry::s_DEFAULT_ALBUM{ U"Unknown Album" };
const UnicodeString SongEntry::s_DEFAULT_GENRE{ U"Unknown Genre" };
const UnicodeString SongEntry::s_DEFAULT_YEAR{ U"Unknown Year" };
const UnicodeString SongEntry::s_DEFAULT_CHARTER{ U"Unknown Charter" };

static std::pair<std::string_view, ModifierNode> constexpr PREDEFINED_MODIFIERS[]
{
	{ "album",                                { "album",   ModifierNode::STRING } },
	{ "album_track",                          { "album_track",   ModifierNode::UINT16} },
	{ "artist",                               { "artist",   ModifierNode::STRING } },

	{ "background",                           { "background",   ModifierNode::STRING_NOCASE } },
	{ "banner_link_a",                        { "banner_link_a",   ModifierNode::STRING_NOCASE } },
	{ "banner_link_b",                        { "banner_link_b",   ModifierNode::STRING_NOCASE } },
	{ "bass_type",                            { "bass_type",   ModifierNode::UINT32} },
	{ "boss_battle",                          { "boss_battle",   ModifierNode::BOOL} },

	{ "cassettecolor",                        { "cassettecolor",   ModifierNode::UINT32} },
	{ "charter",                              { "charter",   ModifierNode::STRING } },
	{ "count",                                { "count",   ModifierNode::UINT32} },
	{ "cover",                                { "cover",   ModifierNode::STRING_NOCASE } },

	{ "dance_type",                           { "dance_type",   ModifierNode::UINT32} },
	{ "delay",                                { "delay",   ModifierNode::FLOAT} },
	{ "diff_band",                            { "diff_band",   ModifierNode::INT32} },
	{ "diff_bass",                            { "diff_bass",   ModifierNode::INT32} },
	{ "diff_bass_real",                       { "diff_bass_real",   ModifierNode::INT32} },
	{ "diff_bass_real_22",                    { "diff_bass_real_22",   ModifierNode::INT32} },
	{ "diff_bassghl",                         { "diff_bassghl",   ModifierNode::INT32} },
	{ "diff_dance",                           { "diff_dance",   ModifierNode::INT32} },
	{ "diff_drums",                           { "diff_drums",   ModifierNode::INT32} },
	{ "diff_drums_real",                      { "diff_drums_real",   ModifierNode::INT32} },
	{ "diff_drums_real_ps",                   { "diff_drums_real_ps",   ModifierNode::INT32} },
	{ "diff_guitar",                          { "diff_guitar",   ModifierNode::INT32} },
	{ "diff_guitar_coop",                     { "diff_guitar_coop",   ModifierNode::INT32} },
	{ "diff_guitar_real",                     { "diff_guitar_real",   ModifierNode::INT32} },
	{ "diff_guitar_real_22",                  { "diff_guitar_real_22",   ModifierNode::INT32} },
	{ "diff_guitarghl",                       { "diff_guitarghl",   ModifierNode::INT32} },
	{ "diff_keys",                            { "diff_keys",   ModifierNode::INT32} },
	{ "diff_keys_real",                       { "diff_keys_real",   ModifierNode::INT32} },
	{ "diff_keys_real_ps",                    { "diff_keys_real_ps",   ModifierNode::INT32} },
	{ "diff_rhythm",                          { "diff_rhythm",   ModifierNode::INT32} },
	{ "diff_vocals",                          { "diff_vocals",   ModifierNode::INT32} },
	{ "diff_vocals_harm",                     { "diff_vocals_harm",   ModifierNode::INT32} },
	{ "drum_fallback_blue",                   { "drum_fallback_blue",   ModifierNode::BOOL} },

	{ "early_hit_window_size",                { "early_hit_window_size",   ModifierNode::STRING_NOCASE } },
	{ "eighthnote_hopo",                      { "eighthnote_hopo",   ModifierNode::UINT32} },
	{ "end_events",                           { "end_events",   ModifierNode::BOOL} },
	{ "eof_midi_import_drum_accent_velocity", { "eof_midi_import_drum_accent_velocity",  ModifierNode::UINT16} },
	{ "eof_midi_import_drum_ghost_velocity",  { "eof_midi_import_drum_ghost_velocity",  ModifierNode::UINT16} },

	{ "five_lane_drums",                      { "five_lane_drums",   ModifierNode::BOOL} },
	{ "frets",                                { "charter",   ModifierNode::STRING } },

	{ "genre",                                { "genre",   ModifierNode::STRING } },
	{ "guitar_type",                          { "guitar_type",   ModifierNode::UINT32} },

	{ "hopo_frequency",                       { "hopo_frequency",   ModifierNode::UINT32} },

	{ "icon",                                 { "icon",   ModifierNode::STRING } },

	{ "keys_type",                            { "keys_type",   ModifierNode::UINT32} },
	{ "kit_type",                             { "kit_type",   ModifierNode::UINT32} },

	{ "link_name_a",                          { "link_name_a",   ModifierNode::STRING_NOCASE } },
	{ "link_name_b",                          { "link_name_b",   ModifierNode::STRING_NOCASE } },
	{ "loading_phrase",                       { "loading_phrase",   ModifierNode::STRING_NOCASE } },
	{ "lyrics",                               { "lyrics",   ModifierNode::BOOL} },
	
	{ "modchart",                             { "modchart",   ModifierNode::BOOL} },
	{ "multiplier_note",                      { "star_power_note",   ModifierNode::UINT16} },

	{ "name",                                 { "name",   ModifierNode::STRING } },

	{ "playlist",                             { "playlist",   ModifierNode::STRING } },
	{ "playlist_track",                       { "playlist_track",   ModifierNode::UINT16} },
	{ "preview",                              { "preview",   ModifierNode::FLOATARRAY } },
	{ "preview_end_time",                     { "preview_end_time",   ModifierNode::FLOAT} },
	{ "preview_start_time",                   { "preview_start_time",   ModifierNode::FLOAT} },

	{ "pro_drum",                             { "pro_drums",   ModifierNode::BOOL} },
	{ "pro_drums",                            { "pro_drums",   ModifierNode::BOOL} },

	{ "rating",                               { "rating",   ModifierNode::UINT32} },
	{ "real_bass_22_tuning",                  { "real_bass_22_tuning",   ModifierNode::UINT32} },
	{ "real_bass_tuning",                     { "real_bass_tuning",   ModifierNode::UINT32} },
	{ "real_guitar_22_tuning",                { "real_guitar_22_tuning",   ModifierNode::UINT32} },
	{ "real_guitar_tuning",                   { "real_guitar_tuning",   ModifierNode::UINT32} },
	{ "real_keys_lane_count_left",            { "real_keys_lane_count_left",   ModifierNode::UINT32} },
	{ "real_keys_lane_count_right",           { "real_keys_lane_count_right",   ModifierNode::UINT32} },

	{ "scores",                               { "scores",   ModifierNode::STRING_NOCASE } },
	{ "scores_ext",                           { "scores_ext",   ModifierNode::STRING_NOCASE } },
	{ "song_length",                          { "song_length",   ModifierNode::UINT32} },
	{ "star_power_note",                      { "star_power_note",   ModifierNode::UINT16} },
	{ "sub_genre",                            { "sub_genre",   ModifierNode::STRING } },
	{ "sub_playlist",                         { "sub_playlist",   ModifierNode::STRING } },
	{ "sustain_cutoff_threshold",             { "sustain_cutoff_threshold",   ModifierNode::UINT32} },
	{ "sysex_high_hat_ctrl",                  { "sysex_high_hat_ctrl",   ModifierNode::BOOL} },
	{ "sysex_open_bass",                      { "sysex_open_bass",   ModifierNode::BOOL} },
	{ "sysex_pro_slide",                      { "sysex_pro_slide",   ModifierNode::BOOL} },
	{ "sysex_rimshot",                        { "sysex_rimshot",   ModifierNode::BOOL} },
	{ "sysex_slider",                         { "sysex_slider",   ModifierNode::BOOL} },

	{ "tags",                                 { "tags",   ModifierNode::STRING_NOCASE } },
	{ "track",                                { "album_track",   ModifierNode::UINT16} },
	{ "tutorial",                             { "tutorial",   ModifierNode::BOOL} },

	{ "unlock_completed",                     { "unlock_completed",   ModifierNode::UINT32 } },
	{ "unlock_id",                            { "unlock_id",   ModifierNode::STRING_NOCASE } },
	{ "unlock_require",                       { "unlock_require",   ModifierNode::STRING_NOCASE } },
	{ "unlock_text",                          { "unlock_text",   ModifierNode::STRING_NOCASE } },

	{ "version",                              { "version",   ModifierNode::UINT32} },
	{ "video",                                { "video",   ModifierNode::STRING_NOCASE } },
	{ "video_end_time",                       { "video_end_time",   ModifierNode::FLOAT} },
	{ "video_loop",                           { "video_loop",   ModifierNode::BOOL} },
	{ "video_start_time",                     { "video_start_time",   ModifierNode::FLOAT} },
	{ "vocal_gender",                         { "vocal_gender",   ModifierNode::UINT32} },

	{ "year",                                 { "year",   ModifierNode::STRING } },
};

void SongEntry::setBaseModifiers()
{
	if (auto previewStart = getModifier("preview_start_time"))
		if (auto previewEnd = getModifier("preview_end_time"))
			if (previewEnd->getValue<float>() <= previewStart->getValue<float>())
			{
				removeModifier("preview_end_time");
				m_writeIniAfterScan = true;
			}

	if (auto artist = getModifier("artist"))
		m_artist = &artist->getValue<UnicodeString>();

	if (auto name = getModifier("name"))
		m_name = &name->getValue<UnicodeString>();

	if (auto album = getModifier("album"))
		m_album = &album->getValue<UnicodeString>();

	if (auto genre = getModifier("genre"))
		m_genre = &genre->getValue<UnicodeString>();

	if (auto year = getModifier("year"))
	{
		UnicodeString& str = year->getValue<UnicodeString>();
		if (str[0] == ',')
		{
			auto iter = str->begin() + 1;
			while (iter != str->end() && *iter == ' ')
				++iter;
			str->erase(str->begin(), iter);
		}
		m_year = &str;
	}

	if (auto charter = getModifier("charter"))
		m_charter = &charter->getValue<UnicodeString>();

	if (auto songLength = getModifier("song_length"))
		m_song_length = &songLength->getValue<uint32_t>();
}

void SongEntry::removeModifier(const std::string_view modifierName)
{
	for (auto iter = begin(m_modifiers); iter != end(m_modifiers); ++iter)
		if (iter->getName() == modifierName)
		{
			m_modifiers.erase(iter);
			return;
		}
}

void SongEntry::removeModifier_if(const std::string_view modifierName, bool(*func)(const TxtFileModifier&))
{
	for (auto iter = begin(m_modifiers); iter != end(m_modifiers); ++iter)
		if (iter->getName() == modifierName)
		{
			if (func(*iter))
				m_modifiers.erase(iter);
			return;
		}
}

void SongEntry::load_Ini()
{
	std::filesystem::path filepath = m_directory;
	filepath /= U"song.ini";

	try
	{
		FilePointers fileData(filepath);
		TextTraversal traversal(fileData);

		while (traversal && traversal != '[')
			traversal.next();

		traversal.setTrackName();

		if (traversal && traversal.getLowercaseTrackName() == "[song]")
		{
			size_t modifierCount = 0;
			{
				TextTraversal counter = traversal;
				while (counter.next())
					++modifierCount;
			}

			m_modifiers.reserve(modifierCount);
			while (traversal.next())
			{
				auto node = ModifierNode::testForModifierName(PREDEFINED_MODIFIERS, traversal.extractModifierName());
				if (node)
				{
					TxtFileModifier* modifier = getModifier(node->m_name);
					try
					{
						if (!modifier)
							m_modifiers.emplace_back(node->createModifier(traversal));
						else if (testifModifierIsDefault(*modifier))
						{
							*modifier = node->createModifier(traversal);
							if (!testifModifierIsDefault(*modifier))
								m_writeIniAfterScan = true;
						}
					}
					catch (...)
					{
						m_writeIniAfterScan = true;
					}
				}
			}

			m_hasIniFile = true;
			m_iniModifiedTime = std::filesystem::last_write_time(filepath);
		}
	}
	catch (...)
	{
	}
}

std::filesystem::file_time_type SongEntry::save_Ini() const
{
	std::filesystem::path filepath = m_directory;
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

	if (m_artist == &s_DEFAULT_ARTIST)
		outFile << "artist" << " = " << s_DEFAULT_ARTIST << '\n';

	if (m_name == &s_DEFAULT_NAME)
		outFile << "name" << " = " << s_DEFAULT_NAME << '\n';

	if (m_album == &s_DEFAULT_ALBUM)
		outFile << "album" << " = " << s_DEFAULT_ALBUM << '\n';

	if (m_genre == &s_DEFAULT_GENRE)
		outFile << "genre" << " = " << s_DEFAULT_GENRE << '\n';

	if (m_year == &s_DEFAULT_YEAR)
		outFile << "year" << " = " << s_DEFAULT_YEAR << '\n';

	if (m_charter == &s_DEFAULT_CHARTER)
		outFile << "charter" << " = " << s_DEFAULT_CHARTER << '\n';
	
	for (const auto& modifier : m_modifiers)
		if (modifier.getName()[0] >= 97)
			modifier.write_ini(outFile);

	if (m_song_length == &s_DEFAULT_SONG_LENGTH)
		outFile << "song_length" << " = " << s_DEFAULT_SONG_LENGTH << '\n';

	outFile.close();
	return std::filesystem::last_write_time(filepath);
}
