#pragma once
#include <QMenu>
#include <QObject

namespace plot {

	struct CtxCurveConfig : QObject
	{
		Q_OBJECT
	public:
		CtxCurveConfig (CurveConfig & config)
		{
		}
	};


	struct CtxAxisConfig : QObject
	{
		Q_OBJECT
	public:
		CtxAxisConfig (AxisConfig & config)
		{
		}
	};


	struct CtxPlotConfig : QObject
	{
		Q_OBJECT
	public:
		CtxPlotConfig (PlotConfig & config)
		{
			m_toggle_ref = new QAction("Toggle Ref", this);
			m_hide_prev = new QAction("Hide prev rows", this);
			m_exclude_fileline = new QAction("Exclude File:Line", this);
		}

		QAction * m_tag;
		QList<CtxCurveConfig *> m_ccfg;
		QList<CtxAxisConfig *> m_acfg;
		QAction * m_timer_delay_ms;


		void onShowPlotContextMenu (QPoint const & pos)
		{
			//QPoint globalPos = m_table_view_widget->mapToGlobal(pos);

			QAction * selectedItem = m_ctx_menu.exec(globalPos); // @TODO: rather async
			if (selectedItem == m_hide_prev)
			{
				onHidePrevFromRow();
			}
			if (selectedItem == m_toggle_ref)
			{
				onToggleRefFromRow();
			}
			else if (selectedItem == m_exclude_fileline)
			{
				onExcludeFileLine(m_last_clicked);
			}
			else
			{ }
	}

