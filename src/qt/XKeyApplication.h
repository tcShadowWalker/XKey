#ifndef XKEY_APP_H
#define XKEY_APP_H

#include <QtGui/QMainWindow>
#include "../XKey.h"


class KeyListModel;
class FolderListModel;
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
	
	void askClose ();
	
private:
	QMainWindow mMain;
	Ui::MainWindow *mUi;
	FolderListModel *mFolders;
	KeyListModel *mKeys;
	XKey::Folder mRoot;
};

#endif
