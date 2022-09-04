#include "FileTraversal/TextFileTraversal.h"
#include "Song/Modifiers/Modifiers.h"

std::unique_ptr<TxtFileModifier> TextTraversal::createModifier(const ModifierNode* node)
{
	switch (node->type)
	{
	case ModifierNode::STRING:
	case ModifierNode::STRING_CHART:
		return std::make_unique<StringModifier>(node->name, extractText(node->type == ModifierNode::STRING));
	case ModifierNode::UINT32:
		return std::make_unique<UINT32Modifier>(node->name, extract<uint32_t>());
	case ModifierNode::INT32:
		return std::make_unique<INT32Modifier>(node->name, extract<int32_t>());
	case ModifierNode::UINT16:
		return std::make_unique<UINT16Modifier>(node->name, extract<uint16_t>());
	case ModifierNode::BOOL:
		return std::make_unique<BooleanModifier>(node->name, extract<bool>());
	case ModifierNode::FLOAT:
		return std::make_unique<FloatModifier>(node->name, extract<float>());
	case ModifierNode::FLOATARRAY:
		return std::make_unique<FloatArrayModifier>(node->name, extract<FloatArray>());
	default:
		return nullptr;
	}
}
