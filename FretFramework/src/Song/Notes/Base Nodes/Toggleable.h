#pragma once

class Toggleable
{
	bool m_state = false;
public:
	void toggle() { m_state = !m_state; }
	inline void operator=(bool state) { m_state = state; }
	operator bool() const { return m_state; }
	bool operator==(const Toggleable& tg)
	{
		return m_state == tg.m_state;
	}

	bool operator!=(const Toggleable& tg)
	{
		return m_state != tg.m_state;
	}
};
