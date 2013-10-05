#ifndef XKEY_PASSPHRASE_GENERATOR_H
#define XKEY_PASSPHRASE_GENERATOR_H

#include <string>
#include <vector>

namespace XKey {

class PassphraseGenerator
{
public:
	enum CharacterClass {
		CHAR_ALPHA = 1,
		CHAR_LOWER_UPPERCASE = 2,
		CHAR_NUMERIC = 4,
		CHAR_SPECIAL = 8,
	};
	
	PassphraseGenerator ();
	
	void setAllowedCharacters (int allowed_classes);
	void disallowCharacterType (CharacterClass c);
	
	void setMinLength (int minLen);
	void setMaxLength (int maxLen);
	
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

#endif
