#include "Modifiers.h"

void StringModifier::read(TextTraversal& traversal)
{
	m_string = traversal.extractText(m_isIniModifier);
}

void StringModifier::write(std::fstream& outFile) const
{
	if (!m_string->empty())
		outFile << '\t' << m_name << " = \"" << m_string << "\"\n";
}

void StringModifier::write_ini(std::fstream& outFile) const
{
	if (!m_string->empty())
		outFile << m_name << " = " << m_string << '\n';
}

void FloatArrayModifier::read(TextTraversal& traversal)
{
	if (traversal.extract(m_floats[0]))
		traversal.extract(m_floats[1]);
}

void FloatArrayModifier::write(std::fstream& outFile) const
{
	if (m_floats[0] || m_floats[1])
		outFile << '\t' << m_name << " = " << m_floats[0] << ' ' << m_floats[1] << '\n';
}

void FloatArrayModifier::write_ini(std::fstream& outFile) const
{
	if (m_floats[0] || m_floats[1])
		outFile << m_name << " = " << m_floats[0] << ' ' << m_floats[1] << '\n';
}

void BooleanModifier::read(TextTraversal& traversal)
{
	m_boolean = traversal.extractBoolean();
	m_isActive = true;
}

void BooleanModifier::write(std::fstream& outFile) const
{
	if (m_isActive)
		outFile << '\t' << m_name << " = " << std::boolalpha << m_boolean << '\n';
}

void BooleanModifier::write_ini(std::fstream& outFile) const
{
	if (m_isActive)
		outFile << m_name << " = " << std::boolalpha << m_boolean << '\n';
}
