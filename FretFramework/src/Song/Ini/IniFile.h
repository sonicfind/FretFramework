#pragma once
#include "Song/Modifiers/Modifiers.h"

class IniFile
{
	bool m_isLoaded = false;
public:
	StringModifier           m_name                                { "name" , U"Unknown Title" };
	StringModifier           m_artist                              { "artist", U"Unknown Artist" };
	StringModifier           m_album                               { "album", U"Unknown Album" };
	StringModifier           m_genre                               { "genre", U"Unknown Genre" };
	StringModifier           m_sub_genre                           { "sub_genre" };
	StringModifier           m_year                                { "year", U"Unknown Year" };

	StringModifier           m_charter                             { "charter", U"Unknown Charter" };
	StringModifier           m_frets                               { "frets" };

	NumberModifier<uint32_t> m_album_track                         { "album_track" };
	NumberModifier<uint32_t> m_track                               { "track" };
	NumberModifier<uint32_t> m_playlist_track                      { "playlist_track" };

	NumberModifier<uint32_t> m_song_length                         { "song_length" };
	NumberModifier<float>    m_preview_start_time                  { "preview_start_time" };
	NumberModifier<float>    m_preview_end_time                    { "preview_end_time" };
	FloatArrayModifier       m_preview                             { "preview" };
	NumberModifier<uint32_t> m_video_start_time                    { "video_start_time" };
	NumberModifier<uint32_t> m_video_end_time                      { "video_end_time" };

	StringModifier           m_tags                                { "tags" };
	NumberModifier<uint32_t> m_cassettecolor                       { "cassettecolor" };
	BooleanModifier          m_modchart                            { "modchart", false, true };
	BooleanModifier          m_lyrics                              { "lyrics", false, true };
	StringModifier           m_playlist                            { "playlist" };
	StringModifier           m_sub_playlist                        { "sub_playlist" };

	StringModifier           m_loading_phrase                      { "loading_phrase" };

	NumberModifier<int32_t>  m_diff_band                           { "diff_band", -1, INT32_MIN };
	NumberModifier<int32_t>  m_diff_guitar                         { "diff_guitar", -1, INT32_MIN };
	NumberModifier<int32_t>  m_diff_guitarghl                      { "diff_guitarghl", -1, INT32_MIN };
	NumberModifier<int32_t>  m_diff_bass                           { "diff_bass", -1, INT32_MIN };
	NumberModifier<int32_t>  m_diff_bassghl                        { "diff_bassghl", -1, INT32_MIN };

	NumberModifier<int32_t>  m_diff_rhythm                         { "diff_rhythm", -1, INT32_MIN };
	NumberModifier<int32_t>  m_diff_guitar_coop                    { "diff_guitar_coop", -1, INT32_MIN };

	NumberModifier<int32_t>  m_diff_keys    	                   { "diff_keys", -1, INT32_MIN };

	NumberModifier<int32_t>  m_diff_drums                          { "diff_drums", -1, INT32_MIN };
	NumberModifier<int32_t>  m_diff_drums_5   	                   { "diff_drums_5", -1, INT32_MIN };

	NumberModifier<int32_t>  m_diff_vocals                         { "diff_vocals", -1, INT32_MIN };
	NumberModifier<int32_t>  m_diff_vocals_harm                    { "diff_vocals_harm", -1, INT32_MIN };
	NumberModifier<int32_t>* const m_difficulties[11] = {
		&m_diff_guitar, &m_diff_guitarghl,
		&m_diff_bass, &m_diff_bassghl,
		&m_diff_rhythm,
		&m_diff_guitar_coop,
		&m_diff_keys,
		&m_diff_drums, &m_diff_drums_5,
		&m_diff_vocals, &m_diff_vocals_harm,
	};

	NumberModifier<int32_t>  m_diff_guitar_real                    { "diff_guitar_real", -1, INT32_MIN };
	NumberModifier<int32_t>  m_diff_guitar_real_22                 { "diff_guitar_real_22", -1, INT32_MIN };

	NumberModifier<int32_t>  m_diff_bass_real                      { "diff_bass_real", -1, INT32_MIN };
	NumberModifier<int32_t>  m_diff_bass_real_22                   { "diff_bass_real_22", -1, INT32_MIN };

	NumberModifier<int32_t>  m_diff_drums_real                     { "diff_drums_real", -1, INT32_MIN };
	NumberModifier<int32_t>  m_diff_drums_real_22                  { "diff_drums_real_22", -1, INT32_MIN };

