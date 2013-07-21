#include "XKeyApplication.h"
#include "KeyListModel.h"
#include <ui_Main.h>
#include <QFileDialog>

XKeyApplication::XKeyApplication()
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
	
	// TODO: addWidget LineEdit and PushButton to qToolbar!
}

XKeyApplication::~XKeyApplication() {
	mMain.close();
	delete mUi;
	delete mKeys;
}

void XKeyApplication::show() {
	mMain.show();
}

// 

void XKeyApplication::openFile (const QString &filename) {
	
}

void XKeyApplication::saveFile (const QString &filename) {
	
}

// Ui actions:

void XKeyApplication::showOpenFile () {
	 QString fileName = QFileDialog::getOpenFileName(&mMain, tr("Open Keystore"), "~", tr("Keystore Files (*.xkey)"));
	 if (!fileName.isEmpty()) {
		 openFile(fileName);
	 }
}
void XKeyApplication::showSaveAsFile () {
	 QString fileName = QFileDialog::getSaveFileName(&mMain, tr("Save Keystore"), "~", tr("Keystore Files (*.xkey)"));
	 if (!fileName.isEmpty()) {
		 saveFile(fileName);
	 }
}
void XKeyApplication::save () {
	
}


