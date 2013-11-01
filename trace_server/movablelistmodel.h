#pragma once
#include <QMimeData>
#include <QString>
#include <QVariant>
#include "utils_qstandarditem.h"

class MyListModel : public QStandardItemModel
{
	QList<QAbstractItemModel *> m_observers;

public:
	int m_flags;
	bool m_checkable;

	MyListModel (QObject * parent = 0 );

	void addObserver (QAbstractItemModel * o) { m_observers.append(o); }

	Qt::ItemFlags flags (QModelIndex const & index) const
	{
		if (index.isValid())
			return m_flags;
		return Qt::ItemIsDropEnabled;
	}

	bool dropMimeData (QMimeData const * mm, Qt::DropAction action, int row, int column, QModelIndex const & parent)
	{
		if (!mm->hasFormat("text/x-tlv-name"))
			return false;

		if (action == Qt::IgnoreAction)
			return true;

		if (column > 0)
			return false;

		int endRow = -1;
		if (!parent.isValid()) {
			if (row < 0)
				endRow = rowCount();
			else
				endRow = qMin(row, rowCount());
		} else
			endRow = parent.row();

		QByteArray encodedData = mm->data("text/x-tlv-name");
		QDataStream stream(&encodedData, QIODevice::ReadOnly);

		while (!stream.atEnd())
		{
			QString tlvname;
			int orig_row = -1;
			int check_state = Qt::Checked;
			stream >> tlvname >> orig_row >> check_state;

			//qDebug("drop: %s, %i -> %i", tlvname.toStdString().c_str(), orig_row, endRow);
			if (m_checkable)
			{
				insertRow(endRow, addRow(tlvname, check_state == Qt::Checked));
			}
			else
			{
				insertRow(endRow, addUncheckableRow(tlvname));
			}

			for (int i = 0, ie = m_observers.size(); i < ie; ++i)
			{
				QString txt = m_observers.at(i)->data(m_observers.at(i)->index(orig_row, 0, QModelIndex())).toString();
				m_observers.at(i)->removeRows(orig_row, 1);
				int const target_row = endRow > orig_row ? endRow - 1 : endRow;
				static_cast<QStandardItemModel *>(m_observers.at(i))->insertRow(target_row, addUncheckableRow(txt));
			}
			++endRow;
		}
		return true;
	}

	QMimeData * mimeData (QModelIndexList const & indexes) const
	{
		QMimeData * mimeData = new QMimeData();
		QByteArray encodedData;
		QDataStream stream(&encodedData, QIODevice::WriteOnly);
		foreach (QModelIndex index, indexes)
		{
			if (index.isValid())
			{
				QString tlvname = data(index, Qt::DisplayRole).toString();
				QStandardItem const * const item = itemFromIndex(index);
				stream << tlvname << index.row() << static_cast<int>(item->checkState());
				//qDebug("drag: %s, %i", tlvname.toStdString().c_str(), index.row());
			}
		}

		mimeData->setData("text/x-tlv-name", encodedData);
		return mimeData;
	}

	QStringList mimeTypes () const
	{
		QStringList types;
		types << "text/x-tlv-name";
		return types;
	}

	Qt::DropActions supportedDropActions () const
	{
		return Qt::MoveAction;
	}
};

inline MyListModel::MyListModel (QObject * parent)
	: QStandardItemModel(parent)
	, m_flags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled)
	, m_checkable(false)
{ }

