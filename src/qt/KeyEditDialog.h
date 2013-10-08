#ifndef KEY_EDIT_DIALOG_H
#define KEY_EDIT_DIALOG_H

#include <QDialog>

namespace XKey {
	class Entry;
	class Folder;
	class PassphraseGenerator;
}

namespace Ui {
	class EditEntryDialog;
}

class KeyEditDialog
	: public QDialog
{
	Q_OBJECT
public:
	KeyEditDialog (XKey::Entry *r, XKey::Folder *folder, QWidget *parent, XKey::PassphraseGenerator *gen);
	~KeyEditDialog();
	
	

public slots:
	void setPasswordHidden (bool hidden);
	void generatePassphraseClicked ();
	
	void makeChanges ();
	
	void copyToClipboard ();
	
private:
	Ui::EditEntryDialog *mUi;
	XKey::Entry *mEntry;
	XKey::PassphraseGenerator *mGen;
	
	QString generatePassphrase ();
};

#endif
