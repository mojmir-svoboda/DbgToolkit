#pragma once
#include <QString>
#include <QVector>
#include <QColor>

namespace logs {

	struct LogConfig
	{
		QString m_tag;
		int m_history_ln;
		int m_sync_group;
		QString m_font;
		int m_fontsize;
		int m_row_width;
		int m_indent_level;
		int m_cut_path_level;
		int m_cut_namespace_level;
		QVector<QString> m_columns_setup;		/// column setup for each registered application
		QVector<int> m_columns_sizes;		/// column sizes for each registered application
		QVector<QString> m_columns_align;		/// column align for each registered application
		QVector<QString> m_columns_elide;		/// column elide for each registered application
		QVector<QColor> m_thread_colors;				/// predefined coloring of threads
		bool m_show;
		bool m_auto_scroll;
		bool m_central_widget;
		bool m_in_view;
		bool m_filtering;
		bool m_clr_filters;
		bool m_scopes;
		bool m_indent;
		bool m_cut_path;
		bool m_cut_namespaces;
		bool m_dt_enabled;
		QString m_csv_separator;

		LogConfig ()
			: m_tag("default")
			, m_history_ln(128*128)
			, m_sync_group(0)
			, m_font("Verdana")
			, m_fontsize(10)
			, m_row_width(18)
			, m_indent_level(2)
			, m_cut_path_level(2)
			, m_cut_namespace_level(3)
			, m_show(true)
			, m_auto_scroll(false)
			, m_central_widget(true)
			, m_in_view(true)
			, m_filtering(true)
			, m_clr_filters(true)
			, m_scopes(true)
			, m_indent(true)
			, m_cut_path(true)
			, m_cut_namespaces(true)
			, m_dt_enabled(false)
			, m_csv_separator(",")
		{ }

		LogConfig (QString const & tag)
			: m_tag()
			, m_history_ln(128*128)
			, m_sync_group(0)
			, m_font("Verdana")
			, m_fontsize(10)
			, m_row_width(18)
			, m_indent_level(2)
			, m_cut_path_level(2)
			, m_cut_namespace_level(3)
			, m_show(true)
			, m_auto_scroll(false)
			, m_central_widget(tag == "default")
			, m_in_view(true)
			, m_filtering(true)
			, m_clr_filters(true)
			, m_scopes(true)
			, m_indent(true)
			, m_cut_path(true)
			, m_cut_namespaces(true)
			, m_dt_enabled(false)
		{ }

		template <class ArchiveT>
		void serialize (ArchiveT & ar, unsigned const version)
		{
			ar & boost::serialization::make_nvp("tag", m_tag);
			ar & boost::serialization::make_nvp("history_ln", m_history_ln);
			ar & boost::serialization::make_nvp("sync_group", m_sync_group);
			ar & boost::serialization::make_nvp("font", m_font);
			ar & boost::serialization::make_nvp("fontsize", m_fontsize);
			ar & boost::serialization::make_nvp("row_width", m_row_width);
			ar & boost::serialization::make_nvp("indent_level", m_indent_level);
			ar & boost::serialization::make_nvp("cut_path_level", m_cut_path_level);
			ar & boost::serialization::make_nvp("cut_namespace_level", m_cut_namespace_level);
			ar & boost::serialization::make_nvp("columns_setup", m_columns_setup);
			ar & boost::serialization::make_nvp("columns_sizes", m_columns_sizes);
			ar & boost::serialization::make_nvp("columns_align", m_columns_align);
			ar & boost::serialization::make_nvp("columns_elide", m_columns_elide);
			ar & boost::serialization::make_nvp("thread_colors", m_thread_colors);
			ar & boost::serialization::make_nvp("show", m_show);
			ar & boost::serialization::make_nvp("autoscroll", m_auto_scroll);
			ar & boost::serialization::make_nvp("central_widget", m_central_widget);
			ar & boost::serialization::make_nvp("in_view", m_in_view);
			ar & boost::serialization::make_nvp("filtering", m_filtering);
			ar & boost::serialization::make_nvp("clr_filters", m_clr_filters);
			ar & boost::serialization::make_nvp("scopes", m_scopes);
			ar & boost::serialization::make_nvp("indent", m_indent);
			ar & boost::serialization::make_nvp("cut_path", m_cut_path);
			ar & boost::serialization::make_nvp("cut_namespaces", m_cut_namespaces);
			ar & boost::serialization::make_nvp("dt_enabled", m_dt_enabled);
		}

