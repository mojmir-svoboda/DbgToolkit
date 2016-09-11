#include "logdelegate.h"
#include <QObject>
#include <QPainter>
#include <models/filterproxymodel.h>

namespace logs {

	LogDelegate::LogDelegate (LogWidget & lw, AppData const & ad, QObject * parent)
		: QStyledItemDelegate(parent), m_log_widget(lw), m_app_data(ad)
	{ }

	void LogDelegate::paintLevel (QPainter * painter, QStyleOptionViewItemV4 & option4, QModelIndex const & index) const
	{
// 		QVariant const value = index.data(Qt::DisplayRole);
// 		if (value.isValid() && !value.isNull())
// 		{
// 			Dict const & dict = m_app_data.m_dict_lvl;
// 
// 			option4.text = dict.findNameFor(value.toULongLong());
// 			QWidget const * widget = option4.widget;
// 			if (widget)
// 			{
// 				QStyle * style = widget->style();
// 				style->drawControl(QStyle::CE_ItemViewItem, &option4, painter, widget);
// 			}
// 		}
	}

	void LogDelegate::paintContext (QPainter * painter, QStyleOptionViewItemV4 & option4, QModelIndex const & index) const
	{
	}

	void LogDelegate::paintTime (QPainter * painter, QStyleOptionViewItemV4 & option4, QModelIndex const & index, unsigned long long ref_t) const
	{
		QVariant const value = index.data(Qt::DisplayRole);
		if (value.isValid() && !value.isNull())
		{
			QVariant value = index.data(Qt::DisplayRole);
			uint64_t const t = value.toULongLong();
			uint64_t const ref_dt = t - ref_t;
			uint64_t used_t = t;

			option4.text.clear();

			/*if (m_log_widget.config().m_dt_enabled)
			{
				if (ref_dt >= 0)
				{
					QAbstractItemModel const * model = m_log_widget.m_tableview->model();
					
					int const col_idx = const_cast<logs::LogWidget &>(m_log_widget).findColumn4Tag(proto::tag_dt);
					
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
			else*/
			{
				if (ref_dt >= 0)
					used_t = ref_dt;
			}

			if (value.isValid())
			{
				float const t_us = static_cast<float>(used_t);
				float const t_natural_units = 1000000.0f; // microseconds
				float const t = t_us / t_natural_units / m_log_widget.config().m_time_units;
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


	double lerp (double x0, double x1, double x)
	{
		return x0 * (1.0 - x) + x1 * x;
	}
	QColor interpolate (QColor c0, QColor c1, double t)
	{
		double const r = lerp(c0.redF(), c1.redF(), t);
		double const g = lerp(c0.greenF(), c1.greenF(), t);
		double const b = lerp(c0.blueF(), c1.blueF(), t);
		QColor ct = QColor::fromRgbF(r, g, b);
		return ct;
	}

	template <typename T>
	T clamp (const T & n, const T & lower, const T & upper)
	{
		return std::max(lower, std::min(n, upper));
	}

	void LogDelegate::paintDiffTime (QPainter * painter, QStyleOptionViewItemV4 & option4, QModelIndex const & index) const
	{
		QColor const cols[] = { QColor(Qt::white), QColor(Qt::yellow), QColor(Qt::red), QColor(Qt::magenta), QColor(Qt::cyan) };
		float const stops[] = { 0.0f, 30.0f * 1000.0f, 250.0f * 1000.0f, 1000.0f * 1000.0f, 2500.0f * 1000.0f };

		bool const colorize = m_log_widget.config().m_dt_colorize;
		QVariant const value = index.data(Qt::DisplayRole);
		if (value.isValid() && !value.isNull())
		{
			QVariant value = index.data(Qt::DisplayRole);
			uint64_t const t64 = value.toULongLong();

			option4.text.clear();
			if (colorize)
			{
				double const t = static_cast<double>(t64);

				int idx = 0;
				for (size_t n = 0; n < sizeof(stops) / sizeof(*stops) - 1; ++n)
				{
					float const stop = stops[n];
					if (t > stop)
						idx = n;
					else
						break;
				}

				double const col_t = (t - stops[idx]) / (stops[idx + 1] - stops[idx]);
				double const c_t  = clamp(col_t, 0.0, 1.0);
				QColor ct = interpolate(cols[idx], cols[idx + 1], c_t);

				painter->save();
				painter->setBrush(QBrush(ct));
				painter->drawRect(option4.rect);
				painter->restore();
			}
			//float const t_us = static_cast<float>(t64);
			option4.text = QString::number(t64);
			//float const t_natural_units = 1000000.0f; // microseconds
			//float const t = t_us / t_natural_units / m_log_widget.config().m_time_units;
			//option4.text = QString::number(t, 'f', 3);

			QWidget const * widget = option4.widget;
			if (widget)
			{
				QStyle * style = widget->style();
				style->drawControl(QStyle::CE_ItemViewItem, &option4, painter, widget);
			}
		}
	}


	void LogDelegate::paintTokenized (QPainter * painter, QStyleOptionViewItemV4 & option4, QModelIndex const & index, QString const & separator, QString const & out_separator, int level) const
	{
		QVariant value = index.data(Qt::DisplayRole);
		if (value.isValid() && !value.isNull())
		{
			QStringList list = value.toString().split(QRegularExpression(separator), QString::SkipEmptyParts);
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

	void LogDelegate::paintTokenized2 (QPainter * painter, QStyleOptionViewItemV4 & option4, QModelIndex const & index, QString const & separator, QString const & out_separator, int level) const
	{
		QVariant value = index.data(Qt::DisplayRole);
		if (value.isValid() && !value.isNull())
		{
			QString const & str = value.toString();
			int const ln = str.length();
			int l = -1;
			for (QChar c : separator)
			{
				int c_l = -1;
				for (int i = 0; i < level; ++i)
				{
					int const last_idx = str.lastIndexOf(c, c_l);
					if (last_idx == -1)
						break; // no more separator(s)
					else
					{
						int const idx = last_idx - ln - 1;
						c_l = idx;
					}
				}
				if (c_l != -1)
					l = c_l;
			}

			if (l != -1)
			{
				QStringRef r = str.rightRef(l);
				option4.text = "r"; // omg, what now? :)
			}

			QWidget const * widget = option4.widget;
			if (widget)
			{
				QStyle * style = widget->style();
				style->drawControl(QStyle::CE_ItemViewItem, &option4, painter, widget);
			}
		}
	}


	void LogDelegate::paintHilited (QPainter * painter, QStyleOptionViewItemV4 & option, QModelIndex const & index) const
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

	void LogDelegate::paintMessage (QPainter * painter, QStyleOptionViewItemV4 & option4, QModelIndex const & index) const
	{
// 		QVariant const value = index.data(Qt::DisplayRole);
// 		if (value.isValid() && !value.isNull())
// 		{
//       QString const & s = value.toString();
//       option4.text = s;
//       int const row = index.row();
// 			QWidget const * widget = option4.widget;
// 			if (widget)
// 			{
// 				QStyle * style = widget->style();
// 				style->drawControl(QStyle::CE_ItemViewItem, &option4, painter, widget);
// 			}
// 		}
	}

	void LogDelegate::paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const
	{
		painter->save();
		QStyleOptionViewItemV4 option4 = option;
		initStyleOption(&option4, index);

		int const tag = proto::getTagForCol(index.column());

		if (tag < proto::e_max_tag_count)
		{
			TagDesc const & desc = m_log_widget.m_config.m_tag_config.findTag(tag);
			option4.displayAlignment = static_cast<Qt::Alignment>(1 << desc.m_align);
			option4.textElideMode = static_cast<Qt::TextElideMode>(desc.m_elide);
			switch (tag)
			{
				case proto::tag_file:
// 					if (m_log_widget.config().m_cut_path)
// 						//paintTokenized(painter, option4, index, QString("[:/\\\\]"), "/", m_log_widget.config().m_cut_path_level);
// 						//paintTokenized2(painter, option4, index, QString("/\\"), "/", m_log_widget.config().m_cut_path_level);
// 					else
						QStyledItemDelegate::paint(painter, option4, index); // no cutting
					break;
				case proto::tag_func:
// 					if (m_log_widget.config().m_cut_namespaces)
// 						//paintTokenized2(painter, option4, index, QString("::"), "::",  m_log_widget.config().m_cut_namespace_level);
// 						//paintTokenized(painter, option4, index, QString("::"), "::", m_log_widget.config().m_cut_namespace_level);
// 					//else
						QStyledItemDelegate::paint(painter, option4, index); // no cutting
					break;
// 				case proto::tag_ctx:
// 					if (m_app_data.m_dict_ctx.size())
// 						paintContext(painter, option4, index);
// 					else
// 						QStyledItemDelegate::paint(painter, option4, index); // no dictionnary
// 					break;
				case proto::tag_lvl:
//					if (m_app_data.m_dict_lvl.size())
//						paintLevel(painter, option4, index);
//					else
					QStyledItemDelegate::paint(painter, option4, index); // no dictionnary
					break;
				case proto::tag_ctime:
				{
					unsigned long long const ref_t = m_log_widget.ctimeRefValue();
					paintTime(painter, option4, index, ref_t);
					break;
				}
				case proto::tag_stime:
				{
					unsigned long long const ref_t = m_log_widget.stimeRefValue();
					paintTime(painter, option4, index, ref_t);
					break;
				}
				case proto::tag_dt:
				{
					paintDiffTime(painter, option4, index);
					break;
				}
				case proto::tag_msg:
					QStyledItemDelegate::paint(painter, option4, index);
					//paintMessage(painter, option4, index);
					break;
				
				default:
					QStyledItemDelegate::paint(painter, option4, index);
					break;
			}
		}
		else
		{
			// user tags
			QStyledItemDelegate::paint(painter, option4, index);
		}

		painter->restore();
	}
}


