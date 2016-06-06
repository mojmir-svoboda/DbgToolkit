#include "basetablemodel.h"
#include <QBrush>
#include <QColor>
#include <QAbstractProxyModel>
#include <sysfn/time_query.h>
#include <utils_qt/utils_tuple.h>

BaseTableModel::BaseTableModel (QObject * parent, std::vector<QString> & hhdr, std::vector<int> & hsize)
	: QAbstractTableModel(parent)
	, m_hhdr(hhdr)
	, m_hsize(hsize)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
}

BaseTableModel::~BaseTableModel ()
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
}

QVariant BaseTableModel::headerData (int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal)
	{
		switch (role)
		{
			case Qt::DisplayRole:
				if (section >= 0)
				{
					size_t const sect_idx = static_cast<size_t>(section);
					if (sect_idx < m_hhdr.size())
						return m_hhdr.at(sect_idx);
				}
				break;
			/*case Qt::SizeHintRole:
				if (section > -1 && section < m_hhdr.size())
				  return QSize(m_hsize[section], 12);
				return 128;*/
		}
	}
	return QVariant();
}

bool  BaseTableModel::setHeaderData (int s, Qt::Orientation orientation, QVariant const & value, int role)
{
	if (s == -1)
		return false;
	size_t const sect_idx = static_cast<size_t>(s);

	if (orientation == Qt::Horizontal)
	{
		if (role == Qt::EditRole)
		{
			if (sect_idx + 1 > m_hhdr.size())
			{
				m_hhdr.resize(sect_idx + 1);
			}
			m_hhdr[sect_idx] = value.toString();
			emit headerDataChanged(orientation, s, s);
		}
		else if (role == Qt::SizeHintRole)
		{
			if (sect_idx + 1 > m_hsize.size())
			{
				m_hsize.resize(sect_idx + 1);
			}
			m_hsize[sect_idx] = value.toInt();
			emit headerDataChanged(orientation, s, s);
		}
	}
	return true;
}

void BaseTableModel::emitLayoutChanged ()
{
	emit layoutChanged();
}

void BaseTableModel::clearModelData ()
{
	beginResetModel();

	removeRows(0, rowCount());
//	m_data_widgets.reset();

	endResetModel();
}

void BaseTableModel::clearModel ()
{
	beginResetModel();

//	m_data.reset();
// 	//m_hhdr.clear();
// 	//m_hsize.clear();
	removeRows(0, rowCount());
	removeColumns(0, columnCount());

	endResetModel();
}

