#include "filter_regex.h"
#include <QPainter>

FilterRegex::FilterRegex (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_FilterRegex)
{
	initUI();
	setupModel();
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
	for (int i = 0, ie = m_filtered_regexps.size(); i < ie; ++i)
	{
		FilteredRegex const & fr = m_filtered_regexps.at(i);
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
	if (m_filtered_regexps.size() > 0)
	{
		QString msg;
		if (!cmd.getString(tlv::tag_msg, msg))
			return true;

		for (int i = 0, ie = m_filtered_regexps.size(); i < ie; ++i)
		{
			FilteredRegex const & fr = m_filtered_regexps.at(i);
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

void FilterRegex::loadConfig (QString const & path)
{
}

void FilterRegex::saveConfig (QString const & path)
{
	//QString const fsname = fname + "." + g_filterStateTag;
	//saveFilterRegex(m_filter_state, fsname.toStdString());
}

void FilterRegex::applyConfig ()
{
	//m_filter_state.m_filtered_regexps = src.m_filtered_regexps;
}

void FilterRegex::clear ()
{
	m_filtered_regexps.clear();
	// @TODO m_regex_model.clear();
}

///////////////////

void FilterRegex::setupModel ()
{
	if (!m_regex_model)
		m_regex_model = new QStandardItemModel;
	m_ui->view->setModel(m_regex_model);
	m_ui->view->setItemDelegate(new RegexDelegate(this));
}

void FilterRegex::destroyModel ()
{
	if (m_ui->view->itemDelegate())
		m_ui->view->setItemDelegate(0);
	if (m_ui->view->model() == m_regex_model)
		m_ui->view->setModel(0);
	delete m_regex_model;
	m_regex_model = 0;
	delete m_delegate;
	m_delegate = 0;
}

bool FilterRegex::isMatchedRegexExcluded (QString str) const
{
	for (int i = 0, ie = m_filtered_regexps.size(); i < ie; ++i)
	{
		FilteredRegex const & fr = m_filtered_regexps.at(i);
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
	for (int i = 0, ie = m_filtered_regexps.size(); i < ie; ++i)
	{
		FilteredRegex & fr = m_filtered_regexps[i];
		if (fr.m_regex_str == s)
		{
			fr.m_state = state;
		}
	}
}
void FilterRegex::setRegexChecked (QString const & s, bool checked)
{
	for (int i = 0, ie = m_filtered_regexps.size(); i < ie; ++i)
	{
		FilteredRegex & fr = m_filtered_regexps[i];
		if (fr.m_regex_str == s)
		{
			fr.m_is_enabled = checked;
		}
	}
}
void FilterRegex::removeFromRegexFilters (QString const & s)
{
	for (int i = 0, ie = m_filtered_regexps.size(); i < ie; ++i)
	{
		FilteredRegex & fr = m_filtered_regexps[i];
		if (fr.m_regex_str == s)
		{
			m_filtered_regexps.removeAt(i);
			return;
		}
	}
}
void FilterRegex::appendToRegexFilters (QString const & s, bool enabled, bool state)
{
	for (int i = 0, ie = m_filtered_regexps.size(); i < ie; ++i)
		if (m_filtered_regexps[i].m_regex_str == s)
			return;
	m_filtered_regexps.push_back(FilteredRegex(s, enabled, state));
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

