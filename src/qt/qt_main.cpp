#include <QtGui/QApplication>
#include "XKeyApplication.h"


int main(int argc, char** argv) {
	QApplication app (argc, argv);
	XKeyApplication xkey;
	// TODO: EVALUATE ARGC ARGV,
	// xkey.openFile
	//
	xkey.show();
	return app.exec();
}
