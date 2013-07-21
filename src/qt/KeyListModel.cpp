#include "KeyListModel.h"
#include <qstringlist.h>

#include <iostream>

static QStringList headerColumns;

KeyListModel::KeyListModel()
{
	headerColumns << tr("Username") << tr("Password") << tr("URL");
}

KeyListModel::~KeyListModel() {

}

QVariant KeyListModel::headerData (int section, Qt::Orientation orientation,  int role) const {
	if (orientation == Qt::Horizontal) {
		if (section >= 0 && section < headerColumns.size())
			std::cout << headerColumns[section].toStdString() << "\n";
			return headerColumns[section];
	}
	return QVariant();
}

int KeyListModel::rowCount(const QModelIndex &) const {
	return 1;
}

int KeyListModel::columnCount (const QModelIndex &) const {
	return headerColumns.size();
}

QVariant KeyListModel::data (const QModelIndex &index, int role) const {
	return QString("Hallo");
}

