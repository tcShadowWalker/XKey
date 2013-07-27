#ifndef KEY_EDIT_DIALOG_H
#define KEY_EDIT_DIALOG_H

#include <QDialog>

namespace XKey {
	class Entry;
}

namespace Ui {
	class EditEntryDialog;
}

class KeyEditDialog
	: public QDialog
{
	Q_OBJECT
public:
	KeyEditDialog (XKey::Entry *r, QWidget *parent);
	~KeyEditDialog();

public slots:
	void setPasswordHidden (bool hidden);
	
	void makeChanges ();
	
private:
	Ui::EditEntryDialog *mUi;
	XKey::Entry *mEntry;
};

#endif
