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
#include <XKeyJsonSerialization.h>

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

using namespace XKey;
int main (int argc, char** argv) {
	if (argc < 2) {
		std::cerr << "Usage: write_test keystore_file\n";
		return -1;
	}
	
	OpenSSL_add_all_ciphers();

	const std::string filename (argv[1]);

	XKey::CryptStream crypt_source (filename, XKey::CryptStream::WRITE);
	
	{
		//std::cout << "Password: ";
		//std::string key = get_password();
		std::string key = "ABC";
		crypt_source.setEncryptionKey(key);
	}
	
	XKey::RootFolder_Ptr root = XKey::createRootFolder();
	root->addEntry (Entry{"SomeTitle1", "User", "Url", "PwdXXX", "E-Mail", "Comment"});
	XKey::Folder *f = root->createSubfolder ("SomeNewFolder");
	f->addEntry (Entry{"SubTitle2", "User2", "Url2", "PwdXXX2", "E-Mail2", "Comment2"});
	
	std::ostream stream (&crypt_source);
	stream.exceptions (std::ios_base::badbit);
	
	XKey::Writer writer;
	if (!writer.write(stream, *root, false)) {
		std::cerr << "Error: " << writer.error() << "\n";
		return 1;
	}

	return 0;
}
