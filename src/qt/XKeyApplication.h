#pragma once

#include <XKey.h>
#include <XKeyGenerator.h>
#include "FileDialog.h"
#include <memory>

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
class QMainWindow;

class XKeyApplication
	: public QObject
{
Q_OBJECT

public:
	XKeyApplication(QSettings *settings);
	~XKeyApplication();
	
	void openFile (const QString &filename);
	void saveFile (const QString &filename, SaveFileOptions &sopt);
	
	void show ();
public slots:
	// Opening keystores
	void newFile ();
	void showOpenFile ();
	void openRecentFile ();
	
	// Saving
	void showSaveAsFile ();
	void save ();
	
	// Selection and editing
	void folderSelectionChanged (const QItemSelection &, const QItemSelection &);
	void editKey (const QModelIndex & index);
	
	void startSearch ();
	void showSettingsDialog ();
	void copyPassphraseToClipboard ();
	
	// Edit folders and entries
	void addFolderClicked ();
	void deleteFolderClicked ();
	void clearSelection ();
	
	void addEntryClicked ();
	void editEntryClicked ();
	void deleteEntryClicked ();

	void aboutDialogClicked ();
	
private:
	std::unique_ptr<QMainWindow> mMain;
	QSettings *mSettings;
	Ui::MainWindow *mUi;
	FolderListModel *mFolders;
	KeyListModel *mKeys;
	XKey::RootFolder_Ptr mRoot;
	QLineEdit *mSearchBar;
	// Open file:
	QString currentFileName;
	bool madeChanges;
	XKey::PassphraseGenerator mGenerator;
	SaveFileOptions mSaveOptions;
	QMenu *mRecentFiles;
	// Search
	QString lastSearchString;
	XKey::SearchResult lastSearchResult;
	
	void setEnabled (bool enabled);
	void loadRecentFileList ();
	
	/// @return true if the current database shall be closed, false if it shall remain opened
	bool askClose ();
	
	void saveApplicationState ();
	
	void addRecentFile (QString filename);
	/// Message timeout for minor notifications in the status bar (in milli-seconds)
	const int statusBarMessageTimeout = 5000;
	
	friend class MyMainWindow;
};

