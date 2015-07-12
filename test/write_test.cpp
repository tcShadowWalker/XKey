#include "XKey.h"
#include "CryptStream.h"
#include <iostream>

std::string get_password ();
void print_folder (const XKey::Folder &f, int print_options, int depth = 0, std::ostream &out = std::cout);

bool writeToFile (const XKey::Folder &root, const std::string &filename, const std::string &key);
bool readFromFile (XKey::RootFolder_Ptr *root, const std::string &filename, const std::string &key);

using namespace XKey;
int main (int argc, char** argv) {
	if (argc < 2) {
		std::cerr << "Usage: WriteTest keystore_file\n";
		return -1;
	}
	
	const std::string filename (argv[1]);
	XKey::CryptStream::InitCrypto();

	//std::cout << "Password: ";
	//std::string key = get_password();
	std::string key = "ABC";
	
	XKey::RootFolder_Ptr root = XKey::createRootFolder();
	XKey::Folder *f = root->createSubfolder ("First Subfolder");
	f->addEntry (Entry{"SomeTitle1", "User", "Url", "PwdXXX", "E-Mail", "Comment"});
	f->addEntry (Entry{"SomeTitle3", "User3", "Url3", "PwdXXX", "E-Mail3", "Comment3"});
	for (int i = 0; i < 10; ++i) {
		f = f->createSubfolder ("New Subfolder");
		f->addEntry (Entry{"SubTitle2", "User2", "Url2", "PwdXXX2", "E-Mail2", "Comment2"});
		f->addEntry (Entry{"SomeSubTitle4", "User4", "Url4", "PwdXXX", "E-Mail4", "Comment4"});
	}
	
	if (!writeToFile (*root, filename, key))
		return 1;
	
	XKey::RootFolder_Ptr cmpRoot;
	if (!readFromFile (&cmpRoot, filename, key))
		return 1;

	print_folder(*cmpRoot, 0);
	
	return 0;
}
