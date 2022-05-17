#pragma once
#include "BasicTrack.h"
#include "Difficulty/Difficulty_bch.hpp"

template <class T>
void BasicTrack<T>::load_bch(const unsigned char* current, const unsigned char* const end)
{
	static struct {
		char tag[4] = {};
		uint32_t length = 0;
	} chunk;

	if (current == end)
		return;

	const unsigned char diffCount = *current++;
	for (int i = 0; current < end && i < diffCount; ++i)
	{
		memcpy(&chunk, current, sizeof(chunk));
		if (strncmp(chunk.tag, "DIFF", 4) != 0)
			break;

		current += sizeof(chunk);
		const unsigned char* next = current + chunk.length;

		if (next > end)
			next = end;

		int diff = *current++;
		if (diff < 4)
			m_difficulties[diff].load_bch(current, next);
		else
			m_difficulties[4].load_bch(current, next);

		current = next;
	}

	if (current + 8 < end)
	{
		memcpy(&chunk, current, sizeof(chunk));
		if (strncmp(chunk.tag, "ANIM", 4) == 0)
		{
			current += sizeof(chunk);
			const unsigned char* next = current + chunk.length;

			if (next > end)
				next = end;

			// Insert implementation here

			current = next;
		}
	}
}

template <class T>
bool BasicTrack<T>::save_bch(std::fstream& outFile) const
{
	if (!occupied())
		return false;

	outFile.write("INST", 4);
	uint32_t length = 0;
	auto start = outFile.tellp();
	outFile.write((char*)&length, 4);
	outFile.put(m_instrumentID);
	char numDiffs = 0;
	outFile.put(numDiffs);

	for (int diff = 4; diff >= 0; --diff)
		if (m_difficulties[diff].occupied())
		{
			m_difficulties[diff].save_bch(outFile);
			++numDiffs;
		}

	outFile.write("ANIM", 4);
	length = 4;
	outFile.write((char*)&length, 4);
	uint32_t count = 0;
	outFile.write((char*)&count, 4);

	auto end = outFile.tellp();
	length = uint32_t(end - start - 4);
	outFile.seekp(start);
	outFile.write((char*)&length, 4);
	outFile.put(m_instrumentID);
	outFile.put(numDiffs);
	outFile.seekp(end);
	return true;
}
