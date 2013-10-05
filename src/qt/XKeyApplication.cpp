#include "XKeyApplication.h"
#include "KeyListModel.h"
#include "FolderListModel.h"
#include "KeyEditDialog.h"
#include "FileDialog.h"
#include "SettingsDialog.h"
#include "../CryptStream.h"
#include <QFileDialog>
#include <QPushButton>
#include <QMessageBox>
#include <QSettings>
#include <QLineEdit>
#include <cassert>
// UIs
#include <ui_Main.h>
#include <iostream>

XKeyApplication::XKeyApplication(QSettings *sett)
	: mSettings(sett), mUi(0), mFolders(0), mKeys(0), madeChanges(false), mRecentFiles(0)
{
	// Read application settings from hard disk
	SettingsDialog::readSettings(mSettings, &mGenerator, &mSaveOptions);
	// Init Ui
	mUi = new Ui::MainWindow;
	mUi->setupUi(&mMain);
	// 
	mFolders = new FolderListModel (this);
	mKeys = new KeyListModel (this);
	//
	mUi->keyTable->setModel(mKeys);
	mUi->keyTable->setDragDropMode(QAbstractItemView::DragOnly);
	mUi->keyTable->setDragEnabled(true);
	mUi->keyTable->setDefaultDropAction(Qt::MoveAction);
	mUi->keyTree->setModel(mFolders);
	mUi->keyTree->setDragDropMode(QAbstractItemView::DragDrop);
	mUi->keyTree->setDragDropOverwriteMode(true);
	mUi->keyTree->setDragEnabled(true);
	mUi->keyTree->setDefaultDropAction(Qt::MoveAction);
	// Signals
	connect (mUi->actionSettings, SIGNAL(triggered()), this, SLOT(showSettingsDialog()));
	
	connect (mUi->actionNew, SIGNAL(triggered()), this, SLOT(newFile()));
	connect (mUi->actionOpen, SIGNAL(triggered()), this, SLOT(showOpenFile()));
	connect (mUi->actionSave_As, SIGNAL(triggered()), this, SLOT(showSaveAsFile()));
	connect (mUi->actionSave, SIGNAL(triggered()), this, SLOT(save()));
	
	connect (mUi->actionAddEntry, SIGNAL(triggered()), this, SLOT(addEntryClicked()));
	connect (mUi->actionDeleteEntry, SIGNAL(triggered()), this, SLOT(deleteEntryClicked()));
	connect (mUi->actionEditEntry, SIGNAL(triggered()), this, SLOT(editEntryClicked()));
	connect (mUi->actionAddFolder, SIGNAL(triggered()), this, SLOT(addFolderClicked()));
	connect (mUi->actionDeleteFolder, SIGNAL(triggered()), this, SLOT(deleteFolderClicked()));
	connect (mUi->actionClearSelection, SIGNAL(triggered()), this, SLOT(clearSelection()));
	
	mUi->keyTree->setSelectionMode (QAbstractItemView::SingleSelection);
	connect (mUi->keyTree->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(folderSelectionChanged(const QItemSelection &, const QItemSelection &)));
	
	mUi->keyTable->setSelectionMode (QAbstractItemView::SingleSelection);
	mUi->keyTable->setSelectionBehavior (QAbstractItemView::SelectRows);
	
	connect (mUi->keyTable, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(editKey(QModelIndex)));
	// Search bar: (Currently disabled because not implemented)
	mSearchBar = new QLineEdit (&mMain);
	mSearchBar->setEnabled(false);
	mUi->toolBar->addWidget(mSearchBar);
	QPushButton *searchButton = new QPushButton (tr("Search"), &mMain);
	searchButton->setEnabled(false);
	searchButton->setObjectName("searchButton");
	connect(searchButton, SIGNAL(clicked(bool)), this, SLOT(startSearch()));
	mUi->toolBar->addWidget(searchButton);
	
	mRecentFiles = new QMenu (tr("Open recent"), &mMain);
	loadRecentFileList();
	QAction *act = mUi->menuFile->insertMenu(mUi->actionOpen, mRecentFiles);
	act->setIcon(QIcon(tr(":/icons/document-open.png", "Icon for recent documents submenu")));
	
	setEnabled(false);
}

XKeyApplication::~XKeyApplication() {
	mMain.close();
	delete mUi;
}

void XKeyApplication::show() {
	mMain.show();
}

// 

bool XKeyApplication::askClose () {
	if (mFolders && madeChanges) {
		int answer = QMessageBox::question(&mMain, tr("Close database"), tr("Are you sure you want to close the current database and discard all changes you may have made?"),
				QMessageBox::Abort | QMessageBox::Yes);
		return (answer == QMessageBox::Yes);
	}
	return true;
}

