#pragma once
#include "InstrumentalTrack.h"
#include "Difficulty/Difficulty_bch.hpp"

template <class T>
void InstrumentalTrack_Scan<T>::scan_bch(BCHTraversal& traversal)
{
	traversal.move(1);
	while (traversal.canParseNewChunk() && traversal.validateChunk("DIFF"))
	{
		if (traversal.doesNextTrackExist() &&
			!traversal.checkNextChunk("DIFF") &&
			!traversal.checkNextChunk("ANIM") &&
			!traversal.checkNextChunk("INST") &&
			!traversal.checkNextChunk("VOCL"))
		{
			// Sets the next track to whatever next valid track comes first, if any exist

			const unsigned char* const diff = traversal.findNextChunk("DIFF");
			const unsigned char* const anim = traversal.findNextChunk("ANIM");
			const unsigned char* const inst = traversal.findNextChunk("INST");
			const unsigned char* const vocl = traversal.findNextChunk("VOCL");
			if (diff && (!anim || diff < anim) && (!inst || diff < inst) && (!vocl || diff < vocl))
				traversal.setNextTrack(diff);
			else if (anim && (!inst || anim < inst) && (!vocl || anim < vocl))
				traversal.setNextTrack(anim);
			else if (inst && (!vocl || inst < vocl))
				traversal.setNextTrack(inst);
			else
				traversal.setNextTrack(vocl);
		}

		unsigned char diff = traversal.getTrackID();
		// Scanning only takes *playable* notes into account, so BRE can be ignored
		if (diff < 4)
		{
			if (m_difficulties[diff].scan_bch(traversal))
				m_scanValue |= 1 << diff;
		}
		else
			traversal.skipTrack();
	}

	if (traversal.canParseNewChunk() && traversal.validateChunk("ANIM"))
	{
		if (traversal.doesNextTrackExist() && !traversal.checkNextChunk("INST") && !traversal.checkNextChunk("VOCL"))
		{
			// Sets the next track to whatever next valid track comes first, if any exist

			const unsigned char* const inst = traversal.findNextChunk("INST");
			const unsigned char* const vocl = traversal.findNextChunk("VOCL");
			if (inst && (!vocl || inst < vocl))
				traversal.setNextTrack(inst);
			else
				traversal.setNextTrack(vocl);
		}
		traversal.skipTrack();
	}
}

template <class T>
void InstrumentalTrack<T>::load_bch(BCHTraversal& traversal)
{
	traversal.move(1);
	while (traversal.canParseNewChunk() && traversal.validateChunk("DIFF"))
	{
		if (traversal.doesNextTrackExist() &&
			!traversal.checkNextChunk("DIFF") &&
			!traversal.checkNextChunk("ANIM") &&
			!traversal.checkNextChunk("INST") &&
			!traversal.checkNextChunk("VOCL"))
		{
			// Sets the next track to whatever next valid track comes first, if any exist

			const unsigned char* const diff = traversal.findNextChunk("DIFF");
			const unsigned char* const anim = traversal.findNextChunk("ANIM");
			const unsigned char* const inst = traversal.findNextChunk("INST");
			const unsigned char* const vocl = traversal.findNextChunk("VOCL");
			if (diff && (!anim || diff < anim) && (!inst || diff < inst) && (!vocl || diff < vocl))
				traversal.setNextTrack(diff);
			else if (anim && (!inst || anim < inst) && (!vocl || anim < vocl))
				traversal.setNextTrack(anim);
			else if (inst && (!vocl || inst < vocl))
				traversal.setNextTrack(inst);
			else
				traversal.setNextTrack(vocl);
		}

		unsigned char diff = traversal.getTrackID();
		if (diff < 5)
			m_difficulties[diff].load_bch(traversal);
		else
			traversal.skipTrack();
	}

	if (traversal.canParseNewChunk() && traversal.validateChunk("ANIM"))
	{
		if (traversal.doesNextTrackExist() && !traversal.checkNextChunk("INST") && !traversal.checkNextChunk("VOCL"))
		{
			// Sets the next track to whatever next valid track comes first, if any exist

			const unsigned char* const inst = traversal.findNextChunk("INST");
			const unsigned char* const vocl = traversal.findNextChunk("VOCL");
			if (inst && (!vocl || inst < vocl))
				traversal.setNextTrack(inst);
			else
				traversal.setNextTrack(vocl);
		}
		traversal.skipTrack();
	}
}

template <class T>
bool InstrumentalTrack<T>::save_bch(std::fstream& outFile) const
{
	if (!occupied())
		return false;

	outFile.write("INST", 4);

	auto start = outFile.tellp();
	uint32_t length = 4;
	outFile.write((char*)&length, 4);
	outFile.put(m_instrumentID);
	outFile.put(0);

	char numDiffs = 0;
	for (int diff = 4; diff >= 0; --diff)
		if (m_difficulties[diff].occupied())
		{
			m_difficulties[diff].save_bch(outFile);
			++numDiffs;
		}

	outFile.write("ANIM", 4);
	outFile.write((char*)&length, 4);
	uint32_t animEventCount = 0;
	outFile.write((char*)&animEventCount, 4);

	auto end = outFile.tellp();
	length = uint32_t(end - start) - 4;
	outFile.seekp(start);
	outFile.write((char*)&length, 4);
	outFile.put(m_instrumentID);
	outFile.put(numDiffs);
	outFile.seekp(end);
	return true;
}
