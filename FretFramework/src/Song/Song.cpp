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
		static auto getAlbumTrackNumber = [](const IniFile& ini)
		{
			if (NumberModifier<uint32_t>* albumTrack = ini.getModifier<NumberModifier<uint32_t>>("album_track"))
				return albumTrack->m_value;
			return UINT32_MAX;
		};

		const uint32_t thisAlbumTrack = getAlbumTrackNumber(m_ini);
		const uint32_t otherAlbumTrack = getAlbumTrackNumber(other.m_ini);
		if (thisAlbumTrack != otherAlbumTrack)
			return thisAlbumTrack < otherAlbumTrack;
	}
	
	int strCmp = 0;
	if (s_sortAttribute == SongAttribute::PLAYLIST)
	{
		strCmp = getAttribute<SongAttribute::PLAYLIST>().compare(other.getAttribute<SongAttribute::PLAYLIST>());
		if (strCmp != 0)
			return strCmp < 0;
	}

	if ((strCmp = m_ini.getName()   ->compare(other.m_ini.getName()))    != 0 ||
		(strCmp = m_ini.getArtist() ->compare(other.m_ini.getArtist()))  != 0 ||
		(strCmp = m_ini.getAlbum()  ->compare(other.m_ini.getAlbum()))   != 0 ||
		(strCmp = m_ini.getCharter()->compare(other.m_ini.getCharter())) != 0)
		return strCmp < 0;
	else
		return m_directory < other.m_directory;
}

bool Song::isHashLessThan(const Song& other) const
{
	return m_hash < other.m_hash;
}
