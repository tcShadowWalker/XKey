#include "CryptStream.h"
#include "XKey.h"

#include <cassert>
#include <cstring> 
#include <stdexcept>

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

namespace XKey {

static int CURRENT_XKEY_FORMAT_VERSION = 13;
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
		if (sscanf(&in[i*2], "%02x", (unsigned int*)&out[i]) != 1)
			throw std::runtime_error ("Invalid hexadecimal format");
	}
	return out;
}

CryptStream::CryptStream (const std::string &filename, OperationMode open_mode, int m_info)
	: _buffer(std::max(256, put_back_) + put_back_),
	_mode(open_mode), _version(CURRENT_XKEY_FORMAT_VERSION)
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
	
	const bool useBase64Encode = (m_info & BASE64_ENCODED),
	           useEncryption = (m_info & USE_ENCRYPTION);
	
	if (useBase64Encode) {
		// Base 64 encoding is requested. Create the base64 filter bio and push it to the bio-stack
		_base64_bio = BIO_new(BIO_f_base64());
		if (!_base64_bio)
			throw std::runtime_error ("Could not create OpenSSL Encoding (Base64) BIO structure");
		_bio_chain = BIO_push(_base64_bio, _bio_chain);
	}
	
	if (useEncryption) {
		_cipherCtx.reset(EVP_CIPHER_CTX_new(), &EVP_CIPHER_CTX_free);
	} else {
		this->_initialized = true;
	}
}

CryptStream::~CryptStream () {
	sync();
	//
	if (_base64_bio) {
		if (_mode == WRITE)
			(void)BIO_flush(_base64_bio);
		BIO_pop(_base64_bio);
		BIO_vfree(_base64_bio);
	}
	if (_mode == WRITE)
		(void)BIO_flush(_file_bio);
	BIO_pop(_file_bio);
	BIO_vfree(_file_bio);
}

bool CryptStream::isEncrypted () const {
	return _cipherCtx != 0;
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
	int r = snprintf (buf, header_buf_size-1,
	                  "*167110* # v:%i # c:%.1i # e:%.1i # o:%i # ciph:%.30s # iv:%.64s # count:%i #",
	                  this->_version, isEncrypted(), isEncoded(), offset, EVP_CIPHER_name(_cipher),
	                  hexIV.c_str(), _pbkdfIterationCount);
	memset(buf+r, '*', header_buf_size-r-1);
	buf[header_buf_size-1] = '\n';
	if (BIO_write(_file_bio, buf, header_buf_size) <= 0)
		throw std::runtime_error ("Failed to write file header");
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
	char iv[64 + 1];
	int useEncryption, useBase64Encode;
	r = sscanf (buf, "*167110* # v:%i # c:%i # e:%i # o:%i # ciph:%30s # iv:%64s # count:%i #",
			&this->_version, &useEncryption, &useBase64Encode, &offset, cipherName, iv, &_pbkdfIterationCount);
	if (r != 7) {
		throw std::runtime_error ("Invalid file header. The file is probably not a valid XKey keystore.");
	}
	this->_cipher = EVP_get_cipherbyname(cipherName);
	if (!_cipher)
		throw std::runtime_error ("OpenSSL library does not provide requested Cipher mode from file header");
	if (strnlen((const char*)iv, 256) != (size_t)_cipher->iv_len * 2)
		throw std::runtime_error ("Invalid initialization vector length");
	if (_pbkdfIterationCount <= 0)
		throw std::runtime_error ("Invalid key iteration count");
	this->_iv.assign( hex2uc(iv, _cipher->iv_len * 2) );
	(void)BIO_seek (_file_bio, offset);
	*headerMode = ((useEncryption) ? (*headerMode | USE_ENCRYPTION) : (*headerMode & ~USE_ENCRYPTION));
	*headerMode = ((useBase64Encode) ? (*headerMode | BASE64_ENCODED) : (*headerMode & ~BASE64_ENCODED));
}

