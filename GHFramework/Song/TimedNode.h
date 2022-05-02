#pragma once
#include <stdint.h>
#include <fstream>
#include <sstream>

class EndofFileException : public std::runtime_error
{
public:
	EndofFileException() : std::runtime_error("") {}
};

class EndofTrackException : public std::runtime_error
{
public:
	EndofTrackException() : std::runtime_error("") {}
};

class EndofLineException : public std::runtime_error
{
public:
	EndofLineException() : std::runtime_error("") {}
};

class InvalidNoteException : public std::runtime_error
{
public:
	InvalidNoteException(int color) : std::runtime_error("invalid color value (" + std::to_string(color) + ')') {}
	InvalidNoteException() : std::runtime_error("invalid color values") {}
};

class Toggleable
{
	bool m_state = false;
public:
	void toggle() { m_state = !m_state; }
	void operator=(bool state) { m_state = state; }
	operator bool() const { return m_state; }
	bool operator==(const Toggleable& tg)
	{
		return m_state == tg.m_state;
	}

	bool operator!=(const Toggleable& tg)
	{
		return m_state != tg.m_state;
	}
};

class Hittable
{
	static uint16_t s_tickRate;
	static float s_forceThreshold;
public:
	Toggleable m_isActive;
	virtual void init(uint32_t sustain = 0) { m_isActive = true; }
	operator bool() const { return m_isActive; }
	void toggle() { m_isActive.toggle(); }
	void save_cht(int lane, std::fstream& outFile) const;
	bool operator==(Hittable& hit)
	{
		return m_isActive == hit.m_isActive;
	}

	bool operator!=(Hittable& hit)
	{
		return m_isActive != hit.m_isActive;
	}

	static void setTickRate(uint16_t tickRate)
	{
		s_tickRate = tickRate;
		s_forceThreshold = s_tickRate / 3.0f;
	}

	static uint16_t getTickRate()
	{
		return s_tickRate;
	}

	static float getForceThreshold()
	{
		return s_forceThreshold;
	}
};

class Sustainable : public Hittable
{
protected:
	// Must take sustain gap into account
	uint32_t m_sustain = 0;
public:
	void init(uint32_t sustain = 0)
	{
		m_isActive = true;
		m_sustain = sustain;
	}
	uint32_t getSustain() const { return m_sustain; }
	void setSustain(uint32_t sustain) { m_sustain = sustain; }
	void save_cht(int lane, std::fstream& outFile) const;
};

class Modifiable : public Sustainable
{
public:
	virtual bool modify(char modifier) = 0;
	virtual void save_modifier_cht(std::fstream& outFile) const = 0;
	virtual void save_modifier_cht(int lane, std::fstream& outFile) const = 0;
};

class DrumPad : public Modifiable
{
public:
	Toggleable m_isAccented;
	Toggleable m_isGhosted;

	bool modify(char modifier);
	void save_modifier_cht(std::fstream& outFile) const;
	void save_modifier_cht(int lane, std::fstream& outFile) const;
};

class DrumPad_Pro : public DrumPad
{
public:
	Toggleable m_isCymbal;

	bool modify(char modifier);
	void save_modifier_cht(std::fstream& outFile) const;
	void save_modifier_cht(int lane, std::fstream& outFile) const;
};

class DrumPad_Bass : public Modifiable
{
public:
	Toggleable m_isDoubleBass;
	bool modify(char modifier);
	void save_modifier_cht(std::fstream& outFile) const;
	void save_modifier_cht(int lane, std::fstream& outFile) const;
};

class Pitched : public Sustainable
{
protected:
	// Valid pitches: 36 - 84 (83 for RB Blitz)
	char m_pitch = 0;

public:
	void setPitch(char pitch);
	char getPitch() const { return m_pitch; }
	void save_pitch_cht(std::fstream& outFile) const;
	void save_pitch_cht(int lane, std::fstream& outFile) const;
};

class Vocal : public Pitched
{
	std::string m_lyric;
public:
	void setLyric(const std::string& text);
	std::string getLyric() const { return m_lyric; }
	void save_cht(int lane, std::fstream& outFile) const;

	operator bool() const { return !m_lyric.empty(); }
	bool isSung() const { return m_pitch != 0 || (!m_lyric.empty() && m_lyric[0] == '#'); }
};

