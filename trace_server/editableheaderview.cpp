#include "editableheaderview.h"
#include <QLineEdit>

EditableHeaderView::EditableHeaderView (Qt::Orientation orientation, QWidget * parent)
	: QHeaderView(orientation, parent)
	, m_lineEdit(0)
	, m_idx(0)
{ 
	setMovable(true);
	setClickable(true);
	m_lineEdit = new QLineEdit(viewport());
	m_lineEdit->setAlignment(Qt::AlignTop);
	m_lineEdit->setHidden(1);
	m_lineEdit->blockSignals(1);

	connect(this, SIGNAL(sectionDoubleClicked(int)), this, SLOT(onEditHeader(int)));
	connect(m_lineEdit, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
}

void EditableHeaderView::onEditingFinished ()
{
	m_lineEdit->blockSignals(1);
	m_lineEdit->setHidden(1);
	QString const old_name = model()->headerData(m_idx, Qt::Horizontal).toString();
	QString const new_name = m_lineEdit->text();
	model()->setHeaderData(m_idx, Qt::Horizontal, new_name);
	m_lineEdit->clear();
	setCurrentIndex(QModelIndex());
}

void EditableHeaderView::onEditHeader (int idx)
{
	QRect geo = m_lineEdit->geometry();
	geo.setWidth(sectionSize(idx));
	geo.moveLeft(sectionViewportPosition(idx));
	m_lineEdit->setGeometry(geo);
	m_lineEdit->setText(model()->headerData(m_idx, Qt::Horizontal).toString());
	m_lineEdit->setHidden(0);
	m_lineEdit->blockSignals(0);
	m_lineEdit->setFocus();
	m_lineEdit->selectAll();
	m_idx = idx;
}

