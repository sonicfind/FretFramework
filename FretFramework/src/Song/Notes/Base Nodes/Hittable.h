#pragma once
#include <stdint.h>
#include <sstream>
#include "Toggleable.h"

class Hittable
{
public:
	Toggleable m_isActive;

	constexpr explicit Hittable() {}
	void init() { m_isActive = true; }
	inline operator bool() const { return m_isActive; }
	void toggle() { m_isActive.toggle(); }
	void save_cht(int lane, std::stringstream& buffer) const;
	void save_bch(int lane, char*& outPtr) const;
	bool operator==(Hittable& hit)
	{
		return m_isActive == hit.m_isActive;
	}

	bool operator!=(Hittable& hit)
	{
		return m_isActive != hit.m_isActive;
	}

	void operator*=(float multiplier) {}
};
