#include "XKeyGenerator.h"
#include <random>
#include <iostream>
#include <cassert>

namespace XKey {

static std::initializer_list<char> specialChars = {'!', '$', '%', '&', '/', '(', ')', '=', '?', '*', '+', '-',
	'_', '.', ',', ':', ';', '~', '{', '}', '[', ']', '^', '#', '<', '>'
};

PassphraseGenerator::PassphraseGenerator ()
	: mAllowed (CHAR_ALPHA | CHAR_LOWER_UPPERCASE | CHAR_NUMERIC | CHAR_SPECIAL), mMinLen(10), mMaxLen(14), mRegenerate(true)
{ }

void PassphraseGenerator::setAllowedCharacters (int allowed_classes) {
	this->mAllowed = allowed_classes;
	this->mRegenerate = true;
}

void PassphraseGenerator::disallowCharacterType (CharacterClass c) {
	assert (c == CHAR_ALPHA || c == CHAR_LOWER_UPPERCASE || c == CHAR_NUMERIC || c == CHAR_SPECIAL);
	int new_mask = mAllowed & ~c;
	if (new_mask != mAllowed) {
		mAllowed = new_mask;
		mRegenerate = true;
	}
}

void PassphraseGenerator::setMinLength (int minLen) {
	assert (minLen >= 0);
	this->mMinLen = minLen;
	this->mRegenerate = true;
}

void PassphraseGenerator::setMaxLength (int maxLen) {
	assert (maxLen >= 0);
	this->mMaxLen = maxLen;
	this->mRegenerate = true;
}

void PassphraseGenerator::generatePassphrase (std::string *passphrase) {
	assert (passphrase);
	if (mRegenerate) {
		_generateCharacterPool();
	}
	const int numSpecialChars = specialChars.size();
	std::random_device rd;
	std::mt19937 rng (rd());
	std::uniform_int_distribution<int> type_dist (1,1);
	// If special characters are allowed, we want them with a probability of 1/5
	if (mAllowed & CHAR_SPECIAL)
		type_dist = std::uniform_int_distribution<int> (0, 4);
	std::uniform_int_distribution<int> char_dist (0, mCharPool.size()-1);
	passphrase->resize( char_dist( rng, std::uniform_int_distribution<int>::param_type( mMinLen, mMaxLen ) ) );
	for (auto &it : *passphrase) {
		// If we got a 0 from the random generator, we want a special char
		if (type_dist(rng) != 0) {
			it = mCharPool[char_dist(rng)];
		} else {
			it = *(specialChars.begin()+char_dist( rng, std::uniform_int_distribution<int>::param_type( 0, numSpecialChars ) ));
		}
	}
}

void PassphraseGenerator::_generateCharacterPool () {
	// We do not include special chars into the pool
	int size = 0;
	if (mAllowed & CHAR_ALPHA)
		size += (mAllowed & CHAR_LOWER_UPPERCASE) ? 26 * 2 : 26;
	if (mAllowed & CHAR_NUMERIC)
		size += 10;
	mCharPool.resize (size);
	char *ptr = &*mCharPool.begin();
	if (mAllowed & CHAR_ALPHA) {
		for (char a = 'a'; a <= 'z'; )
			*ptr++ = a++;
	}
	if (mAllowed & CHAR_LOWER_UPPERCASE) {
		for (char a = 'A'; a <= 'Z'; )
			*ptr++ = a++;
	}
	if (mAllowed & CHAR_NUMERIC) {
		for (int a = '0'; a <= '9'; )
			*ptr++ = a++;
	}
	assert (ptr == &*mCharPool.end());
	mRegenerate = false;
}

}