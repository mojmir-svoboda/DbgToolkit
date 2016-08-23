#pragma once
#include <QList>
#include <QVariant>
#include <QModelIndex>

enum E_ActionHandleType {
	  e_Sync  = 0
	, e_Async = 1
};

enum E_ActionType {
	  e_Visibility = 0
	, e_InCentralWidget
	, e_Close
	, e_SyncGroup
	, e_Select
	, e_AlignH
	, e_AlignV
	, e_Find
	, e_FindAll
	, e_FindAllRefs
	, e_QuickString
	, e_Pop
	, e_Colorize
	, e_ColorTagLastLine
	, e_ScrollToLastLine
  , e_DeleteAllData
  , e_SetRefSTime
	, e_max_action_type
};

struct ActionAble;

struct Action {
	E_ActionType m_type { e_Visibility };
	bool m_broadcast { false };
	ActionAble * m_src { nullptr };
	QStringList  m_src_path;
	mutable ActionAble * m_dst { nullptr };
	QStringList m_dst_path;
	mutable int m_dst_curr_level { 0 };

	QList<QVariant> m_args;
	
	virtual ~Action () { }
	virtual E_ActionType type () const { return m_type; }
};


struct ActionAble {

	QStringList m_path;
	QString m_joined_path;

	ActionAble (QStringList const & path) : m_path(path), m_joined_path(path.join("/")) { }
	ActionAble (QStringList const & path, QModelIndex const & idx) : m_path(path), m_joined_path(path.join("/")) { }
	virtual ~ActionAble () { }

	virtual bool handleAction (Action * a, E_ActionHandleType sync) = 0;
	virtual QWidget * controlWidget () = 0;

	QStringList const & path () const { return m_path; }
	QString const & joinedPath () const { return m_joined_path; }
};



