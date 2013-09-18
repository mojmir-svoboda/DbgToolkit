#include "filter_tid.h"

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

void FilterTid::loadConfig (QString const & path)
{
}

void FilterTid::saveConfig (QString const & path)
{
	//QString const fsname = fname + "." + g_filterStateTag;
	//saveFilterTid(m_filter_state, fsname.toStdString());
}

void FilterTid::applyConfig ()
{
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


