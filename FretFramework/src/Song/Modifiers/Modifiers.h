#pragma once
#include <sstream>
#include <fstream>
template <class T>
class WritableModifier
{
	std::string_view m_name;
	T m_default;
public:
	T m_value;

	WritableModifier(const char* str, T value = T(), T def = T())
		: m_name(str)
		, m_value(value)
		, m_default(def) {}

	bool read(const std::string& name, std::stringstream& ss)
	{
		if (name.find(m_name) != std::string::npos)
		{
			ss >> m_value;
			return true;
		}
		return false;
	}

	bool isWritable() const { return m_value != m_default; }

	void write(std::fstream& outFile) const
	{
		if (isWritable())
			outFile << '\t' << m_name << " = " << m_value << '\n';
	}

	T& operator=(const T& value)
	{
		m_value = value;
		return m_value;
	}

	void reset() { m_value = m_default; }
	void setDefault(T def)
	{
		if (!isWritable())
			m_value = def;
		m_default = def;
	}

	T& operator*=(float multiplier)
	{
		m_default = T(m_default * multiplier);
		return m_value = T(m_value * multiplier);
	}
	operator T() { return m_value; }
};

template<>
bool WritableModifier<std::string>::read(const std::string& name, std::stringstream& ss);

template<>
void WritableModifier<std::string>::write(std::fstream& outFile) const;

template<typename T>
std::istream& operator>>(std::istream& ss, WritableModifier<T> mod)
{
	return ss >> mod.m_value;
}