	NumberModifier<int32_t>  m_diff_keys_real                      { "diff_keys_real", -1, INT32_MIN };
	NumberModifier<int32_t>  m_diff_keys_real_ps                   { "diff_keys_real_ps", -1, INT32_MIN };

	NumberModifier<int32_t>  m_diff_dance                          { "diff_dance", -1, INT32_MIN };

	NumberModifier<float>    m_delay                               { "delay" };

	BooleanModifier          m_pro_drums                           { "pro_drums" };
	BooleanModifier          m_pro_drum                            { "pro_drum" };
	BooleanModifier          m_five_lane_drums                     { "five_lane_drums" };
	BooleanModifier          m_drum_fallback_blue                  { "drum_fallback_blue" };

	NumberModifier<uint32_t> m_sustain_cutoff_threshold            { "sustain_cutoff_threshold" };
	NumberModifier<uint32_t> m_hopo_frequency                      { "hopo_frequency" };
	NumberModifier<uint32_t> m_eighthnote_hopo                     { "eighthnote_hopo" };
	NumberModifier<uint16_t> m_multiplier_note                     { "multiplier_note", 116 };
	NumberModifier<uint16_t> m_star_power_note                     { "star_power_note", 116 };

	BooleanModifier          m_end_events                          { "end_events" };

	StringModifier           m_early_hit_window_size               { "early_hit_window_size" };

	BooleanModifier          m_sysex_slider                        { "sysex_slider" };
	BooleanModifier          m_sysex_high_hat_ctrl                 { "sysex_high_hat_ctrl" };
	BooleanModifier          m_sysex_rimshot                       { "sysex_rimshot" };
	BooleanModifier          m_sysex_open_bass                     { "sysex_open_bass" };
	BooleanModifier          m_sysex_pro_slide                     { "sysex_pro_slide" };

	NumberModifier<uint32_t> m_guitar_type                         { "guitar_type" };
	NumberModifier<uint32_t> m_bass_type                           { "bass_type" };
	NumberModifier<uint32_t> m_kit_type                            { "kit_type" };
	NumberModifier<uint32_t> m_keys_type                           { "keys_type" };
	NumberModifier<uint32_t> m_dance_type                          { "dance_type" };
	NumberModifier<uint32_t> m_vocal_gender                        { "vocal_gender" };

	NumberModifier<uint32_t> m_real_guitar_tuning                  { "real_guitar_tuning" };
	NumberModifier<uint32_t> m_real_guitar_22_tuning               { "real_guitar_22_tuning" };
	NumberModifier<uint32_t> m_real_bass_tuning                    { "real_bass_tuning" };
	NumberModifier<uint32_t> m_real_bass_22_tuning                 { "real_bass_22_tuning" };

	NumberModifier<uint32_t> m_real_keys_lane_count_right          { "real_keys_lane_count_right" };
	NumberModifier<uint32_t> m_real_keys_lane_count_left           { "real_keys_lane_count_left" };

	StringModifier           m_icon                                { "icon" };
	StringModifier           m_background                          { "background" };
	StringModifier           m_video                               { "video" };
	BooleanModifier          m_video_loop                          { "video_loop" };
	StringModifier           m_cover                               { "cover" };
	StringModifier           m_link_name_a                         { "link_name_a" };
	StringModifier           m_banner_link_a                       { "banner_link_a" };
	StringModifier           m_link_name_b                         { "link_name_b" };
	StringModifier           m_banner_link_b                       { "banner_link_b" };

	NumberModifier<uint32_t> m_rating                              { "rating" };
	StringModifier           m_scores                              { "scores" };
	StringModifier           m_scores_ext                          { "scores_ext" };
	NumberModifier<uint32_t> m_count                               { "count" };

	NumberModifier<uint32_t> m_version                             { "version" };
	BooleanModifier          m_tutorial                            { "tutorial" };
	BooleanModifier          m_boss_battle                         { "boss_battle" };

	StringModifier           m_unlock_id                           { "unlock_id" };
	StringModifier           m_unlock_require                      { "unlock_require" };
	StringModifier           m_unlock_text                         { "unlock_text" };
	StringModifier           m_unlock_completed                    { "unlock_completed" };

	NumberModifier<uint16_t> m_eof_midi_import_drum_accent_velocity{ "eof_midi_import_drum_accent_velocity" };
	NumberModifier<uint16_t> m_eof_midi_import_drum_ghost_velocity { "eof_midi_import_drum_ghost_velocity" };

	void load(std::filesystem::path directory);
	bool save(std::filesystem::path directory);
	bool wasLoaded() { return m_isLoaded; }
};

