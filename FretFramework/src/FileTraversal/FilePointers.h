#pragma once
#include <filesystem>

class FilePointers
{
	size_t m_fileSize;
	std::shared_ptr<unsigned char[]> m_fileData;

public:
	FilePointers(const std::filesystem::path& path);

	const unsigned char* begin() const noexcept { return m_fileData.get(); }
	constexpr size_t size() const noexcept { return m_fileSize; }
	const unsigned char* end() const noexcept { return m_fileData.get() + m_fileSize; }
};
