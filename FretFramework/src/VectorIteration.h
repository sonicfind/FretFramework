#pragma once
#include <vector>
#include <algorithm>

namespace VectorIteration
{
	template <class T, class Key>
	T& getIterator(std::vector<std::pair<Key, T>>& vec, Key key)
	{
		auto iter = std::upper_bound(vec.begin(), vec.end(), key,
			[](Key key, const std::pair<Key, T>& pair) {
				return key < pair.first;
			});

		if (iter != vec.begin() && (--iter)->first == key)
			return iter->second;
		else
			throw std::out_of_range("Key not found");
	}

	template <class T, class Key>
	const T& getIterator(const std::vector<std::pair<Key, T>>& vec, Key key)
	{
		auto iter = std::upper_bound(vec.begin(), vec.end(), key,
			[](Key key, const std::pair<Key, T>& pair) {
				return key < pair.first;
			});

		if (iter != vec.begin() && (--iter)->first == key)
			return iter->second;
		else
			throw std::out_of_range("Key not found");
	}

	template <class T, class Key>
	T& try_emplace(std::vector<std::pair<Key, T>>& vec, Key key)
	{
		static std::pair<Key, T> pairNode;
		auto iter = std::upper_bound(vec.begin(), vec.end(), key,
			[](Key key, const std::pair<Key, T>& pair) {
				return key < pair.first;
			});

		if (iter == vec.begin() || (iter - 1)->first != key)
		{
			pairNode.first = key;
			iter = vec.insert(iter, pairNode);
		}
		else
			--iter;
		return iter->second;
	}

	template <class T, class Key>
	void insert_if_not(std::vector<std::pair<Key, T>>& vec, const std::pair<Key, T>& obj)
	{
		auto iter = std::upper_bound(vec.begin(), vec.end(), obj.first,
			[](Key key, const std::pair<Key, T>& pair) {
				return key < pair.first;
			});

		if (iter == vec.begin() || (iter - 1)->first != obj.first)
			iter = vec.insert(iter, obj);
	}
}