#include "filter_tid.h"
#include "constants.h"
#include <serialize/serialize.h>
#include <utils/utils_qstandarditem.h>

namespace logs {

FilterTid::FilterTid (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_FilterTid)
  , m_model(0)
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

bool FilterTid::isPresent (QString const & item, bool & enabled) const
{
	if (FilteredTid const * data = findInData(item))
	{
		enabled = data->m_is_enabled;
		return true;
	}
	return false;
}
bool FilterTid::isPresent (proto::tag_tid_type item, bool & enabled) const
{
	if (FilteredTid const * data = findInData(item))
	{
		enabled = data->m_is_enabled;
		return true;
	}
	return false;
}


bool FilterTid::accept (QModelIndex const & sourceIndex)
{
	const QAbstractItemModel * model = sourceIndex.model();

	QModelIndex const idx = model->index(sourceIndex.row(), proto::tag2col<proto::int_<proto::tag_tid>>::value, QModelIndex());
	QVariant const val = model->data(idx, Qt::DisplayRole);
	proto::tag_tid_type const t = val.toULongLong();

	addToFilter(t);

	bool enabled = true;
	bool const present = isPresent(t, enabled);

	bool excluded = false;
	if (present)
	{
		//if (enabled && lvlmode == e_TidForceInclude) return true; // forced tids (errors etc)
		excluded |= !enabled;
	}
	return !excluded;
}


void FilterTid::defaultConfig ()
{
	m_data.clear();
}

void FilterTid::loadConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	if (!::loadConfigTemplate(*this, fname))
		defaultConfig();
}

void FilterTid::saveConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	::saveConfigTemplate(*this, fname);
}

void FilterTid::applyConfig ()
{
	FilterBase::applyConfig();
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

	m_ui->view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(m_ui->view, SIGNAL(clicked(QModelIndex)), this, SLOT(onClicked(QModelIndex)));
	connect(m_ui->allButton, SIGNAL(clicked()), this, SLOT(onSelectAll()));
	connect(m_ui->noneButton, SIGNAL(clicked()), this, SLOT(onSelectNone()));
}

void FilterTid::destroyModel ()
{
	if (m_ui->view->model() == m_model)
		m_ui->view->setModel(0);
	delete m_model;
	m_model = 0;
}

// bool FilterTid:: (proto::tag_tid_type item)
// {
// 	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
// 		if (m_data[i].m_tid == item)
// 		{
// 			FilteredTid & l = m_data[i];
// 			l.m_is_enabled = true;
// 			return false;
// 		}
// 	add(item);
// }

FilteredTid const * FilterTid::findInData (proto::tag_tid_type tid) const
{
	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_tid == tid)
		{
			FilteredTid const & data = m_data[i];
			return &data;
		}
	return nullptr;
}
FilteredTid const * FilterTid::findInData (QString const & tid) const
{
	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_tid_str == tid)
		{
			FilteredTid const & data = m_data[i];
			return &data;
		}
	return nullptr;
}
FilteredTid * FilterTid::findInData (QString const & tid)
{
	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_tid_str == tid)
		{
			FilteredTid & data = m_data[i];
			return &data;
		}
	return nullptr;
}

QStandardItem const * FilterTid::findInUI (proto::tag_tid_type tid) const
{
	FilteredTid const * data = findInData(tid);
	QStandardItem * root = m_model->invisibleRootItem();
	QStandardItem * child = findChildByText(root, data->m_tid_str);
	return child;
}

void FilterTid::setConfigToUI ()
{
	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
	{
		QStandardItem * root = m_model->invisibleRootItem();
		QStandardItem * child = findChildByText(root, m_data[i].m_tid_str);
		if (child == 0)
		{
			//FilteredContext & fc = m_data[i];
			QList<QStandardItem *> row_items = addRow(m_data[i].m_tid_str, true);
			row_items[0]->setCheckState(m_data[i].m_is_enabled ? Qt::Checked : Qt::Unchecked);
			root->appendRow(row_items);
		}
	}
}


void FilterTid::addToData (proto::tag_tid_type item)
{
	m_data.push_back(FilteredTid(item, true, 0));
	std::sort(m_data.begin(), m_data.end());
	m_ui->view->sortByColumn(0, Qt::AscendingOrder);
}

void FilterTid::addToFilter (proto::tag_tid_type item)
{
	FilteredTid const * data = findInData(item);
	if (!data)
	{
		addToData(item);
		m_model->clear();
		setConfigToUI();
	}
}

void FilterTid::enable (proto::tag_tid_type item, bool enabled)
{
	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_tid == item)
		{
			FilteredTid & l = m_data[i];
			l.m_is_enabled = enabled;
		}
}
void FilterTid::enable (QString const & item, bool enabled)
{
	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_tid_str == item)
			m_data[i].m_is_enabled = enabled;
}

void FilterTid::recompile ()
{
}

void FilterTid::locateItem (QString const & item, bool scrollto, bool expand)
{
	QModelIndexList indexList = m_model->match(m_model->index(0, 0), Qt::DisplayRole, item);
	if (!indexList.empty())
	{
		QModelIndex const selectedIndex(indexList.first());
		getWidget()->setCurrentIndex(selectedIndex);
	}
}

// slots
void FilterTid::onClicked (QModelIndex idx)
{
	if (!idx.isValid()) return;
	QStandardItem * item = m_model->itemFromIndex(idx);
	Q_ASSERT(item);

    QString const & filter_item = m_model->data(idx, Qt::DisplayRole).toString();
    bool const orig_checked = (item->checkState() == Qt::Checked);
    if (orig_checked)
      enable(filter_item, true);
    else
			enable(filter_item, false);

    emitFilterChangedSignal();
}

void FilterTid::onSelectAll ()
{
	auto fn = [this] (QString const & s)
	{
		enable(s, true);
	};
	applyFnOnAllChildren(fn, m_model, Qt::Checked);
	emitFilterChangedSignal();
}
void FilterTid::onSelectNone ()
{
	auto fn = [this](QString const & s)
	{
		enable(s, false);
	};
	applyFnOnAllChildren(fn, m_model, Qt::Unchecked);
	emitFilterChangedSignal();
}

}