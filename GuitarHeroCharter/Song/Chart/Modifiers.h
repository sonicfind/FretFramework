#pragma once
#include <sstream>
#include <fstream>
template <class T>
class WritableModifier
{
	std::string_view m_name;
public:
	T m_value{};

	WritableModifier(const char* str) : m_name(str) {}

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
		outFile << "  " << m_name << " = " << m_value << '\n';
	}

	void set(const T& value) { m_value = value; }
	void reset() { m_value = T(); }
};

template<>
bool WritableModifier<std::string>::read(const std::string& name, std::stringstream& ss);

template<>
void WritableModifier<std::string>::write(std::fstream& outFile) const;
