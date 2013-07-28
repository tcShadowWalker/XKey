#ifndef XKEY_APP_H
#define XKEY_APP_H

#include <QtGui/QMainWindow>
#include <boost/concept_check.hpp>
#include "../XKey.h"

class QModelIndex;
class QLineEdit;
class QItemSelection;

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
	void newFile ();
	void showOpenFile ();
	void showSaveAsFile ();
	
	void save ();
	
	void askClose ();
	
	void folderSelectionChanged (const QItemSelection &, const QItemSelection &);
	void editKey (const QModelIndex & index);
	void startSearch ();
	
	//
	void addFolderClicked ();
	void deleteFolderClicked ();
	
	void addEntryClicked ();
	void editEntryClicked ();
	void deleteEntryClicked ();
	
private:
	QMainWindow mMain;
	Ui::MainWindow *mUi;
	FolderListModel *mFolders;
	KeyListModel *mKeys;
	XKey::Folder mRoot;
	QLineEdit *mSearchBar;
	// Open file:
	QString currentFilePassword;
	QString currentFileName;
	
	void setEnabled (bool enabled);
};

#endif
