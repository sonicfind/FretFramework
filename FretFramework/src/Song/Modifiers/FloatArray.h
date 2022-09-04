#pragma once
#include <assert.h>

struct FloatArray
{
	float floats[2];
	float& operator[](size_t i)
	{
		assert(i < 2);
		return floats[i];
	}

	float operator[](size_t i) const
	{
		assert(i < 2);
		return floats[i];
	}
};
