#pragma once
#include <QStyledItemDelegate>
#include "appdata.h"
#include "logwidget.h"

namespace logs {

	class LogDelegate : public QStyledItemDelegate
	{
		LogTableView const & m_log_widget;
		AppData const & m_app_data;
	public: 
		LogDelegate (LogTableView & lw, AppData const & ad, QObject * parent = 0) 
			: QStyledItemDelegate(parent), m_log_widget(lw), m_app_data(ad)
		{ }

    ~LogDelegate() {}

		void paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;
		void paintCustom (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;
		void paintTokenized (QPainter * painter, QStyleOptionViewItemV4 & option, QModelIndex const & index, QString const & separator, QString const & out_separator, int level = 1) const;
		void paintContext (QPainter * painter, QStyleOptionViewItemV4 & option, QModelIndex const & index) const;
		void paintTime (QPainter * painter, QStyleOptionViewItemV4 & option, QModelIndex const & index) const;
		void paintHilited (QPainter * painter, QStyleOptionViewItemV4 & option, QModelIndex const & index) const;
	};

}

