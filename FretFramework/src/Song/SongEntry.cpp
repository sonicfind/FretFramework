#include "SongEntry.h"

SongEntry::SongEntry(std::filesystem::directory_entry&& fileEntry)
	: m_fileEntry(std::move(fileEntry))
	, m_directory(m_fileEntry.path().parent_path())
	, m_directory_as_playlist(m_directory.parent_path().u32string())
	, m_chartModifiedTime(m_fileEntry.last_write_time()) {}

SongEntry::SongEntry(const std::filesystem::path& filepath) : SongEntry(std::filesystem::directory_entry(filepath)) {}

void SongEntry::setFullPath(const std::filesystem::path& path)
{
	m_fileEntry.assign(path);
	m_chartModifiedTime = m_fileEntry.last_write_time();
	m_directory = path.parent_path();
	m_directory_as_playlist = m_directory.parent_path().u32string();
}

void SongEntry::setDirectory(const std::filesystem::path& directory)
{
	m_directory = directory;
	m_directory_as_playlist = m_directory.parent_path().u32string();
}

void SongEntry::setChartFile(const char32_t* filename)
{
	m_fileEntry.replace_filename(filename);
}

bool SongEntry::scan()
{
	try
	{
		m_writeIniAfterScan = !m_hasIniFile;

		const FilePointers file(m_fileEntry);
		const auto ext = m_fileEntry.path().extension();
		if (ext == U".cht" || ext == U".chart")
			scanFile(TextTraversal(file));
		else if (ext == U".mid" || ext == U"midi")
			scanFile(MidiTraversal(file));
		else
			scanFile(BCHTraversal(file));

		if (!validateScans())
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
	setBaseModifiers();
	if (m_writeIniAfterScan)
	{
		if (m_noteTrackScans.drums4_pro.getValue())
		{
			setModifier("pro_drums", true);
			if (!m_noteTrackScans.drums5.getValue())
				setModifier("five_lane_drums", false);
		}
		else if (m_noteTrackScans.drums5.getValue())
		{
			setModifier("five_lane_drums", true);
			removeModifier("pro_drums");
		}
		m_iniModifiedTime = save_Ini();
		m_writeIniAfterScan = false;
		m_hasIniFile = true;
	}

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

constexpr bool SongEntry::validateScans()
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

bool SongEntry::checkLastModfiedDate() const
{
	try
	{
		return m_chartModifiedTime == m_fileEntry.last_write_time();
	}
	catch (...)
	{
		// File could not be opened/located
		return false;
	}
}

bool SongEntry::areHashesEqual(const SongEntry& other) const
{
	return m_hash == other.m_hash;
}

SongAttribute SongEntry::s_sortAttribute = SongAttribute::TITLE;

bool SongEntry::operator<(const SongEntry& other) const
{
	if (s_sortAttribute == SongAttribute::ALBUM || s_sortAttribute == SongAttribute::PLAYLIST)
	{
		static constexpr auto getTrackNumber = [](const SongEntry& song, const std::string_view name)
		{
			if (const TxtFileModifier* albumTrack = song.getModifier(name))
				return albumTrack->getValue<uint16_t>();
			return UINT16_MAX;
		};

		const std::string_view modifierName = s_sortAttribute == SongAttribute::ALBUM ? "album_track" : "playlist_track";
		const uint16_t thisTrackNumber = getTrackNumber(*this, modifierName);
		const uint16_t otherTrackNumber = getTrackNumber(other, modifierName);
		if (thisTrackNumber != otherTrackNumber)
			return thisTrackNumber < otherTrackNumber;
	}

	int strCmp = 0;
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
