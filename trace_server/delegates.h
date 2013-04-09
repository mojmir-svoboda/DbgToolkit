#pragma once
#include <QStyledItemDelegate>
#include "sessionstate.h"

class TableItemDelegate : public QStyledItemDelegate
{
	SessionState const & m_session_state;
public: 
    TableItemDelegate (SessionState & ss, QObject *parent = 0) : QStyledItemDelegate(parent), m_session_state(ss) { }

    void paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;
    void paintCustom (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;

	void paintTokenized (QPainter * painter, QStyleOptionViewItemV4 & option, QModelIndex const & index, QString const & separator, QString const & out_separator, int level = 1) const;
	void paintContext (QPainter * painter, QStyleOptionViewItemV4 & option, QModelIndex const & index) const;
	void paintTime (QPainter * painter, QStyleOptionViewItemV4 & option, QModelIndex const & index) const;

	void paintHilited (QPainter * painter, QStyleOptionViewItemV4 & option, QModelIndex const & index) const;
    
private slots:
};


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
	SessionState const & m_session_state;
public: 
    LevelDelegate (SessionState & ss, QObject *parent = 0) : QStyledItemDelegate(parent), m_session_state(ss) { }

    void paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;
    
private slots:
};

class CtxDelegate : public QStyledItemDelegate
{
	SessionState const & m_session_state;
public: 
    CtxDelegate (SessionState & ss, QObject *parent = 0) : QStyledItemDelegate(parent), m_session_state(ss) { }

    void paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;
    
private slots:
};


