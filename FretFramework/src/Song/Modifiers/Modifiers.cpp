#include "Modifiers.h"

void StringModifier::write(std::fstream& outFile) const
{
	outFile << '\t' << m_name << " = \"" << m_string << "\"\n";
}

void StringModifier::write_ini(std::fstream& outFile) const
{
	outFile << m_name << " = " << m_string << '\n';
}

void FloatArrayModifier::write(std::fstream& outFile) const
{
	outFile << '\t' << m_name << " = " << m_floats[0] << ' ' << m_floats[1] << '\n';
}

void FloatArrayModifier::write_ini(std::fstream& outFile) const
{
	outFile << m_name << " = " << m_floats[0] << ' ' << m_floats[1] << '\n';
}

void BooleanModifier::write(std::fstream& outFile) const
{
	outFile << '\t' << m_name << " = " << std::boolalpha << m_boolean << '\n';
}

void BooleanModifier::write_ini(std::fstream& outFile) const
{
	outFile << m_name << " = " << std::boolalpha << m_boolean << '\n';
}
