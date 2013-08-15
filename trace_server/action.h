#pragma once
#include <QList>
#include <QVariant>

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
	ActionAble * m_dst;
	QStringList m_dst_path;
	QList<QVariant> m_args;
	
	Action () : m_src(0), m_dst(0) { }
	virtual ~Action () { }
	virtual E_ActionType type () const = 0;
};

struct ActionVisibility : Action {

	virtual E_ActionType type () const { return e_Visibility; }
};


struct ActionAble {

	QStringList m_path;

	ActionAble (QStringList const & path) : m_path(path) { }
	virtual ~ActionAble () { }

	virtual bool handleAction (Action * a, bool sync) = 0;
};



