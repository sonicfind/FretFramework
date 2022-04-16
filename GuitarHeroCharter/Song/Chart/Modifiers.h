#pragma once
#include <sstream>
#include <fstream>
template <class T>
class WritableModifier
{
	std::string_view m_name;
public:
	T m_value;

	WritableModifier(const char* str, T value = T()) : m_name(str), m_value(value) {}

	bool read(const std::string& name, std::stringstream& ss)
	{
		if (name.find(m_name) != std::string::npos)
		{
			ss >> m_value;
			return true;
		}
		return false;
	}

	void write(std::fstream& outFile) const
	{
		if (m_value)
			outFile << "  " << m_name << " = " << m_value << '\n';
	}
	T& operator=(const T& value)
	{
		m_value = value;
		return m_value;
	}
	void reset() { m_value = T(); }
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
