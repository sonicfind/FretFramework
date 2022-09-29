#include "SongEntry.h"

SongEntry::SongEntry(std::filesystem::directory_entry&& fileEntry, StorageDriveType type)
	: m_fileEntry(std::move(fileEntry))
	, m_directory(m_fileEntry.path().parent_path())
	, m_chartModifiedTime(m_fileEntry.last_write_time())
	, m_storageType(type) {}

SongEntry::SongEntry(const std::filesystem::path& filepath) : SongEntry(std::filesystem::directory_entry(filepath), SSD) {}

void SongEntry::setFullPath(const std::filesystem::path& path)
{
	m_fileEntry.assign(path);
	m_chartModifiedTime = m_fileEntry.last_write_time();
	m_directory = path.parent_path();
}

void SongEntry::setChartFile(const char32_t* filename)
{
	m_fileEntry.replace_filename(filename);
}

bool SongEntry::scan(int _chartNameIndex)
{
	try
	{
		m_writeIniAfterScan = !m_hasIniFile;

		const FilePointers file(m_fileEntry);
		switch (_chartNameIndex)
		{
		case 0:
			scanFile(BCHTraversal(file)); break;
		case 2:
		case 3:
			scanFile(MidiTraversal(file)); break;
		default:
			scanFile(TextTraversal(file)); break;
		}

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
		if (m_noteTrackScans.drums4_pro.m_scanValue)
		{
			setModifier("pro_drums", true);
			if (!m_noteTrackScans.drums5.m_scanValue)
				setModifier("five_lane_drums", false);
		}
		else if (m_noteTrackScans.drums5.m_scanValue)
		{
			setModifier("five_lane_drums", true);
			removeModifier("pro_drums");
		}

		try
		{
			m_iniModifiedTime = save_Ini();
			m_writeIniAfterScan = false;
			m_hasIniFile = true;
		}
		catch (...) {}
	}

	if (*m_song_length == 0)
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
		if (m_noteTrackScans.scanArray[i]->m_scanValue > 0)
			return true;
	return false;
}

void SongEntry::displayScanResult() const
{
	for (size_t i = 0; i < 11; ++i)
		if (m_noteTrackScans.scanArray[i]->m_scanValue > 0)
			std::cout << s_NOTETRACKNAMES[i] << ": " << m_noteTrackScans.scanArray[i]->toString() << std::endl;

	m_hash.display();
}

void SongEntry::writeToCache(std::fstream& outFile) const
{
	outFile.put((char)m_storageType);

	auto start = outFile.tellp();
	uint32_t length = 7 * sizeof(uint32_t);
	outFile.write((char*)&length, 4);

	UnicodeString::U32ToWebTypedFile(m_directory.u32string(), outFile);
	{
		const uint64_t iniWriteTime = m_iniModifiedTime.time_since_epoch().count();
		outFile.write((char*)&iniWriteTime, 8);
	}

	UnicodeString::U32ToWebTypedFile(m_fileEntry.path().filename().u32string(), outFile);
	{
		const uint64_t chartWriteTime = m_chartModifiedTime.time_since_epoch().count();
		outFile.write((char*)&chartWriteTime, 8);
	}

	outFile.write((char*)m_previewRange, 2 * sizeof(float));
	outFile.write((char*)&m_albumTrack, sizeof(uint16_t));
	outFile.write((char*)&m_playlistTrack, sizeof(uint16_t));
	outFile.write((char*)m_song_length, sizeof(uint32_t));
	UnicodeString::U32ToWebTypedFile(m_icon, outFile);
	UnicodeString::U32ToWebTypedFile(m_source, outFile);
	outFile.write((char*)&m_hopeFrequency, sizeof(uint32_t));

	for (auto track : m_noteTrackScans.scanArray)
	{
		outFile.put(track->m_scanValue);
		outFile.put(track->m_intensity);
	}
	m_hash.writeToCache(outFile);

	auto end = outFile.tellp();
	length += uint32_t(end - start) - 4;
	outFile.seekp(start);
	outFile.write((char*)&length, 4);
	outFile.seekp(end);
}

