#pragma once
#include <QStyledItemDelegate>
#include <QItemDelegate>
//#include "filterstate.h"
//#include "appdata.h"

class SyncedTableItemDelegate : public QStyledItemDelegate
{
public: 
    SyncedTableItemDelegate (QObject *parent = 0) : QStyledItemDelegate(parent) { }

    void paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;
	void paintHilited (QPainter * painter, QStyleOptionViewItemV4 & option, QModelIndex const & index) const;
    
private slots:
};


class SizeDelegate : public QItemDelegate
{
	QVector<int> & m_sizes;
public:

	SizeDelegate (QVector<int> & s) : m_sizes(s) { }
	QSize sizeHint (QStyleOptionViewItem const & option, QModelIndex const & index) const;
};