		void clear ()
		{
			*this = LogConfig();
		}
	};

	bool loadConfig (LogConfig & config, QString const & fname);
	bool saveConfig (LogConfig const & config, QString const & fname);
	void fillDefaultConfig (LogConfig & config);
}

/*
void MainWindow::storeState ()
{
	qDebug("%s", __FUNCTION__);

	//settings.setValue("splitter", ui->splitter->saveState());
	settings.setValue("autoScrollCheckBox", ui->autoScrollCheckBox->isChecked());
	settings.setValue("inViewCheckBox", ui->inViewCheckBox->isChecked());
	settings.setValue("filterFileCheckBox", ui->filterFileCheckBox->isChecked());
	settings.setValue("clrFiltersCheckBox", ui_settings->clrFiltersCheckBox->isChecked());
	//settings.setValue("filterModeComboBox", ui->filterModeComboBox->currentIndex());
	//settings.setValue("filterPaneComboBox", ui_settings->filterPaneComboBox->currentIndex());

	settings.setValue("scopesCheckBox1", ui_settings->scopesCheckBox->isChecked());
	settings.setValue("indentCheckBox", ui_settings->indentCheckBox->isChecked());
	settings.setValue("cutPathCheckBox", ui_settings->cutPathCheckBox->isChecked());
	settings.setValue("cutNamespaceCheckBox", ui_settings->cutNamespaceCheckBox->isChecked());
	settings.setValue("indentSpinBox", ui_settings->indentSpinBox->value());
	settings.setValue("tableRowSizeSpinBox", ui_settings->tableRowSizeSpinBox->value());
	settings.setValue("tableFontComboBox", ui_settings->tableFontComboBox->currentText());
	settings.setValue("cutPathSpinBox", ui_settings->cutPathSpinBox->value());
	settings.setValue("cutNamespaceSpinBox", ui_settings->cutNamespaceSpinBox->value());
}

void MainWindow::loadState ()
{
	qDebug("%s", __FUNCTION__);
	m_config.m_app_names.clear();
	m_config.m_columns_setup.clear();
	m_config.m_columns_sizes.clear();
	m_config.loadSearchHistory();
	updateSearchHistory();

	QSettings settings("MojoMir", "TraceServer");
	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("windowState").toByteArray());
	int const pane_val = settings.value("filterPaneComboBox", 0).toInt();
	ui_settings->filterPaneComboBox->setCurrentIndex(pane_val);
	if (settings.contains("splitter"))
	{
		//ui->splitter->restoreState(settings.value("splitter").toByteArray());
		//ui->splitter->setOrientation(pane_val ? Qt::Vertical : Qt::Horizontal);
	}

	ui_settings->traceStatsCheckBox->setChecked(settings.value("trace_stats", true).toBool());

	ui->autoScrollCheckBox->setChecked(settings.value("autoScrollCheckBox", true).toBool());

	if (ui->autoScrollCheckBox->checkState() != Qt::Checked)
		ui->inViewCheckBox->setChecked(settings.value("inViewCheckBox", true).toBool());

	ui_settings->reuseTabCheckBox->setChecked(settings.value("reuseTabCheckBox", true).toBool());
	ui_settings->scopesCheckBox->setChecked(settings.value("scopesCheckBox1", true).toBool());
	ui_settings->indentCheckBox->setChecked(settings.value("indentCheckBox", true).toBool());
	ui_settings->cutPathCheckBox->setChecked(settings.value("cutPathCheckBox", true).toBool());
	ui_settings->cutNamespaceCheckBox->setChecked(settings.value("cutNamespaceCheckBox", true).toBool());

	ui_settings->indentSpinBox->setValue(settings.value("indentSpinBox", 2).toInt());
	ui_settings->cutPathSpinBox->setValue(settings.value("cutPathSpinBox", 1).toInt());
	ui_settings->cutNamespaceSpinBox->setValue(settings.value("cutNamespaceSpinBox", 1).toInt());

	ui->tableSlider->setValue(settings.value("tableSlider", 0).toInt());
	ui->plotSlider->setValue(settings.value("plotSlider", 0).toInt());
	ui->ganttSlider->setValue(settings.value("ganttSlider", 0).toInt());
	ui->filterFileCheckBox->setChecked(settings.value("filterFileCheckBox", true).toBool());
	ui->buffCheckBox->setChecked(settings.value("buffCheckBox", true).toBool());
	ui_settings->clrFiltersCheckBox->setChecked(settings.value("clrFiltersCheckBox", false).toBool());
	//ui->filterModeComboBox->setCurrentIndex(settings.value("filterModeComboBox").toInt());
	//@TODO: delete filterMode from registry if exists
	if (m_start_level == -1)
	{
		qDebug("reading saved level from cfg");
		ui->levelSpinBox->setValue(settings.value("levelSpinBox", 3).toInt());
	}
	else
	{
		qDebug("reading level from command line");
		ui->levelSpinBox->setValue(m_start_level);
	}

	ui_settings->tableRowSizeSpinBox->setValue(settings.value("tableRowSizeSpinBox", 18).toInt());
	//ui_settings->tableFontComboBox->setValue(settings.value("tableFontComboBox", "Verdana 8").toInt());

	read_list_of_strings(settings, "known-applications", "application", m_config.m_app_names);
	for (int i = 0, ie = m_config.m_app_names.size(); i < ie; ++i)
	{
		m_config.m_columns_setup.push_back(columns_setup_t());
		settings.beginGroup(tr("column_order_%1").arg(m_config.m_app_names[i]));
		{
			read_list_of_strings(settings, "orders", "column", m_config.m_columns_setup.back());
		}
		settings.endGroup();
		
		m_config.m_columns_sizes.push_back(columns_sizes_t());
		settings.beginGroup(tr("column_sizes_%1").arg(m_config.m_app_names[i]));
		{
			int const size = settings.beginReadArray("sizes");
			for (int i = 0; i < size; ++i) {
				settings.setArrayIndex(i);
				m_config.m_columns_sizes.back().push_back(settings.value("column").toInt());
			}
			settings.endArray();
		}
		settings.endGroup();

		m_config.m_columns_align.push_back(columns_align_t());
		settings.beginGroup(tr("column_align_%1").arg(m_config.m_app_names[i]));
		{
			read_list_of_strings(settings, "aligns", "column", m_config.m_columns_align.back());
		}
		settings.endGroup();

		if (m_config.m_columns_align.back().size() < m_config.m_columns_sizes.back().size())
			for (int i = 0, ie = m_config.m_columns_sizes.back().size(); i < ie; ++i)
				m_config.m_columns_align.back().push_back(QString("L"));

		m_config.m_columns_elide.push_back(columns_elide_t());
		settings.beginGroup(tr("column_elide_%1").arg(m_config.m_app_names[i]));
		{
			read_list_of_strings(settings, "elides", "column", m_config.m_columns_elide.back());
		}
		settings.endGroup();

		if (m_config.m_columns_elide.back().size() < m_config.m_columns_sizes.back().size())
			for (int i = 0, ie = m_config.m_columns_sizes.back().size(); i < ie; ++i)
				m_config.m_columns_elide.back().push_back(QString("R"));
	}

	if (m_config.m_thread_colors.empty())
	{
		for (size_t i = Qt::white; i < Qt::transparent; ++i)
			m_config.m_thread_colors.push_back(QColor(static_cast<Qt::GlobalColor>(i)));
	}

	convertBloodyBollockyBuggeryRegistry();

#ifdef WIN32
	unsigned const hotkeyCode = settings.value("hotkeyCode").toInt();
	m_config.m_hotkey = hotkeyCode ? hotkeyCode : VK_SCROLL;
	DWORD const hotkey = m_config.m_hotkey;
	int mod = 0;
	UnregisterHotKey(getHWNDForWidget(this), 0);
	RegisterHotKey(getHWNDForWidget(this), 0, mod, LOBYTE(hotkey));
#endif

	loadPresets();
	QString const pname = settings.value("presetComboBox").toString();
	ui->presetComboBox->setCurrentIndex(ui->presetComboBox->findText(pname));

	ui->dockedWidgetsToolButton->setChecked(m_docked_widgets->isVisible());
	qApp->installEventFilter(this);
}
*/

