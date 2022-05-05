#include "Hittable.h"

void Hittable::save_cht(int lane, std::fstream& outFile) const
{
	outFile << ' ' << lane;
}
