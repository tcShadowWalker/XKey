#include "XKeyApplication.h"
#include "KeyListModel.h"
#include "FolderListModel.h"
#include "../CryptStream.h"
#include <QFileDialog>
#include <QMessageBox>
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
	connect (mUi->actionOpen, SIGNAL(triggered()), this, SLOT(showOpenFile()));
	connect (mUi->actionSave_As, SIGNAL(triggered()), this, SLOT(showSaveAsFile()));
	connect (mUi->actionSave, SIGNAL(triggered()), this, SLOT(save()));
	
	// TODO: addWidget LineEdit and PushButton to qToolbar!
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


