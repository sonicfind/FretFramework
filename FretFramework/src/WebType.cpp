#include "WebType.h"

WebType::WebType(const unsigned char*& dataPtr)
	: m_value(*dataPtr)
{
	++dataPtr;
	switch (m_value)
	{
	case 255:
		memcpy(&m_value, dataPtr, 4);
		dataPtr += 4;
		break;
	case 254:
		memcpy(&m_value, dataPtr, 2);
		dataPtr += 2;
	}
}

WebType::WebType(uint32_t value)
	: m_value(value)
{
}

char WebType::s_writeBuffer[5];
size_t WebType::s_bufferSize;
void WebType::setBuffer() const
{
	if (m_value > UINT16_MAX)
	{
		s_writeBuffer[0] = (char)255;
		memcpy(s_writeBuffer + 1, &m_value, 4);
		s_bufferSize = 5;
	}
	else if (m_value > 253)
	{
		s_writeBuffer[0] = (char)254;
		memcpy(s_writeBuffer + 1, &m_value, 2);
		s_bufferSize = 3;
	}
	else
	{
		s_writeBuffer[0] = (char)m_value;
		s_bufferSize = 1;
	}
}

void WebType::copyToBuffer(char*& dataPtr) const
{
	setBuffer();
	memcpy(dataPtr, s_writeBuffer, s_bufferSize);
	dataPtr += s_bufferSize;
}

void WebType::writeToFile(std::fstream& outFile) const
{
	setBuffer();
	outFile.write(s_writeBuffer, s_bufferSize);
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
