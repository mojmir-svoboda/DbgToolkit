#pragma once
#include <QWidget>
#include "quickstringconfig.h"
#include "action.h"
#include "types.h"
//#include <dock/dock.h>

class MainWindow;

namespace Ui {
	class QuickStringWidget;
}

class QuickStringWidget : public QWidget
{
	Q_OBJECT

public:
	explicit QuickStringWidget (MainWindow * mw, QWidget * parent = 0);
	QuickStringWidget (QWidget * parent = 0);
	~QuickStringWidget();

	QuickStringConfig & getConfig () { return m_config; }
	QuickStringConfig const & getConfig () const { return m_config; }
	void applyConfig (QuickStringConfig & cfg);
	void applyConfig ();
	void setActionAbleWidget (ActionAble * aa) { m_aa = aa; }
	void setBroadcasting () { m_broadcasting = true; }

public slots:
	void onCancel ();
	void onEditTextChanged (QString str);
	void onReturnPressed ();
	void onActivate ();
	void onResetRegexpState ();
	void onDataChanged (QModelIndex const &, QModelIndex const &);
	void onAdd ();

protected:
	void init ();
	void clearUI ();
	void setConfigValuesToUI (QuickStringConfig const & cfg);
	void setUIValuesToConfig (QuickStringConfig & cfg);
	void mkAction (QString const & str, Action & a);
	void signalRegexpState (E_ExprState state, QString const & reason);

private:
	friend class MainWindow;
	Ui::QuickStringWidget *	m_ui { nullptr };
	MainWindow *		m_main_window { nullptr };
	QuickStringConfig			m_config;
	ActionAble *		m_aa { nullptr };
	bool m_broadcasting { false };
};

