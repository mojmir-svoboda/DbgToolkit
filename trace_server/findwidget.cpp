#include "findwidget.h"
#include "ui_findwidget.h"
#include "utils_history.h"
#include "findconfig.h"
#include "mainwindow.h"
#include <QComboBox>
#include <QLineEdit>

void FindWidget::init ()
{
	m_ui->setupUi(this);

	QStyle const * const style = QApplication::style();
	m_ui->cancelButton->setIcon(style->standardIcon(QStyle::SP_DockWidgetCloseButton));
	m_ui->nextButton->setIcon(style->standardIcon(QStyle::SP_ArrowForward));
	m_ui->prevButton->setIcon(style->standardIcon(QStyle::SP_ArrowBack));
	//connect(m_ui->findBox, SIGNAL(editTextChanged(QString)), this, SLOT(onEditTextChanged(QString)));

	QLineEdit * le = m_ui->findBox->lineEdit();
	connect(le, SIGNAL(returnPressed()), this, SLOT(onReturnPressed()));
	connect(m_ui->selectButton, SIGNAL(clicked()), this, SLOT(onFindAllSelect()));
	connect(m_ui->refsButton, SIGNAL(clicked()), this, SLOT(onFindAllRefs()));
	connect(m_ui->cloneButton, SIGNAL(clicked()), this, SLOT(onFindAllClone()));
	connect(m_ui->cancelButton, SIGNAL(clicked()), this, SLOT(onCancel()));

}

FindWidget::FindWidget (MainWindow * mw, QWidget * parent)
	: QWidget(parent)
	, m_ui(new Ui::FindWidget)
	, m_main_window(mw)
	, m_dwb(0)
	, m_moving_widget(true)
{
	hide();
	init();
}

FindWidget::FindWidget (QWidget * parent) // widget coming from Qt creator
	: QWidget(parent)
	, m_ui(new Ui::FindWidget)
	, m_main_window(0)
	, m_dwb(0)
	, m_moving_widget(false)
{
	init();
}


FindWidget::~FindWidget ()
{
	delete m_ui;
}

void FindWidget::applyConfig (FindConfig & cfg)
{
	m_config = cfg;
	//applyConfig();
	setConfigValuesToUI(m_config);
}

void FindWidget::applyConfig ()
{
	applyConfig(m_config);
}

void FindWidget::onCancel ()
{
	if (isMovingFindWidget())
	{
		hide();
		setParent(m_main_window);
		move(0,0);
	}
	else
	{
	}
}

void FindWidget::onActivate ()
{
	show();
	activateWindow();
	m_ui->findBox->setFocus();
	raise();
}

void FindWidget::onEditTextChanged (QString str)
{
	//qDebug("find!");
}

void FindWidget::find ()
{
	QString str = m_ui->findBox->currentText();
	if (!str.isEmpty())
	{
		mentionStringInHistory_Ref(str, m_ui->findBox, m_config.m_history);
		Action a;
		makeActionFind(str, a);
		m_main_window->dockManager().handleAction(&a, e_Sync);
	}
}

void FindWidget::onReturnPressed ()
{
	//@TODO
	//m_config.saveHistory();
}

void FindWidget::onFocusChanged (QWidget * old, QWidget * now)
{
	//m_find_widget->onFocusChanged(old, now);
}

void FindWidget::makeActionFind (QString const & str, Action & a)
{
	a.m_type = e_Find;
	//a.m_src_path = path();
	//a.m_src = this;
	if (m_dwb)
		a.m_dst_path = m_dwb->path();
	QVariant fc;
	fc.setValue(m_config);
	a.m_args.push_back(fc);
}

void FindWidget::find (bool select, bool refs, bool clone)
{
	QString const str = m_ui->findBox->currentText();
	if (!str.isEmpty())
	{
		mentionStringInHistory_Ref(str, m_ui->findBox, m_config.m_history);
		setUIValuesToConfig(m_config);
		m_config.m_refs = refs;
		m_config.m_clone = clone;
		m_config.m_str = str;
		Action a;
		makeActionFind(str, a);
		m_main_window->dockManager().handleAction(&a, e_Sync);
	}
}

void FindWidget::onFindAllRefs ()
{
	find(0, 1, 0);
}

void FindWidget::onFindAllClone ()
{
	find(0, 0, 1);
}

void FindWidget::onFindAllSelect ()
{
	find(1, 0, 0);
}

void FindWidget::onFindNext ()
{
}

void FindWidget::onFindPrev ()
{
}

void FindWidget::setConfigValuesToUI (FindConfig const & cfg)
{
	syncHistoryToWidget(m_ui->findBox, cfg.m_history);
	m_ui->caseCheckBox->setChecked(cfg.m_case_sensitive);
	m_ui->wholeWordCheckBox->setChecked(cfg.m_whole_word);
	m_ui->regexCheckBox->setChecked(cfg.m_regexp);
}

void FindWidget::setUIValuesToConfig (FindConfig & cfg)
{
	cfg.m_case_sensitive = m_ui->caseCheckBox->isChecked();
	cfg.m_whole_word = m_ui->wholeWordCheckBox->isChecked();
	cfg.m_regexp = m_ui->regexCheckBox->isChecked();
}

