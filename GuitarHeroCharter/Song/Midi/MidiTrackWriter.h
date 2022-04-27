#pragma once
#include "../NodeTrack.h"
#include "MidiFile.h"

class MidiTrackWriter
{
	MidiFile::MidiChunk_Track m_events;
	bool m_doWrite = false;

public:
	MidiTrackWriter(const std::string& name);
	// Function that will be overriden by specializations
	template <typename N>
	void insertNoteEvents(const NodeTrack<N>& track);
	void writeToFile(std::fstream& outFile);
};

template<>
void MidiTrackWriter::insertNoteEvents(const NodeTrack<GuitarNote<5>>& track);

template<>
void MidiTrackWriter::insertNoteEvents(const NodeTrack<GuitarNote<6>>& track);

template<>
void MidiTrackWriter::insertNoteEvents(const NodeTrack<DrumNote>& track);
