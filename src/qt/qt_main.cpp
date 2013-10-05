#include <QtGui/QApplication>
#include "XKeyApplication.h"

int main (int argc, char** argv) {
	QApplication app (argc, argv);
	XKeyApplication xkey;
	if (argc >= 2) {
		xkey.openFile (QString(argv[1]));
	}
	xkey.show();
	return app.exec();
}
