#pragma once

#include <string>
#include <vector>

namespace XKey {

static std::initializer_list<char> SpecialCharPool = {'!', '$', '%', '&', '/', '(', ')', '=', '?', '*', '+', '-',
	'_', '.', ',', ':', ';', '~', '{', '}', '[', ']', '^', '#', '<', '>'
};

/**
 * @brief Generator for random passphrases
 */
class PassphraseGenerator
{
public:
	/// @brief Allowed characters for a passphrase
	enum CharacterClass {
		/// Alphanumeric chars, lowercase
		CHAR_ALPHA = 1,
		/// Uppercase characters
		CHAR_LOWER_UPPERCASE = 2,
		/// Numeric characters
		CHAR_NUMERIC = 4,
		/// Special characters from #SpecialCharPool
		CHAR_SPECIAL = 8,
	};
	
	PassphraseGenerator ();
	
	/// @brief Set the allowed character class pool
	void setAllowedCharacters (int allowed_classes);
	
	/// @brief Disallow a specific character class
	void disallowCharacterType (CharacterClass c);
	
	void setMinLength (int minLen);
	void setMaxLength (int maxLen);
	
	/**
	 * @brief Generate a new random passphrase from the configured pool
	 * @param passphrase will contain the generated passphrase
	 */
	void generatePassphrase (std::string *passphrase);
	
private:
	std::vector<char> mCharPool;
	int mAllowed;
	int mMinLen;
	int mMaxLen;
	bool mRegenerate;
	
	void _generateCharacterPool ();
	
};

}
