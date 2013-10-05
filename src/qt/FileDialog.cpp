#include "FileDialog.h"

#include <ui_OpenKeystorePassphrase.h>

SaveFileDialog::SaveFileDialog (QWidget *parent)
	: QFileDialog(parent, tr("Save file"), tr("Keystore.xkey"), tr(".xkey"))
{
	setAcceptMode(AcceptSave);
}


//

FilePasswordDialog::FilePasswordDialog (Operation op, QWidget *parent)
	: QDialog(parent), ui(new Ui::OpenFilePassphraseDialog)
{
	ui->setupUi (this);
	if (op == WRITE) {
		ui->headerLabel->setText(tr("Save file"));
		ui->textLabel->setText(tr("Please enter the passphrase to encrypt the keystore:"));
	}
}

int FilePasswordDialog::exec () {
	return this->QDialog::exec();
}

QString FilePasswordDialog::password () const {
	return ui->passphraseEdit->text();
}

void FilePasswordDialog::accept () {
	return this->QDialog::accept();
}
