#pragma once
#include "FileTraversal.h"

class TextTraversal : public Traversal<char>
{
	size_t m_lineCount = 0;

public:
	TextTraversal(const std::filesystem::path& path);
	void next() override;
	void skipTrack() override;
	std::string_view extractText() override;
	bool extractUInt(uint32_t& value) override;

	void skipWhiteSpace();
	void skipEqualsSign();
	void move(size_t count);

	const char* getCurrent() { return m_current; }
	const char extractChar()
	{ 
		char c = *m_current++;
		skipWhiteSpace();
		return c;
	}

	size_t getLineNumber() const { return m_lineCount; }
	size_t getLineLength() const { return m_next - m_current; }

	bool operator==(char c) const { return *m_current == c; }
	bool operator!=(char c) const { return *m_current != c; }
	operator bool() const { return m_current && m_current < m_end; }
};
