#include <QtGui/QApplication>
#include <QSettings>
#include "XKeyApplication.h"
#include <openssl/evp.h>

int main (int argc, char** argv) {
	SSLeay_add_all_algorithms();
	QApplication app (argc, argv);
	QSettings settings ("jp-dev.org", "XKey");
	XKeyApplication xkey (&settings);
	if (argc >= 2) {
		xkey.openFile (QString(argv[1]));
	}
	xkey.show();
	return app.exec();
}
