#include "Hittable.h"

void Hittable::save_cht(int lane, std::fstream& outFile) const
{
	outFile << ' ' << lane;
}

void Hittable::save_bch(int lane, char*& outPtr) const
{
	*outPtr++ = (char)lane;
}
