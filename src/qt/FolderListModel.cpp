#include "FolderListModel.h"
#include "../XKey.h"
#include <algorithm>
#include <cassert>
#include <QStringList>
#include <QMimeData>
#include <QDropEvent>
#include <QMessageBox>
#include <QIcon>
#include <iostream>

FolderListModel::FolderListModel(QObject *parent)
	: QAbstractTableModel(parent), root(0)
{ }

FolderListModel::~FolderListModel() { }

void FolderListModel::setRootFolder (XKey::Folder *r) {
	beginResetModel();
	this->root = r;
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

bool FolderListModel::getModelIndex (XKey::Folder *folder, QModelIndex *ind) {
	QModelIndex parentIndex;
	// Recursive up-call
	if (folder->parent()) {
		if (!getModelIndex (folder->parent(), &parentIndex))
			return false;
		*ind = index (folder->row(), 0, parentIndex);
	} else if (folder == root) {
		*ind = QModelIndex();
	} else
		return false;
	return true;
}

int FolderListModel::rowCount (const QModelIndex &parent) const {
	if (!root) {
		return 0;
	}
	if (parent.column() > 0) {
		return 0;
	}
	if (!parent.isValid()) {
		//return 1;
		return root->subfolders().size();
	}
	const XKey::Folder *parentItem = static_cast<const XKey::Folder *> (parent.internalPointer());
	return parentItem->subfolders().size();
}

int FolderListModel::columnCount (const QModelIndex &parent) const {
	return 1;
}

bool FolderListModel::insertRow (int row, const QModelIndex &parent) {
	XKey::Folder *parentItem = (!parent.isValid()) ? root : static_cast<XKey::Folder *> (parent.internalPointer());
	assert (parentItem);
	const int num = parentItem->subfolders().size();
	beginInsertRows(parent, num, num);
	XKey::Folder *created = 0;
	for (int i = 1; !created && i <= 3; ++i) {
		try {
			created = parentItem->createSubfolder( QString("New Folder %1").arg(num+i).toStdString() );
		} catch (const std::exception&) { }
	}
	endInsertRows();
	return (created != 0);
}

bool FolderListModel::removeRows (int row, int count, const QModelIndex &parent) {
	XKey::Folder *parentItem = (!parent.isValid()) ? root : static_cast<XKey::Folder *> (parent.internalPointer());
	assert (parentItem);
	try {
		beginRemoveRows(parent, row, row+count-1);
		for (int i = row+count-1; i >= row; --i ) {
			parentItem->removeSubfolder(i);
		}
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
		return Qt::ItemIsDropEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
	return Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant FolderListModel::data (const QModelIndex &index, int role) const {
	const XKey::Folder *parentItem = static_cast<const XKey::Folder *> (index.internalPointer());
	if (role == Qt::DisplayRole) {
		return QString::fromStdString( parentItem->name() );
	} else if (role == Qt::EditRole) {
		return QString::fromStdString( parentItem->name() );
	} else if (role == Qt::DecorationRole) {
		return QIcon(tr(":/icons/folder.png", "Icon for opened folders in folder-tree"));
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

QMimeData *FolderListModel::mimeData (const QModelIndexList &indexes) const {
	if (indexes.size() == 1) {
		const XKey::Folder *item = static_cast<const XKey::Folder *> (indexes.at(0).internalPointer());
		QMimeData *data = new QMimeData;
		data->setText(QString ("XKey/Folder:") + QString::fromStdString(item->fullPath()));
		return data;
	} else
		return 0;
}

QStringList FolderListModel::mimeTypes () const {
	return QStringList{"text/plain", "application/x-xkey-folder", "application/x-xkey-entry"};
}

Qt::DropActions FolderListModel::supportedDropActions () const {
	return Qt::MoveAction;
}

bool FolderListModel::dropMimeData (const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent) {
	const QString text = data->text();
	if (text.startsWith("XKey/Folder:")) {
		QString fullPath = text.mid(12);
		XKey::Folder *parentItem = 0;
		if (!parent.isValid()) {
			parentItem = root;
		} else {
			parentItem = static_cast<XKey::Folder *> (parent.internalPointer());
		}
		if (!parentItem)
			return false;
		if (row < -1 || row >= (int)parentItem->subfolders().size()) {
			return false;
		}
		
		XKey::Folder *oldFolder = const_cast<XKey::Folder *> (XKey::get_folder_by_path(root, fullPath.toStdString())),
			*oldParent = (oldFolder) ? oldFolder->parent() : 0;
		if (!oldFolder || !oldParent || oldParent == parentItem)
			return false;
		
		try {
			QModelIndex oldIndex;
			if (!getModelIndex (oldFolder, &oldIndex))
				return false;
			beginRemoveRows(oldIndex.parent(), oldIndex.row(), 1);
			beginInsertRows(parent, parentItem->subfolders().size(), parentItem->subfolders().size()+1);
			// move
			XKey::moveFolder (oldFolder, parentItem, parentItem->subfolders().size());
			//
			endInsertRows();
			endRemoveRows();
			return true;
		} catch (const std::exception &e) {
			QMessageBox::warning (0, tr("Error"), tr("Failed to move folders: %1").arg(QString(e.what())) );
			endInsertRows();
			endRemoveRows();
		}
	} else if (text.startsWith("XKey/Entry:")) {
		QString fullPath = text.mid(11);
		std::cout << "move entry: " << fullPath.toStdString() << "\n";
	}
	return false;
}

