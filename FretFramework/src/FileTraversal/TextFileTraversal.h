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

public:
	TextTraversal(const std::filesystem::path& path);
	bool next() override;
	void move(size_t count) override;
	void skipTrack() override;
	unsigned char extractChar() override;
	bool extract(unsigned char& value) override;

	void setTrackName();
	bool isTrackName(const char* str) const;
	bool cmpTrackName(const std::string_view& str);
	std::string getTrackName() const;
	uint32_t extractU32();
	bool extract(uint32_t& value);
	uint16_t extractU16();
	bool extract(uint16_t& value);
	void skipWhiteSpace();
	void skipEqualsSign();

	std::string extractText(bool checkForQuotes = true);
	std::string extractLyric();
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
