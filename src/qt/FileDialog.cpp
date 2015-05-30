#include "FileDialog.h"
#include <QMessageBox>

#include <ui_OpenKeystorePassphrase.h>
#include <ui_SaveKeystorePassphrase.h>

SaveFileDialog::SaveFileDialog (QWidget *parent, SaveFileOptions opt)
	: QFileDialog(parent, tr("Save file"), tr("Keystore.xkey"), tr("Keystore Files (*.xkey)")), mSaveOpt(opt)
{
	setAcceptMode(AcceptSave);
}

void SaveFileDialog::setDefaultFile (const QString &filepath) {
	setDirectory (filepath);
}

//

FilePasswordDialog::FilePasswordDialog (Operation op, QWidget *parent)
	: QDialog(parent), openUi(nullptr), saveUi(nullptr)
{
	if (op == READ) {
		openUi = new Ui::OpenFilePassphraseDialog;
		openUi->setupUi (this);
		openUi->passphraseEdit->setFocus();
	} else if (op == WRITE) {
		saveUi = new Ui::SaveFilePassphraseDialog;
		saveUi->setupUi (this);
		saveUi->passphraseEdit->setFocus();
	}
}

int FilePasswordDialog::exec () {
	return this->QDialog::exec();
}

QString FilePasswordDialog::password () const {
	if (openUi)
		return openUi->passphraseEdit->text();
	else if (saveUi)
		return saveUi->passphraseEdit->text();
	else
		return QString ("");
}

void FilePasswordDialog::accept () {
	if (saveUi) {
		if (saveUi->passphraseEdit->text() != saveUi->passphraseConfirm->text()) {
			QMessageBox::critical (this, tr("Password mismatch"),
			  tr("The two passwords you entered do not match.\nPlease enter matching passwords"));
			return;
		}
	}
	return this->QDialog::accept();
}
