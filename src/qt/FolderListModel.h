#ifndef FOLDERLISTMODEL_H
#define FOLDERLISTMODEL_H

#include <qabstractitemmodel.h>

namespace XKey {
	class Folder;
}

class FolderListModel
	: public QAbstractTableModel
{
	Q_OBJECT
public:
	FolderListModel(QObject *parent);
	~FolderListModel();

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	
	Qt::ItemFlags flags ( const QModelIndex & index ) const;
	QModelIndex index ( int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent (const QModelIndex &index) const;
	
	bool insertRow (int row, const QModelIndex &parent = QModelIndex());
	bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() );
	
	bool setData (const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

	QStringList mimeTypes () const;
	QMimeData *mimeData (const QModelIndexList &indexes) const;
	bool dropMimeData ( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent );
	
	void setRootFolder (XKey::Folder *r);
protected:
	Qt::DropActions supportedDropActions () const;
	
private:
	XKey::Folder *root;
};

#endif
