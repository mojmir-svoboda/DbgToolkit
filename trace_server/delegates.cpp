#include "delegates.h"
#include <QObject>
#include <QPainter>
#include "connection.h"
#include <logs/filterproxymodel.h>

namespace logs {

	void TableItemDelegate::paintContext (QPainter * painter, QStyleOptionViewItemV4 & option4, QModelIndex const & index) const
	{
		QVariant const value = index.data(Qt::DisplayRole);
		if (value.isValid() && !value.isNull())
		{
			Dict const & dict = m_app_data.getDictCtx();

			option4.text = dict.findNameFor(value.toString());
			QWidget const * widget = option4.widget;
			if (widget)
			{
				QStyle * style = widget->style();
				style->drawControl(QStyle::CE_ItemViewItem, &option4, painter, widget);
			}
		}
	}

	void TableItemDelegate::paintTime (QPainter * painter, QStyleOptionViewItemV4 & option4, QModelIndex const & index) const
	{
		QVariant const value = index.data(Qt::DisplayRole);
		if (value.isValid() && !value.isNull())
		{
			Connection const * conn = static_cast<Connection const *>(parent());

			unsigned long long const ref_value = m_log_widget.timeRefValue();
			QVariant value = index.data(Qt::DisplayRole);

			long long const ref_dt = value.toString().toULongLong() - ref_value;

			option4.text.clear();
			if (m_log_widget.getConfig().m_dt_enabled)
			{
				if (ref_dt >= 0)
				{
					QAbstractItemModel const * model = m_log_widget.model();
					
					tlv::tag_t const tag = tlv::tag_max_value + 1;
					// @TODO: register dynamic tag properly
					int const col_idx = const_cast<logs::LogWidget &>(m_log_widget).findColumn4Tag(tag); // ehm
					
					if (col_idx >= 0)
					{
						QVariant value;

						// @TODO: nemela by toto proxy delat sama?
						if (FilterProxyModel const * proxy = m_log_widget.logProxy())
						{
							QModelIndex const idx = proxy->index(index.row(), col_idx, QModelIndex());
							QModelIndex const curr = proxy->mapToSource(idx);
							value = model->data(curr);
						}
						else
						{
							QModelIndex const idx = model->index(index.row(), col_idx, QModelIndex());
							value = model->data(idx).toString();
						}

						if (value.isValid())
						{
							option4.text = value.toString();
						}
					}
				}
			}
			else
			{
				if (ref_dt >= 0)
					option4.text = tr("%1").arg(ref_dt);
			}

			if (value.isValid())
			{
				QString const & val = option4.text;
				float const t_ms = val.toFloat();
				float const t = t_ms / 1000.0f / conn->getMainWindow()->getTimeUnits();
				option4.text = QString::number(t, 'f', 3);
			}

			QWidget const * widget = option4.widget;
			if (widget)
			{
				QStyle * style = widget->style();
				style->drawControl(QStyle::CE_ItemViewItem, &option4, painter, widget);
			}
		}
	}

	void TableItemDelegate::paintTokenized (QPainter * painter, QStyleOptionViewItemV4 & option4, QModelIndex const & index, QString const & separator, QString const & out_separator, int level) const
	{
		QVariant value = index.data(Qt::DisplayRole);
		if (value.isValid() && !value.isNull())
		{
			Connection const * conn = static_cast<Connection const *>(parent());
			QStringList list = value.toString().split(QRegExp(separator), QString::SkipEmptyParts);
			if (level < list.size())
			{
				QString p;
				for (int i = list.size() - level, ie = list.size(); i < ie; ++i)
				{
					if (i > 0)
						p.append(out_separator);
					p.append(list.at(i));
				}
				option4.text = p;
			}

			QWidget const * widget = option4.widget;
			if (widget)
			{
				QStyle * style = widget->style();
				style->drawControl(QStyle::CE_ItemViewItem, &option4, painter, widget);
			}
		}
	}


	void TableItemDelegate::paintHilited (QPainter * painter, QStyleOptionViewItemV4 & option, QModelIndex const & index) const
	{
		if (option.showDecorationSelected && (option.state & QStyle::State_Selected))
		{
			option.font.setBold(true);
		}
		else
		{
			option.font.setBold(false);
		}
		QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &option, painter);
	}

	void TableItemDelegate::paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const
	{
		painter->save();
		QStyleOptionViewItemV4 option4 = option;
		initStyleOption(&option4, index);
	/*	columns_align_t const & column_aligns = *m_filter_state.getColumnsAlignTemplate();
		E_Align const align = stringToAlign(column_aligns[index.column()].at(0).toLatin1());
		option4.displayAlignment = static_cast<Qt::Alignment>(1 << align);
		columns_elide_t const & column_elides = *m_filter_state.getColumnsElideTemplate();
		E_Elide const elide = stringToElide(column_elides[index.column()].at(0).toLatin1());
		option4.textElideMode = static_cast<Qt::TextElideMode>(elide);

		Connection const * conn = static_cast<Connection const *>(parent());

		{	// color tagged line?
			int row = index.row();
			if (conn->isModelProxy())
				if (QAbstractProxyModel const * proxy = conn->proxyView())
				{
					QModelIndex const curr = proxy->mapToSource(index);
					row = curr.row();
				}

			if (m_log_widget.findColorTagRow(row))
			{
				painter->fillRect(option.rect, QColor(202, 225, 255));
			}	
		}

		if (conn->getMainWindow()->cutPathEnabled() && index.column() == m_log_widget.findColumn4Tag(tlv::tag_file))
		{
			int level = conn->getMainWindow()->cutPathLevel();
			paintTokenized(painter, option4, index, QString("[:/\\\\]"), "/", level);
		}
		else if (conn->getMainWindow()->cutNamespaceEnabled() && index.column() == m_log_widget.findColumn4Tag(tlv::tag_func))
		{
			int level = conn->getMainWindow()->cutNamespaceLevel();
			paintTokenized(painter, option4, index, QString("[::]"), "::", level);
		}
		else if (conn->filterState().getDictCtx().m_names.size() && index.column() == m_log_widget.findColumn4Tag(tlv::tag_ctx))
		{
			paintContext(painter, option4, index);
		}
		else if (index.column() == m_log_widget.findColumn4Tag(tlv::tag_time))
		{
			paintTime(painter, option4, index);
		}
		else*/
		{
			QStyledItemDelegate::paint(painter, option4, index);
		}
		painter->restore();
	}
}

