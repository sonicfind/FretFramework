#pragma once
#include <filesystem>

struct FilePointers
{
	std::filesystem::path m_path;
	unsigned char* m_file;
	const unsigned char* m_end;

	FilePointers(const std::filesystem::path& path);
	~FilePointers();
};
