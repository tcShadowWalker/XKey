#include "KeyEditDialog.h"
#include <../src/XKey.h>
#include <../src/XKeyGenerator.h>

#include <ui_Entry.h>

KeyEditDialog::KeyEditDialog (XKey::Entry *r, QWidget *parent, XKey::PassphraseGenerator *gen)
	: QDialog(parent), mUi(0), mEntry(r), mGen(gen)
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
	connect (mUi->generateButton, SIGNAL(clicked()), this, SLOT(generatePassphraseClicked()));
}

KeyEditDialog::~KeyEditDialog() {
	delete mUi;
}

void KeyEditDialog::setPasswordHidden (bool hidden) {
	mUi->hiddenCheckbox->setChecked (hidden);
	mUi->passwordEdit->setEchoMode (hidden ? QLineEdit::Password : QLineEdit::Normal);
}

void KeyEditDialog::generatePassphraseClicked () {
	mUi->passwordEdit->setText( generatePassphrase() );
}

void KeyEditDialog::makeChanges () {
	*mEntry = XKey::Entry ( mUi->titleEdit->text().toStdString(), mUi->usernameEdit->text().toStdString(), mUi->urlEdit->text().toStdString(),
		mUi->passwordEdit->text().toStdString(), mUi->commentEdit->toPlainText().toStdString()
	);
}

QString KeyEditDialog::generatePassphrase () {
	if (!mGen)
		return "";
	std::string password;
	mGen->generatePassphrase(&password);
	return QString::fromStdString(password);
}