class VocalPercussion : public Modifiable
{
public:
	Toggleable m_noiseOnly;
	bool modify(char modifier);
	void save_modifier_cht(std::fstream& outFile) const;
	void save_modifier_cht(int lane, std::fstream& outFile) const;
};

template <size_t numColors, class NoteType, class SpecialType>
class Note
{
public:
	NoteType m_colors[numColors];
	SpecialType m_special;

	bool init(size_t lane, uint32_t sustain = 0)
	{
		if (lane > numColors)
			return false;

		if (lane == 0)
			m_special.init(sustain);
		else
			m_colors[lane - 1].init(sustain);

		return true;
	}

	void init_chartV1(int lane, uint32_t sustain) {}
	virtual void init_cht_single(const char* str) = 0;
	void init_cht_chord(const char* str)
	{
		int colors;
		int count;
		if (sscanf_s(str, " %i%n", &colors, &count) == 1)
		{
			int numAdded = 0;
			str += count;
			int lane;
			for (int i = 0;
				i < colors && sscanf_s(str, " %i%n", &lane, &count) == 1;
				++i)
			{
				str += count;
				unsigned char color = lane & 127;
				uint32_t sustain = 0;
				if (lane & 128)
				{
					if (sscanf_s(str, " %lu%n", &sustain, &count) != 1)
						throw EndofLineException();
					str += count;
				}

				if (color == 0)
				{
					m_special.init(sustain);
					++numAdded;
				}
				else if (color <= numColors)
				{
					m_colors[color - 1].init(sustain);
					++numAdded;
				}
			}

			if (numAdded == 0)
				throw InvalidNoteException();
		}
		else
			throw EndofLineException();
	}

	virtual bool modify(char modifier, bool toggle = true) = 0;
	bool modifyColor(int lane, char modifier) { return false; }
	virtual void modify_cht(const char* str) = 0;

	virtual uint32_t getNumActiveColors() const
	{
		uint32_t num = m_special ? 1 : 0;
		for (int lane = 0; lane < numColors; ++lane)
			if (m_colors[lane])
				++num;
		return num;
	}

	virtual void save_cht(uint32_t position, std::fstream& outFile) const = 0;

protected:
	uint32_t write_notes_cht(uint32_t position, std::fstream& outFile, const char* const tabs = "\t\t") const
	{
		uint32_t numActive = getNumActiveColors();
		if (numActive == 1)
		{
			outFile << tabs << position << " = N";
			if (m_special)
				m_special.save_cht(0, outFile);
			else
			{
				int lane = 0;
				while (!m_colors[lane])
					++lane;

				m_colors[lane].save_cht(lane + 1, outFile);
			}
		}
		else
		{
			outFile << tabs << position << " = C " << numActive;
			if (m_special)
				m_special.save_cht(0, outFile);

			for (int lane = 0; lane < numColors; ++lane)
				if (m_colors[lane])
					m_colors[lane].save_cht(lane + 1, outFile);
		}
		return numActive;
	}

public:
	bool operator==(const Note& note) const
	{
		if (m_special != note.m_special)
			return false;
		else
		{
			for (int i = 0; i < numColors; ++i)
				if (m_colors[i] != note.m_colors[i])
					return false;
			return true;
		}
	}

	bool operator!=(const Note& note) const
	{
		return !operator==(note);
	}

	static uint32_t getLaneSize()
	{
		return numColors;
	}
};

template <size_t numColors>
class GuitarNote : public Note<numColors, Sustainable, Sustainable>
{
public:
	using Note<numColors, Sustainable, Sustainable>::m_colors;
	using Note<numColors, Sustainable, Sustainable>::m_special;
	enum class ForceStatus
	{
		UNFORCED,
		FORCED,
		HOPO_ON,
		HOPO_OFF
	} m_isForced = ForceStatus::UNFORCED;
	Toggleable m_isTap;

private:
	// Checks modifier value from a v1 .chart file
	bool checkModifiers(size_t lane, uint32_t sustain)
	{
		switch (lane)
		{
		case 5:
			m_isForced = ForceStatus::FORCED;
			break;
		case 6:
			m_isTap = true;
			break;
		case 7:
			m_special.init(sustain);
			break;
		default:
			return false;
		}
		return true;
	}

public:
	bool init(size_t lane, uint32_t sustain = 0)
	{
		if (lane > numColors)
			return false;

		if (lane == 0)
		{
			m_special.init(sustain);
			static const Sustainable replacement[numColors];
			memcpy(m_colors, replacement, sizeof(Sustainable) * numColors);
		}
		else
		{
			m_colors[lane - 1].init(sustain);
			m_special = Sustainable();
		}
		return true;
	}

