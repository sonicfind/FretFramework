#pragma once
#include <vector>
#include <algorithm>

namespace VectorIteration
{
	template <class Key, class Object>
	Object& getElement(std::vector<std::pair<Key, Object>>& vec, const Key& key)
	{
		auto iter = std::upper_bound(vec.begin(), vec.end(), key,
			[](const Key& key, const std::pair<Key, Object>& pair) {
				return key < pair.first;
			});

		if (iter != vec.begin() && (--iter)->first == key)
			return iter->second;
		else
			throw std::out_of_range("Position not found");
	}

	template <class Key, class Object>
	const Object& getElement(const std::vector<std::pair<Key, Object>>& vec, const Key& key)
	{
		auto iter = std::upper_bound(vec.begin(), vec.end(), key,
			[](const Key& key, const std::pair<Key, Object>& pair) {
				return key < pair.first;
			});

		if (iter != vec.begin() && (--iter)->first == key)
			return iter->second;
		else
			throw std::out_of_range("Position not found");
	}

	template <class Key, class Object>
	auto getIterator(std::vector<std::pair<Key, Object>>& vec, const Key& key)
	{
		return std::upper_bound(vec.begin(), vec.end(), key,
			[](const Key& key, const std::pair<Key, Object>& pair) {
				return key < pair.first;
			});
	}
	

	template <class Key, class Object>
	Object& try_emplace(std::vector<std::pair<Key, Object>>& vec, const Key& key)
	{
		static const Object obj;
		auto iter = std::upper_bound(vec.begin(), vec.end(), key,
			[](const Key& key, const std::pair<Key, Object>& pair) {
				return key < pair.first;
			});

		if (iter == vec.begin() || (iter - 1)->first != key)
			return vec.emplace(iter, key, obj)->second;

		--iter;
		return iter->second;
	}

	template <class Key, class Object>
	void insert(std::vector<std::pair<Key, Object>>& vec, const std::pair<Key, Object>& element)
	{
		auto iter = std::upper_bound(vec.begin(), vec.end(), element.first,
			[](const Key& key, const std::pair<Key, Object>& pair) {
				return key < pair.first;
			});

		vec.insert(iter, element);
	}

	template <class Key, class Object>
	void insert_ptr(std::vector<const std::pair<Key, Object>*>& vec, const std::pair<Key, Object>* element)
	{
		auto iter = std::upper_bound(vec.begin(), vec.end(), element->first,
			[](const Key& key, const std::pair<Key, Object>* pair) {
				return key < pair->first;
			});

		vec.insert(iter, element);
	}
}