#pragma once
#include "SongBase.h"

class SongScan : public SongBase
{
	std::unique_ptr<NoteTrack_Scan> m_noteTrackScans[11] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	std::filesystem::file_time_type m_last_modified;

public:
	void scan(const std::filesystem::path& chartPath);
	void scan_full(const std::filesystem::path& chartPath, const std::filesystem::path& iniPath, const std::vector<std::filesystem::path>& audioFiles);

	bool isValid() const;

private:
	void finalizeScan(const std::vector<std::filesystem::path>& audioFiles);
	void scanFile_Cht();
	void scanFile_Bch();
	void scanFile_Midi();
};
