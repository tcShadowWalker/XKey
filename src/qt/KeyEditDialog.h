#ifndef KEY_EDIT_DIALOG_H
#define KEY_EDIT_DIALOG_H

#include <QDialog>

namespace XKey {
	class Entry;
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
	KeyEditDialog (XKey::Entry *r, QWidget *parent, XKey::PassphraseGenerator *gen = 0);
	~KeyEditDialog();
	
	

public slots:
	void setPasswordHidden (bool hidden);
	void generatePassphraseClicked ();
	
	void makeChanges ();
	
private:
	Ui::EditEntryDialog *mUi;
	XKey::Entry *mEntry;
	XKey::PassphraseGenerator *mGen;
	
	QString generatePassphrase ();
};

#endif
