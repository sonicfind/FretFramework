#pragma once
#include "FileTraversal/TextFileTraversal.h"
#include "Modifiers.h"

struct ModifierNode
{
	const std::string_view m_name;
	const enum Type
	{
		STRING,
		STRING_NOCASE,
		STRING_CHART,
		STRING_CHART_NOCASE,
		UINT32,
		INT32,
		UINT16,
		BOOL,
		FLOAT,
		FLOATARRAY
	} m_type;

	TxtFileModifier createModifier(TextTraversal& _traversal) const
	{
		switch (m_type)
		{
		case STRING:
		case STRING_NOCASE:
		case STRING_CHART:
		case STRING_CHART_NOCASE:
		{
			std::u32string str = _traversal.extractText(m_type < STRING_CHART);
			if ((m_type & 1) == 0)
				return { m_name, UnicodeString(std::move(str)) };
			else
				return { m_name, std::move(str) };
		}
		case UINT32:
			return { m_name, _traversal.extract<uint32_t>() };
		case INT32:
			return { m_name, _traversal.extract<int32_t>() };
		case UINT16:
			return { m_name, _traversal.extract<uint16_t>() };
		case BOOL:
			return { m_name, _traversal.extract<bool>() };
		case FLOAT:
			return { m_name, _traversal.extract<float>() };
		default:
			return { m_name, _traversal.extract<FloatArray>() };
		}
	}

	template <size_t SIZE>
	static const ModifierNode* testForModifierName(const std::pair<std::string_view, ModifierNode>(&_MODIFIERLIST)[SIZE], const std::string_view _modifierName)
	{
		const auto pairIter = std::lower_bound(std::begin(_MODIFIERLIST), std::end(_MODIFIERLIST), _modifierName,
			[](const std::pair<std::string_view, ModifierNode>& pair, const std::string_view str)
			{
				return pair.first < str;
			});

		if (pairIter == std::end(_MODIFIERLIST) || _modifierName != pairIter->first)
			return nullptr;

		return &pairIter->second;
	}
};
