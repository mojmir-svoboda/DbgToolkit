#include <QLineEdit>
#include <QListView>
#include <QListWidget>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QStandardItem>
#include "types.h"
#include "utils.h"
#include "utils_qstandarditem.h"
#include "mainwindow.h"
#include "connection.h"
#include "movablelistmodel.h"
#include <tlv_parser/tlv_parser.h>


namespace logs {

	//void onSetup (E_SrcProtocol const proto, int curr_app_idx = -1, bool first_time = false, bool mac_user = false);
	//void onSetupCSV (int curr_app_idx = -1, bool first_time = false, bool mac_user = false);
	//void onSetupCSVSeparator (int curr_app_idx = -1, int column = -1, bool first_time = false);
	//void onSetupCSVColumns (int curr_app_idx, int columns, bool first_time);
	//void onSetupCSVSeparator (int curr_app_idx, bool first_time);
	//void onSettingsAppSelectedTLV (int idx, bool first_time = false);
	//void onSettingsAppSelectedCSV (int idx, int columns, bool first_time = false);
	//void settingsDisableButSeparator ();
	//void settingsToDefault ();


/*

void MainWindow::onSetupAction ()
{
	// TODO: protocol from current tab
	onSetup(e_Proto_TLV, -1);
}


void MainWindow::onSetup (E_SrcProtocol const proto, int curr_app_idx, bool first_time, bool mac_user)
{
	settingsToDefault();
	prepareSettingsWidgets(curr_app_idx, first_time);

	if (proto == e_Proto_TLV)
	{
		ui_settings->separatorComboBox->setEnabled(false);
		ui_settings->columnCountSpinBox->setEnabled(false);
		ui_settings->protocolComboBox->setCurrentIndex(ui_settings->protocolComboBox->findText("TLV"));
		onSettingsAppSelectedTLV(curr_app_idx, first_time);
	}
	else
	{
		ui_settings->protocolComboBox->setCurrentIndex(ui_settings->protocolComboBox->findText("CSV"));
		onSettingsAppSelectedCSV(curr_app_idx, first_time);
	}

	if (mac_user)
		onClickedAtSettingPooftahButton();

	m_settings_dialog->exec();

	clearSettingWidgets();
}

void MainWindow::onSetupCSVColumns (int curr_app_idx, int columns, bool first_time)
{
	prepareSettingsWidgets(curr_app_idx, first_time);
	ui_settings->separatorComboBox->setEnabled(false);
	ui_settings->columnCountSpinBox->setEnabled(true);
	ui_settings->columnCountSpinBox->setValue(columns);
	ui_settings->protocolComboBox->setCurrentIndex(ui_settings->protocolComboBox->findText("CSV"));

	// @TODO: fill widgets

	connect(ui_settings->separatorComboBox, SIGNAL(activated(int)), this, SLOT(onSeparatorSelected(int)));
	onSettingsAppSelectedCSV(curr_app_idx, columns, first_time);

	m_settings_dialog->exec();
	clearSettingWidgets();
}

void MainWindow::settingsToDefault ()
{
	ui_settings->separatorComboBox->setDuplicatesEnabled(false);
	ui_settings->protocolComboBox->setDuplicatesEnabled(false);
	ui_settings->protocolComboBox->addItem("CSV");
	ui_settings->protocolComboBox->addItem("TLV");
	ui_settings->columnCountSpinBox->setEnabled(false);

}

void MainWindow::settingsDisableButSeparator ()
{
	ui_settings->separatorComboBox->setEnabled(true);
	ui_settings->columnCountSpinBox->setEnabled(false);
}

void MainWindow::onSetupCSVSeparator (int curr_app_idx, bool first_time)
{
	settingsToDefault();
	prepareSettingsWidgets(curr_app_idx, first_time);
	settingsDisableButSeparator();

	ui_settings->protocolComboBox->setCurrentIndex(ui_settings->protocolComboBox->findText("CSV"));
	onSettingsAppSelectedCSV(curr_app_idx, first_time);

	m_settings_dialog->exec();
	clearSettingWidgets();
}


*/

	bool add (QString const & tlvname, int row, bool checked)
	{
		//insertRow(row, addRow(tlvname, checked));
		//beginInsertRows(QModelIndex(), row, row);
		//insertRow(m_data.count());
		//m_data.insert(row, tlvname);
		//endInsertRows();
		//emit layoutChanged();
		return true;
	}

	// @NOTE: hmm, that not work. me not know why. this no meat bone crush it!
	/*QVariant data (QModelIndex const & index, int role = Qt::DisplayRole) const {
		if (!index.isValid()) return QVariant();
		return QVariant(m_data[index.row()]);
	}
	int rowCount (QModelIndex const & parent) const {
		if (parent.isValid()) return 0;
		else return m_data.size();
	}
	int columnCount (QModelIndex const & parent) const { return 1; }*/

			//beginInsertRows(QModelIndex(), endRow, endRow);
			//m_data.insert(endRow, tlvname);
			//endInsertRows();

}


