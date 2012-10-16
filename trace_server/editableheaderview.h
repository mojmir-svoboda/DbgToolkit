#pragma once
#include <QHeaderView>

class EditableHeaderView : public QHeaderView
{
	QLineEdit * m_lineEdit;
	int m_idx;
	Q_OBJECT

public:
	explicit EditableHeaderView (Qt::Orientation orientation, QWidget * parent = 0);

public slots:
	void onEditingFinished ();
	void onEditHeader (int idx);
};


