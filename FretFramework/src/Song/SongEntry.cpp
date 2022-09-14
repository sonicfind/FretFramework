#include "SongEntry.h"

SongEntry::Tracks SongEntry::s_noteTracks;

SongEntry::SongEntry()
	: m_name(&s_DEFAULT_NAME)
	, m_artist(&s_DEFAULT_ARTIST)
	, m_album(&s_DEFAULT_ALBUM)
	, m_genre(&s_DEFAULT_GENRE)
	, m_year(&s_DEFAULT_YEAR)
	, m_charter(&s_DEFAULT_CHARTER)
	, m_song_length(&s_DEFAULT_SONG_LENGTH) {}


SongEntry::SongEntry(const std::filesystem::path& filepath, bool hasIni)
	: SongEntry()
{
	m_hasIniFile = hasIni;
	setFullPath(filepath);
}

constexpr void SongEntry::setFullPath(const std::filesystem::path& path)
{
	m_fullPath = path;
	m_directory = m_fullPath.parent_path();
	m_directory_playlist = m_directory.parent_path().u32string();
	m_chartFile = m_fullPath.filename();
}

void SongEntry::setDirectory(const std::filesystem::path& directory)
{
	m_directory = directory;
	m_directory_playlist = m_directory.parent_path().u32string();
	m_fullPath = m_directory;
	m_fullPath /= m_chartFile;
}

void SongEntry::setChartFile(const char32_t* filename)
{
	m_chartFile = filename;
	m_fullPath.replace_filename(m_chartFile);
}

bool SongEntry::areHashesEqual(const SongEntry& other) const
{
	return m_hash == other.m_hash;
}

SongAttribute SongEntry::s_sortAttribute = SongAttribute::TITLE;

bool SongEntry::operator<(const SongEntry& other) const
{
	if (s_sortAttribute == SongAttribute::ALBUM)
	{
		static auto getAlbumTrackNumber = [](const SongEntry& song)
		{
			if (const TxtFileModifier* albumTrack = song.getModifier("album_track"))
				return albumTrack->getValue<uint32_t>();
			return UINT32_MAX;
		};

		const uint32_t thisAlbumTrack = getAlbumTrackNumber(*this);
		const uint32_t otherAlbumTrack = getAlbumTrackNumber(other);
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

	if ((strCmp = m_name   ->compare(*other.m_name))    != 0 ||
		(strCmp = m_artist ->compare(*other.m_artist))  != 0 ||
		(strCmp = m_album  ->compare(*other.m_album))   != 0 ||
		(strCmp = m_charter->compare(*other.m_charter)) != 0)
		return strCmp < 0;
	else
		return m_directory < other.m_directory;
}

bool SongEntry::isHashLessThan(const SongEntry& other) const
{
	return m_hash < other.m_hash;
}