	// Pulls values from a V1 .chart file
	// Returns whether a valid value could be utilized
	void init_chartV1(int lane, uint32_t sustain)
	{
		if (lane < 5)
			m_colors[lane].init(sustain);
		else if (!checkModifiers(lane, sustain))
			throw InvalidNoteException(lane);
	}

	void init_cht_single(const char* str)
	{
		// Read note
		int lane, count;
		if (sscanf_s(str, " %i%n", &lane, &count) != 1)
			throw EndofLineException();

		str += count;
		unsigned char color = lane & 127;
		uint32_t sustain = 0;
		if (lane & 128)
		{
			if (sscanf_s(str, " %lu%n", &sustain, &count) != 1)
				throw EndofLineException();
			str += count;
		}

		if (color > numColors)
			throw InvalidNoteException(color);

		if (color == 0)
			m_special.init(sustain);
		else
			m_colors[color - 1].init(sustain);

		// Read modifiers
		int numMods;
		if (*str && sscanf_s(str, " %i%n", &numMods, &count) == 1)
		{
			str += count;
			char modifier;
			for (int i = 0;
				i < numMods && sscanf_s(str, " %c%n", &modifier, 1, &count) == 1;
				++i)
			{
				str += count;
				switch (modifier)
				{
				case 'F':
					m_isForced = ForceStatus::FORCED;
					break;
				case '<':
					m_isForced = ForceStatus::HOPO_ON;
					break;
				case '>':
					m_isForced = ForceStatus::HOPO_OFF;
					break;
				case 'T':
					m_isTap = true;
				}
			}
		}
	}

	void init_cht_chord(const char* str)
	{
		int colors;
		int count;
		if (sscanf_s(str, " %i%n", &colors, &count) == 1)
		{
			int numAdded = 0;
			str += count;
			int lane;
			for (int i = 0;
				i < colors && sscanf_s(str, " %i%n", &lane, &count) == 1;
				++i)
			{
				str += count;
				unsigned char color = lane & 127;
				uint32_t sustain = 0;
				if (lane & 128)
				{
					if (sscanf_s(str, " %lu%n", &sustain, &count) != 1)
						throw EndofLineException();
					str += count;
				}

				if (color == 0)
				{
					m_special.init(sustain);
					numAdded = 1;
					static const Sustainable replacement[numColors];
					memcpy(m_colors, replacement, sizeof(Sustainable) * numColors);
				}
				else if (color <= numColors)
				{
					m_colors[color - 1].init(sustain);
					if (!m_special)
						++numAdded;
					else
						m_special = Sustainable();
				}
			}

			if (numAdded == 0)
				throw InvalidNoteException();
		}
		else
			throw EndofLineException();
	}

	bool modify(char modifier, bool toggle = true)
	{
		switch (modifier)
		{
		case 'F':
			switch (m_isForced)
			{
			case ForceStatus::UNFORCED:
				m_isForced = ForceStatus::FORCED;
				break;
			default:
				m_isForced = ForceStatus::UNFORCED;
				break;
			}
			break;
		case 'T':
			if (toggle)
				m_isTap.toggle();
			else
				m_isTap = true;
			break;
		case '<':
			m_isForced = ForceStatus::HOPO_ON;
			break;
		case '>':
			m_isForced = ForceStatus::HOPO_OFF;
			break;
		default:
			return false;
		}
		return true;
	}

	void modify_cht(const char* str)
	{
		int numMods;
		int count;
		if (sscanf_s(str, " %i%n", &numMods, &count) == 1)
		{
			str += count;
			char modifier;
			for (int i = 0;
				i < numMods && sscanf_s(str, " %c%n", &modifier, 1, &count) == 1;
				++i)
			{
				str += count;
				switch (modifier)
				{
				case 'F':
					m_isForced = ForceStatus::FORCED;
					break;
				case '<':
					m_isForced = ForceStatus::HOPO_ON;
					break;
				case '>':
					m_isForced = ForceStatus::HOPO_OFF;
					break;
				case 'T':
					m_isTap = true;
				}
			}
		}
	}

