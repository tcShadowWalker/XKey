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
	beginResetModel();
	endResetModel();
}

QModelIndex FolderListModel::index ( int row, int column, const QModelIndex &parent) const {
	const XKey::Folder *parentItem;
	if (!parent.isValid()) {
		parentItem = root;
	} else {
		assert (parent.internalPointer());
		parentItem = static_cast<const XKey::Folder *> (parent.internalPointer());
	}
	if (row >= parentItem->subfolders().size()) {
		std::cout << "creat INVALID ind " << row << ", parent: " << parentItem->name() << "\n";
		return QModelIndex();
	}
	const XKey::Folder *f = &parentItem->subfolders().at(row);
	//std::cout << "creat ind " << row << ", "  << f->name() << ", parent: " << parentItem << "\n";
	return createIndex(row, 0, (void*)f );
}

int FolderListModel::rowCount (const QModelIndex &parent) const {
	if (!root)
		return 0;
	if (parent.column() > 0)
		return 0;
	if (!parent.isValid())
		return root->subfolders().size();
	const XKey::Folder *parentItem = static_cast<const XKey::Folder *> (parent.internalPointer());
	std::cout << "rc: "  << parent.row() << ", " << parentItem->name() << ", " << parentItem << ": " << parentItem->subfolders().size() << "\n";
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

