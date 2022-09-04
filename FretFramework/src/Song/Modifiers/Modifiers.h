#pragma once
#include <fstream>
#include <assert.h>
#include "Variable Types/UnicodeString.h"
#include "FloatArray.h"

class TxtFileModifier
{
protected:
	const std::string_view m_name;

public:
	constexpr TxtFileModifier(const std::string_view name) : m_name(name) {}
	constexpr virtual ~TxtFileModifier() = default;

	constexpr std::string_view getName() const { return m_name; }

	virtual void write(std::fstream& outFile) const = 0;
	virtual void write_ini(std::fstream& outFile) const = 0;
};

class StringModifier : public TxtFileModifier
{
public:
	UnicodeString m_string;

	using TxtFileModifier::TxtFileModifier;
	StringModifier(const std::string_view name, std::u32string&& str) : TxtFileModifier(name), m_string(std::move(str)) {}

	void write(std::fstream& outFile) const override;
	void write_ini(std::fstream& outFile) const override;
};

template <typename T>
class NumberModifier : public TxtFileModifier
{
public:
	T m_value = 0;

	using TxtFileModifier::TxtFileModifier;
	constexpr NumberModifier(const std::string_view name, T value) : TxtFileModifier(name), m_value(value) {}

	NumberModifier& operator=(const NumberModifier& mod)
	{
		m_value = mod.m_value;
		return *this;
	}
	
	void write(std::fstream& outFile) const override
	{
		outFile << '\t' << m_name << " = " << m_value << '\n';
	}

	void write_ini(std::fstream& outFile) const override
	{
		outFile << m_name << " = " << m_value << '\n';
	}

	T& operator=(const T& value)
	{
		m_value = value;
		return m_value;
	}

	T& operator*=(float multiplier)
	{
		return m_value = T(m_value * multiplier);
	}
	operator T() const { return m_value; }
	operator const T&() { return m_value; }
};

using UINT32Modifier = NumberModifier<uint32_t>;
using INT32Modifier = NumberModifier<int32_t>;
using UINT16Modifier = NumberModifier<uint16_t>;
using FloatModifier = NumberModifier<float>;

class FloatArrayModifier : public TxtFileModifier
{
public:
	FloatArray m_floats;

	using TxtFileModifier::TxtFileModifier;
	constexpr FloatArrayModifier(const std::string_view name, FloatArray&& values) : TxtFileModifier(name), m_floats(values) {}

	void write(std::fstream& outFile) const override;
	void write_ini(std::fstream& outFile) const override;

	float& operator[](size_t i)
	{
		assert(i < 2);
		return m_floats[i];
	}
};

class BooleanModifier : public TxtFileModifier
{
public:
	bool m_boolean = false;

	using TxtFileModifier::TxtFileModifier;
	constexpr BooleanModifier(const std::string_view name, bool value) : TxtFileModifier(name), m_boolean(value) {}

	void write(std::fstream& outFile) const override;
	void write_ini(std::fstream& outFile) const override;

	bool& operator=(const bool& value)
	{
		m_boolean = value;
		return m_boolean;
	}

	operator bool() const { return m_boolean; }
};
