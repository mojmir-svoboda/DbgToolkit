#include "filter_ctx.h"

FilterCtx::FilterCtx (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_FilterCtx)
{
	//setupModelFile();
}

void FilterCtx::initUI ()
{
	m_ui->setupUi(this);
}

void FilterCtx::doneUI ()
{
	//destroyModelFile();
}

bool FilterCtx::accept (DecodedCommand const & cmd) const
{
	return true;
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


///////// ctx filters
bool FilterCtx::isCtxPresent (QString const & item, bool & enabled) const
{
	for (int i = 0, ie = m_ctx_filters.size(); i < ie; ++i)
		if (m_ctx_filters.at(i).m_ctx_str == item)
		{
			FilteredContext const & fc = m_ctx_filters.at(i);
			enabled = fc.m_is_enabled;
			return true;
		}
	return false;
}
void FilterCtx::appendCtxFilter (QString const & item)
{
	for (int i = 0, ie = m_ctx_filters.size(); i < ie; ++i)
		if (m_ctx_filters[i].m_ctx_str == item)
		{
			FilteredContext & fc = m_ctx_filters[i];
			fc.m_is_enabled = true;
			return;
		}
	m_ctx_filters.push_back(FilteredContext(item, true, 0));

}
void FilterCtx::removeCtxFilter (QString const & item)
{
	for (int i = 0, ie = m_ctx_filters.size(); i < ie; ++i)
		if (m_ctx_filters[i].m_ctx_str == item)
		{
			FilteredContext & fc = m_ctx_filters[i];
			fc.m_is_enabled = false;
			return;
		}
}

