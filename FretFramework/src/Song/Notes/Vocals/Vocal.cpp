#include "Vocal.h"
#include "Variable Types/WebType.h"

void Vocal::init(char pitch, uint32_t duration)
{
	m_pitch = pitch;
	m_duration = duration;
}

void Vocal::init(TextTraversal& traversal)
{
	try
	{
		setLyric(traversal.extractLyric());
		if (uint32_t pitch; traversal.extract(pitch))
			init(pitch, traversal.extract<uint32_t>());
	}
	catch (Traversal::NoParseException)
	{
		throw EndofLineException();
	}
}

void Vocal::init(BCHTraversal& traversal)
{
	try
	{
		uint32_t length = traversal.extract<WebType::WebType_t>();
		setLyric(traversal.extractLyric(length));

		// Read pitch
		if (unsigned char pitch; traversal.extract(pitch))
			init(pitch, traversal.extract<WebType::WebType_t>());
	}
	catch (Traversal::NoParseException)
	{
		throw EndofEventException();
	}
}

void Vocal::setLyric(const std::u32string& lyric)
{
	m_lyric = lyric;
	if (m_lyric.length() > 255)
		m_lyric.resize(255);
}

void Vocal::setLyric(std::u32string&& lyric)
{
	m_lyric = std::move(lyric);
	if (m_lyric.length() > 255)
		m_lyric.resize(255);
}

void Vocal::save_cht(std::fstream& outFile) const
{
	outFile << " \"" << UnicodeString::U32ToStr(m_lyric) << '\"';
	if (m_pitch != 0)
		outFile << ' ' << (int)m_pitch << ' ' << m_duration;
	outFile << '\n';
}

void Vocal::save_bch(int lane, char*& outPtr) const
{
	*outPtr++ = (char)lane;
	const std::string str = UnicodeString::U32ToStr(m_lyric);
	WebType::copyToBuffer((uint32_t)str.length(), outPtr);
	memcpy(outPtr, str.data(), str.length());
	outPtr += str.length();

	if (m_pitch != 0)
	{
		*outPtr++ = m_pitch;
		WebType::copyToBuffer(m_duration, outPtr);
	}
}

bool Vocal::isEventPlayable(TextTraversal& traversal)
{
	return traversal.skipInt() && traversal.skipInt();
}

bool Vocal::isEventPlayable(BCHTraversal& traversal)
{
	try
	{
		const uint32_t lyricLength = traversal.extract<WebType::WebType_t>();
		traversal.move(lyricLength);
	}
	catch (...)
	{
		return false;
	}

	// Pitch AND sustain required
	if (traversal.testExtract<unsigned char>())
	{
		traversal.move(1);
		return traversal.testExtract<WebType::WebType_t>();
	}
	return false;
}
