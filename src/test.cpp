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

std::string get_password () {
	std::string pwd;
	struct termios t;
	tcgetattr(STDIN_FILENO, &t);
	t.c_lflag &= ~ECHO;
	tcsetattr(STDIN_FILENO, TCSANOW, &t);
	std::cin >> pwd;
	t.c_lflag |= ECHO;
	tcsetattr(STDIN_FILENO, TCSANOW, &t);
	return pwd;
}

int main (int argc, char** argv) {
	if (argc < 2) {
		std::cerr << "Usage: XKey keystore_file\n";
		return -1;
	}
	
	OpenSSL_add_all_ciphers();

	const std::string filename (argv[1]);

	XKey::CryptStream crypt_source (filename, std::string(), XKey::CryptStream::READ, XKey::BASE64_ENCODED | XKey::EVALUATE_FILE_HEADER);
	
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
