#include "KeyEditDialog.h"
#include <XKey.h>
#include <XKeyGenerator.h>

#include <ui_Entry.h>
#include <QClipboard>
#include <QMessageBox>

KeyEditDialog::KeyEditDialog (XKey::Entry *r, XKey::Folder *folder, QWidget *parent, XKey::PassphraseGenerator *gen, bool newEnt)
	: QDialog(parent), mUi(0), mEntry(r), mGen(gen), mIsNewEntry(newEnt)
{
	mUi = new Ui::EditEntryDialog;
	mUi->setupUi(this);
	mUi->titleEdit->setText( QString::fromStdString(mEntry->title()) );
	mUi->usernameEdit->setText( QString::fromStdString(mEntry->username()) );
	mUi->urlEdit->setText( QString::fromStdString(mEntry->url()) );
	mUi->emailEdit->setText( QString::fromStdString(mEntry->email()) );
	mUi->commentEdit->setText( QString::fromStdString(mEntry->comment()) );
	setPasswordHidden (true);
	mUi->passwordEdit->setText( QString::fromStdString(mEntry->password()) );
	mUi->headlineLabel->setText ( QString::fromStdString(folder->fullPath()) );
	
	connect (mUi->hiddenCheckbox, SIGNAL(toggled(bool)), this, SLOT(setPasswordHidden(bool)));
	connect (mUi->generateButton, SIGNAL(clicked()), this, SLOT(generatePassphraseClicked()));
	connect (mUi->copyClipboardButton, SIGNAL(clicked()), this, SLOT(copyToClipboard()));
}

KeyEditDialog::~KeyEditDialog() {
	delete mUi;
}

void KeyEditDialog::setPasswordHidden (bool hidden) {
	mUi->hiddenCheckbox->setChecked (hidden);
	mUi->passwordEdit->setEchoMode (hidden ? QLineEdit::Password : QLineEdit::Normal);
}

void KeyEditDialog::generatePassphraseClicked () {
	int r = QMessageBox::Ok;
	if (!mIsNewEntry) {
		r = QMessageBox::question(this, tr("Generate passphrase"), tr("Do you really want to clear "
					"the current passphrase and generate a new one?"), QMessageBox::Ok, QMessageBox::Cancel);
	}
	if (r == QMessageBox::Ok) {
		mUi->passwordEdit->setText( generatePassphrase() );
		mIsNewEntry = false;
	}
}

void KeyEditDialog::makeChanges () {
	*mEntry = XKey::Entry ( mUi->titleEdit->text().toStdString(), mUi->usernameEdit->text().toStdString(), mUi->urlEdit->text().toStdString(),
		mUi->passwordEdit->text().toStdString(),  mUi->emailEdit->text().toStdString(),
		mUi->commentEdit->toPlainText().toStdString()
	);
}

QString KeyEditDialog::generatePassphrase () {
	if (!mGen)
		return "";
	std::string password;
	mGen->generatePassphrase(&password);
	return QString::fromStdString(password);
}

void KeyEditDialog::copyToClipboard () {
	 QClipboard *clipboard = QApplication::clipboard();
	 clipboard->setText (mUi->passwordEdit->text());
}
