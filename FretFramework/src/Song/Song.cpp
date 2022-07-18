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

Song::Song()
	: m_hash(std::make_shared<MD5>()) {}

Song::Song(const std::filesystem::path& filepath)
	: Song()
{
	setFullPath(filepath);
}

void Song::displayScanResult() const
{
	for (size_t i = 0; i < 11; ++i)
		if (m_noteTrackScans[i] && m_noteTrackScans[i]->getValue())
		{
			std::cout << s_noteTracks[i]->m_name << ": ";
			if (i < 9)
			{
				if (m_noteTrackScans[i]->getValue() >= 8)
					std::cout << "Expert ";
				if (m_noteTrackScans[i]->getValue() & 4)
					std::cout << "Hard ";
				if (m_noteTrackScans[i]->getValue() & 2)
					std::cout << "Medium ";
				if (m_noteTrackScans[i]->getValue() & 1)
					std::cout << "Easy";
			}
			else if (i == 9)
			{
				if (m_noteTrackScans[i]->getValue() == 1)
					std::cout << "Main Vocals";
			}
			else
			{
				if (m_noteTrackScans[i]->getValue() & 1)
					std::cout << "Harm1 ";
				if (m_noteTrackScans[i]->getValue() & 2)
					std::cout << "Harm2 ";
				if (m_noteTrackScans[i]->getValue() & 4)
					std::cout << "Harm3";
			}
			std::cout << '\n';
		}
		
	m_hash->display();
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
	if (s_currentAttribute == SongAttribute::ALBUM)
	{
		if (m_ini.m_album_track != other.m_ini.m_album_track)
			return m_ini.m_album_track < other.m_ini.m_album_track;
	}
	
	int strCmp = 0;
	if (s_currentAttribute == SongAttribute::PLAYLIST)
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
