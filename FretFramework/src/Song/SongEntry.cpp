#include "SongEntry.h"

SongEntry::SongEntry(const std::filesystem::path& filepath) : SongEntry()
{
	setFullPath(filepath);
}

constexpr void SongEntry::setFullPath(const std::filesystem::path& path)
{
	m_fullPath = path;
	m_directory = m_fullPath.parent_path();
	m_directory_as_playlist = m_directory.parent_path().u32string();
	m_chartFile = m_fullPath.filename();
}

void SongEntry::setDirectory(const std::filesystem::path& directory)
{
	m_directory = directory;
	m_directory_as_playlist = m_directory.parent_path().u32string();
	m_fullPath = m_directory;
	m_fullPath /= m_chartFile;
}

void SongEntry::setChartFile(const char32_t* filename)
{
	m_chartFile = filename;
	m_fullPath.replace_filename(m_chartFile);
}

bool SongEntry::scan(bool iniLocated, bool iniRequired)
{
	try
	{
		if (iniLocated)
			load_Ini();

		if (!m_hasIniFile && iniRequired)
			return false;

		const FilePointers file(m_fullPath);
		const auto ext = m_chartFile.extension();
		if (ext == U".cht" || ext == U".chart")
			scanFile(TextTraversal(file));
		else if (ext == U".mid" || ext == U"midi")
			scanFile(MidiTraversal(file));
		else
			scanFile(BCHTraversal(file));

		if (!validate())
			return false;

		m_hash.computeHash(file);
	}
	catch (std::runtime_error err)
	{
		//std::wcout << m_filepath << ": " << err.what() << '\n';
		return false;
	}

	return true;
}

void SongEntry::finalizeScan()
{
	if (!m_hasIniFile)
	{
		if (m_noteTrackScans.drums4_pro.getValue())
		{
			setModifier("pro_drums", true);
			if (!m_noteTrackScans.drums5.getValue())
				setModifier("five_lane_drums", false);
		}
		else if (m_noteTrackScans.drums5.getValue())
			setModifier("five_lane_drums", true);
		save_Ini();
		m_hasIniFile = true;
	}

	m_last_modified = std::filesystem::last_write_time(m_fullPath);
	if (getSongLength() == 0)
	{
		std::vector<std::filesystem::path> audioFiles;
		for (const auto& file : std::filesystem::directory_iterator(m_directory))
		{
			if (file.is_regular_file())
			{
				const std::u32string extension = file.path().extension().u32string();
				if (extension == U".ogg" || extension == U".opus" || extension == U".mp3" || extension == U".wav" || extension == U".flac")
				{
					const std::u32string filename = file.path().stem().u32string();
					if (filename == U"song" ||
						filename == U"guitar" ||
						filename == U"bass" ||
						filename == U"rhythm" ||
						filename == U"keys" ||
						filename == U"vocals_1" || filename == U"vocals_2" ||
						filename == U"drums_1" || filename == U"drums_2" || filename == U"drums_3" || filename == U"drums_4")
					{
						// Placeholder for when audio files can be read to retrieve the length
					}
				}
			}
		}
	}
}

constexpr bool SongEntry::validate()
{
	for (int i = 0; i < 11; ++i)
		if (m_noteTrackScans.scanArray[i]->getValue() > 0)
			return true;
	return false;
}

void SongEntry::displayScanResult() const
{
	for (size_t i = 0; i < 11; ++i)
		if (m_noteTrackScans.scanArray[i]->getValue() > 0)
			std::cout << s_NOTETRACKNAMES[i] << ": " << m_noteTrackScans.scanArray[i]->toString() << std::endl;

	m_hash.display();
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

SongEntry::Scans::Scans(Scans&& other) noexcept
	: lead_5(std::move(other.lead_5))
	, lead_6(std::move(other.lead_6))
	, bass_5(std::move(other.bass_5))
	, bass_6(std::move(other.bass_6))
	, rhythm(std::move(other.rhythm))
	, coop(std::move(other.coop))
	, keys(std::move(other.keys))
	, drums4_pro(std::move(other.drums4_pro))
	, drums5(std::move(other.drums5))
	, vocals(std::move(other.vocals))
	, harmonies(std::move(other.harmonies)) {}

SongEntry::Scans& SongEntry::Scans::operator=(Scans&& other) noexcept
{
	lead_5 =     std::move(other.lead_5);
	lead_6 =     std::move(other.lead_6);
	bass_5 =     std::move(other.bass_5);
	bass_6 =     std::move(other.bass_6);
	rhythm =     std::move(other.rhythm);
	coop =       std::move(other.coop);
	keys =       std::move(other.keys);
	drums4_pro = std::move(other.drums4_pro);
	drums5 =     std::move(other.drums5);
	vocals =     std::move(other.vocals);
	harmonies =  std::move(other.harmonies);
	return *this;
}
