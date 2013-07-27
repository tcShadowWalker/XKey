#include "FolderListModel.h"
#include "../XKey.h"
#include <algorithm>
#include <cassert>
#include <qstringlist.h>

#include <iostream>

FolderListModel::FolderListModel(QObject *parent)
	: QAbstractTableModel(parent), root(0)
{
}

FolderListModel::~FolderListModel() {

}

void FolderListModel::setRootFolder (XKey::Folder *r) {
	this->root = r;
	std::cout << "root: " << r << "\n";
	beginResetModel();
	endResetModel();
}

QModelIndex FolderListModel::parent (const QModelIndex &index) const {
	if (!index.isValid())
		return QModelIndex();
	const XKey::Folder *childItem = static_cast<const XKey::Folder*> (index.internalPointer());
	assert (childItem);
	const XKey::Folder *parentItem = childItem->parent();

	if (parentItem == 0)
		return QModelIndex();
	return createIndex(parentItem->row(), 0, (void*)parentItem);
}

QModelIndex FolderListModel::index ( int row, int column, const QModelIndex &parent) const {
	const XKey::Folder *parentItem;
	if (!parent.isValid()) {
		parentItem = root;
		//return createIndex(row, 0, (void*)root );
	} else {
		assert (parent.internalPointer());
		parentItem = static_cast<const XKey::Folder *> (parent.internalPointer());
	}
	if (row >= parentItem->subfolders().size()) {
		return QModelIndex();
	}
	const XKey::Folder *f = &parentItem->subfolders().at(row);
	QModelIndex ind = createIndex(row, 0, (void*)f );
	return ind;
}

int FolderListModel::rowCount (const QModelIndex &parent) const {
	if (!root)
		return 0;
	if (parent.column() > 0)
		return 0;
	if (!parent.isValid())
		//return 1;
		return root->subfolders().size();
	const XKey::Folder *parentItem = static_cast<const XKey::Folder *> (parent.internalPointer());
	return parentItem->subfolders().size();
}

int FolderListModel::columnCount (const QModelIndex &parent) const {
	return 1;
}

QVariant FolderListModel::data (const QModelIndex &index, int role) const {
	if (role == Qt::DisplayRole) {
		const XKey::Folder *parentItem = static_cast<const XKey::Folder *> (index.internalPointer());
		return QString::fromStdString( parentItem->name() );
	}
	return QVariant();
}

