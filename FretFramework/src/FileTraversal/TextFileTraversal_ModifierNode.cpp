#include "FileTraversal/TextFileTraversal.h"
#include "Song/Modifiers/Modifiers.h"

std::unique_ptr<TxtFileModifier> TextTraversal::createModifier(const ModifierNode* node)
{
	switch (node->type)
	{
	case ModifierNode::STRING:
	case ModifierNode::STRING_CHART:
	{
		auto modifier = std::make_unique<StringModifier>(node->name);
		modifier->m_string = extractText(node->type == ModifierNode::STRING);
		return modifier;
	}
	case ModifierNode::UINT32:
		return std::make_unique<UINT32Modifier>(node->name, extractInt<uint32_t>());
	case ModifierNode::INT32:
		return std::make_unique<INT32Modifier>(node->name, extractInt<int32_t>());
	case ModifierNode::UINT16:
		return std::make_unique<UINT16Modifier>(node->name, extractInt<uint16_t>());
	case ModifierNode::BOOL:
		return std::make_unique<BooleanModifier>(node->name, extractBoolean());
	case ModifierNode::FLOAT:
	{
		auto modifier = std::make_unique<FloatModifier>(node->name);
		extract(modifier->m_value);
		return modifier;
	}
	case ModifierNode::FLOATARRAY:
	{
		auto modifier = std::make_unique<FloatArrayModifier>(node->name);
		if (extract(modifier->m_floats[0]))
			extract(modifier->m_floats[1]);
		return modifier;
	}
	default:
		return nullptr;
	}
}
