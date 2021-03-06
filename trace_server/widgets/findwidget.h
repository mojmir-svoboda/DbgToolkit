#pragma once
#include <QWidget>
#include "findconfig.h"
#include "action.h"
#include "types.h"
//#include <dock/dock.h>

class MainWindow;

namespace Ui {
	class FindWidget;
}

class FindWidget : public QWidget
{
	Q_OBJECT

public:
	explicit FindWidget (MainWindow * mw, QWidget * parent = 0);
	FindWidget (QWidget * parent = 0);
	~FindWidget();

	FindConfig & getConfig () { return m_config; }
	FindConfig const & getConfig () const { return m_config; }
	void applyConfig (FindConfig & cfg);
	void applyConfig ();
	void setMainWindow (MainWindow * mw) { m_main_window = mw; }
	void setActionAbleWidget (ActionAble * aa) { m_aa = aa; }
	void setBroadcasting () { m_broadcasting = true; }

public slots:
	void onCancel ();
	void onEditTextChanged (QString str);
	void onReturnPressed ();
	void onFindAllRefs ();
	void onFindAllClone ();
	void onFindAllSelect ();
	void onFindNext ();
	void onFindPrev ();
	void onActivate ();
	void onResetRegexpState ();
	void onDataChanged (QModelIndex const &, QModelIndex const &);

protected:
	void init ();
	void find ();
	void find (bool select, bool refs, bool clone);
	void find (bool prev, bool next);
	void clearUI ();
	void setConfigValuesToUI (FindConfig const & cfg);
	void setUIValuesToConfig (FindConfig & cfg);
	void mkAction (QString const & str, Action & a);
	void signalRegexpState (E_ExprState state, QString const & reason);

private:
	friend class MainWindow;
	Ui::FindWidget *	m_ui { nullptr };
	MainWindow *		m_main_window { nullptr };
	FindConfig			m_config;
	ActionAble *		m_aa { nullptr };
	bool m_broadcasting { false };
};

