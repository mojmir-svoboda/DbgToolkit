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


