#pragma once
#include <QWidget>
#include "findconfig.h"
#include "action.h"
#include "dock.h"

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
	void focusNext ();
	void focusPrev ();
	void setActionAbleWidget (ActionAble * aa) { m_aa = aa; }

public slots:
	void onCancel ();
	void onEditTextChanged (QString str);
	void onFocusChanged (QWidget * old, QWidget * now);
	void onReturnPressed ();
	void onFindAllRefs ();
	void onFindAllClone ();
	void onFindAllSelect ();
	void onFindNext ();
	void onFindPrev ();
	void onActivate ();
	void onResetRegexpState ();

protected:
	void init ();
	void find ();
	void find (bool select, bool refs, bool clone);
	void find (bool prev, bool next);
	void clearUI ();
	void setConfigValuesToUI (FindConfig const & cfg);
	void setUIValuesToConfig (FindConfig & cfg);
	void makeActionFind (QString const & str, Action & a);
	bool isMovingFindWidget () const { return m_moving_widget; }
	void signalRegexpState (E_ExprState state, QString const & reason);

private:
	friend class MainWindow;
	Ui::FindWidget *	m_ui;
	MainWindow *		m_main_window;
	FindConfig			m_config;
	ActionAble *		m_aa;
	bool				m_moving_widget;
};

