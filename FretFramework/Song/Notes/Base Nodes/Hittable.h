#pragma once
#include <stdint.h>
#include <fstream>
#include "Toggleable.h"

class Hittable
{
public:
	Toggleable m_isActive;
	virtual void init(uint32_t sustain = 0) { m_isActive = true; }
	operator bool() const { return m_isActive; }
	void toggle() { m_isActive.toggle(); }
	void save_cht(int lane, std::fstream& outFile) const;
	bool operator==(Hittable& hit)
	{
		return m_isActive == hit.m_isActive;
	}

	bool operator!=(Hittable& hit)
	{
		return m_isActive != hit.m_isActive;
	}
};