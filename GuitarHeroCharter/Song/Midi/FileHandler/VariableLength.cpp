#include "MidiFile.h"
#include <iostream>

namespace MidiFile
{
	const char* VariableLengthQuantity::InvalidIntegerException::what() const
	{
		return "Integer value cannot exceed 134217728";
	}

	VariableLengthQuantity::VariableLengthQuantity(std::fstream& inFile)
		: m_value(0)
		, m_size(0)
	{
		unsigned char ins = 0;
		do
		{
			++m_size;
			m_value <<= 7;
			inFile.read((char*)&ins, 1);
			m_value += (int)ins & 127;
		} while (ins >= 128);
	}

	VariableLengthQuantity::VariableLengthQuantity(uint32_t value)
	{
		operator=(value);
	}

	void VariableLengthQuantity::writeToFile(std::fstream& outFile) const
	{
		unsigned char ins[4] =
		{
			((m_value >> 21) & 127) + 128,
			((m_value >> 14) & 127) + 128,
			((m_value >> 7) & 127) + 128,
			m_value & 127,
		};

		switch (m_size)
		{
		case 4:
			outFile << (char)(((m_value >> 21) & 127) + 128);
			__fallthrough;
		case 3:
			outFile << (char)(((m_value >> 14) & 127) + 128);
			__fallthrough;
		case 2:
			outFile << (char)(((m_value >> 7) & 127) + 128);
			__fallthrough;
		default:
			outFile << (char)(m_value & 127);
		}
	}

	VariableLengthQuantity& VariableLengthQuantity::operator=(uint32_t value)
	{
		if (value & (15 << 28))
			throw InvalidIntegerException();

		m_value = value;
		if (m_value & (127 << 21))
			m_size = 4;
		else if (m_value & (127 << 14))
			m_size = 3;
		else if (m_value & (127 << 7))
			m_size = 2;
		else
			m_size = 1;
		return *this;
	}

	VariableLengthQuantity::operator uint32_t() const
	{
		return m_value;
	}
}