SongEntry::CacheStatus SongEntry::readFromCache(const unsigned char*& currPtr)
{
	static constexpr auto getFileTime = [](const unsigned char*& curr)
	{
		std::filesystem::file_time_type::duration time;
		memcpy(&time, curr, sizeof(std::filesystem::file_time_type::duration));
		curr += sizeof(uint64_t);
		return std::filesystem::file_time_type(time);
	};

	static constexpr auto readValue = [](auto& value, const unsigned char*& curr)
	{
		memcpy(&value, curr, sizeof(value));
		curr += sizeof(value);
	};

	std::u32string directory = UnicodeString::U32FromWebTypedFile(currPtr);
	m_directory = directory;

	directory += U'/';

	std::filesystem::directory_entry iniEntry;
	try
	{
		iniEntry.assign(directory + U"song.ini");
		if (!iniEntry.exists())
			return CacheStatus::NOT_PRESENT;
	}
	catch (...)
	{
		return CacheStatus::NOT_PRESENT;
	}
	m_iniModifiedTime = getFileTime(currPtr);

	const std::u32string chartName = UnicodeString::U32FromWebTypedFile(currPtr);
	try
	{
		m_fileEntry.assign(directory + chartName);
		if (!m_fileEntry.exists())
			return CacheStatus::NOT_PRESENT;
	}
	catch (...)
	{
		return CacheStatus::NOT_PRESENT;
	}
	m_chartModifiedTime = getFileTime(currPtr);

	if (m_fileEntry.last_write_time() != m_chartModifiedTime)
	{
		static const std::u32string CHARTNAMES[] = { U"notes.bch", U"notes.cht", U"notes.mid", U"notes.midi", U"notes.chart" };
		for (int i = 0; i < 5; ++i)
		{
			if (chartName == CHARTNAMES[i])
			{
				if (scan_Ini(iniEntry) || i == 1 || i == 4)
					if (scan(i))
						return CacheStatus::CHANGED;
				break;
			}
		}

		return CacheStatus::NOT_PRESENT;
	}

	CacheStatus status = CacheStatus::UNCHANGED;
	if (iniEntry.last_write_time() != m_iniModifiedTime)
	{
		if (!scan_Ini(iniEntry) && chartName != U"notes.chart" && chartName != U"notes.cht")
			return CacheStatus::NOT_PRESENT;

		currPtr += sizeof(m_previewRange);
		currPtr += 2 * sizeof(uint16_t);
		currPtr += sizeof(uint32_t);
		currPtr += WebType::read(currPtr);
		currPtr += WebType::read(currPtr);
		currPtr += sizeof(uint32_t);
		status = CacheStatus::CHANGED;
	}
	else
	{
		m_modifiers.reserve(8);

		readValue(m_previewRange, currPtr);
		readValue(m_albumTrack, currPtr);
		readValue(m_playlistTrack, currPtr);

		uint32_t songLength;
		readValue(songLength, currPtr);

		if (songLength > 0)
			m_modifiers.push_back({ "song_length", songLength });

		m_icon = UnicodeString::U32FromWebTypedFile(currPtr);
		m_source = UnicodeString::U32FromWebTypedFile(currPtr);
		readValue(m_hopeFrequency, currPtr);
	}

	for (auto track : m_noteTrackScans.scanArray)
	{
		track->m_scanValue = *currPtr++;
		track->m_intensity = (char)*currPtr++;
	}
	m_hash.readFromCache(currPtr);
	return status;
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

void SongEntry::setSongInfoFromCache(
	const UnicodeString& _artist,
	const UnicodeString& _name,
	const UnicodeString& _album,
	const UnicodeString& _genre,
	const UnicodeString& _year,
	const UnicodeString& _charter,
	const UnicodeString& _playlist)
{
	if (_artist != s_DEFAULT_ARTIST)
		m_modifiers.push_back({"artist", _artist });

	if (_name != s_DEFAULT_NAME)
		m_modifiers.push_back({"name", _name });

	if (_album != s_DEFAULT_ALBUM)
		m_modifiers.push_back({"artist", _album });

	if (_genre != s_DEFAULT_GENRE)
		m_modifiers.push_back({"genre", _genre });

	if (_year != s_DEFAULT_YEAR)
		m_modifiers.push_back({"year", _year });

	if (_charter != s_DEFAULT_CHARTER)
		m_modifiers.push_back({"charter", _charter });

	m_modifiers.push_back({"playlist", _playlist });
}
