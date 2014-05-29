#include "KeyListModel.h"
#include "../XKey.h"

#include <qstringlist.h>
#include <QMimeData>

#include <iostream>

static const QStringList headerColumns {QObject::tr("Title"), QObject::tr("Username"), QObject::tr("URL"), QObject::tr("E-Mail")};

KeyListModel::KeyListModel(QObject *parent)
	: QAbstractTableModel(parent), _folder(0)
{ }

KeyListModel::~KeyListModel() {
	
}

void KeyListModel::setCurrentFolder (XKey::Folder *r) {
	this->_folder = r;
	beginResetModel();
	endResetModel();
}

QVariant KeyListModel::headerData (int section, Qt::Orientation orientation,  int role) const {
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
		if (section >= 0 && section < headerColumns.size())
			return headerColumns[section];
	}
	return QVariant();
}

Qt::ItemFlags KeyListModel::flags ( const QModelIndex &) const {
	return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled;
}

int KeyListModel::rowCount (const QModelIndex &) const {
	if (!_folder)
		return 0;
	return this->_folder->entries().size();
}

int KeyListModel::columnCount (const QModelIndex &) const {
	return headerColumns.size();
}

QVariant KeyListModel::data (const QModelIndex &index, int role) const {
	if (role == Qt::DisplayRole) {
		const XKey::Entry &entry = _folder->entries().at(index.row());
		if (index.column() == 0)
			return QString::fromStdString ( entry.title() );
		else if (index.column() == 1)
			return QString::fromStdString ( entry.username() );
		else if (index.column() == 2)
			return QString::fromStdString( entry.url() );
		else
			return QString::fromStdString( entry.email() );
		
	} else
		return QVariant();
}

void KeyListModel::addEntry (XKey::Entry entry) {
	if (!_folder)
		return;
	beginInsertRows(QModelIndex(), _folder->entries().size(), _folder->entries().size());
	_folder->addEntry(entry);
	endInsertRows();
}

void KeyListModel::removeEntry (int index) {
	if (!_folder)
		return;
	beginRemoveRows(QModelIndex(), index, index);
	_folder->removeEntry(index);
	endRemoveRows();
}

bool KeyListModel::removeRows (int row, int count, const QModelIndex & parent) {
	try {
		beginRemoveRows(parent, row, row+count-1);
		for (int i = row+count-1; i >= row; --i ) {
			_folder->removeEntry(i);
		}
		endRemoveRows();
	} catch (const std::exception &e) {
		std::cout << "KeyListModel::removeRows fail: " << e.what() << "\n";
		endRemoveRows();
		return false;
	}
	return true;
}

// Drag and drop

Qt::DropActions KeyListModel::supportedDropActions () const {
	return Qt::MoveAction;
}

QStringList KeyListModel::mimeTypes () const {
	return QStringList{"text/plain", "application/x-xkey-entry"};
}

QMimeData *KeyListModel::mimeData (const QModelIndexList &indexes) const {
	std::unique_ptr<QMimeData> data (new QMimeData);
	QByteArray a;
	QStringList rowList;
	a.append(QString::fromStdString(_folder->fullPath()).toUtf8()).append('\0');
	for (QModelIndex ind : indexes) {
		rowList << QString::number(ind.row());
	}
	rowList.removeDuplicates();
	a.append(rowList.join('\0'));
	data->setData ("application/x-xkey-entry", a);
	return data.release();
}


