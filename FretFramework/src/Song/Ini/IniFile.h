#pragma once
#include "Song/Modifiers/Modifiers.h"

class IniFile
{
	static const StringModifier           s_DEFAULT_NAME;
	static const StringModifier           s_DEFAULT_ARTIST;
	static const StringModifier           s_DEFAULT_ALBUM;
	static const StringModifier           s_DEFAULT_GENRE;
	static const StringModifier           s_DEFAULT_YEAR;
	static const StringModifier           s_DEFAULT_CHARTER;
	static const NumberModifier<uint32_t> s_DEFAULT_SONG_LENGTH;

	bool m_isLoaded = false;
	std::vector<std::unique_ptr<TxtFileModifier>> m_modifiers;
	StringModifier* m_name;
	StringModifier* m_artist;
	StringModifier* m_album;
	StringModifier* m_genre;
	StringModifier* m_year;
	StringModifier* m_charter;
	NumberModifier<uint32_t>* m_song_length;

	std::unique_ptr<TxtFileModifier> extractModifierFromFile(TextTraversal& _traversal);

public:
	void load(std::filesystem::path directory);
	bool save(std::filesystem::path directory);
	bool wasLoaded() { return m_isLoaded; }
	void setBaseModifiers();

	template <class ModifierType = TxtFileModifier>
	ModifierType* const getModifier(const std::string_view modifierName) const
	{
		static_assert(std::is_base_of_v<TxtFileModifier, ModifierType>);

		for (const std::unique_ptr<TxtFileModifier>& modifier : m_modifiers)
			if (modifier->getName() == modifierName)
				return dynamic_cast<ModifierType*>(modifier.get());
		return nullptr;
	}

	// Removes all modifiers that share this name
	void removeAllOf(const std::string_view modifierName);

	template <class ModifierType = TxtFileModifier>
	void setModifier(const std::string_view modifierName, const auto& value)
	{
		ModifierType* modifier = getModifier<ModifierType>(modifierName);
		if (!modifier)
			modifier = static_cast<ModifierType*>(m_modifiers.emplace_back(std::make_unique<ModifierType>(modifierName)).get());

		*modifier = value;
	}

	StringModifier&           getArtist() const     { return *m_artist; }
	StringModifier&           getName() const       { return *m_name; }
	StringModifier&           getAlbum() const      { return *m_album; }
	StringModifier&           getGenre() const      { return *m_genre; }
	StringModifier&           getYear() const       { return *m_year; }
	StringModifier&           getCharter() const    { return *m_charter; }
	NumberModifier<uint32_t>& getSongLength() const { return *m_song_length; }
};

