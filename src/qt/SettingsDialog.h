#pragma once

#include <QDialog>
#include <memory>

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
	~SettingsDialog ();
	
	/**
	 * @brief Read saved settings from disk, if available
	 */
	static void readSettings (QSettings *settings, XKey::PassphraseGenerator *gen, SaveFileOptions *saveOptions);

public slots:
	void trySave ();
	
private:
	QSettings *set;
	std::unique_ptr<Ui::SettingsDialog> mUi;
	XKey::PassphraseGenerator *mGen;
	SaveFileOptions *mSaveOpt;
	
	void saveSettings ();
	
	QString generatePassphrase ();
};
