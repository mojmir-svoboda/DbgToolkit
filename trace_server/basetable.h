#pragma once
#include <QtGui/qwidget.h>
#include <QColorDialog>
#include "curves.h"
#include "config.h"
//#include "tablectxmenu.h"

namespace table {

	class BaseTable
	{
		Q_OBJECT
	public:
		BaseTable (QObject * oparent, QWidget * wparent, TableConfig & cfg, QString const & fname);

		void applyConfig (TableConfig const & pcfg);
		virtual ~BaseTable ();
		void stopUpdate ();

		TableConfig & getConfig () { return m_config; }
		TableConfig const & getConfig () const { return m_config; }

	protected:
		void timerEvent (QTimerEvent * e);
		virtual void update ();

	public Q_SLOTS:

		void onShow ();
		void onHide ();
		void onHideContextMenu ();
		void onShowContextMenu (QPoint const & pos);
		void setConfigValues (TableConfig const & pcfg);
		void onSaveButton ();
		void onApplyButton ();
		void onResetButton ();
		void onDefaultButton ();

	protected:
		//int m_timer;
		TableConfig & m_config;
		table::CtxTableConfig m_config_ui;
		//QList<QColor> m_colors;
		QString m_fname;
		//std::vector<QwtPlotMarker *> m_markers;
	};
}

