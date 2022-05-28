#pragma once
#include "Song/Modifiers/Modifiers.h"
class IniFile
{
	bool m_isLoaded = false;
public:
	WritableModifier<std::string> m_name                                { "name" , "Unknown Title" };
	WritableModifier<std::string> m_artist                              { "artist", "Unknown Artist" };
	WritableModifier<std::string> m_album                               { "album", "Unknown Album" };
	WritableModifier<std::string> m_genre                               { "genre", "Unknown Genre" };
	WritableModifier<std::string> m_sub_genre                           { "sub_genre" };
	WritableModifier<std::string> m_year                                { "year", "Unknown Year" };
												                           			                        
	WritableModifier<std::string> m_charter                             { "charter", "Unknown Charter" };
	WritableModifier<std::string> m_frets                               { "frets" };
												                           			                        
	WritableModifier<uint32_t>    m_album_track                         { "album_track" };
	WritableModifier<uint32_t>    m_track                               { "track" };
	WritableModifier<uint32_t>    m_playlist_track                      { "playlist_track" };
													                       				                    
	WritableModifier<uint32_t>    m_song_length                         { "song_length" };
	WritableModifier<float>    m_preview_start_time                  { "preview_start_time" };
	WritableModifier<float>    m_preview_end_time                    { "preview_end_time" };
	WritableModifier<float[2]> m_preview                             { "preview" };
	WritableModifier<uint32_t>    m_video_start_time                    { "video_start_time" };
	WritableModifier<uint32_t>    m_video_end_time                      { "video_end_time" };
													                       				                    
	WritableModifier<std::string> m_tags                                { "tags" };
	WritableModifier<uint32_t>    m_cassettecolor                       { "cassettecolor" };
	BooleanModifier               m_modchart                            { "modchart" };
	BooleanModifier               m_lyrics                              { "lyrics" };
	WritableModifier<std::string> m_playlist                            { "playlist" };
	WritableModifier<std::string> m_sub_playlist                        { "sub_playlist" };
													                       				                    
	WritableModifier<std::string> m_loading_phrase                      { "loading_phrase" };
													                       				                    
	WritableModifier<int32_t>     m_diff_band                           { "diff_band", -1, INT32_MIN };
							       					                       				                    
	WritableModifier<int32_t>     m_diff_guitar                         { "diff_guitar", -1, INT32_MIN };
	WritableModifier<int32_t>     m_diff_guitarghl                      { "diff_guitarghl", -1, INT32_MIN };
	WritableModifier<int32_t>     m_diff_guitar_coop                    { "diff_guitar_coop", -1, INT32_MIN };
	WritableModifier<int32_t>     m_diff_guitar_real                    { "diff_guitar_real", -1, INT32_MIN };
	WritableModifier<int32_t>     m_diff_guitar_real_22                 { "diff_guitar_real_22", -1, INT32_MIN };
	WritableModifier<int32_t>     m_diff_rhythm                         { "diff_rhythm", -1, INT32_MIN };
							       					                       				                    
	WritableModifier<int32_t>     m_diff_bass                           { "diff_bass", -1, INT32_MIN };
	WritableModifier<int32_t>     m_diff_bassghl                        { "diff_bassghl", -1, INT32_MIN };
	WritableModifier<int32_t>     m_diff_bass_real                      { "diff_bass_real", -1, INT32_MIN };
	WritableModifier<int32_t>     m_diff_bass_real_22                   { "diff_bass_real_22", -1, INT32_MIN };
							       					                       				                    
	WritableModifier<int32_t>     m_diff_drums                          { "diff_drums", -1, INT32_MIN };
	WritableModifier<int32_t>     m_diff_drums_real                     { "diff_drums_real", -1, INT32_MIN };
	WritableModifier<int32_t>     m_diff_drums_real_22                  { "diff_drums_real_22", -1, INT32_MIN };
							       					                       				                    
	WritableModifier<int32_t>     m_diff_keys                           { "diff_keys", -1, INT32_MIN };
	WritableModifier<int32_t>     m_diff_keys_real                      { "diff_keys_real", -1, INT32_MIN };
	WritableModifier<int32_t>     m_diff_keys_real_ps                   { "diff_keys_real_ps", -1, INT32_MIN };
							       					                       				                    
	WritableModifier<int32_t>     m_diff_vocals                         { "diff_vocals", -1, INT32_MIN };
	WritableModifier<int32_t>     m_diff_vocals_harm                    { "diff_vocals_harm", -1, INT32_MIN };
							       					                       				                    
	WritableModifier<int32_t>     m_diff_dance                          { "diff_dance", -1, INT32_MIN };
							       				                           			                        
