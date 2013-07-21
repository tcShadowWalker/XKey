#include "XKey.h"
#include "KeyListModel.h"
#include <ui_Main.h>
#include <QFileDialog>

XKey::XKey()
	: mUi(0)
{
	mUi = new Ui::MainWindow;
	mUi->setupUi(&mMain);
	// 
	mKeys = new KeyListModel;
	//
	mUi->keyTable->setModel(mKeys);
	//mUi->keyTree->setModel();
	// Signals
	connect (mUi->actionOpen, SIGNAL(triggered()), this, SLOT(showOpenFile()));
	connect (mUi->actionSave_As, SIGNAL(triggered()), this, SLOT(showSaveAsFile()));
	connect (mUi->actionSave, SIGNAL(triggered()), this, SLOT(save()));
}

XKey::~XKey() {
	mMain.close();
	delete mUi;
	delete mKeys;
}

void XKey::show() {
	mMain.show();
}

// 

void XKey::openFile (const QString &filename) {
	
}

void XKey::saveFile (const QString &filename) {
	
}

// Ui actions:

void XKey::showOpenFile () {
	 QString fileName = QFileDialog::getOpenFileName(&mMain, tr("Open Keystore"), "~", tr("Keystore Files (*.xkey)"));
	 if (!fileName.isEmpty()) {
		 openFile(fileName);
	 }
}
void XKey::showSaveAsFile () {
	 QString fileName = QFileDialog::getSaveFileName(&mMain, tr("Save Keystore"), "~", tr("Keystore Files (*.xkey)"));
	 if (!fileName.isEmpty()) {
		 saveFile(fileName);
	 }
}
void XKey::save () {
	
}