void CryptStream::setEncryptionKey (const std::string &passphrase, const char *cipherName,
				    const char *digestName, const char *ivParam, int keyIterationCount)
{
	if (!_cipherCtx)
		throw std::logic_error ("CryptSteam was not set up to use encryption");
	if (!_cipher) {
		if (cipherName && cipherName[0] != '\0') {
			this->_cipher = EVP_get_cipherbyname(cipherName);
		} else
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
	this->_md = (digestName) ? EVP_get_digestbyname(digestName) : EVP_sha256();
	_pbkdfIterationCount = (keyIterationCount != -1) ? keyIterationCount : DEFAULT_KEY_ITERATION_COUNT;
	if (_iv.length() != (size_t)_cipher->iv_len)
		throw std::runtime_error ("Invalid initialization vector length does not match cipher");
	// Use PBKDF2 to derive the encryption key from the passphrase. Use iv as Salt.
	unsigned char raw_key[_cipher->key_len + 1];
	int r = PKCS5_PBKDF2_HMAC_SHA1(passphrase.c_str(), passphrase.size(), (const unsigned char*)_iv.c_str(), _iv.length(),
									_pbkdfIterationCount, _cipher->key_len, raw_key);
	if (r != 1)
		throw std::runtime_error ("PBKDF2 algorithm to derive encryption key failed");
	// enc should be set to 1 for encryption and 0 for decryption.
	const int enc = (_mode == READ) ? 0 : 1;
	if (EVP_CipherInit(&*_cipherCtx, _cipher, raw_key, (const unsigned char*)_iv.c_str(), enc) != 1)
		throw std::runtime_error ("Failed to initialize cipher context");

	if (_mode == WRITE) {
		_writeHeader();
	}
	this->_initialized = true;
}

const size_t MaxCheckSumLength = EVP_MAX_MD_SIZE;
struct BlockHead {
	uint16_t length;
	unsigned char checksum[MaxCheckSumLength];
} __attribute__((packed, aligned(1))) ;

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

	BlockHead head;
	int n = BIO_read(_bio_chain, &head, sizeof(head.length) + EVP_MD_size(_md));
	if (n <= 0) {
		if (n == 0)
			return traits_type::eof();
		throw std::runtime_error ("Error reading from OpenSSL BIO");
	}
	
	if (_cipherCtx) {
		// Read to the provided buffer
		unsigned char bytes[head.length];
		n = BIO_read(_bio_chain, bytes, head.length);
		if (n <= 0) {
			if (n == 0)
				return traits_type::eof();
			throw std::runtime_error ("Error reading from OpenSSL BIO");
		}
		if (head.length >= _buffer.size() - (start - base))
			throw std::runtime_error ("Not enough space left in buffer");
		// start, _buffer.size() - (start - base)
		unsigned char compChecksum[MaxCheckSumLength];
		unsigned int checkSumLength;
		if (EVP_Digest(bytes, head.length, &compChecksum[0], &checkSumLength, _md, nullptr) != 1)
			throw std::runtime_error ("Failed to compute message digest");
		assert (checkSumLength == (unsigned int)EVP_MD_size(_md));
		if (memcmp (compChecksum, head.checksum, checkSumLength) != 0)
			throw std::runtime_error ("Message digest does not match message");
		int outLen;
		if (EVP_CipherUpdate (&*_cipherCtx, (unsigned char*)start, &outLen, bytes, head.length) != 1)
			throw std::runtime_error ("Failed to decrypt block");
		assert (outLen == n);
	} else {
		n = BIO_read(_bio_chain, start, _buffer.size() - (start - base));
		if (n <= 0) {
			if (n == 0)
				return traits_type::eof();
			throw std::runtime_error ("Error reading from OpenSSL BIO");
		}
	}
	
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
	
	assert (std::less_equal<char*> ()(pptr(), epptr()));
	size_t r;
	
	if (_cipherCtx) {
		BlockHead head;
		head.length = n;
		unsigned char cryptBlock[ n + EVP_MAX_BLOCK_LENGTH ];
		int length = EVP_MAX_BLOCK_LENGTH;
		if (EVP_CipherUpdate(&*_cipherCtx, cryptBlock, &length, (const unsigned char*)pbase(), n) != 1)
			throw std::runtime_error ("Failed to encrypt block");
		assert (length == n);
		
		uint checkSumLength;
		if (EVP_Digest(cryptBlock, length, &head.checksum[0], &checkSumLength, _md, nullptr) != 1)
			throw std::runtime_error ("Failed to compute message digest");
		assert (checkSumLength <= MaxCheckSumLength);
		
		r = BIO_write(_bio_chain, &head, sizeof(head.length) + checkSumLength);
		if (r <= 0)
			throw std::runtime_error ("Error writing to OpenSSL BIO (1)");
		r = BIO_write(_bio_chain, cryptBlock, length);
		if (r <= 0)
			throw std::runtime_error ("Error writing to OpenSSL BIO (2)");
	} else {
		r = BIO_write(_bio_chain, pbase(), n);
		if (r <= 0)
			throw std::runtime_error ("Error writing to OpenSSL BIO (2)");
	}
	return r;
}

int CryptStream::sync () {
	if (_mode == WRITE) {
		overflow(traits_type::eof());
	}
	return 0;
}

}
