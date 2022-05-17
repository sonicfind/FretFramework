#pragma once
#include <fstream>
class Modifiable
{
public:
	virtual bool modify(char modifier) = 0;
	virtual bool modify_binary(char modifier) = 0;
	virtual void save_modifier_cht(std::fstream& outFile) const = 0;
	virtual void save_modifier_cht(int lane, std::fstream& outFile) const = 0;
	virtual void save_modifier_bch(char* modifier) const = 0;
	virtual bool save_modifier_bch(int lane, char*& outPtr) const = 0;
};
