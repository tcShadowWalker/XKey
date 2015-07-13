#include <XKey.h>
#include <CryptStream.h>
#include <XKeyJsonSerialization.h>
// Needed for no-echo password query
#include <termios.h>
#include <stdio.h>
#include <iostream>
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

enum PrintOptions {
	PRINT_PASSWORD = 4
};

void print_folder (const XKey::Folder &f, int print_options, int depth = 0, std::ostream &out = std::cout) {
	if (depth != 0)
		out << std::string(depth*2, '-') << " " << f.name() << "\n";
	for (const auto &it : f.entries()) {
		out << std::string(depth*2, '-') << "    # " << it.title();
		if (it.username().size() > 0)
			out << ", User: " << it.username();
		if (it.url().size() > 0) {
			out << ", " << it.url();
		}
		if (print_options & PRINT_PASSWORD) {
			out << ", Password: " << it.password() << " ";
		}
		out << "\n";
	}
	for (const auto &it : f.subfolders()) {
		print_folder(it, print_options, depth+1);
	}
}

//
bool writeToFile (const XKey::Folder &root, const std::string &filename, const std::string &key) {
	XKey::CryptStream crypt_source (filename, XKey::CryptStream::WRITE);
	crypt_source.setEncryptionKey(key);
	std::ostream stream (&crypt_source);
	stream.exceptions (std::ios_base::badbit);
	
	XKey::Writer writer;
	if (!writer.write(stream, root)) {
		std::cerr << "Write Error: " << writer.error() << "\n";
		return false;
	}
	return true;
}

bool readFromFile (XKey::RootFolder_Ptr *root, const std::string &filename, const std::string &key) {
	XKey::CryptStream crypt_source (filename, XKey::CryptStream::READ);
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
