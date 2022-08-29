#include "IniFile.h"
#include "FileChecks/FilestreamCheck.h"

constexpr std::pair<std::string_view, size_t> MODIFIERMAP[]
{
	{"album",                                offsetof(IniFile, m_album) },
	{"album_track",                          offsetof(IniFile, m_album_track) },
	{"artist",                               offsetof(IniFile, m_artist) },
					                         
	{"background",                           offsetof(IniFile, m_background) },
	{"banner_link_a",                        offsetof(IniFile, m_banner_link_a) },
	{"banner_link_b",                        offsetof(IniFile, m_banner_link_b) },
	{"bass_type",                            offsetof(IniFile, m_bass_type) },
	{"boss_battle",                          offsetof(IniFile, m_boss_battle) },
					                         
	{"cassettecolor",                        offsetof(IniFile, m_cassettecolor) },
	{"charter",                              offsetof(IniFile, m_charter) },
	{"count",                                offsetof(IniFile, m_count) },
	{"cover",                                offsetof(IniFile, m_cover) },
					                         
	{"dance_type",                           offsetof(IniFile, m_dance_type) },
	{"delay",                                offsetof(IniFile, m_delay) },
	{"diff_band",                            offsetof(IniFile, m_diff_band) },
	{"diff_bass",                            offsetof(IniFile, m_diff_bass) },
	{"diff_bass_real",                       offsetof(IniFile, m_diff_bass_real) },
	{"diff_bass_real_22",                    offsetof(IniFile, m_diff_bass_real_22) },
	{"diff_bassghl",                         offsetof(IniFile, m_diff_bassghl) },
	{"diff_dance",                           offsetof(IniFile, m_diff_dance) },
	{"diff_drums",                           offsetof(IniFile, m_diff_drums) },
	{"diff_drums_real",                      offsetof(IniFile, m_diff_drums_real) },
	{"diff_drums_real_22",                   offsetof(IniFile, m_diff_drums_real_22) },
	{"diff_guitar",                          offsetof(IniFile, m_diff_guitar) },
	{"diff_guitar_coop",                     offsetof(IniFile, m_diff_guitar_coop) },
	{"diff_guitar_real",                     offsetof(IniFile, m_diff_guitar_real) },
	{"diff_guitar_real_22",                  offsetof(IniFile, m_diff_guitar_real_22) },
	{"diff_guitarghl",                       offsetof(IniFile, m_diff_guitarghl) },
	{"diff_keys",                            offsetof(IniFile, m_diff_keys) },
	{"diff_keys_real",                       offsetof(IniFile, m_diff_keys_real) },
	{"diff_keys_real_ps",                    offsetof(IniFile, m_diff_keys_real_ps) },
	{"diff_rhythm",                          offsetof(IniFile, m_diff_rhythm) },
	{"diff_vocals",                          offsetof(IniFile, m_diff_vocals) },
	{"diff_vocals_harm",                     offsetof(IniFile, m_diff_vocals_harm) },
	{"drum_fallback_blue",                   offsetof(IniFile, m_drum_fallback_blue) },
							                 
	{"early_hit_window_size",                offsetof(IniFile, m_early_hit_window_size) },
	{"eighthnote_hopo",                      offsetof(IniFile, m_eighthnote_hopo) },
	{"end_events",                           offsetof(IniFile, m_end_events) },
	{"eof_midi_import_drum_accent_velocity", offsetof(IniFile, m_eof_midi_import_drum_accent_velocity) },
	{"eof_midi_import_drum_ghost_velocity",  offsetof(IniFile, m_eof_midi_import_drum_ghost_velocity) },

	{"five_lane_drums",                      offsetof(IniFile, m_five_lane_drums) },
	{"frets",                                offsetof(IniFile, m_frets) },
						                     
	{"genre",                                offsetof(IniFile, m_genre) },
	{"guitar_type",                          offsetof(IniFile, m_guitar_type) },
					                         
	{"hopo_frequency",                       offsetof(IniFile, m_hopo_frequency) },
					                         
	{"icon",                                 offsetof(IniFile, m_icon) },
				                             
	{"keys_type",                            offsetof(IniFile, m_keys_type) },
	{"kit_type",                             offsetof(IniFile, m_kit_type) },
					                         
	{"link_name_a",                          offsetof(IniFile, m_link_name_a) },
	{"link_name_b",                          offsetof(IniFile, m_link_name_b) },
	{"loading_phrase",                       offsetof(IniFile, m_loading_phrase) },
	{"lyrics",                               offsetof(IniFile, m_lyrics) },
					                         
	{"modchart",                             offsetof(IniFile, m_modchart) },
	{"multiplier_note",                      offsetof(IniFile, m_multiplier_note) },
						                     
	{"name",                                 offsetof(IniFile, m_name) },
				                             
	{"playlist",                             offsetof(IniFile, m_playlist) },
	{"playlist_track",                       offsetof(IniFile, m_playlist_track) },
	{"preview",                              offsetof(IniFile, m_preview) },
	{"preview_end_time",                     offsetof(IniFile, m_preview_end_time) },
	{"preview_start_time",                   offsetof(IniFile, m_preview_start_time) },
						                     
	{"pro_drum",                             offsetof(IniFile, m_pro_drum) },
	{"pro_drums",                            offsetof(IniFile, m_pro_drums) },
						                     
	{"rating",                               offsetof(IniFile, m_rating) },
	{"real_bass_22_tuning",                  offsetof(IniFile, m_real_bass_22_tuning) },
	{"real_bass_tuning",                     offsetof(IniFile, m_real_bass_tuning) },
	{"real_guitar_22_tuning",                offsetof(IniFile, m_real_guitar_22_tuning) },
	{"real_guitar_tuning",                   offsetof(IniFile, m_real_guitar_tuning) },
	{"real_keys_lane_count_left",            offsetof(IniFile, m_real_keys_lane_count_left) },
	{"real_keys_lane_count_right",           offsetof(IniFile, m_real_keys_lane_count_right) },
								             
	{"scores",                               offsetof(IniFile, m_scores) },
	{"scores_ext",                           offsetof(IniFile, m_scores_ext) },
	{"song_length",                          offsetof(IniFile, m_song_length) },
	{"star_power_note",                      offsetof(IniFile, m_star_power_note) },
	{"sub_genre",                            offsetof(IniFile, m_sub_genre) },
	{"sub_playlist",                         offsetof(IniFile, m_sub_playlist) },
	{"sustain_cutoff_threshold",             offsetof(IniFile, m_sustain_cutoff_threshold) },
								             
	{"sysex_high_hat_ctrl",                  offsetof(IniFile, m_sysex_high_hat_ctrl) },
	{"sysex_open_bass",                      offsetof(IniFile, m_sysex_open_bass) },
	{"sysex_pro_slide",                      offsetof(IniFile, m_sysex_pro_slide) },
	{"sysex_rimshot",                        offsetof(IniFile, m_sysex_rimshot) },
	{"sysex_slider",                         offsetof(IniFile, m_sysex_slider) },
								             
	{"tags",                                 offsetof(IniFile, m_tags) },
	{"track",                                offsetof(IniFile, m_track) },
	{"tutorial",                             offsetof(IniFile, m_tutorial) },
						                     
	{"unlock_completed",                     offsetof(IniFile, m_unlock_completed) },
	{"unlock_id",                            offsetof(IniFile, m_unlock_id) },
	{"unlock_require",                       offsetof(IniFile, m_unlock_require) },
	{"unlock_text",                          offsetof(IniFile, m_unlock_text) },
						                     
	{"version",                              offsetof(IniFile, m_version) },
	{"video",                                offsetof(IniFile, m_video) },
	{"video_end_time",                       offsetof(IniFile, m_video_end_time) },
	{"video_loop",                           offsetof(IniFile, m_video_loop) },
	{"video_start_time",                     offsetof(IniFile, m_video_start_time) },
	{"vocal_gender",                         offsetof(IniFile, m_vocal_gender) },
						                     
	{"year",                                 offsetof(IniFile, m_year) },
};

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
			{
				try
				{
					const auto name = traversal.extractModifierName();
					auto iter = std::lower_bound(std::begin(MODIFIERMAP), std::end(MODIFIERMAP), name,
						[](const std::pair<std::string_view, size_t>& pair, const std::string_view& str)
						{
							return pair.first < str;
						});

					if (iter != std::end(MODIFIERMAP) && name == iter->first)
						reinterpret_cast<TxtFileModifier*>((char*)this + iter->second)->read(traversal);
				}
				catch (...) {}
			}

			if (!m_year.m_string->empty() && m_year.m_string[0] == ',')
			{
				auto iter = m_year.m_string->begin() + 1;
				while (iter != m_year.m_string->end() && *iter == ' ')
					++iter;
				m_year.m_string->erase(m_year.m_string->begin(), iter);
			}

			if (m_charter.m_string->size())
				m_frets = m_charter;
			else if (m_frets.m_string->size())
				m_charter = m_frets;
		}

		
		m_isLoaded = true;
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
	m_name.write_ini(outFile);
	m_artist.write_ini(outFile);
	m_album.write_ini(outFile);
	m_genre.write_ini(outFile);
	m_sub_genre.write_ini(outFile);
	m_year.write_ini(outFile);

	m_charter.write_ini(outFile);
	m_frets.write_ini(outFile);
	m_icon.write_ini(outFile);

	m_pro_drums.write_ini(outFile);
	m_pro_drum.write_ini(outFile);
	m_five_lane_drums.write_ini(outFile);
	m_drum_fallback_blue.write_ini(outFile);

	m_sysex_slider.write_ini(outFile);
	m_sysex_high_hat_ctrl.write_ini(outFile);
	m_sysex_rimshot.write_ini(outFile);
	m_sysex_open_bass.write_ini(outFile);
	m_sysex_pro_slide.write_ini(outFile);

	m_sustain_cutoff_threshold.write_ini(outFile);
	m_hopo_frequency.write_ini(outFile);
	m_eighthnote_hopo.write_ini(outFile);
	m_multiplier_note.write_ini(outFile);
	m_star_power_note.write_ini(outFile);

	m_end_events.write_ini(outFile);
	m_early_hit_window_size.write_ini(outFile);

	m_eof_midi_import_drum_accent_velocity.write_ini(outFile);
	m_eof_midi_import_drum_ghost_velocity.write_ini(outFile);

	m_guitar_type.write_ini(outFile);
	m_bass_type.write_ini(outFile);
	m_kit_type.write_ini(outFile);
	m_keys_type.write_ini(outFile);
	m_dance_type.write_ini(outFile);
	m_vocal_gender.write_ini(outFile);

	m_real_guitar_tuning.write_ini(outFile);
	m_real_guitar_22_tuning.write_ini(outFile);
	m_real_bass_tuning.write_ini(outFile);
	m_real_bass_22_tuning.write_ini(outFile);

	m_real_keys_lane_count_right.write_ini(outFile);
	m_real_keys_lane_count_left.write_ini(outFile);

	m_diff_band.write_ini(outFile);
	m_diff_guitar.write_ini(outFile);
	m_diff_guitarghl.write_ini(outFile);
	m_diff_vocals.write_ini(outFile);
	m_diff_drums.write_ini(outFile);
	m_diff_bass.write_ini(outFile);
	m_diff_bassghl.write_ini(outFile);
	m_diff_keys.write_ini(outFile);
	
	m_diff_guitar_real.write_ini(outFile);
	m_diff_vocals_harm.write_ini(outFile);
	m_diff_drums_real.write_ini(outFile);
	m_diff_bass_real.write_ini(outFile);
	m_diff_keys_real.write_ini(outFile);
	m_diff_dance.write_ini(outFile);
	m_diff_guitar_coop.write_ini(outFile);
	m_diff_rhythm.write_ini(outFile);
	
	m_diff_guitar_real_22.write_ini(outFile);
	m_diff_drums_real_22.write_ini(outFile);
	m_diff_bass_real_22.write_ini(outFile);
	m_diff_keys_real_ps.write_ini(outFile);

	m_loading_phrase.write_ini(outFile);

	m_background.write_ini(outFile);
	m_video.write_ini(outFile);
	m_video_loop.write_ini(outFile);
	m_cover.write_ini(outFile);
	m_link_name_a.write_ini(outFile);
	m_banner_link_a.write_ini(outFile);
	m_link_name_b.write_ini(outFile);
	m_banner_link_b.write_ini(outFile);

	m_song_length.write_ini(outFile);
	m_preview_start_time.write_ini(outFile);
	m_preview_end_time.write_ini(outFile);
	m_preview.write_ini(outFile);
	m_video_start_time.write_ini(outFile);
	m_video_end_time.write_ini(outFile);
	m_delay.write_ini(outFile);

	m_album_track.write_ini(outFile);
	m_track.write_ini(outFile);
	m_playlist_track.write_ini(outFile);

	m_tags.write_ini(outFile);
	m_cassettecolor.write_ini(outFile);
	m_playlist.write_ini(outFile);
	m_sub_playlist.write_ini(outFile);

	m_rating.write_ini(outFile);
	m_scores.write_ini(outFile);
	m_scores_ext.write_ini(outFile);
	m_count.write_ini(outFile);

	m_version.write_ini(outFile);
	m_tutorial.write_ini(outFile);
	m_boss_battle.write_ini(outFile);

	m_unlock_id.write_ini(outFile);
	m_unlock_require.write_ini(outFile);
	m_unlock_text.write_ini(outFile);
	m_unlock_completed.write_ini(outFile);

	m_modchart.write_ini(outFile);
	m_lyrics.write_ini(outFile);
	outFile.close();
	return true;
}
