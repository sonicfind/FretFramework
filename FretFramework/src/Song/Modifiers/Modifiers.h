#pragma once
#include <fstream>
#include <assert.h>
#include "Variable Types/UnicodeString.h"
#include "FloatArray.h"
#include <variant>

class TxtFileModifier
{
	std::string_view m_name;
	std::variant<UnicodeString, uint32_t, int32_t, uint16_t, float, bool, FloatArray> m_value;

public:
	template <class T>
	constexpr TxtFileModifier(const std::string_view name, T value) : m_name(name), m_value(std::move(value)) {}
	constexpr TxtFileModifier(const std::string_view name, FloatArray&& floats) : m_name(name), m_value(std::move(floats)) {}
	TxtFileModifier(const std::string_view name, UnicodeString&& string) : m_name(name), m_value(std::move(string)) {}
	constexpr virtual ~TxtFileModifier() = default;

	constexpr std::string_view getName() const { return m_name; }

	template <class T>
	T& getValue()
	{
		return std::get<T>(m_value);
	}

	template <class T>
	const T& getValue() const
	{
		return std::get<T>(m_value);
	}

	template <class T>
	TxtFileModifier& setValue(const T& value)
	{
		m_value = value;
		return *this;
	}

	void write(std::fstream& outFile) const;
	void write_ini(std::fstream& outFile) const;
};
