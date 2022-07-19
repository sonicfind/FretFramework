#include "Modifiers.h"

void StringModifier::write(std::fstream& outFile) const
{
	if (!m_value->empty())
		outFile << '\t' << m_name << " = \"" << m_value << "\"\n";
}

void StringModifier::write_ini(std::fstream& outFile) const
{
	if (!m_value->empty())
		outFile << m_name << " = " << m_value << '\n';
}

void StringModifier::reset() { m_value->clear(); }

bool StringModifier::read_ini(TextTraversal& traversal)
{
	if (isReadable(traversal))
	{
		m_value = std::move(traversal.extractText(false));
		m_value.setCasedStrings();
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
	if (TxtFileModifier::read(traversal))
	{
		m_isActive = true;
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
