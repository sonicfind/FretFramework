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

	TxtFileModifier(const std::string_view name, std::u32string&& string) : m_name(name), m_type(Type::STRING_NOCASE)
	{
		new(c_BUFFER) std::u32string(std::move(string));
	}

	template <class T>
	constexpr TxtFileModifier(const std::string_view name, T value) : m_name(name)
	{
		if constexpr (std::is_same_v<T, uint32_t>)
			m_type = Type::UINT32;
		else if constexpr (std::is_same_v<T, int32_t>)
			m_type = Type::INT32;
		else if constexpr (std::is_same_v<T, uint16_t>)
			m_type = Type::UINT16;
		else if constexpr (std::is_same_v<T, bool>)
			m_type = Type::BOOL;
		else if constexpr (std::is_same_v<T, float>)
			m_type = Type::FLOAT;
		else if constexpr (std::is_same_v<T, FloatArray>)
			m_type = Type::FLOATARRAY;
		else
			throw std::runtime_error("Invalid constructor type");

		memcpy(c_BUFFER, &value, sizeof(T));
	}

	TxtFileModifier(TxtFileModifier&& other) noexcept : m_name(other.m_name), m_type(other.m_type)
	{
		memcpy(c_BUFFER, other.c_BUFFER, sizeof(c_BUFFER));
		other.m_type = Type::NONE;
	}

private:
	template <class T>
	T* cast()
	{
		return reinterpret_cast<T*>(c_BUFFER);
	}

	template <class T>
	const T* cast() const
	{
		return reinterpret_cast<const T*>(c_BUFFER);
	}

public:

	TxtFileModifier& operator=(const TxtFileModifier& other) noexcept
	{
		m_name = other.m_name;
		m_type = other.m_type;
		switch (m_type)
		{
		case Type::STRING:        new(c_BUFFER) UnicodeString(*other.cast<UnicodeString>()); break;
		case Type::STRING_NOCASE: new(c_BUFFER) std::u32string(*other.cast<std::u32string>()); break;
		default:
			memcpy(c_BUFFER, other.c_BUFFER, sizeof(c_BUFFER));
			break;
		}
		return *this;
	}
	
	~TxtFileModifier() noexcept
	{
		static constexpr auto destruct = [](auto* ptr)
		{
			using T = std::decay_t<decltype(*ptr)>;
			ptr->~T();
		};

		if (m_type == Type::STRING)
			destruct((UnicodeString*)c_BUFFER);
		else if (m_type == Type::STRING_NOCASE)
			destruct((std::u32string*)c_BUFFER);
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

		return *reinterpret_cast<T*>(c_BUFFER);
	}

	template <class T>
	const T& getValue() const
	{
		if (!validateType<T>())
			throw std::runtime_error("Template type does match internal type");

		return *reinterpret_cast<const T*>(c_BUFFER);
	}

	template <class T>
	TxtFileModifier& operator=(const T& value)
	{
		if (!validateType<T>())
			throw std::runtime_error("Template type does match internal type");

		*reinterpret_cast<T*>(c_BUFFER) = value;
		return *this;
	}

	void write_cht(std::fstream& outFile) const;
	void write_ini(std::fstream& outFile) const;
};
