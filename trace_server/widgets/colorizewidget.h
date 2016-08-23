#pragma once
#include <QWidget>
#include "colorizeconfig.h"
#include "action.h"
#include <dock/dock.h>

class MainWindow;

namespace Ui {
	class ColorizeWidget;
}

class ColorizeWidget : public QWidget
{
	Q_OBJECT

public:
	explicit ColorizeWidget (MainWindow * mw, QWidget * parent = 0);
	ColorizeWidget (QWidget * parent = 0);
	~ColorizeWidget();

	ColorizeConfig & getConfig () { return m_config; }
	ColorizeConfig const & getConfig () const { return m_config; }
	void applyConfig (ColorizeConfig & cfg);
	void applyConfig ();
	void setMainWindow (MainWindow * mw) { m_main_window = mw; }
	void focusNext ();
	void focusPrev ();
	void setActionAbleWidget (ActionAble * aa) { m_aa = aa; }
	void setBroadcasting () { m_broadcasting = true; }

public slots:
	void onCancel ();
	void onEditTextChanged (QString str);
	//void onFocusChanged (QWidget * old, QWidget * now);
	void onReturnPressed ();
	void onColorizeNext ();
	void onColorizePrev ();
	void onColorizeString ();
	void onActivate ();
	void onResetRegexpState ();

protected:
	void init ();
	void colorize ();
	void colorize (bool select, bool refs, bool clone);
	void colorize (bool prev, bool next);
	void clearUI ();
	void setConfigValuesToUI (ColorizeConfig const & cfg);
	void setUIValuesToConfig (ColorizeConfig & cfg);
	void makeActionColorize (QString const & str, Action & a);
	void signalRegexpState (E_ExprState state, QString const & reason);

private:
	friend class MainWindow;
	Ui::ColorizeWidget *	m_ui { nullptr };
	MainWindow *		m_main_window { nullptr };
	ColorizeConfig			m_config;
	ActionAble *		m_aa { nullptr }; 
	bool m_broadcasting { false };
};

