#pragma once
#include <QComboBox>
#include <widgets/checkedcomboboxconfig.h>

inline void copyValuesToUI (QComboBox const * src, QComboBox * dst)
{
	if (src && dst)
	{
		dst->clear();

		for (int i = 0, ie = src->model()->rowCount(); i < ie; ++i)
			dst->addItem(src->itemText(i));
		dst->setCurrentIndex(src->currentIndex());
	}
}

inline void setValuesToUI (QComboBox * cb, CheckedComboBoxConfig const & cfg)
{
	if (cb)
	{
		cb->clear();

		assert(cfg.m_base.size() == cfg.m_states.size());
		for (size_t i = 0, ie = cfg.m_base.size(); i < ie; ++i)
		{
			QStandardItem * const item = new QStandardItem(cfg.m_base[i]);
			item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
			bool const on = cfg.m_states[i];
			item->setData(on ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
			QStandardItemModel * m = qobject_cast<QStandardItemModel*>(cb->model());
			m->insertRow(cb->count(), item);
		}

		for (QString const & str : cfg.m_combined_states)
		{
			QStandardItem * const item = new QStandardItem(str);
			item->setFlags(Qt::ItemIsEnabled);
			QStandardItemModel * m = qobject_cast<QStandardItemModel*>(cb->model());
			m->insertRow(cb->count(), item);
		}

		if (!cfg.m_current.isEmpty())
			cb->setCurrentText(cfg.m_current);
	}
}

inline void setUIValuesToConfig (QComboBox const * cb, CheckedComboBoxConfig & cfg)
{
	if (cb && cb->count())
	{
		//assert(cfg.m_base.size() == cfg.m_states.size());
		cfg.clear();

		QStandardItemModel * m = qobject_cast<QStandardItemModel*>(cb->model());
		for (int i = 0, ie = cb->count(); i < ie; ++i)
		{
			QString const & s = cb->itemText(i);
			QStandardItem * item = m->item(i, 0);

			if (item->flags() & Qt::ItemIsUserCheckable)
			{
				cfg.m_base.push_back(s);
				QVariant const chk = item->data(Qt::CheckStateRole);
				bool const on = chk.toInt() == Qt::Checked;
				cfg.m_states.push_back(on);
			}
			else
			{
				cfg.m_combined_states.push_back(s);
			}
			
		}
// 		for (QString const & str : cfg.m_combined_states)
// 		{
// 			QStandardItem * const item = new QStandardItem(str);
// 			item->setFlags(Qt::ItemIsEnabled);
// 			QStandardItemModel * m = qobject_cast<QStandardItemModel*>(cb->model());
// 			m->insertRow(cb->count(), item);
// 		}

		if (!cb->currentText().isEmpty())
			cfg.m_current = cb->currentText();
	}
}


