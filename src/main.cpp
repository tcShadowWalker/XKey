#include "XKey.h"
#include "CryptStream.h"
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <cstring>

// Needed for no-echo password query
#include <termios.h>
#include <stdio.h>

#include <openssl/evp.h>
#include <boost/program_options.hpp>

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

void print_folder (const XKey::Folder &f, int depth = 0) {
	if (depth != 0)
		std::cout << std::string(depth*2, '-') << " " << f.name() << "\n";
	for (const auto &it : f.entries()) {
		std::cout << std::string(depth*2, '-') << "    # " << it.username() << " " << it.url() << "\n";
	}
	for (const auto &it : f.subfolders()) {
		print_folder(it, depth+1);
	}
}

std::string input_file, output_file, search_path, key_file;
bool output_no_encrypt = false, output_no_encode = false, output_no_header = false;
bool input_no_header = false, input_not_encoded = false, input_not_encrypted = false;

int parse_commandline (int argc, char** argv) {
	namespace po = boost::program_options; 
	po::options_description desc("Options"); 
	desc.add_options() 
		("help", "Print help messages") 
		("input_file", po::value<std::string>(&input_file), "Input key-database")
		("keyfile", po::value<std::string>(&key_file), "File that contains the password to open a databse. If not given, the password will be read from standard input")
		("search_root,s", po::value<std::string>(&search_path), "Search path, specifying the root-folder withing the key-database")
		("output_file,o", po::value<std::string>(&output_file), "Output key-database") 
		
		("out-no-encrypt", po::bool_switch(&output_no_encrypt), "Do not encrypt output file (Default yes)")
		("out-no-encode", po::bool_switch(&output_no_encode), "Do not base64-encode output file, so that it contains only ascii characters (Default: Yes)")
		("out-no-header", po::bool_switch(&output_no_header), "Do not include keyfile header in output file"
				"If you omit the header, it will not be so easy to recognize the file as an XKey database. "
				"On ther other hand, if you open the file again, XKey won't be able to guess wich encryption and encoding you used "
				"to encrypt the file, so you have to specify those manually\n"
				"(Default: Yes)"
		)
		("in-no-header", po::bool_switch(&input_no_header), "The input file does not have an XKey header (Default: Yes)")
		("in-not-encoded", po::bool_switch(&input_not_encoded), "The input file is not base64-encoded (Default: Yes)")
		("in-not-encrypted", po::bool_switch(&input_not_encrypted), "The input file is in plaintext (Default: Yes)");
		//("out-params,p", po::value<std::vector<std::string>>(&out_options), "Output database options");
		
	po::positional_options_description positionalOptions; 
		positionalOptions.add("input_file", 1); 
		positionalOptions.add("search_root", 1); 
	
	po::variables_map vm; 
	try { 
		po::store(po::command_line_parser(argc, argv).options(desc) 
					.positional(positionalOptions).run(), vm); 

		if ( vm.count("help")  ) { 
			std::cout << "XKey key storage tool for the command-line.\n"
					<< desc << "\n"; 
			return -1; 
		} 

		po::notify(vm); // throws on error, so do after help in case 
						// there are any problems 
	} catch(po::error& e) { 
		std::cerr << "ERROR: " << e.what() << "\n\n"; 
		std::cerr << desc << "\n"; 
		return -1; 
	}
	return 0; 
}

int main (int argc, char** argv) {

	if (parse_commandline (argc, argv) != 0) {
		return -1;
	}
	
	// TODO key_file
	
	OpenSSL_add_all_ciphers();
	
	if (input_file.size() <= 0) {
		std::cerr << "Input file is required!\n";
		return -1;
	}
	
	XKey::RootFolder_Ptr rootKeyFolder = XKey::create_root_folder();

	try {
		int m = 0;
		if (!input_no_header)
			m |= XKey::EVALUATE_FILE_HEADER;
		if (!input_not_encrypted)
			m |= XKey::USE_ENCRYPTION;
		if (!input_not_encoded)
			m |= XKey::BASE64_ENCODED;
		XKey::CryptStream crypt_streambuf (input_file, std::string(), XKey::CryptStream::READ, m);
		
		std::string key;
		
		if (crypt_streambuf.isEncrypted()) {
			std::cout << "Password: ";
			key = get_password();
			crypt_streambuf.setEncryptionKey(key);
		}

		std::istream stream (&crypt_streambuf);
		XKey::Parser pars;
		if (!pars.readFile (stream, &*rootKeyFolder)) {
			std::cerr << "Could not parse keystore file " << input_file << ": " << pars.error() << "\n";
			return -1;
		}
		
		const XKey::Folder *f = &*rootKeyFolder;
		if (search_path.size() > 0) {
			// Search path specified:
			f = XKey::get_folder_by_path (&*rootKeyFolder, search_path);
		}
		if (!f) {
			std::cerr << "Requested path not found.\n";
			return 0;
		}
		
		if (output_file.size() > 0) {
			int m = 0;
			if (output_no_encrypt == false)
				m |=  XKey::USE_ENCRYPTION;
			if (output_no_encode != true)
				m |=  XKey::BASE64_ENCODED;
			if (output_no_header == false)
				m |=  XKey::EVALUATE_FILE_HEADER;
			
			bool pretty_print = (output_no_encrypt && output_no_encode);
			
			std::string outkey;
			if (!output_no_encrypt) {
				std::cout << "Output passphrase: ";
				outkey = get_password();
			}
			std::cout << "Writing...\n";
			XKey::CryptStream crypt_filter (output_file, outkey, XKey::CryptStream::WRITE, m);
			std::ostream stream (&crypt_filter);
			XKey::Writer w;
			if (!w.writeFile(stream, *f, pretty_print)) {
				std::cerr << "Error: " << w.error() << "\n";
				return -1;
			}
		} else {
			// No options. Just show a list
			std::cout << f->fullPath() << "\n";
			print_folder (*f);
		}
	} catch (const std::exception &e) {
		std::cerr << "Error: " << e.what() << "\n";
		return -1;
	}

	return 0;
}
