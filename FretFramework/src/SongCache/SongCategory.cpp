#include "SongCategory.h"

template <>
void SongCategory<char32_t, Song, SongAttribute::TITLE>::add(const Song& element)
{
	Song::setAttributeType(SongAttribute::TITLE);
	VectorIteration::try_emplace(m_elements, element.getAttribute().getLowerCase()[0]).add(element);
}

