#pragma once
#include "Variable Types/VariableLengthQuantity.h"
#include <vector>

namespace MidiFile
{
	void byteSwap_read(std::fstream& inFile, uint16_t& value);
	void byteSwap_read(std::fstream& inFile, uint32_t& value);
	void byteSwap_write(std::fstream& outFile, uint16_t value);
	void byteSwap_write(std::fstream& outFile, uint32_t value);

	class MidiChunk
	{
	protected:
		struct
		{
			char type[5] = { 0 };
			uint32_t length;
		} m_header;

		MidiChunk(const char (&type)[5], uint32_t length = 0);

	public:
		virtual void writeToFile(std::fstream& outFile) const;
		uint32_t getLength() const { return m_header.length; }
	};

	struct MidiChunk_Header : public MidiChunk
	{
		uint16_t m_format;
		uint16_t m_numTracks;
		uint16_t m_tickRate;

		MidiChunk_Header(uint16_t tickRate);
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

		struct MidiEvent_Note : public MidiEvent
		{
			unsigned char m_note;
			unsigned char m_velocity;

			// Going the route of using Note On and Note Off syntaxes instead of velocity
			MidiEvent_Note(unsigned char syntax, unsigned char note, unsigned char velocity = 100);
			void writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const;
		};

		struct MidiEvent_ControlChange : public MidiEvent
		{
			unsigned char m_controller;
			unsigned char m_newValue;

			void writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const;
		};
		
		struct SysexEvent : public MidiEvent
		{
			const unsigned char m_tag[3] = { 'P', 'S', 0 };
			const unsigned char m_messageID = 0;
			unsigned char m_difficulty = 0;
			unsigned char m_phraseID = 1;
			unsigned char m_status = 1;
			const unsigned char m_end = 0xF7;

			SysexEvent(unsigned char diff, unsigned char id, unsigned char status);
			void writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const;
		};

		struct MetaEvent : public MidiEvent
		{
			unsigned char m_type;
			uint32_t m_length;

			MetaEvent(unsigned char type, uint32_t length = 0);
			void writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const;
		};

		struct MetaEvent_Text : public MetaEvent
		{
			std::string m_text;

			MetaEvent_Text(unsigned char type, const std::string& text);
			void writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const;
		};

		struct MetaEvent_End : public MetaEvent
		{
			MetaEvent_End();
		};

		struct MetaEvent_Tempo : public MetaEvent
		{
			uint32_t m_microsecondsPerQuarter = 0;

			MetaEvent_Tempo(uint32_t mpq);
			void writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const;
		};

		struct MetaEvent_TimeSignature : public MetaEvent
		{
			unsigned char m_numerator;
			unsigned char m_denominator;
			unsigned char m_metronome;
			unsigned char m_32ndsPerQuarter;

			MetaEvent_TimeSignature(unsigned char num, unsigned char denom, unsigned char met);
			void writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const;
		};

		struct MetaEvent_KeySignature : public MetaEvent
		{
			char m_numFlatsOrSharps;
			unsigned char m_scaleType;

			void writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const;
		};
		
		MidiChunk_Track();
		MidiChunk_Track(const std::string& name);
		~MidiChunk_Track();
		void writeToFile(std::fstream& outFile);
		void addEvent(uint32_t position, MidiEvent* ev);

	private:
		std::string_view m_name;
		std::vector<std::pair<uint32_t, std::vector<MidiEvent*>>> m_events;
	};
}
