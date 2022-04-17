#include "MidiFile.h"
#include <iostream>
#include <string>

namespace MidiFile
{
	VariableLengthQuantity::InvalidIntegerException::InvalidIntegerException(uint32_t value)
		: std::exception(("Integer value cannot exceed 134217728 (value: " + std::to_string(value) + ")").c_str()) {}

	VariableLengthQuantity::VariableLengthQuantity(const unsigned char*& bufferPtr)
		: m_value(0)
	{
		uint32_t ins = 0;
		do
		{
			m_value <<= 7;
			ins = (uint32_t)*bufferPtr++;
			m_value |= ins & 127;
		} while (ins >= 128);
	}

	VariableLengthQuantity::VariableLengthQuantity(uint32_t value)
	{
		operator=(value);
	}

	void VariableLengthQuantity::writeToFile(std::fstream& outFile) const
	{
		if (m_value & (127 << 21))
			goto Char4;
		else if (m_value & (127 << 14))
			goto Char3;
		else if (m_value & (127 << 7))
			goto Char2;
		else
			goto Char1;

	Char4:
		outFile << (char)(((m_value >> 21) & 127) + 128);
	Char3:
		outFile << (char)(((m_value >> 14) & 127) + 128);
	Char2:
		outFile << (char)(((m_value >> 7) & 127) + 128);
	Char1:
		outFile << (char)(m_value & 127);
	}

	VariableLengthQuantity& VariableLengthQuantity::operator=(uint32_t value)
	{
		if (value & (15 << 28))
			throw InvalidIntegerException(value);

		m_value = value;
		return *this;
	}

	VariableLengthQuantity::operator uint32_t() const
	{
		return m_value;
	}
}
