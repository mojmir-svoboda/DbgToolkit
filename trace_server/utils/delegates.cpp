#include "delegates.h"
#include <QObject>
#include <QPainter>
#include "connection.h"
#include <models/filterproxymodel.h>


void SyncedTableItemDelegate::paintHilited (QPainter * painter, QStyleOptionViewItemV4 & option, QModelIndex const & index) const
{
    if (option.state & QStyle::State_Selected)
	{
		option.state &= ~ QStyle::State_Selected;
		option.backgroundBrush = QBrush(QColor(244,154,193,255));
    	option.font.setBold(true);
		QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &option, painter);
    }
	else
	{
    	option.font.setBold(false);
		QStyledItemDelegate::paint(painter, option, index);
    }
}

void SyncedTableItemDelegate::paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const
{
    painter->save();
    QStyleOptionViewItemV4 option4 = option;
    initStyleOption(&option4, index);
	paintHilited(painter, option4, index);
	//QStyledItemDelegate::paint(painter, option4, index);
	painter->restore();
}


QSize SizeDelegate::sizeHint (QStyleOptionViewItem const & option, QModelIndex const & index) const
{
	return QSize(128,128);
}


