#pragma once
#include <fstream>
#include "FileTraversal/TextFileTraversal.h"
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

	bool read(TextTraversal& traversal)
	{
		if (isReadable(traversal))
		{
			m_value = (T)strtol(traversal.getCurrent(), nullptr, 0);
			return true;
		}
		return false;
	}

private:
	bool isReadable(TextTraversal& traversal)
	{
		size_t length = m_name.length();
		if (strncmp(traversal.getCurrent(), m_name.data(), length) == 0 &&
			(traversal.getCurrent()[length] == ' ' || traversal.getCurrent()[length] == '='))
		{
			traversal.move(length);
			traversal.skipEqualsSign();
			return true;
		}
		return false;
	}

public:

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
bool WritableModifier<float>::read(TextTraversal& traversal);

template<>
bool WritableModifier<std::string>::read(TextTraversal& traversal);

template<>
void WritableModifier<std::string>::write(std::fstream& outFile) const;
