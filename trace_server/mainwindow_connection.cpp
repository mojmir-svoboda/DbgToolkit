#include "mainwindow.h"
#include "connection.h"
#include <QClipboard>
#include <QStatusBar>
#include <QMessageBox>
#include <QTimer>
#include "setupdialogcsv.h"
#include "ui_setupdialogcsv.h"
#include "utils.h"

Connection * MainWindow::findConnectionByName (QString const & app_name)
{
	Q_ASSERT(parent());

	for (connections_t::const_iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
		if ((*it)->getAppName() == app_name)
			return (*it);
	return 0;
}

void MainWindow::newConnection (Connection * c)
{
	statusBar()->showMessage(tr("Incomming tcp connection!"));
}

Connection * MainWindow::createNewConnection ()
{
	static int conn_counter = 0;
	QString const tmp_name = QString("new_app_") + QString::number(++conn_counter);
	Connection * connection = new Connection(tmp_name, this);
	m_connections.push_back(connection);
	qDebug("created new connection[%u] for connection @ 0x%08x", m_connections.size(), connection);
	return connection;
}

void MainWindow::onCloseConnection (Connection * c)
{
	qDebug("MainWindow::onCloseConection(Connection *=0x%08x)", c);
	connections_t::iterator it = std::find(m_connections.begin(), m_connections.end(), c);
	if (it != m_connections.end())
	{
		Connection * connection = (*it);
		m_connections.erase(it);
		delete connection;
		connection = 0;
	}
}
void MainWindow::onCloseMarkedConnections ()
{
	qDebug("%s", __FUNCTION__);

	std::vector<Connection *> to_delete;
	to_delete.reserve(m_connections.size());

	for (connections_t::iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
	{
		if ((*it)->m_marked_for_close)
			to_delete.push_back(*it);
	}

	while (!to_delete.empty())
	{
		onCloseConnection(to_delete.back());
		to_delete.pop_back();
	}
}
void MainWindow::markConnectionForClose (Connection * conn)
{
	conn->m_marked_for_close = true;
	QTimer::singleShot(0, this, SLOT(onCloseMarkedConnections()));
}

// tcp
void MainWindow::importDataStream (QString const & fname)
{
	QFile file(fname);
	if (!file.open(QIODevice::ReadOnly))
	{
		qWarning(QString(tr("Could not open file %1").arg(fname)).toLatin1());
		return;
	}

	Connection * connection = createNewConnection();
	connection->setImportFile(fname);
	statusBar()->showMessage(tr("Importing file!"));

	QDataStream import_stream(&file);
	connection->processDataStream(import_stream);
	file.close();
}

// file
void MainWindow::createTailLogStream (QString const & fname, QString const & separator)
{
	Connection * connection = createNewConnection();
	connection->setTailFile(fname);
	QFileInfo fi(fname);
	QString const tag = fi.fileName();
	
	connection->handleCSVSetup(tag);
	datalogs_t::iterator it = connection->findOrCreateLog(tag);
	connection->m_config.m_csv_separator = separator;

	connection->processTailCSVStream();
	emit newConnection(connection);
}

void MainWindow::onChangeSeparatorDialogCSV (int n)
{
	if (n == Qt::Unchecked)
	{
		m_setup_dialog_csv->ui->separatorComboBox->setEnabled(false);
		m_setup_dialog_csv->ui->headerCheckBox->setEnabled(false);
		m_setup_dialog_csv->ui->columnList->setEnabled(false);
	}
	else
	{
		m_setup_dialog_csv->ui->separatorComboBox->setEnabled(true);
		m_setup_dialog_csv->ui->headerCheckBox->setEnabled(true);
		m_setup_dialog_csv->ui->columnList->setEnabled(true);
	}
	onChangeSetupDialogCSV(0);
}

QString getSeparator (QComboBox * box)
{
	QString separator;
	QString const sep_str = box->currentText();
	if (sep_str == QString("Comma"))          separator = ",";
	else if (sep_str == QString("Tab"))       separator = "\t";
	else if (sep_str == QString("Semicolon")) separator = ";";
	else if (sep_str == QString("Pipe"))      separator = "|";
	else
		separator = sep_str;
	return separator;
}

enum {
	e_Action_Import = 0,
	e_Action_Skip
};

inline QString unquoteString (QString const & str, bool enable_unquote, bool enable_simplify)
{
	QString const simple = enable_simplify ? str.simplified() : str;
	return enable_unquote ? unquoteString(simple) : simple;
}

void MainWindow::onChangeColumnReset ()
{
	onChangeSetupDialogCSV(0);
}

void MainWindow::onChangeColumnImport ()
{
	QStandardItemModel * p_model = static_cast<QStandardItemModel *>(m_setup_dialog_csv->ui->preView->model());
	p_model->clear();

	bool const has_sep = m_setup_dialog_csv->ui->separatorCheckBox->isChecked();
	bool const enable_unquote = m_setup_dialog_csv->ui->unquoteCheckBox->isChecked();
	bool const enable_simplify = m_setup_dialog_csv->ui->simplifyCheckBox->isChecked();
	if (has_sep)
	{
		QString separator = getSeparator(m_setup_dialog_csv->ui->separatorComboBox);

		bool const has_hdr = m_setup_dialog_csv->ui->headerCheckBox->isChecked();
		int start = 0;
		if (has_hdr)
		{
			start = 1;
			QStringList hdr_src = m_setup_dialog_csv->m_data.at(0).split(separator);
			QStringList hdr;
			for (int cl = 0, cle = hdr_src.size(); cl < cle; ++cl)
				if (e_Action_Import == m_setup_dialog_csv->m_column_actions[cl])
					hdr << unquoteString(hdr_src.at(cl), enable_unquote, enable_simplify);

			p_model->setHorizontalHeaderLabels(hdr);

			for (int cl = 0, cle = hdr.size(); cl < cle; ++cl)
				m_setup_dialog_csv->ui->preView->horizontalHeader()->setSectionResizeMode(cl, QHeaderView::ResizeToContents);
		}

		for (int i = start, ie = m_setup_dialog_csv->m_data.size(); i < ie; ++i)
		{
			QStringList di = m_setup_dialog_csv->m_data.at(i).split(separator);
			QList<QStandardItem *> l;
			for (int cl = 0, cle = di.size(); cl < cle; ++cl)
				if (e_Action_Import == m_setup_dialog_csv->m_column_actions[cl])
					l << new QStandardItem(unquoteString(di.at(cl), enable_unquote, enable_simplify));
			p_model->appendRow(l);
		}
	}
	else
	{
		for (int i = 0, ie = m_setup_dialog_csv->m_data.size(); i < ie; ++i)
			p_model->appendRow(new QStandardItem(unquoteString(m_setup_dialog_csv->m_data.at(i), enable_unquote, enable_simplify)));
	}

}

void MainWindow::onChangeSetupDialogCSV (int n)
{
	QStandardItemModel * p_model = static_cast<QStandardItemModel *>(m_setup_dialog_csv->ui->preView->model());
	QStandardItemModel * c_model = static_cast<QStandardItemModel *>(m_setup_dialog_csv->ui->columnList->model());
	p_model->clear();
	c_model->clear();
	m_setup_dialog_csv->ui->actionGroupBox->setEnabled(false);
	m_setup_dialog_csv->m_column_actions.clear();

	bool const has_sep = m_setup_dialog_csv->ui->separatorCheckBox->isChecked();
	bool const enable_unquote = m_setup_dialog_csv->ui->unquoteCheckBox->isChecked();
	bool const enable_simplify = m_setup_dialog_csv->ui->simplifyCheckBox->isChecked();
	if (has_sep)
	{
		QString separator = getSeparator(m_setup_dialog_csv->ui->separatorComboBox);

		bool const has_hdr = m_setup_dialog_csv->ui->headerCheckBox->isChecked();
		int start = 0;
		if (has_hdr)
		{
			start = 1;
			QStringList hdr_src = m_setup_dialog_csv->m_data.at(0).split(separator);
			QStringList hdr;
			for (int cl = 0, cle = hdr_src.size(); cl < cle; ++cl)
				hdr << unquoteString(hdr_src.at(cl), enable_unquote, enable_simplify);

			p_model->setHorizontalHeaderLabels(hdr);

			for (int cl = 0, cle = hdr.size(); cl < cle; ++cl)
			{
				m_setup_dialog_csv->ui->preView->horizontalHeader()->setSectionResizeMode(cl, QHeaderView::ResizeToContents);
				c_model->appendRow(new QStandardItem(unquoteString(hdr.at(cl), enable_unquote, enable_simplify)));
				m_setup_dialog_csv->m_column_actions.push_back(e_Action_Import);
			}
		}

		for (int i = start, ie = m_setup_dialog_csv->m_data.size(); i < ie; ++i)
		{
			QStringList di = m_setup_dialog_csv->m_data.at(i).split(separator);
			QList<QStandardItem *> l;
			for (int col = 0, cole = di.size(); col < cole; ++col)
				l << new QStandardItem(unquoteString(di.at(col), enable_unquote, enable_simplify));
			p_model->appendRow(l);

		}

		if (!has_hdr)
		{
			for (int cl = 0, cle = p_model->columnCount(); cl < cle; ++cl)
			{
				QString const val = p_model->headerData(cl, Qt::Horizontal).toString();
				c_model->appendRow(new QStandardItem(val));
				m_setup_dialog_csv->m_column_actions.push_back(e_Action_Import);
			}
		}
	}
	else
	{
		for (int i = 0, ie = m_setup_dialog_csv->m_data.size(); i < ie; ++i)
			p_model->appendRow(new QStandardItem(unquoteString(m_setup_dialog_csv->m_data.at(i), enable_unquote, enable_simplify)));
	}
}

void MainWindow::onChangeColumnRadioDialogCSV (bool toggled)
{
	QStandardItemModel * c_model = static_cast<QStandardItemModel *>(m_setup_dialog_csv->ui->columnList->model());
	QModelIndex const idx = m_setup_dialog_csv->ui->columnList->currentIndex();
	if (idx.isValid())
	{
		int const row = idx.row();
		bool const on = m_setup_dialog_csv->ui->importButton->isChecked();
		m_setup_dialog_csv->m_column_actions[row] = (on) ? e_Action_Import : e_Action_Skip;

		onChangeColumnImport();
	}
}

void MainWindow::onChangeColumnDialogCSV (QModelIndex const & idx)
{
	if (idx.isValid())
	{
		m_setup_dialog_csv->ui->actionGroupBox->setEnabled(true);
		QStandardItemModel * c_model = static_cast<QStandardItemModel *>(m_setup_dialog_csv->ui->columnList->model());
		int const row = idx.row();
		int const action = m_setup_dialog_csv->m_column_actions.at(row);
		m_setup_dialog_csv->ui->importButton->blockSignals(true);
		m_setup_dialog_csv->ui->skipButton->blockSignals(true);
		if (action == e_Action_Import)
			m_setup_dialog_csv->ui->importButton->setChecked(true);
		if (action == e_Action_Skip)
			m_setup_dialog_csv->ui->skipButton->setChecked(true);
		m_setup_dialog_csv->ui->importButton->blockSignals(false);
		m_setup_dialog_csv->ui->skipButton->blockSignals(false);
	}
}

void MainWindow::createTailDataStream (QString const & fname)
{
	// the csv dialogue
	m_setup_dialog_csv->clear();

	QFile * f = new QFile(fname);
	if (!f->open(QIODevice::ReadOnly))
	{
		QMessageBox::critical(0, tr("Error"), tr("Could not open file\n%1").arg(fname));
		delete f;
		return;
	}
	QTextStream * file_csv_stream = new QTextStream(f);

	enum { e_min_lines = 3 };
	QStringList lines;
	int n_lines = 0;
	while (!file_csv_stream->atEnd() && n_lines < e_min_lines)
	{
		lines << file_csv_stream->readLine(2048);
		++n_lines;
	}
	delete file_csv_stream;

	m_setup_dialog_csv->m_data = lines;
	connect(m_setup_dialog_csv->ui->simplifyCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onChangeSetupDialogCSV(int)));
	connect(m_setup_dialog_csv->ui->unquoteCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onChangeSetupDialogCSV(int)));
	connect(m_setup_dialog_csv->ui->separatorCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onChangeSeparatorDialogCSV(int)));
	connect(m_setup_dialog_csv->ui->separatorComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onChangeSetupDialogCSV(int)));
	connect(m_setup_dialog_csv->ui->separatorComboBox->lineEdit(), SIGNAL(returnPressed()), this, SLOT(onChangeSetupDialogCSV(int)));
	connect(m_setup_dialog_csv->ui->headerCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onChangeSetupDialogCSV(int)));
	connect(m_setup_dialog_csv->ui->columnList, SIGNAL(clicked(QModelIndex const &)), this, SLOT(onChangeColumnDialogCSV(QModelIndex const &)));
	connect(m_setup_dialog_csv->ui->importButton, SIGNAL(toggled(bool)), this, SLOT(onChangeColumnRadioDialogCSV(bool)));
	connect(m_setup_dialog_csv->ui->resetButton, SIGNAL(clicked()), this, SLOT(onChangeColumnReset()));

	onChangeSetupDialogCSV(0);
	m_setup_dialog_csv->exec();

	disconnect(m_setup_dialog_csv->ui->simplifyCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onChangeSetupDialogCSV(int)));
	disconnect(m_setup_dialog_csv->ui->unquoteCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onChangeSetupDialogCSV(int)));
	disconnect(m_setup_dialog_csv->ui->separatorCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onChangeSeparatorDialogCSV(int)));
	disconnect(m_setup_dialog_csv->ui->separatorComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onChangeSetupDialogCSV(int)));
	disconnect(m_setup_dialog_csv->ui->separatorComboBox->lineEdit(), SIGNAL(returnPressed()), this, SLOT(onChangeSetupDialogCSV(int)));
	disconnect(m_setup_dialog_csv->ui->headerCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onChangeSetupDialogCSV(int)));
	disconnect(m_setup_dialog_csv->ui->columnList, SIGNAL(clicked(QModelIndex const &)), this, SLOT(onChangeColumnDialogCSV(QModelIndex const &)));
	disconnect(m_setup_dialog_csv->ui->importButton, SIGNAL(toggled(bool)), this, SLOT(onChangeColumnRadioDialogCSV(bool)));
	disconnect(m_setup_dialog_csv->ui->skipButton, SIGNAL(toggled(bool)), this, SLOT(onChangeColumnRadioDialogCSV(bool)));
	disconnect(m_setup_dialog_csv->ui->resetButton, SIGNAL(clicked()), this, SLOT(onChangeColumnReset()));

	if (m_setup_dialog_csv->result() != QDialog::Accepted)
		return;

	// everything ok, setup connection and widget
	Connection * connection = createNewConnection();
	connection->setTailFile(fname);
	QFileInfo fi(fname);
	QString const tag = fi.fileName();

	datalogs_t::iterator it = connection->findOrCreateLog(tag);
	connection->handleCSVSetup(tag);
	QString const separator = getSeparator(m_setup_dialog_csv->ui->separatorComboBox);
	connection->m_config.m_csv_separator = separator;

	if (0 == (*it)->config().m_columns_setup.size())
	{
		QStandardItemModel * p_model = static_cast<QStandardItemModel *>(m_setup_dialog_csv->ui->preView->model());
		for (int cl = 0, cle = p_model->columnCount(); cl < cle; ++cl)
		{
			if (e_Action_Import == m_setup_dialog_csv->m_column_actions[cl])
			{
				QString const val = p_model->headerData(cl, Qt::Horizontal).toString();
				int const size = m_setup_dialog_csv->ui->preView->horizontalHeader()->sectionSize(cl);
				E_Align const align = e_AlignLeft;
				E_Elide const elide = e_ElideRight;
				(*it)->config().m_columns_setup.push_back(val);
				(*it)->config().m_columns_sizes.push_back(size);
				(*it)->config().m_columns_align.push_back(QString(alignToString(align)));
				(*it)->config().m_columns_elide.push_back(QString(elideToString(elide)));
			}
		}
	}

	connection->processTailCSVStream();
	emit newConnection(connection);
}


