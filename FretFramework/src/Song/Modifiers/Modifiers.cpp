#include "Modifiers.h"

void TxtFileModifier::write_cht(std::fstream& outFile) const
{
	if (m_type == Type::STRING_NOCASE)
		outFile << '\t' << m_name << " = \"" << UnicodeString::U32ToStr(cast<std::u32string>()) << "\"\n";
}

void TxtFileModifier::write_ini(std::fstream& outFile) const
{
	switch (m_type)
	{
	case Type::STRING:        writeVal<UnicodeString >(outFile); break;
	case Type::STRING_NOCASE: writeVal<std::u32string>(outFile); break;
	case Type::UINT32:        writeVal<uint32_t      >(outFile); break;
	case Type::INT32:         writeVal<int32_t       >(outFile); break;
	case Type::UINT16:        writeVal<uint16_t      >(outFile); break;
	case Type::BOOL:          writeVal<bool          >(outFile); break;
	case Type::FLOAT:         writeVal<float         >(outFile); break;
	case Type::FLOATARRAY:    writeVal<FloatArray    >(outFile); break;
	}
}