	WritableModifier<float>     m_delay                               { "delay" };
												                           			                        
	BooleanModifier               m_pro_drums                           { "pro_drums" };
	BooleanModifier               m_pro_drum                            { "pro_drum" };
	BooleanModifier               m_five_lane_drums                     { "five_lane_drums" };
	BooleanModifier               m_drum_fallback_blue                  { "drum_fallback_blue" };
															               						            
	WritableModifier<uint32_t>    m_sustain_cutoff_threshold            { "sustain_cutoff_threshold" };
	WritableModifier<uint32_t>    m_hopo_frequency                      { "hopo_frequency" };
	WritableModifier<uint32_t>    m_hopofreq                            { "hopofreq" };
	WritableModifier<uint32_t>    m_eighthnote_hopo                     { "eighthnote_hopo" };
	WritableModifier<uint16_t>    m_multiplier_note                     { "multiplier_note" };
	WritableModifier<uint16_t>    m_star_power_note                     { "star_power_note" };
															               						            
	BooleanModifier               m_end_events                          { "end_events" };
															               						            
	WritableModifier<std::string> m_early_hit_window_size               { "early_hit_window_size" };
														                   					                
	BooleanModifier               m_sysex_slider                        { "sysex_slider" };
	BooleanModifier               m_sysex_high_hat_ctrl                 { "sysex_high_hat_ctrl" };
	BooleanModifier               m_sysex_rimshot                       { "sysex_rimshot" };
	BooleanModifier               m_sysex_open_bass                     { "sysex_open_bass" };
	BooleanModifier               m_sysex_pro_slide                     { "sysex_pro_slide" };
														                   					                
	WritableModifier<uint32_t>    m_guitar_type                         { "guitar_type" };
	WritableModifier<uint32_t>    m_bass_type                           { "bass_type" };
	WritableModifier<uint32_t>    m_kit_type                            { "kit_type" };
	WritableModifier<uint32_t>    m_keys_type                           { "keys_type" };
	WritableModifier<uint32_t>    m_dance_type                          { "dance_type" };
	WritableModifier<uint32_t>    m_vocal_gender                        { "vocal_gender" };
							      						                   					                
	WritableModifier<uint32_t>    m_real_guitar_tuning                  { "real_guitar_tuning" };
	WritableModifier<uint32_t>    m_real_guitar_22_tuning               { "real_guitar_22_tuning" };
	WritableModifier<uint32_t>    m_real_bass_tuning                    { "real_bass_tuning" };
	WritableModifier<uint32_t>    m_real_bass_22_tuning                 { "real_bass_22_tuning" };
							      							               						            
	WritableModifier<uint32_t>    m_real_keys_lane_count_right          { "real_keys_lane_count_right" };
	WritableModifier<uint32_t>    m_real_keys_lane_count_left           { "real_keys_lane_count_left" };
															               						            
	WritableModifier<std::string> m_icon                                { "icon" };
	WritableModifier<std::string> m_background                          { "background" };
	WritableModifier<std::string> m_video                               { "video" };
	BooleanModifier               m_video_loop                          { "video_loop" };
	WritableModifier<std::string> m_cover                               { "cover" };
	WritableModifier<std::string> m_link_name_a                         { "link_name_a" };
	WritableModifier<std::string> m_banner_link_a                       { "banner_link_a" };
	WritableModifier<std::string> m_link_name_b                         { "link_name_b" };
	WritableModifier<std::string> m_banner_link_b                       { "banner_link_b" };
															               						            
	WritableModifier<uint32_t>    m_rating                              { "rating" };
	WritableModifier<std::string> m_scores                              { "scores" };
	WritableModifier<std::string> m_scores_ext                          { "scores_ext" };
	WritableModifier<uint32_t>    m_count                               { "count" };

	WritableModifier<uint32_t>    m_version                             { "version" };
	BooleanModifier               m_tutorial                            { "tutorial" };
	BooleanModifier               m_boss_battle                         { "boss_battle" };

	WritableModifier<std::string> m_unlock_id                           { "unlock_id" };
	WritableModifier<std::string> m_unlock_require                      { "unlock_require" };
	WritableModifier<std::string> m_unlock_text                         { "unlock_text" };
	WritableModifier<std::string> m_unlock_completed                    { "unlock_completed" };

	WritableModifier<uint16_t>    m_eof_midi_import_drum_accent_velocity{ "eof_midi_import_drum_accent_velocity" };
	WritableModifier<uint16_t>    m_eof_midi_import_drum_ghost_velocity { "eof_midi_import_drum_ghost_velocity" };

	void load(std::filesystem::path filepath);
	bool save(std::filesystem::path filepath);
	bool wasLoaded() { return m_isLoaded; }
};

