#include "basetable.h"
#include <QTimer>

namespace table {

	BaseTable::BaseTable (QObject * oparent, QWidget * wparent, TableConfig & cfg, QString const & fname)
		: QTableView(wparent)
		, m_timer(-1)
		, m_config(cfg)
		, m_config_ui(cfg, this)
		, m_fname(fname)
		, m_modelView(0)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);

		setContextMenuPolicy(Qt::CustomContextMenu);
		connect(this, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowTableContextMenu(QPoint const &)));

		m_modelView = new TableModelView(this);
		setModel(m_modelView);
		setConfigValues(m_config);
		QTimer::singleShot(0, this, SLOT(onApplyButton()));
	}

	BaseTable::~BaseTable ()
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		stopUpdate();
		disconnect(this, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowTableContextMenu(QPoint const &)));
	}

	void BaseTable::onShow ()
	{
		show();
	}

	void BaseTable::onHide ()
	{
		hide();
	}

	void BaseTable::applyConfig (TableConfig const & pcfg)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);

		killTimer(m_timer);
		m_timer = startTimer(1000);
	}

	void BaseTable::stopUpdate ()
	{
		if (m_timer != -1)
			killTimer(m_timer);
	}

	void BaseTable::timerEvent (QTimerEvent * e)
	{
		update();
	}

	void BaseTable::update ()
	{
	}

	void BaseTable::onHideContextMenu ()
	{
		m_config_ui.onHideContextMenu();
	}

	void BaseTable::onShowContextMenu (QPoint const & pos)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		QRect widgetRect = geometry();
		m_config_ui.onShowContextMenu(QCursor::pos());
		Ui::SettingsTable * ui = m_config_ui.ui();

		setConfigValues(m_config);
		//connect(ui->applyButton, SIGNAL(clicked()), this, SLOT(onApplyButton()));
		//connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(onSaveButton()));
		//connect(ui->resetButton, SIGNAL(clicked()), this, SLOT(onResetButton()));
		//connect(ui->defaultButton, SIGNAL(clicked()), this, SLOT(onDefaultButton()));
	}

	void BaseTable::setConfigValues (TableConfig const & pcfg)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		Ui::SettingsTable * ui = m_config_ui.ui();
	}

	void BaseTable::onApplyButton ()
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		Ui::SettingsTable * ui = m_config_ui.ui();
		// ...
		applyConfig(m_config);
	}

	void BaseTable::onSaveButton () { saveConfig(m_config, m_fname); }
	void BaseTable::onResetButton () { setConfigValues(m_config); }
	void BaseTable::onDefaultButton ()
	{
		TableConfig defaults;
		//defaults.partialLoadFrom(m_config);
		setConfigValues(defaults);
	}
}

