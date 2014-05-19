#pragma once
#include <QDialog>

namespace Ui {
class SetupDialogCSV;
}

class SetupDialogCSV : public QDialog
{
	Q_OBJECT

public:
	explicit SetupDialogCSV(QWidget *parent = 0);
	~SetupDialogCSV();

	void clear ()
	{
		m_data.clear();
		m_column_actions.clear();
	}

	Ui::SetupDialogCSV * ui;

	QStringList m_data;
	std::vector<int> 	m_column_actions;
};

