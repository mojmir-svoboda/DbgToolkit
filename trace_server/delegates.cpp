#include "delegates.h"
#include <QObject>
#include <QPainter>
#include "connection.h"

void TableItemDelegate::paintContext (QPainter * painter, QStyleOptionViewItemV4 & option4, QModelIndex const & index) const
{
	QVariant const value = index.data(Qt::DisplayRole);
	if (value.isValid() && !value.isNull())
	{
		Dict const & dict = m_session_state.getDictCtx();

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

		unsigned long long const ref_value = m_session_state.timeRefValue();
		QVariant value = index.data(Qt::DisplayRole);

		long long const ref_dt = value.toString().toULongLong() - ref_value;

		option4.text.clear();
		if (conn->getMainWindow()->dtEnabled())
		{
			if (ref_dt >= 0)
			{
				QAbstractItemModel const * model = conn->modelView();
				
				tlv::tag_t const tag = tlv::tag_max_value + 1;
				int const col_idx = m_session_state.findColumn4Tag(tag);
				
				if (col_idx >= 0)
				{
					QVariant value;
					if (QAbstractProxyModel const * proxy = conn->proxyView())
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

void TableItemDelegate::paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const
{
    painter->save();
    QStyleOptionViewItemV4 option4 = option;
    initStyleOption(&option4, index);
	columns_align_t const & column_aligns = *m_session_state.getColumnsAlignTemplate();
	E_Align const align = stringToAlign(column_aligns[index.column()].at(0).toLatin1());
	option4.displayAlignment = static_cast<Qt::Alignment>(1 << align);
	columns_elide_t const & column_elides = *m_session_state.getColumnsElideTemplate();
	E_Elide const elide = stringToElide(column_elides[index.column()].at(0).toLatin1());
	option4.textElideMode = static_cast<Qt::TextElideMode>(elide);

	Connection const * conn = static_cast<Connection const *>(parent());
	if (conn->getMainWindow()->cutPathEnabled() && index.column() == m_session_state.findColumn4Tag(tlv::tag_file))
	{
		int level = conn->getMainWindow()->cutPathLevel();
		paintTokenized(painter, option4, index, QString("[:/\\\\]"), "/", level);
	}
	else if (conn->getMainWindow()->cutNamespaceEnabled() && index.column() == m_session_state.findColumn4Tag(tlv::tag_func))
	{
		int level = conn->getMainWindow()->cutNamespaceLevel();
		paintTokenized(painter, option4, index, QString("[::]"), "::", level);
	}
	else if (conn->sessionState().getDictCtx().m_names.size() && index.column() == m_session_state.findColumn4Tag(tlv::tag_ctx))
	{
		paintContext(painter, option4, index);
	}
	else if (index.column() == m_session_state.findColumn4Tag(tlv::tag_time))
	{
		paintTime(painter, option4, index);
	}
	else
	{
		QStyledItemDelegate::paint(painter, option4, index);
	}
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

			E_LevelMode const lm = stringToLvlMod(qs.at(0).toLatin1());
			QString const verbose = lvlmodsStr[lm];
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

	if (m_session_state.getDictCtx().m_names.size())
	{
		QVariant const value = index.data(Qt::DisplayRole);
		if (value.isValid() && !value.isNull())
		{
			Dict const & dict = m_session_state.getDictCtx();

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

