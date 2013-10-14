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



FilterWidget::FilterWidget (QWidget * parent)
	: QWidget(parent)
	, ui(new Ui::FilterWidget)
	, m_filter_state()
	, m_func_model(0)
	, m_color_regex_model(0)
{
    ui->setupUi(this);
}

FilterWidget::~FilterWidget ()
{
    delete ui;
}

QComboBox * FilterWidget::getFilterColorRegex () { return ui->comboBoxColorRegex; }
QComboBox const * FilterWidget::getFilterColorRegex () const { return ui->comboBoxColorRegex; }
QListView * FilterWidget::getWidgetColorRegex () { return ui->listViewColorRegex; }
QListView const * FilterWidget::getWidgetColorRegex () const { return ui->listViewColorRegex; }



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
	//std::swap(conn->m_filter_state.m_file_filters.root, src.m_file_filters.root);

	//setupModelFile();
	m_filter_state.m_colorized_texts = src.m_colorized_texts;
	//@TODO: this blows under linux, i wonder why?
	//m_filter_state.m_filtered_regexps.swap(src.m_filtered_regexps);
	//m_filter_state.m_colorized_texts.swap(src.m_colorized_texts);

	//m_filter_widget->hideLinearParents();
	//getWidgetFile()->syncExpandState();

	//conn->onInvalidateFilter();
}

void FilterWidget::loadConfig (QString const & fname, FilterState & config)
{
	/*FilterState dummy;
	if (loadFilterState(dummy, fname.toStdString()))
	{
		applyConfig(dummy, config);
	}*/
}

void FilterWidget::loadConfig (QString const & fname)
{
	/*FilterState dummy;
	QString const fsname = fname + "." + g_filterTag;
	if (loadFilterState(dummy, fsname.toStdString()))
	{
		applyConfig(dummy, m_filter_state);
	}*/
}

void FilterWidget::saveConfig (QString const & fname)
{
}

/*void Connection::setupModelColorRegex ()
{
	if (!m_color_regex_model)
		m_color_regex_model = new QStandardItemModel;
	m_main_window->getWidgetColorRegex()->setModel(m_color_regex_model);
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



