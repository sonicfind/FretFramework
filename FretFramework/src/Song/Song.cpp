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

FileHasher Song::s_hashingQueue;

void Song::startHashQueue()
{
	s_hashingQueue.startThreads();
}

void Song::stopHashQueue()
{
	s_hashingQueue.stopThreads();
}

Song::Song()
	: m_hash(std::make_shared<MD5>()) {}

Song::Song(const std::filesystem::path& filepath)
	: Song()
{
	setFullPath(filepath);
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
	for (const auto& track : s_noteTracks)
		track->clear();
}

bool Song::operator==(const Song& other) const
{
	return *m_hash == *other.m_hash;
}

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

	strCmp = m_ini.m_name.m_value.compare(other.m_ini.m_name.m_value);
	if (strCmp != 0)
		return strCmp < 0;

	strCmp = m_ini.m_artist.m_value.compare(other.m_ini.m_artist.m_value);
	if (strCmp != 0)
		return strCmp < 0;

	strCmp = m_ini.m_album.m_value.compare(other.m_ini.m_album.m_value);
	if (strCmp != 0)
		return strCmp < 0;

	strCmp = m_ini.m_charter.m_value.compare(other.m_ini.m_charter.m_value);
	if (strCmp == 0)
		strCmp = m_directory.compare(other.m_directory);
	return strCmp < 0;
}

bool Song::isHashLessThan(const Song& other) const
{
	return *m_hash < *other.m_hash;
}
