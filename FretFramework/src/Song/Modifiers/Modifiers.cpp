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

template<>
void WritableModifier<std::string>::write_ini(std::fstream& outFile) const
{
	if (!m_value.empty())
		outFile << m_name << " = " << m_value << '\n';
}

bool BooleanModifier::read(TextTraversal& traversal)
{
	if (isReadable(traversal))
	{
		m_isActive = true;
		switch (traversal.getCurrent()[0])
		{
		case '0':
			m_value = false;
			break;
		case '1':
			m_value = true;
			break;
		default:
			std::string_view str = traversal.extractText();
			m_value = str.length() >= 4 &&
				std::tolower(str[0]) == 't' &&
				std::tolower(str[1]) == 'r' &&
				std::tolower(str[2]) == 'u' &&
				std::tolower(str[3]) == 'e';

			break;
		}
		return true;
	}
	return false;
}

void BooleanModifier::write(std::fstream& outFile, bool writeOverride) const
{
	if (writeOverride || m_isActive)
		outFile << '\t' << m_name << " = " << std::boolalpha << m_value << '\n';
}

void BooleanModifier::write_ini(std::fstream& outFile, bool writeOverride) const
{
	if (writeOverride || m_isActive)
		outFile << m_name << " = " << std::boolalpha << m_value << '\n';
}
