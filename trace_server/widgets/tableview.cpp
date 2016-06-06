#include "tableview.h"
#include <QEvent>
#include <QHelpEvent>
#include <QHeaderView>
#include <QScrollBar>
#include "logs/logtablemodel.h"

TableView::TableView (QWidget * parent)
	: QTableView(parent)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
}

TableView::~TableView () 
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
}

/*void TableView::scrollTo (QModelIndex const & index, ScrollHint hint)
{
	QTableView::scrollTo(index, hint);
}*/

