#ifndef XKEY_CRYPTSTREAM_H
#define XKEY_CRYPTSTREAM_H

#include <streambuf>
#include <vector>

struct bio_st;
struct evp_cipher_st;

namespace XKey {

enum ModeInfo {
	NO_OPTIONS = 0,
	BASE64_ENCODED = 1,
	USE_ENCRYPTION = 2,
	EVALUATE_FILE_HEADER = 4
};

/**
 * @brief File stream handling encryption
 */
class CryptStream
	: public std::streambuf
{
public:
	static const int DEFAULT_KEY_ITERATION_COUNT = 20000;
	enum OperationMode {
		READ = 1,
		WRITE = 2
	};
	
	CryptStream (const std::string &filename, OperationMode open_mode, int mode = BASE64_ENCODED | USE_ENCRYPTION | EVALUATE_FILE_HEADER);
	~CryptStream ();
	
	bool isEncrypted () const;
	
	bool isEncoded () const;
	
	/**
	 * @brief Set the key for encryption and description, if not specified for the constructor
	 * @param passphrase The passphrase to dervie the key from
	 * @param cipherName OpenSSL-name of the encryption cipher to use. Defaults to AES in CTR mode.
	 * 
	 * This method uses PBKDF2 to derive the real encryption key from the passphrase
	 */
	void setEncryptionKey (std::string passphrase, const char *cipherName = nullptr, const char *iv = nullptr, int keyIterationCount = DEFAULT_KEY_ITERATION_COUNT);
	
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
	struct bio_st *_bio_chain;
	struct bio_st *_crypt_bio;
	struct bio_st *_file_bio;
	struct bio_st *_base64_bio;
	OperationMode _mode;
	int _version;
	bool _initialized;
	//
	int _pbkdfIterationCount;
	std::string _iv;
	const evp_cipher_st *_cipher;
};


}

#endif