void XKeyApplication::newFile () {
	if (!askClose())
		return;
	this->mRoot = XKey::create_root_folder();
	this->mFolders->setRootFolder(&*mRoot);
	this->mKeys->setCurrentFolder (&*mRoot);
	madeChanges = false;
	setEnabled(true);
}

void XKeyApplication::openFile (const QString &filename) {
	if (!askClose())
		return;
	QString errorMsg;
	bool success = false;
	QString password;
	try {
		XKey::Parser p;
		XKey::CryptStream crypt_source (filename.toStdString(), std::string(), XKey::CryptStream::READ);
		if (crypt_source.isEncrypted()) {
			FilePasswordDialog pwdDiag (FilePasswordDialog::READ, &mMain);
			if (pwdDiag.exec() != QDialog::Accepted)
				return;
			crypt_source.setEncryptionKey(pwdDiag.password().toStdString());
		}
		std::istream isource (&crypt_source);
		XKey::RootFolder_Ptr newRoot = XKey::create_root_folder();
		if (p.readFile(isource, &*newRoot)) {
			success = true;
			// Set attributes:
			this->mRoot = std::move(newRoot);
			currentFileName = filename;
			currentFilePassword = password;
			this->mFolders->setRootFolder(&*mRoot);
			this->mKeys->setCurrentFolder (&*mRoot);
			madeChanges = false;
			setEnabled(true);
			// Add to recent file list:
			QStringList files = mSettings->value("ui/recentFileList").toStringList();
			files.removeAll(filename);
			files.prepend(filename);
			while (files.size() > 10)
				files.removeLast();
			mSettings->setValue("ui/recentFileList", files);
			loadRecentFileList ();
		} else {
			errorMsg = QString::fromStdString(p.error());
		}
	} catch (const std::exception &e) {
		errorMsg = e.what();
	}
	if (!success) {
		QMessageBox::critical (&mMain, tr("Opening keystore failed"), tr("Opening the file failed:\n%1").arg(errorMsg));
	}
}

void XKeyApplication::saveFile (const QString &filename, const SaveFileOptions &sopt) {
	QString errorMsg;
	bool success = false;
	try {
		QString passwd;
		if (sopt.use_encryption) {
			FilePasswordDialog pwdDiag (FilePasswordDialog::WRITE, &mMain);
			if (pwdDiag.exec() != QDialog::Accepted)
				return;
			passwd = pwdDiag.password();
		}
		XKey::Writer w;
		XKey::CryptStream crypt_source (filename.toStdString(), passwd.toStdString(), XKey::CryptStream::WRITE, sopt.makeCryptStreamMode());
		// 
		std::ostream isource (&crypt_source);
		// If we don't use encryption, we want to write formatted.
		bool writeFormatted = (sopt.use_encryption == false);
		if (w.writeFile(isource, *mRoot, writeFormatted)) {
			success = true;
			madeChanges = false;
		} else {
			errorMsg = QString::fromStdString(w.error());
		}
	} catch (const std::exception &e) {
		errorMsg = e.what();
	}
	if (!success) {
		QMessageBox::critical (&mMain, tr("Saving keystore failed"), tr("Saving the keystore file failed:\n%1").arg(errorMsg));
	}
}

// Ui actions:

void XKeyApplication::folderSelectionChanged (const QItemSelection &l, const QItemSelection &) {
	QModelIndexList indexes = l.indexes();
	if (indexes.size() == 1) {
		XKey::Folder *f = static_cast<XKey::Folder *> (indexes.at(0).internalPointer());
		mKeys->setCurrentFolder(f);
	} else {
		mKeys->setCurrentFolder(0);
	}
}

void XKeyApplication::startSearch () {
	if (!mSearchBar->text().isEmpty()) {
		mUi->statusbar->showMessage(tr("Search: %1").arg(mSearchBar->text()));
	}
}

void XKeyApplication::editKey (const QModelIndex & index) {
	XKey::Entry &entry = static_cast<XKey::Entry&> (mKeys->folder()->entries().at(index.row()));
	KeyEditDialog diag (&entry, &mMain, &mGenerator);
	if (diag.exec () == QDialog::Accepted) {
		diag.makeChanges ();
		madeChanges = true;
	}
}

void XKeyApplication::showOpenFile () {
	 QString fileName = QFileDialog::getOpenFileName(&mMain, tr("Open Keystore"), "", tr("Keystore Files (*.xkey)"));
	 if (!fileName.isEmpty()) {
		 openFile(fileName);
	 }
}
void XKeyApplication::showSaveAsFile () {
	SaveFileDialog saveDiag (&mMain, mSaveOptions);
	if (saveDiag.exec () != QDialog::Accepted || saveDiag.selectedFiles().size() == 0)
		return;
	saveFile(saveDiag.selectedFiles().at(0), saveDiag.saveFileOptions());
}

