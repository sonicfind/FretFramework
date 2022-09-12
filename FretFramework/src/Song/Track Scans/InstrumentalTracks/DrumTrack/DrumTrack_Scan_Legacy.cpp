#include "DrumTrack_Scan_Legacy.h"

void InstrumentalTrack_Scan<DrumNote_Legacy>::scan_chart_V1(int diff, TextTraversal& traversal)
{
	bool found = false;
	const int val = 1 << diff;
	traversal.resetPosition();
	while (traversal && traversal != '}' && traversal != '[')
	{
		try
		{
			traversal.extractPosition();
			unsigned char type = traversal.extract<unsigned char>();
			if (type == 'N' || type == 'n')
			{
				const int lane = traversal.extract<uint32_t>();
				if (traversal.skipInt())
				{
					if (DrumNote_Legacy::testIndex_chartV1(lane))
					{
						m_scanValue |= val;
						found = true;

						if (lane == 5)
							m_drumType = FIVELANE;
					}
					else if (m_drumType == UNKNOWN && DrumNote_Legacy::testCymbal_chartV1(lane))
						m_drumType = FOURLANE_PRO;

					if (found && m_drumType != UNKNOWN)
					{
						traversal.skipTrack();
						return;
					}
				}
			}
		}
		catch (std::runtime_error err)
		{

		}

		traversal.next();
	}
}

