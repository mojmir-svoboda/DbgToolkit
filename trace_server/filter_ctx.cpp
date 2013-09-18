#include "filter_ctx.h"
#include <QPainter>
// serialization stuff
#include <boost/serialization/type_info_implementation.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <serialize/ser_qt.h>
#include <fstream>

FilterCtx::FilterCtx (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_FilterCtx)
	, m_data()
	, m_model(0)
	, m_delegate(0)
{
	initUI();
	setupModel();
}

FilterCtx::~FilterCtx ()
{
	destroyModel();
	doneUI();
}

void FilterCtx::initUI ()
{
	m_ui->setupUi(this);
}

void FilterCtx::doneUI ()
{
}

bool FilterCtx::accept (DecodedCommand const & cmd) const
{
	QString ctx;
	if (!cmd.getString(tlv::tag_ctx, ctx))
		return true;

	bool ctx_enabled = true;
	bool const ctx_present = isCtxPresent(ctx, ctx_enabled);
	return ctx_enabled;
}

void FilterCtx::loadConfig (QString const & path)
{
}

void FilterCtx::saveConfig (QString const & path)
{
	//QString const fsname = fname + "." + g_filterStateTag;
	//saveFilterCtx(m_filter_state, fsname.toStdString());
}

void FilterCtx::applyConfig ()
{
	//m_filter_state.merge_with(src.m_file_filters);
}

void FilterCtx::clear ()
{
	m_data.clear();
	// @TODO m_ctx_model.clear();
}


///////// ctx filters
void FilterCtx::setupModel ()
{
	if (!m_model)
	{
		qDebug("new tree view ctx model");
		m_model = new QStandardItemModel;
	}
	m_ui->view->setModel(m_model);
	m_ui->view->expandAll();
	CtxDelegate * d = new CtxDelegate(this);
	m_delegate = d;
	m_ui->view->setItemDelegate(d);
}

void FilterCtx::destroyModel ()
{
	if (m_ui->view->itemDelegate())
		m_ui->view->setItemDelegate(0);
	if (m_ui->view->model() == m_model)
		m_ui->view->setModel(0);
	delete m_model;
	m_model = 0;
	delete m_delegate;
	m_delegate = 0;
}


bool FilterCtx::isCtxPresent (QString const & item, bool & enabled) const
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data.at(i).m_ctx_str == item)
		{
			FilteredContext const & fc = m_data.at(i);
			enabled = fc.m_is_enabled;
			return true;
		}
	return false;
}
void FilterCtx::appendCtxFilter (QString const & item)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_ctx_str == item)
		{
			FilteredContext & fc = m_data[i];
			fc.m_is_enabled = true;
			return;
		}
	m_data.push_back(FilteredContext(item, true, 0));

}
void FilterCtx::removeCtxFilter (QString const & item)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_ctx_str == item)
		{
			FilteredContext & fc = m_data[i];
			fc.m_is_enabled = false;
			return;
		}
}


///////// serialize
bool loadConfig (FilterCtx & w, QString const & fname)
{
	std::ifstream ifs(fname.toLatin1());
	if (!ifs) return false;
	try {
		boost::archive::xml_iarchive ia(ifs);
		ia >> BOOST_SERIALIZATION_NVP(w.m_data);
		ifs.close();
		return true;
	}
	catch (...)
	{
		return false;
	}
}
bool saveConfig (FilterCtx const & w, QString const & fname)
{
	std::ofstream ofs(fname.toLatin1());
	if (!ofs) return false;
	boost::archive::xml_oarchive oa(ofs);
	oa << BOOST_SERIALIZATION_NVP(w.m_data);
	ofs.close();
	return true;
}
void fillDefaultConfig (FilterCtx & w)
{
}


//////// delegate
void CtxDelegate::paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const
{
    painter->save();
    QStyleOptionViewItemV4 option4 = option;
    initStyleOption(&option4, index);

	if (m_app_data && m_app_data->getDictCtx().m_names.size())
	{
		QVariant const value = index.data(Qt::DisplayRole);
		if (value.isValid() && !value.isNull())
		{
			Dict const & dict = m_app_data->getDictCtx();

			option4.text = dict.findNameFor(value.toString());
			QWidget const * widget = option4.widget;
			if (widget)
			{
				QStyle * style = widget->style();
				style->drawControl(QStyle::CE_ItemViewItem, &option4, painter, widget);
			}
		}
	}
	else
		QStyledItemDelegate::paint(painter, option4, index);
	painter->restore();
}


