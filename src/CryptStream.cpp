#include "CryptStream.h"
#include "XKey.h"

#include <cassert>
#include <cstring> 
#include <stdexcept>

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include <iostream>

namespace XKey {

static int CURRENT_XKEY_FORMAT_VERSION = 11;
static const int put_back_ = 8;

/// Unsigned char string to hexadecimal representation
static std::string uc2hex (const unsigned char *in, int in_length) {
	assert (in_length >= 0);
	std::string out ( (size_t) in_length * 2, '\0');
	for (int i = 0; i < in_length; ++i) {
		sprintf((char*)&out[i * 2], "%02x", in[i]);
	}
	return out;
}

/// Signed char hexadecimal notation to unsigned char. in_length must be even, output string has half the length
static std::string hex2uc (const char *in, int in_length) {
	assert (in_length >= 0 && in_length % 2 == 0);
	in_length /= 2;
	std::string out ( (size_t) in_length, '\0');
	for (int i = 0; i < in_length; ++i) {
		if (sscanf(&in[i*2], "%02x", &out[i]) != 1)
			throw std::runtime_error ("Invalid hexadecimal format");
	}
	return out;
}

CryptStream::CryptStream (const std::string &filename, OperationMode open_mode, int m_info)
	: _buffer(std::max(256, put_back_) + put_back_), _crypt_bio(0), _file_bio(0), _base64_bio(0), _mode(open_mode), _initialized(false),
	_version(CURRENT_XKEY_FORMAT_VERSION), _pbkdfIterationCount(DEFAULT_KEY_ITERATION_COUNT), _cipher(nullptr)
{	
	char *end = &_buffer.front() + _buffer.size();
	setg(end, end, end);
	
	setp(&_buffer.front(), end - 2); // -1 to make overflow() easier, another -1 to terminate with null
	
	_file_bio = BIO_new_file(filename.c_str(), (_mode == READ) ? "rb" : "wb");
	if (!_file_bio) {
		if (_mode == READ)
			throw std::runtime_error ("Could not open keystore file. Does the file exist and is it readable?");
		else
			throw std::runtime_error ("Could not open keystore file for writing. Please check filesystem permissions.");
	}
	
	_bio_chain = _file_bio;
	
	if (_mode == READ && (m_info & EVALUATE_FILE_HEADER)) {
		_evaluateHeader(&m_info);
	}
	
	int useBase64Encode = (m_info & BASE64_ENCODED), useEncryption = (m_info & USE_ENCRYPTION);
	
	if (useBase64Encode) {
		// Base 64 encoding is requested. Create the base64 filter bio and push it to the bio-stack
		_base64_bio = BIO_new(BIO_f_base64());
		if (!_base64_bio)
			throw std::runtime_error ("Could not create OpenSSL Encoding (Base64) BIO structure");
		_bio_chain = BIO_push(_base64_bio, _bio_chain);
	}
	
	if (useEncryption) {
		_crypt_bio = BIO_new(BIO_f_cipher());
		if (!_crypt_bio)
			throw std::runtime_error ("Could not create OpenSSL Crypt-filter BIO structure");
	} else
		this->_initialized = true;
}

CryptStream::~CryptStream () {
	sync();
	//
	if (_crypt_bio) {
		if (_mode == WRITE)
			BIO_flush(_crypt_bio);
		BIO_pop(_crypt_bio);
		BIO_vfree(_crypt_bio);
	}
	if (_base64_bio) {
		if (_mode == WRITE)
			BIO_flush(_base64_bio);
		BIO_pop(_base64_bio);
		BIO_vfree(_base64_bio);
	}
	if (_mode == WRITE)
		BIO_flush(_file_bio);
	BIO_pop(_file_bio);
	BIO_vfree(_file_bio);
}

bool CryptStream::isEncrypted () const {
	return _crypt_bio != 0;
}

bool CryptStream::isEncoded () const {
	return _base64_bio != 0;
}

const int header_buf_size = 512;

void CryptStream::_writeHeader () {
	assert (_mode == WRITE);
	assert (_file_bio); // Operate on _file_bio
	
	char buf[header_buf_size+1];
	// Write this header directly to the file, without encoding or encryption
	const int offset = header_buf_size;
	std::string hexIV = uc2hex((const unsigned char*)_iv.c_str(), _iv.length());
	int r = snprintf (buf, header_buf_size-1, "*167110* # v:%i # c:%.1i # e:%.1i # o:%i # ciph:%.30s # iv:%.256s # count:%i #",
						this->_version, isEncrypted(), isEncoded(), offset, EVP_CIPHER_name(_cipher),
						hexIV.c_str(), _pbkdfIterationCount);
	memset(buf+r, '*', header_buf_size-r-1);
	buf[header_buf_size-1] = '\n';
	BIO_write(_file_bio, buf, header_buf_size);
}

void CryptStream::_evaluateHeader (int *headerMode) {
	assert (_mode == READ);
	assert (_file_bio); // Operate on _file_bio
	assert (headerMode);
	
	char buf[header_buf_size+1];
	int r = BIO_read(_file_bio, buf, header_buf_size);
	buf[r] = '\0';
	int offset = 0;
	const int ciphNameLen = 30;
	char cipherName[ciphNameLen + 1];
	char iv[256 + 1];
	int useEncryption, useBase64Encode;
	if (r = sscanf (buf, "*167110* # v:%i # c:%i # e:%i # o:%i # ciph:%30s # iv:%256s # count:%i #",
			&this->_version, &useEncryption, &useBase64Encode, &offset, cipherName, iv, &_pbkdfIterationCount) != 7)
	{
		throw std::runtime_error ("Invalid file header. The file is probably not a valid XKey keystore.");
	}
	this->_cipher = EVP_get_cipherbyname(cipherName);
	if (!_cipher)
		throw std::runtime_error ("OpenSSL library does not provide requested Cipher mode from file header");
	if (strnlen((const char*)iv, 256) != _cipher->iv_len * 2)
		throw std::runtime_error ("Invalid initialization vector length");
	this->_iv.assign( hex2uc(iv, _cipher->iv_len * 2) );
	BIO_seek (_file_bio, offset);
	*headerMode = ((useEncryption) ? (*headerMode | USE_ENCRYPTION) : (*headerMode & ~USE_ENCRYPTION));
	*headerMode = ((useBase64Encode) ? (*headerMode | BASE64_ENCODED) : (*headerMode & ~BASE64_ENCODED));
}

void CryptStream::setEncryptionKey (std::string passphrase, const char *cipherName, const char *ivParam, int keyIterationCount) {
	if (!_crypt_bio)
		throw std::logic_error ("CryptStream was not set up to use encryption");
	if (cipherName && cipherName[0] != '\0') {
		this->_cipher = EVP_get_cipherbyname(cipherName); 
	} else {
		if (!_cipher)
			this->_cipher = EVP_aes_256_ctr();
	}
	if (!this->_cipher)
		throw std::runtime_error ("OpenSSL library does not provide requested Cipher mode");
	if (ivParam) {
		this->_iv = ivParam;
	} else {
		if (this->_iv.size() == 0) {
			this->_iv.resize(_cipher->iv_len);
			if (!RAND_bytes((unsigned char*)&_iv[0], _cipher->iv_len))
				throw std::runtime_error ("Could not generate random bytes to create initialization vector");
		}
	}
	if (_iv.length() != _cipher->iv_len)
		throw std::runtime_error ("Invalid initialization vector length does not match cipher");
	// Use PBKDF2 to get the derive encryption key from the passphrase
	unsigned char raw_key[_cipher->key_len + 1];
	int r = PKCS5_PBKDF2_HMAC_SHA1(passphrase.c_str(), passphrase.size(), (const unsigned char*)_iv.c_str(), _iv.length(),
									keyIterationCount, _cipher->key_len, raw_key);
	if (r != 1)
		throw std::runtime_error ("PBKDF2 algorithm to derive encryption key failed");
	// enc should be set to 1 for encryption and zero for decryption.
	int enc = (_mode == READ) ? 0 : 1;
	BIO_set_cipher (_crypt_bio, _cipher, raw_key, (const unsigned char*)_iv.c_str(), enc);
	_bio_chain = BIO_push(_crypt_bio, _bio_chain);
	
	// Have a magic string in the beggining to find out if decryption actually works
	// Cipher must be secure against known-plaintext attacks
	const std::string encBuffer ("-- FILE -- 85gxk9d7 --");
	if (_mode == WRITE) {
		BIO_write(_bio_chain, encBuffer.data(), encBuffer.size());
	} else {
		std::string rBuffer (encBuffer.size(), '\0');
		r = BIO_read(_bio_chain, &rBuffer[0], encBuffer.size());
		
		if (r != encBuffer.size())
			throw std::runtime_error ("Unexpected file read error");
		if (encBuffer != rBuffer)
			throw std::runtime_error ("Invalid key");
	}
	if (_mode == WRITE) {
		_writeHeader();
	}
	this->_initialized = true;
}

CryptStream::int_type CryptStream::underflow() {
	if (_mode != READ)
		throw std::logic_error ("underflow unexpected on write-only CryptStream");
	if (gptr() < egptr()) // buffer not exhausted
        return traits_type::to_int_type(*gptr());

	char *base = &_buffer.front();
	char *start = base;

	if (eback() == base) { // true when this isn't the first fill
		// Make arrangements for putback characters
		std::memmove (base, egptr() - put_back_, put_back_);
		start += put_back_;
	}

	// start is now the start of the buffer, proper.
	// Read to the provided buffer
	size_t n = BIO_read(_bio_chain, start, _buffer.size() - (start - base));
	if (n == 0)
		return traits_type::eof();
	else if (n < 0)
		throw std::runtime_error ("Error reading from OpenSSL BIO");

	// Set buffer pointers
	setg(base, start, start + n);

	return traits_type::to_int_type(*gptr());
}

CryptStream::int_type CryptStream::overflow (int_type ch) {
	if (_mode != WRITE)
		throw std::logic_error ("overflow unexpected on read-only CryptStream");
	
	if (ch != traits_type::eof()) {
		*pptr() = ch;
		pbump(1);
	}
	std::ptrdiff_t n = pptr() - pbase();
	pbump(-n);
	
	assert (std::less_equal<char *>()(pptr(), epptr()));
	
	size_t r = BIO_write(_bio_chain, pbase(), n);
	if (r <= 0)
		throw std::runtime_error ("Error writing to OpenSSL BIO");
	return r;
}

int CryptStream::sync () {
	if (_mode == WRITE) {
		overflow(traits_type::eof());
	}
	return 0;
}

}
