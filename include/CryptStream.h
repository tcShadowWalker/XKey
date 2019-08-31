#pragma once

#include <streambuf>
#include <vector>
#include <memory>

struct bio_st;
struct evp_cipher_st;
struct evp_cipher_ctx_st;
struct evp_md_st;
struct evp_md_ctx_st;
struct evp_pkey_st;

namespace XKey {

enum ModeInfo {
	NO_OPTIONS = 0,
	/// Encode content in base64
	BASE64_ENCODED = 1,
	/// Encrypt file. If this is omitted, the file is stored in plaintext
	/// This also prepends a cryptographic checksum for integrity validation.
	USE_ENCRYPTION = 2,
	/// If this is not set for reading, the cipher, digest algo, IV and key iteration count
	/// must be set explicity for decryption to succeed. Omitting not recommended.
	EVALUATE_FILE_HEADER = 4
};

/**
 * @brief File stream handling encryption
 * 
 * This class supports C++-style iostreams with transparent encryption,
 * integrity validation and base64-encoding.
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
	
	/**
	 * @brief Create new CryptStream streambuf object
	 * @param filename Path to file to open
	 * @param open_mode mode of operation. Read or write (but not both)
	 * @param mode Combination of @ref ModeInfo
	 */
	CryptStream (const std::string &filename, OperationMode open_mode,
		     int mode = BASE64_ENCODED | USE_ENCRYPTION | EVALUATE_FILE_HEADER);
	~CryptStream ();
	
	bool isEncrypted () const;
	
	bool isEncoded () const;
	
	/**
	 * @brief Set the key for encryption and decryption
	 * @param passphrase The passphrase to derive the key from
	 * @param cipherName OpenSSL-name of the encryption cipher to use. Defaults to AES in CTR mode.
	 * @param digestName Message-Digest algorithm to use. Defaults to SHA-256.
	 * @param iv initialization vector to use. If empty, a random one will be generated
	 * @param keyIterationCount number of iterations to derive the encryption key.
	 * If -1, defaults to DEFAULT_KEY_ITERATION_COUNT
	 * 
	 * This method uses PBKDF2 to derive the real encryption key from the passphrase
	 */
	void setEncryptionKey (const std::string &passphrase, const char *cipherName = nullptr,
			       const char *digestName = nullptr,
			       const char *iv = nullptr, int keyIterationCount = -1);
	
	/// Init crypto library after application startup
	static void InitCrypto ();
	
	static int Version ();
private:
	// Get:
	int_type underflow() override;
	
	// Put:
	int_type overflow (int_type c) override;
	int sync() override;
	
	/// Read header from file and put results in *headerMode
	void _evaluateHeader (int *headerMode);
	
	/**
	 * Write header to file in plaintext
	 * 
	 * The header contains cryptographic information needed for decryption.
	 * It contains the cipher and digest algo used, a randomized IV
	 * and the key-derivation-function iteration count.
	 */
	void _writeHeader ();
		
	// Disallow copying and copy -assignment
	CryptStream(const CryptStream &) = delete;
	CryptStream &operator= (const CryptStream &) = delete;
	
	std::vector<char> _buffer;
	std::unique_ptr<evp_cipher_ctx_st, void(*)(evp_cipher_ctx_st*)> _cipherCtx;
	std::unique_ptr<evp_md_ctx_st, void(*)(evp_md_ctx_st*)> _mdCtx;
	std::unique_ptr<evp_pkey_st, void(*)(evp_pkey_st*)> _mdKey;
	std::unique_ptr<bio_st, void(*)(bio_st*)> _bio_chain;
	struct bio_st *_file_bio = 0;
	OperationMode _mode;
	bool _isEncoded;
	int _version;
	bool _initialized = false;
	//
	int _pbkdfIterationCount = DEFAULT_KEY_ITERATION_COUNT;
	std::string _iv;
	const evp_cipher_st *_cipher = 0;
	const evp_md_st *_md = 0;
	
	struct BlockHead;
	void makeMessageDigest (const unsigned char *data, size_t length, unsigned char *mdOut);
	bio_st *bioChain () const { return &*_bio_chain; }
};

}
