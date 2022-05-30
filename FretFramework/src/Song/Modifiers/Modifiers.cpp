#include "Modifiers.h"

bool StringModifier::read(TextTraversal& traversal)
{
	if (isReadable(traversal))
	{
		m_value = std::string(traversal.extractText());
		return true;
	}
	return false;
}

void StringModifier::write(std::fstream& outFile) const
{
	if (!m_value.empty())
		outFile << '\t' << m_name << " = \"" << m_value << "\"\n";
}

void StringModifier::write_ini(std::fstream& outFile) const
{
	if (!m_value.empty())
		outFile << m_name << " = " << m_value << '\n';
}

void StringModifier::reset() { m_value.clear(); }

bool StringModifier::read_ini(TextTraversal& traversal)
{
	if (isReadable(traversal))
	{
		m_value = std::string(traversal.extractText(false));
		return true;
	}
	return false;
}

template<>
bool NumberModifier<float>::read(TextTraversal& traversal)
{
	if (isReadable(traversal))
	{
		m_value = strtof(traversal.getCurrent(), nullptr);
		return true;
	}
	return false;
}

bool NumberModifier<float[2]>::read(TextTraversal& traversal)
{
	if (TxtFileModifier<float[2]>::isReadable(traversal))
	{
		char* next;
		m_value[0] = strtof(traversal.getCurrent(), &next);
		if (next)
			m_value[1] = strtof(next + 1, nullptr);
		return true;
	}
	return false;
}

void NumberModifier<float[2]>::write(std::fstream& outFile) const
{
	if (m_value[0] || m_value[1])
		outFile << '\t' << m_name << " = " << m_value[0] << ' ' << m_value[1] << '\n';
}

void NumberModifier<float[2]>::write_ini(std::fstream& outFile) const
{
	if (m_value[0] || m_value[1])
		outFile << m_name << " = " << m_value[0] << ' ' << m_value[1] << '\n';
}

void NumberModifier<float[2]>::reset() { m_value[0] = m_value[1] = 0; }

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
				(str[0] == 't' || str[0] == 'T') &&
				(str[1] == 'r' || str[1] == 'R') &&
				(str[2] == 'u' || str[2] == 'U') &&
				(str[3] == 'e' || str[3] == 'E');

			break;
		}
		return true;
	}
	return false;
}

void BooleanModifier::write(std::fstream& outFile) const
{
	if (m_isActive)
		outFile << '\t' << m_name << " = " << std::boolalpha << m_value << '\n';
}

void BooleanModifier::write_ini(std::fstream& outFile) const
{
	if (m_isActive)
		outFile << m_name << " = " << std::boolalpha << m_value << '\n';
}

void BooleanModifier::reset() { m_value = false; m_isActive = false; }
