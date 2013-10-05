#include "KeyEditDialog.h"
#include <../src/XKey.h>

#include <ui_Entry.h>

KeyEditDialog::KeyEditDialog (XKey::Entry *r, QWidget *parent)
	: QDialog(parent), mUi(0), mEntry(r)
{
	mUi = new Ui::EditEntryDialog;
	mUi->setupUi(this);
	mUi->titleEdit->setText( QString::fromStdString(mEntry->title()) );
	mUi->usernameEdit->setText( QString::fromStdString(mEntry->username()) );
	mUi->urlEdit->setText( QString::fromStdString(mEntry->url()) );
	mUi->commentEdit->setText( QString::fromStdString(mEntry->comment()) );
	setPasswordHidden (true);
	mUi->passwordEdit->setText( QString::fromStdString(mEntry->password()) );
	
	connect (mUi->hiddenCheckbox, SIGNAL(toggled(bool)), this, SLOT(setPasswordHidden(bool)));
}

KeyEditDialog::~KeyEditDialog() {
	delete mUi;
}

void KeyEditDialog::setPasswordHidden (bool hidden) {
	mUi->hiddenCheckbox->setChecked (hidden);
	mUi->passwordEdit->setEchoMode (hidden ? QLineEdit::Password : QLineEdit::Normal);
}

void KeyEditDialog::makeChanges () {
	*mEntry = XKey::Entry ( mUi->titleEdit->text().toStdString(), mUi->usernameEdit->text().toStdString(), mUi->urlEdit->text().toStdString(),
		mUi->passwordEdit->text().toStdString(), mUi->commentEdit->toPlainText().toStdString()
	);
}

QString KeyEditDialog::generatePassphrase () {
	
}
