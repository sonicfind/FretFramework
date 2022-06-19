#pragma once
#include <sstream>
class Modifiable
{
public:
	virtual void modify(char modifier) = 0;
	virtual void modify_binary(char modifier) = 0;
	virtual int save_modifier_cht(std::stringstream& outFile) const = 0;
	virtual int save_modifier_cht(int lane, std::stringstream& outFile) const = 0;
	virtual void save_modifier_bch(char* buffer) const = 0;
	virtual bool save_modifier_bch(int lane, char*& buffer) const = 0;
};
