#include "WebType.h"

WebType::WebType(const unsigned char*& dataPtr)
	: m_value(*dataPtr++)
{
	switch (m_value)
	{
	case 255:
		m_value = *reinterpret_cast<const uint32_t*>(dataPtr);
		dataPtr += 4;
		break;
	case 254:
		m_value = *reinterpret_cast<const uint16_t*>(dataPtr);
		dataPtr += 2;
	}
}

WebType::WebType(uint32_t value)
	: m_value(value) {}

void WebType::copyToBuffer(char*& dataPtr) const
{
	if (m_value <= 253)
		*dataPtr++ = (char)m_value;
	else
	{
		*reinterpret_cast<uint32_t*>(dataPtr + 1) = m_value;
		if (m_value > UINT16_MAX)
		{
			*dataPtr = (char)255;
			dataPtr += 5;
		}
		else
		{
			*dataPtr = (char)254;
			dataPtr += 3;
		}
	}
}

void WebType::writeToFile(std::fstream& outFile) const
{
	if (m_value > UINT16_MAX)
	{
		outFile.put((char)255);
		outFile.write((char*)&m_value, 4);
	}
	else if (m_value > 253)
	{
		outFile.put((char)254);
		outFile.write((char*)&m_value, 2);
	}
	else
		outFile.put((char)m_value);
}

WebType& WebType::operator=(uint32_t value)
{
	m_value = value;
	return *this;
}

WebType::operator uint32_t() const
{
	return m_value;
}

void WebType::discard(const unsigned char*& dataPtr)
{
	switch (*dataPtr++)
	{
	case 255:
		dataPtr += 4;
		break;
	case 254:
		dataPtr += 2;
	}
}
