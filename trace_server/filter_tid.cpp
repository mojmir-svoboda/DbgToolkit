#include "filter_tid.h"
#include "constants.h"
#include "serialize.h"
// serialization stuff
#include <boost/serialization/type_info_implementation.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <serialize/ser_qt.h>
#include <fstream>


FilterTid::FilterTid (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_FilterTid)
{
	initUI();
	setupModel();
}

FilterTid::~FilterTid ()
{
	destroyModel();
	doneUI();
}

void FilterTid::initUI ()
{
	m_ui->setupUi(this);
}

void FilterTid::doneUI ()
{
}

bool FilterTid::accept (DecodedCommand const & cmd) const
{
	QString tid;
	if (!cmd.getString(tlv::tag_tid, tid))
		return true;

	bool const excluded = isTIDExcluded(tid);
	return !excluded;
}


void FilterTid::defaultConfig ()
{
	m_data.clear();
}

void FilterTid::loadConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	if (!::loadConfigTemplate(*this, fname))
		defaultConfig();
}

void FilterTid::saveConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	::saveConfigTemplate(*this, fname);
}

void FilterTid::applyConfig ()
{
	FilterBase::applyConfig();
	//m_filter_state.merge_with(src.m_file_filters);
}

void FilterTid::clear ()
{
	m_data.clear();
	// @TODO m_tid_model.clear();
}


///////// tid filters
void FilterTid::setupModel ()
{
	if (!m_model)
		m_model = new QStandardItemModel;
	m_ui->view->setModel(m_model);

	m_ui->view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(m_ui->view, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtTIDList(QModelIndex)));
	connect(m_ui->view, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDoubleClickedAtTIDList(QModelIndex)));
}

void FilterTid::destroyModel ()
{
	if (m_ui->view->model() == m_model)
		m_ui->view->setModel(0);
	delete m_model;
	m_model = 0;
}

void FilterTid::appendTIDFilter (QString const & item)
{
	m_data.push_back(item);
}
void FilterTid::removeTIDFilter (QString const & item)
{
	m_data.erase(std::remove(m_data.begin(), m_data.end(), item), m_data.end());
}
bool FilterTid::isTIDExcluded (QString const & item) const
{
	return std::find(m_data.begin(), m_data.end(), item) != m_data.end();
}

// slots
void FilterTid::onClickedAtTIDList (QModelIndex idx)
{
	if (!idx.isValid())
		return;
	QStandardItem * item = m_model->itemFromIndex(idx);
	Q_ASSERT(item);

	bool const orig_checked = (item->checkState() == Qt::Checked);
	QString const & val = m_model->data(idx, Qt::DisplayRole).toString();
	bool const checked = !orig_checked;
	if (checked)
		appendTIDFilter(val);
	else
		removeTIDFilter(val);
	emit filterChangedSignal();
}


///////// serialize
/*bool loadConfig (FilterTid & w, QString const & fname)
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

bool saveConfig (FilterTid const & w, QString const & fname)
{
	std::ofstream ofs(fname.toLatin1());
	if (!ofs) return false;
	boost::archive::xml_oarchive oa(ofs);
	oa << BOOST_SERIALIZATION_NVP(w.m_data);
	ofs.close();
	return true;
}

void fillDefaultConfig (FilterTid & w)
{
}*/