void XKeyApplication::save () {
	if (!currentFileName.isEmpty())
		saveFile(currentFileName, mSaveOptions);
}

void XKeyApplication::addFolderClicked () {
	QModelIndexList indexes = mUi->keyTree->selectionModel()->selectedRows();
	if (indexes.size() == 0) {
		// Add new folder at root level:
		mFolders->insertRow(mRoot->subfolders().size(), QModelIndex());
	} else if (indexes.size() == 1) {
		QModelIndex parent = indexes.at(0);
		mFolders->insertRow(0, parent);
	}
	madeChanges = true;
}

void XKeyApplication::deleteFolderClicked () {
	QModelIndexList indexes = mUi->keyTree->selectionModel()->selectedRows();
	if (indexes.size() > 0) {
		if (QMessageBox::question(&mMain, tr("Confirm folder deletion"), tr("Are you sure you want to delete the selected folder(s)?"),
			QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
		{
			if (mFolders->removeRow (indexes.at(0).row(), indexes.at(0).parent())) {
				// Check if current folder is shown, if yes, show empty table
				mUi->statusbar->showMessage(tr("Folders deleted"));
			} else {
				mUi->statusbar->showMessage(tr("Failed to remove folders"));
			}
		}
	}
	madeChanges = true;
}

void XKeyApplication::setEnabled (bool enabled) {
	// mMain.findChild<QPushButton*> ("searchButton"), mSearchBar
	QWidget *widgetList[] = { mUi->keyTable, mUi->keyTree
	};
	QAction *actionList[] = { mUi->actionSave, mUi->actionSave_As,
		mUi->actionAddEntry, mUi->actionEditEntry, mUi->actionDeleteEntry, mUi->actionAddFolder, mUi->actionDeleteFolder
	};
	for (QWidget *w : widgetList) {
		w->setEnabled(enabled);
	}
	for (QAction *a : actionList) {
		a->setEnabled(enabled);
	}
}

void XKeyApplication::addEntryClicked () {
	if (!this->mKeys->folder())
		return;
	XKey::Entry entry ( tr("Example title", "NewKeyEntryTitle").toStdString(), tr("Your Username", "NewKeyEntryUser").toStdString(),
						tr("example.org", "NewKeyEntryDomain").toStdString(), tr("", "NewKeyEntryPassword").toStdString(), tr("", "NewKeyEntryComment").toStdString() );
	KeyEditDialog diag (&entry, &mMain);
	if (diag.exec () == QDialog::Accepted) {
		diag.makeChanges ();
		this->mKeys->addEntry(std::move(entry));
	}
	madeChanges = true;
}

void XKeyApplication::editEntryClicked () {
	QModelIndexList indexes = mUi->keyTable->selectionModel()->selectedRows();
	if (indexes.size() == 1) {
		editKey (indexes.at(0));
	}
	madeChanges = true;
}

void XKeyApplication::deleteEntryClicked () {
	QModelIndexList indexes = mUi->keyTable->selectionModel()->selectedRows();
	if (indexes.size() == 1) {
		const int row = indexes.at(0).row();
		this->mKeys->removeEntry(row);
	}
	madeChanges = true;
}

void XKeyApplication::showSettingsDialog () {
	SettingsDialog diag (mSettings, &mGenerator, &mSaveOptions, &mMain);
	diag.exec();
}

void XKeyApplication::clearSelection () {
	mUi->keyTable->clearSelection();
	mUi->keyTree->clearSelection();
}

void XKeyApplication::loadRecentFileList() {
	QStringList files = mSettings->value("ui/recentFileList").toStringList();
	for (const QString &file : files) {
		QAction *act = mRecentFiles->addAction(file);
		act->setData(file);
		act->setIcon(QIcon(tr(":/icons/document-open.png", "Icon for opening recent documents")));
		connect(act, SIGNAL(triggered()), this, SLOT(openRecentFile()));
	}
}

void XKeyApplication::openRecentFile() {
	QAction *action = qobject_cast<QAction *>(sender());
	if (action) {
		openFile(action->data().toString());
	}
}

// SFO

int SaveFileOptions::makeCryptStreamMode () const {
	int m = 0;
	if (use_encoding)
		m |= XKey::BASE64_ENCODED;
	if (write_header)
		m |= XKey::EVALUATE_FILE_HEADER;
	if (use_encryption)
		m |= XKey::USE_ENCRYPTION;
	return m;
}

