#ifndef XKEY_APP_H
#define XKEY_APP_H

#include <QtGui/QMainWindow>

class KeyListModel;
namespace Ui {
class MainWindow;
}


class XKeyApplication
	: public QObject
{
Q_OBJECT

public:
	XKeyApplication();
	~XKeyApplication();
	
	void openFile (const QString &filename);
	void saveFile (const QString &filename);
	
	void show ();
public slots:
	void showOpenFile ();
	void showSaveAsFile ();
	
	void save ();
	
private:
	QMainWindow mMain;
	Ui::MainWindow *mUi;
	KeyListModel *mKeys;
};

#endif
