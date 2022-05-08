#include "Modifiers.h"

template<>
bool WritableModifier<float>::read(TextTraversal& traversal)
{
	if (isReadable(traversal))
	{
		m_value = strtof(traversal.getCurrent(), nullptr);
		return true;
	}
	return false;
}

template<>
bool WritableModifier<std::string>::read(TextTraversal& traversal)
{
	if (isReadable(traversal))
	{
		m_value = std::string(traversal.extractText());
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
