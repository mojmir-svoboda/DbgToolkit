#include "sessionstate.h"
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include "modelview.h"
#include <boost/tokenizer.hpp>
#include "../tlv_parser/tlv_encoder.h"

SessionState::SessionState (QObject * parent)
	: m_app_idx(-1)
	, m_tab_idx(-2)
	, m_tab_widget(0)
	, m_columns_setup_current(0)
	, m_columns_setup_template(0)
	, m_columns_sizes(0)
	, m_name()
{ }

SessionState::~SessionState ()
{
	qDebug("~SessionState()");
	if (m_columns_setup_current)
		delete m_columns_setup_current;
}

void SessionState::setupColumns (QList<QString> const * cs_template, MainWindow::columns_sizes_t * sizes)
{
	m_columns_sizes = sizes;
	m_columns_setup_template = cs_template;

	if (!m_columns_setup_current)
	{
		m_columns_setup_current = new QList<QString>();
	}
	else
	{
		m_columns_setup_current->clear();
	}

	m_tags2columns.clear();
	*m_columns_setup_current = *m_columns_setup_template;
	for (size_t i = 0, ie = cs_template->size(); i < ie; ++i)
	{
		size_t const tag_idx = tlv::tag_for_name(cs_template->at(i).toStdString().c_str());
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

int SessionState::findColumn4TagInTemplate (tlv::tag_t tag) const
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
	m_columns_setup_current->push_back(QString());
	if (m_columns_sizes->size() < m_columns_setup_current->size())
		m_columns_sizes->push_back(127);
	qDebug("inserting column and size. tmpl_sz=%u curr_sz=%u sizes_sz=%u", m_columns_setup_template->size(), m_columns_setup_current->size(), m_columns_sizes->size());
}

int SessionState::insertColumn (tlv::tag_t tag)
{
	insertColumn();
	int const column_index = m_columns_setup_current->size() - 1;
	char const * name = tlv::get_tag_name(tag);
	insertColumn4Tag(tag, column_index);

	if (name)
		m_columns_setup_current->operator[](column_index) = QString::fromStdString(name);
	else
		m_columns_setup_current->operator[](column_index) = QString("???");
	return column_index;
}

bool SessionState::isFileLineExcluded (fileline_t const & item) const
{
	return m_file_filters.is_excluded(item.first + "/" + item.second);
}

void SessionState::appendFileFilter (fileline_t const & item)
{
	m_file_filters.append(item.first + "/" + item.second);
}

void SessionState::appendFileFilter (std::string const & item) { m_file_filters.append(item); }

void SessionState::removeFileFilter (fileline_t const & item)
{
	m_file_filters.exclude_off(item.first + "/" + item.second);
}

void SessionState::appendTIDFilter (std::string const & item)
{
	m_tid_filters.push_back(item);
}

void SessionState::removeTIDFilter (std::string const & item)
{
	m_tid_filters.erase(std::remove(m_tid_filters.begin(), m_tid_filters.end(), item), m_tid_filters.end());
}

bool SessionState::isTIDExcluded (std::string const & item) const
{
	return std::find(m_tid_filters.begin(), m_tid_filters.end(), item) != m_tid_filters.end();
}
