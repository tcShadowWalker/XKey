#include <QtWidgets/QApplication>
#include <QSettings>
#include "XKeyApplication.h"
#include <CryptStream.h>

int main (int argc, char** argv) {
	XKey::CryptStream::InitCrypto();
	QApplication app (argc, argv);
	QSettings settings ("jp-dev.org", "XKey");
	XKeyApplication xkey (&settings);
	if (argc >= 2) {
		xkey.openFile (QString(argv[1]));
	}
	xkey.show();
	return app.exec();
}
