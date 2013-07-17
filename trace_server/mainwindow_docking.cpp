#include "mainwindow.h"
#include "treemodel.h"
#include "connection.h"

QModelIndex MainWindow::addDockedWidget (QString const & name, bool on)
{
	QModelIndex const idx = m_docked_widgets_model->insertItemWithHint(name, on);
	m_docked_widgets_model->setData(idx, QVariant(on ? Qt::Checked : Qt::Unchecked), Qt::CheckStateRole);
	return idx;
}

// @TODO: hmm. this whole fn is.. unfortunately rushed. need to rethink
void MainWindow::onClickedAtDockedWidgets (QModelIndex idx)
{
	TreeModel * model = static_cast<TreeModel *>(getDockedWidgetsTreeView()->model());

	QList<QString> path;
	QList<bool> state;
	path.push_front(model->data(idx).toString());
	state.push_front(model->data(idx, Qt::CheckStateRole).toInt());
	QModelIndex parent = model->parent(idx);
	while (parent.isValid())
	{
		path.push_front(model->data(parent).toString());
		state.push_front(model->data(parent, Qt::CheckStateRole).toInt());
		parent = model->parent(parent);
	}

	//path[0]=WarHorse_App path[1]=table path[2]=pokus path[3]=--
	qDebug("path[0]=%s", path.size() > 0 ? path.at(0).toStdString().c_str() : "--");
	qDebug("path[1]=%s", path.size() > 1 ? path.at(1).toStdString().c_str() : "--");
	qDebug("path[2]=%s", path.size() > 2 ? path.at(2).toStdString().c_str() : "--");
	qDebug("path[3]=%s", path.size() > 3 ? path.at(3).toStdString().c_str() : "--");

	Q_ASSERT(path.size());

	if (Connection * conn = findConnectionByName(path.at(0)))
	{
		if (path.size() > 1)
		{
			QString class_type = path.at(1);
			if (class_type == "table")
			{
				//path.pop_front(); // drop app name
				//path.pop_front(); // drop widget identifier

				if (path.size() > 2)
				{
					for (datatables_t::iterator it = conn->m_data.get<e_data_table>().begin(), ite = conn->m_data.get<e_data_table>().end(); it != ite; ++it)
					{
						DataTable * dp = (*it);
						if (dp->m_config.m_tag == path.at(2))
						{
							bool apply = false;
							bool const xchg = dp->widget().getConfig().m_show ^ state.at(2);
							apply |= xchg;
							if (xchg)
							{
								dp->m_config.m_show = state.at(2);
							}

							if (state.at(2))
								dp->onShow();
							else
								dp->onHide();

							if (apply)
								dp->widget().applyConfig(dp->widget().getConfig());
						}
					}
				}
				else
				{
					if (state.at(1))
						conn->onShowTables();
					else
						conn->onHideTables();
				}
			}
			else
			{
				if (path.size() > 2)
				{
					for (dataplots_t::iterator it = conn->m_data.get<e_data_plot>().begin(), ite = conn->m_data.get<e_data_plot>().end(); it != ite; ++it)
					{
						DataPlot * dp = (*it);
						if (dp->m_config.m_tag == path.at(2))
						{
							bool apply = false;
							bool const xchg = dp->widget().getConfig().m_show ^ state.at(2);
							apply |= xchg;
							if (xchg)
							{
								dp->m_config.m_show = state.at(2);
							}

							if (path.size() > 3)
							{
								for (int cc = 0, cce = dp->m_config.m_ccfg.size(); cc < cce; ++cc)
								{
									plot::CurveConfig & cfg = dp->m_config.m_ccfg[cc];
									if (cfg.m_tag == path.at(3))
									{
										apply |= cfg.m_show ^ state.at(3);
										cfg.m_show = state.at(3);
										break;
									}
								}
							}
							else if (path.size() > 2)
							{
								for (int cc = 0, cce = dp->m_config.m_ccfg.size(); cc < cce; ++cc)
								{
									plot::CurveConfig & cfg = dp->m_config.m_ccfg[cc];
									apply |= cfg.m_show ^ state.at(2);
									cfg.m_show = state.at(2);
								}

								if (state.at(2))
									dp->onShow();
								else
									dp->onHide();
							}

							if (apply)
							{
								dp->widget().applyConfig(dp->widget().getConfig());
							}
						}
					}
				}
				else
				{
					if (state.at(1))
						conn->onShowPlots();
					else
						conn->onHidePlots();
				}
			}
		}

	}


}

