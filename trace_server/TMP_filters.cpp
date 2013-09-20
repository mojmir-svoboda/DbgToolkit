
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




	getWidgetColorRegex()->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(getWidgetColorRegex(), SIGNAL(clicked(QModelIndex)), m_server, SLOT(onClickedAtColorRegexList(QModelIndex)));
	connect(getWidgetColorRegex(), SIGNAL(doubleClicked(QModelIndex)), m_server, SLOT(onDoubleClickedAtColorRegexList(QModelIndex)));
	connect(ui->comboBoxColorRegex, SIGNAL(activated(int)), this, SLOT(onColorRegexActivate(int)));
	connect(ui->buttonAddColorRegex, SIGNAL(clicked()), this, SLOT(onColorRegexAdd()));
	connect(ui->buttonRmColorRegex, SIGNAL(clicked()), this, SLOT(onColorRegexRm()));


}


// @TODO: all the color stuff is almost duplicate, remove duplicity
void MainWindow::onColorRegexActivate (int idx)
{
	if (idx == -1) return;
	if (!getTabTrace()->currentWidget()) return;

	onColorRegexAdd();
}

void MainWindow::onColorRegexAdd ()
{
	Connection * conn = m_server->findCurrentConnection();
	if (!conn) return;

	QString qItem = ui->comboBoxColorRegex->currentText();
	if (!qItem.length())
		return;
	QStandardItem * root = static_cast<QStandardItemModel *>(getWidgetColorRegex()->model())->invisibleRootItem();
	QStandardItem * child = findChildByText(root, qItem);
	if (child == 0)
	{
		QList<QStandardItem *> row_items = addRow(qItem, false);
		root->appendRow(row_items);

		conn->appendToColorRegexFilters(qItem);
	}
	conn->recompileColorRegexps();
}

void MainWindow::onColorRegexRm ()
{
	Connection * conn = m_server->findCurrentConnection();
	QStandardItemModel * model = static_cast<QStandardItemModel *>(getWidgetColorRegex()->model());
	QModelIndex const idx = getWidgetColorRegex()->currentIndex();
	QStandardItem * item = model->itemFromIndex(idx);
	if (!item)
		return;
	QString const & val = model->data(idx, Qt::DisplayRole).toString();
	model->removeRow(idx.row());

	if (conn)
	{
		conn->removeFromColorRegexFilters(val);
		conn->recompileColorRegexps();
	}
}

void MainWindow::onGotoFileFilter () { ui->tabFilters->setCurrentIndex(0); }
void MainWindow::onGotoLevelFilter () { ui->tabFilters->setCurrentIndex(1); }
void MainWindow::onGotoColorFilter () { ui->tabFilters->setCurrentIndex(5); }
void MainWindow::onGotoRegexFilter () { ui->tabFilters->setCurrentIndex(4); }

