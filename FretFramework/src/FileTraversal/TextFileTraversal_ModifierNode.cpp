#include "FileTraversal/TextFileTraversal.h"
#include "Song/Modifiers/Modifiers.h"

TxtFileModifier TextTraversal::createModifier(const ModifierNode* node)
{
	switch (node->type)
	{
	case ModifierNode::STRING:
	case ModifierNode::STRING_CHART:
		return { node->name, extractText(node->type == ModifierNode::STRING) };
	case ModifierNode::UINT32:
		return { node->name, extract<uint32_t>() };
	case ModifierNode::INT32:
		return { node->name, extract<int32_t>() };
	case ModifierNode::UINT16:
		return { node->name, extract<uint16_t>() };
	case ModifierNode::BOOL:
		return { node->name, extract<bool>() };
	case ModifierNode::FLOAT:
		return { node->name, extract<float>() };
	default:
		return { node->name, extract<FloatArray>() };
	}
}
