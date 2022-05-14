#pragma once
#include <filesystem>
#include <string_view>
#include "FilestreamCheck.h"

template <typename T = char>
class Traversal
{
protected:
	T* m_file;
	const T* m_end;
	const T* m_current;
	const T* m_next;

	Traversal(const std::filesystem::path& path)
	{
		FILE* inFile = FilestreamCheck::getFile(path, L"rb");
		fseek(inFile, 0, SEEK_END);
		size_t length = ftell(inFile);
		fseek(inFile, 0, SEEK_SET);

		m_file = new T[length + 1]();
		m_end = m_file + length;
		fread(m_file, sizeof(T), length, inFile);
		fclose(inFile);

		m_current = m_file;
	}

public:
	virtual void next() = 0;
	virtual void skipTrack() = 0;
	virtual std::string_view extractText() = 0;
	virtual bool extractUInt(uint32_t& value) = 0;

	virtual ~Traversal()
	{
		delete[] m_file;
	}
};
