#include <XKey.h>
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