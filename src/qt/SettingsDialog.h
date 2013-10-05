#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <QDialog>

struct SaveFileOptions;
class QSettings;
class QAbstractButton;
namespace XKey {
	class PassphraseGenerator;
}

namespace Ui {
	class SettingsDialog;
}

class SettingsDialog
	: public QDialog
{
	Q_OBJECT
public:
	SettingsDialog (QSettings *set, XKey::PassphraseGenerator *gen, SaveFileOptions *saveOptions, QWidget *parent);
	~SettingsDialog();
	
	/**
	 * @brief Read saved settings from disk, if available
	 */
	void readSettings ();

public slots:
	void trySave ();
	
private:
	QSettings *set;
	Ui::SettingsDialog *mUi;
	XKey::PassphraseGenerator *mGen;
	SaveFileOptions *mSaveOpt;
	
	void saveSettings ();
	
	QString generatePassphrase ();
};

#endif
