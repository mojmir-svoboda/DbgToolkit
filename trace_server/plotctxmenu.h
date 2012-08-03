#pragma once

namespace plot {

	struct CtxPlotConfig
	{
		CtxPlotConfig (PlotConfig & config)
		{
		}

		QAction * m_tag;
		QAction * m_timer_delay_ms;
	};

    m_toggle_ref = new QAction("Toggle Ref", this);
    m_hide_prev = new QAction("Hide prev rows", this);
    m_exclude_fileline = new QAction("Exclude File:Line", this);


void Connection::onShowContextMenu (QPoint const & pos)
{
    QPoint globalPos = m_table_view_widget->mapToGlobal(pos);

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

