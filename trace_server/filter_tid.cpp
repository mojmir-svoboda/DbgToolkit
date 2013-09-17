#include "filter_tid.h"

FilterTid::FilterTid (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_FilterTid)
{
	initUI();
	setupModel();
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
	m_tid_filters.clear();
	// @TODO m_tid_model.clear();
}


///////// tid filters
void FilterTid::setupModel ()
{
	if (!m_tid_model)
		m_tid_model = new QStandardItemModel;
	m_ui->view->setModel(m_tid_model);
}

void FilterTid::destroyModel ()
{
	if (m_ui->view->model() == m_tid_model)
		m_ui->view->setModel(0);
	delete m_tid_model;
	m_tid_model = 0;
}

void FilterTid::appendTIDFilter (QString const & item)
{
	m_tid_filters.push_back(item);
}
void FilterTid::removeTIDFilter (QString const & item)
{
	m_tid_filters.erase(std::remove(m_tid_filters.begin(), m_tid_filters.end(), item), m_tid_filters.end());
}
bool FilterTid::isTIDExcluded (QString const & item) const
{
	return std::find(m_tid_filters.begin(), m_tid_filters.end(), item) != m_tid_filters.end();
}


