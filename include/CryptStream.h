#pragma once

#include <streambuf>
#include <vector>
#include <memory>

struct bio_st;
struct evp_cipher_st;
struct evp_cipher_ctx_st;
struct env_md_st;
struct env_md_ctx_st;
struct evp_pkey_st;

namespace XKey {

enum ModeInfo {
	NO_OPTIONS = 0,
	/// Encode content in base64
	BASE64_ENCODED = 1,
	/// Encrypt file. If this is omitted, the file is stored in plaintext
	USE_ENCRYPTION = 2,
	/// If this is not set for reading, the cipher, IV and key iteration count
	/// must be set explicity for decryption to succeed. Omitting not recommended.
	EVALUATE_FILE_HEADER = 4
};

/**
 * @brief File stream handling encryption
 */
class CryptStream
	: public std::streambuf
{
public:
	static const int DEFAULT_KEY_ITERATION_COUNT = 100000;
	
	enum OperationMode {
		READ = 1,
		WRITE = 2
	};
	
	CryptStream (const std::string &filename, OperationMode open_mode,
		     int mode = BASE64_ENCODED | USE_ENCRYPTION | EVALUATE_FILE_HEADER);
	~CryptStream ();
	
	bool isEncrypted () const;
	
	bool isEncoded () const;
	
	/**
	 * @brief Set the key for encryption and decryption, if not specified for the constructor
	 * @param passphrase The passphrase to derive the key from
	 * @param cipherName OpenSSL-name of the encryption cipher to use. Defaults to AES in CTR mode.
	 * @param digestName Message-Digest algorithm to use. Defaults to SHA-256.
	 * @param iv initialization vector to use. If empty, a random one will be generated
	 * @param keyIterationCount number of iterations to derive the encryption key. If -1, defaults to DEFAULT_KEY_ITERATION_COUNT
	 * 
	 * This method uses PBKDF2 to derive the real encryption key from the passphrase
	 */
	void setEncryptionKey (const std::string &passphrase, const char *cipherName = nullptr,
			       const char *digestName = nullptr,
			       const char *iv = nullptr, int keyIterationCount = -1);
	
private:
	// overrides base class behaviour:
	// Get:
	int_type underflow();
	
	// Put:
	int_type overflow (int_type c);
	int sync();
	
	void _evaluateHeader (int *headerMode);
	void _writeHeader ();
	
	//streamsize xsputn(const char_type* __s, streamsize __n);
		
	// Disallow copying and copy -assignment
	CryptStream(const CryptStream &);
	CryptStream &operator= (const CryptStream &);
	
	std::vector<char> _buffer;
	std::shared_ptr<evp_cipher_ctx_st> _cipherCtx;
	std::shared_ptr<env_md_ctx_st> _mdCtx;
	struct bio_st *_bio_chain = 0;
	struct bio_st *_file_bio = 0;
	struct bio_st *_base64_bio = 0;
	OperationMode _mode;
	int _version;
	bool _initialized = false;
	//
	int _pbkdfIterationCount = DEFAULT_KEY_ITERATION_COUNT;
	std::string _iv;
	const evp_cipher_st *_cipher = 0;
	const env_md_st *_md = 0;
	std::shared_ptr<evp_pkey_st> _mdKey;
	
	struct BlockHead;
	void makeMessageDigest (const unsigned char *data, size_t length, unsigned char *mdOut);
};

}
