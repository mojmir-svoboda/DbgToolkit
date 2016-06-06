#include "filter_ctx.h"
#include "constants.h"
#include <serialize/serialize.h>
#include <QPainter>
#include <utils/utils_qstandarditem.h>
#include <widgets/logs/logfile_protocol.h>

namespace logs {

FilterCtx::FilterCtx (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_FilterCtx)
	, m_data()
	, m_model(0)
{
	initUI();
	setupModel();
}

FilterCtx::~FilterCtx ()
{
	destroyModel();
	doneUI();
}

void FilterCtx::initUI ()
{
	m_ui->setupUi(this);
}

void FilterCtx::doneUI ()
{
}

bool FilterCtx::accept (QModelIndex const & sourceIndex)
{
	QAbstractItemModel const * const model = sourceIndex.model();

	QModelIndex const idx = model->index(sourceIndex.row(), proto::tag2col<proto::int_<proto::tag_ctx>>::value, QModelIndex());
	QVariant const val = model->data(idx, Qt::DisplayRole);
	uint64_t const v = val.toULongLong();
	QString val_str = QString::number(v);

	bool ctx_enabled = true;
	bool const ctx_present = isCtxPresent(v, ctx_enabled);
	if (!ctx_present)
		createEntriesFor(v);
	return ctx_enabled;
}

void FilterCtx::defaultConfig ()
{
	m_data.clear();
}

void FilterCtx::onDictionaryArrived (int type, Dict const & d)
{
	//if (filterMgr()->getFilterCtx())
		//setAppData(d);
}


void FilterCtx::loadConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	if (!::loadConfigTemplate(*this, fname))
		defaultConfig();
}

void FilterCtx::saveConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	::saveConfigTemplate(*this, fname);
}

void FilterCtx::setConfigToUI ()
{
	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
	{
		QStandardItem * root = m_model->invisibleRootItem();
		QStandardItem * child = findChildByText(root, m_data[i].m_ctx_str);
		if (child == 0)
		{
			//FilteredContext & fc = m_data[i];
			QList<QStandardItem *> row_items = addRow(m_data[i].m_ctx_str, true);
			row_items[0]->setCheckState(m_data[i].m_is_enabled ? Qt::Checked : Qt::Unchecked);
			root->appendRow(row_items);
		}
	}
}

void FilterCtx::applyConfig ()
{
	FilterBase::applyConfig();
	setConfigToUI();
	m_ui->view->sortByColumn(0, Qt::AscendingOrder);
}

void FilterCtx::clear ()
{
	m_data.clear();
	// @TODO m_ctx_model.clear();
}

///////// ctx filters
void FilterCtx::setupModel ()
{
	if (!m_model)
	{
		m_model = new QStandardItemModel;
	}
	m_ui->view->setModel(m_model);
	m_ui->view->setSortingEnabled(true);
	m_ui->view->expandAll();
	CtxDelegate * d = new CtxDelegate(this);
	m_ui->view->setItemDelegate(d);

	m_ui->view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(m_ui->view, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtCtx(QModelIndex)));
	connect(m_ui->allButton, SIGNAL(clicked()), this, SLOT(onSelectAll()));
	connect(m_ui->noneButton, SIGNAL(clicked()), this, SLOT(onSelectNone()));
	m_ui->view->header()->hide();
}

void FilterCtx::destroyModel ()
{
	//if (m_ui->view->itemDelegate())
	//	m_ui->view->setItemDelegate(0);
	if (m_ui->view->model() == m_model)
		m_ui->view->setModel(0);
	delete m_model;
	m_model = 0;
}

