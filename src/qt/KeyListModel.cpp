#include "KeyListModel.h"
#include "../XKey.h"

#include <qstringlist.h>

#include <iostream>

static const QStringList headerColumns {QObject::tr("Username"), QObject::tr("Password"), QObject::tr("URL")};

KeyListModel::KeyListModel(QObject *parent)
	: QAbstractTableModel(parent), folder(0)
{
}

KeyListModel::~KeyListModel() {

}

void KeyListModel::setCurrentFolder (XKey::Folder *r) {
	this->folder = r;
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
	return Qt::ItemIsSelectable;
}

int KeyListModel::rowCount (const QModelIndex &) const {
	if (!folder)
		return 0;
	return this->folder->entries().size();
}

int KeyListModel::columnCount (const QModelIndex &) const {
	return headerColumns.size();
}

QVariant KeyListModel::data (const QModelIndex &index, int role) const {
	if (role == Qt::DisplayRole) {
		const XKey::Entry &entry = folder->entries().at(index.row());
		if (index.column() == 0)
			return QString::fromStdString ( entry.username() );
		else if (index.column() == 0)
			return QString( "**********" );
		else
			return QString::fromStdString( entry.url() );
	} else
		return QVariant();
}

