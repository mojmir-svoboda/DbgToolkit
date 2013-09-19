#include "filter_string.h"
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

FilterString::FilterString (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_FilterString)
	, m_data()
	, m_model(0)
	, m_delegate(0)
{
	initUI();
	setupModel();
}

FilterString::~FilterString ()
{
	destroyModel();
	doneUI();
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
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		FilteredString const & fr = m_data.at(i);
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
			FilteredString const & fr = m_data.at(i);
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

void FilterString::defaultConfig ()
{
	m_data.clear();
}

void FilterString::loadConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	if (!::loadConfigTemplate(*this, fname))
		defaultConfig();
}

void FilterString::saveConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	::saveConfigTemplate(*this, fname);
}

void FilterString::applyConfig ()
{
}

void FilterString::clear ()
{
	m_data.clear();
	// @TODO m_string_model.clear();
}


///////////////////
void FilterString::setupModel ()
{
	if (!m_model)
		m_model = new QStandardItemModel;
	m_ui->view->setModel(m_model);
	//m_ui->view->setItemDelegate(m_delegates.get<e_delegate_String>());
}

void FilterString::destroyModel ()
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


bool FilterString::isMatchedStringExcluded (QString str) const
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		FilteredString const & fr = m_data.at(i);
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
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		FilteredString & fr = m_data[i];
		if (fr.m_string == s)
		{
			fr.m_state = state;
		}
	}
}
void FilterString::setStringChecked (QString const & s, bool checked)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		FilteredString & fr = m_data[i];
		if (fr.m_string == s)
		{
			fr.m_is_enabled = checked;
		}
	}
}
void FilterString::removeFromStringFilters (QString const & s)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		FilteredString & fr = m_data[i];
		if (fr.m_string == s)
		{
			m_data.removeAt(i);
			return;
		}
	}
}
void FilterString::appendToStringFilters (QString const & s, bool enabled, int state)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_string == s)
			return;
	m_data.push_back(FilteredString(s, enabled, state));
}


///////// serialize
bool loadConfig (FilterString & w, QString const & fname)
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
bool saveConfig (FilterString const & w, QString const & fname)
{
	std::ofstream ofs(fname.toLatin1());
	if (!ofs) return false;
	boost::archive::xml_oarchive oa(ofs);
	oa << BOOST_SERIALIZATION_NVP(w.m_data);
	ofs.close();
	return true;
}
void fillDefaultConfig (FilterString & w)
{
}


//////// delegate
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





