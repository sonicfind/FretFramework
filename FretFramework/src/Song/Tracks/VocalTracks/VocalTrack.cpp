#include "VocalTrack.h"

template<>
void VocalTrack<1>::save_cht(std::fstream& outFile) const
{
	if (!occupied())
		return;

	outFile << "[" << m_name << "]\n{\n";
	outFile << "\tLyrics = " << m_vocals[0].size() << '\n';
	outFile << "\tPercussion = " << m_percussion.size() << '\n';

	auto vocalIter = m_vocals[0].begin();
	auto percIter = m_percussion.begin();
	auto effectIter = m_effects.begin();
	auto eventIter = m_events.begin();
	bool vocalValid = vocalIter != m_vocals[0].end();
	auto percValid = percIter != m_percussion.end();
	bool effectValid = effectIter != m_effects.end();
	bool eventValid = eventIter != m_events.end();

	while (effectValid || vocalValid || percValid || eventValid)
	{
		while (effectValid &&
			(!vocalValid || effectIter->first <= vocalIter->first) &&
			(!percValid || effectIter->first <= percIter->first) &&
			(!eventValid || effectIter->first <= eventIter->first))
		{
			for (const auto& eff : effectIter->second)
				eff->save_cht(effectIter->first, outFile, "\t");
			effectValid = ++effectIter != m_effects.end();
		}

		while (vocalValid &&
			(!effectValid || vocalIter->first < effectIter->first) &&
			(!percValid || vocalIter->first <= percIter->first) &&
			(!eventValid || vocalIter->first <= eventIter->first))
		{
			outFile << '\t' << vocalIter->first << " = N";
			vocalIter->second.save_cht(1, outFile);
			if (vocalIter->second.m_isActive)
				vocalIter->second.save_pitch_cht(outFile);
			outFile << '\n';
			vocalValid = ++vocalIter != m_vocals[0].end();
		}

		while (percValid &&
			(!effectValid || percIter->first < effectIter->first) &&
			(!vocalValid || percIter->first < vocalIter->first)  &&
			(!eventValid || percIter->first <= eventIter->first))
		{
			outFile << '\t' << percIter->first << " = N";
			percIter->second.save_cht(0, outFile);
			if (!percIter->second.m_isActive)
				percIter->second.save_modifier_cht(outFile);
			outFile << '\n';
		}

		while (eventValid &&
			(!effectValid || eventIter->first < effectIter->first) &&
			(!percValid || eventIter->first < percIter->first) &&
			(!vocalValid || eventIter->first < vocalIter->first))
		{
			for (const auto& str : eventIter->second)
				outFile << "\t" << eventIter->first << " = E \"" << str << "\"\n";
			eventValid = ++eventIter != m_events.end();
		}
	}

	outFile << "}\n";
	outFile.flush();
}

template<>
void VocalTrack<3>::save_cht(std::fstream& outFile) const
{
	if (!occupied())
		return;

	struct SaveToCht
	{
		const Vocal* m_vocals[3] = { nullptr };
		void save_cht(uint32_t position, std::fstream& outFile)
		{
			int numActive = 0;
			for (const auto& vocal : m_vocals)
				if (vocal)
					++numActive;

			if (numActive == 1)
			{
				outFile << '\t' << position << " = N";
				int lane = 0;
				while (!m_vocals[lane])
					++lane;

				m_vocals[lane]->save_cht(lane + 1, outFile);
				if (m_vocals[lane]->m_isActive)
					m_vocals[lane]->save_pitch_cht(outFile);
			}
			else
			{
				outFile << '\t' << position << " = C " << numActive;
				int numSung = 0;
				for (int lane = 0; lane < 3; ++lane)
					if (m_vocals[lane])
					{
						m_vocals[lane]->save_cht(lane + 1, outFile);
						if (m_vocals[lane]->m_isActive)
							++numSung;
					}

				if (numSung > 0)
				{
					outFile << "\n\t" << position << " = V " << numSung;
					for (int lane = 0; lane < 3; ++lane)
						if (m_vocals[lane] && numSung)
							m_vocals[lane]->save_pitch_cht(lane + 1, outFile);
				}
			}
			outFile << '\n';
		}
	};

	std::vector<std::pair<uint32_t, SaveToCht>> vocalsToCht;
	{
		int i = 0;
		while (i < 3 && m_vocals[i].empty())
			++i;

		if (i < 3)
		{
			vocalsToCht.reserve(m_vocals[i].size());
			for (const auto& vocal : m_vocals[i])
			{
				static std::pair<uint32_t, SaveToCht> pairNode;
				pairNode.first = vocal.first;
				pairNode.second.m_vocals[i] = &vocal.second;
				vocalsToCht.push_back(pairNode);
			}
			++i;

			while (i < 3)
			{
				for (const auto& vocal : m_vocals[i])
					VectorIteration::try_emplace(vocalsToCht, vocal.first).m_vocals[i] = &vocal.second;
				++i;
			}
		}
	}

	outFile << "[" << m_name << "]\n{\n";
	outFile << "\tHarm1 = " << m_vocals[0].size() << '\n';
	outFile << "\tHarm2 = " << m_vocals[1].size() << '\n';
	outFile << "\tHarm3 = " << m_vocals[2].size() << '\n';
	outFile << "\tPercussion = " << m_percussion.size() << '\n';

	auto vocalIter = vocalsToCht.begin();
	auto percIter = m_percussion.begin();
	auto effectIter = m_effects.begin();
	auto eventIter = m_events.begin();
	bool vocalValid = vocalIter != vocalsToCht.end();
	auto percValid = percIter != m_percussion.end();
	bool effectValid = effectIter != m_effects.end();
	bool eventValid = eventIter != m_events.end();

	while (effectValid || vocalValid || eventValid)
	{
		while (effectValid &&
			(!vocalValid || effectIter->first <= vocalIter->first) &&
			(!percValid || effectIter->first <= percIter->first) &&
			(!eventValid || effectIter->first <= eventIter->first))
		{
			for (const auto& eff : effectIter->second)
				eff->save_cht(effectIter->first, outFile, "\t");
			effectValid = ++effectIter != m_effects.end();
		}

		while (vocalValid &&
			(!effectValid || vocalIter->first < effectIter->first) &&
			(!percValid || vocalIter->first <= percIter->first) &&
			(!eventValid || vocalIter->first <= eventIter->first))
		{
			vocalIter->second.save_cht(vocalIter->first, outFile);
			vocalValid = ++vocalIter != vocalsToCht.end();
		}

		while (percValid &&
			(!effectValid || percIter->first < effectIter->first) &&
			(!vocalValid || percIter->first < vocalIter->first) &&
			(!eventValid || percIter->first <= eventIter->first))
		{
			outFile << '\t' << percIter->first << " = N";
			percIter->second.save_cht(0, outFile);
			if (!percIter->second.m_isActive)
				percIter->second.save_modifier_cht(outFile);
			outFile << '\n';
			percValid = ++percIter != m_percussion.end();
		}

		while (eventValid &&
			(!effectValid || eventIter->first < effectIter->first) &&
			(!percValid || eventIter->first < percIter->first) &&
			(!vocalValid || eventIter->first < vocalIter->first))
		{
			for (const auto& str : eventIter->second)
				outFile << "\t" << eventIter->first << " = E \"" << str << "\"\n";
			eventValid = ++eventIter != m_events.end();
		}
	}

	outFile << "}\n";
	outFile.flush();
}
