#include "filterwidget.h"
#include <boost/serialization/type_info_implementation.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include "serialize/ser_qvector.h"
#include "serialize/ser_qlist.h"
#include "serialize/ser_qcolor.h"
#include "serialize/ser_qregexp.h"
#include "serialize/ser_qstring.h"
#include <fstream>
#include <sstream>
#include <QMessageBox>
#include <QString>
#include "constants.h"
#include "treeproxy.h"
#include "ui_filterwidget.h"

FilterTreeModel::FilterTreeModel (QObject * parent, tree_data_t * data)
	: TreeModel<TreeModelItem>(parent, data)
{
	qDebug("%s", __FUNCTION__);
}


QModelIndex FilterTreeModel::insertItemWithPath (QStringList const & path, bool checked)
{
	QString const name = path.join("/");
	return insertItemWithHint(name, checked);
}


FilterWidget::FilterWidget (QWidget * parent)
	: QWidget(parent)
	, ui(new Ui::FilterWidget)
	, m_filter_state()
	, m_file_model(0)
	, m_file_proxy(0)
	, m_proxy_selection(0)
	, m_ctx_model(0)
	, m_func_model(0)
	, m_tid_model(0)
	, m_color_regex_model(0)
	, m_regex_model(0)
	, m_lvl_model(0)
	, m_string_model(0)
{
    ui->setupUi(this);
	setupModelFile();
			/*this->setupModelFile();
			this->setupModelLvl();
			this->setupModelCtx();
			this->setupModelTID();
			this->setupModelColorRegex();
			this->setupModelRegex();
			this->setupModelString();*/


}

FilterWidget::~FilterWidget ()
{
	destroyModelFile();
    delete ui;
}

TreeView * FilterWidget::getWidgetFile () { return ui->treeViewFile; }
TreeView const * FilterWidget::getWidgetFile () const { return ui->treeViewFile; }
QTreeView * FilterWidget::getWidgetCtx () { return ui->treeViewCtx; }
QTreeView const * FilterWidget::getWidgetCtx () const { return ui->treeViewCtx; }
QComboBox * FilterWidget::getFilterRegex () { return ui->comboBoxRegex; }
QComboBox const * FilterWidget::getFilterRegex () const { return ui->comboBoxRegex; }
QTreeView * FilterWidget::getWidgetRegex () { return ui->treeViewRegex; }
QTreeView const * FilterWidget::getWidgetRegex () const { return ui->treeViewRegex; }
QComboBox * FilterWidget::getFilterColorRegex () { return ui->comboBoxColorRegex; }
QComboBox const * FilterWidget::getFilterColorRegex () const { return ui->comboBoxColorRegex; }
QListView * FilterWidget::getWidgetColorRegex () { return ui->listViewColorRegex; }
QListView const * FilterWidget::getWidgetColorRegex () const { return ui->listViewColorRegex; }
QListView * FilterWidget::getWidgetTID () { return ui->listViewTID; }
QListView const * FilterWidget::getWidgetTID () const { return ui->listViewTID; }
QTreeView * FilterWidget::getWidgetLvl () { return ui->treeViewLvl; }
QTreeView const * FilterWidget::getWidgetLvl () const { return ui->treeViewLvl; }
QTreeView * FilterWidget::getWidgetString () { return ui->treeViewString; }
QTreeView const * FilterWidget::getWidgetString () const { return ui->treeViewString; }



bool saveFilterState (FilterState const & s, std::string const & filename)
{
	try {
		std::ofstream ofs(filename);
		if (!ofs) return false;
		boost::archive::xml_oarchive oa(ofs);
		oa << BOOST_SERIALIZATION_NVP(s);
		ofs.close();
		return true;
	}
	catch (std::out_of_range const & e)
	{
		QMessageBox::critical(0, QString(__FUNCTION__), QString("OOR exception during decoding: %1").arg(e.what()), QMessageBox::Ok, QMessageBox::Ok);	
	}
	catch (std::length_error const & e)
	{
		QMessageBox::critical(0, QString(__FUNCTION__), QString("LE exception during decoding: %1").arg(e.what()), QMessageBox::Ok, QMessageBox::Ok);
	}
	catch (std::exception const & e)
	{
		QMessageBox::critical(0, QString(__FUNCTION__), QString("generic exception during decoding: %1").arg(e.what()), QMessageBox::Ok, QMessageBox::Ok);
	}
	catch (...)
	{
		QMessageBox::critical(0, QString(__FUNCTION__), QString("... exception during decoding"), QMessageBox::Ok, QMessageBox::Ok);
	}
	return false;
}

void FilterWidget::applyConfig (FilterState const & src, FilterState & dst)
{
	//conn->onClearCurrentFileFilter();
	//destroyModelFile();
	m_filter_state.merge_with(src.m_file_filters);
	//std::swap(conn->m_filter_state.m_file_filters.root, src.m_file_filters.root);

	//setupModelFile();
	m_filter_state.m_filtered_regexps = src.m_filtered_regexps;
	m_filter_state.m_colorized_texts = src.m_colorized_texts;
	m_filter_state.m_lvl_filters = src.m_lvl_filters;
	m_filter_state.m_ctx_filters = src.m_ctx_filters;
	//@TODO: this blows under linux, i wonder why?
	//m_filter_state.m_filtered_regexps.swap(src.m_filtered_regexps);
	//m_filter_state.m_colorized_texts.swap(src.m_colorized_texts);

	//m_filter_widget->hideLinearParents();
	//getWidgetFile()->syncExpandState();

	//conn->onInvalidateFilter();
}