	// write values to a V2 .chart file
	void save_cht(const uint32_t position, std::fstream& outFile) const
	{
		int numActive = Note<numColors, Sustainable, Sustainable>::write_notes_cht(position, outFile);
		int numMods = m_isForced != ForceStatus::UNFORCED ? 1 : 0;
		if (m_isTap)
			++numMods;

		if (numMods > 0)
		{
			if (numActive > 1)
				outFile << "\n\t\t" << position << " = M";
			outFile << ' ' << numMods;

			switch (m_isForced)
			{
			case ForceStatus::FORCED:
				outFile << " F";
				break;
			case ForceStatus::HOPO_ON:
				outFile << " <";
				break;
			case ForceStatus::HOPO_OFF:
				outFile << " >";
			}

			if (m_isTap)
				outFile << " T";
		}
		outFile << '\n';
	}

	uint32_t getLongestSustain() const
	{
		if (m_special)
			return m_special.getSustain();
		else
		{
			uint32_t sustain = 0;
			for (const auto& color : m_colors)
				if (color && color.getSustain() > sustain)
					sustain = color.getSustain();
			return sustain;
		}
	}
};

template<>
void GuitarNote<6>::init_chartV1(int lane, uint32_t sustain);

class DrumNote : public Note<4, DrumPad_Pro, DrumPad_Bass>
{
	static bool s_is5Lane;
public:
	using Note<4, DrumPad_Pro, DrumPad_Bass>::m_special;
	using Note<4, DrumPad_Pro, DrumPad_Bass>::m_colors;
	DrumPad m_fifthLane;
	Toggleable m_isFlamed;

private:
	void checkFlam();

public:
	using Note<4, DrumPad_Pro, DrumPad_Bass>::Note;
	// Pulls values from a V1 .chart file
	// Returns whether a valid value could be utilized
	void init_chartV1(int lane, uint32_t sustain);
	bool init(size_t lane, uint32_t sustain = 0);
	void init_cht_single(const char* str);
	void init_cht_chord(const char* str);
	bool modify(char modifier, bool toggle = true);
	bool modifyColor(int lane, char modifier);
	void modify_cht(const char* str);
	void save_cht(const uint32_t position, std::fstream& outFile) const;
	static void resetLaning();

	uint32_t getNumActiveColors() const
	{
		uint32_t num = m_fifthLane ? 1 : 0;
		return num + Note::getNumActiveColors();
	}

	static uint32_t getLaneSize()
	{
		if (s_is5Lane)
			return 5;
		else
			return 4;
	}
};

template <size_t numTracks>
class VocalNote : public Note<numTracks, Vocal, VocalPercussion>
{
public:
	using Note<numTracks, Vocal, VocalPercussion>::m_colors;
	using Note<numTracks, Vocal, VocalPercussion>::m_special;

	bool init(size_t lane, uint32_t sustain = 0)
	{
		if (lane > numTracks)
			return false;

		if (lane == 0)
		{
			m_special.init(sustain);
			for (auto& col : m_colors)
				if (col)
					col = Vocal();
		}
		else
		{
			m_colors[lane - 1].init(sustain);
			m_special = VocalPercussion();
		}

		return true;
	}

	bool setPitch(size_t lane, char pitch)
	{
		if (lane == 0 || lane > numTracks)
			return false;

		m_colors[lane - 1].setPitch(pitch);
		return true;
	}

	bool setLyric(size_t lane, const std::string& text)
	{
		if (lane == 0 || lane > numTracks)
			return false;

		m_colors[lane - 1].setLyric(text);
		return true;
	}

	void init_cht_single(const char* str)
	{
		// Read note
		int lane, count;
		if (sscanf_s(str, " %i%n", &lane, &count) != 1)
			throw EndofLineException();

		str += count;
		if (lane > numTracks)
			throw InvalidNoteException(lane);

		if (lane == 0)
			m_special.init();
		else
		{
			++str;
			char strBuf[256] = { 0 };
			if (sscanf_s(str, "%[^\"]s%n", &strBuf, 256, &count) == EOF)
				throw EndofLineException();

			m_colors[lane - 1].setLyric(strBuf);
			str += count + 1;

			// Read modifiers
			int numMods;
			if (*str && sscanf_s(str, " %i%n", &numMods, &count) == 1)
			{
				str += count;
				char modifier;
				for (int i = 0;
					i < numMods && sscanf_s(str, " %c%n", &modifier, 1, &count) == 1;
					++i)
				{
					str += count;
					switch (modifier)
					{
					case 'v':
					case 'V':
					{
						int pitch;
						uint32_t sustain = 0;
						if (sscanf_s(str, " %i %lu%n", &pitch, &sustain, &count) > 0)
						{
							m_colors[lane - 1].setPitch(pitch);
							m_colors[lane - 1].init(sustain);
							str += count;
						}
						else
							throw EndofLineException();
					}
					}
				}
				
			}
		}
	}

