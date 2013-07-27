#include "XKeyApplication.h"
#include "KeyListModel.h"
#include "FolderListModel.h"
#include "KeyEditDialog.h"
#include "../CryptStream.h"
#include <QFileDialog>
#include <QPushButton>
#include <QMessageBox>
#include <cassert>
// UIs
#include <ui_Main.h>
#include <ui_OpenKeystorePassphrase.h>

#include <iostream>

XKeyApplication::XKeyApplication()
	: mUi(0), mFolders(0), mKeys(0)
{
	mUi = new Ui::MainWindow;
	mUi->setupUi(&mMain);
	// 
	mFolders = new FolderListModel (this);
	mKeys = new KeyListModel (this);
	//
	mUi->keyTable->setModel(mKeys);
	mUi->keyTree->setModel(mFolders);
	// Signals
	connect (mUi->actionNew, SIGNAL(triggered()), this, SLOT(newFile()));
	connect (mUi->actionOpen, SIGNAL(triggered()), this, SLOT(showOpenFile()));
	connect (mUi->actionSave_As, SIGNAL(triggered()), this, SLOT(showSaveAsFile()));
	connect (mUi->actionSave, SIGNAL(triggered()), this, SLOT(save()));
	
	connect (mUi->actionAddEntry, SIGNAL(triggered()), this, SLOT(addEntryClicked()));
	connect (mUi->actionDeleteEntry, SIGNAL(triggered()), this, SLOT(deleteEntryClicked()));
	connect (mUi->actionEditEntry, SIGNAL(triggered()), this, SLOT(editEntryClicked()));
	connect (mUi->actionAddFolder, SIGNAL(triggered()), this, SLOT(addFolderClicked()));
	connect (mUi->actionDeleteFolder, SIGNAL(triggered()), this, SLOT(deleteFolderClicked()));
	
	// TODO: addWidget LineEdit and PushButton to qToolbar!
	mUi->keyTree->setSelectionMode (QAbstractItemView::SingleSelection);
	connect (mUi->keyTree->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(folderSelectionChanged(const QItemSelection &, const QItemSelection &)));
	
	mUi->keyTable->setSelectionMode (QAbstractItemView::SingleSelection);
	mUi->keyTable->setSelectionBehavior (QAbstractItemView::SelectRows);
	
	connect (mUi->keyTable, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(editKey(QModelIndex)));
	// Search bar:
	mSearchBar = new QLineEdit (&mMain);
	mUi->toolBar->addWidget(mSearchBar);
	QPushButton *searchButton = new QPushButton (tr("Search"), &mMain);
	searchButton->setObjectName("searchButton");
	connect(searchButton, SIGNAL(clicked(bool)), this, SLOT(startSearch()));
	mUi->toolBar->addWidget(searchButton);
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

void XKeyApplication::askClose () {
	// TODO: Check if current keystore open then ask to save
}

void XKeyApplication::newFile () {
	mRoot = XKey::Folder();
	this->mFolders->setRootFolder(&mRoot);
	this->mKeys->setCurrentFolder (&mRoot);
	setEnabled(true);
}

void XKeyApplication::openFile (const QString &filename) {
	askClose();
	QString errorMsg;
	bool success = false;
	try {
		XKey::Parser p;
		XKey::CryptStream crypt_source (filename.toStdString(), std::string(), XKey::CryptStream::READ);
		if (crypt_source.isEncrypted()) {
			QDialog diag;
			Ui::OpenFilePassphraseDialog  opd;
			opd.setupUi (&diag);
			if (diag.exec() != QDialog::Accepted || opd.passphraseEdit->text().isEmpty())
				return;
			crypt_source.setEncryptionKey(opd.passphraseEdit->text().toStdString());
		}
		std::istream isource (&crypt_source);
		if (p.readFile(isource, &mRoot)) {
			success = true;
			this->mFolders->setRootFolder(&mRoot);
			this->mKeys->setCurrentFolder (&mRoot);
			setEnabled(true);
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

void XKeyApplication::saveFile (const QString &filename) {
	
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
	KeyEditDialog diag (&entry, &mMain);
	if (diag.exec () == QDialog::Accepted) {
		diag.makeChanges ();
	}
}

void XKeyApplication::showOpenFile () {
	 QString fileName = QFileDialog::getOpenFileName(&mMain, tr("Open Keystore"), "", tr("Keystore Files (*.xkey)"));
	 if (!fileName.isEmpty()) {
		 openFile(fileName);
	 }
}
void XKeyApplication::showSaveAsFile () {
	 QString fileName = QFileDialog::getSaveFileName(&mMain, tr("Save Keystore"), "", tr("Keystore Files (*.xkey)"));
	 if (!fileName.isEmpty()) {
		 saveFile(fileName);
	 }
}
void XKeyApplication::save () {
	
}

void XKeyApplication::addFolderClicked () {
	QModelIndexList indexes = mUi->keyTree->selectionModel()->selectedRows();
	if (indexes.size() == 0) {
		// Add new folder at root level:
		mFolders->insertRow(mRoot.subfolders().size(), QModelIndex());
	} else if (indexes.size() == 1) {
		QModelIndex parent = indexes.at(0);
		mFolders->insertRow(0, parent);
	}
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
}

void XKeyApplication::setEnabled (bool enabled) {
	QWidget *widgetList[] = { mUi->keyTable, mUi->keyTree, mMain.findChild<QPushButton*> ("searchButton"), mSearchBar
	};
	QAction *actionList[] = { mUi->actionSave, mUi->actionSave_As,
		mUi->actionAddEntry, mUi->actionEditEntry, mUi->actionAddFolder, mUi->actionDeleteFolder
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
}

void XKeyApplication::editEntryClicked () {
	QModelIndexList indexes = mUi->keyTable->selectionModel()->selectedRows();
	if (indexes.size() == 1) {
		editKey (indexes.at(0));
	}
}

void XKeyApplication::deleteEntryClicked () {
	// TOOD
}

