#pragma once
#include <fstream>
#include <assert.h>
#include "Variable Types/UnicodeString.h"
#include "FloatArray.h"
#include <variant>

class TxtFileModifier
{
	char c_BUFFER[sizeof(UnicodeString)];

	std::string_view m_name;
	enum class Type
	{
		STRING,
		STRING_NOCASE,
		UINT32,
		INT32,
		UINT16,
		BOOL,
		FLOAT,
		FLOATARRAY,
		NONE
	} m_type;

public:
	TxtFileModifier(const std::string_view name, UnicodeString&& string) : m_name(name), m_type(Type::STRING)
	{
		new(c_BUFFER) UnicodeString(std::move(string));
	}

	TxtFileModifier(const std::string_view name, const UnicodeString& string) : m_name(name), m_type(Type::STRING)
	{
		new(c_BUFFER) UnicodeString(string);
	}

	TxtFileModifier(const std::string_view name, std::u32string&& string) : m_name(name), m_type(Type::STRING_NOCASE)
	{
		new(c_BUFFER) std::u32string(std::move(string));
	}

	TxtFileModifier(const std::string_view name, uint32_t u32) : m_name(name), m_type(Type::UINT32)
	{
		memcpy(c_BUFFER, &u32, sizeof(uint32_t));
	}

	TxtFileModifier(const std::string_view name, int32_t i32) : m_name(name), m_type(Type::INT32)
	{
		memcpy(c_BUFFER, &i32, sizeof(int32_t));
	}

	TxtFileModifier(const std::string_view name, uint16_t u16) : m_name(name), m_type(Type::UINT16)
	{
		memcpy(c_BUFFER, &u16, sizeof(uint16_t));
	}

	TxtFileModifier(const std::string_view name, bool bl) : m_name(name), m_type(Type::BOOL)
	{
		memcpy(c_BUFFER, &bl, sizeof(bool));
	}

	TxtFileModifier(const std::string_view name, float flt) : m_name(name), m_type(Type::FLOAT)
	{
		memcpy(c_BUFFER, &flt, sizeof(float));
	}

	TxtFileModifier(const std::string_view name, FloatArray fltArr) : m_name(name), m_type(Type::FLOATARRAY)
	{
		memcpy(c_BUFFER, &fltArr, sizeof(FloatArray));
	}

	TxtFileModifier(TxtFileModifier&& other) noexcept : m_name(other.m_name), m_type(other.m_type)
	{
		memcpy(c_BUFFER, other.c_BUFFER, sizeof(c_BUFFER));
		other.m_type = Type::NONE;
	}

private:
	template <class T>
	T& cast()
	{
		return *reinterpret_cast<T*>(c_BUFFER);
	}

	template <class T>
	const T& cast() const
	{
		return *reinterpret_cast<const T*>(c_BUFFER);
	}

public:

	TxtFileModifier& operator=(const TxtFileModifier& other) noexcept
	{
		m_name = other.m_name;
		m_type = other.m_type;
		switch (m_type)
		{
		case Type::STRING:        new(c_BUFFER) UnicodeString(other.cast<UnicodeString>()); break;
		case Type::STRING_NOCASE: new(c_BUFFER) std::u32string(other.cast<std::u32string>()); break;
		default:
			memcpy(c_BUFFER, other.c_BUFFER, sizeof(c_BUFFER));
			break;
		}
		return *this;
	}
	
	~TxtFileModifier() noexcept
	{
		static constexpr auto destruct = [](auto& obj)
		{
			using T = std::decay_t<decltype(obj)>;
			obj.~T();
		};

		if (m_type == Type::STRING)
			destruct(cast<UnicodeString>());
		else if (m_type == Type::STRING_NOCASE)
			destruct(cast<std::u32string>());
	}

	constexpr std::string_view getName() const noexcept { return m_name; }

private:
	template <class T>
	bool validateType() const noexcept
	{
		if constexpr (std::is_same_v<T, UnicodeString>)       return m_type == Type::STRING;
		else if constexpr (std::is_same_v<T, std::u32string>) return m_type == Type::STRING_NOCASE;
		else if constexpr (std::is_same_v<T, uint32_t>)       return m_type == Type::UINT32;
		else if constexpr (std::is_same_v<T, int32_t>)        return m_type == Type::INT32;
		else if constexpr (std::is_same_v<T, uint16_t>)       return m_type == Type::UINT16;
		else if constexpr (std::is_same_v<T, float>)          return m_type == Type::FLOAT;
		else if constexpr (std::is_same_v<T, bool>)           return m_type == Type::BOOL;
		else if constexpr (std::is_same_v<T, FloatArray>)     return m_type == Type::FLOATARRAY;
		else
			return false;
	}
public:

	template <class T>
	T& getValue()
	{
		if (!validateType<T>())
			throw std::runtime_error("Template type does match internal type");

		return cast<T>();
	}

	template <class T>
	const T& getValue() const
	{
		if (!validateType<T>())
			throw std::runtime_error("Template type does match internal type");

		return cast<T>();
	}

	template <class T>
	TxtFileModifier& operator=(const T& value)
	{
		if (!validateType<T>())
			throw std::runtime_error("Template type does match internal type");

		cast<T>() = value;
		return *this;
	}

	void write_cht(std::fstream& outFile) const;

	template <class T>
	void writeVal(std::fstream& outFile) const
	{
		if      constexpr (std::is_same_v<T, bool>)           outFile << m_name << " = " << std::boolalpha << cast<T>() << '\n';
		else if constexpr (std::is_same_v<T, FloatArray>)     outFile << m_name << " = " << cast<T>()[0] << ' ' << cast<T>()[1] << '\n';
		else if constexpr (std::is_same_v<T, std::u32string>) outFile << m_name << " = " << UnicodeString::U32ToStr(cast<T>()) << '\n';
		else                                                  outFile << m_name << " = " << cast<T>() << '\n';
	}

	void write_ini(std::fstream& outFile) const;
};