	// Used mainly for harmony vocals track
	void init_cht_chord(const char* str)
	{
		int colors;
		int count;
		if (sscanf_s(str, " %i%n", &colors, &count) != 1)
			throw EndofLineException();

		str += count;
		int i = 0;
		int numAdded = 0;
		int lane;
		while (i < colors && sscanf_s(str, " %i%n", &lane, &count) == 1)
		{
			if (lane != 0)
			{
				str += count + 2;
				char strBuf[256] = { 0 };
				if (sscanf_s(str, "%[^\"]s%n", &strBuf, 256, &count) == EOF)
					throw EndofLineException();

				if (lane <= numTracks)
				{
					m_colors[lane - 1].setLyric(strBuf);
					if (!m_special)
						++numAdded;
					else
						m_special = VocalPercussion();
				}
				str += count + 1;
			}
			else
			{
				m_special.init();
				numAdded = 1;
				for (auto& col : m_colors)
					col = Vocal();
				str += count;
			}
			++i;
		}

		if (numAdded == 0)
			throw InvalidNoteException();
	}

	bool modify(char modifier, bool toggle = true)
	{
		switch (modifier)
		{
		case 'n':
		case 'N':
			return m_special.modify('N');
		default:
			return false;
		}
	}

	bool modifyColor(int lane, char modifier) { return false; }
	
	void modify_cht(const char* str)
	{
		int numMods;
		int count;
		if (sscanf_s(str, " %i%n", &numMods, &count) == 1)
		{
			str += count;
			char modifier;
			for (int i = 0;
				i < numMods && sscanf_s(str, " %c%n", &modifier, 1, &count) == 1;
				++i)
			{
				str += count;
				switch (modifier)
				{
				case 'n':
				case 'N':
					m_special.modify('N');
				}
			}
		}
	}

	void vocalize_cht(const char* str)
	{
		int numPitches;
		int count;
		if (sscanf_s(str, " %i%n", &numPitches, &count) == 1)
		{
			str += count;
			int lane;
			int pitch;
			uint32_t sustain;

			int i = 0;
			while (i < numPitches && sscanf_s(str, " %i %i %lu%n", &lane, &pitch, &sustain, &count) == 3)
			{
				str += count;
				if (0 < lane && lane < numTracks)
				{
					m_colors[lane - 1].setPitch(pitch);
					m_colors[lane - 1].init(sustain);
				}
				++i;
			}

			if (i < numPitches)
				throw EndofLineException();
		}
	}

	void save_cht(const uint32_t position, std::fstream& outFile) const
	{
		int numActive = Note<numTracks, Vocal, VocalPercussion>::write_notes_cht(position, outFile, "\t");
		if (m_special)
		{
			if (m_special.m_noiseOnly)
			{
				outFile << " 1";
				m_special.save_modifier_cht(outFile);
			}
		}
		else if (numActive == 1)
		{
			for (const auto& col : m_colors)
				if (col)
				{
					if (col.isSung())
						col.save_pitch_cht(outFile);
					break;
				}
		}
		else
		{
			int numPitches = 0;
			for (const auto& col : m_colors)
				if (col && col.isSung())
					++numPitches;

			if (numPitches > 0)
			{
				outFile << "\n\t" << position << " = V " << numPitches;
				for (int i = 0; i < numTracks; ++i)
					if (m_colors[i] && m_colors[i].isSung())
						m_colors[i].save_pitch_cht(i + 1, outFile);
			}
		}
		
		outFile << '\n';
	}

	uint32_t getLongestSustain() const
	{
		uint32_t sustain = 0;
		if (!m_special)
			for (const auto& color : m_colors)
				if (color && color.getSustain() > sustain)
					sustain = color.getSustain();
		return sustain;
	}
};
