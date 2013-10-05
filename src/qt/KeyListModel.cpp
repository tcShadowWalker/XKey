#include "KeyListModel.h"
#include "../XKey.h"

#include <qstringlist.h>

#include <iostream>
#include <boost/concept_check.hpp>

static const QStringList headerColumns {QObject::tr("Title"), QObject::tr("Username"), QObject::tr("URL"), QObject::tr("Password")};

KeyListModel::KeyListModel(QObject *parent)
	: QAbstractTableModel(parent), _folder(0)
{
}

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
	return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
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
			return QString( "**********" );
		
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

