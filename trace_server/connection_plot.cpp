#include "connection.h"
#include <dock/dock.h>
#include "mainwindow.h"

bool Connection::handlePlotCommand (DecodedCommand const & cmd, E_ReceiveMode mode)
{
	if (getClosestFeatureState(e_data_plot) == e_FtrDisabled)
		return true;

	OCTET_STRING const w = cmd.choice.plot.wdgt;
	QString msg = QString::fromLatin1(reinterpret_cast<char const *>(w.buf), w.size);

	int const slash_pos = msg.lastIndexOf(QChar('/'));
	msg.chop(msg.size() - slash_pos);

	dataplots_t::iterator it = findOrCreatePlot(msg);
	(*it)->handleCommand(cmd, mode);
	return true;
}
 
dataplots_t::iterator Connection::findOrCreatePlot (QString const & tag)
{
	typedef SelectIterator<e_data_plot>::type iterator;
	iterator it = m_data_widgets.get<e_data_plot>().find(tag);
	if (it == m_data_widgets.get<e_data_plot>().end())
	{
		it = dataWidgetFactory<e_data_plot>(tag);
		(*it)->applyConfig();
	}
	return it;
}

