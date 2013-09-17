#include "filter_ctx.h"

FilterCtx::FilterCtx (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_FilterCtx)
{
	initUI();
	setupModel();
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
	QString ctx;
	if (!cmd.getString<QString>(tlv::tag_ctx, ctx))
		return true;

	bool ctx_enabled = true;
	bool const ctx_present = isCtxPresent(ctx, ctx_enabled);
	return ctx_enabled;
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

void FilterCtx::clear ()
{
	m_ctx_filters.clear();
	// @TODO m_ctx_model.clear();
}


///////// ctx filters
void FilterCtx::setupModel ()
{
	if (!m_ctx_model)
	{
		qDebug("new tree view ctx model");
		m_ctx_model = new QStandardItemModel;
	}
	m_main_window->getWidgetCtx()->setModel(m_ctx_model);
	m_main_window->getWidgetCtx()->expandAll();
	m_main_window->getWidgetCtx()->setItemDelegate(CtxDelegate(m_log_widget.m_app_data, this));
}

void FilterCtx::destroyModel ()
{
	if (m_ui->view->itemDelegate() == m_delegates.get<e_delegate_Ctx>())
		m_ui->view->setItemDelegate(0);
	if (m_ui->view->model() == m_ctx_model)
		m_ui->view->setModel(0);
	delete m_ctx_model;
	m_ctx_model = 0;
	delete m_delegate;
	m_delegate = 0;
}


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

void CtxDelegate::paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const
{
    painter->save();
    QStyleOptionViewItemV4 option4 = option;
    initStyleOption(&option4, index);

	if (m_app_data.getDictCtx().m_names.size())
	{
		QVariant const value = index.data(Qt::DisplayRole);
		if (value.isValid() && !value.isNull())
		{
			Dict const & dict = m_app_data.getDictCtx();

			option4.text = dict.findNameFor(value.toString());
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


