#pragma once
#include <vector>
#include <algorithm>

namespace VectorIteration
{
	template <class T>
	T& getIterator(std::vector<std::pair<uint32_t, T>>& vec, uint32_t position)
	{
		auto iter = std::upper_bound(vec.begin(), vec.end(), position,
			[](uint32_t position, const std::pair<uint32_t, T>& pair) {
				return position < pair.first;
			});

		if (iter != vec.begin() && (--iter)->first == position)
			return iter->second;
		else
			throw std::out_of_range("Position not found");
	}

	template <class T>
	const T& getIterator(const std::vector<std::pair<uint32_t, T>>& vec, uint32_t position)
	{
		auto iter = std::upper_bound(vec.begin(), vec.end(), position,
			[](uint32_t position, const std::pair<uint32_t, T>& pair) {
				return position < pair.first;
			});

		if (iter != vec.begin() && (--iter)->first == position)
			return iter->second;
		else
			throw std::out_of_range("Position not found");
	}

	template <class T>
	T& try_emplace(std::vector<std::pair<uint32_t, T>>& vec, uint32_t position)
	{
		static std::pair<uint32_t, T> pairNode;
		auto iter = std::upper_bound(vec.begin(), vec.end(), position,
			[](uint32_t position, const std::pair<uint32_t, T>& pair) {
				return position < pair.first;
			});

		if (iter == vec.begin() || (iter - 1)->first != position)
		{
			pairNode.first = position;
			iter = vec.insert(iter, pairNode);
		}
		else
			--iter;
		return iter->second;
	}

	template <class T>
	void insert_if_not(std::vector<std::pair<uint32_t, T>>& vec, const std::pair<uint32_t, T>& obj)
	{
		auto iter = std::upper_bound(vec.begin(), vec.end(), obj.first,
			[](uint32_t position, const std::pair<uint32_t, T>& pair) {
				return position < pair.first;
			});

		if (iter == vec.begin() || (iter - 1)->first != obj.first)
			iter = vec.insert(iter, obj);
	}
}