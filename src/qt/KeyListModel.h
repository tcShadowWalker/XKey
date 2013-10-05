#ifndef KEYLISTMODEL_H
#define KEYLISTMODEL_H

#include <qabstractitemmodel.h>

namespace XKey {
	class Folder;
	class Entry;
}

class KeyListModel
	: public QAbstractTableModel
{
	Q_OBJECT
public:
	KeyListModel(QObject *parent);
	~KeyListModel();

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	
	void addEntry (XKey::Entry entry);
	void removeEntry (int index);
	
	Qt::ItemFlags flags ( const QModelIndex & index ) const;
	
	void setCurrentFolder (XKey::Folder *r);
	
	inline XKey::Folder *folder () const { return _folder; }
	
	QStringList mimeTypes () const;
	QMimeData *mimeData (const QModelIndexList &indexes) const;
private:
	XKey::Folder *_folder;
};

#endif
