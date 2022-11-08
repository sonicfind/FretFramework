#pragma once
#include "Modifiers/ModifierNode.h"
#include "Chords/GuitarNote/GuitarNote_cht.hpp"
#include "Chords/GuitarNote/GuitarNote_bch.hpp"
#include "Chords/Keys.h"
#include "Drums/DrumNote_cht.hpp"
#include "Drums/DrumNote_bch.hpp"

#include "Tracks/InstrumentalTracks/InstrumentalTrack_cht.hpp"
#include "Tracks/InstrumentalTracks/InstrumentalTrack_bch.hpp"
#include "Tracks/InstrumentalTracks/DrumTrack/DrumTrack_Legacy.h"
#include "Tracks/VocalTracks/VocalTrack_cht.hpp"
#include "Tracks/VocalTracks/VocalTrack_bch.hpp"

#include "Song/Track Scans/InstrumentalTracks/InstrumentalTrack_Scan_cht.hpp"
#include "Song/Track Scans/InstrumentalTracks/InstrumentalTrack_Scan_bch.hpp"
#include "Song/Track Scans/InstrumentalTracks/DrumTrack/DrumTrack_Scan_Legacy.h"
#include "Song/Track Scans/VocalTracks/VocalTrack_Scan_cht.hpp"
#include "Song/Track Scans/VocalTracks/VocalTrack_Scan_bch.hpp"

#include "Sync/SyncValues.h"
#include "MD5/MD5.h"
#include <filesystem>

enum class SongAttribute
{
	UNSPECIFIED,
	TITLE,
	ARTIST,
	ALBUM,
	GENRE,
	YEAR,
	CHARTER,
	PLAYLIST
};

enum class InstrumentType
{
	Guitar_lead,
	Guitar_lead_6,
	Guitar_bass,
	Guitar_bass_6,
	Guitar_rhythm,
	Guitar_coop,
	Keys,
	Drums_4,
	Drums_5,
	Vocals,
	Harmonies,
	Drums_Legacy,
	None
};

enum StorageDriveType
{
	HDD,
	SSD
};

class SongListEntry
{
	class InvalidFileException : public std::runtime_error
	{
	public:
		InvalidFileException(const std::string& file) : std::runtime_error("Error: \"" + file + "\" is not a valid chart file") {}
	};

	static constexpr const char* s_NOTETRACKNAMES[] =
	{
		"[LeadGuitar]",
		"[LeadGuitar_GHL]",
		"[BassGuitar]",
		"[BassGuitar_GHL]",
		"[RhythmGuitar]",
		"[CoopGuitar]",
		"[Keys]",
		"[Drums_4Lane]",
		"[Drums_5Lane]",
		"[Vocals]",
		"[Harmonies]"
	};

	struct Scans
	{
		InstrumentalTrack_Scan<GuitarNote<5>>            lead_5;
		InstrumentalTrack_Scan<GuitarNote<6>>            lead_6;
		InstrumentalTrack_Scan<GuitarNote<5>>            bass_5;
		InstrumentalTrack_Scan<GuitarNote<6>>            bass_6;
		InstrumentalTrack_Scan<GuitarNote<5>>            rhythm;
		InstrumentalTrack_Scan<GuitarNote<5>>            coop;
		InstrumentalTrack_Scan<Keys<5>>                  keys;
		InstrumentalTrack_Scan<DrumNote<4, DrumPad_Pro>> drums4_pro;
		InstrumentalTrack_Scan<DrumNote<5, DrumPad>>     drums5;
		VocalTrack_Scan<1>                               vocals;
		VocalTrack_Scan<3>                               harmonies;

		NoteTrack_Scan* const scanArray[11] =
		{
			&lead_5,
			&lead_6,
			&bass_5,
			&bass_6,
			&rhythm,
			&coop,
			&keys,
			&drums4_pro,
			&drums5,
			&vocals,
			&harmonies
		};

		static constexpr size_t s_ARRAYLENGTH = sizeof(scanArray) / sizeof(NoteTrack_Scan*);

		Scans() = default;
		Scans(const Scans& other) noexcept;
		Scans(Scans&&) noexcept;
		Scans& operator=(const Scans& other) noexcept;
		Scans& operator=(Scans&&) noexcept;
	} m_noteTrackScans;

