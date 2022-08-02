#pragma once
#include "FileTraversal.h"
#include "Variable Types/UnicodeString.h"

class TextTraversal : public Traversal
{
	class InvalidLyricExcpetion : std::runtime_error
	{
	public:
		InvalidLyricExcpetion() : std::runtime_error(" no valid lyric string could be extracted") {}
	};

	size_t m_lineCount = 0;
	std::string_view m_trackName;
	uint32_t m_position = 0;

public:
	TextTraversal(const std::filesystem::path& path);
	bool next() override;
	void skipTrack() override;

	void setTrackName();
	bool isTrackName(const char* str) const;
	bool cmpTrackName(const std::string_view& str);
	std::string getTrackName() const;

	unsigned char extractChar();
	bool extract(unsigned char& value);
	uint32_t extractU32();
	bool extract(uint32_t& value);
	uint16_t extractU16();
	bool extract(uint16_t& value);
	void skipWhiteSpace();
	void skipEqualsSign();
	void move(size_t count);

	constexpr void resetPosition() { m_position = 0; }
	uint32_t extractPosition();

	std::u32string extractText(bool isIniFile = false);
	std::u32string extractLyric();
	const char* getCurrent() { return (const char*)m_current; }

	size_t getLineNumber() const { return m_lineCount; }

	bool operator==(char c) const { return *m_current == c; }
	bool operator!=(char c) const { return *m_current != c; }

	// Used only for metadata modifiers
	void extract(int16_t& value);
	void extract(int32_t& value);
	void extract(float& value);
	void extract(bool& value);
	void extract(UnicodeString& str);
	void extract(float(&arr)[2]);
};
