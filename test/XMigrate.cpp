#include <XKey.h>
#include <CryptStream.h>
#include <iostream>
#include <XKeyJsonSerialization.h>
//
#include "CryptStreamOld.h"

std::string get_password ();
bool writeToFile (const XKey::Folder &root, const std::string &filename, const std::string &key);

template<typename OldVersion>
bool readFromFile_OldVersion (XKey::RootFolder_Ptr *root, const std::string &filename, const std::string &key) {
	OldVersion crypt_source (filename, OldVersion::READ);
	crypt_source.setEncryptionKey(key);
	std::istream stream (&crypt_source);
	stream.exceptions (std::ios_base::badbit);
	
	*root = XKey::createRootFolder();
	XKey::Parser reader;
	if (!reader.read(stream, root->get())) {
		std::cerr << "Read Error: " << reader.error() << "\n";
		return false;
	}
	return true;
}

using namespace XKey;
int main (int argc, char** argv) {
	if (argc < 3) {
		std::cerr << "Usage: XMigrate input_keystore output_keystore\n";
		return -1;
	}
	typedef XKey::v12::CryptStream OldVer;
	
	const std::string inFile (argv[1]), outFile(argv[2]);
	XKey::CryptStream::InitCrypto();

	std::cout << "Migrating from CryptStream version " << OldVer::Version()
		<< " to " << XKey::CryptStream::Version() << "\n";
	
	std::cout << "Input Password: ";
	std::string key = get_password();
	std::cout << "\n";
	
	XKey::RootFolder_Ptr root;
	if (!readFromFile_OldVersion<OldVer> (&root, inFile, key))
		return 1;
	
	std::cout << "Output Password: ";
	key = get_password();
	std::cout << "\n";
	
	if (!writeToFile (*root, outFile, key))
		return 1;
	XKey::Writer::setRestrictiveFilePermissions (outFile);
	
	return 0;
}
