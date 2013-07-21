#include "XKey.h"
#include "CryptStream.h"
#include <iostream>
#include <string>
#include <cstring>

#include <openssl/evp.h>

int main (int argc, char** argv) {
	if (argc < 2) {
		std::cerr << "Usage: XKey keystore_file\n";
		return -1;
	}
	
	OpenSSL_add_all_ciphers();

	const std::string filename (argv[1]);

	std::string key;
	std::cout << "Password: ";
	std::cin >> key;
	
	XKey::CryptStream crypt_source (filename, key, XKey::CryptStream::WRITE, XKey::BASE64_ENCODED);
	
	std::ostream stream (&crypt_source);
	stream.exceptions (std::ios_base::failbit | std::ios_base::badbit);
	
	using namespace XKey;
	Folder root;
	Folder * f = root.createSubfolder("ExampleFolder");
	f->addEntry (Entry("John", "example.com", "somepwd"));
	Folder *f2 = f->createSubfolder("SomeSubfolder");
	f2->addEntry(Entry("SomeUser", "jp-dev.org", "newpwd"));
	f->createSubfolder("Empty Subfolder");
	
	Writer w;
	if (!w.writeFile(stream, root)) {
		std::cerr << "Error: " << w.error() << "\n";
		return -1;
	}

	return 0;
}