void SyncedTableItemDelegate::paintHilited (QPainter * painter, QStyleOptionViewItemV4 & option, QModelIndex const & index) const
{
    if (option.state & QStyle::State_Selected)
	{
		option.state &= ~ QStyle::State_Selected;
		option.backgroundBrush = QBrush(QColor(244,154,193,255));
    	option.font.setBold(true);
		QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &option, painter);
    }
	else
	{
    	option.font.setBold(false);
		QStyledItemDelegate::paint(painter, option, index);
    }
}

void SyncedTableItemDelegate::paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const
{
    painter->save();
    QStyleOptionViewItemV4 option4 = option;
    initStyleOption(&option4, index);
	paintHilited(painter, option4, index);
	//QStyledItemDelegate::paint(painter, option4, index);
	painter->restore();
}


// @TODO: tmp, via dictionnary in future
#include <trace_client/default_levels.h>
namespace trace {
	FACT_DEFINE_ENUM_STR(E_TraceLevel,TRACELEVEL_ENUM);
	FACT_DEFINE_ENUM_TO_STRING(E_TraceLevel,TRACELEVEL_ENUM);
}

void LevelDelegate::paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const
{
    painter->save();
    QStyleOptionViewItemV4 option4 = option;
    initStyleOption(&option4, index);

	if (index.column() == 0)
	{
		QVariant value = index.data(Qt::DisplayRole);
		if (value.isValid() && !value.isNull())
		{
			QString const qs = value.toString();
			int const lvl = qs.toInt();
			// @TODO: this should be exchanged via dictionnary in future
			if (lvl >= 0 && lvl < trace::e_max_trace_level)
			{
				option4.text = QString::fromLatin1(trace::enumToString(static_cast<trace::E_TraceLevel>(lvl)));
			}

			if (QWidget const * widget = option4.widget)
			{
				QStyle * style = widget->style();
				style->drawControl(QStyle::CE_ItemViewItem, &option4, painter, widget);
			}
		}
	}
	else if (index.column() == 1)
	{
		QVariant value = index.data(Qt::DisplayRole);
		if (value.isValid() && !value.isNull())
		{
			QString const qs = value.toString();

			E_LevelMode const mode = stringToLvlMod(qs.at(0).toLatin1());
			QString const verbose = lvlmodsStr[mode];
			option4.text = verbose;

			if (QWidget const * widget = option4.widget)
			{
				QStyle * style = widget->style();
				style->drawControl(QStyle::CE_ItemViewItem, &option4, painter, widget);
			}
		}
	}
	else
		QStyledItemDelegate::paint(painter, option4, index);
	painter->restore();
}

void CtxDelegate::paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const
{
    painter->save();
    QStyleOptionViewItemV4 option4 = option;
    initStyleOption(&option4, index);

	if (m_app_data.getDictCtx().m_names.size())
	{
		QVariant const value = index.data(Qt::DisplayRole);
		if (value.isValid() && !value.isNull())
		{
			Dict const & dict = m_app_data.getDictCtx();

			option4.text = dict.findNameFor(value.toString());
			QWidget const * widget = option4.widget;
			if (widget)
			{
				QStyle * style = widget->style();
				style->drawControl(QStyle::CE_ItemViewItem, &option4, painter, widget);
			}
		}
	}
	else
		QStyledItemDelegate::paint(painter, option4, index);
	painter->restore();
}

QSize SizeDelegate::sizeHint (QStyleOptionViewItem const & option, QModelIndex const & index) const
{
	return QSize(128,128);
}

void StringDelegate::paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const
{
    painter->save();
    QStyleOptionViewItemV4 option4 = option;
    initStyleOption(&option4, index);

	if (index.column() == 1)
	{
		QVariant value = index.data(Qt::DisplayRole);
		if (value.isValid() && !value.isNull())
		{
			QString const qs = value.toString();

			E_FilterMode const mode = stringToFltMod(qs.at(0).toLatin1());
			QString const verbose = fltmodsStr[mode];
			option4.text = verbose;

			if (QWidget const * widget = option4.widget)
			{
				QStyle * style = widget->style();
				style->drawControl(QStyle::CE_ItemViewItem, &option4, painter, widget);
			}
		}

	}
	else
		QStyledItemDelegate::paint(painter, option4, index);
	painter->restore();
}

void RegexDelegate::paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const
{
    painter->save();
    QStyleOptionViewItemV4 option4 = option;
    initStyleOption(&option4, index);

	if (index.column() == 1)
	{
		QVariant value = index.data(Qt::DisplayRole);
		if (value.isValid() && !value.isNull())
		{
			QString const qs = value.toString();

			E_FilterMode const mode = stringToFltMod(qs.at(0).toLatin1());
			QString const verbose = fltmodsStr[mode];
			option4.text = verbose;

			if (QWidget const * widget = option4.widget)
			{
				QStyle * style = widget->style();
				style->drawControl(QStyle::CE_ItemViewItem, &option4, painter, widget);
			}
		}

	}
	else
		QStyledItemDelegate::paint(painter, option4, index);
	painter->restore();
}

