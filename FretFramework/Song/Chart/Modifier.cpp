#include "Modifiers.h"
template<>
bool WritableModifier<std::string>::read(const std::string& name, std::stringstream& ss)
{
	if (name.find(m_name) != std::string::npos)
	{
		ss.ignore(1, ' ');
		std::getline(ss, m_value);
		m_value = m_value.substr(1, m_value.length() - 2);
		return true;
	}
	return false;
}

template<>
void WritableModifier<std::string>::write(std::fstream& outFile) const
{
	if (!m_value.empty())
		outFile << '\t' << m_name << " = \"" << m_value << "\"\n";
}
