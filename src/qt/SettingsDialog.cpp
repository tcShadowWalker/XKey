#include "SettingsDialog.h"
#include "FileDialog.h"
#include <../src/XKey.h>
#include <../src/XKeyGenerator.h>

#include <QSettings>
#include <ui_Settings.h>

SettingsDialog::SettingsDialog (QSettings *s, XKey::PassphraseGenerator *gen, SaveFileOptions *saveOptions, QWidget *parent)
	: QDialog(parent), set(s), mUi(0), mGen(gen), mSaveOpt(saveOptions)
{
	mUi = new Ui::SettingsDialog;
	mUi->setupUi(this);
	
	connect ((const QObject*)mUi->buttonBox->button(QDialogButtonBox::Save), SIGNAL(clicked()), this, SLOT(trySave()));
	readSettings(s, gen, saveOptions);
	mUi->asciiArmorCheckBox->setChecked (set->value("keystore.base64_encode", QVariant(true)).toBool());
	mUi->headerCheckBox->setChecked (set->value ("keystore.include_header", QVariant(true)).toBool());
	// pwgen
	mUi->specialCharCheckBox->setChecked( set->value ("generation.special_chars", QVariant(false)).toBool() );
	mUi->numericsCheckBox->setChecked( set->value ("generation.numerics", QVariant(true)).toBool() );
	mUi->uppercaseCheckBox->setChecked( set->value ("generation.mixed_case", QVariant(true)).toBool() );
	mUi->minLengthSpinBox->setValue( set->value ("generation.min_length", QVariant(10)).toInt() );
	mUi->maxLengthSpinBox->setValue( set->value ("generation.max_Length", QVariant(14)).toInt() );
}

SettingsDialog::~SettingsDialog() {
	delete mUi;
}

void SettingsDialog::readSettings (QSettings *set, XKey::PassphraseGenerator *mGen, SaveFileOptions *mSaveOpt) {
	using namespace XKey;
	// Pwgen
	int allowed_chars = PassphraseGenerator::CHAR_ALPHA;
	if (set->value ("generation.special_chars").toBool())
		allowed_chars |= PassphraseGenerator::CHAR_SPECIAL;
	if (set->value ("generation.numerics").toBool())
		allowed_chars |= PassphraseGenerator::CHAR_NUMERIC;
	if (set->value ("generation.mixed_case").toBool())
		allowed_chars |= PassphraseGenerator::CHAR_LOWER_UPPERCASE;
	mGen->setAllowedCharacters(allowed_chars);
	mGen->setMinLength( set->value ("generation.min_length").toInt() );
	mGen->setMaxLength( set->value ("generation.max_Length").toInt() );
	//
	mSaveOpt->use_encoding = set->value("keystore.base64_encode").toBool();
	mSaveOpt->write_header = set->value("keystore.include_header").toBool();
}

void SettingsDialog::saveSettings () {
	set->setValue ("generation.base64_encode", mUi->asciiArmorCheckBox->isChecked());
	set->setValue ("generation.include_header", mUi->headerCheckBox->isChecked());
	// pwgen
	set->setValue ("generation.special_chars", mUi->specialCharCheckBox->isChecked());
	set->setValue ("generation.numerics", mUi->numericsCheckBox->isChecked());
	set->setValue ("generation.mixed_case", mUi->uppercaseCheckBox->isChecked());
	set->setValue ("generation.min_length", mUi->minLengthSpinBox->value());
	set->setValue ("generation.max_Length", mUi->maxLengthSpinBox->value());
	set->sync();
	readSettings(set, mGen, mSaveOpt);
}

void SettingsDialog::trySave () {
	if (true) {
		saveSettings();
		this->accept();
	}
}
