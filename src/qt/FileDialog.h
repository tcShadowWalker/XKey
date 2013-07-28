#ifndef SAVE_FILE_DIALOG_H
#define SAVE_FILE_DIALOG_H

#include <QFileDialog>

struct SaveFileOptions {
	bool use_encryption;
	enum EncryptionType {
		AES_256
	} encType;
	bool use_encoding;
};

class SaveFileDialog
	: public QFileDialog
{
public:
	SaveFileDialog (QWidget *parent);
	
	SaveFileOptions getSaveFileOptions ();
	
private:
	SaveFileOptions mSaveOpt;
};

#endif
