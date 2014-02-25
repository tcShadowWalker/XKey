#ifndef SAVE_FILE_DIALOG_H
#define SAVE_FILE_DIALOG_H

#include <QFileDialog>

namespace Ui {
class OpenFilePassphraseDialog;
class SaveFilePassphraseDialog;
}

struct SaveFileOptions {
	bool use_encryption;
	enum EncryptionType {
		AES_256
	} cipherType;
	bool use_encoding;
	bool write_header;
	bool save_password;
	
	int makeCryptStreamMode () const;
	
	inline SaveFileOptions() : use_encryption(true), cipherType(AES_256), use_encoding(true), write_header(true), save_password(false) { }
	inline ~SaveFileOptions () {
		// Clear passphrase on deletion
		std::fill (_lastPassword.begin(), _lastPassword.end(), '\0');
	}
	
	inline void setLastPassword (std::string pwd) {
		// Only store the password if that is allowed.
		// Resetting with an empty password is always allowed
		if (save_password || pwd.empty())
			this->_lastPassword = std::move(pwd);
	}
	
	inline std::string password () const {
		return _lastPassword;
	}
	
private:
	std::string _lastPassword;
};

class SaveFileDialog
	: public QFileDialog
{
	Q_OBJECT
public:
	SaveFileDialog (QWidget *parent, SaveFileOptions options = SaveFileOptions());
	
	void setDefaultFile (const QString &filepath);
	
	inline SaveFileOptions &saveFileOptions ()  { return mSaveOpt; }
	
private:
	SaveFileOptions mSaveOpt;
};

class FilePasswordDialog
	: public QDialog
{
	Q_OBJECT
public:
	enum Operation {
		READ = 1,
		WRITE = 2
	};
	
	FilePasswordDialog (Operation op, QWidget *parent);
	
	int exec ();
	
	QString password () const;
public slots:
	void accept ();
private:
	Ui::OpenFilePassphraseDialog *openUi;
	Ui::SaveFilePassphraseDialog *saveUi;
};

#endif
