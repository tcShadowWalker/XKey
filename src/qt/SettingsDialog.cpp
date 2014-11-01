#include "SettingsDialog.h"
#include "FileDialog.h"
#include <../src/XKey.h>
#include <../src/XKeyGenerator.h>

#include <QSettings>
#include <QMessageBox>
#include <ui_Settings.h>

namespace SettingNames {
const char *KeystoreEncrypt = "keystore/encrypt", *KeystoreEncode = "keystore/base64_encode", *KeystoreHeader = "keystore/include_header",
	*KeystoreIteration = "keystore/key_iteration_count", *KeystoreAlgorithm = "keystore/algorithm";
const char *GenerationSpecial = "generation/special_chars", *GenerationNumerics = "generation/numerics",
	*GenerationMixed = "generation/mixed_case", *GenerationMin = "generation/min_length", *GenerationMax = "generation/max_Length";
// Extern
extern const char *UiAlwaysExpand, *UiExampleData, *UiAlwaysAsk, *UiCustomStyle;
}

SettingsDialog::SettingsDialog (QSettings *s, XKey::PassphraseGenerator *gen, SaveFileOptions *saveOptions, QWidget *parent)
	: QDialog(parent), set(s), mUi(0), mGen(gen), mSaveOpt(saveOptions)
{
	using namespace SettingNames;
	mUi = new Ui::SettingsDialog;
	mUi->setupUi(this);
	
	connect ((const QObject*)mUi->buttonBox->button(QDialogButtonBox::Save), SIGNAL(clicked()), this, SLOT(trySave()));
	readSettings(s, gen, saveOptions);
	// ui
	mUi->expandFoldersCheckBox->setChecked ( set->value (UiAlwaysExpand, false).toBool() );
	mUi->exampleEntriesCheckBox->setChecked ( set->value (UiExampleData, true).toBool() );
	mUi->alwaysAskPasswordCheckBox->setChecked ( set->value (UiAlwaysAsk, false).toBool() );
	mUi->customStylesheetCheckBox->setChecked ( set->value (UiCustomStyle, false).toBool() );
	// keystore
	mUi->encryptionCheckBox->setChecked (set->value(KeystoreEncrypt, QVariant(true)).toBool());
	mUi->asciiArmorCheckBox->setChecked (set->value(KeystoreEncode, QVariant(true)).toBool());
	mUi->headerCheckBox->setChecked (set->value (KeystoreHeader, QVariant(true)).toBool());
	mUi->keyIterationSpinBox->setValue ( set->value (KeystoreIteration, DEFAULT_KEY_ITERATION_COUNT).toInt() );
	QString algo = set->value(KeystoreAlgorithm, QString(DEFAULT_CIPHER_ALGORITHM)).toString();
	int index = mUi->cipherComboBox->findData( algo, Qt::DisplayRole );
	if (index >= 0) {
		mUi->cipherComboBox->setCurrentIndex( (index >= 0) ? index : 0 );
	} else {
		mUi->cipherComboBox->insertItem(0, QIcon(), algo);
		mUi->cipherComboBox->setCurrentIndex(0);
	}
	// pwgen
	mUi->specialCharCheckBox->setChecked( set->value (GenerationSpecial, QVariant(false)).toBool() );
	mUi->numericsCheckBox->setChecked( set->value (GenerationNumerics, QVariant(true)).toBool() );
	mUi->uppercaseCheckBox->setChecked( set->value (GenerationMixed, QVariant(true)).toBool() );
	mUi->minLengthSpinBox->setValue( set->value (GenerationMin, QVariant(10)).toInt() );
	mUi->maxLengthSpinBox->setValue( set->value (GenerationMax, QVariant(14)).toInt() );
	
}

SettingsDialog::~SettingsDialog() {
	delete mUi;
}

void SettingsDialog::readSettings (QSettings *set, XKey::PassphraseGenerator *mGen, SaveFileOptions *mSaveOpt) {
	using namespace XKey;
	using namespace SettingNames;
	// Pwgen
	int allowed_chars = PassphraseGenerator::CHAR_ALPHA;
	if (set->value (GenerationSpecial, false).toBool())
		allowed_chars |= PassphraseGenerator::CHAR_SPECIAL;
	if (set->value (GenerationNumerics, true).toBool())
		allowed_chars |= PassphraseGenerator::CHAR_NUMERIC;
	if (set->value (GenerationMixed, true).toBool())
		allowed_chars |= PassphraseGenerator::CHAR_LOWER_UPPERCASE;
	mGen->setAllowedCharacters(allowed_chars);
	mGen->setMinLength( set->value (GenerationMin, 10).toInt() );
	mGen->setMaxLength( set->value (GenerationMax, 14).toInt() );
	//
	mSaveOpt->use_encryption = set->value(KeystoreEncrypt, true).toBool();
	mSaveOpt->use_encoding = set->value(KeystoreEncode, true).toBool();
	mSaveOpt->write_header = set->value(KeystoreHeader, true).toBool();
	mSaveOpt->key_iteration_count = set->value(KeystoreIteration, DEFAULT_KEY_ITERATION_COUNT).toInt();
	mSaveOpt->cipher_name = set->value(KeystoreAlgorithm, QString(DEFAULT_CIPHER_ALGORITHM)).toString().toStdString();
	// This option is negated
	mSaveOpt->save_password = !set->value(UiAlwaysAsk, false).toBool();
}

void SettingsDialog::saveSettings () {
	using namespace SettingNames;
	// ui
	set->setValue (UiAlwaysExpand, mUi->expandFoldersCheckBox->isChecked());
	set->setValue (UiExampleData, mUi->exampleEntriesCheckBox->isChecked());
	set->setValue (UiAlwaysAsk, mUi->alwaysAskPasswordCheckBox->isChecked());
	set->setValue (UiCustomStyle, mUi->customStylesheetCheckBox->isChecked());
	// keystore
	set->setValue (KeystoreEncrypt, mUi->encryptionCheckBox->isChecked());
	set->setValue (KeystoreEncode, mUi->asciiArmorCheckBox->isChecked());
	set->setValue (KeystoreHeader, mUi->headerCheckBox->isChecked());
	set->setValue (KeystoreIteration, mUi->keyIterationSpinBox->value());
	set->setValue (KeystoreAlgorithm, mUi->cipherComboBox->currentText());
	// pwgen
	set->setValue (GenerationSpecial, mUi->specialCharCheckBox->isChecked());
	set->setValue (GenerationNumerics, mUi->numericsCheckBox->isChecked());
	set->setValue (GenerationMixed, mUi->uppercaseCheckBox->isChecked());
	set->setValue (GenerationMin, mUi->minLengthSpinBox->value());
	set->setValue (GenerationMax, mUi->maxLengthSpinBox->value());
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
