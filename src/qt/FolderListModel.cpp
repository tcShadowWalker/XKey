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

bool FolderListModel::insertRow (int row, const QModelIndex &parent) {
	XKey::Folder *parentItem = (!parent.isValid()) ? root : static_cast<XKey::Folder *> (parent.internalPointer());
	assert (parentItem);
	std::cout << "insert: " << parentItem->subfolders().size() << "\n";
	beginInsertRows(parent, parentItem->subfolders().size(), parentItem->subfolders().size()+1);
	parentItem->createSubfolder("New Folder");
	endInsertRows();
	return true;
}

bool FolderListModel::removeRows (int row, int count, const QModelIndex &parent) {
	XKey::Folder *parentItem = (!parent.isValid()) ? root : static_cast<XKey::Folder *> (parent.internalPointer());
	assert (parentItem);
	try {
		beginRemoveRows(parent, row, row+count);
		int m = std::min ((int)parentItem->subfolders().size(), count + row);
		std::cout << "del: " << row << ", " << m << "\n";
		parentItem->subfolders().erase (parentItem->subfolders().begin() + row, parentItem->subfolders().begin() + m);
		endRemoveRows();
	} catch (const std::exception &e) {
		std::cout << "fail: " << e.what() << "\n";
		endRemoveRows();
		return false;
	}
	return true;
}

Qt::ItemFlags FolderListModel::flags ( const QModelIndex & index ) const {
	if (!index.isValid())
		return Qt::NoItemFlags;
	return Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant FolderListModel::data (const QModelIndex &index, int role) const {
	const XKey::Folder *parentItem = static_cast<const XKey::Folder *> (index.internalPointer());
	if (role == Qt::DisplayRole) {
		return QString::fromStdString( parentItem->name() );
	} else if (role == Qt::EditRole) {
		return QString::fromStdString( parentItem->name() );
	}
	return QVariant();
}

bool FolderListModel::setData (const QModelIndex & index, const QVariant & value, int role) {
	if (!index.isValid() || role != Qt::EditRole || value.type() != QVariant::String) {
		return false;
	} else {
		XKey::Folder *parentItem = static_cast<XKey::Folder *> (index.internalPointer());
		QString name = value.toString();
		parentItem->setName (name.toStdString());
	}
	return true;
}
