#pragma once
#include "FileTraversal.h"

class TextTraversal : public Traversal
{
	class InvalidLyricExcpetion : std::runtime_error
	{
	public:
		InvalidLyricExcpetion() : std::runtime_error(" no valid lyric string could be extracted") {}
	};

	size_t m_lineCount = 0;

public:
	TextTraversal(const std::filesystem::path& path);
	bool next() override;
	void move(size_t count) override;
	void skipTrack() override;
	bool extract(uint32_t& value) override;
	bool extract(uint16_t& value) override;
	unsigned char extract() override;

	void skipWhiteSpace();
	void skipEqualsSign();

	std::string_view extractText();
	std::string extractLyric();
	const char* getCurrent() { return (const char*)m_current; }

	size_t getLineNumber() const { return m_lineCount; }
	size_t getLineLength() const { return m_next - m_current; }

	bool operator==(char c) const { return *m_current == c; }
	bool operator!=(char c) const { return *m_current != c; }
};
