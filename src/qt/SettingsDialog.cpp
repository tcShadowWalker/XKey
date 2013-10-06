#include "SettingsDialog.h"
#include "FileDialog.h"
#include <../src/XKey.h>
#include <../src/XKeyGenerator.h>

#include <QSettings>
#include <QMessageBox>
#include <ui_Settings.h>

SettingsDialog::SettingsDialog (QSettings *s, XKey::PassphraseGenerator *gen, SaveFileOptions *saveOptions, QWidget *parent)
	: QDialog(parent), set(s), mUi(0), mGen(gen), mSaveOpt(saveOptions)
{
	mUi = new Ui::SettingsDialog;
	mUi->setupUi(this);
	
	connect ((const QObject*)mUi->buttonBox->button(QDialogButtonBox::Save), SIGNAL(clicked()), this, SLOT(trySave()));
	readSettings(s, gen, saveOptions);
	// ui
	mUi->expandFoldersCheckBox->setChecked ( set->value ("ui/always_expand", false).toBool() );
	mUi->exampleEntriesCheckBox->setChecked ( set->value ("ui/example_data", true).toBool() );
	// keystore
	mUi->encryptionCheckBox->setChecked (set->value("keystore/encrypt", QVariant(true)).toBool());
	mUi->asciiArmorCheckBox->setChecked (set->value("keystore/base64_encode", QVariant(true)).toBool());
	mUi->headerCheckBox->setChecked (set->value ("keystore/include_header", QVariant(true)).toBool());
	// pwgen
	mUi->specialCharCheckBox->setChecked( set->value ("generation/special_chars", QVariant(false)).toBool() );
	mUi->numericsCheckBox->setChecked( set->value ("generation/numerics", QVariant(true)).toBool() );
	mUi->uppercaseCheckBox->setChecked( set->value ("generation/mixed_case", QVariant(true)).toBool() );
	mUi->minLengthSpinBox->setValue( set->value ("generation/min_length", QVariant(10)).toInt() );
	mUi->maxLengthSpinBox->setValue( set->value ("generation/max_Length", QVariant(14)).toInt() );
	
}

SettingsDialog::~SettingsDialog() {
	delete mUi;
}

void SettingsDialog::readSettings (QSettings *set, XKey::PassphraseGenerator *mGen, SaveFileOptions *mSaveOpt) {
	using namespace XKey;
	// Pwgen
	int allowed_chars = PassphraseGenerator::CHAR_ALPHA;
	if (set->value ("generation/special_chars", false).toBool())
		allowed_chars |= PassphraseGenerator::CHAR_SPECIAL;
	if (set->value ("generation/numerics", true).toBool())
		allowed_chars |= PassphraseGenerator::CHAR_NUMERIC;
	if (set->value ("generation/mixed_case", true).toBool())
		allowed_chars |= PassphraseGenerator::CHAR_LOWER_UPPERCASE;
	mGen->setAllowedCharacters(allowed_chars);
	mGen->setMinLength( set->value ("generation/min_length", 10).toInt() );
	mGen->setMaxLength( set->value ("generation/max_Length", 14).toInt() );
	//
	mSaveOpt->use_encryption = set->value("keystore/encrypt", true).toBool();
	mSaveOpt->use_encoding = set->value("keystore/base64_encode", true).toBool();
	mSaveOpt->write_header = set->value("keystore/include_header", true).toBool();
	//
}

void SettingsDialog::saveSettings () {
	// ui
	set->setValue ("ui/always_expand", mUi->expandFoldersCheckBox->isChecked());
	set->setValue ("ui/example_data", mUi->exampleEntriesCheckBox->isChecked());
	// keystore
	set->setValue ("keystore/encrypt", mUi->encryptionCheckBox->isChecked());
	set->setValue ("keystore/base64_encode", mUi->asciiArmorCheckBox->isChecked());
	set->setValue ("keystore/include_header", mUi->headerCheckBox->isChecked());
	// pwgen
	set->setValue ("generation/special_chars", mUi->specialCharCheckBox->isChecked());
	set->setValue ("generation/numerics", mUi->numericsCheckBox->isChecked());
	set->setValue ("generation/mixed_case", mUi->uppercaseCheckBox->isChecked());
	set->setValue ("generation/min_length", mUi->minLengthSpinBox->value());
	set->setValue ("generation/max_Length", mUi->maxLengthSpinBox->value());
	set->sync();
	readSettings(set, mGen, mSaveOpt);
}

void SettingsDialog::trySave () {
	int answer = QMessageBox::Yes;
	// maybe we only want to check changes: add mSaveOpt->use_encryption &&
	if (!mUi->encryptionCheckBox->isChecked()) {
		answer = QMessageBox::question(this, tr("Deactivate encryption"), tr("<p>You have chosen to save all files<br /><b>WITHOUT ANY ENCRYPTION</b>.</p>"
			"<p>After you confirm this action and save a key-database to disk, anyone who has access to those files can read all passwords stored in it</p>"
			"<p>Are you sure you want to deactivate encryption?</p>"), QMessageBox::Yes | QMessageBox::Abort);
	}
	if (answer == QMessageBox::Yes) {
		saveSettings();
		this->accept();
	}
}
