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
	PRINT_PASSWORD = 4,
	PRINT_COMMENT = 8,
};

void print_entry (const XKey::Entry &entry, int print_options, int depth = 0, std::ostream &out = std::cout)
{
	out << std::string(depth*2, '-') << "    # " << entry.title();
	if (entry.username().size() > 0)
		out << ", User: " << entry.username();
	if (entry.url().size() > 0) {
		out << ", " << entry.url();
	}
	if (print_options & PRINT_PASSWORD) {
		out << ", Password: " << entry.password() << " ";
	}
	if (print_options & PRINT_COMMENT) {
		out << "\n" << entry.comment() << " ";
	}
	out << "\n";
}

void print_folder (const XKey::Folder &f, int print_options, int depth = 0, std::ostream &out = std::cout) {
	if (depth != 0)
		out << std::string(depth*2, '-') << " " << f.name() << "\n";
	for (const auto &it : f.entries()) {
		print_entry(it, print_options, depth, out);
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
