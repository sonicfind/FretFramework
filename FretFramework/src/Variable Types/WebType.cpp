#include "WebType.h"

WebType::WebType(const unsigned char*& dataPtr)
	: m_value(*dataPtr++)
{
	if (m_value >= 254)
	{
		if (m_value == 255)
		{
			m_value = *reinterpret_cast<const uint32_t*>(dataPtr);
			dataPtr += 4;
		}
		else
		{
			m_value = *reinterpret_cast<const uint16_t*>(dataPtr);
			dataPtr += 2;
		}
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

void WebType::copyToBuffer(const uint32_t& value, char*& buffer)
{
	if (value <= 253)
		*buffer++ = (char)value;
	else
	{
		*reinterpret_cast<uint32_t*>(buffer + 1) = value;
		if (value > UINT16_MAX)
		{
			*buffer = char(255);
			buffer += 5;
		}
		else
		{
			*buffer = char(254);
			buffer += 3;
		}
	}
}

void WebType::writeToFile(const uint32_t& value, std::fstream& outFile)
{
	if (value > UINT16_MAX)
	{
		outFile.put((char)255);
		outFile.write((char*)&value, 4);
	}
	else if (value > 253)
	{
		outFile.put((char)254);
		outFile.write((char*)&value, 2);
	}
	else
		outFile.put((char)value);
}
