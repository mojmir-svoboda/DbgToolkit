
//bool MainWindow::autoScrollEnabled () const { return ui->autoScrollCheckBox->isChecked(); }
//bool MainWindow::inViewEnabled () const { return ui->inViewCheckBox->isChecked(); }
bool MainWindow::scopesEnabled () const { return ui_settings->scopesCheckBox->isChecked(); }
bool MainWindow::indentEnabled () const { return ui_settings->indentCheckBox->isChecked(); }
int MainWindow::indentLevel () const { return ui_settings->indentSpinBox->value(); }
int MainWindow::tableRowSize () const { return ui_settings->tableRowSizeSpinBox->value(); }
QString MainWindow::tableFont () const { return ui_settings->tableFontComboBox->currentText(); }
bool MainWindow::cutPathEnabled () const { return ui_settings->cutPathCheckBox->isChecked(); }
int MainWindow::cutPathLevel () const { return ui_settings->cutPathSpinBox->value(); }
bool MainWindow::cutNamespaceEnabled () const { return ui_settings->cutNamespaceCheckBox->isChecked(); }
int MainWindow::cutNamespaceLevel () const { return ui_settings->cutNamespaceSpinBox->value(); }
bool MainWindow::clrFltEnabled () const { return ui_settings->clrFiltersCheckBox->isChecked(); }

void MainWindow::ondtToolButton ()
{
	if (Connection * conn = m_server->findCurrentConnection())
	{
		//conn->onInvalidateFilter();
	}
}

void MainWindow::onTimeUnitsChanged (int i)
{
	QString unit = ui->timeComboBox->currentText();
	qDebug("%s unit=%s", __FUNCTION__, unit.toStdString().c_str());
	if (unit == "ms")
		m_time_units = 0.001f;
	if (unit == "us")
		m_time_units = 0.000001f;
	if (unit == "s")
		m_time_units = 1.0f;
	if (unit == "m")
		m_time_units = 60.0f;
}




void MainWindow::connectFiltersToWidget ()
{

	connect(ui->filterFileComboBox, SIGNAL(editTextChanged(QString)), this, SLOT(onFilterFileComboChanged(QString)));
	bool const cancel_on = !ui->filterFileComboBox->currentText().isEmpty();
	ui->cancelFilterButton->setEnabled(cancel_on);
	connect(ui->cancelFilterButton, SIGNAL(clicked()), this, SLOT(onCancelFilterFileButton()));
	//connect(ui->filterFileComboBox, SIGNAL(activated(int)), this, SLOT(onTimeUnitsChanged(int)));
}


void MainWindow::onGotoFileFilter () { ui->tabFilters->setCurrentIndex(0); }
void MainWindow::onGotoLevelFilter () { ui->tabFilters->setCurrentIndex(1); }
void MainWindow::onGotoColorFilter () { ui->tabFilters->setCurrentIndex(5); }
void MainWindow::onGotoRegexFilter () { ui->tabFilters->setCurrentIndex(4); }

