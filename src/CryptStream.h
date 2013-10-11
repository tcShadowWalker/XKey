#ifndef XKEY_CRYPTSTREAM_H
#define XKEY_CRYPTSTREAM_H

#include <streambuf>
#include <vector>

struct bio_st;

namespace XKey {

enum ModeInfo {
	NO_OPTIONS = 0,
	BASE64_ENCODED = 1,
	USE_ENCRYPTION = 2,
	EVALUATE_FILE_HEADER = 4
};

class CryptStream
	: public std::streambuf
{
public:
	enum OperationMode {
		READ = 1,
		WRITE = 2
	};
	
	CryptStream (const std::string &filename, std::string key, OperationMode open_mode, int mode = BASE64_ENCODED | USE_ENCRYPTION | EVALUATE_FILE_HEADER);
	~CryptStream ();
	
	bool isEncrypted () const;
	
	bool isEncoded () const;
	
	void setEncryptionKey (std::string key);
	
private:
	// overrides base class behaviour:
	// Get:
	int_type underflow();
	
	// Put:
	int_type overflow (int_type c);
	int sync();
	
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
};


}

#endif