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
	
	QModelIndex index ( int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent (const QModelIndex &index) const;
	
	void setRootFolder (XKey::Folder *r);
private:
	XKey::Folder *root;
};

#endif
