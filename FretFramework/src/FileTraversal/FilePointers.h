#pragma once
#include <filesystem>

class FilePointers
{
	size_t m_fileSize;
	unsigned char* m_fileData;

public:
	explicit FilePointers(const std::filesystem::path& path);
	~FilePointers();

	const unsigned char* begin() const noexcept { return m_fileData; }
	constexpr size_t length() const noexcept { return m_fileSize; }
	const unsigned char* end() const noexcept { return m_fileData + m_fileSize; }
};
