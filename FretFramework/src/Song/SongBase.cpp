#include "SongBase.h"

// 0 -  Guitar 5
// 1 -  Guitar 6
// 2 -  Bass 5
// 3 -  Bass 6
// 4 -  Rhythm
// 5 -  Co-op
// 6 -  Keys
// 7 -  Drums 4
// 8 -  Drums 5
// 9 -  Vocals
// 10 - Harmonies
NoteTrack* const SongBase::s_noteTracks[11] =
{
	new InstrumentalTrack<GuitarNote<5>>("[LeadGuitar]", 0),
	new InstrumentalTrack<GuitarNote<6>>("[LeadGuitar_GHL]", 1),
	new InstrumentalTrack<GuitarNote<5>>("[BassGuitar]", 2),
	new InstrumentalTrack<GuitarNote<6>>("[BassGuitar_GHL]", 3),
	new InstrumentalTrack<GuitarNote<5>>("[RhythmGuitar]", 4),
	new InstrumentalTrack<GuitarNote<5>>("[CoopGuitar]", 5),
	new InstrumentalTrack<Keys<5>>("[Keys]", 6),
	new InstrumentalTrack<DrumNote<4, DrumPad_Pro>>("[Drums_4Lane]", 7),
	new InstrumentalTrack<DrumNote<5, DrumPad>>("[Drums_5Lane]", 8),
	new VocalTrack<1>("[Vocals]", 9),
	new VocalTrack<3>("[Harmonies]", 10),
};

FileHasher SongBase::s_fileHasher;

SongBase::SongBase()
	: m_hash(std::make_shared<MD5>()) {}

void SongBase::wait()
{
	m_hash->wait();
}

void SongBase::deleteTracks()
{
	for (NoteTrack* track : s_noteTracks)
		delete track;
}