void FilterWidget::loadConfig (QString const & fname, FilterState & config)
{
	FilterState dummy;
	if (loadFilterState(dummy, fname.toStdString()))
	{
		applyConfig(dummy, config);
	}
}

void FilterWidget::loadConfig (QString const & fname)
{
	FilterState dummy;
	QString const fsname = fname + "." + g_filterStateTag;
	if (loadFilterState(dummy, fsname.toStdString()))
	{
		applyConfig(dummy, m_filter_state);
	}
}

void FilterWidget::saveConfig (QString const & fname)
{
	QString const fsname = fname + "." + g_filterStateTag;
	saveFilterState(m_filter_state, fsname.toStdString());
}

void FilterWidget::setupModelFile ()
{
	if (!m_file_model)
	{
		qDebug("new tree view file model");
		m_file_model = new FilterTreeModel(this, &m_filter_state.m_file_filters);

		  //->setFilterBehavior( KSelectionProxyModel::ExactSelection );
		m_proxy_selection = new QItemSelectionModel(m_file_model, this);
		m_file_proxy = new TreeProxyModel(m_file_model, m_proxy_selection);
	}
	getWidgetFile()->setModel(m_file_model);
	getWidgetFile()->syncExpandState();
	getWidgetFile()->hideLinearParents();
	//connect(m_file_model, SIGNAL(invalidateFilter()), this, SLOT(onInvalidateFilter()));
}

void FilterWidget::destroyModelFile ()
{
	if (m_file_model)
	{
		qDebug("destroying file model");
		//disconnect(m_file_model, SIGNAL(invalidateFilter()), this, SLOT(onInvalidateFilter()));
		getWidgetFile()->unsetModel(m_file_model);
		delete m_file_model;
		m_file_model = 0;
		delete m_file_proxy;
		m_file_proxy = 0;
		delete m_proxy_selection;
		m_proxy_selection = 0;
	}
}

/*void Connection::setupModelCtx ()
{
	if (!m_ctx_model)
	{
		qDebug("new tree view ctx model");
		m_ctx_model = new QStandardItemModel;
	}
	m_main_window->getWidgetCtx()->setModel(m_ctx_model);
	m_main_window->getWidgetCtx()->expandAll();
	m_main_window->getWidgetCtx()->setItemDelegate(m_delegates.get<e_delegate_Ctx>());
}

void Connection::setupModelTID ()
{
	if (!m_tid_model)
		m_tid_model = new QStandardItemModel;
	m_main_window->getWidgetTID()->setModel(m_tid_model);
}

void Connection::setupModelColorRegex ()
{
	if (!m_color_regex_model)
		m_color_regex_model = new QStandardItemModel;
	m_main_window->getWidgetColorRegex()->setModel(m_color_regex_model);
}

void Connection::setupModelRegex ()
{
	if (!m_regex_model)
		m_regex_model = new QStandardItemModel;
	m_main_window->getWidgetRegex()->setModel(m_regex_model);
	m_main_window->getWidgetRegex()->setItemDelegate(m_delegates.get<e_delegate_Regex>());
}

void Connection::setupModelString ()
{
	if (!m_string_model)
		m_string_model = new QStandardItemModel;
	m_main_window->getWidgetString()->setModel(m_string_model);
	m_main_window->getWidgetString()->setItemDelegate(m_delegates.get<e_delegate_String>());
}

void Connection::setupModelLvl ()
{
	if (!m_lvl_model)
		m_lvl_model = new QStandardItemModel;
	m_main_window->getWidgetLvl()->setModel(m_lvl_model);
	m_main_window->getWidgetLvl()->setSortingEnabled(true);
	m_main_window->getWidgetLvl()->setItemDelegate(m_delegates.get<e_delegate_Level>());
	m_main_window->getWidgetLvl()->setRootIndex(m_lvl_model->indexFromItem(m_lvl_model->invisibleRootItem()));
}
		{
			QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetColorRegex()->model());
			QStandardItem * root = model->invisibleRootItem();
			for (int i = 0; i < sessionState().m_colorized_texts.size(); ++i)
			{
				ColorizedText & ct = sessionState().m_colorized_texts[i];
				ct.m_regex = QRegExp(ct.m_regex_str);

				QStandardItem * child = findChildByText(root, ct.m_regex_str);
				if (child == 0)
				{
					QList<QStandardItem *> row_items = addRow(ct.m_regex_str, ct.m_is_enabled);
					root->appendRow(row_items);
				}
			}
			recompileColorRegexps();
		}

		{
			QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetRegex()->model());
			QStandardItem * root = model->invisibleRootItem();
			for (int i = 0; i < sessionState().m_filtered_regexps.size(); ++i)
			{
				FilteredRegex & flt = sessionState().m_filtered_regexps[i];
				flt.m_regex = QRegExp(flt.m_regex_str);

				QStandardItem * child = findChildByText(root, flt.m_regex_str);
				if (child == 0)
				{
					Qt::CheckState const state = flt.m_is_enabled ? Qt::Checked : Qt::Unchecked;
					QList<QStandardItem *> row_items = addTriRow(flt.m_regex_str, state, static_cast<bool>(flt.m_state));
					root->appendRow(row_items);
					child = findChildByText(root, flt.m_regex_str);
					child->setCheckState(state);
				}
			}
			recompileRegexps();
		}
		{
			QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetString()->model());
			QStandardItem * root = model->invisibleRootItem();
			for (int i = 0; i < sessionState().m_filtered_strings.size(); ++i)
			{
				FilteredString & flt = sessionState().m_filtered_strings[i];
				appendToStringWidgets(flt);
			}
		}

	}

*/



