#pragma once
#include "Base Nodes/Sustainable.h"
#include "Base Nodes/Modifiable.h"

class DrumPad : public Sustainable, public Modifiable
{
public:
	Toggleable m_isAccented;
	Toggleable m_isGhosted;

	bool modify(char modifier);
	bool modify_binary(char modifier);
	int save_modifier_cht(std::stringstream& buffer) const;
	int save_modifier_cht(int lane, std::stringstream& buffer) const;
	void save_modifier_bch(char* buffer) const;
	bool save_modifier_bch(int lane, char*& buffer) const;
};

class DrumPad_Pro : public DrumPad
{
public:
	Toggleable m_isCymbal;

	bool modify(char modifier);
	bool modify_binary(char modifier);
	int save_modifier_cht(std::stringstream& buffer) const;
	int save_modifier_cht(int lane, std::stringstream& buffer) const;
	void save_modifier_bch(char* buffer) const;
	bool save_modifier_bch(int lane, char*& buffer) const;
};

class DrumPad_Bass : public Sustainable, public Modifiable
{
public:
	Toggleable m_isDoubleBass;
	bool modify(char modifier);
	bool modify_binary(char modifier);
	int save_modifier_cht(std::stringstream& buffer) const;
	int save_modifier_cht(int lane, std::stringstream& buffer) const;
	void save_modifier_bch(char* buffer) const;
	bool save_modifier_bch(int lane, char*& buffer) const;
};
