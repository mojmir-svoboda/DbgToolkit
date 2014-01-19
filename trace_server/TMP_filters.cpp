
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

