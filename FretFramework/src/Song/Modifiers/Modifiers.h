#pragma once
#include <fstream>
#include "FileTraversal/TextFileTraversal.h"
#include <assert.h>

class TxtFileModifier
{
protected:
	const std::string_view m_name;

public:
	constexpr TxtFileModifier(const char* name) : m_name(name) {}
	virtual ~TxtFileModifier() = default;

	constexpr std::string_view getName() const { return m_name; }

	virtual void read(TextTraversal& traversal) = 0;
	virtual void write(std::fstream& outFile) const = 0;
	virtual void write_ini(std::fstream& outFile) const = 0;
};

class StringModifier : public TxtFileModifier
{
private:
	bool m_isIniModifier = true;
public:
	UnicodeString m_string;

	StringModifier(const char* name, bool ini = true) : TxtFileModifier(name), m_isIniModifier(ini) {}
	StringModifier(const char* name, const char32_t* str) : TxtFileModifier(name), m_string(str) {}

	void read(TextTraversal& traversal) override;
	void write(std::fstream& outFile) const override;
	void write_ini(std::fstream& outFile) const override;

	UnicodeString& operator=(const std::u32string& value)
	{
		m_string = value;
		return m_string;
	}

	StringModifier& operator=(const StringModifier& mod)
	{
		m_string = mod.m_string;
		return *this;
	}

	int compare(const StringModifier& mod) const
	{
		return m_string.compare(mod.m_string);
	}

	char32_t operator[](size_t index) const { return m_string[index]; }
	operator UnicodeString() const { return m_string; }
	operator const UnicodeString&() const { return m_string; }
};

template <typename T>
class NumberModifier : public TxtFileModifier
{
public:
	T m_value = 0;

private:
	T m_default = 0;

	bool isWritable() const { return m_value != m_default; }

public:
	constexpr NumberModifier(const char* name) : TxtFileModifier(name) {}
	constexpr NumberModifier(const char* name, T value, T def = T()) : TxtFileModifier(name), m_value(value), m_default(def) {}

	NumberModifier& operator=(const NumberModifier& mod)
	{
		m_value = mod.m_value;
		return *this;
	}

	void read(TextTraversal& traversal) override
	{
		traversal.extract(m_value);
	}
	
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
	operator T() const { return m_value; }
	operator const T&() { return m_value; }
};

class FloatArrayModifier : public TxtFileModifier
{
public:
	float m_floats[2] = { 0, 0 };

	constexpr FloatArrayModifier(const char* name) : TxtFileModifier(name) {}

	void read(TextTraversal& traversal) override;
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

private:
	bool m_isActive = false;

public:
	constexpr BooleanModifier(const char* name) : TxtFileModifier(name) {}
	constexpr BooleanModifier(const char* name, bool value, bool active) : TxtFileModifier(name), m_boolean(value), m_isActive(active) {}

	void read(TextTraversal& traversal) override;
	void write(std::fstream& outFile) const override;
	void write_ini(std::fstream& outFile) const override;

	bool& operator=(const bool& value)
	{
		m_boolean = value;
		m_isActive = true;
		return m_boolean;
	}

	bool isActive() const { return m_isActive; }
	void deactivate() { m_isActive = false; }
	operator bool() const { return m_boolean; }
};
