#include "Modifiers.h"

void TxtFileModifier::write(std::fstream& outFile) const
{
	outFile << '\t' << m_name << " = ";
	std::visit([&outFile](auto&& arg) {
		using T = std::decay_t<decltype(arg)>;
		if      constexpr (std::is_same_v<T, UnicodeString>) outFile << '\"' << arg << "\"\n";
		else if constexpr (std::is_same_v<T, bool>)          outFile << std::boolalpha << arg << '\n';
		else if constexpr (std::is_same_v<T, FloatArray>)    outFile << arg[0] << ' ' << arg[1] << '\n';
		else                                                 outFile << arg << '\n';
		}, m_value);
}

void TxtFileModifier::write_ini(std::fstream& outFile) const
{
	outFile << m_name << " = ";
	std::visit([&outFile](auto&& arg) {
		using T = std::decay_t<decltype(arg)>;
		if      constexpr (std::is_same_v<T, bool>)       outFile << std::boolalpha << arg << '\n';
		else if constexpr (std::is_same_v<T, FloatArray>) outFile << arg[0] << ' ' << arg[1] << '\n';
		else                                              outFile << arg << '\n';
		}, m_value);
}
