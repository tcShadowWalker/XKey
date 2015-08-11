#include "SettingsDialog.h"
#include "FileDialog.h"
#include <XKey.h>
#include <XKeyGenerator.h>

#include <QSettings>
#include <QMessageBox>
#include <ui_Settings.h>

namespace Settings {

struct Option
{
	enum class Type {
		BOOL,
		INT,
		STRING,
	};
	Type type;
	const char *name;
	union {
		struct BoolOpt {
			bool defaultVal;
			QCheckBox *Ui::SettingsDialog::*widget;
			bool SaveFileOptions::*opt;
		} b;
		struct IntOpt {
			int defaultVal;
			QSpinBox *Ui::SettingsDialog::*widget;
			int SaveFileOptions::*opt;
		} i;
		struct StringOpt {
			const char *defaultVal;
			QComboBox *Ui::SettingsDialog::*widget;
			std::string SaveFileOptions::*opt;
		} s;
	} val;
	
	Option (const char *pName, bool dVal, QCheckBox *Ui::SettingsDialog::*checkbox, bool SaveFileOptions::*opt) {
		type = Option::Type::BOOL;
		name = pName;
		val.b = { dVal, checkbox, opt };
	}

	Option (const char *pName, int dVal, QSpinBox *Ui::SettingsDialog::*spinbox, int SaveFileOptions::*opt) {
		type = Option::Type::INT;
		name = pName;
		val.i = { dVal, spinbox, opt };
	}

	Option (const char *pName, const char *dVal, QComboBox *Ui::SettingsDialog::*combobox, std::string SaveFileOptions::*opt) {
		type = Option::Type::STRING;
		name = pName;
		val.s = { dVal, combobox, opt };
	}
	
	void writeToUi (Ui::SettingsDialog *d, QSettings *s) const;
	void readFromUi (const Ui::SettingsDialog *d, QSettings *s) const;
	void apply (SaveFileOptions *sfo, QSettings *s) const;
};

extern const char *UiAlwaysExpand, *UiExampleData, *UiAlwaysAsk, *UiCustomStyle;

typedef Ui::SettingsDialog Diag;
typedef SaveFileOptions SFO;

const char *GenerationSpecial = "generation/special_chars", *GenerationNumerics = "generation/numerics",
           *GenerationMixed = "generation/mixed_case", *GenerationMin = "generation/min_length",
           *GenerationMax = "generation/max_Length";

const std::initializer_list<Option> configOptions = {
	Option("keystore/encrypt", true, &Diag::encryptionCheckBox, &SFO::use_encryption),
	Option("keystore/base64_encode", true, &Diag::asciiArmorCheckBox, &SFO::use_encoding),
	Option("keystore/key_iteration_count", DEFAULT_KEY_ITERATION_COUNT, &Diag::keyIterationSpinBox, &SFO::key_iteration_count),
	Option("keystore/algorithm", DEFAULT_CIPHER_ALGORITHM, &Diag::cipherComboBox, &SFO::cipher_name),
	Option("keystore/digest_algorithm", DEFAULT_DIGEST_ALGORITHM, &Diag::digestAlgoComboBox, &SFO::digest_name),
	Option(GenerationSpecial, false, &Diag::specialCharCheckBox, nullptr),
	Option(GenerationNumerics, true, &Diag::numericsCheckBox, nullptr),
	Option(GenerationMixed, true, &Diag::uppercaseCheckBox, nullptr),
	Option(GenerationMin, 12, &Diag::minLengthSpinBox, nullptr),
	Option(GenerationMax, 16, &Diag::maxLengthSpinBox, nullptr),
	// UI
	Option(UiAlwaysExpand, false, &Diag::expandFoldersCheckBox, nullptr),
	Option(UiExampleData, true, &Diag::exampleEntriesCheckBox, nullptr),
	Option(UiAlwaysAsk, false, &Diag::alwaysAskPasswordCheckBox, &SFO::always_ask_password),
	Option(UiCustomStyle, false, &Diag::customStylesheetCheckBox, nullptr),
};

void Option::writeToUi (Diag *d, QSettings *s) const {
	if (type == Type::BOOL) {
		(d->*val.b.widget)->setChecked ( s->value (name, val.b.defaultVal).toBool() );
	} else if (type == Type::INT) {
		(d->*val.i.widget)->setValue ( s->value (name, val.i.defaultVal).toInt() );
	} else if (type == Type::STRING) {
		QComboBox *w = (d->*val.s.widget);
		const QString v = s->value(name, QString(val.s.defaultVal)).toString();
		int index = w->findData( v, Qt::DisplayRole );
		if (index >= 0) {
			w->setCurrentIndex( (index >= 0) ? index : 0 );
		} else {
			w->insertItem(0, QIcon(), v);
			w->setCurrentIndex(0);
		}
	} else {
		throw std::logic_error ("Invalid option type!");
	}
}

void Option::readFromUi (const Diag *d, QSettings *s) const {
	if (type == Type::BOOL) {
		s->setValue (name, (d->*val.b.widget)->isChecked());
	} else if (type == Type::INT) {
		s->setValue (name, (d->*val.i.widget)->value());
	} else if (type == Type::STRING) {
		s->setValue (name, (d->*val.s.widget)->currentText());
	} else {
		throw std::logic_error ("Invalid option type!");
	}
}

void Option::apply (SaveFileOptions *sfo, QSettings *s) const {
	if (type == Type::BOOL) {
		if (val.b.opt != nullptr)
			(sfo->*val.b.opt) = s->value(name, val.b.defaultVal).toBool();
	} else if (type == Type::INT) {
		if (val.i.opt != nullptr)
			(sfo->*val.i.opt) = s->value(name, val.i.defaultVal).toInt();
	} else if (type == Type::STRING) {
		if (val.s.opt != nullptr)
			(sfo->*val.s.opt) = s->value(name, val.s.defaultVal).toString().toStdString();
	} else {
		throw std::logic_error ("Invalid option type!");
	}
}

}

