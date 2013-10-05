#ifndef XKEY_APP_H
#define XKEY_APP_H

#include <QtGui/QMainWindow>
#include <boost/concept_check.hpp>
#include "../XKey.h"
#include "../XKeyGenerator.h"
#include "FileDialog.h"

class QMenu;
class QSettings;
struct SaveFileOptions;
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
	XKeyApplication(QSettings *settings);
	~XKeyApplication();
	
	void openFile (const QString &filename);
	void saveFile (const QString &filename, const SaveFileOptions &sopt);
	
	void show ();
public slots:
	void newFile ();
	void showOpenFile ();
	void showSaveAsFile ();
	
	void save ();
	
	/**
	 * @return true if the current database shall be closed, false if it shall remain opened
	 */
	bool askClose ();
	
	void folderSelectionChanged (const QItemSelection &, const QItemSelection &);
	void editKey (const QModelIndex & index);
	void startSearch ();
	
	void showSettingsDialog ();
	void openRecentFile ();
	
	//
	void addFolderClicked ();
	void deleteFolderClicked ();
	void clearSelection ();
	
	void addEntryClicked ();
	void editEntryClicked ();
	void deleteEntryClicked ();
	
private:
	QMainWindow mMain;
	QSettings *mSettings;
	Ui::MainWindow *mUi;
	FolderListModel *mFolders;
	KeyListModel *mKeys;
	XKey::RootFolder_Ptr mRoot;
	QLineEdit *mSearchBar;
	// Open file:
	QString currentFilePassword;
	QString currentFileName;
	bool madeChanges;
	XKey::PassphraseGenerator mGenerator;
	SaveFileOptions mSaveOptions;
	QMenu *mRecentFiles;
	
	void setEnabled (bool enabled);
	void loadRecentFileList ();
};

#endif
