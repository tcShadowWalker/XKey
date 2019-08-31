#pragma once

#include <qabstractitemmodel.h>

namespace XKey {
	class Folder;
}

class FolderListModel
	: public QAbstractItemModel
{
	Q_OBJECT
public:
	FolderListModel(QObject *parent);
	~FolderListModel();

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	
	Qt::ItemFlags flags ( const QModelIndex & index ) const override;
	QModelIndex index ( int row, int column, const QModelIndex &parent = QModelIndex()) const override;
	QModelIndex parent (const QModelIndex &index) const override;
	
	bool insertRows (int row, int count, const QModelIndex &parent = QModelIndex()) override;
	bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() ) override;
	
	bool setData (const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;

	QStringList mimeTypes () const override;
	QMimeData *mimeData (const QModelIndexList &indexes) const override;
	bool dropMimeData ( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent ) override;
	
	void setRootFolder (XKey::Folder *r);
	
	bool getModelIndex (const XKey::Folder *folder, QModelIndex *ind);
protected:
	Qt::DropActions supportedDropActions () const override;
	
private:
	XKey::Folder *root;
};
