#include "Modifiers.h"

void TxtFileModifier::write(std::fstream& outFile) const
{
	const auto writeValue = [&outFile](const auto& arg)
	{
		using T = std::decay_t<decltype(arg)>;
		if      constexpr (std::is_same_v<T, UnicodeString>)  outFile << '\"' << arg << "\"\n";
		else if constexpr (std::is_same_v<T, std::u32string>) outFile << '\"' << UnicodeString::U32ToStr(arg) << "\"\n";
		else if constexpr (std::is_same_v<T, bool>)           outFile << std::boolalpha << arg << '\n';
		else if constexpr (std::is_same_v<T, FloatArray>)     outFile << arg[0] << ' ' << arg[1] << '\n';
		else                                                  outFile << arg << '\n';
	};

	outFile << '\t' << m_name << " = ";
	switch (m_type)
	{
	case Type::STRING_NOCASE: writeValue(*cast<std::u32string>()); break;
	case Type::INT32:         writeValue(*cast<int32_t      >()); break;
	case Type::UINT16:        writeValue(*cast<uint16_t     >()); break;
	case Type::FLOAT:         writeValue(*cast<float        >()); break;
	}
}

void TxtFileModifier::write_ini(std::fstream& outFile) const
{
	const auto writeValue = [&outFile](const auto& arg)
	{
		using T = std::decay_t<decltype(arg)>;
		if      constexpr (std::is_same_v<T, bool>)           outFile << std::boolalpha << arg << '\n';
		else if constexpr (std::is_same_v<T, FloatArray>)     outFile << arg[0] << ' ' << arg[1] << '\n';
		else if constexpr (std::is_same_v<T, std::u32string>) outFile << UnicodeString::U32ToStr(arg) << '\n';
		else                                                  outFile << arg << '\n';
	};

	outFile << m_name << " = ";
	switch (m_type)
	{
	case Type::STRING:        writeValue(*cast<UnicodeString>()); break;
	case Type::STRING_NOCASE: writeValue(*cast<std::u32string>()); break;
	case Type::UINT32:        writeValue(*cast<uint32_t     >()); break;
	case Type::INT32:         writeValue(*cast<int32_t      >()); break;
	case Type::UINT16:        writeValue(*cast<uint16_t     >()); break;
	case Type::BOOL:          writeValue(*cast<bool         >()); break;
	case Type::FLOAT:         writeValue(*cast<float        >()); break;
	case Type::FLOATARRAY:    writeValue(*cast<FloatArray   >()); break;
	}
}