SettingsDialog::SettingsDialog (QSettings *s, XKey::PassphraseGenerator *gen, SaveFileOptions *saveOptions, QWidget *parent)
	: QDialog(parent), set(s), mGen(gen), mSaveOpt(saveOptions)
{
	mUi.reset(new Ui::SettingsDialog);
	mUi->setupUi(this);
	
	connect ((const QObject*)mUi->buttonBox->button(QDialogButtonBox::Save), SIGNAL(clicked()), this, SLOT(trySave()));
	readSettings(set, gen, saveOptions);
	for (const Settings::Option &opt : Settings::configOptions) {
		opt.writeToUi (mUi.get(), set);
	}
}
SettingsDialog::~SettingsDialog () { }

void SettingsDialog::readSettings (QSettings *set, XKey::PassphraseGenerator *mGen, SaveFileOptions *mSaveOpt) {
	for (const Settings::Option &opt : Settings::configOptions) {
		opt.apply (mSaveOpt, set);
	}
	// Pwgen
	using namespace Settings;
	using namespace XKey;
	int allowed_chars = PassphraseGenerator::CHAR_ALPHA;
	if (set->value (GenerationSpecial, false).toBool())
		allowed_chars |= PassphraseGenerator::CHAR_SPECIAL;
	if (set->value (GenerationNumerics, true).toBool())
		allowed_chars |= PassphraseGenerator::CHAR_NUMERIC;
	if (set->value (GenerationMixed, true).toBool())
		allowed_chars |= PassphraseGenerator::CHAR_LOWER_UPPERCASE;
	mGen->setAllowedCharacters(allowed_chars);
	mGen->setMinLength( set->value (GenerationMin, 12).toInt() );
	mGen->setMaxLength( set->value (GenerationMax, 15).toInt() );
}

void SettingsDialog::saveSettings () {
	for (const Settings::Option &opt : Settings::configOptions) {
		opt.readFromUi (mUi.get(), set);
	}
	set->sync();
	readSettings(set, mGen, mSaveOpt);
}

void SettingsDialog::trySave () {
	int answer = QMessageBox::Yes;
	// maybe we only want to check changes: add mSaveOpt->use_encryption &&
	if (!mUi->encryptionCheckBox->isChecked()) {
		answer = QMessageBox::question(this, tr("Deactivate encryption"),
			tr("<p>You have chosen to save all files <b>WITHOUT ANY ENCRYPTION</b>.</p>"
			"<p>After you confirm this action and save a key-database to disk,"
			"anyone who has access to those files can read all sensitive data stored in it</p>"
			"<p>Disabling encryption is <b>NOT RECOMMENDED</b></p>"
			"<p>Are you sure you want to deactivate encryption?</p>"),
			QMessageBox::Yes | QMessageBox::Abort);
	}
	if (answer == QMessageBox::Yes) {
		saveSettings();
		this->accept();
	}
}
