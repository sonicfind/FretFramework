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
		void setValue(uint32_t value) throw(InvalidIntegerException);
		uint32_t getValue() const;
		int getSize() const;
		operator uint32_t() const;
	};

	void byteSwap_read(std::fstream& inFile, uint16_t& value)
	{
		inFile >> value;
		value = _byteswap_ushort(value);
	}

	void byteSwap_write(std::fstream& outFile, const uint16_t value)
	{
		outFile << _byteswap_ushort(value);
	}

	void byteSwap_read(std::fstream& inFile, uint32_t& value)
	{
		inFile >> value;
		value = _byteswap_ulong(value);
	}

	void byteSwap_write(std::fstream& outFile, const uint32_t value)
	{
		outFile << _byteswap_ulong(value);
	}

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
			const char m_syntax;
			bool m_isRunningStatus;

			MidiEvent(char syntax, bool running = false);
			virtual void fillFromFile(std::fstream& inFile) {}
			virtual ~MidiEvent() {}
			virtual uint32_t getSize() const;
		};

		struct MidiEvent_Note : public MidiEvent
		{
			char m_note;
			char m_velocity;

			using MidiEvent::MidiEvent;
			void fillFromFile(std::fstream& inFile);
			uint32_t getSize() const;
		};
		
		struct SysexEvent : public MidiEvent
		{
			VariableLengthQuantity m_length;
			char* m_data;

			SysexEvent(char syntax, std::fstream& inFile);
			void fillFromFile(std::fstream& inFile);
			~SysexEvent();
			uint32_t getSize() const;
		};

		struct MetaEvent : public SysexEvent
		{
			char m_type;
			MetaEvent(char type, std::fstream& inFile);
			uint32_t getSize() const;
		};

		struct MetaEvent_Text : public MetaEvent
		{
			std::string_view m_text;

			using MetaEvent::MetaEvent;
			void fillFromFile(std::fstream& inFile);
		};

		struct MetaEvent_ChannelPrefix : public MetaEvent
		{
			char m_prefix;

			using MetaEvent::MetaEvent;
			void fillFromFile(std::fstream& inFile);
		};

		struct MetaEvent_End : public MetaEvent
		{
			using MetaEvent::MetaEvent;
			void fillFromFile(std::fstream& inFile);
		};

		struct MetaEvent_Tempo : public MetaEvent
		{
			uint32_t m_microsecondsPerQuarter = 0;

			using MetaEvent::MetaEvent;
			void fillFromFile(std::fstream& inFile);
		};

		struct MetaEvent_SMPTE: public MetaEvent
		{
			struct
			{
				char hour;
				char minute;
				char second;
				char frame;
				char subframe;
			} m_smpte;

			using MetaEvent::MetaEvent;
			void fillFromFile(std::fstream& inFile);
		};

		struct MetaEvent_TimeSignature : public MetaEvent
		{
			struct
			{
				char numerator;
				char denominator;
				char ticksPerQuarterNote;
				char idk;
			} m_timeSig;

			using MetaEvent::MetaEvent;
			void fillFromFile(std::fstream& inFile);
		};

		struct MetaEvent_KeySignature : public MetaEvent
		{
			struct
			{
				char numFlatsOrSharps;
				char scaleType;
			} m_keySig;

			using MetaEvent::MetaEvent;
			void fillFromFile(std::fstream& inFile);
		};

		struct MetaEvent_Data : public MetaEvent
		{
			using MetaEvent::MetaEvent;
		};
		
	public:
		std::string_view m_name;
		std::map<uint32_t, std::vector<MidiEvent*>> m_events;
		
		MidiChunk_Track(std::fstream& inFile);
		~MidiChunk_Track();
		virtual void writeToFile(std::fstream& outFile) const;
	};
}
