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

	Ui::SetupDialogCSV *ui;
	QStringList m_data;
};