bool FilterCtx::isCtxPresent (uint64_t ctx, bool & enabled) const
{
	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data.at(i).m_ctx == ctx)
		{
			FilteredContext const & fc = m_data.at(i);
			enabled = fc.m_is_enabled;
			return true;
		}
	return false;
}
void FilterCtx::appendCtxFilter (uint64_t ctx)
{
	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_ctx == ctx)
		{
			FilteredContext & fc = m_data[i];
			fc.m_is_enabled = true;
			return;
		}
	m_data.push_back(FilteredContext(ctx, true, 0));
	std::sort(m_data.begin(), m_data.end());
	m_ui->view->sortByColumn(0, Qt::AscendingOrder);
}
void FilterCtx::createEntriesFor (uint64_t ctx)
{
	bool enabled = false;
	if (isCtxPresent(ctx, enabled))
		return;

	QStandardItem * root = m_model->invisibleRootItem();
	QString const ctx_str = QString::number(ctx);
	QStandardItem * child = findChildByText(root, ctx_str);
	if (child == 0)
	{
		QList<QStandardItem *> row_items = addRow(ctx_str, true);
		row_items[0]->setCheckState(Qt::Checked);
		root->appendRow(row_items);
		appendCtxFilter(ctx);
	}
}
void FilterCtx::removeCtxFilter (uint64_t ctx)
{
	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_ctx == ctx)
		{
			FilteredContext & fc = m_data[i];
			fc.m_is_enabled = false;
			return;
		}
}

void FilterCtx::setAppData (AppData const * appdata)
{
	static_cast<CtxDelegate *>(m_ui->view->itemDelegate())->setAppData(appdata);
}


//////// slots
void FilterCtx::onSelectAll ()
{
	auto fn = &FilterCtx::appendCtxFilter;
/*	applyFnOnAllChildren(fn, this, m_model, Qt::Checked);*/
	emitFilterChangedSignal();
}

void FilterCtx::onSelectNone ()
{
	auto fn = &FilterCtx::removeCtxFilter;
/*	applyFnOnAllChildren(fn, this, m_model, Qt::Unchecked);*/
	emitFilterChangedSignal();
}

void FilterCtx::onClickedAtCtx (QModelIndex idx)
{
	QStandardItem * item = m_model->itemFromIndex(idx);
	Q_ASSERT(item);

	QString const & ctx = m_model->data(idx, Qt::DisplayRole).toString();
	bool const checked = (item->checkState() == Qt::Checked);
	if (checked)
		appendCtxFilter(ctx.toULongLong());
	else
		removeCtxFilter(ctx.toULongLong());

	emitFilterChangedSignal();
}


void FilterCtx::recompile ()
{ }

void FilterCtx::appendToCtxWidgets (FilteredContext const & flt)
{
	QStandardItem * root = m_model->invisibleRootItem();
	QStandardItem * child = findChildByText(root, flt.m_ctx_str);
	if (child == 0)
	{
		QList<QStandardItem *> row_items = addRow(flt.m_ctx_str, true);
		row_items[0]->setCheckState(flt.m_is_enabled ? Qt::Checked : Qt::Unchecked);
		root->appendRow(row_items);
	}
}

void FilterCtx::locateItem (QString const & item, bool scrollto, bool expand)
{
	QModelIndexList indexList = m_model->match(m_model->index(0, 0), Qt::DisplayRole, item);
	if (!indexList.empty())
	{
		QModelIndex const selectedIndex(indexList.first());
		getWidget()->setCurrentIndex(selectedIndex);
	}
}


//////// delegate
CtxDelegate::~CtxDelegate ()
{
	qDebug("%s", __FUNCTION__);
}
void CtxDelegate::paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const
{
	painter->save();
	QStyleOptionViewItemV4 option4 = option;
	initStyleOption(&option4, index);

	if (m_app_data && m_app_data->m_dict_ctx.size())
	{
		QVariant const value = index.data(Qt::DisplayRole);
		if (value.isValid() && !value.isNull())
		{
			Dict const & dict = m_app_data->m_dict_ctx;

			option4.text = dict.findNameFor(value.toULongLong());
			QWidget const * widget = option4.widget;
			if (widget)
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

}
