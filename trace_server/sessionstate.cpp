#include "SessionState.h"
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include "modelview.h"
#include <boost/tokenizer.hpp>
#include "../tlv_parser/tlv_encoder.h"

SessionState::SessionState (QObject * parent)
	: m_app_idx(-1)
	, m_tab_idx(-2)
	, m_tab_widget(0)
	, m_columns_setup(0)
	, m_columns_sizes(0)
	, m_name()
{ }

SessionState::~SessionState () { qDebug("~SessionState()"); }

void SessionState::setupColumns (QList<QString> * cs, MainWindow::columns_sizes_t * csz)
{
	m_columns_setup = cs;
	m_columns_sizes = csz;

	m_tags2columns.clear();
	for (size_t i = 0, ie = cs->size(); i < ie; ++i)
	{
		size_t const tag_idx = tlv::tag_for_name(cs->at(i).toStdString().c_str());
		if (tag_idx != tlv::tag_invalid)
		{
			m_tags2columns.insert(tag_idx, static_cast<int>(i)); // column index is int in Qt toolkit
			//qDebug("SessionState::setupColumns col[%u] tag_idx=%u tag_name=%s", i, tag_idx, cs->at(i).toStdString().c_str());
		}
	}
}

void SessionState::setupThreadColors (QList<QColor> const & tc)
{
	m_thread_colors = tc;
}

int SessionState::findColumn4Tag (tlv::tag_t tag) const
{
	QMap<tlv::tag_t, int>::const_iterator it = m_tags2columns.find(tag);
	if (it != m_tags2columns.end())
		return it.value();
	return -1;
}
void SessionState::insertColumn4Tag (tlv::tag_t tag, int column_idx)
{
	m_tags2columns.insert(tag, column_idx);
}
void SessionState::insertColumn ()
{
	m_columns_setup->push_back(QString());
	m_columns_sizes->push_back(127);
}


bool SessionState::isFileLineExcluded (fileline_t const & item)
{
	return m_file_filters.is_excluded(item.first + "/" + item.second);
}

void SessionState::appendFileFilter (fileline_t const & item)
{
	m_file_filters.append(item.first + "/" + item.second);
}

void SessionState::removeFileFilter (fileline_t const & item)
{
	m_file_filters.exclude_off(item.first + "/" + item.second);
}

