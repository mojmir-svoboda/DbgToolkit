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
	~FindWidget();

	FindConfig & getConfig () { return m_config; }
	FindConfig const & getConfig () const { return m_config; }
	void applyConfig (FindConfig & cfg);
	void applyConfig ();

public slots:
	void onCancel ();
	void onEditTextChanged (QString str);
	void onFocusChanged (QWidget * old, QWidget * now);
	void onReturnPressed ();
	
protected:
	void setConfigValuesToUI (FindConfig const & cfg);
	void setUIValuesToConfig (FindConfig & cfg);
	void makeActionFind (QString const & str, Action & a);

private:
	friend class MainWindow;
	Ui::FindWidget * m_ui;
	MainWindow * m_main_window;
	FindConfig m_config;
	DockedWidgetBase * m_dwb;
};

