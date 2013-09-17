#include "filter_lvl.h"
#include <QPainter>

FilterLvl::FilterLvl (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_FilterLvl)
{
	initUI();
	setupModel();
}

void FilterLvl::initUI ()
{
	m_ui->setupUi(this);
}

void FilterLvl::doneUI ()
{
	//destroyModelFile();
}

bool FilterLvl::accept (DecodedCommand const & cmd) const
{
	QString lvl;
	if (!cmd.getString(tlv::tag_lvl, lvl))
		return true;

	E_LevelMode lvlmode = e_LvlInclude;
	bool lvl_enabled = true;
	bool const lvl_present = isLvlPresent(lvl, lvl_enabled, lvlmode);

	bool excluded = false;
	if (lvl_present)
	{
		if (lvl_enabled && lvlmode == e_LvlForceInclude)
			return true; // forced levels (errors etc)
		excluded |= !lvl_enabled;
	}
	return !excluded;
}

void FilterLvl::loadConfig (QString const & path)
{
}

void FilterLvl::saveConfig (QString const & path)
{
	//QString const fsname = fname + "." + g_filterStateTag;
	//saveFilterLvl(m_filter_state, fsname.toStdString());
}

void FilterLvl::applyConfig ()
{
	//m_lvl_filters = src.m_lvl_filters;
}

void FilterLvl::clear ()
{
	m_lvl_filters.clear();
	// @TODO m_lvl_model.clear();
}


///////// lvl filters
void FilterLvl::setupModel ()
{
	if (!m_lvl_model)
		m_lvl_model = new QStandardItemModel;
	m_ui->view->setModel(m_lvl_model);
	m_ui->view->setSortingEnabled(true);

	//m_ui->view->setItemDelegate(new LevelDelegate(m_log_widget.m_app_data, this););
	m_ui->view->setRootIndex(m_lvl_model->indexFromItem(m_lvl_model->invisibleRootItem()));
}

void FilterLvl::destroyModel ()
{
	//if (m_ui->view->itemDelegate() == m_delegates.get<e_delegate_Level>())
	//	m_ui->view->setItemDelegate(0);
	if (m_ui->view->model() == m_lvl_model)
		m_ui->view->setModel(0);
	delete m_lvl_model;
	m_lvl_model = 0;
}

void FilterLvl::appendLvlFilter (QString const & item)
{
	for (int i = 0, ie = m_lvl_filters.size(); i < ie; ++i)
		if (m_lvl_filters[i].m_level_str == item)
		{
			FilteredLevel & l = m_lvl_filters[i];
			l.m_is_enabled = true;
			return;
		}
	m_lvl_filters.push_back(FilteredLevel(item, true, e_LvlInclude));
	std::sort(m_lvl_filters.begin(), m_lvl_filters.end());
}
void FilterLvl::removeLvlFilter (QString const & item)
{
	for (int i = 0, ie = m_lvl_filters.size(); i < ie; ++i)
		if (m_lvl_filters[i].m_level_str == item)
		{
			FilteredLevel & l = m_lvl_filters[i];
			l.m_is_enabled = false;
			return;
		}
}
bool FilterLvl::isLvlPresent (QString const & item, bool & enabled, E_LevelMode & lvlmode) const
{
	for (int i = 0, ie = m_lvl_filters.size(); i < ie; ++i)
		if (m_lvl_filters.at(i).m_level_str == item)
		{
			FilteredLevel const & l = m_lvl_filters.at(i);
			lvlmode = static_cast<E_LevelMode>(l.m_state);
			enabled = l.m_is_enabled;
			return true;
		}
	return false;
}
bool FilterLvl::setLvlMode (QString const & item, bool enabled, E_LevelMode lvlmode)
{
	for (int i = 0, ie = m_lvl_filters.size(); i < ie; ++i)
		if (m_lvl_filters.at(i).m_level_str == item)
		{
			FilteredLevel & l = m_lvl_filters[i];
			l.m_state = lvlmode;
			l.m_is_enabled = enabled;
			return true;
		}
	return false;

}



// @TODO: tmp, via dictionnary in future
#include <trace_client/default_levels.h>
namespace trace {
	FACT_DEFINE_ENUM_STR(E_TraceLevel,TRACELEVEL_ENUM);
	FACT_DEFINE_ENUM_TO_STRING(E_TraceLevel,TRACELEVEL_ENUM);
}

void LevelDelegate::paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const
{
    painter->save();
    QStyleOptionViewItemV4 option4 = option;
    initStyleOption(&option4, index);

	if (index.column() == 0)
	{
		QVariant value = index.data(Qt::DisplayRole);
		if (value.isValid() && !value.isNull())
		{
			QString const qs = value.toString();
			int const lvl = qs.toInt();
			// @TODO: this should be exchanged via dictionnary in future
			if (lvl >= 0 && lvl < trace::e_max_trace_level)
			{
				option4.text = QString::fromLatin1(trace::enumToString(static_cast<trace::E_TraceLevel>(lvl)));
			}

			if (QWidget const * widget = option4.widget)
			{
				QStyle * style = widget->style();
				style->drawControl(QStyle::CE_ItemViewItem, &option4, painter, widget);
			}
		}
	}
	else if (index.column() == 1)
	{
		QVariant value = index.data(Qt::DisplayRole);
		if (value.isValid() && !value.isNull())
		{
			QString const qs = value.toString();

			E_LevelMode const mode = stringToLvlMod(qs.at(0).toLatin1());
			QString const verbose = lvlmodsStr[mode];
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

