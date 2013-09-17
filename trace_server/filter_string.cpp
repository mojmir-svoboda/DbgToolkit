#include "filter_string.h"
#include <QPainter>

FilterString::FilterString (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_FilterString)
{
	initUI();
	setupModel();
}

void FilterString::initUI ()
{
	m_ui->setupUi(this);
}

void FilterString::doneUI ()
{
}

bool FilterString::accept (DecodedCommand const & cmd) const
{
	bool inclusive_filters = false;
	for (int i = 0, ie = m_filtered_strings.size(); i < ie; ++i)
	{
		FilteredString const & fr = m_filtered_strings.at(i);
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
	if (m_filtered_strings.size() > 0)
	{
		QString msg;
		if (!cmd.getString(tlv::tag_msg, msg))
			return true;

		for (int i = 0, ie = m_filtered_strings.size(); i < ie; ++i)
		{
			FilteredString const & fr = m_filtered_strings.at(i);
			if (fr.match(msg))
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

void FilterString::loadConfig (QString const & path)
{
}

void FilterString::saveConfig (QString const & path)
{
	//QString const fsname = fname + "." + g_filterStateTag;
	//saveFilterString(m_filter_state, fsname.toStdString());
}

void FilterString::applyConfig ()
{
}

void FilterString::clear ()
{
	m_filtered_strings.clear();
	// @TODO m_string_model.clear();
}


///////////////////
void FilterString::setupModel ()
{
	if (!m_string_model)
		m_string_model = new QStandardItemModel;
	m_ui->view->setModel(m_string_model);
	//m_ui->view->setItemDelegate(m_delegates.get<e_delegate_String>());
}

void FilterString::destroyModel ()
{
	if (m_ui->view->itemDelegate())
		m_ui->view->setItemDelegate(0);
	if (m_ui->view->model() == m_string_model)
		m_ui->view->setModel(0);
	delete m_string_model;
	m_string_model = 0;
	delete m_delegate;
	m_delegate = 0;
}


bool FilterString::isMatchedStringExcluded (QString str) const
{
	for (int i = 0, ie = m_filtered_strings.size(); i < ie; ++i)
	{
		FilteredString const & fr = m_filtered_strings.at(i);
		if (fr.match(str))
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
void FilterString::setStringState (QString const & s, int state)
{
	for (int i = 0, ie = m_filtered_strings.size(); i < ie; ++i)
	{
		FilteredString & fr = m_filtered_strings[i];
		if (fr.m_string == s)
		{
			fr.m_state = state;
		}
	}
}
void FilterString::setStringChecked (QString const & s, bool checked)
{
	for (int i = 0, ie = m_filtered_strings.size(); i < ie; ++i)
	{
		FilteredString & fr = m_filtered_strings[i];
		if (fr.m_string == s)
		{
			fr.m_is_enabled = checked;
		}
	}
}
void FilterString::removeFromStringFilters (QString const & s)
{
	for (int i = 0, ie = m_filtered_strings.size(); i < ie; ++i)
	{
		FilteredString & fr = m_filtered_strings[i];
		if (fr.m_string == s)
		{
			m_filtered_strings.removeAt(i);
			return;
		}
	}
}
void FilterString::appendToStringFilters (QString const & s, bool enabled, int state)
{
	for (int i = 0, ie = m_filtered_strings.size(); i < ie; ++i)
		if (m_filtered_strings[i].m_string == s)
			return;
	m_filtered_strings.push_back(FilteredString(s, enabled, state));
}


void StringDelegate::paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const
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