	MD5 m_hash;
	std::vector<TxtFileModifier> m_modifiers;

	std::filesystem::directory_entry m_fileEntry;
	std::filesystem::path m_directory;
	StorageDriveType m_storageType;

	std::filesystem::file_time_type m_iniModifiedTime;
	std::filesystem::file_time_type m_chartModifiedTime;

	float m_previewRange[2]{};
	uint16_t m_albumTrack = UINT16_MAX;
	uint16_t m_playlistTrack = UINT16_MAX;
	std::u32string m_icon;
	std::u32string m_source;
	uint32_t m_hopeFrequency = 0;
	
	static const     UnicodeString s_DEFAULT_NAME;
	static const     UnicodeString s_DEFAULT_ARTIST;
	static const     UnicodeString s_DEFAULT_ALBUM;
	static const     UnicodeString s_DEFAULT_GENRE;
	static const     UnicodeString s_DEFAULT_YEAR;
	static const     UnicodeString s_DEFAULT_CHARTER;
	static constexpr uint32_t      s_DEFAULT_SONG_LENGTH = 0;

	const UnicodeString* m_name =         &s_DEFAULT_NAME;
	const UnicodeString* m_artist =       &s_DEFAULT_ARTIST;
	const UnicodeString* m_album =        &s_DEFAULT_ALBUM;
	const UnicodeString* m_genre =        &s_DEFAULT_GENRE;
	const UnicodeString* m_year =         &s_DEFAULT_YEAR;
	const UnicodeString* m_charter =      &s_DEFAULT_CHARTER;
	const uint32_t*      m_song_length =  &s_DEFAULT_SONG_LENGTH;
	const UnicodeString* m_playlist =     nullptr;

	bool m_hasIniFile = false;
	bool m_writeIniAfterScan = false;

public:
	SongListEntry() = default;
	SongListEntry(const SongListEntry&) = default;
	SongListEntry(SongListEntry&&) = default;
	SongListEntry& operator=(const SongListEntry&) = default;
	SongListEntry& operator=(SongListEntry&&) = default;

	SongListEntry(std::filesystem::directory_entry&& fileEntry, StorageDriveType type = SSD);
	SongListEntry(const std::filesystem::path& filepath);
	SongListEntry(StorageDriveType type) : m_storageType(type) {}

	void load_Ini(const std::filesystem::path& filepath);
	bool scan_Ini(const std::filesystem::directory_entry& iniEntry);
	std::filesystem::file_time_type save_Ini() const;
	bool hasIniFile() const { return m_hasIniFile; }

	bool scan(const int _chartNameIndex);
	void setDriveType(StorageDriveType type) { m_storageType = type; }
	constexpr bool validateScans();
	void finalizeScan();
	void displayScanResult() const;
	void writeChartDataToCache(std::fstream& outFile) const;
	void writeSongInfoToCache(std::fstream& outFile) const;

	enum class CacheStatus
	{
		UNCHANGED,
		CHANGED,
		NOT_PRESENT
	};

	CacheStatus readFromCache(const unsigned char*& currPtr);

	void setSongInfoFromCache(
		const UnicodeString& _artist,
		const UnicodeString& _name,
		const UnicodeString& _album,
		const UnicodeString& _genre,
		const UnicodeString& _year,
		const UnicodeString& _charter,
		const UnicodeString& _playlist);

	bool checkLastModfiedDate() const;
	unsigned char getScanValue(InstrumentType trackType)
	{
		assert(static_cast<int>(trackType) < m_noteTrackScans.s_ARRAYLENGTH);
		return m_noteTrackScans.scanArray[static_cast<int>(trackType)]->m_scanValue;
	}

	const UnicodeString& getArtist() const { return *m_artist; }
	const UnicodeString& getName() const { return *m_name; }
	const UnicodeString& getAlbum() const { return *m_album; }
	const UnicodeString& getGenre() const { return *m_genre; }
	const UnicodeString& getYear() const { return *m_year; }
	const UnicodeString& getCharter() const { return *m_charter; }
	const UnicodeString& getPlaylist() const { return *m_playlist; }
	const uint32_t& getSongLength() const { return *m_song_length; }
	const std::filesystem::path& getDirectory() const { return m_directory; }

