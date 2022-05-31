#include "IniFile.h"
#include "FileChecks/FilestreamCheck.h"
void IniFile::load(std::filesystem::path filepath)
{
	filepath.replace_filename("song.ini");

	try
	{
		// Loads the file into a char array and traverses it byte by byte
		// or by skipping to a new line character
		TextTraversal traversal(filepath);

		while (traversal && traversal != '[')
			traversal.next();

		if (!traversal)
			return;

		std::string title(traversal.getCurrent(), 6);
		for (char& c : title)
			c = std::tolower(c);

		if (title == "[song]")
		{
			while (traversal.next())
			{
				// Utilize short circuiting to stop if a read was valid
				m_name.read_ini(traversal) ||
					m_artist.read_ini(traversal) ||
					m_album.read_ini(traversal) ||
					m_genre.read_ini(traversal) ||
					m_sub_genre.read_ini(traversal) ||
					m_year.read_ini(traversal) ||

					m_charter.read_ini(traversal) ||
					m_frets.read_ini(traversal) ||
					m_icon.read_ini(traversal) ||

					m_album_track.read(traversal) ||
					m_track.read(traversal) ||
					m_playlist_track.read(traversal) ||

					m_song_length.read(traversal) ||
					m_preview_start_time.read(traversal) ||
					m_preview_end_time.read(traversal) ||
					m_preview.read(traversal) ||
					m_video_start_time.read(traversal) ||
					m_video_end_time.read(traversal) ||

					m_tags.read_ini(traversal) ||
					m_cassettecolor.read(traversal) ||
					m_modchart.read(traversal) ||
					m_lyrics.read(traversal) ||
					m_playlist.read_ini(traversal) ||
					m_sub_playlist.read_ini(traversal) ||

					m_diff_band.read(traversal) ||

					m_diff_guitar.read(traversal) ||
					m_diff_guitarghl.read(traversal) ||
					m_diff_guitar_coop.read(traversal) ||
					m_diff_guitar_real.read(traversal) ||
					m_diff_guitar_real_22.read(traversal) ||
					m_diff_rhythm.read(traversal) ||

					m_diff_bass.read(traversal) ||
					m_diff_bassghl.read(traversal) ||
					m_diff_bass_real.read(traversal) ||
					m_diff_bass_real_22.read(traversal) ||

					m_diff_drums.read(traversal) ||
					m_diff_drums_real.read(traversal) ||
					m_diff_drums_real_22.read(traversal) ||

					m_diff_keys.read(traversal) ||
					m_diff_keys_real.read(traversal) ||
					m_diff_keys_real_ps.read(traversal) ||

					m_diff_vocals.read(traversal) ||
					m_diff_vocals_harm.read(traversal) ||

					m_diff_dance.read(traversal) ||

					m_delay.read(traversal) ||
					m_loading_phrase.read_ini(traversal) ||

					m_pro_drums.read(traversal) ||
					m_pro_drum.read(traversal) ||
					m_five_lane_drums.read(traversal) ||
					m_drum_fallback_blue.read(traversal) ||

					m_sustain_cutoff_threshold.read(traversal) ||
					m_hopo_frequency.read(traversal) ||
					m_eighthnote_hopo.read(traversal) ||
					m_multiplier_note.read(traversal) ||
					m_star_power_note.read(traversal) ||

					m_end_events.read(traversal) ||

					m_early_hit_window_size.read_ini(traversal) ||

					m_sysex_slider.read(traversal) ||
					m_sysex_high_hat_ctrl.read(traversal) ||
					m_sysex_rimshot.read(traversal) ||
					m_sysex_open_bass.read(traversal) ||
					m_sysex_pro_slide.read(traversal) ||

					m_eof_midi_import_drum_accent_velocity.read(traversal) ||
					m_eof_midi_import_drum_ghost_velocity.read(traversal) ||

					m_guitar_type.read(traversal) ||
					m_bass_type.read(traversal) ||
					m_kit_type.read(traversal) ||
					m_keys_type.read(traversal) ||
					m_dance_type.read(traversal) ||
					m_vocal_gender.read(traversal) ||

					m_real_guitar_tuning.read(traversal) ||
					m_real_guitar_22_tuning.read(traversal) ||
					m_real_bass_tuning.read(traversal) ||
					m_real_bass_22_tuning.read(traversal) ||

					m_real_keys_lane_count_right.read(traversal) ||
					m_real_keys_lane_count_left.read(traversal) ||

					m_background.read_ini(traversal) ||
					m_video.read_ini(traversal) ||
					m_video_loop.read(traversal) ||
					m_cover.read_ini(traversal) ||
					m_link_name_a.read_ini(traversal) ||
					m_banner_link_a.read_ini(traversal) ||
					m_link_name_b.read_ini(traversal) ||
					m_banner_link_b.read_ini(traversal) ||

					m_rating.read(traversal) ||
					m_scores.read_ini(traversal) ||
					m_scores_ext.read_ini(traversal) ||
					m_count.read(traversal) ||

					m_version.read(traversal) ||
					m_tutorial.read(traversal) ||
					m_boss_battle.read(traversal) ||

					m_unlock_id.read_ini(traversal) ||
					m_unlock_require.read_ini(traversal) ||
					m_unlock_text.read_ini(traversal) ||
					m_unlock_completed.read_ini(traversal);
			}

			if (!m_year.m_value.empty() && m_year.m_value[0] == ',')
			{
				auto iter = m_year.m_value.begin() + 1;
				while (iter != m_year.m_value.end() && *iter == ' ')
					++iter;
				m_year.m_value.erase(m_year.m_value.begin(), iter);
			}

			if (m_charter.m_value.size())
				m_frets = m_charter;
			else if (m_frets.m_value.size())
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
	filepath.replace_filename("song.ini");

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
