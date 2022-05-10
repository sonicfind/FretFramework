#pragma once
#include "NoteExceptions.h"
#include "FilestreamCheck.h"
#include <filesystem>
#include <string_view>

class TextTraversal
{
	const char* m_file;
	const char* m_end;
	const char* m_current;
	const char* m_next;
	size_t m_lineCount = 0;
	size_t m_textLength = 0;
public:
	TextTraversal(const std::filesystem::path& path);
	void skipWhiteSpace();
	void nextLine();
	void skipScope();
	void move(size_t count);
	void skipEqualsSign();
	std::string_view extractText();
	bool extractUInt(uint32_t& value);
	~TextTraversal();

	const char* getCurrent() { return m_current; }
	const char getChar() const { return *m_current; }
	size_t getLineNumber() const { return m_lineCount; }
	size_t getLineLength() const { return m_next - m_current; }

	bool operator==(char c) const { return *m_current == c; }
	bool operator!=(char c) const { return *m_current != c; }
	operator bool() const { return m_current && m_current < m_end; }
};