	const TxtFileModifier* const getModifier(const std::string_view modifierName) const
	{
		for (const TxtFileModifier& modifier : m_modifiers)
			if (modifier.getName() == modifierName)
				return &modifier;
		return nullptr;
	}

	TxtFileModifier* const getModifier(const std::string_view modifierName)
	{
		for (TxtFileModifier& modifier : m_modifiers)
			if (modifier.getName() == modifierName)
				return &modifier;
		return nullptr;
	}

	void removeModifier(const std::string_view modifierName);

	void removeModifier_if(const std::string_view modifierName, bool(*func)(const TxtFileModifier&));

	template <class T>
	void setModifier(const std::string_view modifierName, const T& value)
	{
		if (TxtFileModifier* modifier = getModifier(modifierName))
			*modifier = value;
		else
			m_modifiers.push_back({ modifierName, value });
	}

	template <size_t SIZE>
	std::pair<uint16_t, uint16_t> readModifiersFromChart(const std::pair<std::string_view, ModifierNode>(&_MODIFIERLIST)[SIZE], TextTraversal& _traversal)
	{
		uint16_t version = 0;
		uint16_t tickRate = 192;

		if (!m_hasIniFile)
		{
			size_t modifierCount = 0;
			TextTraversal counter = _traversal;
			while (counter && counter != '}' && counter != '[')
			{
				++modifierCount;
				counter.next();
			}

			m_modifiers.reserve(modifierCount);
		}

		std::vector<TxtFileModifier> tmp;

		bool versionChecked = false;
		bool resolutionChecked = false;
		while (_traversal && _traversal != '}' && _traversal != '[')
		{
			if (const ModifierNode* node = ModifierNode::testForModifierName(_MODIFIERLIST, _traversal.extractModifierName()))
			{
				if (node->m_name[0] == 'F')
				{
					if (!versionChecked)
					{
						version = _traversal.extract<uint16_t>();
						versionChecked = true;
					}
				}
				else if (node->m_name[0] == 'R' && node->m_name[1] == 'e')
				{
					if (!resolutionChecked)
					{
						tickRate = _traversal.extract<uint16_t>();;
						resolutionChecked = true;
					}
				}
				else
				{
					TxtFileModifier* modifier = getModifier(node->m_name);
					try
					{
						if (!modifier)
						{
							TxtFileModifier newMod = node->createModifier(_traversal);
							if (node->m_name != "delay" &&
								node->m_name != "preview_start_time" &&
								node->m_name != "preview_end_time" &&
								node->m_name != "diff_band")
							{
								m_modifiers.push_back(std::move(newMod));
								m_writeIniAfterScan = true;
							}
							else
								tmp.push_back(std::move(newMod));
						}
						else if (testifModifierIsDefault(*modifier))
						{
							*modifier = node->createModifier(_traversal);
							if (!testifModifierIsDefault(*modifier))
								m_writeIniAfterScan = true;
						}
					}
					catch (...)
					{
						m_writeIniAfterScan = true;
					}
				}
			}
			_traversal.next();
		}

		if (!getModifier("preview_start_time") && !getModifier("preview_end_time"))
		{
			TxtFileModifier* startTime = nullptr;
			TxtFileModifier* endTime = nullptr;
			for (auto& mod : tmp)
			{
				if (!startTime && mod.getName() == "preview_start_time")
					startTime = &mod;
				else if (!endTime && mod.getName() == "preview_end_time")
					endTime = &mod;

				if (startTime && endTime)
				{
					const float start = startTime->getValue<float>();
					if (endTime->getValue<float>() <= start)
					{
						if (start)
						{
							m_modifiers.push_back(std::move(*startTime));
							m_writeIniAfterScan = true;
						}
					}
					else
					{
						m_modifiers.push_back(std::move(*startTime));
						m_modifiers.push_back(std::move(*endTime));
					}
					break;
				}
			}

			if (startTime)
			{
				m_modifiers.push_back(std::move(*startTime));
				m_writeIniAfterScan = true;
			}
			else if (endTime)
			{
				m_modifiers.push_back(std::move(*endTime));
				m_writeIniAfterScan = true;
			}
		}

		if (auto delay = getModifier("delay"); !delay || delay->getValue<float>() == 0)
		{
			for (auto& mod : tmp)
				if (mod.getName() == "delay")
				{
					if (mod.getValue<float>() != 0)
					{
						if (!delay)
							m_modifiers.push_back(std::move(mod));
						else
							*delay = std::move(mod);

						m_writeIniAfterScan = true;
					}
					break;
				}
		}

		if (!getModifier("diff_band"))
		{
			for (auto& mod : tmp)
				if (mod.getName() == "diff_band")
				{
					if (mod.getValue<int32_t>() != 0)
					{
						m_modifiers.push_back(std::move(mod));
						m_writeIniAfterScan = true;
					}
					break;
				}
		}

		return { version, tickRate };
	}

