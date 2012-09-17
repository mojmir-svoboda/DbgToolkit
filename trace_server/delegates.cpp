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
			for (size_t i = list.size() - level, ie = list.size(); i < ie; ++i)
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
	E_Align const align = stringToAlign(column_aligns[index.column()].at(0).toAscii());
	option4.displayAlignment = static_cast<Qt::Alignment>(1 << align);
	columns_elide_t const & column_elides = *m_session_state.getColumnsElideTemplate();
	E_Elide const elide = stringToElide(column_elides[index.column()].at(0).toAscii());
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
	else if (index.column() == m_session_state.findColumn4Tag(tlv::tag_ctx))
	{
		paintContext(painter, option4, index);
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
				option4.text = QString::fromAscii(trace::enumToString(static_cast<trace::E_TraceLevel>(lvl)));
			}

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

	/*if (index.column() == 0)
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
	else*/
		QStyledItemDelegate::paint(painter, option4, index);
	painter->restore();
}


