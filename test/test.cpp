#include "XKey.h"
#include "CryptStream.h"
#include <iostream>
#include <string>
#include <cstring>
#include <openssl/evp.h>
// Needed for no-echo password query
#include <termios.h>
#include <stdio.h>
#include <unistd.h>

std::string get_password ();

int main (int argc, char** argv) {
	if (argc < 2) {
		std::cerr << "Usage: XKey keystore_file\n";
		return -1;
	}

	const std::string filename (argv[1]);

	XKey::CryptStream::InitCrypto();
	XKey::CryptStream crypt_source (filename, XKey::CryptStream::READ, XKey::BASE64_ENCODED | XKey::EVALUATE_FILE_HEADER);
	
	if (crypt_source.isEncrypted()) {
		std::cout << "Password: ";
		std::string key = get_password();
		crypt_source.setEncryptionKey(key);
	}
	
	std::istream stream (&crypt_source);
	stream.exceptions (std::ios_base::badbit);
	
	while (!stream.eof()) {
		std::string s;
		stream >> s;
		std::cout << s;
	}
	std::cout << "\n";

	return 0;
}
