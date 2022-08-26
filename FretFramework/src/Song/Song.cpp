#include "Song.h"

// 0 -  Guitar 5
// 1 -  Guitar 6
// 2 -  Bass 5
// 3 -  Bass 6
// 4 -  Rhythm
// 5 -  Co-op
// 6 -  Keys
// 7 -  Drums 4
// 8 -  Drums 5
// 9 -  Vocals
// 10 - Harmonies
std::unique_ptr<NoteTrack> const Song::s_noteTracks[11] =
{
	std::unique_ptr<NoteTrack>(new InstrumentalTrack<GuitarNote<5>>("[LeadGuitar]", 0)),
	std::unique_ptr<NoteTrack>(new InstrumentalTrack<GuitarNote<6>>("[LeadGuitar_GHL]", 1)),
	std::unique_ptr<NoteTrack>(new InstrumentalTrack<GuitarNote<5>>("[BassGuitar]", 2)),
	std::unique_ptr<NoteTrack>(new InstrumentalTrack<GuitarNote<6>>("[BassGuitar_GHL]", 3)),
	std::unique_ptr<NoteTrack>(new InstrumentalTrack<GuitarNote<5>>("[RhythmGuitar]", 4)),
	std::unique_ptr<NoteTrack>(new InstrumentalTrack<GuitarNote<5>>("[CoopGuitar]", 5)),
	std::unique_ptr<NoteTrack>(new InstrumentalTrack<Keys<5>>("[Keys]", 6)),
	std::unique_ptr<NoteTrack>(new InstrumentalTrack<DrumNote<4, DrumPad_Pro>>("[Drums_4Lane]", 7)),
	std::unique_ptr<NoteTrack>(new InstrumentalTrack<DrumNote<5, DrumPad>>("[Drums_5Lane]", 8)),
	std::unique_ptr<NoteTrack>(new VocalTrack<1>("[Vocals]", 9)),
	std::unique_ptr<NoteTrack>(new VocalTrack<3>("[Harmonies]", 10)),
};

Song::Song(const std::filesystem::path& filepath)
{
	setFullPath(filepath);
}

constexpr void Song::setFullPath(const std::filesystem::path& path)
{
	m_fullPath = path;
	m_directory = m_fullPath.parent_path();
	m_directory_playlist = m_directory.parent_path().u32string();
	m_chartFile = m_fullPath.filename();
}

void Song::setDirectory(const std::filesystem::path& directory)
{
	m_directory = directory;
	m_directory_playlist = m_directory.parent_path().u32string();
	m_fullPath = m_directory;
	m_fullPath /= m_chartFile;
}

void Song::setChartFile(const char32_t* filename)
{
	m_chartFile = filename;
	m_fullPath.replace_filename(m_chartFile);
}

bool Song::operator==(const Song& other) const
{
	return m_hash == other.m_hash;
}

SongAttribute Song::s_sortAttribute = SongAttribute::TITLE;

bool Song::operator<(const Song& other) const
{
	if (s_sortAttribute == SongAttribute::ALBUM)
	{
		if (m_ini.m_album_track != other.m_ini.m_album_track)
			return m_ini.m_album_track < other.m_ini.m_album_track;
	}
	
	int strCmp = 0;
	if (s_sortAttribute == SongAttribute::PLAYLIST)
	{
		strCmp = getAttribute<SongAttribute::PLAYLIST>().compare(other.getAttribute<SongAttribute::PLAYLIST>());
		if (strCmp != 0)
			return strCmp < 0;
	}

	strCmp = m_ini.m_name.m_string.compare(other.m_ini.m_name.m_string);
	if (strCmp != 0)
		return strCmp < 0;

	strCmp = m_ini.m_artist.m_string.compare(other.m_ini.m_artist.m_string);
	if (strCmp != 0)
		return strCmp < 0;

	strCmp = m_ini.m_album.m_string.compare(other.m_ini.m_album.m_string);
	if (strCmp != 0)
		return strCmp < 0;

	strCmp = m_ini.m_charter.m_string.compare(other.m_ini.m_charter.m_string);
	if (strCmp == 0)
		strCmp = m_directory.compare(other.m_directory);
	return strCmp < 0;
}

bool Song::isHashLessThan(const Song& other) const
{
	return m_hash < other.m_hash;
}

const std::vector<std::pair<std::string_view, size_t>>& Song::constructSongInfoMap()
{
	static const std::vector<std::pair<std::string_view, size_t>> arr =
	{
		std::pair<std::string_view, size_t>{ "Album",        offsetof(Song, m_songInfo.album)},
		std::pair<std::string_view, size_t>{ "Artist",       offsetof(Song, m_songInfo.artist) },
		std::pair<std::string_view, size_t>{ "Charter",      offsetof(Song, m_songInfo.charter) },
		std::pair<std::string_view, size_t>{ "Difficulty",   offsetof(Song, m_songInfo.difficulty) },
		std::pair<std::string_view, size_t>{ "FileVersion",  offsetof(Song, m_version_cht) },
		std::pair<std::string_view, size_t>{ "Genre",        offsetof(Song, m_songInfo.genre) },
		std::pair<std::string_view, size_t>{ "Name",         offsetof(Song, m_songInfo.name) },
		std::pair<std::string_view, size_t>{ "Offset",       offsetof(Song, m_offset) },
		std::pair<std::string_view, size_t>{ "PreviewEnd",   offsetof(Song, m_songInfo.preview_end_time) },
		std::pair<std::string_view, size_t>{ "PreviewStart", offsetof(Song, m_songInfo.preview_start_time) },
		std::pair<std::string_view, size_t>{ "Year",         offsetof(Song, m_songInfo.year)},
	};
	return arr;
}
