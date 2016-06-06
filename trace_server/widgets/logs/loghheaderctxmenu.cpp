#include "loghheaderctxmenu.h"
#include "ui_loghheaderctxmenu.h"
#include "logwidget.h"

namespace logs {

LogHHeaderCtxMenu::LogHHeaderCtxMenu (LogWidget & log_widget, QWidget * parent)
	: QWidget(parent)
	, m_log_widget(log_widget)
	, m_ui(new Ui::LogHHeaderCtxMenu)

{
    m_ui->setupUi(this);
		init();
}

LogHHeaderCtxMenu::~LogHHeaderCtxMenu ()
{
    delete m_ui;
}

void LogHHeaderCtxMenu::onShowContextMenu (QPoint const & pt)
{
	//setConfigToUI();
	setHeaderValuesToUI();

	bool const visible = isVisible();
	if (!visible)
	{
		QPoint global_pos = mapToGlobal(pt);
		move(pt);
	}

	setVisible(!visible);
}

void LogHHeaderCtxMenu::clearUI ()
{
	clearListView(m_ui->view);
	m_ui->view->reset();
}

void LogHHeaderCtxMenu::onClickedAtColumn (QStandardItem * item)
{
	bool const checked = item->checkState() == Qt::Checked;
	QHeaderView * hdr = m_log_widget.m_tableview->horizontalHeader();
	QAbstractItemModel * model = m_log_widget.m_tableview->model();
	QStandardItemModel * uimodel = static_cast<QStandardItemModel *>(m_ui->view->model());
	int const row = item->row();
	int const log_idx = row;
	if (checked)
		hdr->showSection(log_idx);
	else
		hdr->hideSection(log_idx);
}

void LogHHeaderCtxMenu::setHeaderValuesToUI ()
{
	clearUI();
	QHeaderView * hdr = m_log_widget.m_tableview->horizontalHeader();
	QAbstractItemModel * model = m_log_widget.m_tableview->model();
	QStandardItemModel * uimodel = static_cast<QStandardItemModel *>(m_ui->view->model());
	QStandardItem * cs_root = uimodel->invisibleRootItem();

	int c0 = hdr->count();
	int c1 = hdr->hiddenSectionCount();

	for (int i = 0, ie = c0; i < ie; ++i)
	{
		int const li = hdr->logicalIndex(i);
		int const currentVisualIndex = hdr->visualIndex(i);

		bool const hidden = hdr->isSectionHidden(li);
		//csh_root->appendRow(addRow(QString(""), !hidden));

		Q_ASSERT(li > -1);
		if (li == -1)
		{
			qWarning("li == -1 for i=%i", i);
			continue;
		}

		//bool const hidden = cfg.m_columns_sizes[li] == 0;
		//bool const hidden = m_log_widget.horizontalHeader()->isSectionHidden(li);
		//cs_root->appendRow(addRow(QString(""), !hidden));
		QString const & str = model->headerData(i, Qt::Horizontal).toString();
		QList<QStandardItem *> & tmp = addRow(tr("%1").arg(str), !hidden);
		QStandardItem * tmp2 = tmp[0];
		Qt::CheckState aa = tmp2->checkState();
		
		cs_root->appendRow(tmp);
		connect(uimodel, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(onClickedAtColumn(QStandardItem *)));
	}
}

void LogHHeaderCtxMenu::init ()
{
	m_ui->view->setModel(new QStandardItemModel(this));
	m_ui->view->setEditTriggers(QAbstractItemView::NoEditTriggers);

	connect(m_ui->allButton, SIGNAL(clicked()), this, SLOT(onAllButton()));
	connect(m_ui->noneButton, SIGNAL(clicked()), this, SLOT(onNoneButton()));
	connect(m_ui->defaultButton, SIGNAL(clicked()), this, SLOT(onDefaultButton()));
	setWindowFlags(Qt::Tool);
}

void LogHHeaderCtxMenu::onAllButton ()
{
}
void LogHHeaderCtxMenu::onNoneButton ()
{
}
void LogHHeaderCtxMenu::onDefaultButton ()
{
}

void LogHHeaderCtxMenu::onClickedAtSettingColumnShow (QModelIndex const & idx)
{

}

}

