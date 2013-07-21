#ifndef KEYLISTMODEL_H
#define KEYLISTMODEL_H

#include <qabstractitemmodel.h>

class KeyListModel
	: public QAbstractTableModel
{
	Q_OBJECT
public:
	KeyListModel();
	~KeyListModel();

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
};

#endif
