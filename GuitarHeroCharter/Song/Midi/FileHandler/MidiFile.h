#pragma once
#include <stdint.h>
#include <exception>
#include <fstream>
#include <vector>
#include <map>

namespace MidiFile
{
	class VariableLengthQuantity
	{
		class InvalidIntegerException : public std::exception
		{
		public:
			char const* what() const;
		};

		uint32_t m_value;
		int m_size;

	public:
		VariableLengthQuantity(std::fstream& inFile);
		VariableLengthQuantity(uint32_t value);
		void writeToFile(std::fstream& outFile) const;
		void setValue(uint32_t value);
		uint32_t getValue() const;
		int getSize() const;
		operator uint32_t() const;
	};

	void byteSwap_read(std::fstream& inFile, uint16_t& value);
	void byteSwap_read(std::fstream& inFile, uint32_t& value);
	void byteSwap_write(std::fstream& outFile, const uint16_t value);
	void byteSwap_write(std::fstream& outFile, const uint32_t value);

	class MidiChunk
	{
		class ChunkTagNotFoundException : public std::exception
		{

		public:
			char const* what() const;
		};
	public:
		struct
		{
			char type[5] = { 0 };
			uint32_t length;
		} m_header;

	protected:
		MidiChunk(const char (&type)[5], uint32_t length = 0);

	public:
		MidiChunk(std::fstream& inFile);
		virtual void writeToFile(std::fstream& outFile) const;
	};

	struct MidiChunk_Header : public MidiChunk
	{
		uint16_t m_format;
		uint16_t m_numTracks;
		uint16_t m_tickRate;

		MidiChunk_Header();
		MidiChunk_Header(std::fstream& inFile);
		virtual void writeToFile(std::fstream& outFile) const;
	};

	struct MidiChunk_Track : public MidiChunk
	{
	public:
		struct MidiEvent
		{
			const unsigned char m_syntax;
			bool m_isRunningStatus;

			MidiEvent(unsigned char syntax, bool running = false);
			virtual ~MidiEvent() {}
			virtual uint32_t getSize() const;
		};

		struct MidiEvent_Note : public MidiEvent
		{
			unsigned char m_note;
			unsigned char m_velocity;

			MidiEvent_Note(unsigned char syntax, std::fstream& inFile, bool running = false);
			uint32_t getSize() const;
		};
		
		struct SysexEvent : public MidiEvent
		{
			VariableLengthQuantity m_length;
			char* m_data;

			SysexEvent(unsigned char syntax, std::fstream& inFile, bool read = false);
			~SysexEvent();
			uint32_t getSize() const;
		};

		struct MetaEvent : public SysexEvent
		{
			unsigned char m_type;
			MetaEvent(unsigned char type, std::fstream& inFile, bool read = false);
			uint32_t getSize() const;
		};

		struct MetaEvent_Text : public MetaEvent
		{
			std::string_view m_text;

			MetaEvent_Text(unsigned char type, std::fstream& inFile);
		};

		struct MetaEvent_ChannelPrefix : public MetaEvent
		{
			unsigned char m_prefix;

			MetaEvent_ChannelPrefix(unsigned char type, std::fstream& inFile);
		};

		struct MetaEvent_End : public MetaEvent
		{
			using MetaEvent::MetaEvent;
		};

		struct MetaEvent_Tempo : public MetaEvent
		{
			uint32_t m_microsecondsPerQuarter = 0;

			MetaEvent_Tempo(unsigned char type, std::fstream& inFile);
		};

		struct MetaEvent_SMPTE: public MetaEvent
		{
			struct
			{
				unsigned char hour;
				unsigned char minute;
				unsigned char second;
				unsigned char frame;
				unsigned char subframe;
			} m_smpte;

			MetaEvent_SMPTE(unsigned char type, std::fstream& inFile);
		};

		struct MetaEvent_TimeSignature : public MetaEvent
		{
			struct
			{
				unsigned char numerator;
				unsigned char denominator;
				unsigned char ticksPerQuarterNote;
				unsigned char idk;
			} m_timeSig;

			MetaEvent_TimeSignature(unsigned char type, std::fstream& inFile);
		};

		struct MetaEvent_KeySignature : public MetaEvent
		{
			struct
			{
				unsigned char numFlatsOrSharps;
				unsigned char scaleType;
			} m_keySig;

			MetaEvent_KeySignature(unsigned char type, std::fstream& inFile);
		};

		struct MetaEvent_Data : public MetaEvent
		{
			MetaEvent_Data(unsigned char type, std::fstream& inFile);
		};
		
	public:
		std::string_view m_name;
		std::map<uint32_t, std::vector<MidiEvent*>> m_events;
		
		MidiChunk_Track(std::fstream& inFile);
		~MidiChunk_Track();
		virtual void writeToFile(std::fstream& outFile) const;
	};
}
