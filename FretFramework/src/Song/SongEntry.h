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
	TITLE,
	ARTIST,
	ALBUM,
	GENRE,
	YEAR,
	CHARTER,
	PLAYLIST
};

class SongEntry
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

		Scans() = default;
		Scans(const Scans&) = delete;
		Scans(Scans&&) noexcept;
		Scans& operator=(const Scans&) = delete;
		Scans& operator=(Scans&&) noexcept;
	} m_noteTrackScans;

	MD5 m_hash;
	std::vector<TxtFileModifier> m_modifiers;

	std::filesystem::path m_directory;
	std::filesystem::path m_chartFile;
	std::filesystem::path m_fullPath;
	UnicodeString m_directory_as_playlist;

	std::filesystem::file_time_type m_last_modified;
	
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

	const UnicodeString& getArtist() const { return *m_artist; }
	const UnicodeString& getName() const { return *m_name; }
	const UnicodeString& getAlbum() const { return *m_album; }
	const UnicodeString& getGenre() const { return *m_genre; }
	const UnicodeString& getYear() const { return *m_year; }
	const UnicodeString& getCharter() const { return *m_charter; }
	const uint32_t& getSongLength() const { return *m_song_length; }

	bool m_hasIniFile = false;
	bool m_writeIniAfterScan = false;

public:
	SongEntry() = default;
	SongEntry(const SongEntry&) = delete;
	SongEntry(SongEntry&&) = default;
	SongEntry& operator=(const SongEntry&) = delete;
	SongEntry& operator=(SongEntry&&) = default;

	SongEntry(const std::filesystem::path& filepath);
	void load_Ini();
	void save_Ini() const;
	bool hasIniFile() const { return m_hasIniFile; }

	bool scan(bool iniLocated, bool iniRequired);
	constexpr bool validateScans();
	void finalizeScan();
	void displayScanResult() const;

	bool checkLastModfiedDate() const;

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
							m_modifiers.emplace_back(node->createModifier(_traversal));
							m_writeIniAfterScan = true;
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

		return { version, tickRate };
	}

	void writeModifiersToChart(std::fstream& outFile)
	{
		for (const auto& modifier : m_modifiers)
			if (modifier.getName()[0] <= 90)
				modifier.write(outFile);
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

	constexpr void setFullPath(const std::filesystem::path& path);
	void setDirectory(const std::filesystem::path& directory);
	void setChartFile(const char32_t* filename);
	std::filesystem::path getFilePath() const { return m_fullPath; }
	std::filesystem::path getDirectory() const { return m_directory; }
	std::filesystem::path getChartFile() const { return m_chartFile; }

	static SongAttribute s_sortAttribute;
	static constexpr void setSortAttribute(SongAttribute attribute) { s_sortAttribute = attribute; }

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
		{
			if (auto playlist = getModifier("playlist"))
				return &playlist->getValue<UnicodeString>();
			return &m_directory_as_playlist;
		}
	}

	// Compares only by the file's hash
	bool operator<(const SongEntry& other) const;
	bool areHashesEqual(const SongEntry& other) const;
	bool isHashLessThan(const SongEntry& other) const;
};
