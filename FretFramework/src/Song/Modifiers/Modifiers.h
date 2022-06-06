#pragma once
#include <fstream>
#include "FileTraversal/TextFileTraversal.h"
#include <assert.h>

template <class T>
class TxtFileModifier
{
protected:
	std::string_view m_name;

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
	T m_value = {};

	TxtFileModifier(const char* str)
		: m_name(str) {}

	TxtFileModifier(const char* str, T value)
		: m_name(str)
		, m_value(value) {}

	bool read(TextTraversal& traversal)
	{
		if (isReadable(traversal))
		{
			traversal.extract(m_value);
			return true;
		}
		return false;
	}

	virtual void write(std::fstream& outFile) const = 0;
	virtual void write_ini(std::fstream& outFile) const = 0;
	virtual void reset() = 0;
};

class StringModifier : public TxtFileModifier<std::string>
{
public:
	using TxtFileModifier::TxtFileModifier;
	void write(std::fstream& outFile) const override;
	void write_ini(std::fstream& outFile) const override;
	void reset() override;
	bool read_ini(TextTraversal& traversal);

	std::string& operator=(const std::string& value)
	{
		m_value = value;
		return m_value;
	}

	operator std::string&() { return m_value; }
};

template <typename T>
class NumberModifier : public TxtFileModifier<T>
{
public:
	using TxtFileModifier<T>::m_value;
	using TxtFileModifier<T>::m_name;
private:
	T m_default;

	bool isWritable() const { return m_value != m_default; }
public:

	NumberModifier(const char* str, T value = T(), T def = T())
		: TxtFileModifier<T>(str, value)
		, m_default(value) {}

	void write(std::fstream& outFile) const override
	{
		if (isWritable())
			outFile << '\t' << m_name << " = " << m_value << '\n';
	}

	void write_ini(std::fstream& outFile) const override
	{
		if (isWritable())
			outFile << m_name << " = " << m_value << '\n';
	}

	void reset() override { m_value = m_default; }

	T& operator=(const T& value)
	{
		m_value = value;
		return m_value;
	}

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

template <>
class NumberModifier<float[2]> : public TxtFileModifier<float[2]>
{
	using TxtFileModifier<float[2]>::m_value;
public:
	NumberModifier(const char* str)
		: TxtFileModifier<float[2]>(str) {}

	void write(std::fstream& outFile) const override;
	void write_ini(std::fstream& outFile) const override;
	void reset() override;
	float& operator[](size_t i)
	{
		assert(i < 2);
		return m_value[i];
	}
};

class BooleanModifier : public TxtFileModifier<bool>
{
	bool m_isActive = false;
public:
	BooleanModifier(const char* str, bool active = false)
		: TxtFileModifier(str, false)
		, m_isActive(active) {}

	void write(std::fstream& outFile) const override;
	void write_ini(std::fstream& outFile) const override;
	void reset() override;

	bool& operator=(const bool& value)
	{
		m_value = value;
		m_isActive = true;
		return m_value;
	}

	bool isActive() const { return m_isActive; }
	void deactivate() { m_isActive = false; }
	operator bool() const { return m_value; }
};
