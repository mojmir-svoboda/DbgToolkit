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
{
    ui->setupUi(this);
}

FilterWidget::~FilterWidget ()
{
    delete ui;
}

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
	//m_filter_state.m_colorized_texts = src.m_colorized_texts;
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


