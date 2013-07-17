
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
	getWidgetFile()->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(getWidgetFile(), SIGNAL(clicked(QModelIndex)), m_server, SLOT(onClickedAtFileTree(QModelIndex)));
	connect(getWidgetFile(), SIGNAL(doubleClicked(QModelIndex)), m_server, SLOT(onDoubleClickedAtFileTree(QModelIndex)));
	getWidgetFile()->header()->hide();


	connect(ui->filterFileComboBox, SIGNAL(editTextChanged(QString)), this, SLOT(onFilterFileComboChanged(QString)));
	bool const cancel_on = !ui->filterFileComboBox->currentText().isEmpty();
	ui->cancelFilterButton->setEnabled(cancel_on);
	connect(ui->cancelFilterButton, SIGNAL(clicked()), this, SLOT(onCancelFilterFileButton()));
	//connect(ui->filterFileComboBox, SIGNAL(activated(int)), this, SLOT(onTimeUnitsChanged(int)));

	getWidgetCtx()->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(getWidgetCtx(), SIGNAL(clicked(QModelIndex)), m_server, SLOT(onClickedAtCtxTree(QModelIndex)));
	connect(getWidgetCtx(), SIGNAL(doubleClicked(QModelIndex)), m_server, SLOT(onDoubleClickedAtCtxTree(QModelIndex)));
	connect(ui->allCtxButton, SIGNAL(clicked()), m_server, SLOT(onSelectAllCtxs()));
	connect(ui->noCtxButton, SIGNAL(clicked()), m_server, SLOT(onSelectNoCtxs()));
	getWidgetCtx()->header()->hide();

	getWidgetTID()->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(getWidgetTID(), SIGNAL(clicked(QModelIndex)), m_server, SLOT(onClickedAtTIDList(QModelIndex)));
	connect(getWidgetTID(), SIGNAL(doubleClicked(QModelIndex)), m_server, SLOT(onDoubleClickedAtTIDList(QModelIndex)));

	getWidgetLvl()->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(getWidgetLvl(), SIGNAL(clicked(QModelIndex)), m_server, SLOT(onClickedAtLvlList(QModelIndex)));
	connect(getWidgetLvl(), SIGNAL(doubleClicked(QModelIndex)), m_server, SLOT(onDoubleClickedAtLvlList(QModelIndex)));
	connect(ui->allLevelButton, SIGNAL(clicked()), m_server, SLOT(onSelectAllLevels()));
	connect(ui->noLevelButton, SIGNAL(clicked()), m_server, SLOT(onSelectNoLevels()));
	getWidgetLvl()->header()->hide();

	getWidgetRegex()->setEditTriggers(QAbstractItemView::NoEditTriggers);
	getWidgetRegex()->header()->hide();
	connect(getWidgetRegex(), SIGNAL(clicked(QModelIndex)), m_server, SLOT(onClickedAtRegexList(QModelIndex)));
	connect(getWidgetRegex(), SIGNAL(doubleClicked(QModelIndex)), m_server, SLOT(onDoubleClickedAtRegexList(QModelIndex)));
	connect(ui->comboBoxRegex, SIGNAL(activated(int)), this, SLOT(onRegexActivate(int)));
	connect(ui->buttonAddRegex, SIGNAL(clicked()), this, SLOT(onRegexAdd()));
	connect(ui->buttonRmRegex, SIGNAL(clicked()), this, SLOT(onRegexRm()));

	getWidgetColorRegex()->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(getWidgetColorRegex(), SIGNAL(clicked(QModelIndex)), m_server, SLOT(onClickedAtColorRegexList(QModelIndex)));
	connect(getWidgetColorRegex(), SIGNAL(doubleClicked(QModelIndex)), m_server, SLOT(onDoubleClickedAtColorRegexList(QModelIndex)));
	connect(ui->comboBoxColorRegex, SIGNAL(activated(int)), this, SLOT(onColorRegexActivate(int)));
	connect(ui->buttonAddColorRegex, SIGNAL(clicked()), this, SLOT(onColorRegexAdd()));
	connect(ui->buttonRmColorRegex, SIGNAL(clicked()), this, SLOT(onColorRegexRm()));

	connect(ui->cutParentSpinBox, SIGNAL(valueChanged(int)), m_server, SLOT(onCutParentValueChanged(int)));
	connect(ui->collapseChildsButton, SIGNAL(clicked()), m_server, SLOT(onCollapseChilds()));

	connect(ui->qFilterLineEdit, SIGNAL(returnPressed()), this, SLOT(onQFilterLineEditFinished()));
	getWidgetString()->setEditTriggers(QAbstractItemView::NoEditTriggers);
	getWidgetString()->header()->hide();
	connect(getWidgetString(), SIGNAL(clicked(QModelIndex)), m_server, SLOT(onClickedAtStringList(QModelIndex)));
	connect(getWidgetString(), SIGNAL(doubleClicked(QModelIndex)), m_server, SLOT(onDoubleClickedAtStringList(QModelIndex)));
	//connect(ui->comboBoxString, SIGNAL(activated(int)), this, SLOT(onStringActivate(int)));
	connect(ui->buttonAddString, SIGNAL(clicked()), this, SLOT(onStringAdd()));
	connect(ui->buttonRmString, SIGNAL(clicked()), this, SLOT(onStringRm()));
}


