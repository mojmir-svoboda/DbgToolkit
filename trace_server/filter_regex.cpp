#include "filter_regex.h"
#include "constants.h"
#include "serialize.h"
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

FilterRegex::FilterRegex (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_FilterRegex)
	, m_data()
	, m_model(0)
	, m_delegate(0)
{
	initUI();
	setupModel();
}

FilterRegex::~FilterRegex ()
{
	destroyModel();
	doneUI();
}

void FilterRegex::initUI ()
{
	m_ui->setupUi(this);
}

void FilterRegex::doneUI ()
{
}

bool FilterRegex::accept (DecodedCommand const & cmd) const
{
	bool inclusive_filters = false;
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		FilteredRegex const & fr = m_data.at(i);
		if (!fr.m_is_enabled)
			continue;
		else
		{
			if (fr.m_state)
			{
				inclusive_filters = true;
				break;
			}
		}
	}
	if (m_data.size() > 0)
	{
		QString msg;
		if (!cmd.getString(tlv::tag_msg, msg))
			return true;

		for (int i = 0, ie = m_data.size(); i < ie; ++i)
		{
			FilteredRegex const & fr = m_data.at(i);
			if (fr.exactMatch(msg))
			{
				if (!fr.m_is_enabled)
					continue;
				else
				{
					if (fr.m_state)
						return true;
					else
						return false;
				}
			}
		}

	}
	return true;
}

void FilterRegex::defaultConfig ()
{
	m_data.clear();
}

void FilterRegex::loadConfig (QString const & path)
{
}
void FilterRegex::saveConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	::saveConfigTemplate(*this, fname);
}

void FilterRegex::applyConfig ()
{
	//m_filter_state.m_filtered_regexps = src.m_filtered_regexps;
}

void FilterRegex::clear ()
{
	m_data.clear();
	// @TODO m_regex_model.clear();
}

///////////////////

void FilterRegex::setupModel ()
{
	if (!m_model)
		m_model = new QStandardItemModel;
	m_ui->view->setModel(m_model);
	m_ui->view->setItemDelegate(new RegexDelegate(this));
}

void FilterRegex::destroyModel ()
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

bool FilterRegex::isMatchedRegexExcluded (QString str) const
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		FilteredRegex const & fr = m_data.at(i);
		if (fr.exactMatch(str))
		{
			if (!fr.m_is_enabled)
				return false;
			else
			{
				return fr.m_state ? false : true;
			}
		}
	}
	return false;
}
void FilterRegex::setRegexInclusive (QString const & s, bool state)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		FilteredRegex & fr = m_data[i];
		if (fr.m_regex_str == s)
		{
			fr.m_state = state;
		}
	}
}
void FilterRegex::setRegexChecked (QString const & s, bool checked)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		FilteredRegex & fr = m_data[i];
		if (fr.m_regex_str == s)
		{
			fr.m_is_enabled = checked;
		}
	}
}
void FilterRegex::removeFromRegexFilters (QString const & s)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		FilteredRegex & fr = m_data[i];
		if (fr.m_regex_str == s)
		{
			m_data.removeAt(i);
			return;
		}
	}
}
void FilterRegex::appendToRegexFilters (QString const & s, bool enabled, bool state)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_regex_str == s)
			return;
	m_data.push_back(FilteredRegex(s, enabled, state));
}

//////// delegate
void RegexDelegate::paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const
{
    painter->save();
    QStyleOptionViewItemV4 option4 = option;
    initStyleOption(&option4, index);

	if (index.column() == 1)
	{
		QVariant value = index.data(Qt::DisplayRole);
		if (value.isValid() && !value.isNull())
		{
			QString const qs = value.toString();

			E_FilterMode const mode = stringToFltMod(qs.at(0).toLatin1());
			QString const verbose = fltmodsStr[mode];
			option4.text = verbose;

			if (QWidget const * widget = option4.widget)
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