	void writeModifiersToChart(std::fstream& outFile)
	{
		for (const auto& modifier : m_modifiers)
			if (modifier.getName()[0] <= 90)
				modifier.write_cht(outFile);
	}

private:
	void scanFile(TextTraversal&& traversal);
	void scanFile(BCHTraversal&& traversal);
	void scanFile(MidiTraversal&& traversal);

	bool testifModifierIsDefault(const TxtFileModifier& _modifier)
	{
		if      (_modifier.getName() == "artist")      return _modifier.getValue<UnicodeString>() == s_DEFAULT_ARTIST;
		else if (_modifier.getName() == "name")        return _modifier.getValue<UnicodeString>() == s_DEFAULT_NAME;
		else if (_modifier.getName() == "album")       return _modifier.getValue<UnicodeString>() == s_DEFAULT_ALBUM;
		else if (_modifier.getName() == "genre")       return _modifier.getValue<UnicodeString>() == s_DEFAULT_GENRE;
		else if (_modifier.getName() == "year")        return _modifier.getValue<UnicodeString>() == s_DEFAULT_YEAR;
		else if (_modifier.getName() == "charter")     return _modifier.getValue<UnicodeString>() == s_DEFAULT_CHARTER;
		else if (_modifier.getName() == "song_length") return _modifier.getValue<uint32_t>() == s_DEFAULT_SONG_LENGTH;
		return false;
	}
	void setBaseModifiers();

public:

	void setFullPath(const std::filesystem::path& path);
	void setChartFile(const char32_t* filename);
	const std::filesystem::directory_entry& getFileEntry() const { return m_fileEntry; }
	const std::filesystem::path& getFilePath() const { return m_fileEntry.path(); }

	template<SongAttribute Attribute>
	constexpr const UnicodeString* getAttribute() const
	{
		if constexpr (Attribute == SongAttribute::TITLE)
			return m_name;
		else if constexpr (Attribute == SongAttribute::ARTIST)
			return m_artist;
		else if constexpr (Attribute == SongAttribute::ALBUM)
			return m_album;
		else if constexpr (Attribute == SongAttribute::GENRE)
			return m_genre;
		else if constexpr (Attribute == SongAttribute::YEAR)
			return m_year;
		else if constexpr (Attribute == SongAttribute::CHARTER)
			return m_charter;
		else if constexpr (Attribute == SongAttribute::PLAYLIST)
			return m_playlist;
	}

	template <SongAttribute Attribute>
	bool isLowerOrdered(const SongListEntry& other) const
	{
		if constexpr (Attribute == SongAttribute::ALBUM)
		{
			if (m_albumTrack != other.m_albumTrack)
				return m_albumTrack < other.m_albumTrack;
		}
		else if constexpr (Attribute == SongAttribute::PLAYLIST)
		{
			if (m_playlistTrack != other.m_playlistTrack)
				return m_playlistTrack < other.m_playlistTrack;
		}

		int strCmp = 0;
		if ((strCmp = m_name->compare(*other.m_name)) != 0 ||
			(strCmp = m_artist->compare(*other.m_artist)) != 0 ||
			(strCmp = m_album->compare(*other.m_album)) != 0 ||
			(strCmp = m_charter->compare(*other.m_charter)) != 0)
			return strCmp < 0;
		else
			return m_directory < other.m_directory;
	}

	bool areHashesEqual(const SongListEntry& other) const;
	bool isHashLessThan(const SongListEntry& other) const;
};
