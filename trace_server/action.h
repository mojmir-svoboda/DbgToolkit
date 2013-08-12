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

struct Action {
	QStringList m_src_path;
	DockedWidgetBase * m_src;
	QStringList m_dst_path;
	DockedWidgetBase * m_dst;
	QList<QVariant> m_args;
	
	Action () : m_src(0), m_dst(0) { }
	virtual ~Action () { }
	virtual E_ActionType type () const = 0;
};

struct ActionVisibility : Action {

	virtual E_ActionType type () const { return e_Visibility; }
};



