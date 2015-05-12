#pragma once

#include <QFileDialog>

namespace Ui {
class OpenFilePassphraseDialog;
class SaveFilePassphraseDialog;
}

const int DEFAULT_KEY_ITERATION_COUNT = 100000;
extern const char *DEFAULT_CIPHER_ALGORITHM;

struct SaveFileOptions {
	bool use_encryption;
	std::string cipher_name;
	bool use_encoding;
	bool write_header;
	bool always_ask_password;
	int key_iteration_count;
	
	int makeCryptStreamMode () const;
	
	inline SaveFileOptions() : use_encryption(true), cipher_name(DEFAULT_CIPHER_ALGORITHM),
		use_encoding(true), write_header(true), always_ask_password(true), key_iteration_count(DEFAULT_KEY_ITERATION_COUNT) { }
	inline ~SaveFileOptions () {
		// Clear passphrase on destruction
		std::fill (_lastPassword.begin(), _lastPassword.end(), '\0');
	}
	
	inline void setLastPassword (std::string pwd) {
		// Only store the password if that is allowed.
		// Resetting with an empty password is always allowed
		if (!always_ask_password || pwd.empty()) {
			std::fill (_lastPassword.begin(), _lastPassword.end(), '\0');
			this->_lastPassword = std::move(pwd);
		}
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
