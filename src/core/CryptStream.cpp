#include "CryptStream.h"
#include "XKey.h"

#include <cassert>
#include <cstring> 
#include <stdexcept>

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

namespace XKey {

static int CURRENT_XKEY_FORMAT_VERSION = 14;
static const int put_back_ = 8;

void CryptStream::InitCrypto () {
	OpenSSL_add_all_ciphers();
	OpenSSL_add_all_digests();
}

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
	_cipherCtx(nullptr, &EVP_CIPHER_CTX_free),
	_mdCtx(nullptr, &EVP_MD_CTX_destroy),
	_mdKey(nullptr, &EVP_PKEY_free),
	_bio_chain(nullptr, &BIO_free_all),
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
	
	_bio_chain.reset (_file_bio);
	
	if (_mode == READ && (m_info & EVALUATE_FILE_HEADER)) {
		_evaluateHeader(&m_info);
	}
	
	const bool useBase64Encode = _isEncoded = (m_info & BASE64_ENCODED),
	           useEncryption = (m_info & USE_ENCRYPTION);
	
	if (useBase64Encode) {
		// Base 64 encoding is requested. Create the base64 filter bio and push it to the bio-stack
		BIO *_base64_bio = BIO_new(BIO_f_base64());
		if (!_base64_bio)
			throw std::runtime_error ("Could not create OpenSSL Encoding (Base64) BIO structure");
		_bio_chain.release();
		_bio_chain.reset( BIO_push(_base64_bio, _file_bio));
	}
	
	if (useEncryption) {
		_cipherCtx.reset(EVP_CIPHER_CTX_new());
		_mdCtx.reset(EVP_MD_CTX_create());
	} else {
		this->_initialized = true;
	}
}

CryptStream::~CryptStream () {
	sync();
	if (_mode == WRITE)
		(void)BIO_flush(bioChain());
}

bool CryptStream::isEncrypted () const {
	return _cipherCtx != 0;
}

bool CryptStream::isEncoded () const {
	return _isEncoded;
}

const int HeaderBufSize = 512;

void CryptStream::_writeHeader () {
	assert (_mode == WRITE);
	assert (_file_bio); // Operate on _file_bio
	
	char buf[HeaderBufSize+1];
	// Write this header directly to the file, without encoding or encryption
	const int offset = HeaderBufSize;
	std::string hexIV = uc2hex((const unsigned char*)_iv.c_str(), _iv.length());
	int r = snprintf (buf, HeaderBufSize-1,
	                  "*167110* # v:%i # c:%.1i # e:%.1i # o:%i # ciph:%.30s # iv:%.64s # count:%i # md:%.30s #",
	                  this->_version, isEncrypted(), isEncoded(), offset, EVP_CIPHER_name(_cipher),
	                  hexIV.c_str(), _pbkdfIterationCount, EVP_MD_name(_md));
	memset(buf+r, '*', HeaderBufSize-r-1);
	buf[HeaderBufSize-1] = '\n';
	if (BIO_write(_file_bio, buf, HeaderBufSize) <= 0)
		throw std::runtime_error ("Failed to write file header");
}

void CryptStream::_evaluateHeader (int *headerMode) {
	assert (_mode == READ);
	assert (_file_bio); // Operate on _file_bio
	assert (headerMode);
	
	char buf[HeaderBufSize+1];
	int r = BIO_read(_file_bio, buf, HeaderBufSize);
	buf[r] = '\0';
	int offset = 0;
	const int ciphNameLen = 30;
	char cipherName[ciphNameLen + 1];
	char digestName[ciphNameLen + 1];
	char iv[64 + 1];
	int useEncryption, useBase64Encode;
	r = sscanf (buf, "*167110* # v:%i # c:%i # e:%i # o:%i # ciph:%30s # iv:%64s # count:%i # md:%30s #",
	            &this->_version, &useEncryption, &useBase64Encode, &offset, cipherName,
	            iv, &_pbkdfIterationCount, digestName);
	if (r != 8) {
		throw std::runtime_error ("Invalid file header. The file is probably not a valid XKey keystore.");
	}
	cipherName[ciphNameLen] = digestName[ciphNameLen] = '\0';
	this->_cipher = EVP_get_cipherbyname(cipherName);
	if (!_cipher)
		throw std::runtime_error ("OpenSSL library does not provide requested Cipher mode from file header");
	this->_md = EVP_get_digestbyname(digestName);
	if (!_md)
		throw std::runtime_error ("OpenSSL library does not provide requested Digest algorithm from file header");
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
	_mdKey.reset(EVP_PKEY_new_mac_key(EVP_PKEY_HMAC, nullptr, raw_key, _cipher->key_len));
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
struct CryptStream::BlockHead {
	uint16_t length;
	unsigned char checksum[MaxCheckSumLength];
} __attribute__((packed, aligned(1))) ;

void CryptStream::makeMessageDigest (const unsigned char *data, size_t length, unsigned char *mdOut) {
	if (EVP_DigestSignInit(&*_mdCtx, nullptr, _md, nullptr, &*_mdKey) != 1)
		throw std::runtime_error ("Failed to initialize message digest");
	if (EVP_DigestSignUpdate (&*_mdCtx, data, length) != 1)
		throw std::runtime_error ("Failed to update message digest");
	size_t checkSumLen = EVP_MD_size(_md);
	if (EVP_DigestSignFinal(&*_mdCtx, &mdOut[0], &checkSumLen) != 1)
		throw std::runtime_error ("Failed to finalize message digest");
	assert (checkSumLen == (size_t)EVP_MD_size(_md));
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

	BlockHead head;
	int n = BIO_read(bioChain(), &head, sizeof(head.length) + EVP_MD_size(_md));
	if (n <= 0) {
		if (n == 0)
			return traits_type::eof();
		throw std::runtime_error ("Error reading from OpenSSL BIO");
	}
	
	if (_cipherCtx) {
		// Read to the provided buffer
		unsigned char bytes[head.length];
		n = BIO_read(bioChain(), bytes, head.length);
		if (n <= 0) {
			if (n == 0)
				return traits_type::eof();
			throw std::runtime_error ("Error reading from OpenSSL BIO");
		}
		if (head.length >= _buffer.size() - (start - base))
			throw std::runtime_error ("Not enough space left in buffer");
		unsigned char compChecksum[MaxCheckSumLength];
		makeMessageDigest(bytes, head.length, compChecksum);
		if (CRYPTO_memcmp (compChecksum, head.checksum, EVP_MD_size(_md)) != 0) {
			throw std::runtime_error ("Message digest does not match message");
		}
		int outLen;
		if (EVP_CipherUpdate (&*_cipherCtx, (unsigned char*)start, &outLen, bytes, head.length) != 1)
			throw std::runtime_error ("Failed to decrypt block");
		assert (outLen == n);
	} else {
		n = BIO_read(bioChain(), start, _buffer.size() - (start - base));
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
		makeMessageDigest (cryptBlock, length, &head.checksum[0]);
		
		r = BIO_write(bioChain(), &head, sizeof(head.length) + EVP_MD_size(_md));
		if (r <= 0)
			throw std::runtime_error ("Error writing to OpenSSL BIO (1)");
		r = BIO_write(bioChain(), cryptBlock, length);
		if (r <= 0)
			throw std::runtime_error ("Error writing to OpenSSL BIO (2)");
	} else {
		r = BIO_write(bioChain(), pbase(), n);
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
