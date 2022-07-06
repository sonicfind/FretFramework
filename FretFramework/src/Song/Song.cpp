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
NoteTrack* const Song::s_noteTracks[11] =
{
	new InstrumentalTrack<GuitarNote<5>>("[LeadGuitar]", 0),
	new InstrumentalTrack<GuitarNote<6>>("[LeadGuitar_GHL]", 1),
	new InstrumentalTrack<GuitarNote<5>>("[BassGuitar]", 2),
	new InstrumentalTrack<GuitarNote<6>>("[BassGuitar_GHL]", 3),
	new InstrumentalTrack<GuitarNote<5>>("[RhythmGuitar]", 4),
	new InstrumentalTrack<GuitarNote<5>>("[CoopGuitar]", 5),
	new InstrumentalTrack<Keys<5>>("[Keys]", 6),
	new InstrumentalTrack<DrumNote<4, DrumPad_Pro>>("[Drums_4Lane]", 7),
	new InstrumentalTrack<DrumNote<5, DrumPad>>("[Drums_5Lane]", 8),
	new VocalTrack<1>("[Vocals]", 9),
	new VocalTrack<3>("[Harmonies]", 10),
};

FileHasher Song::s_fileHasher;

Song::Song()
	: m_hash(std::make_shared<MD5>()) {}

Song::Song(const std::filesystem::path& filepath)
	: Song()
{
	setFullPath(filepath);
}

void Song::wait()
{
	m_hash->wait();
}

void Song::setFullPath(const std::filesystem::path& path)
{
	m_fullPath = path;
	m_directory = path.parent_path();
	m_directory_playlist = m_directory.parent_path().u32string();
	m_chartFile = path.filename();
}

void Song::setDirectory(const std::filesystem::path& directory)
{
	m_directory = directory;
	m_directory_playlist = m_directory.parent_path().u32string();
	m_fullPath = directory;
	m_fullPath /= m_chartFile;
}

void Song::setChartFile(const char32_t* filename)
{
	m_chartFile = filename;
	m_fullPath.replace_filename(filename);
}

void Song::clearTracks()
{
	for (NoteTrack* track : s_noteTracks)
		track->clear();
}

void Song::deleteTracks()
{
	for (NoteTrack* track : s_noteTracks)
		delete track;
}

bool Song::operator==(const Song& other) const
{
	return *m_hash == *other.m_hash;
}

bool Song::operator<(const Song& other) const
{
	if (s_currentAttribute == SongAttribute::MD5_HASH)
		return *m_hash < *other.m_hash;

	if (s_currentAttribute == SongAttribute::ALBUM)
	{
		if (m_ini.m_album_track < other.m_ini.m_album_track)
			return true;
		else if (m_ini.m_album_track > other.m_ini.m_album_track)
			return false;
	}
	else if (s_currentAttribute == SongAttribute::PLAYLIST)
	{
		const UnicodeString& str1 = getAttribute();
		const UnicodeString& str2 = other.getAttribute();
		if (str1 < str2)
			return true;
		else if (str1 > str2)
			return false;
	}

	if (m_ini.m_name < other.m_ini.m_name)
		return true;
	else if (m_ini.m_name > other.m_ini.m_name)
		return false;

	if (m_ini.m_artist < other.m_ini.m_artist)
		return true;
	else if (m_ini.m_artist > other.m_ini.m_artist)
		return false;

	if (m_ini.m_album < other.m_ini.m_album)
		return true;
	else if (m_ini.m_album > other.m_ini.m_album)
		return false;

	if (m_ini.m_charter < other.m_ini.m_charter)
		return true;
	else if (m_ini.m_charter > other.m_ini.m_charter)
		return false;

	return m_directory < other.m_directory;
}
