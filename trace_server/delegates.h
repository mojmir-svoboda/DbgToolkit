#pragma once
#include <QStyledItemDelegate>
#include <QItemDelegate>
#include "filterstate.h"
#include "appdata.h"

class SyncedTableItemDelegate : public QStyledItemDelegate
{
public: 
    SyncedTableItemDelegate (QObject *parent = 0) : QStyledItemDelegate(parent) { }

    void paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;
	void paintHilited (QPainter * painter, QStyleOptionViewItemV4 & option, QModelIndex const & index) const;
    
private slots:
};



class LevelDelegate : public QStyledItemDelegate
{
	FilterState const & m_filter_state;
public: 
    LevelDelegate (FilterState & fs, QObject *parent = 0) : QStyledItemDelegate(parent), m_filter_state(fs) { }

    void paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;
    
private slots:
};

class CtxDelegate : public QStyledItemDelegate
{
	FilterState const & m_filter_state;
	AppData const & m_app_data;
public: 
    CtxDelegate (FilterState & fs, AppData const & ad, QObject *parent = 0) : QStyledItemDelegate(parent), m_filter_state(fs), m_app_data(ad) { }

    void paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;
    
private slots:
};

class StringDelegate : public QStyledItemDelegate
{
	FilterState const & m_filter_state;
public: 
    StringDelegate (FilterState & fs, QObject *parent = 0) : QStyledItemDelegate(parent), m_filter_state(fs) { }

    void paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;
    
private slots:
};
class RegexDelegate : public QStyledItemDelegate
{
	FilterState const & m_filter_state;
public: 
    RegexDelegate (FilterState & fs, QObject *parent = 0) : QStyledItemDelegate(parent), m_filter_state(fs) { }

    void paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;
    
private slots:
};


class SizeDelegate : public QItemDelegate
{
	QVector<int> & m_sizes;
public:

	SizeDelegate (QVector<int> & s) : m_sizes(s) { }
	QSize sizeHint (QStyleOptionViewItem const & option, QModelIndex const & index) const;
};

