#include "connection.h"
#include "frameview.h"
#include <QScrollBar>
#include <QSplitter>
#include <utils.h>
#include <utils_qstandarditem.h>
//#include <delegates.h>
#include "ganttview.h"
#include <label.h>
#include "frameview.h"
#include <syncwidgets.h>


DataFrame::DataFrame (Connection * connection, FrameViewConfig & config, QString const & confname, QStringList const & path)
	: DockedData<e_data_frame>(connection, config, confname, path)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	m_widget = new FrameView(connection, 0, m_config, confname, path);
}


dataframes_t::iterator Connection::findOrCreateFrame (QString const & tag)
{
	dataframes_t::iterator it = dataWidgetFactory<e_data_frame>(tag);
	if (it != m_data.get<e_data_frame>().end())
	{
		DataFrame * d = *it;
		//d->onShow();
	}
	else
		assert(false);
	return it;
}

	void GanttWidget::appendFrameEnd (DecodedData & dd)
	{
		gantt::GanttView * gv = findOrCreateGanttView(dd.m_subtag);
		QString const tag = QString("%1").arg(gv->config().m_sync_group);
		dataframes_t::iterator fv_it = m_connection->findOrCreateFrame(tag);

		unsigned long long from, to;
		gv->appendFrameEnd(dd, from, to);
		(*fv_it)->widget().appendFrame(from, to);
	}

	/*dataframes_t::iterator GanttWidget::findOrCreateFrameView (int sync_group)
	{
		QString const tag = QString("%1").arg(sync_group);

		dataframes_t::iterator it = m_connection->dataWidgetFactory<e_data_gantt>(tag);

		m_dataframeviews.insert(sync_group, fv);
			//QModelIndex const item_idx = m_data_model->insertItemWithHint(name, template_config.m_show);
		return it;
	}*/


