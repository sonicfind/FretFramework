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
		VariableLengthQuantity& operator=(uint32_t value);
		operator uint32_t() const;
	};

	void byteSwap_read(std::fstream& inFile, uint16_t& value);
	void byteSwap_read(std::fstream& inFile, uint32_t& value);
	void byteSwap_write(std::fstream& outFile, uint16_t value);
	void byteSwap_write(std::fstream& outFile, uint32_t value);

	class MidiChunk
	{
		class ChunkTagNotFoundException : public std::exception
		{

		public:
			char const* what() const;
		};

	protected:
		struct
		{
			char type[5] = { 0 };
			uint32_t length;
		} m_header;

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

			MidiEvent(unsigned char syntax = false);
			// Used as the main event write function signature
			// Checks whether the syntax denotes a running event
			virtual void writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const;
			virtual ~MidiEvent() {}
		protected:
			// Used for Meta/SysexEvents bypassing the unnecessary syntax check
			void writeToFile(std::fstream& outFile, unsigned char& prevSyntax) const;
		};

		struct MidiEvent_Single : public MidiEvent
		{
			unsigned char m_value;

			MidiEvent_Single(unsigned char syntax, std::fstream& inFile);
			void writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const;
		};

		struct MidiEvent_Double : public MidiEvent
		{
			unsigned char m_value_1;
			unsigned char m_value_2;

			MidiEvent_Double(unsigned char syntax, std::fstream& inFile);
			void writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const;
		};

		struct MidiEvent_Note : public MidiEvent
		{
			unsigned char m_note;
			unsigned char m_velocity;

			MidiEvent_Note(unsigned char syntax, std::fstream& inFile);
			void writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const;
		};

		struct MidiEvent_ControlChange : public MidiEvent
		{
			unsigned char m_controller;
			unsigned char m_newValue;

			MidiEvent_ControlChange(unsigned char syntax, std::fstream& inFile);
			void writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const;
		};
		
		struct SysexEvent : public MidiEvent
		{
			VariableLengthQuantity m_length;
			char* m_data;

			SysexEvent(unsigned char syntax, std::fstream& inFile);
			void writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const;
			~SysexEvent();
		};

		struct MetaEvent : public MidiEvent
		{
			unsigned char m_type;
			VariableLengthQuantity m_length;
			MetaEvent(unsigned char type, std::fstream& inFile);
			void writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const;
		};

		struct MetaEvent_Text : public MetaEvent
		{
			std::string_view m_text;

			MetaEvent_Text(unsigned char type, std::fstream& inFile);
			void writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const;
			~MetaEvent_Text();
		};

		struct MetaEvent_ChannelPrefix : public MetaEvent
		{
			unsigned char m_prefix;

			MetaEvent_ChannelPrefix(std::fstream& inFile);
			void writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const;
		};

		struct MetaEvent_End : public MetaEvent
		{
			MetaEvent_End(std::fstream& inFile);
		};

		struct MetaEvent_Tempo : public MetaEvent
		{
			uint32_t m_microsecondsPerQuarter = 0;

			MetaEvent_Tempo(std::fstream& inFile);
			void writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const;
		};

		struct MetaEvent_SMPTE: public MetaEvent
		{
			unsigned char m_hour;
			unsigned char m_minute;
			unsigned char m_second;
			unsigned char m_frame;
			unsigned char m_subframe;

			MetaEvent_SMPTE(std::fstream& inFile);
			void writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const;
		};

		struct MetaEvent_TimeSignature : public MetaEvent
		{
			unsigned char m_numerator;
			unsigned char m_denominator;
			unsigned char m_metronome;
			unsigned char m_32ndsPerQuarter;

			MetaEvent_TimeSignature(std::fstream& inFile);
			void writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const;
		};

		struct MetaEvent_KeySignature : public MetaEvent
		{
			char m_numFlatsOrSharps;
			unsigned char m_scaleType;

			MetaEvent_KeySignature(std::fstream& inFile);
			void writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const;
		};

		struct MetaEvent_Data : public MetaEvent
		{
			char* m_data;
			MetaEvent_Data(unsigned char type, std::fstream& inFile);
			void writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const;
			~MetaEvent_Data();
		};
		
		std::map<uint32_t, std::vector<MidiEvent*>> m_events;
		
		MidiChunk_Track(std::fstream& inFile);
		~MidiChunk_Track();
		void writeToFile(std::fstream& outFile);
	};
}
