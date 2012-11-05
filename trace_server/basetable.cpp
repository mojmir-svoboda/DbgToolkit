#include "basetable.h"
#include <QTimer>
#include "editableheaderview.h"

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
		connect(this, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));

		setHorizontalHeader(new EditableHeaderView(Qt::Horizontal, this));

		m_modelView = new TableModelView(this, m_config.m_hhdr, m_config.m_hsize);
		setModel(m_modelView);
		setConfigValues(m_config);
		QTimer::singleShot(0, this, SLOT(onApplyButton()));
	}

	BaseTable::~BaseTable ()
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		stopUpdate();
		disconnect(this, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));
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
		m_config.m_hsize.reserve(model()->columnCount());
		for (int i = 0, ie = model()->columnCount(); i < ie; ++i)
		{
			int sz = 32;
			if (i < m_config.m_hsize.size())
			{
				sz = m_config.m_hsize.at(i);
			}
			else
			{
				m_config.m_hsize.resize(i + 1);
				m_config.m_hsize[i] = sz;
			}

			horizontalHeader()->blockSignals(1);
			horizontalHeader()->resizeSection(i, sz);
			horizontalHeader()->blockSignals(0);
		}
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

	void BaseTable::onSectionResized (int idx, int /*old_size*/, int new_size)
	{
		m_config.m_hsize.reserve(idx);
		m_config.m_hsize[idx] = new_size;
	}
}

