#pragma once
#include "Sustainable.h"

class Modifiable
{
public:
	virtual bool modify(char modifier) = 0;
	virtual void save_modifier_cht(std::fstream& outFile) const = 0;
	virtual void save_modifier_cht(int lane, std::fstream& outFile) const = 0;
};
