#include "quickstringwidget.h"
#include "ui_quickstringwidget.h"
#include <utils/utils_history.h>
#include <utils/utils_widgets.h>
#include "quickstringconfig.h"
#include "mainwindow.h"
#include <QComboBox>
#include <QLineEdit>
#include <QTimer>

void QuickStringWidget::init ()
{
	m_ui->setupUi(this);

	QStyle const * const style = QApplication::style();
	connect(m_ui->findBox, SIGNAL(editTextChanged(QString)), this, SLOT(onEditTextChanged(QString)));

	QLineEdit * le = m_ui->findBox->lineEdit();
	connect(le, SIGNAL(returnPressed()), this, SLOT(onReturnPressed()));
	connect(m_ui->addButton, SIGNAL(clicked()), this, SLOT(onAdd()));
	connect(m_ui->cancelButton, SIGNAL(clicked()), this, SLOT(onCancel()));

	QStandardItemModel * m = new QStandardItemModel;
	m_ui->whereComboBox->setModel(m);
	connect(m, SIGNAL(dataChanged(QModelIndex const &, QModelIndex const &)), this, SLOT(onDataChanged(QModelIndex const &, QModelIndex const &)));

	setAutoFillBackground(true);
}

QuickStringWidget::QuickStringWidget (MainWindow * mw, QWidget * parent)
	: QWidget(parent)
	, m_ui(new Ui::QuickStringWidget)
	, m_main_window(mw)
	, m_aa(0)
{
	hide();
	init();
}

QuickStringWidget::QuickStringWidget (QWidget * parent) // widget coming from Qt creator
	: QWidget(parent)
	, m_ui(new Ui::QuickStringWidget)
	, m_aa(0)
{
	hide();
	init();
}

QuickStringWidget::~QuickStringWidget ()
{
	delete m_ui;
}

void QuickStringWidget::applyConfig (QuickStringConfig & cfg)
{
	m_config = cfg;
	setConfigValuesToUI(m_config);
}

void QuickStringWidget::applyConfig ()
{
	applyConfig(m_config);
}

void QuickStringWidget::onCancel ()
{
	QObject * o = parent();
	QWidget * w = qobject_cast<QWidget *>(o);
	w->setFocus();
	hide();
}

void QuickStringWidget::onActivate ()
{
	show();
	activateWindow();
	m_ui->findBox->setFocus();
	m_ui->findBox->lineEdit()->selectAll();
	raise();
}

void QuickStringWidget::onEditTextChanged (QString str)
{
	onResetRegexpState();
	//qDebug("find!");
}

void QuickStringWidget::onReturnPressed ()
{
	onAdd();
	//m_config.saveHistory();
}

void QuickStringWidget::onResetRegexpState ()
{
	m_ui->findBox->setStyleSheet("");
	m_ui->findBox->setToolTip("");
}

void QuickStringWidget::signalRegexpState (E_ExprState state, QString const & reason)
{
	if (state == e_ExprInvalid)
	{
		m_ui->findBox->setStyleSheet(
			"QComboBox {\
				 border: 1px solid red;\
				 border-radius: 3px;\
			}");

		m_ui->findBox->setToolTip(reason);
	}
	else
	{
		m_ui->findBox->setStyleSheet(
			"QComboBox {\
				 border: 1px solid green;\
				 border-radius: 3px;\
			}");
		m_ui->findBox->setToolTip("");
	}
}

void QuickStringWidget::makeActionQuickString (QString const & str, Action & a)
{
	a.m_type = e_QuickString;
	//a.m_src_path = path();
	//a.m_src = this;
	if (m_aa)
		a.m_dst_path = m_aa->path();
	QVariant c;
	c.setValue(m_config);
	a.m_args.push_back(c);
}

void QuickStringWidget::onAdd ()
{
	QString const str = m_ui->findBox->currentText();
	if (!str.isEmpty())
	{
		mentionStringInHistory_Ref(str, m_ui->findBox, m_config.m_history);
		m_ui->findBox->setCurrentIndex(m_ui->findBox->findText(str));
		setUIValuesToConfig(m_config);
		m_config.m_str = str;
		if (m_config.m_regexp)
		{
			m_config.m_regexp_val = QRegularExpression(m_config.m_str);
			if (m_config.m_regexp_val.isValid())
			{
				signalRegexpState(e_ExprValid, "");
			}
			else
			{
				QString const & reason = m_config.m_regexp_val.errorString();
				signalRegexpState(e_ExprInvalid, QString("regexp failed: ") + reason);
				return;
			}
		}
		Action a;
		makeActionQuickString(str, a);
		m_main_window->dockManager().handleAction(&a, e_Sync);
		QTimer::singleShot(750, this, SLOT(onResetRegexpState()));
		onCancel();
	}
}

void QuickStringWidget::clearUI ()
{
	m_ui->whereComboBox->clear();
	m_ui->findBox->clear();
}

void QuickStringWidget::setConfigValuesToUI (QuickStringConfig const & cfg)
{
	clearUI();
	syncHistoryToWidget(m_ui->findBox, cfg.m_history);
	m_ui->caseCheckBox->setChecked(cfg.m_case_sensitive);
	m_ui->wholeWordCheckBox->setChecked(cfg.m_whole_word);
	m_ui->regexCheckBox->setChecked(cfg.m_regexp);
	setValuesToUI(m_ui->whereComboBox, cfg.m_where);
}

void QuickStringWidget::setUIValuesToConfig (QuickStringConfig & cfg)
{
	cfg.m_case_sensitive = m_ui->caseCheckBox->isChecked();
	cfg.m_whole_word = m_ui->wholeWordCheckBox->isChecked();
	cfg.m_regexp = m_ui->regexCheckBox->isChecked();
	cfg.m_str = m_ui->findBox->currentText();
}

void QuickStringWidget::onDataChanged (QModelIndex const & idx, QModelIndex const & parent)
{
	QAbstractItemModel const * const m = idx.model();
	QVariant val = m->data(idx, Qt::CheckStateRole);

	bool const checked = val == Qt::Checked;
	m_config.m_where.m_states[idx.row()] = checked;
}
