#pragma once
#include <QList>
#include <QVariant>
#include <QModelIndex>

enum E_ActionHandleType {
	  e_Sync  = 0
	, e_Async = 1
};

enum E_ActionType {
	  e_Unknown = 0
	, e_Visibility
	, e_ToCentralWidget
	, e_SetSyncGroup
	, e_SyncGroup
	, e_AlignH
	, e_AlignV
};

struct ActionAble;

struct Action {
	ActionAble * m_src;
	QStringList m_src_path;
	mutable ActionAble * m_dst;
	QStringList m_dst_path;
	mutable int m_dst_curr_level;

	QList<QVariant> m_args;
	
	Action () : m_src(0), m_dst(0), m_dst_curr_level(0) { }
	virtual ~Action () { }
	virtual E_ActionType type () const = 0;
};

struct ActionVisibility : Action {

	virtual E_ActionType type () const { return e_Visibility; }
};


struct ActionAble {

	QStringList m_path;
	QString m_joined_path;
	QModelIndex m_idx;

	ActionAble (QStringList const & path) : m_path(path), m_joined_path(path.join("/")), m_idx() { }
	ActionAble (QStringList const & path, QModelIndex const & idx) : m_path(path), m_joined_path(path.join("/")), m_idx(idx) { }
	virtual ~ActionAble () { }

	virtual bool handleAction (Action * a, E_ActionHandleType sync) = 0;

	QStringList const & path () const { return m_path; }
	QString const & joinedPath () const { return m_joined_path; }
	QModelIndex const & index () const { return m_idx; }
};