void MainWindow::onRegexActivate (int idx)
{
	if (idx == -1) return;
	if (!getTabTrace()->currentWidget()) return;

	Connection * conn = m_server->findCurrentConnection();
	if (!conn) return;

	onRegexAdd();
}

void MainWindow::onRegexAdd ()
{
	Connection * conn = m_server->findCurrentConnection();
	if (!conn) return;

	QString qItem = ui->comboBoxRegex->currentText();

	conn->onRegexAdd(qItem);
/*
 *
	if (!qItem.length())
		return;
	QStandardItem * root = static_cast<QStandardItemModel *>(getWidgetRegex()->model())->invisibleRootItem();
	QStandardItem * child = findChildByText(root, qItem);
	if (child == 0)
	{
		QList<QStandardItem *> row_items = addTriRow(qItem, Qt::Unchecked, true);
		root->appendRow(row_items);
		conn->appendToRegexFilters(qItem, false, true);
		conn->recompileRegexps();
	}
	*/
}

void MainWindow::onRegexRm ()
{
	/*QStandardItemModel * model = static_cast<QStandardItemModel *>(getWidgetRegex()->model());
	QModelIndex const idx = getWidgetRegex()->currentIndex();
	QStandardItem * item = model->itemFromIndex(idx);
	if (!item)
		return;
	QString const & val = model->data(idx, Qt::DisplayRole).toString();
	model->removeRow(idx.row());*/
	Connection * conn = m_server->findCurrentConnection();
	if (conn)
	{
		conn->onRegexRm();
		//conn->removeFromRegexFilters(val);
		//conn->recompileRegexps();
	}
}

void MainWindow::onStringAdd ()
{
	Connection * conn = m_server->findCurrentConnection();
	if (!conn) return;

	QString qItem = ui->qFilterLineEdit->text();
	conn->onStringAdd(qItem);

	/*
	 *
	 *
	if (!qItem.length())
		return;
	 * QStandardItem * root = static_cast<QStandardItemModel *>(getWidgetString()->model())->invisibleRootItem();
	QStandardItem * child = findChildByText(root, qItem);
	if (child == 0)
	{
		QList<QStandardItem *> row_items = addTriRow(qItem, Qt::Checked, true);
		root->appendRow(row_items);
		conn->appendToStringFilters(qItem, true, true);
		row_items[0]->setCheckState(Qt::Checked);
		conn->recompileStrings();
	}*/
}

void MainWindow::onStringRm ()
{
	/*QStandardItemModel * model = static_cast<QStandardItemModel *>(getWidgetString()->model());
	QModelIndex const idx = getWidgetString()->currentIndex();
	QStandardItem * item = model->itemFromIndex(idx);
	if (!item)
		return;
	QString const & val = model->data(idx, Qt::DisplayRole).toString();
	model->removeRow(idx.row());*/
	Connection * conn = m_server->findCurrentConnection();
	if (conn)
	{
		conn->onStringRm();
		//conn->removeFromStringFilters(val);
		//conn->recompileStrings();
	}
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

void MainWindow::onCancelFilterFileButton ()
{
	ui->filterFileComboBox->clearEditText();
	ui->cancelFilterButton->setEnabled(false);
	ui->cancelFilterButton->setStyleSheet("color: rgb(128, 128, 128)"); 
	if (Connection * conn = m_server->findCurrentConnection())
	{
		conn->onCancelFilterFileButton();
	}
}

void MainWindow::onFilterFileComboChanged (QString str)
{
	bool cancel_on = !str.isEmpty();
	ui->cancelFilterButton->setEnabled(cancel_on);
	if (cancel_on)
		ui->cancelFilterButton->setStyleSheet("color: rgb(255, 0, 0)"); 
	else
		ui->cancelFilterButton->setStyleSheet("color: rgb(128, 128, 128)"); 

	if (Connection * conn = m_server->findCurrentConnection())
	{
		conn->onFilterFileComboChanged(str);
	}
}

void MainWindow::onGotoFileFilter () { ui->tabFilters->setCurrentIndex(0); }
void MainWindow::onGotoLevelFilter () { ui->tabFilters->setCurrentIndex(1); }
void MainWindow::onGotoColorFilter () { ui->tabFilters->setCurrentIndex(5); }
void MainWindow::onGotoRegexFilter () { ui->tabFilters->setCurrentIndex(4); }

